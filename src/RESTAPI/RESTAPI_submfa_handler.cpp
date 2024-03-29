//
// Created by stephane bourque on 2021-12-01.
//

#include "RESTAPI_submfa_handler.h"
#include "SMSSender.h"
#include "StorageService.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	void RESTAPI_submfa_handler::DoGet() {
		SecurityObjects::UserInfo User;

		if (StorageService()->SubDB().GetUserById(UserInfo_.userinfo.id, User)) {
			Poco::JSON::Object Answer;
			SecurityObjects::SubMfaConfig MFC;

			MFC.id = User.id;
			if (User.userTypeProprietaryInfo.mfa.enabled) {
				if (User.userTypeProprietaryInfo.mfa.method == "sms") {
					MFC.sms = User.userTypeProprietaryInfo.mobiles[0].number;
					MFC.type = "sms";
				} else if (User.userTypeProprietaryInfo.mfa.method == "email") {
					MFC.email = User.email;
					MFC.type = "email";
				}
			} else {
				MFC.type = "disabled";
			}
			MFC.to_json(Answer);
			return ReturnObject(Answer);
		}
		NotFound();
	}

	void RESTAPI_submfa_handler::DoPut() {

		try {
			const auto &Body = ParsedBody_;

			SecurityObjects::SubMfaConfig MFC;

			if (!MFC.from_json(Body)) {
				return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
			}

			if (MFC.type == "disabled") {
				SecurityObjects::UserInfo User;
				StorageService()->SubDB().GetUserById(UserInfo_.userinfo.id, User);
				User.userTypeProprietaryInfo.mfa.enabled = false;
				StorageService()->SubDB().UpdateUserInfo(UserInfo_.userinfo.email,
														 UserInfo_.userinfo.id, User);

				Poco::JSON::Object Answer;
				MFC.to_json(Answer);
				return ReturnObject(Answer);
			} else if (MFC.type == "email") {
				SecurityObjects::UserInfo User;

				StorageService()->SubDB().GetUserById(UserInfo_.userinfo.id, User);
				User.userTypeProprietaryInfo.mfa.enabled = true;
				User.userTypeProprietaryInfo.mfa.method = "email";
				StorageService()->SubDB().UpdateUserInfo(UserInfo_.userinfo.email,
														 UserInfo_.userinfo.id, User);

				MFC.sms = MFC.sms;
				MFC.type = "email";
				MFC.email = UserInfo_.userinfo.email;
				MFC.id = MicroServiceCreateUUID();

				Poco::JSON::Object Answer;
				MFC.to_json(Answer);
				return ReturnObject(Answer);

			} else if (MFC.type == "sms") {
				if (GetBoolParameter("startValidation", false)) {
					if (MFC.sms.empty()) {
						return BadRequest(RESTAPI::Errors::SMSMissingPhoneNumber);
					}

					if (!SMSSender()->Enabled()) {
						return BadRequest(RESTAPI::Errors::SMSMFANotEnabled);
					}

					if (SMSSender()->StartValidation(MFC.sms, UserInfo_.userinfo.email)) {
						return OK();
					} else {
						return InternalError(RESTAPI::Errors::SMSTryLater);
					}
				} else if (GetBoolParameter("completeValidation", false)) {

					if (!SMSSender()->Enabled()) {
						return BadRequest(RESTAPI::Errors::SMSMFANotEnabled);
					}

					auto ChallengeCode = GetParameter("challengeCode", "");
					if (ChallengeCode.empty()) {
						return BadRequest(RESTAPI::Errors::SMSMissingChallenge);
					}
					if (MFC.sms.empty()) {
						return BadRequest(RESTAPI::Errors::SMSMissingPhoneNumber);
					}
					if (SMSSender()->CompleteValidation(MFC.sms, ChallengeCode,
														UserInfo_.userinfo.email)) {
						SecurityObjects::UserInfo User;

						StorageService()->SubDB().GetUserById(UserInfo_.userinfo.id, User);
						User.userTypeProprietaryInfo.mfa.enabled = true;
						User.userTypeProprietaryInfo.mfa.method = "sms";
						SecurityObjects::MobilePhoneNumber PhoneNumber;
						PhoneNumber.number = MFC.sms;
						PhoneNumber.primary = true;
						PhoneNumber.verified = true;
						User.userTypeProprietaryInfo.mobiles.clear();
						User.userTypeProprietaryInfo.mobiles.push_back(PhoneNumber);

						StorageService()->SubDB().UpdateUserInfo(UserInfo_.userinfo.email,
																 UserInfo_.userinfo.id, User);

						MFC.sms = MFC.sms;
						MFC.type = "sms";
						MFC.email = UserInfo_.userinfo.email;
						MFC.id = MicroServiceCreateUUID();

						Poco::JSON::Object Answer;
						MFC.to_json(Answer);

						return ReturnObject(Answer);

					} else {
						return InternalError(RESTAPI::Errors::SMSTryLater);
					}
				}
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

} // namespace OpenWifi
