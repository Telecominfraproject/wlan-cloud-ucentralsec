//
// Created by stephane bourque on 2021-10-09.
//

#include "SMSSender.h"
#include "Daemon.h"
#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>

namespace OpenWifi {
    class SMSSender * SMSSender::instance_ = nullptr;

    int SMSSender::Start() {
        SecretKey_ = Daemon()->ConfigGetString("smssender.aws.secretkey","");
        AccessKey_ = Daemon()->ConfigGetString("smssender.aws.accesskey","");
        Region_ = Daemon()->ConfigGetString("smssender.aws.region","");
        if(SecretKey_.empty() || AccessKey_.empty()) {
            Logger_.debug("SMSSender is disabled. Please provide key and access key in configuration.");
            return -1;
        }
        AwsConfig_.enableTcpKeepAlive = true;
        AwsConfig_.enableEndpointDiscovery = true;
        AwsConfig_.useDualStack = true;
        if(!Region_.empty())
            AwsConfig_.region = Region_;
        AwsCreds_.SetAWSAccessKeyId(AccessKey_.c_str());
        AwsCreds_.SetAWSSecretKey(SecretKey_.c_str());
        return 0;
    }

    void SMSSender::Stop() {
    }

    int SMSSender::Send(const std::string &PhoneNumber, const std::string &Message) {
        Aws::SNS::SNSClient sns(AwsCreds_,AwsConfig_);

        Aws::SNS::Model::PublishRequest psms_req;
        psms_req.SetMessage(Message.c_str());
        psms_req.SetPhoneNumber(PhoneNumber.c_str());

        auto psms_out = sns.Publish(psms_req);

        if (psms_out.IsSuccess())
        {
            std::cout << "Message published successfully " << psms_out.GetResult().GetMessageId()
            << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Error while publishing message " << psms_out.GetError().GetMessage()
            << std::endl;
            return -1;
        }
    }

}