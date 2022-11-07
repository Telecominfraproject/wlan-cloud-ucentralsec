//
// Created by stephane bourque on 2022-11-07.
//

#include "RESTAPI_validate_apikey.h"
#include "AuthService.h"

namespace OpenWifi {

    void RESTAPI_validate_apikey::DoGet() {
        Poco::URI URI(Request->getURI());
        auto Parameters = URI.getQueryParameters();
        std::cout << __LINE__ << std::endl;
        for(auto const &i:Parameters) {
            std::cout << __LINE__ << std::endl;
            if (i.first == "apikey") {
                std::cout << __LINE__ << std::endl;
                //  can we find this token?
                SecurityObjects::UserInfoAndPolicy SecObj;
                bool Expired = false;
                std::uint64_t expiresOn=0;
                std::cout << __LINE__ << " > " << i.second << std::endl;
                if (AuthService()->IsValidApiKey(i.second, SecObj.webtoken, SecObj.userinfo, Expired, expiresOn)) {
                    std::cout << __LINE__ << std::endl;
                    Poco::JSON::Object Answer;
                    SecObj.to_json(Answer);
                    Answer.set("expiresOn", expiresOn);
                    std::cout << __LINE__ << std::endl;
                    return ReturnObject(Answer);
                }
                std::cout << __LINE__ << std::endl;
                return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
            }
            std::cout << __LINE__ << std::endl;
        }
        std::cout << __LINE__ << std::endl;
        return NotFound();
    }

} // OpenWifi