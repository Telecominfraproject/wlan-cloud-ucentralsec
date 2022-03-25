//
// Created by stephane bourque on 2022-01-01.
//

#pragma once

#include "framework/orm.h"

namespace OpenWifi {

    inline void Sanitize([[maybe_unused]] const SecurityObjects::UserInfoAndPolicy &User, SecurityObjects::UserInfo & U) {
        U.currentPassword.clear();
        U.lastPasswords.clear();
        U.oauthType.clear();
    }

}