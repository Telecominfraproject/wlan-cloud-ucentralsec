//
// Created by stephane bourque on 2021-09-02.
//

#include "RESTAPI_email_handler.h"


#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "Daemon.h"
#include "SMTPMailerService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi {
    void RESTAPI_email_handler::DoPost() {
        auto Obj = ParseStream();
        if (Obj->has("subject") &&
            Obj->has("from") &&
            Obj->has("text") &&
            Obj->has("recipients") &&
            Obj->isArray("recipients")) {

            std::cout << "subject: " << Obj->get("subject").toString() << std::endl;
            std::cout << "from: " << Obj->get("from").toString() << std::endl;
            std::cout << "text: " << Obj->get("text").toString() << std::endl;

            Poco::JSON::Array::Ptr Recipients = Obj->getArray("recipients");
            auto Recipient = Recipients->get(0).toString();
            std::cout << "Size: " << Recipients->size() << std::endl;
            std::cout << "R: " << Recipient << std::endl;
            MessageAttributes Attrs;
            std::cout << "Mailing to:" << Recipient << std::endl;
            Attrs[RECIPIENT_EMAIL] = Recipient;
            Attrs[SUBJECT] = Obj->get("subject").toString();
            Attrs[TEXT] = Obj->get("text").toString();
            if(SMTPMailerService()->SendMessage(Recipient, "password_reset.txt", Attrs)) {
                return OK();
            }
            return ReturnStatus(Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }
}