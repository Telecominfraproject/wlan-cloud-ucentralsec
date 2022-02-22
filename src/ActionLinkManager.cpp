//
// Created by stephane bourque on 2021-11-08.
//

#include "ActionLinkManager.h"
#include "StorageService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    int ActionLinkManager::Start() {
        if(!Running_)
            Thr_.start(*this);
        return 0;
    }

    void ActionLinkManager::Stop() {
        if(Running_) {
            Running_ = false;
            Thr_.wakeUp();
            Thr_.join();
        }
    }

    void ActionLinkManager::run() {
        Running_ = true ;

        while(Running_) {
            Poco::Thread::trySleep(2000);
            if(!Running_)
                break;
            std::vector<SecurityObjects::ActionLink>    Links;
            {
                std::lock_guard G(Mutex_);
                StorageService()->ActionLinksDB().GetActions(Links);
            }

            if(Links.empty())
                continue;

            for(auto &i:Links) {
                std::cout << "Sending email " << __LINE__ << std::endl;
                if(!Running_)
                    break;

                std::cout << "Sending email " << __LINE__ << std::endl;
                SecurityObjects::UserInfo UInfo;
                std::cout << "Sending email " << __LINE__ << std::endl;
                if((i.action==OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD ||
                    i.action==OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL) && !StorageService()->UserDB().GetUserById(i.userId,UInfo)) {
                    StorageService()->ActionLinksDB().CancelAction(i.id);
                    continue;
                } else if(( i.action==OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD ||
                            i.action==OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL) && !StorageService()->SubDB().GetUserById(i.userId,UInfo)) {
                    StorageService()->ActionLinksDB().CancelAction(i.id);
                    continue;
                }

                std::cout << "Sending email " << __LINE__ << std::endl;
                switch(i.action) {
                    case OpenWifi::SecurityObjects::LinkActions::FORGOT_PASSWORD: {
                            if(AuthService::SendEmailToUser(i.id, UInfo.email, AuthService::FORGOT_PASSWORD)) {
                                Logger().information(Poco::format("Send password reset link to %s",UInfo.email));
                            }
                            StorageService()->ActionLinksDB().SentAction(i.id);
                        }
                        break;

                    case OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL: {
                            if(AuthService::SendEmailToUser(i.id, UInfo.email, AuthService::EMAIL_VERIFICATION)) {
                                Logger().information(Poco::format("Send email verification link to %s",UInfo.email));
                            }
                            StorageService()->ActionLinksDB().SentAction(i.id);
                        }
                        break;

                    case OpenWifi::SecurityObjects::LinkActions::SUB_FORGOT_PASSWORD: {
                            if(AuthService::SendEmailToSubUser(i.id, UInfo.email, AuthService::FORGOT_PASSWORD)) {
                                Logger().information(Poco::format("Send subscriber password reset link to %s",UInfo.email));
                            }
                            StorageService()->ActionLinksDB().SentAction(i.id);
                        }
                        break;

                    case OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL: {
                            if(AuthService::SendEmailToSubUser(i.id, UInfo.email, AuthService::EMAIL_VERIFICATION)) {
                                Logger().information(Poco::format("Send subscriber email verification link to %s",UInfo.email));
                            }
                            StorageService()->ActionLinksDB().SentAction(i.id);
                        }
                        break;

                    case OpenWifi::SecurityObjects::LinkActions::SUB_SIGNUP: {
                        std::cout << "Sending email " << __LINE__ << std::endl;
                        if(AuthService::SendEmailToSubUser(i.id, UInfo.email, AuthService::SIGNUP_VERIFICATION)) {
                            std::cout << "Sending email " << __LINE__ << std::endl;
                            Logger().information(Poco::format("Send new subscriber email verification link to %s",UInfo.email));
                        }
                        std::cout << "Sending email " << __LINE__ << std::endl;
                        StorageService()->ActionLinksDB().SentAction(i.id);
                        }
                        break;

                    default: {
                        std::cout << "Sending email " << __LINE__ << std::endl;
                        StorageService()->ActionLinksDB().SentAction(i.id);
                    }
                }
            }
        }
    }

}