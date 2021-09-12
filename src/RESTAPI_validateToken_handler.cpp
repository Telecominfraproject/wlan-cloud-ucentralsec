//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_validateToken_handler.h"
#include "Daemon.h"
#include "AuthService.h"
#include "Utils.h"

namespace OpenWifi {
    void RESTAPI_validateToken_handler::DoGet() {
        try {
            Poco::URI URI(Request->getURI());
            auto Parameters = URI.getQueryParameters();
            for(auto const &i:Parameters) {
                if (i.first == "token") {
                    //  can we find this token?
                    SecurityObjects::UserInfoAndPolicy SecObj;
                    if (AuthService()->IsValidToken(i.second, SecObj.webtoken, SecObj.userinfo)) {
                        Poco::JSON::Object Obj;
                        SecObj.to_json(Obj);
                        ReturnObject(Obj);
                        return;
                    }
                }
            }
            NotFound();
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

}