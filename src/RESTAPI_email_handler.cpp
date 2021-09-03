//
// Created by stephane bourque on 2021-09-02.
//

#include "RESTAPI_email_handler.h"


#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "Daemon.h"
#include "SMTPMailerService.h"

namespace OpenWifi {
    void RESTAPI_email_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                               Poco::Net::HTTPServerResponse &Response) {

        if (!ContinueProcessing(Request, Response))
            return;

        std::cout << __LINE__ << std::endl;
        if (!IsAuthorized(Request, Response))
            return;

        std::cout << __LINE__ << std::endl;
        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            DoPost(Request, Response);
        else
            BadRequest(Request, Response, "Unsupported method.");
    }

    void RESTAPI_email_handler::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::cout << __LINE__ << std::endl;
            Poco::JSON::Parser parser;
            auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

            std::cout << __LINE__ << std::endl;
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
                    OK(Request, Response);
                    return;
                }
                std::cout << __LINE__ << std::endl;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        std::cout << __LINE__ << std::endl;
        BadRequest(Request, Response, "Unsupported or missing parameters.");
    }

}