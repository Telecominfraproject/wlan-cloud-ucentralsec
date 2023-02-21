//
// Created by stephane bourque on 2021-11-30.
//

#include "RESTAPI_subusers_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"
#include "StorageService.h"

namespace OpenWifi {

	void RESTAPI_subusers_handler::DoGet() {
		bool IdOnly = GetBoolParameter("idOnly");
		auto operatorId = GetParameter("operatorId");
		auto nameSearch = GetParameter("nameSearch");
		auto emailSearch = GetParameter("emailSearch");

		std::string baseQuery;
		if (!nameSearch.empty() || !emailSearch.empty()) {
			if (!nameSearch.empty())
				baseQuery = fmt::format(" Lower(name) like('%{}%') ",
										ORM::Escape(Poco::toLower(nameSearch)));
			if (!emailSearch.empty())
				baseQuery += baseQuery.empty()
								 ? fmt::format(" Lower(email) like('%{}%') ",
											   ORM::Escape(Poco::toLower(emailSearch)))
								 : fmt::format(" and Lower(email) like('%{}%') ",
											   ORM::Escape(Poco::toLower(emailSearch)));
		}

		if (QB_.CountOnly) {
			std::string whereClause;
			if (!operatorId.empty() && Utils::ValidUUID(operatorId)) {
				whereClause = baseQuery.empty()
								  ? fmt::format(" owner='{}' ", operatorId)
								  : fmt::format(" owner='{}' and {} ", operatorId, baseQuery);
				auto count = StorageService()->SubDB().Count(whereClause);
				return ReturnCountOnly(count);
			}
			auto count = StorageService()->UserDB().Count();
			return ReturnCountOnly(count);
		} else if (QB_.Select.empty()) {
			std::string whereClause;
			if (!operatorId.empty() && Utils::ValidUUID(operatorId)) {
				whereClause = baseQuery.empty()
								  ? fmt::format(" owner='{}' ", operatorId)
								  : fmt::format(" owner='{}' and {} ", operatorId, baseQuery);
			}

			SecurityObjects::UserInfoList Users;
			if (StorageService()->SubDB().GetUsers(QB_.Offset, QB_.Limit, Users.users,
												   whereClause)) {
				for (auto &i : Users.users) {
					Sanitize(UserInfo_, i);
				}
			}

			if (IdOnly) {
				Poco::JSON::Array Arr;
				Poco::JSON::Object Answer;

				for (const auto &i : Users.users) {
					Arr.add(i.id);
				}
				Answer.set("users", Arr);
				return ReturnObject(Answer);
			}

			Poco::JSON::Object Answer;
			Users.to_json(Answer);
			return ReturnObject(Answer);
		} else {
			SecurityObjects::UserInfoList Users;
			for (auto &i : SelectedRecords()) {
				SecurityObjects::UserInfo UInfo;
				if (StorageService()->SubDB().GetUserById(i, UInfo)) {
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
} // namespace OpenWifi