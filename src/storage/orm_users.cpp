//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_users.h"
#include "AuthService.h"
#include "RESTObjects/RESTAPI_CertObjects.h"
#include "StorageService.h"

/*
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
            uint64_t,       // modified
            std::string     // signingUp
 */

namespace OpenWifi {
    static ORM::FieldVec BaseUserDB_Fields{
            // object info
            ORM::Field{"id", 36, true},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"description", ORM::FieldType::FT_TEXT},
            ORM::Field{"avatar", ORM::FieldType::FT_TEXT},
            ORM::Field{"email", ORM::FieldType::FT_TEXT},
            ORM::Field{"validated", ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"validationEmail", ORM::FieldType::FT_TEXT},
            ORM::Field{"validationDate", ORM::FieldType::FT_BIGINT},
            ORM::Field{"creationDate", ORM::FieldType::FT_BIGINT},
            ORM::Field{"validationURI", ORM::FieldType::FT_TEXT},
            ORM::Field{"changePassword", ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"lastLogin", ORM::FieldType::FT_BIGINT},
            ORM::Field{"currentLoginURI", ORM::FieldType::FT_TEXT},
            ORM::Field{"lastPasswordChange", ORM::FieldType::FT_BIGINT},
            ORM::Field{"lastEmailCheck", ORM::FieldType::FT_BIGINT},
            ORM::Field{"waitingForEmailCheck", ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"locale", ORM::FieldType::FT_TEXT},
            ORM::Field{"notes", ORM::FieldType::FT_TEXT},
            ORM::Field{"location", ORM::FieldType::FT_TEXT},
            ORM::Field{"owner", ORM::FieldType::FT_TEXT},
            ORM::Field{"suspended", ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"blackListed", ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"userRole", ORM::FieldType::FT_TEXT},
            ORM::Field{"userTypeProprietaryInfo", ORM::FieldType::FT_TEXT},
            ORM::Field{"securityPolicy", ORM::FieldType::FT_TEXT},
            ORM::Field{"securityPolicyChange", ORM::FieldType::FT_BIGINT},
            ORM::Field{"currentPassword", ORM::FieldType::FT_TEXT},
            ORM::Field{"lastPasswords", ORM::FieldType::FT_TEXT},
            ORM::Field{"oauthType", ORM::FieldType::FT_TEXT},
            ORM::Field{"oauthUserInfo", ORM::FieldType::FT_TEXT},
            ORM::Field{"modified", ORM::FieldType::FT_TEXT},
            ORM::Field{"signingUp", ORM::FieldType::FT_TEXT}
    };

    static ORM::IndexVec MakeIndices(const std::string & shortname) {
        return ORM::IndexVec{
                {std::string(shortname + "_user_email_index"),
                        ORM::IndexEntryVec{
                                {std::string("email"),
                                 ORM::Indextype::ASC}}},
                {std::string(shortname + "_user_name_index"),
                        ORM::IndexEntryVec{
                                {std::string("name"),
                                 ORM::Indextype::ASC}}}
        };
    }

    BaseUserDB::BaseUserDB(const std::string &Name, const std::string & ShortName, OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L, UserCache * Cache, bool Users) :
            DB(T, Name.c_str(), BaseUserDB_Fields, MakeIndices(ShortName), P, L, ShortName.c_str(), Cache),
            UsersOnly_(Users) {
    }

    bool BaseUserDB::Upgrade(uint32_t from, uint32_t &to) {
        std::vector<std::string> Statements{
              "alter table " + TableName_ + " add column modified BIGINT;",
              "alter table " + TableName_ + " add column signingUp TEXT default '';"
        };
        RunScript(Statements);
        to = CurrentVersion;
        return true;
    }

    bool BaseUserDB::CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready ) {
        try {
            Poco::toLowerInPlace(NewUser.email);
            if(Exists("email", NewUser.email)) {
                return false;
            }

            Poco::Data::Session Sess = Pool_.get();

            if(!PasswordHashedAlready) {
                NewUser.id = MicroService::CreateUUID();
                NewUser.creationDate = std::time(nullptr);
            }

            //  if there is a password, we assume that we do not want email verification,
            //  if there is no password, we will do email verification
            if(NewUser.currentPassword.empty()) {

            } else {
                if(!PasswordHashedAlready) {
                    NewUser.currentPassword = AuthService()->ComputeNewPasswordHash(NewUser.email,NewUser.currentPassword);
                    NewUser.lastPasswords.clear();
                    NewUser.lastPasswords.push_back(NewUser.currentPassword);
                    NewUser.lastPasswordChange = std::time(nullptr);
                    NewUser.validated = true;
                }
            }
            return CreateRecord(NewUser);

        } catch (const Poco::Exception &E) {
            std::cout << "What: " << E.what() << " name: " << E.name() << std::endl;
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::GetUserByEmail(const std::string & email, SecurityObjects::UserInfo & User) {
        try {
            return GetRecord("email", email, User);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::GetUserById(const std::string &Id, SecurityObjects::UserInfo &User) {
        try {
            return GetRecord("id", Id, User);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::GetUsers( uint64_t Offset, uint64_t HowMany, SecurityObjects::UserInfoVec & Users, const std::string & WhereClause) {
        try {
            return GetRecords(Offset, HowMany, Users, WhereClause);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::SetAvatar(const std::string &Id, const std::string &Value) {
        try {
            SecurityObjects::UserInfo   User;
            if(GetRecord("id",Id,User)) {
                if(Value.empty()) {
                    User.avatar = "0";
                } else {
                    User.avatar = std::to_string(std::time(nullptr));
                }
                return UpdateRecord("id",Id,User);
            }
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::UpdateUserInfo(const std::string & Admin, SecurityObjects::USER_ID_TYPE & Id, const SecurityObjects::UserInfo &UInfo) {
        try {
            return UpdateRecord("id", Id, UInfo);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool BaseUserDB::DeleteUser(const std::string & Admin, const SecurityObjects::USER_ID_TYPE & Id)  {
        return DeleteRecord("id", Id);
    }

    bool BaseUserDB::DeleteUsers(const std::string & Admin, const std::string & owner) {
        std::string WhereClause{ " owner='" + owner +"' "};
        return DeleteRecords(WhereClause);
    }

    bool BaseUserDB::SetLastLogin(const std::string &Id) {
        try {
            Poco::Data::Session Sess = Pool_.get();
            Poco::Data::Statement Update(Sess);

            auto tId{Id};

            std::string     St1{"update " + TableName_ + " set lastLogin=? where id=?"};
            uint64_t Now=std::time(nullptr);
            Update << ConvertParams(St1),
                    Poco::Data::Keywords::use(Now),
                    Poco::Data::Keywords::use(tId);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    UserCache::UserCache(unsigned Size, unsigned TimeOut, bool Users) :
            UsersOnly_(Users),
            ORM::DBCache<SecurityObjects::UserInfo>(Size,TimeOut) {
        CacheById_ = std::make_unique<Poco::ExpireLRUCache<std::string,SecurityObjects::UserInfo>>(Size,TimeOut);
        CacheByEMail_ = std::make_unique<Poco::ExpireLRUCache<std::string,std::string>>(Size,TimeOut);
    }

    void UserCache::UpdateCache(const SecurityObjects::UserInfo &R) {
        // std::cout << "Update user cache:" << R.Id << std::endl;
        CacheById_->update(R.id,R);
        CacheByEMail_->update(R.email,R.id);
        if(UsersOnly_)
            StorageService()->UserTokenDB().DeleteRecordsFromCache("userName", R.id);
        else
            StorageService()->SubTokenDB().DeleteRecordsFromCache("userName", R.id);
    }

    inline void UserCache::Create(const SecurityObjects::UserInfo &R)  {
    }

    inline bool UserCache::GetFromCache(const std::string &FieldName, const std::string &Value, SecurityObjects::UserInfo &R) {
        std::lock_guard     M(Mutex_);
        if(FieldName=="id") {
            auto Entry = CacheById_->get(Value);
            if(Entry.isNull())
                return false;
            R = *Entry;
            return true;
        } else if(FieldName=="email") {
            auto Entry = CacheByEMail_->get(Value);
            if(Entry.isNull())
                return false;
            auto Record = CacheById_->get(*Entry);
            if(Record.isNull())
                return false;
            R = *Record;
            return true;
        }
        return false;
    }

    inline void UserCache::Delete(const std::string &FieldName, const std::string &Value) {
    std::lock_guard     M(Mutex_);
        // std::cout << "Delete user cache:" << Value << std::endl;
        if(FieldName=="id") {
            auto  E = CacheById_->get(Value);
            if(!E.isNull())
                CacheByEMail_->remove(E->email);
            CacheById_->remove(Value);
        } else if(FieldName=="email") {
            auto E = CacheByEMail_->get(Value);
            if(!E.isNull()) {
                CacheById_->remove(*E);
                CacheByEMail_->remove(Value);
            }
        }
    }
}

template<> void ORM::DB<OpenWifi::UserInfoRecordTuple,
        OpenWifi::SecurityObjects::UserInfo>::Convert(const OpenWifi::UserInfoRecordTuple &T,
                                                      OpenWifi::SecurityObjects::UserInfo &U) {
    U.id = T.get<0>();
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
    U.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(T.get<17>());
    U.location = T.get<18>();
    U.owner = T.get<19>();
    U.suspended = T.get<20>();
    U.blackListed = T.get<21>();
    U.userRole = OpenWifi::SecurityObjects::UserTypeFromString(T.get<22>());
    U.userTypeProprietaryInfo = OpenWifi::RESTAPI_utils::to_object<OpenWifi::SecurityObjects::UserLoginLoginExtensions>(T.get<23>());
    U.securityPolicy = T.get<24>();
    U.securityPolicyChange = T.get<25>();
    U.currentPassword = T.get<26>();
    U.lastPasswords = OpenWifi::RESTAPI_utils::to_object_array(T.get<27>());
    U.oauthType = T.get<28>();
    U.oauthUserInfo = T.get<29>();
    U.modified = T.get<30>();
    U.signingUp = T.get<31>();
}

template<> void ORM::DB< OpenWifi::UserInfoRecordTuple,
    OpenWifi::SecurityObjects::UserInfo>::Convert(const OpenWifi::SecurityObjects::UserInfo &U,
                                                  OpenWifi::UserInfoRecordTuple &T) {
    T.set<0>(U.id);
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
    T.set<17>(OpenWifi::RESTAPI_utils::to_string(U.notes));
    T.set<18>(U.location);
    T.set<19>(U.owner);
    T.set<20>(U.suspended);
    T.set<21>(U.blackListed);
    T.set<22>(OpenWifi::SecurityObjects::UserTypeToString(U.userRole));
    T.set<23>(OpenWifi::RESTAPI_utils::to_string(U.userTypeProprietaryInfo));
    T.set<24>(U.securityPolicy);
    T.set<25>(U.securityPolicyChange);
    T.set<26>(U.currentPassword);
    T.set<27>(OpenWifi::RESTAPI_utils::to_string(U.lastPasswords));
    T.set<28>(U.oauthType);
    T.set<29>(U.oauthUserInfo);
    T.set<30>(U.modified);
    T.set<31>(U.signingUp);
}
