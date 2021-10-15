//
// Created by stephane bourque on 2021-10-09.
//

#ifndef OWSEC_SMSSENDER_H
#define OWSEC_SMSSENDER_H

#include "SubSystemServer.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentials.h>
#include "SMS_provider.h"

namespace OpenWifi {

    struct SMSValidationCacheEntry {
        std::string Number;
        std::string Code;
        std::string UserName;
        uint64_t    Created;
        bool        Validated=false;
    };

    class SMSSender : public SubSystemServer {
        public:
            static SMSSender *instance() {
                if (instance_ == nullptr) {
                    instance_ = new SMSSender;
                }
                return instance_;
            }

            int  Start() final;
            void Stop() final;
            bool Enabled() const { return Enabled_; }
            bool StartValidation(const std::string &Number, const std::string &UserName);
            bool CompleteValidation(const std::string &Number, const std::string &Code, const std::string &UserName);
            bool IsNumberValid(const std::string &Number, const std::string &UserName);
            [[nodiscard]] bool Send(const std::string &PhoneNumber, const std::string &Message);
        private:
            static SMSSender * instance_;
            std::string         Provider_;
            bool                Enabled_=false;
            std::vector<SMSValidationCacheEntry>    Cache_;
            std::unique_ptr<SMS_provider>           ProviderImpl_;

            SMSSender() noexcept:
                SubSystemServer("SMSSender", "SMS-SVR", "smssender.aws")
            {
            }

            bool SendAWS(const std::string &PhoneNumber, const std::string &Message);
            void CleanCache();

    };
    inline SMSSender * SMSSender() { return SMSSender::instance(); }

}


#endif //OWSEC_SMSSENDER_H
