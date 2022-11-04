//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"
#include "SpecialUserHelpers.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"

namespace OpenWifi {

    int StorageService::Start() {
		std::lock_guard		Guard(Mutex_);
        poco_information(Logger(),"Starting...");

		StorageClass::Start();

        UserCache_ = std::make_unique<OpenWifi::UserCache>(64,1200000,true);
        SubCache_ = std::make_unique<OpenWifi::UserCache>(2048,1200000,false);
        UserTokenCache_ = std::make_unique<OpenWifi::TokenCache>(64,1200000, true);
        SubTokenCache_ = std::make_unique<OpenWifi::TokenCache>(2048,1200000,false);

        UserDB_ = std::make_unique<OpenWifi::BaseUserDB>("Users", "usr", dbType_,*Pool_, Logger(), UserCache_.get(), true);
        SubDB_ = std::make_unique<OpenWifi::BaseUserDB>("Subscribers", "sub", dbType_,*Pool_, Logger(), SubCache_.get(), false);
        UserTokenDB_ = std::make_unique<OpenWifi::BaseTokenDB>("Tokens", "tok", dbType_,*Pool_, Logger(), UserTokenCache_.get(), true);
        SubTokenDB_ = std::make_unique<OpenWifi::BaseTokenDB>("SubTokens", "stk", dbType_,*Pool_, Logger(), SubTokenCache_.get(), false);

        PreferencesDB_ = std::make_unique<OpenWifi::PreferencesDB>("Preferences", "pre", dbType_,*Pool_, Logger());
        SubPreferencesDB_ = std::make_unique<OpenWifi::PreferencesDB>("SubPreferences", "prs", dbType_,*Pool_, Logger());
        ActionLinksDB_ = std::make_unique<OpenWifi::ActionLinkDB>("Actions", "act", dbType_,*Pool_, Logger());
        AvatarDB_ = std::make_unique<OpenWifi::AvatarDB>("Avatars2", "ava", dbType_,*Pool_, Logger());
        SubAvatarDB_ = std::make_unique<OpenWifi::AvatarDB>("SubAvatars", "avs", dbType_,*Pool_, Logger());
        LoginDB_ = std::make_unique<OpenWifi::LoginDB>("Logins", "lin", dbType_,*Pool_, Logger());
        SubLoginDB_ = std::make_unique<OpenWifi::LoginDB>("SubLogins", "lis", dbType_,*Pool_, Logger());

        UserDB_->Create();
        SubDB_->Create();
        UserTokenDB_->Create();
        SubTokenDB_->Create();
        ActionLinksDB_->Create();
        PreferencesDB_->Create();
        SubPreferencesDB_->Create();
        AvatarDB_->Create();
        SubAvatarDB_->Create();
        LoginDB_->Create();
        SubLoginDB_->Create();

		OpenWifi::SpecialUserHelpers::InitializeDefaultUser();

		Archivercallback_ = std::make_unique<Poco::TimerCallback<Archiver>>(Archiver_,&Archiver::onTimer);
		Timer_.setStartInterval( 5 * 60 * 1000);  // first run in 5 minutes
		Timer_.setPeriodicInterval(1 * 60 * 60 * 1000); // 1 hours
		Timer_.start(*Archivercallback_, MicroServiceTimerPool());

		return 0;
    }

    void StorageService::Stop() {
        poco_information(Logger(),"Stopping...");
        Timer_.stop();
        StorageClass::Stop();
        poco_information(Logger(),"Stopped...");
    }

    void Archiver::onTimer([[maybe_unused]] Poco::Timer &timer) {
        Utils::SetThreadName("strg-arch");
        Poco::Logger &logger = Poco::Logger::get("STORAGE-ARCHIVER");
        logger.information("Squiggy the DB: removing old tokens.");
        StorageService()->SubTokenDB().CleanExpiredTokens();
        StorageService()->UserTokenDB().CleanExpiredTokens();
        logger.information("Squiggy the DB: removing old actionLinks.");
        StorageService()->ActionLinksDB().CleanOldActionLinks();
    }

}
// namespace