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

#include "Utils.h"

namespace uCentral {

    class RESTAPI_InternalServer *RESTAPI_InternalServer::instance_ = nullptr;

    RESTAPI_InternalServer::RESTAPI_InternalServer() noexcept:
        SubSystemServer("RESTAPIInternalServer", "REST-ISRV", "ucentral.internal.restapi")
    {
    }

    int RESTAPI_InternalServer::Start() {
        Logger_.information("Starting.");

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

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new InternalRequestHandlerFactory, Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    void RESTAPI_InternalServer::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
    }

    Poco::Net::HTTPRequestHandler *InternalRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

        Logger_.debug(Poco::format("REQUEST(%s): %s %s",
                                   uCentral::Utils::FormatIPv6(Request.clientAddress().toString()),
                                   Request.getMethod(), Request.getURI()));

        Poco::URI uri(Request.getURI());
        const auto & Path = uri.getPath();
        RESTAPIHandler::BindingMap Bindings;

        std::cout << __LINE__ << std::endl;

        return RESTAPI_Router<
                RESTAPI_users_handler,
                RESTAPI_user_handler,
                RESTAPI_system_command,
                RESTAPI_action_links,
                RESTAPI_validateToken_handler
        >(Path,Bindings,Logger_);
    }

}