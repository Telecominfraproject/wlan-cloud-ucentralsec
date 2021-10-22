//
// Created by stephane bourque on 2021-10-15.
//

#include "SMS_provider_twilio.h"

#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/URI.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "framework/MicroService.h"

namespace OpenWifi {
    bool SMS_provider_twilio::Initialize() {
        Sid_ = MicroService::instance().ConfigGetString("smssender.twilio.sid","");
        Token_ = MicroService::instance().ConfigGetString("smssender.twilio.token","");
        PhoneNumber_ = MicroService::instance().ConfigGetString("smssender.twilio.phonenumber","");

        if(Sid_.empty() || Token_.empty() || PhoneNumber_.empty()) {
            Logger_.debug("SMSSender is disabled. Please provide SID, TOKEN, and PHONE NUMBER.");
            return false;
        }
        Running_=true;
        Uri_ = "https://api.twilio.com/2010-04-01/Accounts/" + Sid_ + "/Messages.json";
        return true;
    }

    bool SMS_provider_twilio::Start() {
        return true;
    }

    bool SMS_provider_twilio::Stop() {
        return true;
    }

    bool SMS_provider_twilio::Running() {
        return Running_;
    }

    bool SMS_provider_twilio::Send(const std::string &PhoneNumber, const std::string &Message) {
        if(!Running_)
            return false;

        Poco::Net::HTTPBasicCredentials Creds(Sid_,Token_);
        Poco::URI   uri(Uri_);
        Poco::Net::HTMLForm  form;

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_1);
        Creds.authenticate(req);

        Poco::JSON::Object  RObj;

        form.add("To",PhoneNumber);
        form.add("From",PhoneNumber_);
        form.add("Body","This is from twillio");

        form.prepareSubmit(req);
        std::ostream& ostr = session.sendRequest(req);
        form.write(ostr);

        Poco::Net::HTTPResponse res;
        std::istream& rs = session.receiveResponse(res);

        if(res.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
            Logger_.information(Poco::format("Message sent to %s", PhoneNumber));
            return true;
        } else {
            std::ostringstream os;
            Poco::StreamCopier::copyStream(rs,os);
            Logger_.information(Poco::format("Message was not to %s: Error:%s", PhoneNumber, os.str()));
            return false;
        }
    }
}