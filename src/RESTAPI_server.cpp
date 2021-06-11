//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/URI.h"

#include "RESTAPI_server.h"
#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_unknownRequestHandler.h"

#include "uUtils.h"

namespace uCentral::RESTAPI {

    Service *Service::instance_ = nullptr;

    int Start() {
        return Service::instance()->Start();
    }

    void Stop() {
        Service::instance()->Stop();
    }

    Service::Service() noexcept: SubSystemServer("RESTAPIServer", "RESTAPIServer", "ucentral.restapi")
    {
    }

    int Service::Start() {
        Logger_.information("Starting.");

        for(const auto & Svr: ConfigServersList_) {
			Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
											 Svr.KeyFile(),Svr.CertFile()));

            auto Sock{Svr.CreateSecureSocket(Logger_)};

//			Sock.setReceiveTimeout(Poco::Timespan(10,0));

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(50);
            Params->setMaxQueued(200);
			Params->setKeepAlive(true);
//			uint64_t T = 45000;
//			Params->setKeepAliveTimeout(T);
//			Params->setMaxKeepAliveRequests(200);
//			Params->setTimeout();

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory, Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

        Logger_.debug(Poco::format("REQUEST(%s): %s %s", uCentral::Utils::FormatIPv6(Request.clientAddress().toString()), Request.getMethod(), Request.getURI()));

        Poco::URI uri(Request.getURI());
        const auto & Path = uri.getPath();
        RESTAPIHandler::BindingMap bindings;

        if (RESTAPIHandler::ParseBindings(Path, "/api/v1/oauth2/{token}", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(Path, "/api/v1/oauth2", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger_);
        }

		Logger_.error(Poco::format("INVALID-API-ENDPOINT: %s",Path));
        return new RESTAPI_UnknownRequestHandler(bindings,Logger_);
    }

    void Service::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
    }

}  // namespace