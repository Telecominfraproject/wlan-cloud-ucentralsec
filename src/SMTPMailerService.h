//
// Created by stephane bourque on 2021-06-17.
//

#ifndef UCENTRALSEC_SMTPMAILERSERVICE_H
#define UCENTRALSEC_SMTPMAILERSERVICE_H

#include "SubSystemServer.h"

#include "Poco/File.h"

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
        CHALLENGE_CODE
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
                                 {  CHALLENGE_CODE, "CHALLENGE_CODE"}
                                 };

    inline const std::string & MessageAttributeToVar(MESSAGE_ATTRIBUTES Attr) {
        static const std::string EmptyString{};
        auto E = MessageAttributeMap.find(Attr);
        if(E == MessageAttributeMap.end())
            return EmptyString;
        return E->second;
    }
    typedef std::map<MESSAGE_ATTRIBUTES, std::string>   MessageAttributes;

    class SMTPMailerService : public SubSystemServer, Poco::Runnable {
        public:
           static SMTPMailerService *instance() {
                if (instance_ == nullptr) {
                    instance_ = new SMTPMailerService;
                }
                return instance_;
            }

            struct MessageEvent {
               uint64_t             Posted=0;
               uint64_t             LastTry=0;
               uint64_t             Sent=0;
               Poco::File           File;
               MessageAttributes    Attrs;
            };

            struct MessageCacheEntry {
               uint64_t         LastRequest=0;
               uint64_t         HowManyRequests=0;
            };

            void run() override;

            int Start() override;
            void Stop() override;
            bool SendMessage(const std::string &Recipient, const std::string &Name, const MessageAttributes &Attrs);
            bool SendIt(const MessageEvent &Msg);
            void LoadMyConfig();
            void reinitialize(Poco::Util::Application &self) override;
            bool Enabled() const { return Enabled_; }
        private:
            static SMTPMailerService * instance_;
            std::string             MailHost_;
            std::string             Sender_;
            int                     MailHostPort_=25;
            std::string             SenderLoginUserName_;
            std::string             SenderLoginPassword_;
            std::string             LoginMethod_ = "login";
            std::string             LogoFileName_;
            std::string             TemplateDir_;
            std::list<MessageEvent> Messages_;
            std::map<std::string,MessageCacheEntry> Cache_;
            Poco::Thread            SenderThr_;
            std::atomic_bool        Running_=false;
            bool                    Enabled_=false;

            SMTPMailerService() noexcept:
                SubSystemServer("SMTPMailer", "MAILER-SVR", "smtpmailer")
            {
                std::string E{"SHA512"};
            }
    };

    inline SMTPMailerService * SMTPMailerService() { return SMTPMailerService::instance(); }
}

#endif //UCENTRALSEC_SMTPMAILERSERVICE_H
