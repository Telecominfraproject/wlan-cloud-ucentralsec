//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    enum USER_TYPE {
        UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, SPECIAL
    };

    typedef std::string USER_ID_TYPE;

    inline USER_TYPE to_userType(const std::string &U) {
        if (U=="root")
            return ROOT;
        else if (U=="admin")
            return ADMIN;
        else if (U=="subscriber")
            return SUBSCRIBER;
        else if (U=="csr")
            return CSR;
        else if (U=="system")
            return SYSTEM;
        else if (U=="SPECIAL")
            return SPECIAL;
        return UNKNOWN;
    }

    inline std::string from_userType(USER_TYPE U) {
        switch(U) {
            case ROOT: return "root";
            case ADMIN: return "admin";
            case SUBSCRIBER: return "subscriber";
            case CSR: return "csr";
            case SYSTEM: return "system";
            case SPECIAL: return "special";
            case UNKNOWN:
            default: return "unknown";
        }
    }

    typedef Poco::Tuple<
            std::string,    // Id = 0;
            std::string,    // name;
            std::string,    // description;
            std::string,    // avatar;
            std::string,    // email;
            bool,           // bool validated = false;
            std::string,    // validationEmail;
            uint64_t,       // validationDate = 0;
            uint64_t,       // creationDate = 0;
            std::string,    // validationURI;
            bool,           // bool changePassword = true;
            uint64_t,       // lastLogin = 0;
            std::string,    // currentLoginURI;
            uint64_t,       // lastPasswordChange = 0;
            uint64_t,       // lastEmailCheck = 0;
            bool,           // bool waitingForEmailCheck = false;
            std::string,    // locale;
            std::string,    // notes;
            std::string,    // location;
            std::string,    // owner;
            bool,           // bool suspended = false;
            bool,           // bool blackListed = false;
            std::string,    // userRole;
            std::string,    // userTypeProprietaryInfo;
            std::string,    // securityPolicy;
            uint64_t,       // securityPolicyChange;
            std::string,    // currentPassword;
            std::string,    // lastPasswords;
            std::string,    // oauthType;
            std::string     // oauthUserInfo;
    > UserInfoRecordTuple;

    typedef std::vector <UserInfoRecordTuple> UserInfoRecordTupleList;

    class BaseUserDB : public ORM::DB<UserInfoRecordTuple, SecurityObjects::UserInfo> {
    public:
        BaseUserDB( const std::string &name, const std::string &shortname, OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);

        void ReplaceOldDefaultUUID();
        bool InitializeDefaultUser();
        bool CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready = false );
        bool GetUserByEmail(const std::string & email, SecurityObjects::UserInfo & User);
        bool GetUserById(const std::string &Id, SecurityObjects::UserInfo &User);
        bool GetUsers( uint64_t Offset, uint64_t HowMany, SecurityObjects::UserInfoVec & Users);
        bool UpdateUserInfo(const std::string & Admin, USER_ID_TYPE & Id, SecurityObjects::UserInfo &UInfo);
        bool DeleteUser(const std::string & Admin, USER_ID_TYPE & Id);
        bool SetLastLogin(const std::string &Id);

    private:
    };

}

