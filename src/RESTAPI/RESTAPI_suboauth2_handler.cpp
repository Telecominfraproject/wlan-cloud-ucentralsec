//
// Created by stephane bourque on 2021-11-30.
//

#include "RESTAPI_suboauth2_handler.h"
#include "AuthService.h"
#include "MFAServer.h"
#include "framework/MicroService.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_suboauth2_handler::DoGet() {
        bool Expired = false, Contacted = false;
        if (!IsAuthorized(Expired, Contacted, true)) {
            if(Expired)
                return UnAuthorized(RESTAPI::Errors::EXPIRED_TOKEN);
            return UnAuthorized(RESTAPI::Errors::INVALID_TOKEN);
        }
        bool GetMe = GetBoolParameter(RESTAPI::Protocol::ME, false);
        if(GetMe) {
            Logger_.information(fmt::format("REQUEST-ME({}): Request for {}", Request->clientAddress().toString(),
                                             UserInfo_.userinfo.email));
            Poco::JSON::Object Me;
            SecurityObjects::UserInfo   ReturnedUser = UserInfo_.userinfo;
            Sanitize(UserInfo_, ReturnedUser);
            ReturnedUser.to_json(Me);
            return ReturnObject(Me);
        }
        BadRequest(RESTAPI::Errors::UnrecognizedRequest);
    }

    void RESTAPI_suboauth2_handler::DoDelete() {
        bool Expired = false, Contacted = false;
        if (!IsAuthorized(Expired, Contacted, true)) {
            if(Expired)
                return UnAuthorized(RESTAPI::Errors::EXPIRED_TOKEN);
            return UnAuthorized(RESTAPI::Errors::INVALID_TOKEN);
        }

        auto Token = GetBinding(RESTAPI::Protocol::TOKEN, "...");
        if (Token == SessionToken_) {
            AuthService()->SubLogout(Token);
            return ReturnStatus(Poco::Net::HTTPResponse::HTTP_NO_CONTENT, true);
        }

        Logger_.information(fmt::format("BAD-LOGOUT({}): Request for {}", Request->clientAddress().toString(), UserInfo_.userinfo.email));
        NotFound();
    }

    void RESTAPI_suboauth2_handler::DoPost() {
        const auto & Obj = ParsedBody_;
        auto userId = GetS(RESTAPI::Protocol::USERID, Obj);
        auto password = GetS(RESTAPI::Protocol::PASSWORD, Obj);
        auto newPassword = GetS(RESTAPI::Protocol::NEWPASSWORD, Obj);
        auto refreshToken = GetS("refresh_token", Obj);
        auto grant_type = GetParameter("grant_type");

        Poco::toLowerInPlace(userId);

        if(!refreshToken.empty() && grant_type == "refresh_token") {
            SecurityObjects::UserInfoAndPolicy UInfo;
            if(AuthService()->RefreshSubToken(*Request, refreshToken, UInfo)) {
                Poco::JSON::Object  Answer;
                UInfo.webtoken.to_json(Answer);
                return ReturnObject(Answer);
            } else {
                return UnAuthorized(RESTAPI::Errors::CANNOT_REFRESH_TOKEN);
            }
        }

        if(GetBoolParameter(RESTAPI::Protocol::REQUIREMENTS)) {
            Logger_.information(fmt::format("POLICY-REQUEST({}): Request.", Request->clientAddress().toString()));
            Poco::JSON::Object  Answer;
            Answer.set(RESTAPI::Protocol::PASSWORDPATTERN, AuthService()->SubPasswordValidationExpression());
            Answer.set(RESTAPI::Protocol::ACCESSPOLICY, AuthService()->GetSubAccessPolicy());
            Answer.set(RESTAPI::Protocol::PASSWORDPOLICY, AuthService()->GetSubPasswordPolicy());
            return ReturnObject(Answer);
        }

        if(GetBoolParameter(RESTAPI::Protocol::FORGOTPASSWORD)) {
            SecurityObjects::UserInfo UInfo1;
            auto UserExists = StorageService()->SubDB().GetUserByEmail(userId,UInfo1);
            if(UserExists) {
                Logger_.information(fmt::format("FORGOTTEN-PASSWORD({}): Request for {}", Request->clientAddress().toString(), userId));
                SecurityObjects::ActionLink NewLink;

                NewLink.action = OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD;
                NewLink.id = MicroService::CreateUUID();
                NewLink.userId = UInfo1.id;
                NewLink.created = std::time(nullptr);
                NewLink.expires = NewLink.created + (24*60*60);
                NewLink.userAction = false;
                StorageService()->ActionLinksDB().CreateAction(NewLink);

                Poco::JSON::Object ReturnObj;
                SecurityObjects::UserInfoAndPolicy UInfo;
                UInfo.webtoken.userMustChangePassword = true;
                UInfo.webtoken.to_json(ReturnObj);
                return ReturnObject(ReturnObj);
            } else {
                Poco::JSON::Object ReturnObj;
                SecurityObjects::UserInfoAndPolicy UInfo;
                UInfo.webtoken.userMustChangePassword = true;
                UInfo.webtoken.to_json(ReturnObj);
                return ReturnObject(ReturnObj);
            }
        }

        if(GetBoolParameter(RESTAPI::Protocol::RESENDMFACODE)) {
            Logger_.information(fmt::format("RESEND-MFA-CODE({}): Request for {}", Request->clientAddress().toString(), userId));
            if(Obj->has("uuid")) {
                auto uuid = Obj->get("uuid").toString();
                if(MFAServer()->ResendCode(uuid))
                    return OK();
            }
            return UnAuthorized(RESTAPI::Errors::BAD_MFA_TRANSACTION);
        }

        if(GetBoolParameter(RESTAPI::Protocol::COMPLETEMFACHALLENGE)) {
            Logger_.information(fmt::format("COMPLETE-MFA-CHALLENGE({}): Request for {}", Request->clientAddress().toString(), userId));
            if(Obj->has("uuid") && Obj->has("answer")) {
                SecurityObjects::UserInfoAndPolicy UInfo;
                if(MFAServer()->CompleteMFAChallenge(Obj,UInfo)) {
                    Poco::JSON::Object ReturnObj;
                    UInfo.webtoken.to_json(ReturnObj);
                    return ReturnObject(ReturnObj);
                }
            }
            return UnAuthorized(RESTAPI::Errors::MFA_FAILURE);
        }

        SecurityObjects::UserInfoAndPolicy UInfo;
        bool Expired=false;
        auto Code=AuthService()->AuthorizeSub(userId, password, newPassword, UInfo, Expired);
        if (Code==SUCCESS) {
            Poco::JSON::Object ReturnObj;
            if(AuthService()->RequiresMFA(UInfo)) {
                if(MFAServer()->StartMFAChallenge(UInfo, ReturnObj)) {
                    return ReturnObject(ReturnObj);
                }
                Logger_.warning("MFA Seems to be broken. Please fix. Disabling MFA checking for now.");
            }
            UInfo.webtoken.to_json(ReturnObj);
            return ReturnObject(ReturnObj);
        } else {
            switch(Code) {
                case INVALID_CREDENTIALS:
                    return UnAuthorized(RESTAPI::Errors::INVALID_CREDENTIALS);
                case PASSWORD_INVALID:
                    return UnAuthorized(RESTAPI::Errors::PASSWORD_INVALID);
                case PASSWORD_ALREADY_USED:
                    return UnAuthorized(RESTAPI::Errors::PASSWORD_ALREADY_USED);
                case USERNAME_PENDING_VERIFICATION:
                    return UnAuthorized(RESTAPI::Errors::USERNAME_PENDING_VERIFICATION);
                case PASSWORD_CHANGE_REQUIRED:
                    return UnAuthorized(RESTAPI::Errors::PASSWORD_CHANGE_REQUIRED);
                default:
                    return UnAuthorized(RESTAPI::Errors::INVALID_CREDENTIALS); break;
            }
            return;
        }
    }
}