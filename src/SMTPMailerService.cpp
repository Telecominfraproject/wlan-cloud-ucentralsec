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
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"

#include "SMTPMailerService.h"
#include "Utils.h"
#include "Daemon.h"

namespace OpenWifi {

    class SMTPMailerService * SMTPMailerService::instance_ = nullptr;

    void SMTPMailerService::LoadMyConfig() {
        MailHost_ = Daemon()->ConfigGetString("mailer.hostname");
        SenderLoginUserName_ = Daemon()->ConfigGetString("mailer.username");
        SenderLoginPassword_ = Daemon()->ConfigGetString("mailer.password");
        Sender_ = Daemon()->ConfigGetString("mailer.sender");
        LoginMethod_ = Daemon()->ConfigGetString("mailer.loginmethod");
        MailHostPort_ = (int)Daemon()->ConfigGetInt("mailer.port");
        TemplateDir_ = Daemon()->ConfigPath("mailer.templates", Daemon()->DataDir());
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
        Daemon()->LoadConfigurationFile();
        Logger_.information("Reinitializing.");
        LoadMyConfig();
    }

    bool SMTPMailerService::SendMessage(const std::string &Recipient, const std::string &Name, const MessageAttributes &Attrs) {
        std::lock_guard G(Mutex_);

        uint64_t Now = std::time(nullptr);
        auto CE = Cache_.find(Poco::toLower(Recipient));
        if(CE!=Cache_.end()) {
            // only allow messages to the same user within 2 minutes
            if((CE->second.LastRequest-Now)<30)
                return false;
            if((CE->second.HowManyRequests>30))
                return false;
        }

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
                std::remove_if(Messages_.begin(),Messages_.end(),[Now](MessageEvent &E){ return (E.Sent!=0 || ((Now-E.LastTry)>(24*60*60)));});
            }
        }
    }

    void  FillVariables(const MessageAttributes &Attrs, Types::StringPairVec &R) {
        for(const auto &[Variable,Value]:Attrs) {
            R.push_back(std::make_pair(MessageAttributeToVar(Variable),Value));
        }
    }

    bool SMTPMailerService::SendIt(const MessageEvent &Msg) {
        try
        {
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrHandler = new Poco::Net::AcceptCertificateHandler(false);

            Poco::Net::MailMessage  Message;
            std::string             Recipient = Msg.Attrs.find(RECIPIENT_EMAIL)->second;
            Message.setSender(Sender_);
            Message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, Recipient));
            Message.setSubject(Msg.Attrs.find(SUBJECT)->second);

            if(Msg.Attrs.find(TEXT) != Msg.Attrs.end()) {
                std::string Content = Msg.Attrs.find(TEXT)->second;
                Message.addContent(new Poco::Net::StringPartSource(Content));
            } else {
                std::string Content = Utils::LoadFile(Msg.File);
                Types::StringPairVec    Variables;
                FillVariables(Msg.Attrs, Variables);
                Utils::ReplaceVariables(Content, Variables);
                Message.addContent(new Poco::Net::StringPartSource(Content));
            }

            auto Logo = Msg.Attrs.find(LOGO);
            if(Logo!=Msg.Attrs.end()) {
                Poco::File  LogoFile(TemplateDir_ + "/" + Logo->second);
                std::ifstream   IF(LogoFile.path());
                std::ostringstream   OS;
                Poco::StreamCopier::copyStream(IF, OS);
                Message.addAttachment("logo", new Poco::Net::StringPartSource(OS.str(), "image/jpeg"));
            }

            Poco::Net::SecureSMTPClientSession session(MailHost_,MailHostPort_);
            Poco::Net::Context::Params P;
            auto ptrContext = Poco::AutoPtr<Poco::Net::Context>
                    (new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "",
                                                            Poco::Net::Context::VERIFY_RELAXED, 9, true,
                                                            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
            Poco::Net::SSLManager::instance().initializeClient(nullptr,
                                                               ptrHandler,
                                                               ptrContext);
            session.login();
            session.startTLS(ptrContext);
            session.login(MailHost_,
                          Poco::Net::SecureSMTPClientSession::AUTH_LOGIN,
                          SenderLoginUserName_,
                          SenderLoginPassword_
            );
            session.sendMessage(Message);
            session.close();
            return true;
        }
        catch (const Poco::Exception& E)
        {
            Logger_.log(E);
        }
        return false;
    }

}
