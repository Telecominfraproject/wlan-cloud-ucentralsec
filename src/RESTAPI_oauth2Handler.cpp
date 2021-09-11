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
	void RESTAPI_oauth2Handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {

		if (!ContinueProcessing(Request, Response))
			return;

        ParseParameters(Request);
        if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            DoPost(Request, Response);
        } else if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DoDelete(Request, Response);
        } else if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            DoGet(Request, Response);
        } else {
            BadRequest(Request, Response, "Unsupported HTTP method.");
        }
	}

	void RESTAPI_oauth2Handler::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	    try {
	        if (!IsAuthorized(Request, Response)) {
	            UnAuthorized(Request, Response, "Not authorized.");
	            return;
	        }
	        bool GetMe = GetBoolParameter(RESTAPI::Protocol::ME, false);
	        if(GetMe) {
	            Poco::JSON::Object Me;
	            UserInfo_.userinfo.to_json(Me);
	            ReturnObject(Request, Me, Response);
	            return;
	        }
	        BadRequest(Request, Response, "Ill-fromed request. Please consult documentation.");
	        return;
	    } catch(const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    BadRequest(Request, Response, "Internal error has occurred. Please try later.");
	}

    void RESTAPI_oauth2Handler::DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	    try {
	        if (!IsAuthorized(Request, Response)) {
	            UnAuthorized(Request, Response, "Not authorized.");
	            return;
	        }
	        auto Token = GetBinding(RESTAPI::Protocol::TOKEN, "...");
	        if (Token == SessionToken_) {
	            AuthService()->Logout(Token);
	            ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_NO_CONTENT, true);
	        } else {
	            NotFound(Request, Response);
	        }
	        return;
	    } catch(const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    BadRequest(Request, Response, "Internal error has occurred. Please try later.");
	}

	void RESTAPI_oauth2Handler::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	    try {

	        // Extract the info for login...
	        Poco::JSON::Parser parser;
	        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
	        auto userId = GetS(RESTAPI::Protocol::USERID, Obj);
	        auto password = GetS(RESTAPI::Protocol::PASSWORD, Obj);
	        auto newPassword = GetS(RESTAPI::Protocol::NEWPASSWORD, Obj);
	        Poco::toLowerInPlace(userId);

	        if(GetBoolParameter(RESTAPI::Protocol::REQUIREMENTS, false)) {
	            Poco::JSON::Object  Answer;
	            Answer.set(RESTAPI::Protocol::PASSWORDPATTERN, AuthService()->PasswordValidationExpression());
	            Answer.set(RESTAPI::Protocol::ACCESSPOLICY, RESTAPI_Server()->GetAccessPolicy());
	            Answer.set(RESTAPI::Protocol::PASSWORDPOLICY, RESTAPI_Server()->GetPasswordPolicy());
	            ReturnObject(Request, Answer, Response);
	            return;
	        }

	        if(GetBoolParameter(RESTAPI::Protocol::FORGOTPASSWORD,false)) {
	            //  Send an email to the userId
	            SecurityObjects::UserInfoAndPolicy UInfo;
	            if(AuthService::SendEmailToUser(userId,AuthService::FORGOT_PASSWORD))
	                Logger_.information(Poco::format("Send password reset link to %s",userId));
	            UInfo.webtoken.userMustChangePassword=true;
	            Poco::JSON::Object ReturnObj;
	            UInfo.webtoken.to_json(ReturnObj);
	            ReturnObject(Request, ReturnObj, Response);
	            return;
	        }

	        SecurityObjects::UserInfoAndPolicy UInfo;

	        auto Code=AuthService()->Authorize(userId, password, newPassword, UInfo);
	        if (Code==AuthService::SUCCESS) {
	            Poco::JSON::Object ReturnObj;
	            UInfo.webtoken.to_json(ReturnObj);
	            ReturnObject(Request, ReturnObj, Response);
	            return;
	        } else {
	            switch(Code) {
	                case AuthService::INVALID_CREDENTIALS: UnAuthorized(Request, Response, "Unrecognized credentials (username/password)."); break;
	                case AuthService::PASSWORD_INVALID: UnAuthorized(Request, Response, "Invalid password."); break;
	                case AuthService::PASSWORD_ALREADY_USED: UnAuthorized(Request, Response, "Password already used previously."); break;
	                case AuthService::USERNAME_PENDING_VERIFICATION: UnAuthorized(Request, Response, "User access pending email verification."); break;
	                case AuthService::PASSWORD_CHANGE_REQUIRED: UnAuthorized(Request, Response, "Password change expected."); break;
	                default: UnAuthorized(Request, Response, "Unrecognized credentials (username/password)."); break;
	            }
	            return;
	        }
	    } catch(const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    BadRequest(Request, Response, "Internal error has occurred. Please try later.");
	}
}