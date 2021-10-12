//
// Created by stephane bourque on 2021-10-09.
//

#include "RESTAPI_sms_handler.h"
#include "SMSSender.h"
#include "Utils.h"
#include "RESTAPI_errors.h"

namespace OpenWifi {

    void OpenWifi::RESTAPI_sms_handler::DoPost() {
        auto Obj = ParseStream();

        std::string Arg;
        if(HasParameter("validateNumber",Arg) && Arg=="true" && Obj->has("to")) {
            auto Number = Obj->get("to").toString();
            if(SMSSender()->StartValidation(Number, UserInfo_.userinfo.email)) {
                return OK();
            }
            return BadRequest("SMS could not be sent to validate device, try later or change the phone number.");
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
            return BadRequest("Code and number could not be validated");
        }

        if (Obj->has("to") &&
            Obj->has("text")) {

            std::string PhoneNumber = Obj->get("to").toString();
            std::string Text = Obj->get("text").toString();
            if(SMSSender()->Send(PhoneNumber, Text))
                return OK();

            return InternalError("SMS Message could not be sent.");
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }

}