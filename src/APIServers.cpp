//
// Created by stephane bourque on 2021-10-23.
//

#include "framework/MicroService.h"

#include "RESTAPI/RESTAPI_oauth2Handler.h"
#include "RESTAPI/RESTAPI_user_handler.h"
#include "RESTAPI/RESTAPI_users_handler.h"
#include "RESTAPI/RESTAPI_action_links.h"
#include "RESTAPI/RESTAPI_systemEndpoints_handler.h"
#include "RESTAPI/RESTAPI_AssetServer.h"
#include "RESTAPI/RESTAPI_avatarHandler.h"
#include "RESTAPI/RESTAPI_email_handler.h"
#include "RESTAPI/RESTAPI_sms_handler.h"
#include "RESTAPI/RESTAPI_validateToken_handler.h"

namespace OpenWifi {

    Poco::Net::HTTPRequestHandler * RESTAPI_external_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServer & S) {
        return RESTAPI_Router<
            RESTAPI_oauth2Handler,
            RESTAPI_users_handler,
            RESTAPI_user_handler,
            RESTAPI_system_command,
            RESTAPI_AssetServer,
            RESTAPI_systemEndpoints_handler,
            RESTAPI_action_links,
            RESTAPI_avatarHandler,
            RESTAPI_email_handler,
            RESTAPI_sms_handler
        >(Path, Bindings, L, S);
    }

    Poco::Net::HTTPRequestHandler * RESTAPI_internal_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServer & S) {
        return RESTAPI_Router_I<
            RESTAPI_users_handler,
            RESTAPI_user_handler,
            RESTAPI_system_command,
            RESTAPI_action_links,
            RESTAPI_validateToken_handler,
            RESTAPI_sms_handler
        >(Path, Bindings, L, S);
    }
}