//
// Created by stephane bourque on 2021-06-22.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_action_links : public RESTAPIHandler {
    public:
        RESTAPI_action_links(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
                : RESTAPIHandler(bindings, L,
             std::vector<std::string>{
                                        Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_POST,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                        Server,
                                        Internal,
                                        false,
                                        true, RateLimit{.Interval=1000,.MaxCalls=10}) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/actionLink"}; };
        void RequestResetPassword(SecurityObjects::ActionLink &Link);
        void CompleteResetPassword();
        void DoEmailVerification(SecurityObjects::ActionLink &Link);
        void DoReturnA404();

        void DoGet() final;
        void DoPost() final;
        void DoDelete() final {};
        void DoPut() final {};
    };
}
