//
// Created by stephane bourque on 2021-10-11.
//

#pragma once

#include "Poco/JSON/Object.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/SubSystemServer.h"

#include "fmt/format.h"

namespace OpenWifi {

	namespace MFAMETHODS {
		inline const static std::string SMS{"sms"};
		inline const static std::string EMAIL{"email"};
		inline const static std::string AUTHENTICATOR{"authenticator"};
		inline const static std::vector<std::string> Methods{SMS, EMAIL, AUTHENTICATOR};
		inline bool Validate(const std::string &M) {
			return std::find(cbegin(Methods), cend(Methods), M) != Methods.end();
		}
	} // namespace MFAMETHODS

	struct MFACacheEntry {
		SecurityObjects::UserInfoAndPolicy UInfo;
		std::string Answer;
		uint64_t Created;
		std::string Method;
	};

	typedef std::map<std::string, MFACacheEntry> MFAChallengeCache;

	class MFAServer : public SubSystemServer {
	  public:
		int Start() override;
		void Stop() override;
		static auto instance() {
			static auto instance_ = new MFAServer;
			return instance_;
		}

		bool StartMFAChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo,
							   Poco::JSON::Object &Challenge);
		bool CompleteMFAChallenge(const Poco::JSON::Object::Ptr &ChallengeResponse,
								  SecurityObjects::UserInfoAndPolicy &UInfo);
		static bool MethodEnabled(const std::string &Method);
		bool ResendCode(const std::string &uuid);
		static bool SendChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo,
								  const std::string &Method, const std::string &Challenge);

		static inline std::string MakeChallenge() {
			return fmt::format("{0:06}", MicroServiceRandom(1, 999999));
		}

	  private:
		MFAChallengeCache Cache_;
		MFAServer() noexcept : SubSystemServer("MFServer", "MFA-SVR", "mfa") {}

		void CleanCache();
	};

	inline auto MFAServer() { return MFAServer::instance(); }
} // namespace OpenWifi
