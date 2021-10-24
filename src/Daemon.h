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

#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    static const char * vDAEMON_PROPERTIES_FILENAME = "owsec.properties";
    static const char * vDAEMON_ROOT_ENV_VAR = "OWSEC_ROOT";
    static const char * vDAEMON_CONFIG_ENV_VAR = "OWSEC_CONFIG";
    static const char * vDAEMON_APP_NAME = uSERVICE_SECURITY.c_str();
    static const uint64_t vDAEMON_BUS_TIMER = 5000;

    class Daemon : public MicroService {
    public:
        explicit Daemon(const std::string & PropFile,
                        const std::string & RootEnv,
                        const std::string & ConfigEnv,
                        const std::string & AppName,
                        uint64_t BusTimer,
                        const SubSystemVec & SubSystems) :
                MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

        void initialize(Poco::Util::Application &self) override;
        static Daemon *instance();
        inline const std::string & AssetDir() { return AssetDir_; }
        inline const std::string & GetPasswordPolicy() const { return PasswordPolicy_; }
        inline const std::string & GetAccessPolicy() const { return AccessPolicy_; }
    private:
        static Daemon 		*instance_;
        std::string         AssetDir_;
        std::string         PasswordPolicy_;
        std::string         AccessPolicy_;
    };

    inline Daemon * Daemon() { return Daemon::instance(); }
}

#endif //UCENTRALSEC_DAEMON_H
