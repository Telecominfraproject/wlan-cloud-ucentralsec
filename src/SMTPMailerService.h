//
// Created by stephane bourque on 2021-06-17.
//

#ifndef UCENTRALSEC_SMTPMAILERSERVICE_H
#define UCENTRALSEC_SMTPMAILERSERVICE_H

#include "SubSystemServer.h"

namespace uCentral {
    class SMTPMailerService : public SubSystemServer {
    public:
        enum MESSAGE_ATTRIBUTES {
            RECIPIENT_EMAIL,
            RECIPIENT_FIRST_NAME,
            RECIPIENT_LAST_NAME,
            RECIPIENT_INITIALS,
            RECIPIENT_FULL_NAME,
            RECIPIENT_SALUTATION,
            SUBJECT,
            SIGNATURE,
            TEMPLATE,
            LOGO
        };

        typedef std::map<MESSAGE_ATTRIBUTES, std::string>   MessageAttributes;

        static SMTPMailerService *instance() {
            if (instance_ == nullptr) {
                instance_ = new SMTPMailerService;
            }
            return instance_;
        }

        int Start() override;
        void Stop() override;
        bool SendMessage(MessageAttributes Attrs);
        bool SendIt();

    private:
        static SMTPMailerService * instance_;
        std::string     MailHost_;
        std::string     Sender_;
        int             MailHostPort_=25;
        std::string     SenderLoginUserName_;
        std::string     SenderLoginPassword_;
        std::string     LoginMethod_ = "login";
        std::string     LogoFileName_;

        SMTPMailerService() noexcept:
            SubSystemServer("SMTPMailer", "MAILER-SVR", "smtpmailer")
        {
            std::string E{"SHA512"};
        }
    };

    inline SMTPMailerService * SMTPMailerService() { return SMTPMailerService::instance(); }
}

#endif //UCENTRALSEC_SMTPMAILERSERVICE_H
