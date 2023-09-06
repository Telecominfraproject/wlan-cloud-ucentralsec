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


    void ActionLinkManager::run() {
		Running_ = true;
		Utils::SetThreadName("action-mgr");

        Poco::Thread::trySleep(10000);

		while (Running_) {
			Poco::Thread::trySleep(2000);
            DBGLINE
			if (!Running_)
				break;
			std::vector<SecurityObjects::ActionLink> Links;
			{
				std::lock_guard G(Mutex_);
                DBGLINE
				StorageService()->ActionLinksDB().GetActions(Links);
                DBGLINE
			}

			if (Links.empty())
				continue;

			for (auto &i : Links) {
				if (!Running_)
					break;

				SecurityObjects::UserInfo UInfo;
                DBGLINE
				if ((i.action == OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD ||
					 i.action == OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL) &&
					!StorageService()->UserDB().GetUserById(i.userId, UInfo)) {
                    DBGLINE
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBGLINE
					continue;
				} else if ((i.action ==
								OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD ||
							i.action == OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL ||
							i.action == OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP) &&
						   !StorageService()->SubDB().GetUserById(i.userId, UInfo)) {
                    DBGLINE
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBGLINE
					continue;
				} else if ((i.action == OpenWifi::SecurityObjects::LinkActions::EMAIL_INVITATION) &&
						   (OpenWifi::Now() - i.created) > (24 * 60 * 60)) {
                    DBGLINE
					StorageService()->ActionLinksDB().CancelAction(i.id);
                    DBGLINE
					continue;
				}

                DBGLINE
				switch (i.action) {
				case OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD: {
                    DBGLINE
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::FORGOT_PASSWORD)) {
						poco_information(
							Logger(), fmt::format("Send password reset link to {}", UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				case OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL: {
                    DBGLINE
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::EMAIL_VERIFICATION)) {
                        DBGLINE
						poco_information(Logger(), fmt::format("Send email verification link to {}",
															   UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				case OpenWifi::SecurityObjects::LinkActions::EMAIL_INVITATION: {
                    DBGLINE
					if (AuthService()->SendEmailToUser(i.id, UInfo.email,
													   MessagingTemplates::EMAIL_INVITATION)) {
                        DBGLINE
						poco_information(
							Logger(), fmt::format("Send new subscriber email invitation link to {}",
												  UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD: {
                    DBGLINE
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
                    DBGLINE
					if (AuthService()->SendEmailToSubUser(i.id, UInfo.email,
														  MessagingTemplates::SUB_FORGOT_PASSWORD,
														  Signup.count() == 1 ? "" : Signup[0])) {
                        DBGLINE
						poco_information(
							Logger(),
							fmt::format("Send subscriber password reset link to {}", UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL: {
                    DBGLINE
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
                    DBGLINE
					if (AuthService()->SendEmailToSubUser(
							i.id, UInfo.email, MessagingTemplates::SUB_EMAIL_VERIFICATION,
							Signup.count() == 1 ? "" : Signup[0])) {
                        DBGLINE
						poco_information(
							Logger(), fmt::format("Send subscriber email verification link to {}",
												  UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				case OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP: {
                    DBGLINE
					auto Signup = Poco::StringTokenizer(UInfo.signingUp, ":");
                    DBGLINE
					if (AuthService()->SendEmailToSubUser(
							i.id, UInfo.email, MessagingTemplates::SUB_SIGNUP_VERIFICATION,
							Signup.count() == 1 ? "" : Signup[0])) {
                        DBGLINE
						poco_information(
							Logger(),
							fmt::format("Send new subscriber email verification link to {}",
										UInfo.email));
					}
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				} break;

				default: {
                    DBGLINE
					StorageService()->ActionLinksDB().SentAction(i.id);
                    DBGLINE
				}
				}
			}
		}
	}

} // namespace OpenWifi