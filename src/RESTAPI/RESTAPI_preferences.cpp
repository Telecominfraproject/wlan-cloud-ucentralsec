//
// Created by stephane bourque on 2021-11-16.
//

#include "RESTAPI_preferences.h"
#include "StorageService.h"

namespace OpenWifi {

    void RESTAPI_preferences::DoGet() {
        SecurityObjects::Preferences    P;
        Poco::JSON::Object  Answer;
        StorageService()->GetPreferences(UserInfo_.userinfo.Id, P);
        P.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_preferences::DoPut() {
        SecurityObjects::Preferences    P;
        auto RawObject = ParseStream();

        if(!P.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        P.id = UserInfo_.userinfo.Id;
        P.modified = std::time(nullptr);
        StorageService()->SetPreferences(P);

        Poco::JSON::Object  Answer;
        P.to_json(Answer);
        ReturnObject(Answer);
    }

}