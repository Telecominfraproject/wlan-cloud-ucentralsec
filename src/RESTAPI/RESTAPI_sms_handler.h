//
// Created by stephane bourque on 2021-10-09.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_sms_handler : public RESTAPIHandler {
    public:
        RESTAPI_sms_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Server,
                                                  TransactionId,
                                                  Internal) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/sms"};}
        void DoGet() final {};
        void DoPost() final;
        void DoDelete() final {};
        void DoPut() final {};
    };
}
