//
// Created by stephane bourque on 2021-10-15.
//

#include "SMS_provider_aws.h"

#include <aws/sns/SNSClient.h>
#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>

#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {
	bool SMS_provider_aws::Initialize() {
		SecretKey_ = MicroServiceConfigGetString("smssender.aws.secretkey", "");
		AccessKey_ = MicroServiceConfigGetString("smssender.aws.accesskey", "");
		Region_ = MicroServiceConfigGetString("smssender.aws.region", "");

		if (SecretKey_.empty() || AccessKey_.empty() || Region_.empty()) {
			poco_debug(Logger(), "SMSSender is disabled. Please provide key, secret, and region.");
			return false;
		}
		Running_ = true;
		AwsConfig_.region = Region_;
		AwsCreds_.SetAWSAccessKeyId(AccessKey_.c_str());
		AwsCreds_.SetAWSSecretKey(SecretKey_.c_str());
		return true;
	}

	bool SMS_provider_aws::Start() { return true; }

	bool SMS_provider_aws::Stop() { return true; }

	bool SMS_provider_aws::Running() { return Running_; }

	bool SMS_provider_aws::Send(const std::string &PhoneNumber, const std::string &Message) {
		if (!Running_)
			return false;

		try {
			Aws::SNS::SNSClient sns(AwsCreds_, AwsConfig_);
			Aws::SNS::Model::PublishRequest psms_req;
			psms_req.SetMessage(Message.c_str());
			psms_req.SetPhoneNumber(PhoneNumber.c_str());

			auto psms_out = sns.Publish(psms_req);
			if (psms_out.IsSuccess()) {
				poco_debug(Logger(), fmt::format("SMS sent to {}", PhoneNumber));
				return true;
			}
			std::string ErrMsg{psms_out.GetError().GetMessage()};
			poco_debug(Logger(), fmt::format("SMS NOT sent to {}: {}", PhoneNumber, ErrMsg));
			return false;
		} catch (...) {
		}
		poco_debug(Logger(),
				   fmt::format("SMS NOT sent to {}: failure in SMS service", PhoneNumber));
		return false;
	}

} // namespace OpenWifi