//
// Created by stephane bourque on 2021-10-23.
//

#include "framework/MicroService.h"

#include "RESTAPI/RESTAPI_oauth2_handler.h"
#include "RESTAPI/RESTAPI_user_handler.h"
#include "RESTAPI/RESTAPI_users_handler.h"
#include "RESTAPI/RESTAPI_action_links.h"
#include "RESTAPI/RESTAPI_system_endpoints_handler.h"
#include "RESTAPI/RESTAPI_asset_server.h"
#include "RESTAPI/RESTAPI_avatar_handler.h"
#include "RESTAPI/RESTAPI_email_handler.h"
#include "RESTAPI/RESTAPI_sms_handler.h"
#include "RESTAPI/RESTAPI_validate_token_handler.h"
#include "RESTAPI/RESTAPI_preferences.h"
#include "RESTAPI/RESTAPI_suboauth2_handler.h"
#include "RESTAPI/RESTAPI_subuser_handler.h"
#include "RESTAPI/RESTAPI_subusers_handler.h"
#include "RESTAPI/RESTAPI_validate_sub_token_handler.h"
#include "RESTAPI/RESTAPI_submfa_handler.h"

namespace OpenWifi {

    Poco::Net::HTTPRequestHandler * RESTAPI_external_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServer & S) {
        return RESTAPI_Router<
            RESTAPI_oauth2_handler,
            RESTAPI_users_handler,
            RESTAPI_user_handler,
            RESTAPI_system_command,
            RESTAPI_asset_server,
            RESTAPI_system_endpoints_handler,
            RESTAPI_action_links,
            RESTAPI_avatar_handler,
            RESTAPI_email_handler,
            RESTAPI_sms_handler,
            RESTAPI_preferences,
            RESTAPI_suboauth2_handler,
            RESTAPI_subuser_handler,
            RESTAPI_subusers_handler
        >(Path, Bindings, L, S);
    }

    Poco::Net::HTTPRequestHandler * RESTAPI_internal_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServer & S) {
        return RESTAPI_Router_I<
            RESTAPI_users_handler,
            RESTAPI_user_handler,
            RESTAPI_system_command,
            RESTAPI_action_links,
            RESTAPI_validate_token_handler,
            RESTAPI_validate_sub_token_handler,
            RESTAPI_sms_handler,
            RESTAPI_suboauth2_handler,
            RESTAPI_subuser_handler,
            RESTAPI_suboauth2_handler,
            RESTAPI_subusers_handler,
            RESTAPI_submfa_handler
        >(Path, Bindings, L, S);
    }
}