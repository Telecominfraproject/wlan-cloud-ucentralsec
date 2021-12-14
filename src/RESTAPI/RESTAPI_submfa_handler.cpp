//
// Created by stephane bourque on 2021-12-01.
//

#include "RESTAPI_submfa_handler.h"
#include "StorageService.h"
#include "SMSSender.h"

namespace OpenWifi {

    void RESTAPI_submfa_handler::DoGet() {
        SecurityObjects::UserInfo   User;

        // std::cout << "submfa get " << UserInfo_.userinfo.Id << "   user:" << UserInfo_.userinfo.email << std::endl;
        if (StorageService()->GetSubUserById(UserInfo_.userinfo.Id,User)) {
            Poco::JSON::Object              Answer;
            SecurityObjects::SubMfaConfig   MFC;

            MFC.id = User.Id;
            if(User.userTypeProprietaryInfo.mfa.enabled) {
                if(User.userTypeProprietaryInfo.mfa.method == "sms") {
                    MFC.sms = User.userTypeProprietaryInfo.mobiles[0].number;
                    MFC.type = "sms";
                } else if(User.userTypeProprietaryInfo.mfa.method == "email") {
                    MFC.email = User.email;
                    MFC.type = "email";
                }
            } else {
                MFC.type = "disabled";
            }
            MFC.to_json(Answer);
            return ReturnObject(Answer);
        }
        NotFound();
    }

    void RESTAPI_submfa_handler::DoPut() {

        try {
            auto Body = ParseStream();

            SecurityObjects::SubMfaConfig MFC;

            if (!MFC.from_json(Body)) {
                return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            }

            if (MFC.type == "disabled") {
                SecurityObjects::UserInfo User;
                StorageService()->GetSubUserById(UserInfo_.userinfo.Id, User);
                User.userTypeProprietaryInfo.mfa.enabled = false;
                StorageService()->UpdateSubUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                Poco::JSON::Object Answer;
                MFC.to_json(Answer);
                return ReturnObject(Answer);
            } else if (MFC.type == "email") {
                SecurityObjects::UserInfo User;

                StorageService()->GetSubUserById(UserInfo_.userinfo.Id, User);
                User.userTypeProprietaryInfo.mfa.enabled = true;
                User.userTypeProprietaryInfo.mfa.method = "email";
                StorageService()->UpdateSubUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                MFC.sms = MFC.sms;
                MFC.type = "email";
                MFC.email = UserInfo_.userinfo.email;
                MFC.id = MicroService::instance().CreateUUID();

                Poco::JSON::Object Answer;
                MFC.to_json(Answer);
                return ReturnObject(Answer);

            } else if (MFC.type == "sms") {
                if (GetBoolParameter("startValidation", false)) {
                    if (MFC.sms.empty()) {
                        return BadRequest("Missing phone number");
                    }

                    if (SMSSender()->StartValidation(MFC.sms, UserInfo_.userinfo.email)) {
                        return OK();
                    } else {
                        return InternalError("SMS could not be sent. Verify the number or try again later.");
                    }
                } else if (GetBoolParameter("completeValidation", false)) {
                    auto ChallengeCode = GetParameter("challengeCode", "");
                    if (ChallengeCode.empty()) {
                        return BadRequest("Missing 'challengeCode'");
                    }
                    if (MFC.sms.empty()) {
                        return BadRequest("Missing phone number");
                    }
                    if (SMSSender()->CompleteValidation(MFC.sms, ChallengeCode, UserInfo_.userinfo.email)) {
                        SecurityObjects::UserInfo User;

                        StorageService()->GetSubUserById(UserInfo_.userinfo.Id, User);
                        User.userTypeProprietaryInfo.mfa.enabled = true;
                        User.userTypeProprietaryInfo.mfa.method = "sms";
                        SecurityObjects::MobilePhoneNumber PhoneNumber;
                        PhoneNumber.number = MFC.sms;
                        PhoneNumber.primary = true;
                        PhoneNumber.verified = true;
                        User.userTypeProprietaryInfo.mobiles.clear();
                        User.userTypeProprietaryInfo.mobiles.push_back(PhoneNumber);

                        StorageService()->UpdateSubUserInfo(UserInfo_.userinfo.email, UserInfo_.userinfo.Id, User);

                        MFC.sms = MFC.sms;
                        MFC.type = "sms";
                        MFC.email = UserInfo_.userinfo.email;
                        MFC.id = MicroService::instance().CreateUUID();

                        Poco::JSON::Object Answer;
                        MFC.to_json(Answer);

                        return ReturnObject(Answer);

                    } else {
                        return InternalError("SMS could not be sent. Verify the number or try again later.");
                    }
                }
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }

}
