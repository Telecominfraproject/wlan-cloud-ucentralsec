//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_users_handler.h"
#include "StorageService.h"
#include "RESTAPI_protocol.h"
#include "Utils.h"

namespace OpenWifi {
    void RESTAPI_users_handler::handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else
            BadRequest(Request, Response);
    }

    void RESTAPI_users_handler::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::vector<SecurityObjects::UserInfo> Users;
            InitQueryBlock();
            bool IdOnly = (GetParameter("idOnly","false")=="true");

            if(QB_.Select.empty()) {
                if (Storage()->GetUsers(QB_.Offset, QB_.Limit, Users)) {
                    Poco::JSON::Array ArrayObj;
                    for (const auto &i : Users) {
                        Poco::JSON::Object Obj;
                        if (IdOnly) {
                            ArrayObj.add(i.Id);
                        } else {
                            i.to_json(Obj);
                            ArrayObj.add(Obj);
                        }
                    }
                    Poco::JSON::Object RetObj;
                    RetObj.set(RESTAPI::Protocol::USERS, ArrayObj);
                    ReturnObject(Request, RetObj, Response);
                    return;
                }
            } else {
                Types::StringVec IDs = Utils::Split(QB_.Select);
                Poco::JSON::Array ArrayObj;
                for(auto &i:IDs) {
                    SecurityObjects::UserInfo   UInfo;
                    if(Storage()->GetUserById(i,UInfo)) {
                        Poco::JSON::Object Obj;
                        if (IdOnly) {
                            ArrayObj.add(UInfo.Id);
                        } else {
                            UInfo.to_json(Obj);
                            ArrayObj.add(Obj);
                        }
                    }
                }
                Poco::JSON::Object RetObj;
                RetObj.set(RESTAPI::Protocol::USERS, ArrayObj);
                ReturnObject(Request, RetObj, Response);
                return;
            }
        } catch ( const Poco::Exception &E ) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}