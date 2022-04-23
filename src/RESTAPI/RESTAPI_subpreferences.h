//
// Created by stephane bourque on 2021-11-16.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_subpreferences : public RESTAPIHandler {
    public:
        RESTAPI_subpreferences(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_PUT,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            TransactionId,
            Internal) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/subpreferences"}; };
        void DoGet() final;
        void DoPut() final;
        void DoPost() final {};
        void DoDelete() final {};
    };
}
