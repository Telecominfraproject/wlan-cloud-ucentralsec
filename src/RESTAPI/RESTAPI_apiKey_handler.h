//
// Created by stephane bourque on 2022-11-04.
//

#pragma once

#include "framework/RESTAPI_Handler.h"
#include "StorageService.h"
namespace OpenWifi {
    class RESTAPI_apiKey_handler : public RESTAPIHandler {
    public:
        RESTAPI_apiKey_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_GET,
                                         Poco::Net::HTTPRequest::HTTP_PUT,
                                         Poco::Net::HTTPRequest::HTTP_POST,
                                         Poco::Net::HTTPRequest::HTTP_DELETE,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                 Server,
                                 TransactionId,
                                 Internal) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/apiKey/{uuid}"}; };
    private:
        ApiKeyDB     &DB_=StorageService()->ApiKeyDB();

        void DoGet() final;
        void DoPut() final;
        void DoPost() final;
        void DoDelete() final;

    };
}

