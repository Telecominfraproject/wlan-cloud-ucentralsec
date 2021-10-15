//
// Created by stephane bourque on 2021-10-15.
//

#ifndef OWSEC_SMS_PROVIDER_TWILIO_H
#define OWSEC_SMS_PROVIDER_TWILIO_H

#include "SMS_provider.h"

namespace OpenWifi {
    class SMS_provider_twilio : public SMS_provider {
    public:
        explicit SMS_provider_twilio(Poco::Logger &L) : Logger_(L) {}
        ~SMS_provider_twilio() {};
        bool Initialize() final ;
        bool Start() final ;
        bool Stop() final ;
        bool Send(const std::string &Number, const std::string &Message) final;
        bool Running() final;
    private:
        bool                                Running_=false;
        Poco::Logger                        &Logger_;
        std::string                         Sid_;
        std::string                         Token_;
        std::string                         PhoneNumber_;
        std::string                         Uri_;
    };
}

#endif //OWSEC_SMS_PROVIDER_TWILIO_H
