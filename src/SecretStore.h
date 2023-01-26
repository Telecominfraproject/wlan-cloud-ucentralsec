//
// Created by stephane bourque on 2023-01-25.
//

#pragma once

#include <framework/SubSystemServer.h>

namespace OpenWifi {

    class SecretStore : public SubSystemServer {
    public:
        static SecretStore *instance() {
            static auto *instance_ = new SecretStore;
            return instance_;
        }

        int  Start() final;
        void Stop() final;

    private:

        SecretStore() noexcept:
                SubSystemServer("SecretStore", "SECRET-SVR", "secret.store")
        {
        }
    };
    inline SecretStore * SecretStore() { return SecretStore::instance(); }

} // OpenWifi
