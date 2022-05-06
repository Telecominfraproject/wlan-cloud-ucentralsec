//
// Created by stephane bourque on 2021-09-02.
//

#include "RESTAPI_email_handler.h"


#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "SMTPMailerService.h"
#include "framework/ow_constants.h"
#include "framework/MicroService.h"

namespace OpenWifi {
    void RESTAPI_email_handler::DoPost() {
        const auto & Obj = ParsedBody_;
        if (Obj->has("subject") &&
            Obj->has("from") &&
            Obj->has("text") &&
            Obj->has("recipients") &&
            Obj->isArray("recipients")) {

            Poco::JSON::Array::Ptr Recipients = Obj->getArray("recipients");
            auto Recipient = Recipients->get(0).toString();
            MessageAttributes Attrs;
            Attrs[RECIPIENT_EMAIL] = Recipient;
            Attrs[SUBJECT] = Obj->get("subject").toString();
            Attrs[TEXT] = Obj->get("text").toString();
            Attrs[SENDER] = Obj->get("from").toString();
            if(SMTPMailerService()->SendMessage(Recipient, "password_reset.txt", Attrs)) {
                return OK();
            }
            return ReturnStatus(Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }
}