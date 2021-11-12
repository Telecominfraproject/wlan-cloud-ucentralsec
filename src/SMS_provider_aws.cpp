//
// Created by stephane bourque on 2021-10-15.
//


#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>

#include "framework/MicroService.h"
#include "SMS_provider_aws.h"

namespace OpenWifi {
    bool SMS_provider_aws::Initialize() {
        SecretKey_ = MicroService::instance().ConfigGetString("smssender.aws.secretkey","");
        AccessKey_ = MicroService::instance().ConfigGetString("smssender.aws.accesskey","");
        Region_ = MicroService::instance().ConfigGetString("smssender.aws.region","");

        if(SecretKey_.empty() || AccessKey_.empty() || Region_.empty()) {
            Logger_.debug("SMSSender is disabled. Please provide key, secret, and region.");
            return false;
        }
        Running_=true;
        AwsConfig_.region = Region_;
        AwsCreds_.SetAWSAccessKeyId(AccessKey_.c_str());
        AwsCreds_.SetAWSSecretKey(SecretKey_.c_str());
        return true;
    }

    bool SMS_provider_aws::Start() {
        return true;
    }

    bool SMS_provider_aws::Stop() {
        return true;
    }

    bool SMS_provider_aws::Running() {
        return Running_;
    }

    bool SMS_provider_aws::Send(const std::string &PhoneNumber, const std::string &Message) {
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        if(!Running_)
            return false;

        std::cout << __func__ << " : " << __LINE__ << std::endl;
        Aws::SNS::SNSClient sns(AwsCreds_,AwsConfig_);
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        Aws::SNS::Model::PublishRequest psms_req;
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        psms_req.SetMessage(Message.c_str());
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        psms_req.SetPhoneNumber(PhoneNumber.c_str());
        std::cout << __func__ << " : " << __LINE__ << std::endl;

        auto psms_out = sns.Publish(psms_req);
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        if (psms_out.IsSuccess()) {
            std::cout << __func__ << " : " << __LINE__ << std::endl;
            Logger_.debug(Poco::format("SMS sent to %s",PhoneNumber));
            std::cout << __func__ << " : " << __LINE__ << std::endl;
            return true;
        }
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        std::string ErrMsg{psms_out.GetError().GetMessage()};
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        Logger_.debug(Poco::format("SMS NOT sent to %s: %s",PhoneNumber, ErrMsg));
        std::cout << __func__ << " : " << __LINE__ << std::endl;
        return false;
    }

}