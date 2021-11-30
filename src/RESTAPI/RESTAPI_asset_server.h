//
// Created by stephane bourque on 2021-07-10.
//

#pragma once

#include "../framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_asset_server : public RESTAPIHandler {
    public:
        RESTAPI_asset_server(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_POST,
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_PUT,
                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                          Server,
                                          Internal, false) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/wwwassets/{id}" ,
                                                                                         "/favicon.ico"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final {};

    private:

    };
}

