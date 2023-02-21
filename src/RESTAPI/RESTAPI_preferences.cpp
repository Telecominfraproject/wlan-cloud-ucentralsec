//
// Created by stephane bourque on 2021-11-16.
//

#include "RESTAPI_preferences.h"
#include "StorageService.h"

namespace OpenWifi {

	void RESTAPI_preferences::DoGet() {
		SecurityObjects::Preferences P;
		Poco::JSON::Object Answer;
		StorageService()->PreferencesDB().GetPreferences(UserInfo_.userinfo.id, P);
		P.to_json(Answer);
		ReturnObject(Answer);
	}

	void RESTAPI_preferences::DoPut() {

		SecurityObjects::Preferences P;

		const auto &RawObject = ParsedBody_;
		if (!P.from_json(RawObject)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		P.id = UserInfo_.userinfo.id;
		P.modified = OpenWifi::Now();
		StorageService()->PreferencesDB().SetPreferences(P);

		Poco::JSON::Object Answer;
		P.to_json(Answer);
		ReturnObject(Answer);
	}

} // namespace OpenWifi