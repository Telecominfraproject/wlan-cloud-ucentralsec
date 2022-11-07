//
// Created by stephane bourque on 2021-10-23.
//

#include "RESTAPI/RESTAPI_oauth2_handler.h"
#include "RESTAPI/RESTAPI_user_handler.h"
#include "RESTAPI/RESTAPI_users_handler.h"
#include "RESTAPI/RESTAPI_action_links.h"
#include "RESTAPI/RESTAPI_system_endpoints_handler.h"
#include "RESTAPI/RESTAPI_asset_server.h"
#include "RESTAPI/RESTAPI_avatar_handler.h"
#include "RESTAPI/RESTAPI_subavatar_handler.h"
#include "RESTAPI/RESTAPI_email_handler.h"
#include "RESTAPI/RESTAPI_sms_handler.h"
#include "RESTAPI/RESTAPI_validate_token_handler.h"
#include "RESTAPI/RESTAPI_preferences.h"
#include "RESTAPI/RESTAPI_subpreferences.h"
#include "RESTAPI/RESTAPI_suboauth2_handler.h"
#include "RESTAPI/RESTAPI_subuser_handler.h"
#include "RESTAPI/RESTAPI_subusers_handler.h"
#include "RESTAPI/RESTAPI_validate_sub_token_handler.h"
#include "RESTAPI/RESTAPI_submfa_handler.h"
#include "RESTAPI/RESTAPI_totp_handler.h"
#include "RESTAPI/RESTAPI_subtotp_handler.h"
#include "RESTAPI/RESTAPI_signup_handler.h"
#include "RESTAPI/RESTAPI_apiKey_handler.h"
#include "RESTAPI/RESTAPI_validate_apikey.h"

#include "framework/RESTAPI_SystemCommand.h"
#include "framework/RESTAPI_WebSocketServer.h"

namespace OpenWifi {

    Poco::Net::HTTPRequestHandler * RESTAPI_ExtRouter(const std::string &Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServerAccounting & S,
                                                            uint64_t TransactionId) {
        return RESTAPI_Router<
            RESTAPI_oauth2_handler,
            RESTAPI_user_handler,
            RESTAPI_users_handler,
            RESTAPI_system_command,
            RESTAPI_asset_server,
            RESTAPI_system_endpoints_handler,
            RESTAPI_action_links,
            RESTAPI_avatar_handler,
            RESTAPI_subavatar_handler,
            RESTAPI_email_handler,
            RESTAPI_sms_handler,
            RESTAPI_preferences,
            RESTAPI_subpreferences,
            RESTAPI_suboauth2_handler,
            RESTAPI_subuser_handler,
            RESTAPI_subusers_handler,
            RESTAPI_submfa_handler,
            RESTAPI_totp_handler,
            RESTAPI_subtotp_handler,
            RESTAPI_signup_handler,
            RESTAPI_validate_sub_token_handler,
            RESTAPI_validate_token_handler,
            RESTAPI_validate_apikey,
            RESTAPI_webSocketServer,
            RESTAPI_apiKey_handler
        >(Path, Bindings, L, S,TransactionId);
    }

    Poco::Net::HTTPRequestHandler * RESTAPI_IntRouter(const std::string &Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServerAccounting & S, uint64_t TransactionId) {

        return RESTAPI_Router_I<
            RESTAPI_oauth2_handler,
            RESTAPI_user_handler,
            RESTAPI_users_handler,
            RESTAPI_system_command,
            RESTAPI_asset_server,
            RESTAPI_system_endpoints_handler,
            RESTAPI_action_links,
            RESTAPI_avatar_handler,
            RESTAPI_subavatar_handler,
            RESTAPI_email_handler,
            RESTAPI_sms_handler,
            RESTAPI_preferences,
            RESTAPI_subpreferences,
            RESTAPI_suboauth2_handler,
            RESTAPI_subuser_handler,
            RESTAPI_subusers_handler,
            RESTAPI_submfa_handler,
            RESTAPI_totp_handler,
            RESTAPI_subtotp_handler,
            RESTAPI_validate_sub_token_handler,
            RESTAPI_validate_token_handler,
            RESTAPI_signup_handler
        >(Path, Bindings, L, S, TransactionId);
    }
}