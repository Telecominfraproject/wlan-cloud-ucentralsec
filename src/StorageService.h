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
#include "storage/orm_preferences.h"
#include "storage/orm_actionLinks.h"
#include "storage/orm_avatar.h"

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
        OpenWifi::PreferencesDB & PreferencesDB() { return *PreferencesDB_; }
        OpenWifi::ActionLinkDB & ActionLinksDB() { return *ActionLinksDB_; }
        OpenWifi::AvatarDB & AvatarDB() { return *AvatarDB_; }

	  private:

        std::unique_ptr<OpenWifi::BaseUserDB>           UserDB_;
        std::unique_ptr<OpenWifi::BaseUserDB>           SubDB_;
        std::unique_ptr<OpenWifi::BaseTokenDB>          UserTokenDB_;
        std::unique_ptr<OpenWifi::BaseTokenDB>          SubTokenDB_;
        std::unique_ptr<OpenWifi::PreferencesDB>        PreferencesDB_;
        std::unique_ptr<OpenWifi::ActionLinkDB>         ActionLinksDB_;
        std::unique_ptr<OpenWifi::AvatarDB>             AvatarDB_;

        Poco::Timer                     Timer_;
        Archiver                        Archiver_;
        std::unique_ptr<Poco::TimerCallback<Archiver>>   Archivercallback_;

        /// This is to support a mistake that was deployed...
        void ReplaceOldDefaultUUID();
   };

    inline auto StorageService() { return StorageService::instance(); };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
