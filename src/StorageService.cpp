//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace OpenWifi {

    int Storage::Start() {
		std::lock_guard		Guard(Mutex_);

		StorageClass::Start();
		Create_Tables();
		InitializeDefaultUser();

		Archivercallback_ = std::make_unique<Poco::TimerCallback<Archiver>>(Archiver_,&Archiver::onTimer);
		Timer_.setStartInterval( 30 * 1000);  // first run in 5 minutes
		Timer_.setPeriodicInterval(60 * 1000); // 1 hours
		Timer_.start(*Archivercallback_);

		return 0;
    }

    void Storage::Stop() {
        Logger_.notice("Stopping.");
        Timer_.stop();
        StorageClass::Stop();
    }

    void Archiver::onTimer(Poco::Timer &timer) {
        std::cout << "Timer fired..." << std::endl;
    }

}
// namespace