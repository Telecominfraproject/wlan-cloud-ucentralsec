//
// Created by stephane bourque on 2021-06-17.
//

#ifndef UCENTRALSEC_SMTPMAILERSERVICE_H
#define UCENTRALSEC_SMTPMAILERSERVICE_H

#include "framework/MicroService.h"

#include "Poco/File.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"

namespace OpenWifi {

    enum MESSAGE_ATTRIBUTES {
        RECIPIENT_EMAIL,
        RECIPIENT_FIRST_NAME,
        RECIPIENT_LAST_NAME,
        RECIPIENT_INITIALS,
        RECIPIENT_FULL_NAME,
        RECIPIENT_SALUTATION,
        ACTION_LINK,
        SUBJECT,
        TEMPLATE_TXT,
        TEMPLATE_HTML,
        LOGO,
        TEXT,
        CHALLENGE_CODE,
        SENDER
    };

    static const std::map<MESSAGE_ATTRIBUTES,const std::string>
            MessageAttributeMap{ {  RECIPIENT_EMAIL,"RECIPIENT_EMAIL"},
                                 {  RECIPIENT_FIRST_NAME, "RECIPIENT_FIRST_NAME"},
                                 {  RECIPIENT_LAST_NAME, "RECIPIENT_LAST_NAME"},
                                 {  RECIPIENT_INITIALS, "RECIPIENT_INITIALS"},
                                 {  RECIPIENT_FULL_NAME, "RECIPIENT_FULL_NAME"},
                                 {  RECIPIENT_SALUTATION, "RECIPIENT_SALUTATION"},
                                 {  ACTION_LINK, "ACTION_LINK"},
                                 {  SUBJECT, "SUBJECT"},
                                 {  TEMPLATE_TXT, "TEMPLATE_TXT"},
                                 {  TEMPLATE_HTML, "TEMPLATE_HTML"},
                                 {  LOGO, "LOGO"},
                                 {  TEXT, "TEXT"},
                                 {  CHALLENGE_CODE, "CHALLENGE_CODE"},
                                 {  SENDER, "SENDER"}
                                 };

    inline const std::string & MessageAttributeToVar(MESSAGE_ATTRIBUTES Attr) {
        static const std::string EmptyString{};
        auto E = MessageAttributeMap.find(Attr);
        if(E == MessageAttributeMap.end())
            return EmptyString;
        return E->second;
    }
    typedef std::map<MESSAGE_ATTRIBUTES, std::string>   MessageAttributes;

    enum class MessageSendStatus {
        msg_sent,
        msg_not_sent_but_resend,
        msg_not_sent_but_do_not_resend
    };

    class SMTPMailerService : public SubSystemServer, Poco::Runnable {
        public:
           static SMTPMailerService *instance() {
               static auto * instance_ = new SMTPMailerService;
               return instance_;
            }

            struct MessageEvent {
               uint64_t             Posted=0;
               uint64_t             LastTry=0;
               uint64_t             Sent=0;
               std::string          TemplateName;
               MessageAttributes    Attrs;
            };

            void run() override;
            int Start() override;
            void Stop() override;

            bool SendMessage(const std::string &Recipient, const std::string &Name, const MessageAttributes &Attrs);
            MessageSendStatus SendIt(const MessageEvent &Msg);
            void LoadMyConfig();
            void reinitialize(Poco::Util::Application &self) override;
            bool Enabled() const { return Enabled_; }

        private:
            std::string             MailHost_;
            std::string             Sender_;
            uint32_t                MailHostPort_=25;
            uint64_t                MailRetry_=2*60;
            uint64_t                MailAbandon_=2*60*20;
            std::string             SenderLoginUserName_;
            std::string             SenderLoginPassword_;
            std::string             LoginMethod_ = "login";
            std::string             TemplateDir_;
            std::list<MessageEvent> Messages_;
            std::list<MessageEvent> PendingMessages_;
            Poco::Thread            SenderThr_;
            std::atomic_bool        Running_=false;
            bool                    Enabled_=false;
            bool                    UseHTML_=false;
            std::string             EmailLogo_{"logo.jpg"};

            SMTPMailerService() noexcept:
                SubSystemServer("SMTPMailer", "MAILER-SVR", "smtpmailer")
            {
            }
    };

    inline SMTPMailerService * SMTPMailerService() { return SMTPMailerService::instance(); }
}

#endif //UCENTRALSEC_SMTPMAILERSERVICE_H
