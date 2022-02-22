//
// Created by stephane bourque on 2022-02-20.
//

#include "RESTAPI_signup_handler.h"
#include "StorageService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

#define __DBG__ std::cout << __LINE__ << std::endl;
namespace OpenWifi {

    void RESTAPI_signup_handler::DoPost() {

        auto UserName = GetParameter("email","");
        auto signupUUID = GetParameter("signupUUID","");
        __DBG__
        if(UserName.empty() || signupUUID.empty()) {
            __DBG__
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!Utils::ValidEMailAddress(UserName)) {
            __DBG__
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        // Do we already exist? Can only signup once...
        SecurityObjects::UserInfo   Existing;
        if(StorageService()->SubDB().GetUserByEmail(UserName,Existing)) {
            __DBG__

            if(Existing.signingUp.empty()) {
                __DBG__
                return BadRequest(1, "Subscriber already signed up.");
            }

            if(Existing.waitingForEmailCheck) {
                __DBG__
                return BadRequest(2, "Waiting for email check completion.");
            }

            __DBG__
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

        __DBG__
        StorageService()->SubDB().CreateRecord(NewSub);
        __DBG__

        Logger_.information(Poco::format("SIGNUP-PASSWORD(%s): Request for %s", Request->clientAddress().toString(), UserName));
        SecurityObjects::ActionLink NewLink;

        NewLink.action = OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP;
        NewLink.id = MicroService::CreateUUID();
        NewLink.userId = NewSub.id;
        NewLink.created = std::time(nullptr);
        NewLink.expires = NewLink.created + (1*60*60);  // 1 hour
        NewLink.userAction = false;
        StorageService()->ActionLinksDB().CreateAction(NewLink);
        __DBG__

        return OK();
    }

    void RESTAPI_signup_handler::DoPut() {
        // TODO
    }

}