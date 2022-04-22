//
// Created by stephane bourque on 2021-11-30.
//

#include "RESTAPI_subusers_handler.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_subusers_handler::DoGet() {
        std::vector<SecurityObjects::UserInfo> Users;
        bool IdOnly = (GetParameter("idOnly","false")=="true");
        auto operatorId = GetParameter("operatorId");
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

        if(QB_.CountOnly) {
            std::string whereClause;
            if(!operatorId.empty()) {
                whereClause = baseQuery.empty() ? fmt::format(" owner='{}' ", operatorId) :
                              fmt::format(" owner='{}' and {} ", operatorId, baseQuery);
                auto count = StorageService()->SubDB().Count(whereClause);
                return ReturnCountOnly(count);
            }
            auto count = StorageService()->UserDB().Count();
            return ReturnCountOnly(count);
        } else if(QB_.Select.empty()) {
            Poco::JSON::Array ArrayObj;
            Poco::JSON::Object Answer;
            std::string whereClause;
            if(!operatorId.empty()) {
                whereClause = baseQuery.empty() ? fmt::format(" owner='{}' ", operatorId) :
                              fmt::format(" owner='{}' and {} ", operatorId, baseQuery);
            }
            if (StorageService()->SubDB().GetUsers(QB_.Offset, QB_.Limit, Users, whereClause)) {
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
                if(StorageService()->SubDB().GetUserById(tI,UInfo)) {
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