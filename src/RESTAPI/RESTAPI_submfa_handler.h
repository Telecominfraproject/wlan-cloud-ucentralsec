//
// Created by stephane bourque on 2021-12-01.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_submfa_handler : public RESTAPIHandler {
    public:
        RESTAPI_submfa_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_PUT,
                                                  Poco::Net::HTTPRequest::HTTP_GET,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Server,
                                                  Internal, false, false , RateLimit{.Interval=1000,.MaxCalls=10},
                                                  true) {}
                                                  static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/submfa"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final ;
    };
}
