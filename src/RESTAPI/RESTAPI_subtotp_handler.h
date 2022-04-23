//
// Created by stephane bourque on 2022-01-31.
//

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_subtotp_handler : public RESTAPIHandler {
    public:
        RESTAPI_subtotp_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {
                                                 Poco::Net::HTTPRequest::HTTP_GET,
                                                 Poco::Net::HTTPRequest::HTTP_PUT,
                                                 Poco::Net::HTTPRequest::HTTP_OPTIONS
                                         },
                                 Server,
                                 TransactionId,
                                 Internal) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/subtotp"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final;
    private:

    };
}
