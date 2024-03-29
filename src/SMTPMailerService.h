//
// Created by stephane bourque on 2021-06-17.
//

#pragma once

#include "Poco/File.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/InvalidCertificateHandler.h"

#include "framework/SubSystemServer.h"

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
		SENDER,
		ACTION_LINK_HTML,
		USER_HELPER_EMAIL,
		SUB_HELPER_EMAIL,
		GLOBAL_USER_HELPER_EMAIL,
		GLOBAL_SUB_HELPER_EMAIL,
		USER_HELPER_SITE,
		SUB_HELPER_SITE,
		USER_SYSTEM_LOGIN,
		SUB_SYSTEM_LOGIN,
		USER_SIGNATURE,
		SUB_SIGNATURE,
		TRANSFER_REQUESTER,
		TRANSFER_ENTITY,
		ORIGINAL_REDIRECTOR,
		NEW_REDIRECTOR,
		TRANSFER_REASON,
		SERIAL_NUMBER,
		ORIGINAL_ENTITY_NAME,
		UUID
	};

	static const std::map<MESSAGE_ATTRIBUTES, const std::string> MessageAttributeMap{
		{RECIPIENT_EMAIL, "RECIPIENT_EMAIL"},
		{RECIPIENT_FIRST_NAME, "RECIPIENT_FIRST_NAME"},
		{RECIPIENT_LAST_NAME, "RECIPIENT_LAST_NAME"},
		{RECIPIENT_INITIALS, "RECIPIENT_INITIALS"},
		{RECIPIENT_FULL_NAME, "RECIPIENT_FULL_NAME"},
		{RECIPIENT_SALUTATION, "RECIPIENT_SALUTATION"},
		{ACTION_LINK, "ACTION_LINK"},
		{SUBJECT, "SUBJECT"},
		{TEMPLATE_TXT, "TEMPLATE_TXT"},
		{TEMPLATE_HTML, "TEMPLATE_HTML"},
		{LOGO, "LOGO"},
		{TEXT, "TEXT"},
		{CHALLENGE_CODE, "CHALLENGE_CODE"},
		{SENDER, "SENDER"},
		{ACTION_LINK_HTML, "SUB_SYSTEM_LOGIN"},
		{USER_HELPER_EMAIL, "USER_HELPER_EMAIL"},
		{SUB_HELPER_EMAIL, "SUB_HELPER_EMAIL"},
		{GLOBAL_USER_HELPER_EMAIL, "GLOBAL_USER_HELPER_EMAIL"},
		{GLOBAL_SUB_HELPER_EMAIL, "GLOBAL_SUB_HELPER_EMAIL"},
		{USER_HELPER_SITE, "USER_HELPER_SITE"},
		{SUB_HELPER_SITE, "SUB_USER_HELPER_SITE"},
		{USER_SYSTEM_LOGIN, "USER_SYSTEM_LOGIN"},
		{SUB_SYSTEM_LOGIN, "SUB_SYSTEM_LOGIN"},
		{USER_SIGNATURE, "USER_SIGNATURE"},
		{SUB_SIGNATURE, "SUB_USER_SIGNATURE"},
		{TRANSFER_REQUESTER, "TRANSFER_REQUESTER"},
		{TRANSFER_ENTITY, "TRANSFER_ENTITY"},
		{ORIGINAL_REDIRECTOR, "ORIGINAL_REDIRECTOR"},
		{NEW_REDIRECTOR, "NEW_REDIRECTOR"},
		{TRANSFER_REASON, "TRANSFER_REASON"},
		{SERIAL_NUMBER, "SERIAL_NUMBER"},
		{ORIGINAL_ENTITY_NAME, "ORIGINAL_ENTITY_NAME"},
		{UUID, "UUID"}};

	inline const std::string &MessageAttributeToVar(MESSAGE_ATTRIBUTES Attr) {
		static const std::string EmptyString{};
		auto E = MessageAttributeMap.find(Attr);
		if (E == MessageAttributeMap.end())
			return EmptyString;
		return E->second;
	}
	typedef std::map<MESSAGE_ATTRIBUTES, std::string> MessageAttributes;

	enum class MessageSendStatus {
		msg_sent,
		msg_not_sent_but_resend,
		msg_not_sent_but_do_not_resend
	};

	class SMTPMailerService : public SubSystemServer, Poco::Runnable {
	  public:
		static SMTPMailerService *instance() {
			static auto *instance_ = new SMTPMailerService;
			return instance_;
		}

		struct MessageEvent {
			uint64_t Posted = 0;
			uint64_t LastTry = 0;
			uint64_t Sent = 0;
			std::string TemplateName;
			MessageAttributes Attrs;
			bool Subscriber = false;
		};

		void run() override;
		int Start() override;
		void Stop() override;

		bool SendMessage(const std::string &Recipient, const std::string &Name,
						 const MessageAttributes &Attrs, bool Subscriber);
		MessageSendStatus SendIt(const MessageEvent &Msg);
		void LoadMyConfig();
		void reinitialize(Poco::Util::Application &self) override;
		bool Enabled() const { return Enabled_; }

		void AddUserVars(MessageAttributes &Attrs);
		void AddSubVars(MessageAttributes &Attrs);

	  private:
		std::string MailHost_;
		std::string Sender_;
		uint32_t MailHostPort_ = 25;
		uint64_t MailRetry_ = 2 * 60;
		uint64_t MailAbandon_ = 2 * 60 * 20;
		std::string SenderLoginUserName_;
		std::string SenderLoginPassword_;
		std::string LoginMethod_ = "login";
		std::string TemplateDir_;
		std::list<MessageEvent> Messages_;
		std::list<MessageEvent> PendingMessages_;
		Poco::Thread SenderThr_;
		std::atomic_bool Running_ = false;
		bool Enabled_ = false;
		bool UseHTML_ = false;
		std::string LogoFilename;
		std::string SubLogoFilename;

		SMTPMailerService() noexcept : SubSystemServer("SMTPMailer", "MAILER-SVR", "smtpmailer") {}
	};

	inline SMTPMailerService *SMTPMailerService() { return SMTPMailerService::instance(); }
} // namespace OpenWifi
