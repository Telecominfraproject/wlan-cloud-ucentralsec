//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/StorageClass.h"
#include "AuthService.h"

#include "Poco/Timer.h"

#include "storage/orm_users.h"
#include "storage/orm_tokens.h"

namespace OpenWifi {

    class Archiver {
    public:
        void onTimer(Poco::Timer & timer);
    private:
    };

    class StorageService : public StorageClass {
    public:

        static auto instance() {
            static auto instance_ = new StorageService;
            return instance_;
        }

        int 	Start() override;
        void 	Stop() override;

        OpenWifi::BaseUserDB & UserDB() { return *UserDB_; }
        OpenWifi::BaseUserDB & SubDB() { return *SubDB_; }
        OpenWifi::BaseTokenDB & UserTokenDB() { return *UserTokenDB_; }
        OpenWifi::BaseTokenDB & SubTokenDB() { return *SubTokenDB_; }

        /*
         *  All user management functions
         */
        bool SetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type, std::string & Name);
        bool GetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type, std::string & Name);
        bool DeleteAvatar(const std::string & Admin, std::string &Id);

        /*
         *  All ActionLinks functions
         */
        bool CreateAction( SecurityObjects::ActionLink & A);
        bool DeleteAction(std::string &ActionId);
        bool CompleteAction(std::string &ActionId);
        bool CancelAction(std::string &ActionId);
        bool SentAction(std::string &ActionId);
        bool GetActionLink(std::string &ActionId, SecurityObjects::ActionLink &A);
        bool GetActions(std::vector<SecurityObjects::ActionLink> &Links, uint64_t Max=200);
        void CleanOldActionLinks();

        bool GetPreferences(std::string &Id, SecurityObjects::Preferences &P);
        bool SetPreferences(SecurityObjects::Preferences &P);
        bool DeletePreferences(const std::string &AdminId, std::string & Id);

	  private:
        int Create_Tables();
        int Create_UserTable();
        int Create_AvatarTable();
        int Create_TokensTable();
        int Create_ActionLinkTable();
        int Create_Preferences();
        int Create_SubTokensTable();
        int Create_SubscriberTable();

        std::unique_ptr<OpenWifi::BaseUserDB>           UserDB_;
        std::unique_ptr<OpenWifi::BaseUserDB>           SubDB_;
        std::unique_ptr<OpenWifi::BaseTokenDB>          UserTokenDB_;
        std::unique_ptr<OpenWifi::BaseTokenDB>          SubTokenDB_;


        Poco::Timer                     Timer_;
        Archiver                        Archiver_;
        std::unique_ptr<Poco::TimerCallback<Archiver>>   Archivercallback_;

        /// This is to support a mistake that was deployed...
        void ReplaceOldDefaultUUID();
   };

    inline auto StorageService() { return StorageService::instance(); };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
