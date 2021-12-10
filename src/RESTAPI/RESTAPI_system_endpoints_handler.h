//
// Created by stephane bourque on 2021-07-01.
//

#pragma once

#include "../framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_system_endpoints_handler : public RESTAPIHandler {
    public:
        RESTAPI_system_endpoints_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                          Server,
                                                          TransactionId,
                                                          Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/systemEndpoints"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final {};
    };
}
