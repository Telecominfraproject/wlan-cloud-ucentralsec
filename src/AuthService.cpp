//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "framework/KafkaManager.h"
#include "framework/KafkaTopics.h"

#include "Poco/JWT/Signer.h"
#include "Poco/JWT/Token.h"
#include "Poco/Net/OAuth20Credentials.h"
#include "Poco/StringTokenizer.h"

#include "AuthService.h"
#include "StorageService.h"
#include "framework/MicroServiceFuncs.h"

#include "MFAServer.h"
#include "MessagingTemplates.h"
#include "SMTPMailerService.h"

namespace OpenWifi {

	AuthService::ACCESS_TYPE AuthService::IntToAccessType(int C) {
		switch (C) {
		case 1:
			return USERNAME;
		case 2:
			return SERVER;
		case 3:
			return CUSTOM;
		default:
			return USERNAME;
		}
	}

	int AuthService::AccessTypeToInt(ACCESS_TYPE T) {
		switch (T) {
		case USERNAME:
			return 1;
		case SERVER:
			return 2;
		case CUSTOM:
			return 3;
		}
		return 1; // some compilers complain...
	}

#if defined(TIP_CERT_SERVICE)
	static const std::string DefaultPasswordRule{
		"^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[\\{\\}\\(\\)~_\\+\\|\\\\\\[\\]\\;\\:\\<\\>\\."
		"\\,\\/\\?\\\"\\'\\`\\=#?!@$%^&*-]).{12,}$"};
#else
	static const std::string DefaultPasswordRule{
		"^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[\\{\\}\\(\\)~_\\+\\|\\\\\\[\\]\\;\\:\\<\\>\\."
		"\\,\\/\\?\\\"\\'\\`\\=#?!@$%^&*-]).{8,}$"};
#endif

	int AuthService::Start() {
		poco_information(Logger(), "Starting...");
		TokenAging_ =
			(uint64_t)MicroServiceConfigGetInt("authentication.token.ageing", 30 * 24 * 60 * 60);
		RefreshTokenLifeSpan_ = (uint64_t)MicroServiceConfigGetInt(
			"authentication.refresh_token.lifespan", 90 * 24 * 60 * 600);
		HowManyOldPassword_ = MicroServiceConfigGetInt("authentication.oldpasswords", 5);

		AccessPolicy_ = MicroServiceConfigGetString("openwifi.document.policy.access",
													"/wwwassets/access_policy.html");
		PasswordPolicy_ = MicroServiceConfigGetString("openwifi.document.policy.password",
													  "/wwwassets/password_policy.html");
		PasswordValidation_ = PasswordValidationStr_ = MicroServiceConfigGetString(
			"authentication.validation.expression", DefaultPasswordRule);

		SubPasswordValidation_ = SubPasswordValidationStr_ =
			MicroServiceConfigGetString("subscriber.validation.expression", DefaultPasswordRule);
		SubAccessPolicy_ = MicroServiceConfigGetString("subscriber.policy.access",
													   "/wwwassets/access_policy.html");
		SubPasswordPolicy_ = MicroServiceConfigGetString("subscriber.policy.password",
														 "/wwwassets/password_policy.html");

		HelperEmail_ =
			MicroServiceConfigGetString("helper.user.email", "openwifi@telecominfraproject.com");
		SubHelperEmail_ =
			MicroServiceConfigGetString("helper.sub.email", "openwifi@telecominfraproject.com");

		GlobalHelperEmail_ = MicroServiceConfigGetString("helper.user.global.email",
														 "openwifi@telecominfraproject.com");
		GlobalSubHelperEmail_ = MicroServiceConfigGetString("helper.sub.global.email",
															"openwifi@telecominfraproject.com");

		HelperSite_ = MicroServiceConfigGetString("helper.user.site", "telecominfraproject.com");
		SubHelperSite_ = MicroServiceConfigGetString("helper.sub.site", "telecominfraproject.com");

		SystemLoginSite_ =
			MicroServiceConfigGetString("helper.user.login", "telecominfraproject.com");
		SubSystemLoginSite_ =
			MicroServiceConfigGetString("helper.sub.login", "telecominfraproject.com");

		UserSignature_ =
			MicroServiceConfigGetString("helper.user.signature", "Telecom Infra Project");
		SubSignature_ =
			MicroServiceConfigGetString("helper.sub.signature", "Telecom Infra Project");

		return 0;
	}

	void AuthService::Stop() {
		poco_information(Logger(), "Stopping...");
		poco_information(Logger(), "Stopped...");
	}

	bool AuthService::RefreshUserToken(Poco::Net::HTTPServerRequest &Request,
									   const std::string &RefreshToken,
									   SecurityObjects::UserInfoAndPolicy &UI) {
		try {
			std::string CallToken;
			Poco::Net::OAuth20Credentials Auth(Request);
			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}

			if (CallToken.empty()) {
				return false;
			}

			uint64_t RevocationDate = 0;
			std::string UserId;
			if (StorageService()->UserTokenDB().GetToken(CallToken, UI.webtoken, UserId,
														 RevocationDate) &&
				UI.webtoken.refresh_token_ == RefreshToken) {
				auto now = Utils::Now();

				//  Create a new token
				auto NewToken = GenerateTokenHMAC(UI.webtoken.access_token_, CUSTOM);
				auto NewRefreshToken = RefreshToken;
				if (now - UI.webtoken.lastRefresh_ < RefreshTokenLifeSpan_) {
					NewRefreshToken = GenerateTokenHMAC(UI.webtoken.refresh_token_, CUSTOM);
					UI.webtoken.lastRefresh_ = now;
				}

				StorageService()->UserTokenDB().RefreshToken(CallToken, NewToken, NewRefreshToken,
															 UI.webtoken.lastRefresh_);
				UI.webtoken.access_token_ = NewToken;
				UI.webtoken.refresh_token_ = NewRefreshToken;
				return true;
			}
			return false;

		} catch (...) {
		}
		return false;
	}

	bool AuthService::RefreshSubToken(Poco::Net::HTTPServerRequest &Request,
									  const std::string &RefreshToken,
									  SecurityObjects::UserInfoAndPolicy &UI) {
		try {
			std::string CallToken;
			Poco::Net::OAuth20Credentials Auth(Request);
			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}

			if (CallToken.empty()) {
				return false;
			}

			uint64_t RevocationDate = 0;
			std::string UserId;
			if (StorageService()->SubTokenDB().GetToken(CallToken, UI.webtoken, UserId,
														RevocationDate) &&
				UI.webtoken.refresh_token_ == RefreshToken) {
				auto now = Utils::Now();

				//  Create a new token
				auto NewToken = GenerateTokenHMAC(UI.webtoken.access_token_, CUSTOM);
				auto NewRefreshToken = RefreshToken;
				if (now - UI.webtoken.lastRefresh_ < RefreshTokenLifeSpan_) {
					NewRefreshToken = GenerateTokenHMAC(UI.webtoken.refresh_token_, CUSTOM);
					UI.webtoken.lastRefresh_ = now;
				}

				StorageService()->SubTokenDB().RefreshToken(CallToken, NewToken, NewRefreshToken,
															UI.webtoken.lastRefresh_);
				UI.webtoken.access_token_ = NewToken;
				UI.webtoken.refresh_token_ = NewRefreshToken;
				return true;
			}
			return false;

		} catch (...) {
		}
		return false;
	}

	[[nodiscard]] bool AuthService::IsAuthorized(const std::string &SessionToken,
												 SecurityObjects::UserInfoAndPolicy &UInfo,
												 std::uint64_t TID, bool &Expired) {
		// std::lock_guard	Guard(Mutex_);
		std::string CallToken{SessionToken};
		Expired = false;
		try {
			SecurityObjects::WebToken WT;
			uint64_t RevocationDate = 0;
			std::string UserId;
			if (StorageService()->UserTokenDB().GetToken(CallToken, WT, UserId, RevocationDate)) {
				if (RevocationDate != 0) {
					poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}",
													 TID, Utils::SanitizeToken(CallToken)));
					return false;
				}
				auto now = Utils::Now();
				Expired = (WT.created_ + WT.expires_in_) < now;
				if (StorageService()->UserDB().GetUserById(UserId, UInfo.userinfo)) {
					UInfo.webtoken = WT;
					poco_trace(Logger(), fmt::format("TokenValidation success for TID={} Token={}",
													 TID, Utils::SanitizeToken(CallToken)));
					return true;
				}
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}", TID,
										 Utils::SanitizeToken(CallToken)));
		return false;
	}

	bool AuthService::IsAuthorized(Poco::Net::HTTPServerRequest &Request, std::string &SessionToken,
								   SecurityObjects::UserInfoAndPolicy &UInfo, std::uint64_t TID,
								   bool &Expired) {
		// std::lock_guard	Guard(Mutex_);
		std::string CallToken;
		Expired = false;

		try {
			Poco::Net::OAuth20Credentials Auth(Request);
			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}

			if (CallToken.empty()) {
				poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}", TID,
												 Utils::SanitizeToken(CallToken)));
				return false;
			}
			SessionToken = CallToken;
			return IsAuthorized(SessionToken, UInfo, TID, Expired);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}", TID,
										 Utils::SanitizeToken(CallToken)));
		return false;
	}

	bool AuthService::IsSubAuthorized(Poco::Net::HTTPServerRequest &Request,
									  std::string &SessionToken,
									  SecurityObjects::UserInfoAndPolicy &UInfo, std::uint64_t TID,
									  bool &Expired) {
		// std::lock_guard	Guard(Mutex_);

		std::string CallToken;
		Expired = false;
		try {
			Poco::Net::OAuth20Credentials Auth(Request);
			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}

			if (CallToken.empty()) {
				poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}", TID,
												 Utils::SanitizeToken(CallToken)));
				return false;
			}

			SecurityObjects::WebToken WT;
			uint64_t RevocationDate = 0;
			std::string UserId;
			if (StorageService()->SubTokenDB().GetToken(CallToken, WT, UserId, RevocationDate)) {
				if (RevocationDate != 0) {
					poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}",
													 TID, Utils::SanitizeToken(CallToken)));
					return false;
				}
				auto now = Utils::Now();
				Expired = (WT.created_ + WT.expires_in_) < now;
				if (StorageService()->SubDB().GetUserById(UserId, UInfo.userinfo)) {
					UInfo.webtoken = WT;
					SessionToken = CallToken;
					poco_debug(Logger(), fmt::format("TokenValidation success for TID={} Token={}",
													 TID, Utils::SanitizeToken(CallToken)));
					return true;
				}
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		poco_debug(Logger(), fmt::format("TokenValidation failed for TID={} Token={}", TID,
										 Utils::SanitizeToken(CallToken)));
		return false;
	}

	void AuthService::RevokeToken(std::string &Token) {
		StorageService()->UserTokenDB().RevokeToken(Token);
	}

	void AuthService::RevokeSubToken(std::string &Token) {
		StorageService()->SubTokenDB().RevokeToken(Token);
	}

	bool AuthService::DeleteUserFromCache(const std::string &Id) {
		return StorageService()->UserTokenDB().DeleteRecordsFromCache("userName", Id);
	}

	bool AuthService::DeleteSubUserFromCache(const std::string &Id) {
		return StorageService()->SubTokenDB().DeleteRecordsFromCache("userName", Id);
	}

	bool AuthService::RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo) {
		return (UInfo.userinfo.userTypeProprietaryInfo.mfa.enabled &&
				MFAServer::MethodEnabled(UInfo.userinfo.userTypeProprietaryInfo.mfa.method));
	}

	bool AuthService::ValidatePassword(const std::string &Password) {
		return std::regex_match(Password, PasswordValidation_);
	}

	bool AuthService::ValidateSubPassword(const std::string &Password) {
		return std::regex_match(Password, SubPasswordValidation_);
	}

	void AuthService::RemoveTokenSystemWide(const std::string &token) {
		try {
			if (KafkaManager()->Enabled()) {
				Poco::JSON::Object Obj;
				Obj.set("event", "remove-token");
				Obj.set("id", MicroServiceID());
				Obj.set("token", token);
				KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,
											MicroServicePrivateEndPoint(), Obj, false);
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
	}

	void AuthService::Logout(const std::string &Token, [[maybe_unused]] bool EraseFromCache) {
		std::lock_guard Guard(Mutex_);

		try {
			auto tToken{Token};
			StorageService()->UserTokenDB().DeleteRecord("token", tToken);
			StorageService()->LoginDB().AddLogout(Token);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
	}

	void AuthService::SubLogout(const std::string &Token, [[maybe_unused]] bool EraseFromCache) {
		std::lock_guard Guard(Mutex_);

		try {
			auto tToken{Token};
			StorageService()->SubTokenDB().DeleteRecord("token", tToken);
			StorageService()->SubLoginDB().AddLogout(Token);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
	}

	[[nodiscard]] std::string AuthService::GenerateTokenHMAC(const std::string &UserName,
															 [[maybe_unused]] ACCESS_TYPE Type) {
		std::string Identity(UserName + ":" + fmt::format("{}", Utils::Now()) + ":" +
							 std::to_string(rand()));
		HMAC_.update(Identity);
		return Poco::DigestEngine::digestToHex(HMAC_.digest());
	}

	std::string AuthService::GenerateTokenJWT(const std::string &Identity, ACCESS_TYPE Type) {
		std::lock_guard Guard(Mutex_);

		Poco::JWT::Token T;

		T.setType("JWT");
		switch (Type) {
		case USERNAME:
			T.setSubject("usertoken");
			break;
		case SERVER:
			T.setSubject("servertoken");
			break;
		case CUSTOM:
			T.setSubject("customtoken");
			break;
		}

		T.payload().set("identity", Identity);
		T.setIssuedAt(Poco::Timestamp());
		T.setExpiration(Poco::Timestamp() + (long long)TokenAging_);
		std::string JWT = MicroServiceSign(T, Poco::JWT::Signer::ALGO_RS256);

		return JWT;
	}

	void AuthService::CreateToken(const std::string &UserName,
								  SecurityObjects::UserInfoAndPolicy &UInfo) {
		std::lock_guard Guard(Mutex_);

		SecurityObjects::AclTemplate ACL;
		ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
		UInfo.webtoken.acl_template_ = ACL;
		UInfo.webtoken.expires_in_ = TokenAging_;
		UInfo.webtoken.idle_timeout_ = 5 * 60;
		UInfo.webtoken.token_type_ = "Bearer";
		UInfo.webtoken.access_token_ = GenerateTokenHMAC(UInfo.userinfo.id, USERNAME);
		UInfo.webtoken.id_token_ = GenerateTokenHMAC(UInfo.userinfo.id, USERNAME);
		UInfo.webtoken.refresh_token_ = GenerateTokenHMAC(UInfo.userinfo.id, CUSTOM);
		UInfo.webtoken.created_ = time(nullptr);
		UInfo.webtoken.username_ = UserName;
		UInfo.webtoken.errorCode = 0;
		UInfo.webtoken.userMustChangePassword = false;
		StorageService()->UserDB().SetLastLogin(UInfo.userinfo.id);
		StorageService()->UserTokenDB().AddToken(
			UInfo.userinfo.id, UInfo.webtoken.access_token_, UInfo.webtoken.refresh_token_,
			UInfo.webtoken.token_type_, UInfo.webtoken.expires_in_, UInfo.webtoken.idle_timeout_);
		StorageService()->LoginDB().AddLogin(UInfo.userinfo.id, UInfo.userinfo.email,
											 UInfo.webtoken.access_token_);
	}

	void AuthService::CreateSubToken(const std::string &UserName,
									 SecurityObjects::UserInfoAndPolicy &UInfo) {
		std::lock_guard Guard(Mutex_);

		SecurityObjects::AclTemplate ACL;
		ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
		UInfo.webtoken.acl_template_ = ACL;
		UInfo.webtoken.expires_in_ = TokenAging_;
		UInfo.webtoken.idle_timeout_ = 5 * 60;
		UInfo.webtoken.token_type_ = "Bearer";
		UInfo.webtoken.access_token_ = GenerateTokenHMAC(UInfo.userinfo.id, USERNAME);
		UInfo.webtoken.id_token_ = GenerateTokenHMAC(UInfo.userinfo.id, USERNAME);
		UInfo.webtoken.refresh_token_ = GenerateTokenHMAC(UInfo.userinfo.id, CUSTOM);
		UInfo.webtoken.created_ = time(nullptr);
		UInfo.webtoken.username_ = UserName;
		UInfo.webtoken.errorCode = 0;
		UInfo.webtoken.userMustChangePassword = false;
		StorageService()->SubDB().SetLastLogin(UInfo.userinfo.id);
		StorageService()->SubTokenDB().AddToken(
			UInfo.userinfo.id, UInfo.webtoken.access_token_, UInfo.webtoken.refresh_token_,
			UInfo.webtoken.token_type_, UInfo.webtoken.expires_in_, UInfo.webtoken.idle_timeout_);
		StorageService()->SubLoginDB().AddLogin(UInfo.userinfo.id, UInfo.userinfo.email,
												UInfo.webtoken.access_token_);
	}

	bool AuthService::SetPassword(const std::string &NewPassword,
								  SecurityObjects::UserInfo &UInfo) {
		std::lock_guard G(Mutex_);

		Poco::toLowerInPlace(UInfo.email);
		for (const auto &i : UInfo.lastPasswords) {
			auto Tokens = Poco::StringTokenizer(i, "|");
			if (Tokens.count() == 2) {
				const auto &Salt = Tokens[0];
				for (const auto &j : UInfo.lastPasswords) {
					auto OldTokens = Poco::StringTokenizer(j, "|");
					if (OldTokens.count() == 2) {
						SHA2_.update(Salt + NewPassword + UInfo.email);
						if (OldTokens[1] == Utils::ToHex(SHA2_.digest()))
							return false;
					}
				}
			} else {
				SHA2_.update(NewPassword + UInfo.email);
				if (Tokens[0] == Utils::ToHex(SHA2_.digest()))
					return false;
			}
		}

		if (UInfo.lastPasswords.size() == HowManyOldPassword_) {
			UInfo.lastPasswords.erase(UInfo.lastPasswords.begin());
		}

		auto NewHash = ComputeNewPasswordHash(UInfo.email, NewPassword);
		UInfo.lastPasswords.push_back(NewHash);
		UInfo.currentPassword = NewHash;
		UInfo.changePassword = false;
		return true;
	}

	bool AuthService::SetSubPassword(const std::string &NewPassword,
									 SecurityObjects::UserInfo &UInfo) {
		std::lock_guard G(Mutex_);

		Poco::toLowerInPlace(UInfo.email);
		for (const auto &i : UInfo.lastPasswords) {
			auto Tokens = Poco::StringTokenizer(i, "|");
			if (Tokens.count() == 2) {
				const auto &Salt = Tokens[0];
				for (const auto &j : UInfo.lastPasswords) {
					auto OldTokens = Poco::StringTokenizer(j, "|");
					if (OldTokens.count() == 2) {
						SHA2_.update(Salt + NewPassword + UInfo.email);
						if (OldTokens[1] == Utils::ToHex(SHA2_.digest()))
							return false;
					}
				}
			} else {
				SHA2_.update(NewPassword + UInfo.email);
				if (Tokens[0] == Utils::ToHex(SHA2_.digest()))
					return false;
			}
		}

		if (UInfo.lastPasswords.size() == HowManyOldPassword_) {
			UInfo.lastPasswords.erase(UInfo.lastPasswords.begin());
		}

		auto NewHash = ComputeNewPasswordHash(UInfo.email, NewPassword);
		UInfo.lastPasswords.push_back(NewHash);
		UInfo.currentPassword = NewHash;
		UInfo.changePassword = false;
		return true;
	}

	static std::string GetMeSomeSalt() {
		auto start = std::chrono::high_resolution_clock::now();
		return std::to_string(start.time_since_epoch().count());
	}

	std::string AuthService::ComputeNewPasswordHash(const std::string &UserName,
													const std::string &Password) {
		std::string UName = Poco::trim(Poco::toLower(UserName));
		auto Salt = GetMeSomeSalt();
		SHA2_.update(Salt + Password + UName);
		return Salt + "|" + Utils::ToHex(SHA2_.digest());
	}

	bool AuthService::ValidatePasswordHash(const std::string &UserName, const std::string &Password,
										   const std::string &StoredPassword) {
		std::lock_guard G(Mutex_);

		std::string UName = Poco::trim(Poco::toLower(UserName));
		auto Tokens = Poco::StringTokenizer(StoredPassword, "|");
		if (Tokens.count() == 1) {
			SHA2_.update(Password + UName);
			if (Tokens[0] == Utils::ToHex(SHA2_.digest()))
				return true;
		} else if (Tokens.count() == 2) {
			SHA2_.update(Tokens[0] + Password + UName);
			if (Tokens[1] == Utils::ToHex(SHA2_.digest()))
				return true;
		}
		return false;
	}

	bool AuthService::ValidateSubPasswordHash(const std::string &UserName,
											  const std::string &Password,
											  const std::string &StoredPassword) {
		std::lock_guard G(Mutex_);

		std::string UName = Poco::trim(Poco::toLower(UserName));
		auto Tokens = Poco::StringTokenizer(StoredPassword, "|");
		if (Tokens.count() == 1) {
			SHA2_.update(Password + UName);
			if (Tokens[0] == Utils::ToHex(SHA2_.digest()))
				return true;
		} else if (Tokens.count() == 2) {
			SHA2_.update(Tokens[0] + Password + UName);
			if (Tokens[1] == Utils::ToHex(SHA2_.digest()))
				return true;
		}
		return false;
	}

	UNAUTHORIZED_REASON AuthService::Authorize(std::string &UserName, const std::string &Password,
											   const std::string &NewPassword,
											   SecurityObjects::UserInfoAndPolicy &UInfo,
											   [[maybe_unused]] bool &Expired) {
		std::lock_guard Guard(Mutex_);

		Poco::toLowerInPlace(UserName);

		if (StorageService()->UserDB().GetUserByEmail(UserName, UInfo.userinfo)) {

			if (UInfo.userinfo.suspended) {
				return ACCOUNT_SUSPENDED;
			}

			if (UInfo.userinfo.waitingForEmailCheck) {
				return USERNAME_PENDING_VERIFICATION;
			}

			if (!ValidatePasswordHash(UserName, Password, UInfo.userinfo.currentPassword)) {
				return INVALID_CREDENTIALS;
			}

			if (UInfo.userinfo.changePassword && NewPassword.empty()) {
				UInfo.webtoken.userMustChangePassword = true;
				return PASSWORD_CHANGE_REQUIRED;
			}

			if (!NewPassword.empty() && !ValidatePassword(NewPassword)) {
				return PASSWORD_INVALID;
			}

			if (UInfo.userinfo.changePassword || !NewPassword.empty()) {
				if (!SetPassword(NewPassword, UInfo.userinfo)) {
					UInfo.webtoken.errorCode = 1;
					return PASSWORD_ALREADY_USED;
				}
				UInfo.userinfo.lastPasswordChange = Utils::Now();
				UInfo.userinfo.changePassword = false;
				UInfo.userinfo.modified = Utils::Now();
				StorageService()->UserDB().UpdateUserInfo(AUTHENTICATION_SYSTEM, UInfo.userinfo.id,
														  UInfo.userinfo);
			}

			//  so we have a good password, password up date has taken place if need be, now
			//  generate the token.
			UInfo.userinfo.lastLogin = Utils::Now();
			StorageService()->UserDB().SetLastLogin(UInfo.userinfo.id);
			CreateToken(UserName, UInfo);

			return SUCCESS;
		}
		return INVALID_CREDENTIALS;
	}

	UNAUTHORIZED_REASON AuthService::AuthorizeSub(std::string &UserName,
												  const std::string &Password,
												  const std::string &NewPassword,
												  SecurityObjects::UserInfoAndPolicy &UInfo,
												  [[maybe_unused]] bool &Expired) {
		std::lock_guard Guard(Mutex_);

		Poco::toLowerInPlace(UserName);

		if (StorageService()->SubDB().GetUserByEmail(UserName, UInfo.userinfo)) {

			if (UInfo.userinfo.suspended) {
				return ACCOUNT_SUSPENDED;
			}

			if (UInfo.userinfo.waitingForEmailCheck) {
				return USERNAME_PENDING_VERIFICATION;
			}

			if (!ValidateSubPasswordHash(UserName, Password, UInfo.userinfo.currentPassword)) {
				return INVALID_CREDENTIALS;
			}

			if (UInfo.userinfo.changePassword && NewPassword.empty()) {
				UInfo.webtoken.userMustChangePassword = true;
				return PASSWORD_CHANGE_REQUIRED;
			}

			if (!NewPassword.empty() && !ValidateSubPassword(NewPassword)) {
				return PASSWORD_INVALID;
			}

			if (UInfo.userinfo.changePassword || !NewPassword.empty()) {
				if (!SetSubPassword(NewPassword, UInfo.userinfo)) {
					UInfo.webtoken.errorCode = 1;
					return PASSWORD_ALREADY_USED;
				}
				UInfo.userinfo.lastPasswordChange = Utils::Now();
				UInfo.userinfo.changePassword = false;
				UInfo.userinfo.modified = Utils::Now();
				StorageService()->SubDB().UpdateUserInfo(AUTHENTICATION_SYSTEM, UInfo.userinfo.id,
														 UInfo.userinfo);
			}

			//  so we have a good password, password update has taken place if need be, now generate
			//  the token.
			UInfo.userinfo.lastLogin = Utils::Now();
			StorageService()->SubDB().SetLastLogin(UInfo.userinfo.id);
			CreateSubToken(UserName, UInfo);

			return SUCCESS;
		}

		return INVALID_CREDENTIALS;
	}

	bool AuthService::SendEmailChallengeCode(const SecurityObjects::UserInfoAndPolicy &UInfo,
											 const std::string &Challenge) {
		auto OperatorParts = Poco::StringTokenizer(UInfo.userinfo.signingUp, ":");

		bool IsSub = UInfo.userinfo.userRole == SecurityObjects::SUBSCRIBER;

		if (UInfo.userinfo.signingUp.empty() || OperatorParts.count() != 2) {
			MessageAttributes Attrs;
			Attrs[RECIPIENT_EMAIL] = UInfo.userinfo.email;
			Attrs[LOGO] = AuthService::GetLogoAssetURI();
			Attrs[SUBJECT] = "Login validation code";
			Attrs[CHALLENGE_CODE] = Challenge;
			if (!IsSub) {
				SMTPMailerService()->AddUserVars(Attrs);
			} else {
				SMTPMailerService()->AddSubVars(Attrs);
			}
			return SMTPMailerService()->SendMessage(
				UInfo.userinfo.email,
				MessagingTemplates::TemplateName(MessagingTemplates::VERIFICATION_CODE), Attrs,
				false);
		} else {
			MessageAttributes Attrs;
			Attrs[RECIPIENT_EMAIL] = UInfo.userinfo.email;
			Attrs[LOGO] = AuthService::GetSubLogoAssetURI();
			Attrs[SUBJECT] = "Login validation code";
			Attrs[CHALLENGE_CODE] = Challenge;
			if (!IsSub) {
				SMTPMailerService()->AddUserVars(Attrs);
			} else {
				SMTPMailerService()->AddSubVars(Attrs);
			}
			return SMTPMailerService()->SendMessage(
				UInfo.userinfo.email,
				MessagingTemplates::TemplateName(MessagingTemplates::SUB_VERIFICATION_CODE,
												 OperatorParts[0]),
				Attrs, true);
		}
	}

	bool AuthService::SendEmailToUser(const std::string &LinkId, std::string &Email,
									  MessagingTemplates::EMAIL_REASON Reason) {
		SecurityObjects::UserInfo UInfo;

		if (StorageService()->UserDB().GetUserByEmail(Email, UInfo)) {
			switch (Reason) {

			case MessagingTemplates::FORGOT_PASSWORD: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetLogoAssetURI();
				Attrs[SUBJECT] = "Password reset link";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=password_reset&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] = "/api/v1/actionLink?action=password_reset&id=" + LinkId;
				SMTPMailerService()->AddUserVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::FORGOT_PASSWORD), Attrs,
					false);
			} break;

			case MessagingTemplates::EMAIL_VERIFICATION: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetLogoAssetURI();
				Attrs[SUBJECT] = "e-mail Address Verification";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=email_verification&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] =
					"/api/v1/actionLink?action=email_verification&id=" + LinkId;
				SMTPMailerService()->AddUserVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::EMAIL_VERIFICATION), Attrs,
					false);
				UInfo.waitingForEmailCheck = true;
			} break;

			case MessagingTemplates::EMAIL_INVITATION: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetLogoAssetURI();
				Attrs[SUBJECT] = "e-mail Invitation";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=email_invitation&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] = "/api/v1/actionLink?action=email_invitation&id=" + LinkId;
				SMTPMailerService()->AddUserVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::EMAIL_INVITATION), Attrs,
					false);
				UInfo.waitingForEmailCheck = true;
			} break;

			default:
				break;
			}
			return true;
		}
		return false;
	}

	bool AuthService::SendEmailToSubUser(const std::string &LinkId, std::string &Email,
										 MessagingTemplates::EMAIL_REASON Reason,
										 const std::string &OperatorName) {
		SecurityObjects::UserInfo UInfo;

		if (StorageService()->SubDB().GetUserByEmail(Email, UInfo)) {
			switch (Reason) {
			case MessagingTemplates::SUB_FORGOT_PASSWORD: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetSubLogoAssetURI();
				Attrs[SUBJECT] = "Password reset link";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=sub_password_reset&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] =
					"/api/v1/actionLink?action=sub_password_reset&id=" + LinkId;
				SMTPMailerService()->AddSubVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::SUB_FORGOT_PASSWORD,
													 OperatorName),
					Attrs, true);
			} break;

			case MessagingTemplates::SUB_EMAIL_VERIFICATION: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetSubLogoAssetURI();
				Attrs[SUBJECT] = "e-mail Address Verification";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=sub_email_verification&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] =
					"/api/v1/actionLink?action=sub_email_verification&id=" + LinkId;
				SMTPMailerService()->AddSubVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::SUB_EMAIL_VERIFICATION,
													 OperatorName),
					Attrs, true);
				UInfo.waitingForEmailCheck = true;
			} break;

			case MessagingTemplates::SUB_SIGNUP_VERIFICATION: {
				MessageAttributes Attrs;
				Attrs[RECIPIENT_EMAIL] = UInfo.email;
				Attrs[LOGO] = GetSubLogoAssetURI();
				Attrs[SUBJECT] = "Signup e-mail Address Verification";
				Attrs[ACTION_LINK] = MicroServiceGetPublicAPIEndPoint() +
									 "/actionLink?action=signup_verification&id=" + LinkId;
				Attrs[ACTION_LINK_HTML] =
					"/api/v1/actionLink?action=signup_verification&id=" + LinkId;
				SMTPMailerService()->AddSubVars(Attrs);
				SMTPMailerService()->SendMessage(
					UInfo.email,
					MessagingTemplates::TemplateName(MessagingTemplates::SUB_SIGNUP_VERIFICATION,
													 OperatorName),
					Attrs, true);
				UInfo.waitingForEmailCheck = true;
			} break;

			default:
				break;
			}
			return true;
		}
		return false;
	}

	bool AuthService::VerifyEmail(SecurityObjects::UserInfo &UInfo) {
		SecurityObjects::ActionLink A;

		A.action = OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL;
		A.userId = UInfo.id;
		A.id = MicroServiceCreateUUID();
		A.created = Utils::Now();
		A.expires = A.created + 24 * 60 * 60;
		A.userAction = true;
		StorageService()->ActionLinksDB().CreateAction(A);
		UInfo.waitingForEmailCheck = true;
		UInfo.validated = false;
		return true;
	}

	bool AuthService::VerifySubEmail(SecurityObjects::UserInfo &UInfo) {
		SecurityObjects::ActionLink A;

		A.action = OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL;
		A.userId = UInfo.id;
		A.id = MicroServiceCreateUUID();
		A.created = Utils::Now();
		A.expires = A.created + 24 * 60 * 60;
		A.userAction = false;
		StorageService()->ActionLinksDB().CreateAction(A);
		UInfo.waitingForEmailCheck = true;
		UInfo.validated = false;
		return true;
	}

	bool AuthService::IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken,
								   SecurityObjects::UserInfo &UserInfo, bool &Expired) {

		std::lock_guard G(Mutex_);
		Expired = false;

		std::string TToken{Token}, UserId;
		SecurityObjects::WebToken WT;
		uint64_t RevocationDate = 0;
		if (StorageService()->UserTokenDB().GetToken(TToken, WT, UserId, RevocationDate)) {
			if (RevocationDate != 0)
				return false;
			Expired = (WT.created_ + WT.expires_in_) < Utils::Now();
			if (StorageService()->UserDB().GetUserById(UserId, UserInfo)) {
				WebToken = WT;
				return true;
			}
		}
		return false;
	}

	bool AuthService::IsValidSubToken(const std::string &Token, SecurityObjects::WebToken &WebToken,
									  SecurityObjects::UserInfo &UserInfo, bool &Expired) {
		std::lock_guard G(Mutex_);
		Expired = false;

		std::string TToken{Token}, UserId;
		SecurityObjects::WebToken WT;
		uint64_t RevocationDate = 0;
		if (StorageService()->SubTokenDB().GetToken(TToken, WT, UserId, RevocationDate)) {
			if (RevocationDate != 0)
				return false;
			Expired = (WT.created_ + WT.expires_in_) < Utils::Now();
			if (StorageService()->SubDB().GetUserById(UserId, UserInfo)) {
				WebToken = WT;
				return true;
			}
		}
		return false;
	}

	bool AuthService::IsValidApiKey(const std::string &ApiKey, SecurityObjects::WebToken &WebToken,
									SecurityObjects::UserInfo &UserInfo, bool &Expired,
									std::uint64_t &expiresOn, bool &Suspended) {

		std::lock_guard G(Mutex_);

		Suspended = false;
		std::string UserId;
		SecurityObjects::WebToken WT;
		SecurityObjects::ApiKeyEntry ApiKeyEntry;
		if (StorageService()->ApiKeyDB().GetRecord("apiKey", ApiKey, ApiKeyEntry)) {
			expiresOn = ApiKeyEntry.expiresOn;
			Expired = ApiKeyEntry.expiresOn < Utils::Now();
			if (Expired)
				return false;
			if (StorageService()->UserDB().GetUserById(ApiKeyEntry.userUuid, UserInfo)) {
				if (UserInfo.suspended) {
					Suspended = true;
					return false;
				}
				WebToken = WT;
				ApiKeyEntry.lastUse = Utils::Now();
				StorageService()->ApiKeyDB().UpdateRecord("id", ApiKeyEntry.id, ApiKeyEntry);
				return true;
			}
		}
		return false;
	}

} // namespace OpenWifi
