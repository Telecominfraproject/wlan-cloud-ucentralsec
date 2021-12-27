//
// Created by stephane bourque on 2021-11-30.
//

#pragma once
#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {
    inline bool Convert(const UserInfoRecordTuple &T, SecurityObjects::UserInfo &U) {
        U.Id = T.get<0>();
        U.name = T.get<1>();
        U.description = T.get<2>();
        U.avatar = T.get<3>();
        U.email = T.get<4>();
        U.validated = T.get<5>();
        U.validationEmail = T.get<6>();
        U.validationDate = T.get<7>();
        U.creationDate = T.get<8>();
        U.validationURI = T.get<9>();
        U.changePassword = T.get<10>();
        U.lastLogin = T.get<11>();
        U.currentLoginURI = T.get<12>();
        U.lastPasswordChange = T.get<13>();
        U.lastEmailCheck = T.get<14>();
        U.waitingForEmailCheck = T.get<15>();
        U.locale = T.get<16>();
        U.notes = RESTAPI_utils::to_object_array<SecurityObjects::NoteInfo>(T.get<17>());
        U.location = T.get<18>();
        U.owner = T.get<19>();
        U.suspended = T.get<20>();
        U.blackListed = T.get<21>();
        U.userRole = SecurityObjects::UserTypeFromString(T.get<22>());
        U.userTypeProprietaryInfo = RESTAPI_utils::to_object<SecurityObjects::UserLoginLoginExtensions>(T.get<23>());
        U.securityPolicy = T.get<24>();
        U.securityPolicyChange = T.get<25>();
        U.currentPassword = T.get<26>();
        U.lastPasswords = RESTAPI_utils::to_object_array(T.get<27>());
        U.oauthType = T.get<28>();
        U.oauthUserInfo = T.get<29>();
        return true;
    }

    inline bool Convert(const SecurityObjects::UserInfo &U, UserInfoRecord &T) {
        T.set<0>(U.Id);
        T.set<1>(U.name);
        T.set<2>(U.description);
        T.set<3>(U.avatar);
        T.set<4>(U.email);
        T.set<5>(U.validated);
        T.set<6>(U.validationEmail);
        T.set<7>(U.validationDate);
        T.set<8>(U.creationDate);
        T.set<9>(U.validationURI);
        T.set<10>(U.changePassword);
        T.set<11>(U.lastLogin);
        T.set<12>(U.currentLoginURI);
        T.set<13>(U.lastPasswordChange);
        T.set<14>(U.lastEmailCheck);
        T.set<15>(U.waitingForEmailCheck);
        T.set<16>(U.locale);
        T.set<17>(RESTAPI_utils::to_string(U.notes));
        T.set<18>(U.location);
        T.set<19>(U.owner);
        T.set<20>(U.suspended);
        T.set<21>(U.blackListed);
        T.set<22>(SecurityObjects::UserTypeToString(U.userRole));
        T.set<23>(RESTAPI_utils::to_string(U.userTypeProprietaryInfo));
        T.set<24>(U.securityPolicy);
        T.set<25>(U.securityPolicyChange);
        T.set<26>(U.currentPassword);
        T.set<27>(RESTAPI_utils::to_string(U.lastPasswords));
        T.set<28>(U.oauthType);
        T.set<29>(U.oauthUserInfo);
        return true;
    }
}
