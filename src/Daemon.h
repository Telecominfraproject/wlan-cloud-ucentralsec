//
// Created by stephane bourque on 2021-06-10.
//

#pragma once

#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

#include "framework/MicroServiceNames.h"
#include "framework/MicroService.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"


namespace OpenWifi {

    [[maybe_unused]] static const char * vDAEMON_PROPERTIES_FILENAME = "owsec.properties";
    [[maybe_unused]] static const char * vDAEMON_ROOT_ENV_VAR = "OWSEC_ROOT";
    [[maybe_unused]] static const char * vDAEMON_CONFIG_ENV_VAR = "OWSEC_CONFIG";
    [[maybe_unused]] static const char * vDAEMON_APP_NAME = uSERVICE_SECURITY.c_str();
    [[maybe_unused]] static const uint64_t vDAEMON_BUS_TIMER = 5000;

    class Daemon : public MicroService {
    public:
        explicit Daemon(const std::string & PropFile,
                        const std::string & RootEnv,
                        const std::string & ConfigEnv,
                        const std::string & AppName,
                        uint64_t BusTimer,
                        const SubSystemVec & SubSystems) :
                MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

        void PostInitialization(Poco::Util::Application &self);
        static Daemon *instance();
        inline const std::string & AssetDir() { return AssetDir_; }
    private:
        static Daemon 		*instance_;
        std::string         AssetDir_;
    };

    inline Daemon * Daemon() { return Daemon::instance(); }
    void DaemonPostInitialization(Poco::Util::Application &self);
}

