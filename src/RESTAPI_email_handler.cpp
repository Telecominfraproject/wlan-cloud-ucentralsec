//
// Created by stephane bourque on 2021-09-02.
//

#include "RESTAPI_email_handler.h"


#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "Daemon.h"
#include "SMTPMailerService.h"

namespace OpenWifi {
    void RESTAPI_email_handler::DoPost() {
        try {
            auto Obj = ParseStream();
            if (Obj->has("subject") &&
            Obj->has("from") &&
            Obj->has("text") &&
            Obj->has("recipients")) {
                auto   Recipients = Obj->getArray("recipients");
                MessageAttributes Attrs;
                Attrs[RECIPIENT_EMAIL] = Recipients->get(0).toString();
                Attrs[SUBJECT] = Obj->get("subject").toString();
                Attrs[TEXT] = Obj->get("text").toString();
                if(SMTPMailerService()->SendMessage(Recipients->get(0).toString(), "password_reset.txt", Attrs)) {
                    OK();
                    return;
                }
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Unsupported or missing parameters.");
    }
}