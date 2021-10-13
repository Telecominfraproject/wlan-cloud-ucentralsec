//
// Created by stephane bourque on 2021-10-09.
//

#include "SMSSender.h"
#include "Daemon.h"
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>
#include <aws/sns/model/GetSMSAttributesRequest.h>
#include "MFAServer.h"

namespace OpenWifi {
    class SMSSender * SMSSender::instance_ = nullptr;

    int SMSSender::Start() {
        Provider_ = Daemon()->ConfigGetString("sms.provider","aws");
        if(Provider_=="aws") {
            SecretKey_ = Daemon()->ConfigGetString("smssender.aws.secretkey","");
            AccessKey_ = Daemon()->ConfigGetString("smssender.aws.accesskey","");
            Region_ = Daemon()->ConfigGetString("smssender.aws.region","");

            if(SecretKey_.empty() || AccessKey_.empty() || Region_.empty()) {
                Logger_.debug("SMSSender is disabled. Please provide key, secret, and region.");
                return -1;
            }
            Enabled_=true;
            AwsConfig_.region = Region_;
            AwsCreds_.SetAWSAccessKeyId(AccessKey_.c_str());
            AwsCreds_.SetAWSSecretKey(SecretKey_.c_str());
        }
        return 0;
    }

    void SMSSender::Stop() {
    }

    void SMSSender::CleanCache() {
        uint64_t Now=std::time(nullptr);
        for(auto i=begin(Cache_);i!=end(Cache_);) {
            if((Now-i->Created)>300)
                i = Cache_.erase(i);
            else
                ++i;
        }
    }

    bool SMSSender::StartValidation(const std::string &Number, const std::string &UserName) {
        std::lock_guard     G(Mutex_);
        CleanCache();
        uint64_t Now=std::time(nullptr);
        auto Challenge = MFAServer::MakeChallenge();
        Cache_.emplace_back(SMSValidationCacheEntry{.Number=Number, .Code=Challenge, .UserName=UserName, .Created=Now});
        std::string Message = "Please enter the following code on your login screen: " + Challenge;
        return Send(Number, Message);
    }

    bool SMSSender::IsNumberValid(const std::string &Number, const std::string &UserName) {
        std::lock_guard     G(Mutex_);

        for(const auto &i:Cache_) {
            if(i.Number==Number && i.UserName==UserName)
                return i.Validated;
        }
        return false;
    }

    bool SMSSender::CompleteValidation(const std::string &Number, const std::string &Code, const std::string &UserName) {
        std::lock_guard     G(Mutex_);

        for(auto &i:Cache_) {
            if(i.Code==Code && i.Number==Number && i.UserName==UserName) {
                i.Validated=true;
                return true;
            }
        }
        return false;
    }

    bool SMSSender::SendAWS(const std::string &PhoneNumber, const std::string &Message) {
        Aws::SNS::SNSClient sns(AwsCreds_,AwsConfig_);

        Aws::SNS::Model::PublishRequest psms_req;
        psms_req.SetMessage(Message.c_str());
        psms_req.SetPhoneNumber(PhoneNumber.c_str());

        auto psms_out = sns.Publish(psms_req);
        if (psms_out.IsSuccess()) {
            Logger_.debug(Poco::format("SMS sent to %s",PhoneNumber));
            return true;
        }
        std::string ErrMsg = psms_out.GetError().GetMessage().c_str();
        Logger_.debug(Poco::format("SMS NOT sent to %s: %s",PhoneNumber, ErrMsg));
        return false;
    }

    bool SMSSender::Send(const std::string &PhoneNumber, const std::string &Message) {
        if(!Enabled_) {
            Logger_.information("SMS has not been enabled. Messages cannot be sent.");
            return false;
        }
        if(Provider_=="aws")
            return SendAWS(PhoneNumber, Message);
        return false;
    }
}