//
// Created by stephane bourque on 2023-01-25.
//

#include "RESTAPI_systemSecret_handler.h"

namespace OpenWifi {

    void RESTAPI_systemSecret_handler::DoGet() {
        if(!Internal_ && UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }
    }

    void RESTAPI_systemSecret_handler::DoDelete() {
        if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

    }

    void RESTAPI_systemSecret_handler::DoPut() {
        if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

    }

} // OpenWifi