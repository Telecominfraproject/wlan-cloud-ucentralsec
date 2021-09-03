//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_validateToken_handler.h"
#include "Daemon.h"
#include "AuthService.h"
#include "Utils.h"

namespace OpenWifi {
    void RESTAPI_validateToken_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                      Poco::Net::HTTPServerResponse &Response) {

        if (!ContinueProcessing(Request, Response))
            return;
        std::cout << "Line: " << __LINE__ << std::endl;
        if (!IsAuthorized(Request, Response))
            return;
        std::cout << "Line: " << __LINE__ << std::endl;

        try {
            Poco::URI URI(Request.getURI());
            auto Parameters = URI.getQueryParameters();
            std::cout << "Line: " << __LINE__ << std::endl;
            for(auto const &i:Parameters) {
                std::cout << "Line: " << __LINE__ << std::endl;
                if (i.first == "token") {
                    //  can we find this token?
                    std::cout << "Line: " << __LINE__ << std::endl;
                    SecurityObjects::UserInfoAndPolicy SecObj;
                    std::cout << "Line: " << i.first << std::endl;
                    if (AuthService()->IsValidToken(i.second, SecObj.webtoken, SecObj.userinfo)) {
                        Poco::JSON::Object Obj;
                        SecObj.to_json(Obj);
                        ReturnObject(Request, Obj, Response);
                        std::cout << "Line: " << SecObj.webtoken.access_token_ << std::endl;
                        return;
                    }
                }
            }
            NotFound(Request, Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    };
}