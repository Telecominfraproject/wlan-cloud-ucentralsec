//
// Created by stephane bourque on 2021-11-16.
//

#include "RESTAPI_subpreferences.h"
#include "StorageService.h"

namespace OpenWifi {

	void RESTAPI_subpreferences::DoGet() {
		SecurityObjects::Preferences P;
		Poco::JSON::Object Answer;
		StorageService()->SubPreferencesDB().GetPreferences(UserInfo_.userinfo.id, P);
		P.to_json(Answer);
		ReturnObject(Answer);
	}

	void RESTAPI_subpreferences::DoPut() {

		SecurityObjects::Preferences P;

		const auto &RawObject = ParsedBody_;
		if (!P.from_json(RawObject)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		P.id = UserInfo_.userinfo.id;
		P.modified = OpenWifi::Now();
		StorageService()->SubPreferencesDB().SetPreferences(P);

		Poco::JSON::Object Answer;
		P.to_json(Answer);
		ReturnObject(Answer);
	}

} // namespace OpenWifi