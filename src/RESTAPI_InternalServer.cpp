//
// Created by stephane bourque on 2021-06-29.
//


#include "Poco/URI.h"

#include "RESTAPI_system_command.h"
#include "RESTAPI_user_handler.h"
#include "RESTAPI_users_handler.h"
#include "RESTAPI_action_links.h"
#include "RESTAPI_validateToken_handler.h"
#include "RESTAPI_InternalServer.h"
#include "RESTAPI_sms_handler.h"

#include "Utils.h"

namespace OpenWifi {

    class RESTAPI_InternalServer *RESTAPI_InternalServer::instance_ = nullptr;

    RESTAPI_InternalServer::RESTAPI_InternalServer() noexcept:
        SubSystemServer("RESTAPIInternalServer", "REST-ISRV", "openwifi.internal.restapi")
    {
    }

    int RESTAPI_InternalServer::Start() {
        Logger_.information("Starting.");
        Server_.InitLogging();

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

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new InternalRequestHandlerFactory(Server_), Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    void RESTAPI_InternalServer::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
        RESTServers_.clear();
    }

    void RESTAPI_InternalServer::reinitialize(Poco::Util::Application &self) {
        Daemon()->LoadConfigurationFile();
        Logger_.information("Reinitializing.");
        Stop();
        Start();
    }

    Poco::Net::HTTPRequestHandler *InternalRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {
        Poco::URI uri(Request.getURI());
        const auto & Path = uri.getPath();
        RESTAPIHandler::BindingMap Bindings;
        return RESTAPI_Router_I<
                RESTAPI_users_handler,
                RESTAPI_user_handler,
                RESTAPI_system_command,
                RESTAPI_action_links,
                RESTAPI_validateToken_handler,
                RESTAPI_sms_handler
        >(Path,Bindings,Logger_, Server_);
    }

}