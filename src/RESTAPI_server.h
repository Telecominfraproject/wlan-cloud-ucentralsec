//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCENTRALRESTAPISERVER_H
#define UCENTRAL_UCENTRALRESTAPISERVER_H

#include "SubSystemServer.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/NetException.h"

namespace uCentral {

    class RESTAPI_Server : public SubSystemServer {

    public:
        RESTAPI_Server() noexcept;

        static RESTAPI_Server *instance() {
            if (instance_ == nullptr) {
                instance_ = new RESTAPI_Server;
            }
            return instance_;
        }

        int Start() override;
        void Stop() override;

    private:
		static RESTAPI_Server *instance_;
        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
		Poco::ThreadPool	Pool_;
    };

    inline RESTAPI_Server * RESTAPI_Server() { return RESTAPI_Server::instance(); };

    class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
        public:
            RequestHandlerFactory() :
                Logger_(RESTAPI_Server()->Logger()){}

            Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
        private:
            Poco::Logger    & Logger_;
    };


} //   namespace

#endif //UCENTRAL_UCENTRALRESTAPISERVER_H
