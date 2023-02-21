//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_validate_token_handler.h"
#include "AuthService.h"

namespace OpenWifi {
	void RESTAPI_validate_token_handler::DoGet() {
		Poco::URI URI(Request->getURI());
		auto Parameters = URI.getQueryParameters();
		for (auto const &i : Parameters) {
			if (i.first == "token") {
				//  can we find this token?
				SecurityObjects::UserInfoAndPolicy SecObj;
				bool Expired = false;
				if (AuthService()->IsValidToken(i.second, SecObj.webtoken, SecObj.userinfo,
												Expired)) {
					Poco::JSON::Object Obj;
					SecObj.to_json(Obj);
					return ReturnObject(Obj);
				}
			}
		}
		return NotFound();
	}
} // namespace OpenWifi