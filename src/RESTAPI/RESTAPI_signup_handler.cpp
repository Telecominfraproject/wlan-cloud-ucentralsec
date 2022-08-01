//
// Created by stephane bourque on 2022-02-20.
//

#include "RESTAPI_signup_handler.h"
#include "StorageService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

#define __DBG__ std::cout << __LINE__ << std::endl;
namespace OpenWifi {

    void RESTAPI_signup_handler::DoPost() {
        auto UserName = GetParameter("email");
        auto signupUUID = GetParameter("signupUUID");
        auto owner = GetParameter("owner");
        auto operatorName = GetParameter("operatorName");
        if(UserName.empty() || signupUUID.empty() || owner.empty() || operatorName.empty()) {
            Logger().error("Signup requires: email, signupUUID, operatorName, and owner.");
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!Utils::ValidEMailAddress(UserName)) {
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        // Do we already exist? Can only signup once...
        SecurityObjects::UserInfo   Existing;
        if(StorageService()->SubDB().GetUserByEmail(UserName,Existing)) {
            if(Existing.signingUp.empty()) {
                return BadRequest(RESTAPI::Errors::SignupAlreadySigned);
            }

            if(Existing.waitingForEmailCheck) {
                return BadRequest(RESTAPI::Errors::SignupEmailCheck);
            }

            return BadRequest(RESTAPI::Errors::SignupWaitingForDevice);
        }

        SecurityObjects::UserInfo   NewSub;
        NewSub.signingUp = operatorName + ":" + signupUUID;
        NewSub.waitingForEmailCheck = true;
        NewSub.name = UserName;
        NewSub.modified = OpenWifi::Now();
        NewSub.creationDate = OpenWifi::Now();
        NewSub.id = MicroService::instance().CreateUUID();
        NewSub.email = UserName;
        NewSub.userRole = SecurityObjects::SUBSCRIBER;
        NewSub.changePassword = true;
        NewSub.owner = owner;

        StorageService()->SubDB().CreateRecord(NewSub);

        Logger_.information(fmt::format("SIGNUP-PASSWORD({}): Request for {}", Request->clientAddress().toString(), UserName));
        SecurityObjects::ActionLink NewLink;

        NewLink.action = OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP;
        NewLink.id = MicroService::CreateUUID();
        NewLink.userId = NewSub.id;
        NewLink.created = OpenWifi::Now();
        NewLink.expires = NewLink.created + (1*60*60);  // 1 hour
        NewLink.userAction = false;
        StorageService()->ActionLinksDB().CreateAction(NewLink);

        Poco::JSON::Object  Answer;
        NewSub.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_signup_handler::DoPut() {
        // TODO
    }

}