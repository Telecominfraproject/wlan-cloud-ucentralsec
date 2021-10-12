//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_user_handler.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Utils.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_errors.h"
#include "SMSSender.h"

namespace OpenWifi {
    void RESTAPI_user_handler::DoGet() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        SecurityObjects::UserInfo   UInfo;
        if(!Storage()->GetUserById(Id,UInfo)) {
            return NotFound();
        }
        Poco::JSON::Object  UserInfoObject;
        UInfo.to_json(UserInfoObject);
        ReturnObject(UserInfoObject);
    }

    void RESTAPI_user_handler::DoDelete() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        SecurityObjects::UserInfo UInfo;
        if(!Storage()->GetUserById(Id,UInfo)) {
            return NotFound();
        }

        if(!Storage()->DeleteUser(UserInfo_.userinfo.email,Id)) {
            return NotFound();
        }

        if(AuthService()->DeleteUserFromCache(UInfo.email))
            ;
        Logger_.information(Poco::format("Remove all tokens for '%s'", UserInfo_.userinfo.email));
        Storage()->RevokeAllTokens(UInfo.email);
        Logger_.information(Poco::format("User '%s' deleted by '%s'.",Id,UserInfo_.userinfo.email));
        OK();
    }

    void RESTAPI_user_handler::DoPost() {
        std::string Id = GetBinding("id", "");
        if(Id!="0") {
            return BadRequest(RESTAPI::Errors::IdMustBe0);
        }

        SecurityObjects::UserInfo   UInfo;
        RESTAPI_utils::from_request(UInfo,*Request);

        if(UInfo.userRole == SecurityObjects::UNKNOWN) {
            return BadRequest(RESTAPI::Errors::InvalidUserRole);
        }

        Poco::toLowerInPlace(UInfo.email);
        if(!Utils::ValidEMailAddress(UInfo.email)) {
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        if(!UInfo.currentPassword.empty()) {
            if(!AuthService()->ValidatePassword(UInfo.currentPassword)) {
                return BadRequest(RESTAPI::Errors::InvalidPassword);
            }
        }

        if(UInfo.name.empty())
            UInfo.name = UInfo.email;

        if(!Storage()->CreateUser(UInfo.email,UInfo)) {
            Logger_.information(Poco::format("Could not add user '%s'.",UInfo.email));
            return BadRequest(RESTAPI::Errors::RecordNotCreated);
        }

        if(GetParameter("email_verification","false")=="true") {
            if(AuthService::VerifyEmail(UInfo))
                Logger_.information(Poco::format("Verification e-mail requested for %s",UInfo.email));
            Storage()->UpdateUserInfo(UserInfo_.userinfo.email,UInfo.Id,UInfo);
        }

        if(!Storage()->GetUserByEmail(UInfo.email, UInfo)) {
            Logger_.information(Poco::format("User '%s' but not retrieved.",UInfo.email));
            return NotFound();
        }

        Poco::JSON::Object  UserInfoObject;
        UInfo.to_json(UserInfoObject);

        ReturnObject(UserInfoObject);

        Logger_.information(Poco::format("User '%s' has been added by '%s')",UInfo.email, UserInfo_.userinfo.email));
    }

    void RESTAPI_user_handler::DoPut() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        SecurityObjects::UserInfo   Existing;
        if(!Storage()->GetUserById(Id,Existing)) {
            return NotFound();
        }

        SecurityObjects::UserInfo   NewUser;
        auto RawObject = ParseStream();
        if(!NewUser.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        // some basic validations
        if(RawObject->has("userRole") && SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString())==SecurityObjects::UNKNOWN) {
            return BadRequest(RESTAPI::Errors::InvalidUserRole);
        }

        // The only valid things to change are: changePassword, name,
        AssignIfPresent(RawObject,"name", Existing.name);
        AssignIfPresent(RawObject,"description", Existing.description);
        AssignIfPresent(RawObject,"owner", Existing.owner);
        AssignIfPresent(RawObject,"location", Existing.location);
        AssignIfPresent(RawObject,"locale", Existing.locale);
        AssignIfPresent(RawObject,"changePassword", Existing.changePassword);
        AssignIfPresent(RawObject,"suspended", Existing.suspended);
        AssignIfPresent(RawObject,"blackListed", Existing.blackListed);

        if(RawObject->has("userRole"))
            Existing.userRole = SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString());
        if(RawObject->has("notes")) {
            SecurityObjects::NoteInfoVec NIV;
            NIV = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(RawObject->get("notes").toString());
            for(auto const &i:NIV) {
                SecurityObjects::NoteInfo   ii{.created=(uint64_t)std::time(nullptr), .createdBy=UserInfo_.userinfo.email, .note=i.note};
                Existing.notes.push_back(ii);
            }
        }
        if(RawObject->has("currentPassword")) {
            if(!AuthService()->ValidatePassword(RawObject->get("currentPassword").toString())) {
                return BadRequest(RESTAPI::Errors::InvalidPassword);
            }
            if(!AuthService()->SetPassword(RawObject->get("currentPassword").toString(),Existing)) {
                return BadRequest(RESTAPI::Errors::PasswordRejected);
            }
        }

        if(GetParameter("email_verification","false")=="true") {
            if(AuthService::VerifyEmail(Existing))
                Logger_.information(Poco::format("Verification e-mail requested for %s",Existing.email));
        }

        if(RawObject->has("userTypeProprietaryInfo")) {
            Existing.userTypeProprietaryInfo.mfa.enabled = NewUser.userTypeProprietaryInfo.mfa.enabled;
            if(NewUser.userTypeProprietaryInfo.mfa.method=="sms") {
                Existing.userTypeProprietaryInfo.mfa.method=NewUser.userTypeProprietaryInfo.mfa.method;
                auto MobileStruct = RawObject->get("userTypeProprietaryInfo");
                auto Info = MobileStruct.extract<Poco::JSON::Object::Ptr>();
                if(Info->isArray("mobiles")) {
                    Existing.userTypeProprietaryInfo.mobiles = NewUser.userTypeProprietaryInfo.mobiles;
                }
                if(!NewUser.userTypeProprietaryInfo.mobiles.empty() && !SMSSender()->IsNumberValid(NewUser.userTypeProprietaryInfo.mobiles[0].number,UserInfo_.userinfo.email)){
                    return BadRequest(RESTAPI::Errors::NeedMobileNumber);
                }
                if(NewUser.userTypeProprietaryInfo.mfa.enabled && Existing.userTypeProprietaryInfo.mobiles.empty()) {
                    return BadRequest(RESTAPI::Errors::NeedMobileNumber);
                }
            } else if(NewUser.userTypeProprietaryInfo.mfa.method=="email") {
                Existing.userTypeProprietaryInfo.mfa.method=NewUser.userTypeProprietaryInfo.mfa.method;
            } else {
                return BadRequest(RESTAPI::Errors::BadMFAMethod);
            }
        }

        if(Storage()->UpdateUserInfo(UserInfo_.userinfo.email,Id,Existing)) {
            SecurityObjects::UserInfo   NewUserInfo;
            Storage()->GetUserByEmail(UserInfo_.userinfo.email,NewUserInfo);
            Poco::JSON::Object  ModifiedObject;
            NewUserInfo.to_json(ModifiedObject);
            return ReturnObject(ModifiedObject);
        }
        BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }
}