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
#include "RESTAPI_objects.h"
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

        enum CommandExecutionType {
            COMMAND_PENDING,
            COMMAND_EXECUTED,
            COMMAND_COMPLETED
        };

        static Storage *instance() {
            if (instance_ == nullptr) {
                instance_ = new Storage;
            }
            return instance_;
        }

        int 	Start() override;
        void 	Stop() override;

        bool IdentityExists(std::string & Identity, AuthService::ACCESS_TYPE Type);
        bool AddIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
        bool GetIdentity(std::string & Identity, std::string & Password,AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
        bool UpdateIdentity(std::string & Identity, std::string & Password, AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
        bool DeleteIdentity(std::string & Identity, AuthService::ACCESS_TYPE Type);
        bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, AuthService::ACCESS_TYPE Type);
        bool GetIdentityRights(std::string &Identity, uCentral::Objects::AclTemplate &ACL);















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
