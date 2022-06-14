//
// Created by stephane bourque on 2022-01-31.
//

#include "RESTAPI_totp_handler.h"
#include "TotpCache.h"

namespace OpenWifi {

    void RESTAPI_totp_handler::DoGet() {

        auto Reset = GetBoolParameter("reset",false);
        std::string QRCode;
        std::cout << __LINE__ << std::endl;

        if(TotpCache()->StartValidation(UserInfo_.userinfo,false,QRCode,Reset)) {
            std::cout << __LINE__ << std::endl;
            return SendFileContent(QRCode, "image/svg+xml","qrcode.svg");
        }
        std::cout << __LINE__ << std::endl;
        return BadRequest(RESTAPI::Errors::InvalidCommand);
    }

    void RESTAPI_totp_handler::DoPut() {
        std::cout << __LINE__ << std::endl;
        auto Value = GetParameter("value","");
        std::cout << __LINE__ << std::endl;
        auto nextIndex = GetParameter("index",0);
        std::cout << __LINE__ << std::endl;
        bool moreCodes=false;

        std::cout << __LINE__ << std::endl;
        RESTAPI::Errors::msg Err;
        std::cout << __LINE__ << std::endl;
        if(TotpCache()->ContinueValidation(UserInfo_.userinfo,false,Value,nextIndex,moreCodes, Err)) {
            Poco::JSON::Object Answer;
            Answer.set("nextIndex", nextIndex);
            Answer.set("moreCodes", moreCodes);
            std::cout << __LINE__ << std::endl;
            return ReturnObject(Answer);
        }
        std::cout << __LINE__ << std::endl;
        return BadRequest(Err);
    }

}
