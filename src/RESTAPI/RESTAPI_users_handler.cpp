//
// Created by stephane bourque on 2021-06-21.
//

#include "RESTAPI_users_handler.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {
    void RESTAPI_users_handler::DoGet() {
        bool IdOnly = (GetParameter("idOnly","false")=="true");
        auto nameSearch = GetParameter("nameSearch");
        auto emailSearch = GetParameter("emailSearch");

        std::string baseQuery;
        if(!nameSearch.empty() || !emailSearch.empty()) {
            if(!nameSearch.empty())
                baseQuery = fmt::format(" Lower(name) like('%{}%') ", Poco::toLower(nameSearch) );
            if(!emailSearch.empty())
                baseQuery += baseQuery.empty() ? fmt::format(" Lower(email) like('%{}%') ", Poco::toLower(emailSearch))
                                               : fmt::format(" and Lower(email) like('%{}%') ", Poco::toLower(emailSearch));
        }

        if(QB_.Select.empty()) {
            SecurityObjects::UserInfoList   Users;
            if(StorageService()->UserDB().GetUsers(QB_.Offset, QB_.Limit, Users.users, baseQuery)) {
                for (auto &i : Users.users) {
                    Sanitize(UserInfo_, i);
                }
                if(IdOnly) {
                    Poco::JSON::Array   Arr;
                    for(const auto &i:Users.users)
                        Arr.add(i.id);
                    Poco::JSON::Object  Answer;
                    Answer.set("users", Arr);
                    return ReturnObject(Answer);
                }
            }
            Poco::JSON::Object  Answer;
            Users.to_json(Answer);
            return ReturnObject(Answer);
        } else {
            SecurityObjects::UserInfoList   Users;
            for(auto &i:SelectedRecords()) {
                SecurityObjects::UserInfo   UInfo;
                if(StorageService()->UserDB().GetUserById(i,UInfo)) {
                    Poco::JSON::Object Obj;
                    Sanitize(UserInfo_, UInfo);
                    Users.users.emplace_back(UInfo);
                }
            }
            Poco::JSON::Object Answer;
            Users.to_json(Answer);
            return ReturnObject(Answer);
        }
    }
}