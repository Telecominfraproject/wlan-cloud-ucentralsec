//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

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

    class UserCache : public ORM::DBCache<SecurityObjects::UserInfo> {
    public:
        UserCache(unsigned Size, unsigned TimeOut, bool Users);
        void UpdateCache(const SecurityObjects::UserInfo &R) override;
        void Create(const SecurityObjects::UserInfo &R) override;
        bool GetFromCache(const std::string &FieldName, const std::string &Value, SecurityObjects::UserInfo &R) override;
        void Delete(const std::string &FieldName, const std::string &Value) override;
    private:
        std::mutex  Mutex_;
        bool        UsersOnly_;
        std::unique_ptr<Poco::ExpireLRUCache<std::string,SecurityObjects::UserInfo>>    CacheById_;
        std::unique_ptr<Poco::ExpireLRUCache<std::string,std::string>>                  CacheByEMail_;

    };

    class BaseUserDB : public ORM::DB<UserInfoRecordTuple, SecurityObjects::UserInfo> {
    public:
        BaseUserDB( const std::string &name, const std::string &shortname, OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L, UserCache * Cache, bool users);

        bool CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready = false );
        bool GetUserByEmail(const std::string & email, SecurityObjects::UserInfo & User);
        bool GetUserById(const std::string &Id, SecurityObjects::UserInfo &User);
        bool GetUsers( uint64_t Offset, uint64_t HowMany, SecurityObjects::UserInfoVec & Users, std::string WhereClause="");
        bool UpdateUserInfo(const std::string & Admin, SecurityObjects::USER_ID_TYPE & Id, SecurityObjects::UserInfo &UInfo);
        bool DeleteUser(const std::string & Admin, SecurityObjects::USER_ID_TYPE & Id);
        bool DeleteUsers(const std::string & Admin, std::string & owner);
        bool SetLastLogin(const std::string &Id);
        bool SetAvatar(const std::string &Id, const std::string &Value);

        bool UsersOnly_;
    };

}

