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
            BadRequest(RESTAPI::Errors::MissingUserID);
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
            BadRequest(RESTAPI::Errors::MissingUserID);
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
            ;
        Logger_.information(Poco::format("Remove all tokens for '%s'", UserInfo_.userinfo.email));
        Storage()->RevokeAllTokens(UInfo.email);
        Logger_.information(Poco::format("User '%s' deleted by '%s'.",Id,UserInfo_.userinfo.email));
        OK();
    }

    void RESTAPI_user_handler::DoPost() {
        std::string Id = GetBinding("id", "");
        if(Id!="0") {
            BadRequest(RESTAPI::Errors::IdMustBe0);
            return;
        }

        SecurityObjects::UserInfo   UInfo;
        RESTAPI_utils::from_request(UInfo,*Request);

        if(UInfo.userRole == SecurityObjects::UNKNOWN) {
            BadRequest(RESTAPI::Errors::InvalidUserRole);
            return;
        }

        Poco::toLowerInPlace(UInfo.email);
        if(!Utils::ValidEMailAddress(UInfo.email)) {
            BadRequest(RESTAPI::Errors::InvalidEmailAddress);
            return;
        }

        if(!UInfo.currentPassword.empty()) {
            if(!AuthService()->ValidatePassword(UInfo.currentPassword)) {
                BadRequest(RESTAPI::Errors::InvalidPassword);
                return;
            }
        }

        if(UInfo.name.empty())
            UInfo.name = UInfo.email;

        if(!Storage()->CreateUser(UInfo.email,UInfo)) {
            Logger_.information(Poco::format("Could not add user '%s'.",UInfo.email));
            BadRequest(RESTAPI::Errors::RecordNotCreated);
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
                BadRequest(RESTAPI::Errors::InvalidPassword);
                return;
            }
            if(!AuthService()->SetPassword(RawObject->get("currentPassword").toString(),Existing)) {
                BadRequest(RESTAPI::Errors::PasswordRejected);
                return;
            }
        }

        if(GetParameter("email_verification","false")=="true") {
            if(AuthService::VerifyEmail(Existing))
                Logger_.information(Poco::format("Verification e-mail requested for %s",Existing.email));
        }

        if(NewUser.userTypeProprietaryInfo.mfa.enabled!=Existing.userTypeProprietaryInfo.mfa.enabled) {
            std::cout << "Saving MFA" << std::endl;
            if(!NewUser.userTypeProprietaryInfo.mfa.enabled) {
                Existing.userTypeProprietaryInfo.mfa.enabled=false;
            } else {
                //  Need to make sure the provided number has been validated.
                if(NewUser.userTypeProprietaryInfo.mfa.method=="sms") {
                    std::cout << "Saving in sms" << std::endl;
                    if(NewUser.userTypeProprietaryInfo.mobiles.empty()) {
                        return BadRequest(RESTAPI::Errors::NeedMobileNumber);
                    }
                    if(!SMSSender()->IsNumberValid(NewUser.userTypeProprietaryInfo.mobiles[0].number)){
                        return BadRequest(RESTAPI::Errors::NeedMobileNumber);
                    }
                    Existing.userTypeProprietaryInfo.mfa.method = "sms";
                    Existing.userTypeProprietaryInfo.mobiles = NewUser.userTypeProprietaryInfo.mobiles;
                    std::cout << "Saving in mobiles" << std::endl;
                } else if(NewUser.userTypeProprietaryInfo.mfa.method=="email") {

                } else {
                    return BadRequest(RESTAPI::Errors::BadMFAMethod);
                }
            }
            Existing.userTypeProprietaryInfo.mfa.enabled = NewUser.userTypeProprietaryInfo.mfa.enabled;
        }

        if(Storage()->UpdateUserInfo(UserInfo_.userinfo.email,Id,Existing)) {

            std::cout << "Saved data." << std::endl;

            SecurityObjects::UserInfo   NewUserInfo;
            Storage()->GetUserByEmail(UserInfo_.userinfo.email,NewUserInfo);

            Poco::JSON::Object  ModifiedObject;
            NewUserInfo.to_json(ModifiedObject);

            return ReturnObject(ModifiedObject);
        }
        BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }
}