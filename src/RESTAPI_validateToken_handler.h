//
// Created by stephane bourque on 2021-07-01.
//

#ifndef UCENTRALSEC_RESTAPI_VALIDATETOKEN_HANDLER_H
#define UCENTRALSEC_RESTAPI_VALIDATETOKEN_HANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_validateToken_handler : public RESTAPIHandler {
    public:
        RESTAPI_validateToken_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS}) {};

        void handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/validateToken"}; };
    };
}

#endif //UCENTRALSEC_RESTAPI_VALIDATETOKEN_HANDLER_H
