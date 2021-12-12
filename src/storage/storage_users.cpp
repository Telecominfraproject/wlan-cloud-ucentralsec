//
// Created by stephane bourque on 2021-06-25.
//

#include <vector>

#include "Poco/Tuple.h"
#include "storage_users.h"

#include "StorageService.h"
#include "framework/MicroService.h"
#include "storage/storage_conversions.h"

namespace OpenWifi {

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
            Logger().log(E);
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
            Logger().log(E);
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
            Logger().log(E);
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
            Logger().log(E);
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
            Logger().log(E);
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
            Logger().log(E);
        }
        return false;
    }

    bool Storage::SetOwner(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Owner)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool Storage::SetLocation(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Location)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
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
            Logger().log(E);
        }
        return false;
    }

    Storage::AUTH_ERROR Storage::ChangePassword(const std::string & Admin, USER_ID_TYPE & Id, const std::string &OldPassword, const std::string &NewPassword)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return SUCCESS;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return INTERNAL_ERROR;
    }

    bool Storage::AddNotes(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Notes)   {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool Storage::SetPolicyChange(const std::string & Admin, USER_ID_TYPE & Id, const std::string &NewPolicy) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

}

