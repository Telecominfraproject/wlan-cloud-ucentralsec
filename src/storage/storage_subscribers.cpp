//
// Created by stephane bourque on 2021-11-30.
//


#include <vector>

#include "Poco/Tuple.h"

#include "storage_subscribers.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "storage/storage_conversions.h"

namespace OpenWifi {

    bool Storage::CreateSubUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready ) {
        try {
            Poco::Data::Session Sess = Pool_->get();

            Poco::toLowerInPlace(NewUser.email);

            //  if the user exists, must return an error
            std::string St1{"select " + AllSubUsersFieldsForSelect + " from Subscribers where email=?"};
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

            St1 = "INSERT INTO Subscribers (" + AllSubUsersFieldsForSelect + ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
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

    bool Storage::GetSubUserByEmail(std::string & email, SecurityObjects::UserInfo & User) {
        std::string St1;
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            Poco::toLowerInPlace(email);

            //  if the user exists, must return an error
            St1 = "select " + AllSubUsersFieldsForSelect + " from Subscribers where email=?";
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

    bool Storage::GetSubUserById(std::string &Id, SecurityObjects::UserInfo &User) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            //  if the user exists, must return an error
            std::string St1{"select " + AllSubUsersFieldsForSelect + " from Subscribers where id=?"};
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

    bool Storage::GetSubUsers( uint64_t Offset, uint64_t HowMany, SecurityObjects::UserInfoVec & Users) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);
            UserInfoRecordList Records;

            std::string St1{"select " + AllSubUsersFieldsForSelect + " from Subscribers order by id ASC "};

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

    bool Storage::UpdateSubUserInfo(const std::string & Admin, USER_ID_TYPE & Id, SecurityObjects::UserInfo &UInfo) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            std::string St1{"update Subscribers set " + AllSubUsersFieldsForUpdate + " where id=?"};
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

    bool Storage::DeleteSubUser(const std::string & Admin, USER_ID_TYPE & Id)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string     St1{"delete from Subscribers where id=?"};

            Delete << ConvertParams(St1),
            Poco::Data::Keywords::use(Id);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetSubOwner(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Owner)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetSubLocation(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Location)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetSubLastLogin(std::string &Id) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            std::string     St1{"update Subscribers set lastLogin=? where id=?"};
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

    Storage::AUTH_ERROR Storage::ChangeSubPassword(const std::string & Admin, USER_ID_TYPE & Id, const std::string &OldPassword, const std::string &NewPassword)  {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return SUCCESS;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return INTERNAL_ERROR;
    }

    bool Storage::AddSubNotes(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Notes)   {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetSubPolicyChange(const std::string & Admin, USER_ID_TYPE & Id, const std::string &NewPolicy) {
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

