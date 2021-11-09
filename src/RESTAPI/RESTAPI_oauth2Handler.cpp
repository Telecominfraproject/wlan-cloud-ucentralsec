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
            return UnAuthorized("Not authorized.");
        }
        bool GetMe = GetBoolParameter(RESTAPI::Protocol::ME, false);
        if(GetMe) {
            Logger_.information(Poco::format("REQUEST-ME(%s): Request for %s", Request->clientAddress().toString(), UserInfo_.userinfo.email));
            Poco::JSON::Object Me;
            UserInfo_.userinfo.to_json(Me);
            return ReturnObject(Me);
        }
        BadRequest("Ill-formed request. Please consult documentation.");
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
            //  Send an email to the userId
            Logger_.information(Poco::format("FORGOTTEN-PASSWORD(%s): Request for %s", Request->clientAddress().toString(), userId));
            SecurityObjects::ActionLink NewLink;

            NewLink.action = AuthService::EMailReasons[AuthService::FORGOT_PASSWORD];
            NewLink.id = MicroService::instance().CreateUUID();
            NewLink.userId = userId;
            NewLink.created = std::time(nullptr);
            NewLink.expires = NewLink.created + (24*60*60);
            Storage().CreateAction(NewLink);

            Poco::JSON::Object ReturnObj;
            SecurityObjects::UserInfoAndPolicy UInfo;
            UInfo.webtoken.userMustChangePassword = true;
            UInfo.webtoken.to_json(ReturnObj);
            return ReturnObject(ReturnObj);
        }

        if(GetBoolParameter(RESTAPI::Protocol::RESENDMFACODE,false)) {
            Logger_.information(Poco::format("RESEND-MFA-CODE(%s): Request for %s", Request->clientAddress().toString(), userId));
            if(Obj->has("uuid")) {
                auto uuid = Obj->get("uuid").toString();
                if(MFAServer().ResendCode(uuid))
                    return OK();
                return UnAuthorized("Unrecognized credentials (username/password).");
            }
            return UnAuthorized("Unrecognized credentials (username/password).");
        }

        if(GetBoolParameter(RESTAPI::Protocol::COMPLETEMFACHALLENGE,false)) {
            Logger_.information(Poco::format("COMPLETE-MFA-CHALLENGE(%s): Request for %s", Request->clientAddress().toString(), userId));
            if(Obj->has("uuid")) {
                SecurityObjects::UserInfoAndPolicy UInfo;
                if(MFAServer().CompleteMFAChallenge(Obj,UInfo)) {
                    Poco::JSON::Object ReturnObj;
                    UInfo.webtoken.to_json(ReturnObj);
                    return ReturnObject(ReturnObj);
                }
            }
            return UnAuthorized("Unrecognized credentials (username/password).");
        }

        SecurityObjects::UserInfoAndPolicy UInfo;
        auto Code=AuthService()->Authorize(userId, password, newPassword, UInfo);
        if (Code==AuthService::SUCCESS) {
            Poco::JSON::Object ReturnObj;
            if(AuthService()->RequiresMFA(UInfo)) {
                if(MFAServer().StartMFAChallenge(UInfo, ReturnObj)) {
                    return ReturnObject(ReturnObj);
                }
                Logger_.warning("MFA Seems ot be broken. Please fix. Disabling MFA checking for now.");
            }
            UInfo.webtoken.to_json(ReturnObj);
            return ReturnObject(ReturnObj);
        } else {
            switch(Code) {
                case AuthService::INVALID_CREDENTIALS: return UnAuthorized("Unrecognized credentials (username/password)."); break;
                case AuthService::PASSWORD_INVALID: return UnAuthorized("Invalid password."); break;
                case AuthService::PASSWORD_ALREADY_USED: return UnAuthorized("Password already used previously."); break;
                case AuthService::USERNAME_PENDING_VERIFICATION: return UnAuthorized("User access pending email verification."); break;
                case AuthService::PASSWORD_CHANGE_REQUIRED: return UnAuthorized("Password change expected."); break;
                default: return UnAuthorized("Unrecognized credentials (username/password)."); break;
            }
            return;
        }
	}
}