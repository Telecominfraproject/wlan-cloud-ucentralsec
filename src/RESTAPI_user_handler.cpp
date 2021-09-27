//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_user_handler.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Utils.h"
#include "RESTAPI_utils.h"

namespace OpenWifi {
    void RESTAPI_user_handler::DoGet() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            BadRequest("You must supply the ID of the user.");
            return;
        }

        SecurityObjects::UserInfo   UInfo;
        if(!Storage()->GetUserById(Id,UInfo)) {
            NotFound();
            return;
        }
        Poco::JSON::Object  UserInfoObject;
        UInfo.to_json(UserInfoObject);
        ReturnObject(UserInfoObject);
    }

    void RESTAPI_user_handler::DoDelete() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            BadRequest("You must supply the ID of the user.");
            return;
        }

        SecurityObjects::UserInfo UInfo;
        if(!Storage()->GetUserById(Id,UInfo)) {
            NotFound();
            return;
        }

        if(!Storage()->DeleteUser(UserInfo_.userinfo.email,Id)) {
            NotFound();
            return;
        }

        if(AuthService()->DeleteUserFromCache(UInfo.email))
            Logger_.information(Poco::format("Remove all tokens for '%s'", UserInfo_.userinfo.email));

        Storage()->RevokeAllTokens(UInfo.email);

        Logger_.information(Poco::format("User '%s' deleted by '%s'.",Id,UserInfo_.userinfo.email));
        OK();
    }

    void RESTAPI_user_handler::DoPost() {
        std::string Id = GetBinding("id", "");
        if(Id!="0") {
            BadRequest("To create a user, you must set the ID to 0");
            return;
        }

        SecurityObjects::UserInfo   UInfo;
        RESTAPI_utils::from_request(UInfo,*Request);

        if(UInfo.userRole == SecurityObjects::UNKNOWN) {
            BadRequest("Invalid userRole.");
            return;
        }

        Poco::toLowerInPlace(UInfo.email);
        if(!Utils::ValidEMailAddress(UInfo.email)) {
            BadRequest("Invalid email address.");
            return;
        }

        if(!UInfo.currentPassword.empty()) {
            if(!AuthService()->ValidatePassword(UInfo.currentPassword)) {
                BadRequest("Invalid password.");
                return;
            }
        }

        if(UInfo.name.empty())
            UInfo.name = UInfo.email;

        if(!Storage()->CreateUser(UInfo.email,UInfo)) {
            Logger_.information(Poco::format("Could not add user '%s'.",UInfo.email));
            BadRequest("Could not ad this user.");
            return;
        }

        if(GetParameter("email_verification","false")=="true") {
            if(AuthService::VerifyEmail(UInfo))
                Logger_.information(Poco::format("Verification e-mail requested for %s",UInfo.email));
            Storage()->UpdateUserInfo(UserInfo_.userinfo.email,UInfo.Id,UInfo);
        }

        if(!Storage()->GetUserByEmail(UInfo.email, UInfo)) {
            Logger_.information(Poco::format("User '%s' but not retrieved.",UInfo.email));
            NotFound();
            return;
        }

        Poco::JSON::Object  UserInfoObject;
        UInfo.to_json(UserInfoObject);

        ReturnObject(UserInfoObject);

        Logger_.information(Poco::format("User '%s' has been added by '%s')",UInfo.email, UserInfo_.userinfo.email));
    }

    void RESTAPI_user_handler::DoPut() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            BadRequest("You must supply the ID of the user.");
            return;
        }

        SecurityObjects::UserInfo   LocalObject;
        if(!Storage()->GetUserById(Id,LocalObject)) {
            NotFound();
            return;
        }

        // some basic validations
        auto RawObject = ParseStream();
        if(RawObject->has("userRole") && SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString())==SecurityObjects::UNKNOWN) {
            BadRequest("Bad userRole value.");
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
                BadRequest("Invalid password.");
                return;
            }
            if(!AuthService()->SetPassword(RawObject->get("currentPassword").toString(),LocalObject)) {
                BadRequest("Password was rejected. This maybe an old password.");
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
            ReturnObject(ModifiedObject);
            return;
        } else {
            BadRequest("Failed to update user.");
        }
    }
}