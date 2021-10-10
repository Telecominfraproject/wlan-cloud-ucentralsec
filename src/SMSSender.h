//
// Created by stephane bourque on 2021-10-09.
//

#ifndef OWSEC_SMSSENDER_H
#define OWSEC_SMSSENDER_H

#include "SubSystemServer.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentials.h>

namespace OpenWifi {

    class SMSSender : public SubSystemServer {
        public:
            static SMSSender *instance() {
                if (instance_ == nullptr) {
                    instance_ = new SMSSender;
                }
                return instance_;
            }

            int Start() final;
            void Stop() final;
            [[nodiscard]] int Send(const std::string &PhoneNumber, const std::string &Message);

        private:
            static SMSSender * instance_;
            std::string         SecretKey_;
            std::string         AccessKey_;
            std::string         Region_;
            Aws::Client::ClientConfiguration    AwsConfig_;
            Aws::Auth::AWSCredentials           AwsCreds_;
            bool                Enabled_=false;

            SMSSender() noexcept:
                SubSystemServer("SMSSender", "SMS-SVR", "smssender.aws")
            {
            }
    };
    inline SMSSender * SMSSender() { return SMSSender::instance(); }

}


#endif //OWSEC_SMSSENDER_H
