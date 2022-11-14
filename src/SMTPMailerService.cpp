//
// Created by stephane bourque on 2021-06-17.
//

#include "Poco/Net/MailMessage.h"
#include "Poco/Net/MailRecipient.h"
#include "Poco/Net/SMTPClientSession.h"
#include "Poco/Net/SecureSMTPClientSession.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Exception.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/NetException.h"

#include "SMTPMailerService.h"
#include "AuthService.h"

#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"
#include "fmt/format.h"

namespace OpenWifi {

    void SMTPMailerService::LoadMyConfig() {
        Enabled_ = MicroServiceConfigGetBool("mailer.enabled",false);
        if(Enabled_) {
            MailHost_ = MicroServiceConfigGetString("mailer.hostname","");
            SenderLoginUserName_ = MicroServiceConfigGetString("mailer.username","");
            SenderLoginPassword_ = MicroServiceConfigGetString("mailer.password","");
            Sender_ = MicroServiceConfigGetString("mailer.sender","");
            LoginMethod_ = MicroServiceConfigGetString("mailer.loginmethod","");
            MailHostPort_ = MicroServiceConfigGetInt("mailer.port", 587);
            TemplateDir_ = MicroServiceConfigPath("mailer.templates", MicroServiceDataDirectory());
            MailRetry_ = MicroServiceConfigGetInt("mailer.retry",2*60);
            MailAbandon_ = MicroServiceConfigGetInt("mailer.abandon",2*60*60);
            UseHTML_ = MicroServiceConfigGetBool("mailer.html",false);
            Enabled_ = (!MailHost_.empty() && !SenderLoginPassword_.empty() && !SenderLoginUserName_.empty());
            EmailLogo_ = TemplateDir_ + "/" + MicroServiceConfigGetString("mailer.logo","logo.png");
        }
    }

    int SMTPMailerService::Start() {
        LoadMyConfig();
        SenderThr_.start(*this);
        return 0;
    }

    void SMTPMailerService::Stop() {
        Running_ = false;
        SenderThr_.wakeUp();
        SenderThr_.join();
    }

    void SMTPMailerService::reinitialize([[maybe_unused]] Poco::Util::Application &self) {
        MicroServiceLoadConfigurationFile();
        poco_information(Logger(),"Reinitializing.");
        LoadMyConfig();
    }

    bool SMTPMailerService::SendMessage([[maybe_unused]] const std::string &Recipient, const std::string &Name, const MessageAttributes &Attrs, bool Subscriber) {
        std::lock_guard G(Mutex_);
        PendingMessages_.push_back(MessageEvent{.Posted= OpenWifi::Now(),
                                            .LastTry=0,
                                            .Sent=0,
                                            .TemplateName=Name,
                                            .Attrs=Attrs,
                                            .Subscriber=Subscriber});
        return true;
    }

    void SMTPMailerService::run() {
        Running_ = true;
        Utils::SetThreadName("smtp-mailer");
        while(Running_) {

            Poco::Thread::trySleep(10000);
            if(!Running_)
                break;

            {
                std::lock_guard G(Mutex_);
                Messages_.splice(Messages_.end(),PendingMessages_);
            }

            for(auto i=Messages_.begin();i!=Messages_.end();) {
                if(!Running_)
                    break;
                auto Recipient = i->Attrs.find(RECIPIENT_EMAIL)->second;
                uint64_t now = OpenWifi::Now();
                if((i->LastTry==0 || (now-i->LastTry)>MailRetry_)) {
                    switch(SendIt(*i)) {
                        case MessageSendStatus::msg_sent: {
                            poco_information(Logger(),fmt::format("Attempting to deliver for mail '{}'.", Recipient));
                            i = Messages_.erase(i);
                        } break;
                        case MessageSendStatus::msg_not_sent_but_resend: {
                            poco_information(Logger(),fmt::format("Mail for '{}' was not. We will retry later.", Recipient));
                            i->LastTry = now;
                            ++i;
                        } break;
                        case MessageSendStatus::msg_not_sent_but_do_not_resend: {
                            poco_information(Logger(),fmt::format("Mail for '{}' will not be sent. Check email address", Recipient));
                            i = Messages_.erase(i);
                        } break;
                    }
                } else if ((now-i->Posted)>MailAbandon_) {
                    poco_information(Logger(),fmt::format("Mail for '{}' has timed out and will not be sent.", Recipient));
                    i = Messages_.erase(i);
                } else {
                    ++i;
                }
            }
        }
    }

    void  FillVariables(const MessageAttributes &Attrs, Types::StringPairVec &R) {
        for(const auto &[Variable,Value]:Attrs) {
            R.push_back(std::make_pair(MessageAttributeToVar(Variable),Value));
        }
    }

    MessageSendStatus SMTPMailerService::SendIt(const MessageEvent &Msg) {

        std::string             Recipient;

        try
        {
            auto H1 = Msg.Attrs.find(SENDER);
            std::string TheSender;
            if(H1!=Msg.Attrs.end()) {
                TheSender = H1->second ;
            } else {
                TheSender = Sender_ ;
            }

            auto Message = std::make_unique<Poco::Net::MailMessage>();

            Recipient = Msg.Attrs.find(RECIPIENT_EMAIL)->second;
            Message->setSender( TheSender );
            Message->addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, Recipient));
            Message->setSubject(Msg.Attrs.find(SUBJECT)->second);

            poco_information(Logger(),fmt::format("Sending message to:{} from {}",Recipient,TheSender));

            if(Msg.Attrs.find(TEXT) != Msg.Attrs.end()) {
                std::string Content = Msg.Attrs.find(TEXT)->second;
                Message->addContent(new Poco::Net::StringPartSource(Content));
            } else {
                for(const auto &format:{"html","txt"}) {
                    std::string Content = Utils::LoadFile(TemplateDir_ + Msg.TemplateName + "." + format );
                    Types::StringPairVec Variables;
                    FillVariables(Msg.Attrs, Variables);
                    Utils::ReplaceVariables(Content, Variables);
                    Message->addContent(
                            new Poco::Net::StringPartSource(Content, (strcmp(format,"html") == 0 ? "text/html" : "text/plain") ));
                }
            }

            auto Logo = Msg.Attrs.find(LOGO);
            if(Logo!=Msg.Attrs.end()) {
                try {
                    Poco::File          LogoFile( Msg.Subscriber ? AuthService::GetSubLogoAssetFileName() :  AuthService::GetLogoAssetFileName ());
                    std::ifstream       IF(LogoFile.path());
                    std::ostringstream  OS;
                    Poco::StreamCopier::copyStream(IF, OS);
                    Message->addAttachment("logo", new Poco::Net::StringPartSource(OS.str(), "image/png"));
                } catch (...) {
                    poco_warning(Logger(),fmt::format("Cannot add '{}' logo in email",AuthService::GetLogoAssetFileName()));
                }
            }

            Poco::SharedPtr<Poco::Net::AcceptCertificateHandler>  ptrHandler_ = new Poco::Net::AcceptCertificateHandler(false);

            Poco::Net::SecureSMTPClientSession session(MailHost_,MailHostPort_);
            auto ptrContext = Poco::AutoPtr<Poco::Net::Context>
                    (new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "",
                                                            Poco::Net::Context::VERIFY_RELAXED, 9, true,
                                                            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
            Poco::Net::SSLManager::instance().initializeClient(nullptr,
                                                               ptrHandler_,
                                                               ptrContext);
            session.login();
            session.startTLS(ptrContext);
            session.login(MailHost_,
                          Poco::Net::SecureSMTPClientSession::AUTH_LOGIN,
                          SenderLoginUserName_,
                          SenderLoginPassword_
            );
            session.sendMessage(*Message);
            session.close();
            return MessageSendStatus::msg_sent;
        }
        catch (const Poco::Net::SMTPException &S) {
            Logger().log(S);
            return MessageSendStatus::msg_not_sent_but_do_not_resend;
        }
        catch (const Poco::Exception& E)
        {
            Logger().log(E);
            return MessageSendStatus::msg_not_sent_but_resend;
        }
        catch (const std::exception &E) {
            poco_warning(Logger(),fmt::format("Cannot send message to:{}, error: {}",Recipient, E.what()));
            return MessageSendStatus::msg_not_sent_but_do_not_resend;
        }
    }

}
