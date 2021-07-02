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
            PASSWORD_INVALID
        };

        enum USER_TYPE {
            UNKNOWN, ROOT, ADMIN, SUBSCRIBER, CSR, SYSTEM, SPECIAL
        };

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
        bool DeleteUser(const std::string & Admin, uint64_t Id);
        bool SetOwner(const std::string & Admin, uint64_t Id, const std::string &Owner);
        bool SetLocation(const std::string & Admin, uint64_t Id, const std::string &Location);
        AUTH_ERROR ChangePassword(const std::string & Admin, uint64_t Id, const std::string &OldPassword, const std::string &NewPassword);
        bool AddNotes(const std::string & Admin, uint64_t Id, const std::string &Notes);
        bool SetPolicyChange(const std::string & Admin, const std::string &NewPolicy);












        bool IdentityExists(std::string & Identity, AuthService::ACCESS_TYPE Type);
        bool AddIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, SecurityObjects::AclTemplate & ACL);
        bool GetIdentity(std::string & Identity, std::string & Password,AuthService::ACCESS_TYPE Type, SecurityObjects::AclTemplate & ACL);
        bool UpdateIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, SecurityObjects::AclTemplate & ACL);
        bool DeleteIdentity(std::string & Identity, AuthService::ACCESS_TYPE Type);
        bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, AuthService::ACCESS_TYPE Type);
        bool GetIdentityRights(std::string &Identity, SecurityObjects::AclTemplate &ACL);















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
