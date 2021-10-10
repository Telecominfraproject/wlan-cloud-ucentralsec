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
        if (Obj->has("to") &&
            Obj->has("text")) {

            std::string PhoneNumber = Obj->get("to").toString();
            std::string Text = Obj->get("text").toString();
            if(SMSSender()->Send(PhoneNumber, Text)==0)
                OK();
            else
                InternalError("SMS Message could not be sent.");
            return;
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }

}