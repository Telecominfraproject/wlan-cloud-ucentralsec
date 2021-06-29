//
// Created by stephane bourque on 2021-06-28.
//

#ifndef UCENTRALSEC_RESTAPI_SYSTEMSERVICES_HANDLER_H
#define UCENTRALSEC_RESTAPI_SYSTEMSERVICES_HANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_systemServices_handler : public RESTAPIHandler {
    public:
        RESTAPI_systemServices_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                 {Poco::Net::HTTPRequest::HTTP_POST,
                                  Poco::Net::HTTPRequest::HTTP_GET,
                                  Poco::Net::HTTPRequest::HTTP_PUT,
                                  Poco::Net::HTTPRequest::HTTP_DELETE,
                                  Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
        void handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) override;
    private:

    };
}

#endif //UCENTRALSEC_RESTAPI_SYSTEMSERVICES_HANDLER_H
