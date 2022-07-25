//
// Created by stephane bourque on 2021-06-22.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_action_links : public RESTAPIHandler {
    public:
        RESTAPI_action_links(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
             std::vector<std::string>{
                                        Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_POST,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                        Server,
                                        TransactionId,
                                        Internal,
                                        false,
                                        true, RateLimit{.Interval=1000,.MaxCalls=10}) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/actionLink"}; };
        void RequestResetPassword(SecurityObjects::ActionLink &Link);
        void CompleteResetPassword();
        void CompleteSubVerification();
        void DoEmailVerification(SecurityObjects::ActionLink &Link);
        void DoReturnA404();
        void DoNewSubVerification(SecurityObjects::ActionLink &Link);
        void CompleteEmailInvitation();

        void DoGet() final;
        void DoPost() final;
        void DoDelete() final {};
        void DoPut() final {};
    };
}
