//
// Created by stephane bourque on 2021-07-01.
//

#ifndef UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H
#define UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H

#include "../framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_systemEndpoints_handler : public RESTAPIHandler {
    public:
        RESTAPI_systemEndpoints_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                          Server,
                                                          Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/systemEndpoints"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final {};
    };
}

#endif //UCENTRALSEC_RESTAPI_SYSTEMENDPOINTS_HANDLER_H
