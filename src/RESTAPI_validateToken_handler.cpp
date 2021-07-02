//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_validateToken_handler.h"
#include "Daemon.h"
#include "AuthService.h"
#include "Utils.h"

namespace uCentral {
    void RESTAPI_validateToken_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                      Poco::Net::HTTPServerResponse &Response) {

        if (!ContinueProcessing(Request, Response))
            return;

        if (!Daemon()->IsValidAPIKEY(Request))
            return;

        try {
            Poco::URI URI(Request.getURI());
            auto Parameters = URI.getQueryParameters();
            for(auto const &i:Parameters) {
                if (i.first == "token") {
                    //  can we find this token?
                    SecurityObjects::UserInfoAndPolicy SecObj;
                    if (AuthService()->IsValidToken(i.first, SecObj.WebToken, SecObj.UserInfo)) {
                        Poco::JSON::Object Obj;
                        SecObj.to_json(Obj);
                        ReturnObject(Request, Obj, Response);
                        return;
                    }
                }
            }
            NotFound(Request, Response);
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    };
}