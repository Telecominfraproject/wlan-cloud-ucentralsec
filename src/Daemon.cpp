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

#include <cstdlib>
#include <boost/algorithm/string.hpp>

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Environment.h"

#include "Daemon.h"

#include "ALBHealthCheckServer.h"
#include "KafkaManager.h"
#include "StorageService.h"
#include "RESTAPI_server.h"
#include "SMTPMailerService.h"


namespace uCentral {
    class Daemon *Daemon::instance_ = nullptr;

    class Daemon *Daemon::instance() {
        if (instance_ == nullptr) {
            instance_ = new Daemon(vDAEMON_PROPERTIES_FILENAME,
                                   vDAEMON_ROOT_ENV_VAR,
                                   vDAEMON_CONFIG_ENV_VAR,
                                   vDAEMON_APP_NAME,
                                   Types::SubSystemVec{
                                           Storage(),
                                           RESTAPI_Server(),
                                           SMTPMailerService()
                                   });
        }
        return instance_;
    }

    void Daemon::initialize(Poco::Util::Application &self) {
        MicroService::initialize(*this);
    }
}

int main(int argc, char **argv) {
    try {
        auto App = uCentral::Daemon::instance();
        auto ExitCode =  App->run(argc, argv);
        delete App;

        return ExitCode;

    } catch (Poco::Exception &exc) {
        std::cerr << exc.displayText() << std::endl;
        return Poco::Util::Application::EXIT_SOFTWARE;
    }
}


// end of namespace
