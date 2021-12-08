//
// Created by stephane bourque on 2021-12-01.
//

#include "RESTAPI_submfa_handler.h"
#include "StorageService.h"
#include "SMSSender.h"

namespace OpenWifi {

    void RESTAPI_submfa_handler::DoGet() {
        SecurityObjects::UserInfo   User;

        if (StorageService()->GetUserById(UserInfo_.userinfo.Id,User)) {
            Poco::JSON::Object              Answer;
            SecurityObjects::SubMfaConfig   MFC;

            MFC.id = User.Id;
            if(User.userTypeProprietaryInfo.mfa.enabled) {
                if(User.userTypeProprietaryInfo.mfa.method == "sms") {
                    MFC.sms = User.userTypeProprietaryInfo.mobiles[0].number;
                } else if(User.userTypeProprietaryInfo.mfa.method == "email") {
                    MFC.email = User.email;
                }
            } else {
                MFC.type = "disabled";
            }
            MFC.to_json(Answer);
            return ReturnObject(Answer);
        }
        NotFound();
    }

#define DBGLINE std::cout << __FILE__ << " : " << __LINE__ << std::endl;
    void RESTAPI_submfa_handler::DoPut() {

        std::cout << "DoPut..." << std::endl;

        try {
            DBGLINE
            auto Body = ParseStream();
            DBGLINE

            SecurityObjects::SubMfaConfig MFC;

            DBGLINE

            if (!MFC.from_json(Body)) {
                DBGLINE
                return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            }

            if (MFC.type == "disabled") {
                DBGLINE
                SecurityObjects::UserInfo User;
                StorageService()->GetUserById(UserInfo_.userinfo.Id, User);
                User.userTypeProprietaryInfo.mfa.enabled = false;
                StorageService()->UpdateUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                Poco::JSON::Object Answer;
                MFC.to_json(Answer);
                DBGLINE
                return ReturnObject(Answer);
            } else if (MFC.type == "email") {
                DBGLINE
                SecurityObjects::UserInfo User;

                StorageService()->GetUserById(UserInfo_.userinfo.Id, User);
                User.userTypeProprietaryInfo.mfa.enabled = true;
                User.userTypeProprietaryInfo.mfa.method = "email";
                StorageService()->UpdateUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                Poco::JSON::Object Answer;
                MFC.to_json(Answer);
                DBGLINE
                return ReturnObject(Answer);
            } else if (MFC.type == "sms") {
                DBGLINE
                if (GetBoolParameter("startValidation", false)) {
                    DBGLINE
                    if (MFC.sms.empty()) {
                        return BadRequest("Missing phone number");
                    }
                    DBGLINE

                    if (SMSSender()->StartValidation(MFC.sms, UserInfo_.userinfo.email)) {
                        return OK();
                    } else {
                        return InternalError("SMS could not be sent. Verify the number or try again later.");
                    }
                    DBGLINE
                } else if (GetBoolParameter("completeValidation", false)) {
                    auto ChallengeCode = GetParameter("challengeCode", "");
                    if (ChallengeCode.empty()) {
                        DBGLINE
                        return BadRequest("Missing 'challengeCode'");
                    }
                    if (MFC.sms.empty()) {
                        DBGLINE
                        return BadRequest("Missing phone number");
                    }
                    if (SMSSender()->CompleteValidation(MFC.sms, ChallengeCode, UserInfo_.userinfo.email)) {
                        SecurityObjects::UserInfo User;
                        DBGLINE

                        StorageService()->GetUserById(UserInfo_.userinfo.Id, User);
                        User.userTypeProprietaryInfo.mfa.method = "sms";
                        SecurityObjects::MobilePhoneNumber PhoneNumber;
                        PhoneNumber.number = MFC.sms;
                        PhoneNumber.primary = true;
                        PhoneNumber.verified = true;
                        User.userTypeProprietaryInfo.mfa.enabled = true;
                        User.userTypeProprietaryInfo.mobiles.clear();
                        User.userTypeProprietaryInfo.mobiles.push_back(PhoneNumber);
                        StorageService()->UpdateUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                        Poco::JSON::Object Answer;
                        MFC.to_json(Answer);
                        return ReturnObject(Answer);
                    } else {
                        DBGLINE
                        return InternalError("SMS could not be sent. Verify the number or try again later.");
                    }
                }
            }
        } catch (const Poco::Exception &E) {
            DBGLINE
            Logger_.log(E);
        }
        DBGLINE
        return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }

}
