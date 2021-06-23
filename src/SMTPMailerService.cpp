//
// Created by stephane bourque on 2021-06-17.
//
#include <iostream>

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
#include "Daemon.h"

namespace uCentral {

    class SMTPMailerService * SMTPMailerService::instance_ = nullptr;

    int SMTPMailerService::Start() {
        MailHost_ = Daemon()->ConfigGetString("mailer.hostname");
        SenderLoginUserName_ = Daemon()->ConfigGetString("mailer.username");
        SenderLoginPassword_ = Daemon()->ConfigGetString("mailer.password");
        LoginMethod_ = Daemon()->ConfigGetString("mailer.loginmethod");
        MailHostPort_ = Daemon()->ConfigGetInt("mailer.port");
        return 0;
    }

    void SMTPMailerService::Stop() {

    }

    bool SMTPMailerService::SendMessage(SMTPMailerService::MessageAttributes Attrs) {
        return false;
    }

    bool SMTPMailerService::SendIt() {
        try
        {
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrHandler = new Poco::Net::AcceptCertificateHandler(false);

            /*
            Poco::Net::MailMessage message;
            message.setSender(Sender_);
            message.addRecipient(MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipient));
            message.setSubject("Hello from the POCO C++ Libraries");

            std::string content;

            content += "Hello ";
            content += recipient;
            content += ",\r\n\r\n";
            content += "This is a greeting from the POCO C++ Libraries.\r\n\r\n";
            std::string logo(reinterpret_cast<const char*>(PocoLogo), sizeof(PocoLogo));
            message.addContent(new Poco::Net::StringPartSource(content));
            message.addAttachment("logo", new Poco::Net::StringPartSource(logo, "image/gif"));

            Poco::Net::SecureSMTPClientSession session(MailHost_,MailHostPort_);

            Poco::Net::Context::Params P;
            auto ptrContext = new Poco::Net::Context(       Poco::Net::Context::CLIENT_USE, "", "", "",
                                                            Poco::Net::Context::VERIFY_RELAXED, 9, true,
                                                            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

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
            session.sendMessage(message);
            session.close();
             */
        }
        catch (const Poco::Exception& exc)
        {
            std::cerr << exc.displayText() << std::endl;
            return 1;
        }
        return 0;

    }

}