//
// Created by stephane bourque on 2022-01-31.
//

#include "RESTAPI_subtotp_handler.h"

#include "TotpCache.h"

namespace OpenWifi {

    void RESTAPI_subtotp_handler::DoGet() {

        auto Reset = GetBoolParameter("reset",false);
        std::string QRCode;

        if(TotpCache()->StartValidation(UserInfo_.userinfo,true,QRCode,Reset)) {
            return SendFileContent(QRCode, "image/svg+xml","qrcode.svg");
        }
        return BadRequest(RESTAPI::Errors::InvalidCommand);
    }

    void RESTAPI_subtotp_handler::DoPut() {
        auto Value = GetParameter("value","");
        auto nextIndex = GetParameter("index",0);
        bool moreCodes=false;

        uint64_t ErrorCode = 0;
        std::string ErrorText;
        if(TotpCache()->ContinueValidation(UserInfo_.userinfo,true,Value,nextIndex,moreCodes, ErrorCode, ErrorText )) {
            Poco::JSON::Object Answer;
            Answer.set("nextIndex", nextIndex);
            Answer.set("moreCodes", moreCodes);
            return ReturnObject(Answer);
        }
        return BadRequest(ErrorCode, ErrorText);
    }

}