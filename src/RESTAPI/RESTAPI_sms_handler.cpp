//
// Created by stephane bourque on 2021-10-09.
//

#include "RESTAPI_sms_handler.h"
#include "SMSSender.h"
#include "framework/ow_constants.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    void OpenWifi::RESTAPI_sms_handler::DoPost() {
        const auto &Obj = ParsedBody_;

        if(!SMSSender()->Enabled()) {
            return BadRequest(RESTAPI::Errors::SMSMFANotEnabled);
        }

        std::string Arg;
        if(HasParameter("validateNumber",Arg) && Arg=="true" && Obj->has("to")) {
            auto Number = Obj->get("to").toString();
            if(SMSSender()->StartValidation(Number, UserInfo_.userinfo.email)) {
                return OK();
            }
            return BadRequest(RESTAPI::Errors::SMSCouldNotBeSentRetry);
        }

        std::string Code;
        if( HasParameter("completeValidation",Arg) &&
            Arg=="true" &&
            HasParameter("validationCode", Code) &&
            Obj->has("to")) {
            auto Number = Obj->get("to").toString();
            if(SMSSender()->CompleteValidation(Number, Code, UserInfo_.userinfo.email)) {
                return OK();
            }
            return BadRequest(RESTAPI::Errors::SMSCouldNotValidate);
        }

        if( UserInfo_.userinfo.userRole!=SecurityObjects::ROOT &&
            UserInfo_.userinfo.userRole!=SecurityObjects::PARTNER &&
            UserInfo_.userinfo.userRole!=SecurityObjects::ADMIN) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

        if (Obj->has("to") &&
            Obj->has("text")) {

            std::string PhoneNumber = Obj->get("to").toString();
            std::string Text = Obj->get("text").toString();
            if(SMSSender()->Send(PhoneNumber, Text))
                return OK();

            return InternalError(RESTAPI::Errors::SMSCouldNotBeSentRetry);
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }

}