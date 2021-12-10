//
// Created by stephane bourque on 2021-09-02.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_email_handler : public RESTAPIHandler {
    public:
        RESTAPI_email_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Server,
                                                  TransactionId,
                                                  Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/email"};}
        void DoGet() final {};
        void DoPost() final;
        void DoDelete() final {};
        void DoPut() final {};
    };
}
