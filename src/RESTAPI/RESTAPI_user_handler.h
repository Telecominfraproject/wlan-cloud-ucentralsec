//
// Created by stephane bourque on 2021-06-21.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_user_handler : public RESTAPIHandler {
    public:
        RESTAPI_user_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {Poco::Net::HTTPRequest::HTTP_POST,
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_PUT,
                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                          Server,
                                          Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/user/{id}"}; };
        void DoGet() final;
        void DoPost() final;
        void DoDelete() final;
        void DoPut() final;
    private:

    };
}
