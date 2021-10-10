//
// Created by stephane bourque on 2021-10-09.
//

#include "SMSSender.h"
#include "Daemon.h"
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>
#include <aws/sns/model/GetSMSAttributesRequest.h>

namespace OpenWifi {
    class SMSSender * SMSSender::instance_ = nullptr;

    int SMSSender::Start() {
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

        return 0;
    }

    void SMSSender::Stop() {
    }

    int SMSSender::Send(const std::string &PhoneNumber, const std::string &Message) {
        if(!Enabled_) {
            Logger_.information("SMS has not been enabled. Messages cannot be sent.");
            return -2;
        }

        Aws::SNS::SNSClient sns(AwsCreds_,AwsConfig_);

        Aws::SNS::Model::PublishRequest psms_req;
        psms_req.SetMessage(Message.c_str());
        psms_req.SetPhoneNumber(PhoneNumber.c_str());

        auto psms_out = sns.Publish(psms_req);
        if (psms_out.IsSuccess()) {
            Logger_.debug(Poco::format("SMS sent to %s",PhoneNumber));
            return 0;
        }

        Logger_.debug(Poco::format("SMS NOT sent to %s",PhoneNumber));
        return -1;
    }
}