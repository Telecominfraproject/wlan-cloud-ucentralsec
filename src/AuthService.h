//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <regex>

#include "Poco/Crypto/DigestEngine.h"
#include "Poco/ExpireLRUCache.h"
#include "Poco/HMACEngine.h"
#include "Poco/JSON/Object.h"
#include "Poco/JWT/Signer.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/SHA2Engine.h"
#include "framework/SubSystemServer.h"

#include "framework/MicroServiceFuncs.h"
#include "framework/ow_constants.h"

#include "MessagingTemplates.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

	static const std::string AUTHENTICATION_SYSTEM{"SYSTEM"};

	class AuthService : public SubSystemServer {
	  public:
		enum ACCESS_TYPE { USERNAME, SERVER, CUSTOM };

		static ACCESS_TYPE IntToAccessType(int C);
		static int AccessTypeToInt(ACCESS_TYPE T);

		static auto instance() {
			static auto instance_ = new AuthService;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		[[nodiscard]] bool IsAuthorized(Poco::Net::HTTPServerRequest &Request,
										std::string &SessionToken,
										SecurityObjects::UserInfoAndPolicy &UInfo,
										std::uint64_t TID, bool &Expired);
		[[nodiscard]] bool IsAuthorized(const std::string &SessionToken,
										SecurityObjects::UserInfoAndPolicy &UInfo,
										std::uint64_t TID, bool &Expired);

		[[nodiscard]] UNAUTHORIZED_REASON Authorize(std::string &UserName,
													const std::string &Password,
													const std::string &NewPassword,
													SecurityObjects::UserInfoAndPolicy &UInfo,
													bool &Expired);
		void CreateToken(const std::string &UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
		[[nodiscard]] bool SetPassword(const std::string &Password,
									   SecurityObjects::UserInfo &UInfo);
		[[nodiscard]] const std::string &PasswordValidationExpression() const {
			return PasswordValidationStr_;
		};
		void Logout(const std::string &token, bool EraseFromCache = true);

		[[nodiscard]] bool IsSubAuthorized(Poco::Net::HTTPServerRequest &Request,
										   std::string &SessionToken,
										   SecurityObjects::UserInfoAndPolicy &UInfo,
										   std::uint64_t TID, bool &Expired);
		[[nodiscard]] UNAUTHORIZED_REASON AuthorizeSub(std::string &UserName,
													   const std::string &Password,
													   const std::string &NewPassword,
													   SecurityObjects::UserInfoAndPolicy &UInfo,
													   bool &Expired);

		void CreateSubToken(const std::string &UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
		[[nodiscard]] bool SetSubPassword(const std::string &Password,
										  SecurityObjects::UserInfo &UInfo);
		[[nodiscard]] const std::string &SubPasswordValidationExpression() const {
			return PasswordValidationStr_;
		};
		void SubLogout(const std::string &token, bool EraseFromCache = true);

		void RemoveTokenSystemWide(const std::string &token);

		bool ValidatePassword(const std::string &pwd);
		bool ValidateSubPassword(const std::string &pwd);

		[[nodiscard]] bool IsValidToken(const std::string &Token,
										SecurityObjects::WebToken &WebToken,
										SecurityObjects::UserInfo &UserInfo, bool &Expired);
		[[nodiscard]] bool IsValidSubToken(const std::string &Token,
										   SecurityObjects::WebToken &WebToken,
										   SecurityObjects::UserInfo &UserInfo, bool &Expired);
		[[nodiscard]] std::string GenerateTokenJWT(const std::string &UserName, ACCESS_TYPE Type);
		[[nodiscard]] std::string GenerateTokenHMAC(const std::string &UserName, ACCESS_TYPE Type);

		[[nodiscard]] bool IsValidApiKey(const std::string &ApiKey,
										 SecurityObjects::WebToken &WebToken,
										 SecurityObjects::UserInfo &UserInfo, bool &Expired,
										 std::uint64_t &expiresOn, bool &Suspended);
		[[nodiscard]] std::string ComputeNewPasswordHash(const std::string &UserName,
														 const std::string &Password);
		[[nodiscard]] bool ValidatePasswordHash(const std::string &UserName,
												const std::string &Password,
												const std::string &StoredPassword);
		[[nodiscard]] bool ValidateSubPasswordHash(const std::string &UserName,
												   const std::string &Password,
												   const std::string &StoredPassword);

		[[nodiscard]] bool UpdatePassword(const std::string &Admin, const std::string &UserName,
										  const std::string &OldPassword,
										  const std::string &NewPassword);
		[[nodiscard]] std::string ResetPassword(const std::string &Admin,
												const std::string &UserName);

		[[nodiscard]] bool UpdateSubPassword(const std::string &Admin, const std::string &UserName,
											 const std::string &OldPassword,
											 const std::string &NewPassword);
		[[nodiscard]] std::string ResetSubPassword(const std::string &Admin,
												   const std::string &UserName);

		[[nodiscard]] static bool VerifyEmail(SecurityObjects::UserInfo &UInfo);
		[[nodiscard]] static bool VerifySubEmail(SecurityObjects::UserInfo &UInfo);

		[[nodiscard]] bool SendEmailToUser(const std::string &LinkId, std::string &Email,
										   MessagingTemplates::EMAIL_REASON Reason);
		[[nodiscard]] bool SendEmailToSubUser(const std::string &LinkId, std::string &Email,
											  MessagingTemplates::EMAIL_REASON Reason,
											  const std::string &OperatorName);
		[[nodiscard]] bool RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo);

		[[nodiscard]] bool SendEmailChallengeCode(const SecurityObjects::UserInfoAndPolicy &UInfo,
												  const std::string &code);

		bool DeleteUserFromCache(const std::string &UserName);
		bool DeleteSubUserFromCache(const std::string &UserName);
		void RevokeToken(std::string &Token);
		void RevokeSubToken(std::string &Token);

		[[nodiscard]] static inline const std::string GetLogoAssetURI() {
			return MicroServicePublicEndPoint() + "/wwwassets/logo.png";
		}

		[[nodiscard]] static inline const std::string GetLogoAssetFileName() {
			return MicroServiceWWWAssetsDir() + "/logo.png";
		}

		[[nodiscard]] static inline const std::string GetSubLogoAssetURI() {
			return MicroServicePublicEndPoint() + "/wwwassets/sub_logo.png";
		}

		[[nodiscard]] static inline const std::string GetSubLogoAssetFileName() {
			return MicroServiceWWWAssetsDir() + "/sub_logo.png";
		}

		inline const std::string &GetPasswordPolicy() const { return PasswordPolicy_; }
		inline const std::string &GetAccessPolicy() const { return AccessPolicy_; }

		inline const std::string &GetSubPasswordPolicy() const { return SubPasswordPolicy_; }
		inline const std::string &GetSubAccessPolicy() const { return SubAccessPolicy_; }

		bool RefreshUserToken(Poco::Net::HTTPServerRequest &Request,
							  const std::string &RefreshToken,
							  SecurityObjects::UserInfoAndPolicy &UI);
		bool RefreshSubToken(Poco::Net::HTTPServerRequest &Request, const std::string &RefreshToken,
							 SecurityObjects::UserInfoAndPolicy &UI);

		[[nodiscard]] inline auto HelperEmail() const { return HelperEmail_; };
		[[nodiscard]] inline auto SubHelperEmail() const { return SubHelperEmail_; };
		[[nodiscard]] inline auto GlobalHelperEmail() const { return GlobalHelperEmail_; };
		[[nodiscard]] inline auto GlobalSubHelperEmail() const { return GlobalSubHelperEmail_; };
		[[nodiscard]] inline auto HelperSite() const { return HelperSite_; };
		[[nodiscard]] inline auto SubHelperSite() const { return SubHelperSite_; };
		[[nodiscard]] inline auto SystemLoginSite() const { return SystemLoginSite_; };
		[[nodiscard]] inline auto SubSystemLoginSite() const { return SubSystemLoginSite_; };
		[[nodiscard]] inline auto UserSignature() const { return UserSignature_; };
		[[nodiscard]] inline auto SubSignature() const { return SubSignature_; };

	  private:
		Poco::SHA2Engine SHA2_;

		std::string AccessPolicy_;
		std::string PasswordPolicy_;
		std::string SubAccessPolicy_;
		std::string SubPasswordPolicy_;
		std::string PasswordValidationStr_;
		std::string SubPasswordValidationStr_;
		std::regex PasswordValidation_;
		std::regex SubPasswordValidation_;

		uint64_t TokenAging_ = 15 * 24 * 60 * 60;
		uint64_t HowManyOldPassword_ = 5;
		uint64_t RefreshTokenLifeSpan_ = 90 * 24 * 60 * 60;

		std::string HelperEmail_;
		std::string SubHelperEmail_;
		std::string GlobalHelperEmail_;
		std::string GlobalSubHelperEmail_;
		std::string HelperSite_;
		std::string SubHelperSite_;
		std::string SystemLoginSite_;
		std::string SubSystemLoginSite_;
		std::string UserSignature_;
		std::string SubSignature_;

		class SHA256Engine : public Poco::Crypto::DigestEngine {
		  public:
			enum { BLOCK_SIZE = 64, DIGEST_SIZE = 32 };

			SHA256Engine() : DigestEngine("SHA256") {}
		};

		Poco::HMACEngine<SHA256Engine> HMAC_{"tipopenwifi"};

		AuthService() noexcept : SubSystemServer("Authentication", "AUTH-SVR", "authentication") {}
	};

	inline auto AuthService() { return AuthService::instance(); }

	[[nodiscard]] inline bool AuthServiceIsAuthorized(Poco::Net::HTTPServerRequest &Request,
													  std::string &SessionToken,
													  SecurityObjects::UserInfoAndPolicy &UInfo,
													  std::uint64_t TID, bool &Expired, bool Sub) {
		if (Sub)
			return AuthService()->IsSubAuthorized(Request, SessionToken, UInfo, TID, Expired);
		else
			return AuthService()->IsAuthorized(Request, SessionToken, UInfo, TID, Expired);
	}

} // namespace OpenWifi
