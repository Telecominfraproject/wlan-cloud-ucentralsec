//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "Daemon.h"
#include "AuthService.h"
#include "RESTAPI_oauth2Handler.h"
#include "MFAServer.h"
#include "framework/RESTAPI_protocol.h"
#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_oauth2Handler::DoGet() {
        if (!IsAuthorized()) {
            return UnAuthorized(RESTAPI::Errors::MissingAuthenticationInformation);
        }
        bool GetMe = GetBoolParameter(RESTAPI::Protocol::ME, false);
        if(GetMe) {
            Logger_.information(Poco::format("REQUEST-ME(%s): Request for %s", Request->clientAddress().toString(), UserInfo_.userinfo.email));
            Poco::JSON::Object Me;
            UserInfo_.userinfo.to_json(Me);
            return ReturnObject(Me);
        }
        BadRequest(RESTAPI::Errors::UnrecognizedRequest);
	}

    void RESTAPI_oauth2Handler::DoDelete() {
        if (!IsAuthorized()) {
            return UnAuthorized("Not authorized.");
        }

        auto Token = GetBinding(RESTAPI::Protocol::TOKEN, "...");
        if (Token == SessionToken_) {
            AuthService()->Logout(Token);
            return ReturnStatus(Poco::Net::HTTPResponse::HTTP_NO_CONTENT, true);
        }

        Logger_.information(Poco::format("BAD-LOGOUT(%s): Request for %s", Request->clientAddress().toString(), UserInfo_.userinfo.email));
        NotFound();
	}

	void RESTAPI_oauth2Handler::DoPost() {
        auto Obj = ParseStream();
        auto userId = GetS(RESTAPI::Protocol::USERID, Obj);
        auto password = GetS(RESTAPI::Protocol::PASSWORD, Obj);
        auto newPassword = GetS(RESTAPI::Protocol::NEWPASSWORD, Obj);

        Poco::toLowerInPlace(userId);

        if(GetBoolParameter(RESTAPI::Protocol::REQUIREMENTS, false)) {
            Logger_.information(Poco::format("POLICY-REQUEST(%s): Request.", Request->clientAddress().toString()));
            Poco::JSON::Object  Answer;
            Answer.set(RESTAPI::Protocol::PASSWORDPATTERN, AuthService()->PasswordValidationExpression());
            Answer.set(RESTAPI::Protocol::ACCESSPOLICY, Daemon()->GetAccessPolicy());
            Answer.set(RESTAPI::Protocol::PASSWORDPOLICY, Daemon()->GetPasswordPolicy());
            return ReturnObject(Answer);
        }

        if(GetBoolParameter(RESTAPI::Protocol::FORGOTPASSWORD,false)) {
            SecurityObjects::UserInfo UInfo1;
            auto UserExists = StorageService()->GetUserByEmail(userId,UInfo1);
            if(UserExists) {
                Logger_.information(Poco::format("FORGOTTEN-PASSWORD(%s): Request for %s", Request->clientAddress().toString(), userId));
                SecurityObjects::ActionLink NewLink;

                NewLink.action = OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD;
                NewLink.id = MicroService::instance().CreateUUID();
                NewLink.userId = UInfo1.Id;
                NewLink.created = std::time(nullptr);
                NewLink.expires = NewLink.created + (24*60*60);
                StorageService()->CreateAction(NewLink);

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

        if(GetBoolParameter(RESTAPI::Protocol::RESENDMFACODE,false)) {
            Logger_.information(Poco::format("RESEND-MFA-CODE(%s): Request for %s", Request->clientAddress().toString(), userId));
            if(Obj->has(RESTAPI::Protocol::UUID)) {
                auto uuid = Obj->get(RESTAPI::Protocol::UUID).toString();
                if(MFAServer().ResendCode(uuid))
                    return OK();
            }
            return UnAuthorized(RESTAPI::Errors::InvalidCredentials);
        }

        std::cout << __func__ << ":" << __LINE__ << std::endl;

        if(GetBoolParameter(RESTAPI::Protocol::COMPLETEMFACHALLENGE,false)) {
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            Logger_.information(Poco::format("COMPLETE-MFA-CHALLENGE(%s): Request for %s", Request->clientAddress().toString(), userId));
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            if(Obj->has(RESTAPI::Protocol::UUID)) {
                std::cout << __func__ << ":" << __LINE__ << std::endl;
                SecurityObjects::UserInfoAndPolicy UInfo;
                std::cout << __func__ << ":" << __LINE__ << std::endl;
                if(MFAServer().CompleteMFAChallenge(Obj,UInfo)) {
                    std::cout << __func__ << ":" << __LINE__ << std::endl;
                    Poco::JSON::Object ReturnObj;
                    std::cout << __func__ << ":" << __LINE__ << std::endl;
                    UInfo.webtoken.to_json(ReturnObj);
                    std::cout << __func__ << ":" << __LINE__ << std::endl;
                    return ReturnObject(ReturnObj);
                }
                std::cout << __func__ << ":" << __LINE__ << std::endl;
            }
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            return UnAuthorized(RESTAPI::Errors::InvalidCredentials);
        }

        SecurityObjects::UserInfoAndPolicy UInfo;
        auto Code=AuthService()->Authorize(userId, password, newPassword, UInfo);
        if (Code==SUCCESS) {
            Poco::JSON::Object ReturnObj;
            if(AuthService()->RequiresMFA(UInfo)) {
                if(MFAServer().StartMFAChallenge(UInfo, ReturnObj)) {
                    return ReturnObject(ReturnObj);
                }
                Logger_.warning("MFA Seems to be broken. Please fix. Disabling MFA checking for now.");
            }
            UInfo.webtoken.to_json(ReturnObj);
            return ReturnObject(ReturnObj);
        } else {

            switch(Code) {
                case INVALID_CREDENTIALS:
                    return UnAuthorized(RESTAPI::Errors::InvalidCredentials, Code);
                case PASSWORD_INVALID:
                    return UnAuthorized(RESTAPI::Errors::InvalidPassword, Code);
                case PASSWORD_ALREADY_USED:
                    return UnAuthorized(RESTAPI::Errors::PasswordRejected, Code);
                case USERNAME_PENDING_VERIFICATION:
                    return UnAuthorized(RESTAPI::Errors::UserPendingVerification, Code);
                case PASSWORD_CHANGE_REQUIRED:
                    return UnAuthorized(RESTAPI::Errors::PasswordMustBeChanged, Code);
                default:
                    return UnAuthorized(RESTAPI::Errors::InvalidCredentials); break;
            }
            return;
        }
	}
}