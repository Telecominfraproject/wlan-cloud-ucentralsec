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


#include "OpenWifiTypes.h"
#include "MicroService.h"

namespace OpenWifi {

    static const char * vDAEMON_PROPERTIES_FILENAME = "ucentralsec.properties";
    static const char * vDAEMON_ROOT_ENV_VAR = "UCENTRALSEC_ROOT";
    static const char * vDAEMON_CONFIG_ENV_VAR = "UCENTRALSEC_CONFIG";
    static const char * vDAEMON_APP_NAME = uSERVICE_SECURITY.c_str();
    static const uint64_t vDAEMON_BUS_TIMER = 5000;

    class Daemon : public MicroService {
    public:
        explicit Daemon(std::string PropFile,
                        std::string RootEnv,
                        std::string ConfigEnv,
                        std::string AppName,
                        uint64_t BusTimer,
                        Types::SubSystemVec SubSystems) :
                MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

        void initialize(Poco::Util::Application &self);
        static Daemon *instance();
    private:
        static Daemon 				*instance_;
    };

    inline Daemon * Daemon() { return Daemon::instance(); }
}

#endif //UCENTRALSEC_DAEMON_H
