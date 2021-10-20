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
#include "Poco/File.h"
#include "Poco/TemporaryFile.h"

#ifndef SMALL_BUILD
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#endif

#include "AuthService.h"
#include "RESTAPI/RESTAPI_SecurityObjects.h"
#include "framework/SubSystemServer.h"
#include "framework/Storage.h"

namespace OpenWifi {

    static const std::string AllActionLinksFieldsForSelect {
            "Id, "
            "Action,"
            "UserId,"
            "template,"
            "locale,"
            "message,"
            "sent,"
            "created,"
            "expires,"
            "completed,"
            "canceled"
    };

    static const std::string AllActionLinksFieldsForUpdate {
            "Id=?, "
            "Action=?,"
            "UserId=?,"
            "template=?,"
            "locale=?,"
            "message=?,"
            "sent=?,"
            "created=?,"
            "expires=?,"
            "completed=?,"
            "canceled=?"
    };

    static const std::string AllEmailTemplatesFieldsForCreation {

    };

    static const std::string AllEmailTemplatesFieldsForSelect {

    };

    static const std::string AllEmailTemplatesFieldsForUpdate {

    };

    class Storage : public SubSystemServer {
    public:

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

        /*
         *  All user management functions
         */
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
        bool SetLastLogin(USER_ID_TYPE & Id);

        bool SetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type, std::string & Name);
        bool GetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type, std::string & Name);
        bool DeleteAvatar(const std::string & Admin, std::string &Id);

        bool AddToken(std::string &UserName, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut);
        bool RevokeToken( std::string & Token );
        bool IsTokenRevoked( std::string & Token );
        bool CleanRevokedTokens( uint64_t Oldest );
        bool RevokeAllTokens( std::string & UserName );
        bool GetToken(std::string &Token, SecurityObjects::UserInfoAndPolicy &UInfo);

        /*
         *  All ActionLinks functions
         */
        bool CreateAction(std::string &ActionId, std::string &Action, USER_ID_TYPE & Id, Types::StringPairVec & Elements );
        bool DeleteAction(std::string &ActionId);
        bool CompleteAction(std::string &ActionId);
        bool CancelAction(std::string &ActionId);

	  private:
		static Storage      							*instance_;
		std::unique_ptr<Poco::Data::SessionPool>        Pool_= nullptr;
		DBType       									dbType_ = sqlite;
		std::unique_ptr<Poco::Data::SQLite::Connector>  SQLiteConn_= nullptr;
#ifndef SMALL_BUILD
		std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_= nullptr;
		std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_= nullptr;
#endif

        int Create_Tables();
        int Create_UserTable();
        int Create_AvatarTable();
        int Create_TokensTable();

		[[nodiscard]] std::string ConvertParams(const std::string &S) const;
		[[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
		    if(dbType_==sqlite) {
		        return " LIMIT " + std::to_string(From-1) + ", " + std::to_string(HowMany) + " ";
		    } else if(dbType_==pgsql) {
		        return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
		    } else if(dbType_==mysql) {
		        return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
		    }
		    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
		}

		Storage() noexcept:
            SubSystemServer("Storage", "STORAGE-SVR", "storage")
            {
            }

        int 	Setup_SQLite();
        int 	Setup_MySQL();
        int 	Setup_PostgreSQL();

   };

    inline Storage * Storage() { return Storage::instance(); };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
