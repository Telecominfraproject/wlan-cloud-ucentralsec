//
// Created by stephane bourque on 2022-11-04.
//

#include "RESTAPI_apiKey_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

	void RESTAPI_apiKey_handler::DoGet() {
		std::string user_uuid = GetBinding("uuid", "");
		if (user_uuid.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}
		if (user_uuid != UserInfo_.userinfo.id &&
			UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		SecurityObjects::ApiKeyEntryList List;
		if (DB_.GetRecords(0, 500, List.apiKeys, fmt::format(" userUuid='{}' ", user_uuid))) {
			for (auto &key : List.apiKeys) {
				Sanitize(UserInfo_, key);
			}
			Poco::JSON::Object Answer;
			List.to_json(Answer);
			return ReturnObject(Answer);
		}
		return NotFound();
	}

	void RESTAPI_apiKey_handler::DoDelete() {
		std::string user_uuid = GetBinding("uuid", "");
		if (user_uuid.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (user_uuid != UserInfo_.userinfo.id &&
			UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if (user_uuid != UserInfo_.userinfo.id) {
			if (!StorageService()->UserDB().Exists("id", user_uuid)) {
				return NotFound();
			}
		}

		std::string ApiKeyId = GetParameter("keyUuid", "");
		if (ApiKeyId.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		SecurityObjects::ApiKeyEntry ApiKey;
		if (StorageService()->ApiKeyDB().GetRecord("id", ApiKeyId, ApiKey)) {
			if (ApiKey.userUuid == user_uuid) {
				AuthService()->RemoveTokenSystemWide(ApiKey.apiKey);
				DB_.DeleteRecord("id", ApiKeyId);
				return OK();
			}
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}
		return NotFound();
	}

	void RESTAPI_apiKey_handler::DoPost() {
		std::string user_uuid = GetBinding("uuid", "");

		if (user_uuid.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (user_uuid != UserInfo_.userinfo.id &&
			UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if (user_uuid != UserInfo_.userinfo.id) {
			//  Must verify if the user exists
			if (!StorageService()->UserDB().Exists("id", user_uuid)) {
				return BadRequest(RESTAPI::Errors::UserMustExist);
			}
		}

		SecurityObjects::ApiKeyEntry NewKey;
		if (!NewKey.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}
		NewKey.lastUse = 0;

		if (!Utils::IsAlphaNumeric(NewKey.name) || NewKey.name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		Poco::toLowerInPlace(NewKey.name);
		NewKey.userUuid = user_uuid;
		if (NewKey.expiresOn < Utils::Now()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		//  does a key of that name already exit for this user?
		SecurityObjects::ApiKeyEntryList ExistingList;
		if (DB_.GetRecords(0, 500, ExistingList.apiKeys,
						   fmt::format(" userUuid='{}' ", user_uuid))) {
			if (std::find_if(ExistingList.apiKeys.begin(), ExistingList.apiKeys.end(),
							 [NewKey](const SecurityObjects::ApiKeyEntry &E) -> bool {
								 return E.name == NewKey.name;
							 }) != ExistingList.apiKeys.end()) {
				return BadRequest(RESTAPI::Errors::ApiKeyNameAlreadyExists);
			}
		}

		if (ExistingList.apiKeys.size() >= 10) {
			return BadRequest(RESTAPI::Errors::TooManyApiKeys);
		}

		NewKey.id = MicroServiceCreateUUID();
		NewKey.userUuid = user_uuid;
		NewKey.salt = std::to_string(Utils::Now());
		NewKey.apiKey = Utils::ComputeHash(NewKey.salt, UserInfo_.userinfo.id,
										   UserInfo_.webtoken.access_token_);
		NewKey.created = Utils::Now();

		if (DB_.CreateRecord(NewKey)) {
			Poco::JSON::Object Answer;
			NewKey.to_json(Answer);
			return ReturnObject(Answer);
		}
		return BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_apiKey_handler::DoPut() {
		std::string user_uuid = GetBinding("uuid", "");
		if (user_uuid.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}
		if (user_uuid != UserInfo_.userinfo.id &&
			UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}
		SecurityObjects::ApiKeyEntry NewKey;
		if (!NewKey.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		SecurityObjects::ApiKeyEntry ExistingKey;
		if (!DB_.GetRecord("id", NewKey.id, ExistingKey)) {
			return BadRequest(RESTAPI::Errors::ApiKeyDoesNotExist);
		}

		if (ExistingKey.userUuid != user_uuid) {
			return BadRequest(RESTAPI::Errors::MissingUserID);
		}

		AssignIfPresent(ParsedBody_, "description", ExistingKey.description);

		if (DB_.UpdateRecord("id", ExistingKey.id, ExistingKey)) {
			Poco::JSON::Object Answer;
			ExistingKey.to_json(Answer);
			return ReturnObject(Answer);
		}
		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}

} // namespace OpenWifi