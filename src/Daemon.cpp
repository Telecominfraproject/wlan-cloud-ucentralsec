//
// Created by stephane bourque on 2021-06-10.
//

//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Environment.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"

#include "Daemon.h"

#include <aws/core/Aws.h>
#include <aws/s3/model/AccessControlPolicy.h>

#include "ActionLinkManager.h"
#include "AuthService.h"
#include "SMSSender.h"
#include "SMTPMailerService.h"
#include "StorageService.h"
#include "TotpCache.h"
#include "framework/RESTAPI_RateLimiter.h"
#include "framework/UI_WebSocketClientServer.h"
#include <SecretStore.h>

namespace OpenWifi {
	class Daemon *Daemon::instance_ = nullptr;

	class Daemon *Daemon::instance() {
		if (instance_ == nullptr) {
			instance_ =
				new Daemon(vDAEMON_PROPERTIES_FILENAME, vDAEMON_ROOT_ENV_VAR,
						   vDAEMON_CONFIG_ENV_VAR, vDAEMON_APP_NAME, vDAEMON_BUS_TIMER,
						   SubSystemVec{StorageService(), SMSSender(), AuthService(), ActionLinkManager(),
										SMTPMailerService(), RESTAPI_RateLimiter(), TotpCache(),
										UI_WebSocketClientServer(), SecretStore()});
		}
		return instance_;
	}

	void Daemon::PostInitialization([[maybe_unused]] Poco::Util::Application &self) {
		AssetDir_ = MicroService::instance().ConfigPath("openwifi.restapi.wwwassets");
	}

	void DaemonPostInitialization(Poco::Util::Application &self) {
		Daemon()->PostInitialization(self);
	}
} // namespace OpenWifi

int main(int argc, char **argv) {
	try {
		SSL_library_init();
		Aws::SDKOptions AwsOptions;
		AwsOptions.memoryManagementOptions.memoryManager = nullptr;
		AwsOptions.cryptoOptions.initAndCleanupOpenSSL = false;
		AwsOptions.httpOptions.initAndCleanupCurl = true;

		Aws::InitAPI(AwsOptions);

		int ExitCode = 0;
		{
			auto App = OpenWifi::Daemon::instance();
			ExitCode = App->run(argc, argv);
		}
		ShutdownAPI(AwsOptions);
		return ExitCode;
	} catch (Poco::Exception &exc) {
		std::cout << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace
