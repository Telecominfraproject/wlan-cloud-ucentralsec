//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_user_handler.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Utils.h"
#include "RESTAPI_utils.h"

namespace OpenWifi {
    void RESTAPI_user_handler::handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {

        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_POST)
            DoPost(Request, Response);
        else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_DELETE)
            DoDelete(Request, Response);
        else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_PUT)
            DoPut(Request, Response);
        else
            BadRequest(Request, Response, "Unimplemented HTTP Operation.");
    }

    void RESTAPI_user_handler::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if(Id.empty()) {
                BadRequest(Request, Response, "You must supply the ID of the user.");
                return;
            }

            SecurityObjects::UserInfo   UInfo;
            if(!Storage()->GetUserById(Id,UInfo)) {
                NotFound(Request, Response);
                return;
            }

            Poco::JSON::Object  UserInfoObject;
            UInfo.to_json(UserInfoObject);

            ReturnObject(Request, UserInfoObject, Response);
            return;
        } catch (const Poco::Exception &E ) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_user_handler::DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if(Id.empty()) {
                BadRequest(Request, Response, "You must supply the ID of the user.");
                return;
            }

            SecurityObjects::UserInfo UInfo;
            if(!Storage()->GetUserById(Id,UInfo)) {
                NotFound(Request, Response);
                return;
            }

            if(!Storage()->DeleteUser(UserInfo_.userinfo.email,Id)) {
                NotFound(Request, Response);
                return;
            }

            if(AuthService()->DeleteUserFromCache(UInfo.email))
                Logger_.information(Poco::format("Remove all tokens for '%s'", UserInfo_.userinfo.email));

            Storage()->RevokeAllTokens(UInfo.email);

            Logger_.information(Poco::format("User '%s' deleted by '%s'.",Id,UserInfo_.userinfo.email));
            OK(Request, Response);

            return;
        } catch (const Poco::Exception &E ) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_user_handler::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if(Id!="0") {
                BadRequest(Request, Response, "To create a user, you must set the ID to 0");
                return;
            }

            SecurityObjects::UserInfo   UInfo;
            RESTAPI_utils::from_request(UInfo,Request);

            if(UInfo.userRole == SecurityObjects::UNKNOWN) {
                BadRequest(Request, Response, "Invalid userRole.");
                return;
            }

            Poco::toLowerInPlace(UInfo.email);
            if(!Utils::ValidEMailAddress(UInfo.email)) {
                BadRequest(Request, Response, "Invalid email address.");
                return;
            }

            if(!UInfo.currentPassword.empty()) {
                if(!AuthService()->ValidatePassword(UInfo.currentPassword)) {
                    BadRequest(Request, Response, "Invalid password.");
                    return;
                }
            }

            if(UInfo.name.empty())
                UInfo.name = UInfo.email;

            if(!Storage()->CreateUser(UInfo.email,UInfo)) {
                Logger_.information(Poco::format("Could not add user '%s'.",UInfo.email));
                BadRequest(Request, Response);
                return;
            }

            if(GetParameter("email_verification","false")=="true") {
                if(AuthService::VerifyEmail(UInfo))
                    Logger_.information(Poco::format("Verification e-mail requested for %s",UInfo.email));
                Storage()->UpdateUserInfo(UserInfo_.userinfo.email,UInfo.Id,UInfo);
            }

            if(!Storage()->GetUserByEmail(UInfo.email, UInfo)) {
                Logger_.information(Poco::format("User '%s' but not retrieved.",UInfo.email));
                BadRequest(Request, Response);
                return;
            }

            Poco::JSON::Object  UserInfoObject;
            UInfo.to_json(UserInfoObject);

            ReturnObject(Request, UserInfoObject, Response);

            Logger_.information(Poco::format("User '%s' has been added by '%s')",UInfo.email, UserInfo_.userinfo.email));
            return;
        } catch (const Poco::Exception &E ) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_user_handler::DoPut(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if(Id.empty()) {
                BadRequest(Request, Response, "You must supply the ID of the user.");
                return;
            }

            SecurityObjects::UserInfo   LocalObject;
            if(!Storage()->GetUserById(Id,LocalObject)) {
                NotFound(Request, Response);
                return;
            }

            // some basic validations
            Poco::JSON::Parser IncomingParser;
            auto RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

            if(RawObject->has("userRole") && SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString())==SecurityObjects::UNKNOWN) {
                BadRequest(Request, Response, "Bad userRole value.");
                return;
            }

            // The only valid things to change are: changePassword, name,
            if(RawObject->has("name"))
                LocalObject.name = RawObject->get("name").toString();
            if(RawObject->has("description"))
                LocalObject.description = RawObject->get("description").toString();
            if(RawObject->has("avatar"))
                LocalObject.avatar = RawObject->get("avatar").toString();
            if(RawObject->has("changePassword"))
                LocalObject.changePassword = RawObject->get("changePassword").toString()=="true";
            if(RawObject->has("owner"))
                LocalObject.owner = RawObject->get("owner").toString();
            if(RawObject->has("location"))
                LocalObject.location = RawObject->get("location").toString();
            if(RawObject->has("locale"))
                LocalObject.locale = RawObject->get("locale").toString();
            if(RawObject->has("userRole"))
                LocalObject.userRole = SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString());
            if(RawObject->has("suspended"))
                LocalObject.suspended = RawObject->get("suspended").toString()=="true";
            if(RawObject->has("blackListed"))
                LocalObject.blackListed = RawObject->get("blackListed").toString()=="true";
            if(RawObject->has("notes")) {
                SecurityObjects::NoteInfoVec NIV;
                NIV = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(RawObject->get("notes").toString());
                for(auto const &i:NIV) {
                    SecurityObjects::NoteInfo   ii{.created=(uint64_t)std::time(nullptr), .createdBy=UserInfo_.userinfo.email, .note=i.note};
                    LocalObject.notes.push_back(ii);
                }
            }
            if(RawObject->has("currentPassword")) {
                if(!AuthService()->ValidatePassword(RawObject->get("currentPassword").toString())) {
                    BadRequest(Request, Response, "Invalid password.");
                    return;
                }
                if(!AuthService()->SetPassword(RawObject->get("currentPassword").toString(),LocalObject)) {
                    BadRequest(Request, Response, "Password was rejected. This maybe an old password.");
                    return;
                }
            }

            if(GetParameter("email_verification","false")=="true") {
                if(AuthService::VerifyEmail(LocalObject))
                    Logger_.information(Poco::format("Verification e-mail requested for %s",LocalObject.email));
            }

            if(Storage()->UpdateUserInfo(UserInfo_.userinfo.email,Id,LocalObject)) {
                Poco::JSON::Object  ModifiedObject;
                LocalObject.to_json(ModifiedObject);
                ReturnObject(Request, ModifiedObject, Response);
                return;
            }
        } catch( const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "Request rejected.");
    }
}