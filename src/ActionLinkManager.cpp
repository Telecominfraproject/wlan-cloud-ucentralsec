//
// Created by stephane bourque on 2021-11-08.
//

#include "ActionLinkManager.h"
#include "StorageService.h"

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
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            Poco::Thread::trySleep(2000);
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            if(!Running_)
                break;
            std::cout << __func__ << ":" << __LINE__ << std::endl;
            std::vector<SecurityObjects::ActionLink>    Links;
            {
                std::lock_guard G(Mutex_);
                StorageService()->GetActions(Links);
            }
            std::cout << __func__ << ":" << __LINE__ << std::endl;

            if(Links.empty())
                continue;
            std::cout << __func__ << ":" << __LINE__ << std::endl;

            for(auto &i:Links) {
                std::cout << __func__ << ":" << __LINE__ << std::endl;

                if(!Running_)
                    break;

                std::cout << __func__ << ":" << __LINE__ << std::endl;
                if(i.action=="forgot_password") {
                    if(AuthService::SendEmailToUser(i.id, i.userId, AuthService::FORGOT_PASSWORD)) {
                        Logger_.information(Poco::format("Send password reset link to %s",i.userId));
                    }
                    StorageService()->SentAction(i.id);
                } else if (i.action=="email_verification") {
                    if(AuthService::SendEmailToUser(i.id, i.userId, AuthService::EMAIL_VERIFICATION)) {
                        Logger_.information(Poco::format("Send password reset link to %s",i.userId));
                    }
                    StorageService()->SentAction(i.id);
                } else {
                    StorageService()->SentAction(i.id);
                }
            }
        }
    }

}