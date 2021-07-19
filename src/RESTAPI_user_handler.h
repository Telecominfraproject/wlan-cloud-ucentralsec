//
// Created by stephane bourque on 2021-06-21.
//

#ifndef UCENTRALSEC_RESTAPI_USER_HANDLER_H
#define UCENTRALSEC_RESTAPI_USER_HANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_user_handler : public RESTAPIHandler {
    public:
        RESTAPI_user_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_POST,
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_PUT,
                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                          Internal) {}
        void handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/user/{id}"}; };
        void DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
        void DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
        void DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
        void DoPut(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
    private:

    };
}


#endif //UCENTRALSEC_RESTAPI_USER_HANDLER_H
