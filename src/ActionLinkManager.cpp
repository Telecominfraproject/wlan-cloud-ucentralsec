//
// Created by stephane bourque on 2021-11-08.
//

#include "ActionLinkManager.h"
#include "MessagingTemplates.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "fmt/format.h"
#include "framework/utils.h"

namespace OpenWifi {

	int ActionLinkManager::Start() {
		poco_information(Logger(), "Starting...");
		if (!Running_)
			Thr_.start(*this);
		return 0;
	}

	void ActionLinkManager::Stop() {
		poco_information(Logger(), "Stopping...");
		if (Running_) {
			Running_ = false;
			Thr_.wakeUp();
			Thr_.join();
		}
		poco_information(Logger(), "Stopped...");
	}

#define DBG         std::cout << __LINE__ << std::endl;

    void ActionLinkManager::run() {
		Running_ = true;
		Utils::SetThreadName("action-mgr");

		while (Running_) {
			Poco::Thread::trySleep(2000);
            DBG
			if (!Running_)
				break;
			std::vector<SecurityObjects::ActionLink> Links;
			{
				std::lock_guard G(Mutex_);
                DBG
				StorageService()->ActionLinksDB().GetActions(Links);
                DBG
			}

			if (Links.empty())
				continue;

			for (auto &i : Links) {
				if (!Running_)
					break;

				SecurityObjects::UserInfo UInfo;
                DBG
				if ((i.action == OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD ||
					 i.action == OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL) &&
					!StorageService()->UserDB().GetUserById(i.userId, UInfo)) {
                    DBG
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBG
					continue;
				} else if ((i.action ==
								OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD ||
							i.action == OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL ||
							i.action == OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP) &&
						   !StorageService()->SubDB().GetUserById(i.userId, UInfo)) {
                    DBG
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBG
					continue;
				} else if ((i.action == OpenWifi::SecurityObjects::LinkActions::EMAIL_INVITATION) &&
						   (OpenWifi::Now() - i.created) > (24 * 60 * 60)) {
                    DBG
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBG
					continue;
				}

				switch (i.action) {
				case OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD: {
                    DBG
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::FORGOT_PASSWORD)) {
						poco_information(
							Logger(), fmt::format("Send password reset link to {}", UInfo.email));
					}
                    DBG
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBG
				} break;

				case OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL: {
                    DBG
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::EMAIL_VERIFICATION)) {
                        DBG
						poco_information(Logger(), fmt::format("Send email verification link to {}",
															   UInfo.email));
					}
                    DBG
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBG
				} break;

				case OpenWifi::SecurityObjects::LinkActions::EMAIL_INVITATION: {
                    DBG
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::EMAIL_INVITATION)) {
                        DBG
						poco_information(
							Logger(), fmt::format("Send new subscriber email invitation link to {}",
												  UInfo.email));
					}
                    DBG
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBG
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD: {
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
					if (AuthService()->SendEmailToSubUser(i.id, UInfo.email,
														  MessagingTemplates::SUB_FORGOT_PASSWORD,
														  Signup.count() == 1 ? "" : Signup[0])) {
						poco_information(
							Logger(),
							fmt::format("Send subscriber password reset link to {}", UInfo.email));
					}
					StorageService()->ActionLinksDB().SentAction(i.id);
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL: {
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
					if (AuthService()->SendEmailToSubUser(
							i.id, UInfo.email, MessagingTemplates::SUB_EMAIL_VERIFICATION,
							Signup.count() == 1 ? "" : Signup[0])) {
						poco_information(
							Logger(), fmt::format("Send subscriber email verification link to {}",
												  UInfo.email));
					}
					StorageService()->ActionLinksDB().SentAction(i.id);
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP: {
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
					if (AuthService()->SendEmailToSubUser(
							i.id, UInfo.email, MessagingTemplates::SUB_SIGNUP_VERIFICATION,
							Signup.count() == 1 ? "" : Signup[0])) {
						poco_information(
							Logger(),
							fmt::format("Send new subscriber email verification link to {}",
										UInfo.email));
					}
					StorageService()->ActionLinksDB().SentAction(i.id);
				} break;

				default: {
					StorageService()->ActionLinksDB().SentAction(i.id);
				}
				}
			}
		}
	}

} // namespace OpenWifi