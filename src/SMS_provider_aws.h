//
// Created by stephane bourque on 2021-10-15.
//

#ifndef OWSEC_SMS_PROVIDER_AWS_H
#define OWSEC_SMS_PROVIDER_AWS_H

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentials.h>

#include "SMS_provider.h"

namespace OpenWifi {
    class SMS_provider_aws : public SMS_provider {
    public:
        explicit SMS_provider_aws(Poco::Logger &L) : Logger_(L) {}
        ~SMS_provider_aws() {};
        bool Initialize() final ;
        bool Start() final ;
        bool Stop() final ;
        bool Send(const std::string &Number, const std::string &Message) final;
        bool Running() final;
        inline Poco::Logger & Logger() { return Logger_; }
    private:
        bool                                Running_=false;
        Poco::Logger                        &Logger_;
        std::string                         SecretKey_;
        std::string                         AccessKey_;
        std::string                         Region_;
        Aws::Client::ClientConfiguration    AwsConfig_;
        Aws::Auth::AWSCredentials           AwsCreds_;
    };
}

#endif //OWSEC_SMS_PROVIDER_AWS_H
