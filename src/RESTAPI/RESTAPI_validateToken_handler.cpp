//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_validateToken_handler.h"
#include "AuthService.h"

namespace OpenWifi {
    void RESTAPI_validateToken_handler::DoGet() {
        Poco::URI URI(Request->getURI());
        auto Parameters = URI.getQueryParameters();
        for(auto const &i:Parameters) {
            if (i.first == "token") {
                //  can we find this token?
                SecurityObjects::UserInfoAndPolicy SecObj;
                if (AuthService()->IsValidToken(i.second, SecObj.webtoken, SecObj.userinfo)) {
                    Poco::JSON::Object Obj;
                    SecObj.to_json(Obj);
                    return ReturnObject(Obj);
                }
            }
        }
        return NotFound();
    }
}