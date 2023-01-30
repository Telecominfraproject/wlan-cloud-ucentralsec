//
// Created by stephane bourque on 2023-01-25.
//

#include <fstream>
#include "SecretStore.h"
#include <framework/MicroServiceFuncs.h>
#include <Poco/File.h>
#include <Poco/JSON/Parser.h>

namespace OpenWifi {

    int SecretStore::Start() {
        std::lock_guard G(Mutex_);
        ReadStore();
        return 0;
    }

    void SecretStore::Stop() {
        std::lock_guard G(Mutex_);
        SaveStore();
    }

    void SecretStore::ReadStore() {
        Poco::File  StoreFileName(MicroServiceDataDirectory() + "/secrets.json");
        if(StoreFileName.exists() && StoreFileName.isFile()) {
            try {
                std::ostringstream  OS;
                std::ifstream       IF(StoreFileName.path().c_str());
                Poco::StreamCopier::copyStream(IF, OS);
                Poco::JSON::Parser  P;
                auto Doc = P.parse(OS.str()).extract<Poco::JSON::Object::Ptr>();
                if(Doc->isArray("secrets")) {
                    auto Secrets = Doc->getArray("secrets");
                    for(const auto &secret:*Secrets) {
                        const auto &entry = secret.extract<Poco::JSON::Object::Ptr>();
                        if(entry->has("key") && entry->has("value")) {
                            Store_[entry->get("key")] = entry->get("value").toString();
                        }
                    }
                }
            } catch (const Poco::Exception &E) {
                Logger().log(E);
            }
        }
    }

    void SecretStore::SaveStore() {
        Poco::JSON::Object  StoreJSON;
        Poco::JSON::Array   Secrets;

        for(const auto &[key,value]:Store_) {
            Poco::JSON::Object  Entry;
            Entry.set("key", key);
            Entry.set("value", value);
            Secrets.add(Entry);
        }

        StoreJSON.set("secrets",Secrets);
        Poco::File  StoreFileName(MicroServiceDataDirectory() + "/secrets.json");
        std::ofstream   OF(StoreFileName.path(),std::ios_base::trunc);
        StoreJSON.stringify(OF);
    }

    bool SecretStore::Get(const std::string & key, std::string & value, const std::string & default_value) {
        std::lock_guard G(Mutex_);

        auto It = Store_.find(key);
        if(It!=end(Store_)) {
            value = It->second;
            return true;
        } else {
            value = default_value;
            return false;
        }
    }

    void SecretStore::Set(const std::string & key, const std::string & value ) {
        std::lock_guard G(Mutex_);

        Store_[key] = value;
        SaveStore();
    }

    void SecretStore::Remove(const std::string & key) {
        std::lock_guard G(Mutex_);

        Store_.erase(key);
        SaveStore();
    }

} // OpenWifi