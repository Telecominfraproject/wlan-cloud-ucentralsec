//
// Created by stephane bourque on 2021-07-10.
//

#ifndef UCENTRALSEC_RESTAPI_ASSETSERVER_H
#define UCENTRALSEC_RESTAPI_ASSETSERVER_H

#include "RESTAPI_handler.h"

namespace uCentral {
    class RESTAPI_AssetServer : public RESTAPIHandler {
    public:
        RESTAPI_AssetServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_POST,
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_PUT,
                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
        void handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) override;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/wwwassets/{id}"}; };
        void DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
    private:

    };
}


#endif //UCENTRALSEC_RESTAPI_ASSETSERVER_H
