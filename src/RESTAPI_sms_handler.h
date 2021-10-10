//
// Created by stephane bourque on 2021-10-09.
//

#ifndef OWSEC_RESTAPI_SMS_HANDLER_H
#define OWSEC_RESTAPI_SMS_HANDLER_H


#include "RESTAPI_handler.h"

namespace OpenWifi {
    class RESTAPI_sms_handler : public RESTAPIHandler {
    public:
        RESTAPI_sms_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Server,
                                                  Internal) {}
                                                  static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/sms"};}
        void DoGet() final {};
        void DoPost() final;
        void DoDelete() final {};
        void DoPut() final {};
    };
}

#endif //OWSEC_RESTAPI_SMS_HANDLER_H
