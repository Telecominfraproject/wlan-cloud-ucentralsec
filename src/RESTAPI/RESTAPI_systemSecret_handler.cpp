//
// Created by stephane bourque on 2023-01-25.
//

#include "RESTAPI_systemSecret_handler.h"
#include <SecretStore.h>

namespace OpenWifi {

    void RESTAPI_systemSecret_handler::DoGet() {
        if(!Internal_ && UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

        if(GetBoolParameter("all")) {
            auto Store = SecretStore()->Store();
            Poco::JSON::Array   Entries;
            Poco::JSON::Object  List;

            for(const auto &[Key,Value]:Store) {
                Poco::JSON::Object  E;
                E.set("key",Key);
                E.set("value",Value);
                Entries.add(E);
            }
            List.set("secrets",Entries);
            return ReturnObject(List);
        }

        if(GetBoolParameter("dictionary")) {
            static std::vector<std::pair<std::string,std::string>> KnownKeys =
                    {
                            { "google.maps.apikey" , "A Google Key specific for the Google MAPS API."},
                            { "iptocountry.ipinfo.token", "IPInfo.io service token."},
                            { "iptocountry.ipdata.apikey", "IPData.co API Key."},
                            { "iptocountry.ip2location.apikey", "IP2Location.com API Key"}
                    };

            Poco::JSON::Object  Answer;
            Poco::JSON::Array   Entries;
            for(const auto &[key,description]:KnownKeys) {
                Poco::JSON::Object  E;
                E.set("key",key);
                E.set("description",description);
                Entries.add(E);
            }
            Answer.set("knownKeys", Entries);
            return ReturnObject(Answer);
        }

        auto Key = GetBinding("secret");
        if(Key.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        std::string Value;
        if(SecretStore()->Get(Key,Value,"")) {
            Poco::JSON::Object  Answer;
            Answer.set("key", Key);
            Answer.set("value", Value);
            return ReturnObject(Answer);
        }
        return NotFound();
    }

    void RESTAPI_systemSecret_handler::DoDelete() {
        if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

        auto Key = GetBinding("secret");
        if(Key.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        SecretStore()->Remove(Key);
        return OK();
    }

    void RESTAPI_systemSecret_handler::DoPut() {
        if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
            return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
        }

        auto Key = GetBinding("secret");
        auto Value = GetParameter("value","_______no_value_____");
        if(Key.empty() || Value == "_______no_value_____") {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        SecretStore()->Set(Key,Value);
        Poco::JSON::Object  Answer;
        Answer.set("key", Key);
        Answer.set("value", Value);
        return ReturnObject(Answer);
    }

} // OpenWifi