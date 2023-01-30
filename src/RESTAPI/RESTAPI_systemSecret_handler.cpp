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