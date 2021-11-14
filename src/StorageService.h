//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/StorageClass.h"
#include "AuthService.h"

#include "Poco/Timer.h"

namespace OpenWifi {

    static const std::string AllEmailTemplatesFieldsForCreation {

    };

    static const std::string AllEmailTemplatesFieldsForSelect {

    };

    static const std::string AllEmailTemplatesFieldsForUpdate {

    };

    class Archiver {
    public:
        void onTimer(Poco::Timer & timer);
    private:
    };

    class Storage : public StorageClass {
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

        static std::string from_userType(USER_TYPE U) {
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
            static Storage instance;
            return &instance;
        }

        int 	Start() override;
        void 	Stop() override;

        /*
         *  All user management functions
         */
        bool InitializeDefaultUser();
        bool CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser, bool PasswordHashedAlready = false);
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

        bool AddToken(std::string &UserId, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut);
        bool RevokeToken( std::string & Token );
        bool IsTokenRevoked( std::string & Token );
        bool CleanExpiredTokens();
        bool RevokeAllTokens( std::string & UserName );
        bool GetToken(std::string &Token, SecurityObjects::UserInfoAndPolicy &UInfo);

        /*
         *  All ActionLinks functions
         */
        bool CreateAction( SecurityObjects::ActionLink & A);
        bool DeleteAction(std::string &ActionId);
        bool CompleteAction(std::string &ActionId);
        bool CancelAction(std::string &ActionId);
        bool SentAction(std::string &ActionId);
        bool GetActionLink(std::string &ActionId, SecurityObjects::ActionLink &A);
        bool GetActions(std::vector<SecurityObjects::ActionLink> &Links, uint64_t Max=200);
        void CleanOldActionLinks();

	  private:
        int Create_Tables();
        int Create_UserTable();
        int Create_AvatarTable();
        int Create_TokensTable();
        int Create_ActionLinkTable();

        Poco::Timer                     Timer_;
        Archiver                        Archiver_;
        std::unique_ptr<Poco::TimerCallback<Archiver>>   Archivercallback_;

        /// This is to support a mistake that was deployed...
        void ReplaceOldDefaultUUID();
   };

    inline Storage * StorageService() { return Storage::instance(); };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
