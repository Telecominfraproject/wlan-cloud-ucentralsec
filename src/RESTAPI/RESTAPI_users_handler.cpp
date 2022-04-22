//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_users_handler.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {
    void RESTAPI_users_handler::DoGet() {
        std::vector<SecurityObjects::UserInfo> Users;
        bool IdOnly = (GetParameter("idOnly","false")=="true");
        auto nameSearch = GetParameter("nameSearch");
        auto emailSearch = GetParameter("emailSearch");

        std::string baseQuery;
        if(!nameSearch.empty() || !emailSearch.empty()) {
            if(!nameSearch.empty())
                baseQuery = fmt::format(" Lower(name) like('%{}%' ", Poco::toLower(nameSearch) );
            if(!emailSearch.empty())
                baseQuery += baseQuery.empty() ? fmt::format(" Lower(email) like('%{}%' ", Poco::toLower(emailSearch))
                                               : fmt::format(" and Lower(email) like('%{}%' ", Poco::toLower(emailSearch));
        }

        if(QB_.Select.empty()) {
            Poco::JSON::Array ArrayObj;
            Poco::JSON::Object Answer;
            if (StorageService()->UserDB().GetUsers(QB_.Offset, QB_.Limit, Users, baseQuery )) {
                for (auto &i : Users) {
                    Poco::JSON::Object Obj;
                    if (IdOnly) {
                        ArrayObj.add(i.id);
                    } else {
                        Sanitize(UserInfo_, i);
                        i.to_json(Obj);
                        ArrayObj.add(Obj);
                    }
                }
            }
            Answer.set(RESTAPI::Protocol::USERS, ArrayObj);
            return ReturnObject(Answer);
        } else {
            Poco::JSON::Array ArrayObj;
            for(auto &i:SelectedRecords()) {
                SecurityObjects::UserInfo   UInfo;
                auto tI{i};
                if(StorageService()->UserDB().GetUserById(i,UInfo)) {
                    Poco::JSON::Object Obj;
                    if (IdOnly) {
                        ArrayObj.add(UInfo.id);
                    } else {
                        Sanitize(UserInfo_, UInfo);
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