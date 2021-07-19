//
// Created by stephane bourque on 2021-07-01.
//

#ifndef UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H
#define UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H

#include "RESTAPI_handler.h"
namespace uCentral {
    class RESTAPI_systemEndpoints_handler : public RESTAPIHandler {
    public:
        RESTAPI_systemEndpoints_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                          Internal) {}
        void handleRequest(Poco::Net::HTTPServerRequest &request,
                           Poco::Net::HTTPServerResponse &response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/systemEndpoints"}; };
    };
}

#endif //UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H
