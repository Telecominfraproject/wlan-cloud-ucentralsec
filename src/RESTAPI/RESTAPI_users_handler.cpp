//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_users_handler.h"
#include "StorageService.h"
#include "framework/RESTAPI_protocol.h"
#include "framework/MicroService.h"

namespace OpenWifi {
    void RESTAPI_users_handler::DoGet() {
        std::vector<SecurityObjects::UserInfo> Users;
        bool IdOnly = (GetParameter("idOnly","false")=="true");

        if(QB_.Select.empty()) {
            Poco::JSON::Array ArrayObj;
            Poco::JSON::Object Answer;
            if (StorageService()->GetUsers(QB_.Offset, QB_.Limit, Users)) {
                for (auto &i : Users) {
                    Poco::JSON::Object Obj;
                    if (IdOnly) {
                        ArrayObj.add(i.Id);
                    } else {
                        i.currentPassword.clear();
                        i.lastPasswords.clear();
                        i.oauthType.clear();
                        i.to_json(Obj);
                        ArrayObj.add(Obj);
                    }
                }
                Answer.set(RESTAPI::Protocol::USERS, ArrayObj);
            }
            return ReturnObject(Answer);
        } else {
            Types::StringVec IDs = Utils::Split(QB_.Select);
            Poco::JSON::Array ArrayObj;
            for(auto &i:IDs) {
                SecurityObjects::UserInfo   UInfo;
                if(StorageService()->GetUserById(i,UInfo)) {
                    Poco::JSON::Object Obj;
                    if (IdOnly) {
                        ArrayObj.add(UInfo.Id);
                    } else {
                        UInfo.currentPassword.clear();
                        UInfo.lastPasswords.clear();
                        UInfo.oauthType.clear();
                        UInfo.to_json(Obj);
                        ArrayObj.add(Obj);
                    }
                }
            }
            Poco::JSON::Object RetObj;
            RetObj.set(RESTAPI::Protocol::USERS, ArrayObj);
            return ReturnObject(RetObj);
        }
    }
}