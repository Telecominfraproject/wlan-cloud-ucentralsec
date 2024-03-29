//
// Created by stephane bourque on 2021-10-15.
//

#include "SMS_provider_twilio.h"

#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/URI.h"

#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {
	bool SMS_provider_twilio::Initialize() {
		Sid_ = MicroServiceConfigGetString("smssender.twilio.sid", "");
		Token_ = MicroServiceConfigGetString("smssender.twilio.token", "");
		PhoneNumber_ = MicroServiceConfigGetString("smssender.twilio.phonenumber", "");

		if (Sid_.empty() || Token_.empty() || PhoneNumber_.empty()) {
			poco_debug(Logger(),
					   "SMSSender is disabled. Please provide SID, TOKEN, and PHONE NUMBER.");
			return false;
		}
		Running_ = true;
		Uri_ = "https://api.twilio.com/2010-04-01/Accounts/" + Sid_ + "/Messages.json";
		return true;
	}

	bool SMS_provider_twilio::Start() { return true; }

	bool SMS_provider_twilio::Stop() { return true; }

	bool SMS_provider_twilio::Running() { return Running_; }

	bool SMS_provider_twilio::Send(const std::string &PhoneNumber, const std::string &Message) {
		if (!Running_)
			return false;

		Poco::Net::HTTPBasicCredentials Creds(Sid_, Token_);
		Poco::URI uri(Uri_);
		Poco::Net::HTMLForm form;

		Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, uri.getPath(),
								   Poco::Net::HTTPMessage::HTTP_1_1);
		Creds.authenticate(req);

		Poco::JSON::Object RObj;

		form.add("To", PhoneNumber);
		form.add("From", PhoneNumber_);
		form.add("Body", Message);

		form.prepareSubmit(req);
		std::ostream &ostr = session.sendRequest(req);
		form.write(ostr);

		Poco::Net::HTTPResponse res;
		std::istream &rs = session.receiveResponse(res);

		if (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
			poco_information(Logger(), fmt::format("Message sent to {}", PhoneNumber));
			return true;
		} else {
			std::ostringstream os;
			Poco::StreamCopier::copyStream(rs, os);
			poco_information(Logger(),
							 fmt::format("Message was not to {}: Error:{}", PhoneNumber, os.str()));
			return false;
		}
	}
} // namespace OpenWifi