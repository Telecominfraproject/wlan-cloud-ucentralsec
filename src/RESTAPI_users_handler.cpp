//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_users_handler.h"
#include "StorageService.h"
#include "RESTAPI_protocol.h"
#include "Utils.h"

namespace OpenWifi {
    void RESTAPI_users_handler::DoGet() {
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
                    ReturnObject(RetObj);
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
                ReturnObject(RetObj);
                return;
            }
        } catch ( const Poco::Exception &E ) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }
}