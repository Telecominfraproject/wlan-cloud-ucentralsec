//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/Connector.h"

#ifndef SMALL_BUILD
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/ODBC/Connector.h"
#endif

#include "AuthService.h"
#include "RESTAPI_SecurityObjects.h"
#include "SubSystemServer.h"

namespace uCentral {

    static const std::string AllUsersFieldsForCreation{
            "Id             varchar(36),"
            "name           varchar,"
            "description    varchar,"
            "avatar         varchar,"
            "email          varchar,"
            "validated      int,"
            "validationEmail    varchar,"
            "validationDate bigint,"
            "creationDate   bigint,"
            "validationURI  varchar,"
            "changePassword int,"
            "lastLogin      bigint,"
            "currentLoginURI    varchar,"
            "lastPasswordChange bigint,"
            "lastEmailCheck     bigint,"
            "waitingForEmailCheck   int,"
            "locale             varchar,"
            "notes              text,"
            "location           varchar,"
            "owner              varchar,"
            "suspended          int,"
            "blackListed        int,"
            "userRole           varchar,"
            "userTypeProprietaryInfo    text,"
            "securityPolicy     text,"
            "securityPolicyChange   bigint,"
            "currentPassword    varchar,"
            "lastPasswords      varchar,"
            "oauthType          varchar,"
            "oauthUserInfo      text"};

    static const std::string AllUsersFieldsForSelect{
            "Id,"
            "name,"
            "description,"
            "avatar,"
            "email,"
            "validated,"
            "validationEmail,"
            "validationDate,"
            "creationDate,"
            "validationURI,"
            "changePassword,"
            "lastLogin,"
            "currentLoginURI,"
            "lastPasswordChange,"
            "lastEmailCheck,"
            "waitingForEmailCheck,"
            "locale,"
            "notes,"
            "location,"
            "owner,"
            "suspended,"
            "blackListed,"
            "userRole,"
            "userTypeProprietaryInfo,"
            "securityPolicy,"
            "securityPolicyChange,"
            "currentPassword,"
            "lastPasswords,"
            "oauthType,"
            "oauthUserInfo"};


    class Storage : public SubSystemServer {

    public:

        enum StorageType {
            sqlite,
            pgsql,
            mysql,
            odbc
        };

        enum AUTH_ERROR {
            SUCCESS,
            PASSWORD_CHANGE_REQUIRED,
            PASSWORD_DOES_NOT_MATCH,
            PASSWORD_ALREADY_USED,
            USERNAME_PENDING_VERIFICATION,
            PASSWORD_INVALID,
            INTERNAL_ERROR
        };

        enum USER_TYPE {
            UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, SPECIAL
        };

        typedef std::string USER_ID_TYPE;

        static USER_TYPE to_userType(const std::string &U) {
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

        static const std::string from_userType(USER_TYPE U) {
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

        static Storage *instance() {
            if (instance_ == nullptr) {
                instance_ = new Storage;
            }
            return instance_;
        }

        int 	Start() override;
        void 	Stop() override;

        //  all passwords passed here are all plaintext
        bool CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser);
        bool GetUserByEmail(std::string & email, SecurityObjects::UserInfo & User);
        bool GetUserById(USER_ID_TYPE & Id, SecurityObjects::UserInfo & User);
        bool DeleteUser(const std::string & Admin, USER_ID_TYPE & Id);
        bool SetOwner(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Owner);
        bool SetLocation(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Location);
        AUTH_ERROR ChangePassword(const std::string & Admin, USER_ID_TYPE & Id, const std::string &OldPassword, const std::string &NewPassword);
        bool AddNotes(const std::string & Admin, USER_ID_TYPE & Id, const std::string &Notes);
        bool SetPolicyChange(const std::string & Admin, USER_ID_TYPE & Id, const std::string &NewPolicy);
        bool UpdateUserInfo(const std::string & Admin, USER_ID_TYPE & Id, SecurityObjects::UserInfo &UInfo);
        bool GetUsers( uint64_t Offset, uint64_t Limit, SecurityObjects::UserInfoVec & Users);

	  private:
		static Storage      							*instance_;
		std::unique_ptr<Poco::Data::SessionPool>        Pool_= nullptr;
		StorageType 									dbType_ = sqlite;
		std::unique_ptr<Poco::Data::SQLite::Connector>  SQLiteConn_= nullptr;
#ifndef SMALL_BUILD
		std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_= nullptr;
		std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_= nullptr;
		std::unique_ptr<Poco::Data::ODBC::Connector>        ODBCConn_= nullptr;
#endif

        int Create_Tables();
        int Create_UserTable();
        int Create_APIKeyTable();

        int 	Setup_SQLite();
		[[nodiscard]] std::string ConvertParams(const std::string &S) const;

#ifndef SMALL_BUILD
        int 	Setup_MySQL();
        int 	Setup_PostgreSQL();
        int 	Setup_ODBC();
#endif
        Storage() noexcept;
   };

    inline Storage * Storage() { return Storage::instance(); };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
