//
// Created by stephane bourque on 2021-06-21.
//

#ifndef UCENTRALSEC_RESTAPI_USERS_HANDLER_H
#define UCENTRALSEC_RESTAPI_USERS_HANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_users_handler : public RESTAPIHandler {
    public:
        RESTAPI_users_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                 {Poco::Net::HTTPRequest::HTTP_GET,
                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                  Internal) {}
        void handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/users"}; };
        void DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
    };
};


#endif //UCENTRALSEC_RESTAPI_USERS_HANDLER_H
