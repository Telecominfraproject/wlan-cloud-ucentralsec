//
// Created by stephane bourque on 2021-06-10.
//

#ifndef UCENTRALSEC_DAEMON_H
#define UCENTRALSEC_DAEMON_H

#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"


#include "uCentralTypes.h"
#include "MicroService.h"

namespace uCentral {

    static const char * vDAEMON_PROPERTIES_FILENAME = "ucentralsec.properties";
    static const char * vDAEMON_ROOT_ENV_VAR = "UCENTRALSEC_ROOT";
    static const char * vDAEMON_CONFIG_ENV_VAR = "UCENTRALSEC_CONFIG";
    static const char * vDAEMON_APP_NAME = "uCentralSec";
    class Daemon : public MicroService {
    public:
        explicit Daemon(std::string PropFile,
                        std::string RootEnv,
                        std::string ConfigEnv,
                        std::string AppName,
                        Types::SubSystemVec SubSystems) :
                MicroService( PropFile, RootEnv, ConfigEnv, AppName, SubSystems) {};

        bool AutoProvisioning() const { return AutoProvisioning_ ; }
        [[nodiscard]] std::string IdentifyDevice(const std::string & Compatible) const;
        void initialize(Poco::Util::Application &self);
        static Daemon *instance();
    private:
        static Daemon 				*instance_;
        bool                        AutoProvisioning_ = false;
        Types::StringMapStringSet   DeviceTypeIdentifications_;
    };

    inline Daemon * Daemon() { return Daemon::instance(); }
}

#endif //UCENTRALSEC_DAEMON_H
