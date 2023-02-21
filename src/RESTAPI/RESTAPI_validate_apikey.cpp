//
// Created by stephane bourque on 2022-11-07.
//

#include "RESTAPI_validate_apikey.h"
#include "AuthService.h"

namespace OpenWifi {

	void RESTAPI_validate_apikey::DoGet() {
		Poco::URI URI(Request->getURI());
		auto Parameters = URI.getQueryParameters();
		for (auto const &i : Parameters) {
			if (i.first == "apikey") {
				//  can we find this token?
				SecurityObjects::UserInfoAndPolicy SecObj;
				bool Expired = false;
				bool Suspended = false;
				std::uint64_t expiresOn = 0;
				if (AuthService()->IsValidApiKey(i.second, SecObj.webtoken, SecObj.userinfo,
												 Expired, expiresOn, Suspended)) {
					Poco::JSON::Object Answer;
					SecObj.to_json(Answer);
					Answer.set("expiresOn", expiresOn);
					return ReturnObject(Answer);
				}
				if (Suspended)
					return UnAuthorized(RESTAPI::Errors::ACCOUNT_SUSPENDED);
				return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
			}
		}
		return NotFound();
	}

} // namespace OpenWifi