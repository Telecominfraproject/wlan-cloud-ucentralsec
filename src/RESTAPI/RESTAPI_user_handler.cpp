//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_user_handler.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "framework/RESTAPI_errors.h"
#include "SMSSender.h"
#include "ACLProcessor.h"

namespace OpenWifi {

    static void FilterCredentials(SecurityObjects::UserInfo & U) {
        U.currentPassword.clear();
        U.lastPasswords.clear();
        U.oauthType.clear();
    }

    void RESTAPI_user_handler::DoGet() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        Poco::toLowerInPlace(Id);
        std::string Arg;
        SecurityObjects::UserInfo   UInfo;
        if(HasParameter("byEmail",Arg) && Arg=="true") {
            if(!StorageService()->GetUserByEmail(Id,UInfo)) {
                return NotFound();
            }
        } else if(!StorageService()->GetUserById(Id,UInfo)) {
            return NotFound();
        }

        Poco::JSON::Object  UserInfoObject;
        FilterCredentials(UInfo);
        UInfo.to_json(UserInfoObject);
        ReturnObject(UserInfoObject);
    }

    void RESTAPI_user_handler::DoDelete() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        SecurityObjects::UserInfo UInfo;
        if(!StorageService()->GetUserById(Id,UInfo)) {
            return NotFound();
        }

        if(!ACLProcessor::Can(UserInfo_.userinfo, UInfo,ACLProcessor::DELETE)) {
            return UnAuthorized(RESTAPI::Errors::InsufficientAccessRights, ACCESS_DENIED);
        }

        if(!StorageService()->DeleteUser(UserInfo_.userinfo.email,Id)) {
            return NotFound();
        }

        if(AuthService()->DeleteUserFromCache(UInfo.email)) {
            // nothing to do
        }

        StorageService()->DeleteAvatar(UserInfo_.userinfo.email,Id);
        StorageService()->DeletePreferences(UserInfo_.userinfo.email,Id);

        Logger_.information(Poco::format("Remove all tokens for '%s'", UserInfo_.userinfo.email));
        StorageService()->RevokeAllTokens(UInfo.email);
        Logger_.information(Poco::format("User '%s' deleted by '%s'.",Id,UserInfo_.userinfo.email));
        OK();
    }

    void RESTAPI_user_handler::DoPost() {
        std::string Id = GetBinding("id", "");
        if(Id!="0") {
            return BadRequest(RESTAPI::Errors::IdMustBe0);
        }

        SecurityObjects::UserInfo   NewUser;
        RESTAPI_utils::from_request(NewUser,*Request);

        if(NewUser.userRole == SecurityObjects::UNKNOWN) {
            return BadRequest(RESTAPI::Errors::InvalidUserRole);
        }

        if(!ACLProcessor::Can(UserInfo_.userinfo,NewUser,ACLProcessor::CREATE)) {
            return UnAuthorized("Insufficient access rights.", ACCESS_DENIED);
        }

        Poco::toLowerInPlace(NewUser.email);
        if(!Utils::ValidEMailAddress(NewUser.email)) {
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        if(!NewUser.currentPassword.empty()) {
            if(!AuthService()->ValidatePassword(NewUser.currentPassword)) {
                return BadRequest(RESTAPI::Errors::InvalidPassword);
            }
        }

        if(NewUser.name.empty())
            NewUser.name = NewUser.email;

        if(!StorageService()->CreateUser(NewUser.email,NewUser)) {
            Logger_.information(Poco::format("Could not add user '%s'.",NewUser.email));
            return BadRequest(RESTAPI::Errors::RecordNotCreated);
        }

        if(GetParameter("email_verification","false")=="true") {
            if(AuthService::VerifyEmail(NewUser))
                Logger_.information(Poco::format("Verification e-mail requested for %s",NewUser.email));
            StorageService()->UpdateUserInfo(UserInfo_.userinfo.email,NewUser.Id,NewUser);
        }

        if(!StorageService()->GetUserByEmail(NewUser.email, NewUser)) {
            Logger_.information(Poco::format("User '%s' but not retrieved.",NewUser.email));
            return NotFound();
        }

        Poco::JSON::Object  UserInfoObject;
        FilterCredentials(NewUser);
        NewUser.to_json(UserInfoObject);
        ReturnObject(UserInfoObject);
        Logger_.information(Poco::format("User '%s' has been added by '%s')",NewUser.email, UserInfo_.userinfo.email));
    }

    void RESTAPI_user_handler::DoPut() {
        std::string Id = GetBinding("id", "");
        if(Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUserID);
        }

        SecurityObjects::UserInfo   Existing;
        if(!StorageService()->GetUserById(Id,Existing)) {
            return NotFound();
        }

        if(!ACLProcessor::Can(UserInfo_.userinfo,Existing,ACLProcessor::MODIFY)) {
            return UnAuthorized("Insufficient access rights.", ACCESS_DENIED);
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

        if(RawObject->has("userRole")) {
            auto NewRole = SecurityObjects::UserTypeFromString(RawObject->get("userRole").toString());
            if(NewRole!=Existing.userRole) {
                if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT && NewRole==SecurityObjects::ROOT) {
                    return UnAuthorized(RESTAPI::Errors::InsufficientAccessRights, ACCESS_DENIED);
                }
                if(Id==UserInfo_.userinfo.Id) {
                    return UnAuthorized(RESTAPI::Errors::InsufficientAccessRights, ACCESS_DENIED);
                }
                Existing.userRole = NewRole;
            }
        }

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
            bool ChangingMFA = NewUser.userTypeProprietaryInfo.mfa.enabled && !Existing.userTypeProprietaryInfo.mfa.enabled;

            Existing.userTypeProprietaryInfo.mfa.enabled = NewUser.userTypeProprietaryInfo.mfa.enabled;

            auto PropInfo = RawObject->get("userTypeProprietaryInfo");
            auto PInfo = PropInfo.extract<Poco::JSON::Object::Ptr>();

            if(PInfo->isArray("mobiles")) {
                Existing.userTypeProprietaryInfo.mobiles = NewUser.userTypeProprietaryInfo.mobiles;
            }

            if(ChangingMFA && !NewUser.userTypeProprietaryInfo.mobiles.empty() && !SMSSender()->IsNumberValid(NewUser.userTypeProprietaryInfo.mobiles[0].number,UserInfo_.userinfo.email)){
                return BadRequest(RESTAPI::Errors::NeedMobileNumber);
            }

            if(NewUser.userTypeProprietaryInfo.mfa.method=="sms" && Existing.userTypeProprietaryInfo.mobiles.empty()) {
                return BadRequest(RESTAPI::Errors::NeedMobileNumber);
            }

            if(!NewUser.userTypeProprietaryInfo.mfa.method.empty()) {
                if(NewUser.userTypeProprietaryInfo.mfa.method!="email" && NewUser.userTypeProprietaryInfo.mfa.method!="sms" ) {
                    return BadRequest("Unknown MFA method");
                }
                Existing.userTypeProprietaryInfo.mfa.method=NewUser.userTypeProprietaryInfo.mfa.method;
            }

            if(Existing.userTypeProprietaryInfo.mfa.enabled && Existing.userTypeProprietaryInfo.mfa.method.empty()) {
                return BadRequest("Illegal MFA method");
            }
        }

        if(StorageService()->UpdateUserInfo(UserInfo_.userinfo.email,Id,Existing)) {
            SecurityObjects::UserInfo   NewUserInfo;
            StorageService()->GetUserByEmail(UserInfo_.userinfo.email,NewUserInfo);
            Poco::JSON::Object  ModifiedObject;
            FilterCredentials(NewUserInfo);
            NewUserInfo.to_json(ModifiedObject);
            return ReturnObject(ModifiedObject);
        }
        BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }
}