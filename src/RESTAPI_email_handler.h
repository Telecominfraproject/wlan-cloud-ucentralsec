//
// Created by stephane bourque on 2021-09-02.
//

#ifndef OWSEC_RESTAPI_EMAIL_HANDLER_H
#define OWSEC_RESTAPI_EMAIL_HANDLER_H


#include "RESTAPI_handler.h"

namespace OpenWifi {
    class RESTAPI_email_handler : public RESTAPIHandler {
    public:
        RESTAPI_email_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Internal) {}
                                                  void handleRequest(Poco::Net::HTTPServerRequest &request,
                                                                     Poco::Net::HTTPServerResponse &response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/email"};}
        void DoPost(Poco::Net::HTTPServerRequest &Request,
                    Poco::Net::HTTPServerResponse &Response);
    };
}

#endif //OWSEC_RESTAPI_EMAIL_HANDLER_H
