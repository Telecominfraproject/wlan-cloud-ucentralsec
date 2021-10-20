//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <memory>

#include "Poco/URI.h"

#include "RESTAPI_server.h"
#include "RESTAPI_oauth2Handler.h"
#include "../framework/RESTAPI_system_command.h"
#include "RESTAPI_user_handler.h"
#include "RESTAPI_users_handler.h"
#include "RESTAPI_action_links.h"
#include "RESTAPI_systemEndpoints_handler.h"
#include "RESTAPI_AssetServer.h"
#include "RESTAPI_avatarHandler.h"
#include "RESTAPI_email_handler.h"
#include "RESTAPI_sms_handler.h"

#include "../Daemon.h"
#include "../framework/Utils.h"

namespace OpenWifi {

    class RESTAPI_Server *RESTAPI_Server::instance_ = nullptr;

    int RESTAPI_Server::Start() {
        Logger_.information("Starting.");
        Server_.InitLogging();

        AsserDir_ = Daemon()->ConfigPath("openwifi.restapi.wwwassets");
        AccessPolicy_ = Daemon()->ConfigGetString("openwifi.document.policy.access", "/wwwassets/access_policy.html");
        PasswordPolicy_ = Daemon()->ConfigGetString("openwifi.document.policy.password", "/wwwassets/password_policy.html");

        for(const auto & Svr: ConfigServersList_) {
			Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
											 Svr.KeyFile(),Svr.CertFile()));

            auto Sock{Svr.CreateSecureSocket(Logger_)};

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(50);
            Params->setMaxQueued(200);
			Params->setKeepAlive(true);

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory(Server_), Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }
        return 0;
    }

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {
        Poco::URI uri(Request.getURI());
        const auto & Path = uri.getPath();
        RESTAPIHandler::BindingMap Bindings;
        return RESTAPI_Router<
                RESTAPI_oauth2Handler,
                RESTAPI_users_handler,
                RESTAPI_user_handler,
                RESTAPI_system_command,
                RESTAPI_AssetServer,
                RESTAPI_systemEndpoints_handler,
                RESTAPI_action_links,
                RESTAPI_avatarHandler,
                RESTAPI_email_handler,
                RESTAPI_sms_handler
                >(Path,Bindings,Logger_,Server_);
    }

    void RESTAPI_Server::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
        RESTServers_.clear();
    }

    void RESTAPI_Server::reinitialize(Poco::Util::Application &self) {
        Daemon()->LoadConfigurationFile();
        Logger_.information("Reinitializing.");
        Stop();
        Start();
    }

}  // namespace
