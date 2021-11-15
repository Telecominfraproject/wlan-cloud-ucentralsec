//
// Created by stephane bourque on 2021-06-25.
//

#include <vector>

#include "Poco/Tuple.h"
#include "storage_users.h"

#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    bool Convert(const UserInfoRecord &T, SecurityObjects::UserInfo &U) {
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

    bool Convert(const SecurityObjects::UserInfo &U, UserInfoRecord &T) {
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

    std::string OldDefaultUseridStockUUID{"DEFAULT-USER-UUID-SHOULD-BE-DELETED!!!"};
    std::string NewDefaultUseridStockUUID{"11111111-0000-0000-6666-999999999999"};

    void Storage::ReplaceOldDefaultUUID() {
        try {
            Poco::Data::Session Sess = Pool_->get();
            std::string St1{"update users set id=? where id=?"};

            Poco::Data::Statement Update(Sess);
            Update << ConvertParams(St1),
                Poco::Data::Keywords::use(NewDefaultUseridStockUUID),
                Poco::Data::Keywords::use(OldDefaultUseridStockUUID);
            Update.execute();
        } catch (...) {

        }
    }

    //  if we do not find a default user, then we need to create one based on the
    //  property file. We must set its flag to "must change password", this user has root privilege.
    //  if the "DEFAULT-USER-UUID", we keep the UUID of that user. We want to hide the UUID of the default root user
    bool Storage::InitializeDefaultUser() {
        SecurityObjects::UserInfo   U;
        bool DefaultUserCreated = false;

        ReplaceOldDefaultUUID();
        AppServiceRegistry().Get("defaultusercreated",DefaultUserCreated);
        if(!GetUserById(NewDefaultUseridStockUUID,U) && !DefaultUserCreated) {
            U.currentPassword = MicroService::instance().ConfigGetString("authentication.default.password","");
            U.lastPasswords.push_back(U.currentPassword);
            U.email = MicroService::instance().ConfigGetString("authentication.default.username","");
            U.Id = NewDefaultUseridStockUUID;
            U.userRole = SecurityObjects::ROOT;
            U.creationDate = std::time(nullptr);
            U.validated = true;
            U.name = "Default User";
            U.description = "Default user should be deleted.";
            U.changePassword = true;
            CreateUser("SYSTEM",U, true);
            AppServiceRegistry().Set("defaultusercreated",true);
            return true;
        }
        return false;
    }

    bool Storage::CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready ) {
        try {
            Poco::Data::Session Sess = Pool_->get();

            Poco::toLowerInPlace(NewUser.email);

            //  if the user exists, must return an error
            std::string St1{"select " + AllUsersFieldsForSelect + " from users where email=?"};
            UserInfoRecordList Records;

            try {
                Poco::Data::Statement Statement(Sess);
                Statement << ConvertParams(St1),
                        Poco::Data::Keywords::into(Records),
                        Poco::Data::Keywords::use(NewUser.email);
                Statement.execute();
            } catch (const Poco::Exception &E) {

            }

            if(!Records.empty())
                return false;

            if(!PasswordHashedAlready) {
                NewUser.Id = MicroService::CreateUUID();
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

            auto Notes = RESTAPI_utils::to_string(NewUser.notes);
            auto UserType = SecurityObjects::UserTypeToString(NewUser.userRole);
            auto OldPasswords = RESTAPI_utils::to_string(NewUser.lastPasswords);
            auto userTypeProprietaryInfo = RESTAPI_utils::to_string(NewUser.userTypeProprietaryInfo);

            St1 = "INSERT INTO Users (" + AllUsersFieldsForSelect + ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
            Poco::Data::Statement Statement(Sess);

            UserInfoRecord  R;
            Convert(NewUser, R);

            Statement << ConvertParams(St1),
                    Poco::Data::Keywords::use(R);
            Statement.execute();
            return true;

        } catch (const Poco::Exception &E) {
            std::cout << "What: " << E.what() << " name: " << E.name() << std::endl;
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetUserByEmail(std::string & email, SecurityObjects::UserInfo & User) {
        std::string St1;
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            Poco::toLowerInPlace(email);

            //  if the user exists, must return an error
            St1 = "select " + AllUsersFieldsForSelect + " from users where email=?";
            UserInfoRecordList Records;

            Select << ConvertParams(St1) ,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::use(email);
            Select.execute();

            if(Records.empty())
                return false;

            Convert(Records[0],User);

            return true;
        } catch (const Poco::Exception &E) {
            std::cout << "Statement: " << St1 << std::endl;
            std::cout << "What:" << E.what() << " name: " << E.name() << std::endl;
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetUserById(std::string &Id, SecurityObjects::UserInfo &User) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            //  if the user exists, must return an error
            std::string St1{"select " + AllUsersFieldsForSelect + " from users where id=?"};
            UserInfoRecordList Records;

            Select << ConvertParams(St1) ,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::use(Id);
            Select.execute();

            if(Records.empty())
                return false;

            Convert(Records[0],User);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetUsers( uint64_t Offset, uint64_t HowMany, SecurityObjects::UserInfoVec & Users) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);
            UserInfoRecordList Records;

            std::string St1{"select " + AllUsersFieldsForSelect + " from users order by id ASC "};

            Select << ConvertParams(St1) + ComputeRange(Offset, HowMany),
                        Poco::Data::Keywords::into(Records);
            Select.execute();

            for(const auto &R:Records) {
                SecurityObjects::UserInfo   U;
                Convert(R,U);
                Users.push_back(U);
            }
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::UpdateUserInfo(const std::string & Admin, USER_ID_TYPE & Id, SecurityObjects::UserInfo &UInfo) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            std::string St1{"update users set " + AllUsersFieldsForUpdate + " where id=?"};
            auto Notes = RESTAPI_utils::to_string(UInfo.notes);
            auto UserType = SecurityObjects::UserTypeToString(UInfo.userRole);
            auto OldPasswords = RESTAPI_utils::to_string(UInfo.lastPasswords);
            auto userTypeProprietaryInfo = RESTAPI_utils::to_string(UInfo.userTypeProprietaryInfo);
            UserInfoRecord R;
            Convert(UInfo, R);
            Update << ConvertParams(St1),
                    Poco::Data::Keywords::use(R),
                    Poco::Data::Keywords::use(UInfo.Id);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            std::cout << " Exception: " << E.what() << "  name: " << E.name() << std::endl;
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::DeleteUser(const std::string & Admin, USER_ID_TYPE & Id)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string     St1{"delete from users where id=?"};

            Delete << ConvertParams(St1),
                Poco::Data::Keywords::use(Id);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetOwner(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Owner)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetLocation(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Location)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetLastLogin(std::string &Id) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            std::string     St1{"update users set lastLogin=? where id=?"};
            uint64_t Now=std::time(nullptr);
            Update << ConvertParams(St1),
                    Poco::Data::Keywords::use(Now),
                    Poco::Data::Keywords::use(Id);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    Storage::AUTH_ERROR Storage::ChangePassword(const std::string & Admin, USER_ID_TYPE & Id, const std::string &OldPassword, const std::string &NewPassword)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return SUCCESS;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return INTERNAL_ERROR;
    }

    bool Storage::AddNotes(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Notes)   {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetPolicyChange(const std::string & Admin, USER_ID_TYPE & Id, const std::string &NewPolicy) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

}

