//
// Created by stephane bourque on 2021-12-28.
//

#pragma once

#include "StorageService.h"
#include "framework/AppServiceRegistry.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	namespace SpecialUserHelpers {
		static inline std::string NewDefaultUseridStockUUID{"11111111-0000-0000-6666-999999999999"};

		inline bool InitializeDefaultUser() {
			SecurityObjects::UserInfo U;
			bool DefaultUserCreated = false;

			AppServiceRegistry().Get("defaultusercreated", DefaultUserCreated);
			if (!StorageService()->UserDB().GetUserById(NewDefaultUseridStockUUID, U) &&
				!DefaultUserCreated) {
				U.currentPassword =
					MicroServiceConfigGetString("authentication.default.password", "");
				U.lastPasswords.push_back(U.currentPassword);
				U.email = MicroServiceConfigGetString("authentication.default.username", "");
				U.id = NewDefaultUseridStockUUID;
				U.userRole = SecurityObjects::ROOT;
				U.creationDate = OpenWifi::Now();
				U.validated = true;
				U.name = "Default User";
				U.description = "Default user should be deleted.";
				U.changePassword = true;
				StorageService()->UserDB().CreateUser("SYSTEM", U, true);
				AppServiceRegistry().Set("defaultusercreated", true);
				return true;
			}
			return false;
		}
	} // namespace SpecialUserHelpers
} // namespace OpenWifi