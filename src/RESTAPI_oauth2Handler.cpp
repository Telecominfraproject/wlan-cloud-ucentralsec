//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "AuthService.h"
#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_server.h"

#include "Utils.h"

namespace OpenWifi {
	void RESTAPI_oauth2Handler::DoGet() {
	    std::cout << __LINE__ << std::endl;
	    try {
	        if (!IsAuthorized()) {
	            UnAuthorized("Not authorized.");
	            return;
	        }
	        bool GetMe = GetBoolParameter(RESTAPI::Protocol::ME, false);
	        if(GetMe) {
	            Poco::JSON::Object Me;
	            UserInfo_.userinfo.to_json(Me);
	            ReturnObject(Me);
	            return;
	        }
	        BadRequest("Ill-formed request. Please consult documentation.");
	        return;
	    } catch(const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    BadRequest("Internal error has occurred. Please try later.");
	}

    void RESTAPI_oauth2Handler::DoDelete() {
	    try {
	        if (!IsAuthorized()) {
	            UnAuthorized("Not authorized.");
	            return;
	        }
	        auto Token = GetBinding(RESTAPI::Protocol::TOKEN, "...");
	        if (Token == SessionToken_) {
	            AuthService()->Logout(Token);
	            ReturnStatus(Poco::Net::HTTPResponse::HTTP_NO_CONTENT, true);
	        } else {
	            NotFound();
	        }
	        return;
	    } catch(const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    BadRequest("Internal error has occurred. Please try later.");
	}

	void RESTAPI_oauth2Handler::DoPost() {
	    std::cout << __LINE__ << std::endl;
	    try {

	        // Extract the info for login...
	        std::cout << __LINE__ << std::endl;
	        auto Obj = ParseStream();
	        std::cout << __LINE__ << std::endl;
	        auto userId = GetS(RESTAPI::Protocol::USERID, Obj);
	        auto password = GetS(RESTAPI::Protocol::PASSWORD, Obj);
	        auto newPassword = GetS(RESTAPI::Protocol::NEWPASSWORD, Obj);
	        Poco::toLowerInPlace(userId);

	        std::cout << __LINE__ << std::endl;

	        if(GetBoolParameter(RESTAPI::Protocol::REQUIREMENTS, false)) {
	            std::cout << __LINE__ << std::endl;
	            Poco::JSON::Object  Answer;
	            Answer.set(RESTAPI::Protocol::PASSWORDPATTERN, AuthService()->PasswordValidationExpression());
	            Answer.set(RESTAPI::Protocol::ACCESSPOLICY, RESTAPI_Server()->GetAccessPolicy());
	            Answer.set(RESTAPI::Protocol::PASSWORDPOLICY, RESTAPI_Server()->GetPasswordPolicy());
	            ReturnObject(Answer);
	            return;
	        }
	        std::cout << __LINE__ << std::endl;

	        if(GetBoolParameter(RESTAPI::Protocol::FORGOTPASSWORD,false)) {
	            std::cout << __LINE__ << std::endl;
	            //  Send an email to the userId
	            SecurityObjects::UserInfoAndPolicy UInfo;
	            if(AuthService::SendEmailToUser(userId,AuthService::FORGOT_PASSWORD))
	                Logger_.information(Poco::format("Send password reset link to %s",userId));
	            UInfo.webtoken.userMustChangePassword=true;
	            Poco::JSON::Object ReturnObj;
	            UInfo.webtoken.to_json(ReturnObj);
	            ReturnObject(ReturnObj);
	            std::cout << __LINE__ << std::endl;
	            return;
	        }

	        SecurityObjects::UserInfoAndPolicy UInfo;
	        std::cout << __LINE__ << std::endl;

	        auto Code=AuthService()->Authorize(userId, password, newPassword, UInfo);
	        std::cout << __LINE__ << std::endl;
	        if (Code==AuthService::SUCCESS) {
	            Poco::JSON::Object ReturnObj;
	            UInfo.webtoken.to_json(ReturnObj);
	            ReturnObject(ReturnObj);
	            std::cout << __LINE__ << std::endl;
	            return;
	        } else {
	            std::cout << __LINE__ << std::endl;
	            switch(Code) {
	                case AuthService::INVALID_CREDENTIALS: UnAuthorized("Unrecognized credentials (username/password)."); break;
	                case AuthService::PASSWORD_INVALID: UnAuthorized("Invalid password."); break;
	                case AuthService::PASSWORD_ALREADY_USED: UnAuthorized("Password already used previously."); break;
	                case AuthService::USERNAME_PENDING_VERIFICATION: UnAuthorized("User access pending email verification."); break;
	                case AuthService::PASSWORD_CHANGE_REQUIRED: UnAuthorized("Password change expected."); break;
	                default: UnAuthorized("Unrecognized credentials (username/password)."); break;
	            }
	            return;
	        }
	        std::cout << __LINE__ << std::endl;
	    } catch(const Poco::Exception &E) {
	        std::cout << __LINE__ << std::endl;
	        Logger_.log(E);
	    }
	    std::cout << __LINE__ << std::endl;
	    BadRequest("Internal error has occurred. Please try later.");
	}
}