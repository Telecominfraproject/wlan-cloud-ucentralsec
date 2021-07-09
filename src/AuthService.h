//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UAUTHSERVICE_H
#define UCENTRAL_UAUTHSERVICE_H

#include "SubSystemServer.h"

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JWT/Signer.h"
#include "Poco/SHA2Engine.h"

#include "RESTAPI_SecurityObjects.h"

namespace uCentral{

    static const std::string AUTHENTICATION_SYSTEM{"SYSTEM"};

    class AuthService : public SubSystemServer {
    public:

        enum ACCESS_TYPE {
            USERNAME,
            SERVER,
            CUSTOM
        };

        static ACCESS_TYPE IntToAccessType(int C);
        static int AccessTypeToInt(ACCESS_TYPE T);

        static AuthService *instance() {
            if (instance_ == nullptr) {
                instance_ = new AuthService;
            }
            return instance_;
        }

        int Start() override;
        void Stop() override;

        [[nodiscard]] bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo );
        [[nodiscard]] bool Authorize( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo );
        void CreateToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
        [[nodiscard]] bool ValidateToken(const std::string & Token, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UserInfo  );
        [[nodiscard]] bool SetPassword(const std::string &Password, SecurityObjects::UserInfo & UInfo);

        void Logout(const std::string &token);

        [[nodiscard]] bool IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo);
        [[nodiscard]] bool IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);
        [[nodiscard]] std::string GenerateToken(const std::string & UserName, ACCESS_TYPE Type, int NumberOfDays);
        [[nodiscard]] bool ValidateToken(const std::string & Token, std::string & SessionToken, SecurityObjects::WebToken & UserInfo  );
        [[nodiscard]] std::string ComputePasswordHash(const std::string &UserName, const std::string &Password);
        [[nodiscard]] bool UpdatePassword(const std::string &Admin, const std::string &UserName, const std::string & OldPassword, const std::string &NewPassword);
        [[nodiscard]] std::string ResetPassword(const std::string &Admin, const std::string &UserName);

    private:
		static AuthService *instance_;
		bool    			Secure_ = false ;
		std::string     	DefaultUserName_;
		std::string			DefaultPassword_;
		std::string     	Mechanism_;
		Poco::JWT::Signer	Signer_;
		Poco::SHA2Engine	SHA2_;
		SecurityObjects::UserInfoCache UserCache_;

        AuthService() noexcept:
            SubSystemServer("Authentication", "AUTH-SVR", "authentication")
        {
        }
    };

    inline AuthService * AuthService() { return AuthService::instance(); }

} // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
