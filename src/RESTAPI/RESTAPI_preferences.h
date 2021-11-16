//
// Created by stephane bourque on 2021-11-16.
//

#ifndef OWSEC_RESTAPI_PREFERENCES_H
#define OWSEC_RESTAPI_PREFERENCES_H

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_preferences : public RESTAPIHandler {
    public:
        RESTAPI_preferences(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_PUT,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal) {}
            static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/preferences"}; };
        void DoGet() final;
        void DoPut() final;
        void DoPost() final {};
        void DoDelete() final {};
    };
}

#endif //OWSEC_RESTAPI_PREFERENCES_H
