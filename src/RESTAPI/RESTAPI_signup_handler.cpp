//
// Created by stephane bourque on 2022-02-20.
//

#include "RESTAPI_signup_handler.h"
#include "StorageService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    void RESTAPI_signup_handler::DoPost() {

        auto UserName = GetParameter("email","");
        auto signupUUID = GetParameter("signupUUID","");

        if(UserName.empty() || signupUUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!Utils::ValidEMailAddress(UserName)) {
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        // Do we already exist? Can only signup once...
        SecurityObjects::UserInfo   Existing;
        if(StorageService()->SubDB().GetUserByEmail(UserName,Existing)) {

            if(Existing.signingUp.empty()) {
                return BadRequest(1, "Subscriber already signed up.");
            }

            if(Existing.waitingForEmailCheck) {
                return BadRequest(2, "Waiting for email check completion.");
            }

            return BadRequest(3, "Waiting for device:" + Existing.signingUp);
        }

        SecurityObjects::UserInfo   NewSub;
        NewSub.signingUp = signupUUID;
        NewSub.waitingForEmailCheck = true;
        NewSub.modified = std::time(nullptr);
        NewSub.creationDate = std::time(nullptr);
        NewSub.id = MicroService::instance().CreateUUID();
        NewSub.email = UserName;
        NewSub.userRole = SecurityObjects::SUBSCRIBER;
        NewSub.changePassword = true;

        StorageService()->SubDB().CreateRecord(NewSub);

        Logger_.information(Poco::format("SIGNUP-PASSWORD(%s): Request for %s", Request->clientAddress().toString(), UserName));
        SecurityObjects::ActionLink NewLink;

        NewLink.action = OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP;
        NewLink.id = MicroService::CreateUUID();
        NewLink.userId = NewSub.id;
        NewLink.created = std::time(nullptr);
        NewLink.expires = NewLink.created + (1*60*60);  // 1 hour
        NewLink.userAction = false;
        StorageService()->ActionLinksDB().CreateAction(NewLink);

        return OK();
    }

    void RESTAPI_signup_handler::DoPut() {
        // TODO
    }

}