//
// Created by stephane bourque on 2021-07-10.
//

#ifndef UCENTRALSEC_RESTAPI_ASSETSERVER_H
#define UCENTRALSEC_RESTAPI_ASSETSERVER_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
    class RESTAPI_AssetServer : public RESTAPIHandler {
    public:
        RESTAPI_AssetServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_POST,
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_PUT,
                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                          Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/wwwassets/{id}" ,
                                                                                         "/favicon.ico"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final {};

    private:

    };
}


#endif //UCENTRALSEC_RESTAPI_ASSETSERVER_H
