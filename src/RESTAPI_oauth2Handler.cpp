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
#include "Utils.h"

namespace uCentral {
	void RESTAPI_oauth2Handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {

		if (!ContinueProcessing(Request, Response))
			return;

		try {
			if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
				// Extract the info for login...
				Poco::JSON::Parser parser;
				Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

				auto userId = GetS(uCentral::RESTAPI::Protocol::USERID, Obj);
				auto password = GetS(uCentral::RESTAPI::Protocol::PASSWORD, Obj);
				auto newPassword = GetS(uCentral::RESTAPI::Protocol::NEWPASSWORD, Obj);

                ParseParameters(Request);

                if(GetParameter("forgotPassword","false") == "true") {
                    //  Send an email to the userId
                    SecurityObjects::UserInfoAndPolicy UInfo;
                    AuthService()->SendEmailToUser(userId,AuthService::FORGOT_PASSWORD);
                    UInfo.webtoken.userMustChangePassword=true;
                    Poco::JSON::Object ReturnObj;
                    UInfo.webtoken.to_json(ReturnObj);
                    ReturnObject(Request, ReturnObj, Response);
                    return;
                }

				Poco::toLowerInPlace(userId);
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
			} else if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
				if (!IsAuthorized(Request, Response)) {
                    UnAuthorized(Request, Response, "Not authorized.");
					return;
				}
				auto Token = GetBinding(uCentral::RESTAPI::Protocol::TOKEN, "...");
				if (Token == SessionToken_) {
					AuthService()->Logout(Token);
					ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_NO_CONTENT, true);
				} else {
					NotFound(Request, Response);
				}
			} else {
				BadRequest(Request, Response, "Unsupported HTTP method.");
			}
			return;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		BadRequest(Request, Response);
	}
}