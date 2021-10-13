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
            Obj->has("recipients")) {
            auto Recipients = Obj->getArray("recipients");
            std::string Recipient;
            for(const auto &i:*Recipients) {
                Recipient = i.toString();
                std::cout << "R: " << Recipient << std::endl;
            }
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