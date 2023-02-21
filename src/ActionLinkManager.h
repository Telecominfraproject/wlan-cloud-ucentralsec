//
// Created by stephane bourque on 2021-11-08.
//

#pragma once

#include "framework/SubSystemServer.h"

namespace OpenWifi {

	class ActionLinkManager : public SubSystemServer, Poco::Runnable {
	  public:
		static ActionLinkManager *instance() {
			static auto instance_ = new ActionLinkManager;
			return instance_;
		}

		int Start() final;
		void Stop() final;
		void run() final;

	  private:
		Poco::Thread Thr_;
		std::atomic_bool Running_ = false;

		ActionLinkManager() noexcept
			: SubSystemServer("ActionLinkManager", "ACTION-SVR", "action.server") {}
	};
	inline ActionLinkManager *ActionLinkManager() { return ActionLinkManager::instance(); }
} // namespace OpenWifi
