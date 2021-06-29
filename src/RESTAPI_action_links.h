//
// Created by stephane bourque on 2021-06-22.
//

#ifndef UCENTRALSEC_RESTAPI_ACTION_LINKS_H
#define UCENTRALSEC_RESTAPI_ACTION_LINKS_H


#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_action_links : public RESTAPIHandler {
    public:
        RESTAPI_action_links(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                                            Poco::Net::HTTPRequest::HTTP_GET,
                                                            Poco::Net::HTTPRequest::HTTP_POST,
                                                            Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
        void handleRequest(Poco::Net::HTTPServerRequest &Request,
                           Poco::Net::HTTPServerResponse &Response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/actions"}; };
    };
}

#endif //UCENTRALSEC_RESTAPI_ACTION_LINKS_H
