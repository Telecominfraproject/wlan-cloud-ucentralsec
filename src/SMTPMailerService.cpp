//
// Created by stephane bourque on 2021-06-17.
//
#include <iostream>
#include <fstream>

#include "Poco/Net/MailMessage.h"
#include "Poco/Net/MailRecipient.h"
#include "Poco/Net/SMTPClientSession.h"
#include "Poco/Net/SecureSMTPClientSession.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Exception.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/Context.h"

#include "SMTPMailerService.h"
#include "framework/MicroService.h"
#include "AuthService.h"

namespace OpenWifi {

    void SMTPMailerService::LoadMyConfig() {
        MailHost_ = MicroService::instance().ConfigGetString("mailer.hostname");
        SenderLoginUserName_ = MicroService::instance().ConfigGetString("mailer.username");
        SenderLoginPassword_ = MicroService::instance().ConfigGetString("mailer.password");
        Sender_ = MicroService::instance().ConfigGetString("mailer.sender");
        LoginMethod_ = MicroService::instance().ConfigGetString("mailer.loginmethod");
        MailHostPort_ = (int) MicroService::instance().ConfigGetInt("mailer.port");
        TemplateDir_ = MicroService::instance().ConfigPath("mailer.templates", MicroService::instance().DataDir());
        Enabled_ = (!MailHost_.empty() && !SenderLoginPassword_.empty() && !SenderLoginUserName_.empty());
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

    void SMTPMailerService::reinitialize(Poco::Util::Application &self) {
        MicroService::instance().LoadConfigurationFile();
        Logger_.information("Reinitializing.");
        LoadMyConfig();
    }

    bool SMTPMailerService::SendMessage(const std::string &Recipient, const std::string &Name, const MessageAttributes &Attrs) {
        std::lock_guard G(Mutex_);

        Messages_.push_back(MessageEvent{.Posted=(uint64_t )std::time(nullptr),
                                            .LastTry=0,
                                            .Sent=0,
                                            .File=Poco::File(TemplateDir_ + "/" +Name),
                                            .Attrs=Attrs});

        return true;
    }

    void SMTPMailerService::run() {

        Running_ = true;
        while(Running_) {
            Poco::Thread::trySleep(10000);
            if(!Running_)
                break;
            {
                std::lock_guard G(Mutex_);

                uint64_t Now = std::time(nullptr);

                for(auto &i:Messages_) {
                    if(i.Sent==0 && (i.LastTry==0 || (Now-i.LastTry)>120)) {
                        if (SendIt(i)) {
                            i.LastTry = i.Sent = std::time(nullptr);
                        } else
                            i.LastTry = std::time(nullptr);
                    }
                }

                //  Clean the list
                std::remove_if(Messages_.begin(),Messages_.end(),[Now](MessageEvent &E){ return (E.Sent!=0 || ((Now-E.LastTry)>(15*60)));});
            }
        }
    }

    void  FillVariables(const MessageAttributes &Attrs, Types::StringPairVec &R) {
        for(const auto &[Variable,Value]:Attrs) {
            R.push_back(std::make_pair(MessageAttributeToVar(Variable),Value));
        }
    }

    bool SMTPMailerService::SendIt(const MessageEvent &Msg) {
        std::string             Recipient;

        try
        {
            std::cout << __LINE__ << std::endl;
            Poco::Net::MailMessage  Message;
            std::cout << __LINE__ << std::endl;
            Recipient = Msg.Attrs.find(RECIPIENT_EMAIL)->second;
            std::cout << __LINE__ << std::endl;

            auto H1 = Msg.Attrs.find(SENDER);
            std::cout << __LINE__ << std::endl;
            std::string TheSender;
            std::cout << __LINE__ << std::endl;
            if(H1!=Msg.Attrs.end()) {
                std::cout << __LINE__ << std::endl;
                TheSender = H1->second ;
            } else {
                std::cout << __LINE__ << std::endl;
                TheSender = Sender_ ;
            }
            std::cout << __LINE__ << std::endl;
            Message.setSender( TheSender );
            std::cout << __LINE__ << std::endl;
            Logger_.information(Poco::format("Sending message to:%s from %s",Recipient,TheSender));
            std::cout << __LINE__ << std::endl;

            std::cout << __LINE__ << std::endl;
            Message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, Recipient));
            std::cout << __LINE__ << std::endl;
            Message.setSubject(Msg.Attrs.find(SUBJECT)->second);
            std::cout << __LINE__ << std::endl;

            if(Msg.Attrs.find(TEXT) != Msg.Attrs.end()) {
                std::cout << __LINE__ << std::endl;
                std::string Content = Msg.Attrs.find(TEXT)->second;
                std::cout << __LINE__ << std::endl;
                Message.addContent(new Poco::Net::StringPartSource(Content));
                std::cout << __LINE__ << std::endl;
            } else {
                std::cout << __LINE__ << std::endl;
                std::string Content = Utils::LoadFile(Msg.File);
                std::cout << __LINE__ << std::endl;
                Types::StringPairVec    Variables;
                std::cout << __LINE__ << std::endl;
                FillVariables(Msg.Attrs, Variables);
                std::cout << __LINE__ << std::endl;
                Utils::ReplaceVariables(Content, Variables);
                std::cout << __LINE__ << std::endl;
                Message.addContent(new Poco::Net::StringPartSource(Content));
                std::cout << __LINE__ << std::endl;
            }

            std::cout << __LINE__ << std::endl;
            auto Logo = Msg.Attrs.find(LOGO);
            std::cout << __LINE__ << std::endl;
            if(Logo!=Msg.Attrs.end()) {
                std::cout << __LINE__ << std::endl;
                try {
                    std::cout << __LINE__ << std::endl;
                    Poco::File          LogoFile(AuthService::GetLogoAssetFileName());
                    std::cout << __LINE__ << std::endl;
                    std::ifstream       IF(LogoFile.path());
                    std::cout << __LINE__ << std::endl;
                    std::ostringstream  OS;
                    std::cout << __LINE__ << std::endl;
                    Poco::StreamCopier::copyStream(IF, OS);
                    std::cout << __LINE__ << std::endl;
                    std::cout << __LINE__ << std::endl;
                    Message.addAttachment("logo", new Poco::Net::StringPartSource(OS.str(), "image/png"));
                    std::cout << __LINE__ << std::endl;
                } catch (...) {
                    std::cout << __LINE__ << std::endl;
                    Logger_.warning(Poco::format("Cannot add '%s' logo in email",AuthService::GetLogoAssetFileName()));
                    std::cout << __LINE__ << std::endl;
                }
            }

//            Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> pConsoleHandler = new KeyConsoleHandler;
//            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> pInvalidCertHandler = new ConsoleCertificateHandler;
            Poco::SharedPtr<Poco::Net::AcceptCertificateHandler>  ptrHandler_ = new Poco::Net::AcceptCertificateHandler(false);

            std::cout << __LINE__ << std::endl;
            Poco::Net::SecureSMTPClientSession session(MailHost_,MailHostPort_);
            std::cout << __LINE__ << std::endl;
            auto ptrContext = Poco::AutoPtr<Poco::Net::Context>
                    (new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "",
                                                            Poco::Net::Context::VERIFY_RELAXED, 9, true,
                                                            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
            std::cout << __LINE__ << std::endl;
            Poco::Net::SSLManager::instance().initializeClient(nullptr,
                                                               ptrHandler_,
                                                               ptrContext);
            std::cout << __LINE__ << std::endl;
            session.login();
            std::cout << __LINE__ << std::endl;
            session.startTLS(ptrContext);
            std::cout << __LINE__ << std::endl;
            session.login(MailHost_,
                          Poco::Net::SecureSMTPClientSession::AUTH_LOGIN,
                          SenderLoginUserName_,
                          SenderLoginPassword_
            );
            std::cout << __LINE__ << std::endl;
            session.sendMessage(Message);
            std::cout << __LINE__ << std::endl;
            session.close();
            std::cout << __LINE__ << std::endl;
            return true;
        }
        catch (const Poco::Exception& E)
        {
            Logger_.log(E);
        }
        catch (const std::exception &E) {
            Logger_.warning(Poco::format("Cannot send message to:%s, error: %s",Recipient, E.what()));
        }
        return false;
    }

}
