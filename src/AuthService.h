//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UAUTHSERVICE_H
#define UCENTRAL_UAUTHSERVICE_H

#include <regex>

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JWT/Signer.h"
#include "Poco/SHA2Engine.h"
#include "Poco/Crypto/DigestEngine.h"
#include "Poco/HMACEngine.h"

#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi{

    static const std::string AUTHENTICATION_SYSTEM{"SYSTEM"};

    class AuthService : public SubSystemServer {
    public:

        enum ACCESS_TYPE {
            USERNAME,
            SERVER,
            CUSTOM
        };

        enum AUTH_ERROR {
            SUCCESS,
            PASSWORD_CHANGE_REQUIRED,
            INVALID_CREDENTIALS,
            PASSWORD_ALREADY_USED,
            USERNAME_PENDING_VERIFICATION,
            PASSWORD_INVALID,
            INTERNAL_ERROR
        };

        enum EMAIL_REASON {
            FORGOT_PASSWORD,
            EMAIL_VERIFICATION
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
        [[nodiscard]] AUTH_ERROR Authorize( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo );
        void CreateToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
        [[nodiscard]] bool SetPassword(const std::string &Password, SecurityObjects::UserInfo & UInfo);
        [[nodiscard]] const std:: string & PasswordValidationExpression() const { return PasswordValidationStr_;};
        void Logout(const std::string &token, bool EraseFromCache=true);

        bool ValidatePassword(const std::string &pwd);

        [[nodiscard]] bool IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo);
        [[nodiscard]] bool IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);
        [[nodiscard]] std::string GenerateTokenJWT(const std::string & UserName, ACCESS_TYPE Type);
        [[nodiscard]] std::string GenerateTokenHMAC(const std::string & UserName, ACCESS_TYPE Type);
        [[nodiscard]] std::string ComputePasswordHash(const std::string &UserName, const std::string &Password);
        [[nodiscard]] bool UpdatePassword(const std::string &Admin, const std::string &UserName, const std::string & OldPassword, const std::string &NewPassword);
        [[nodiscard]] std::string ResetPassword(const std::string &Admin, const std::string &UserName);

        [[nodiscard]] static bool VerifyEmail(SecurityObjects::UserInfo &UInfo);
        [[nodiscard]] static bool SendEmailToUser(std::string &Email, EMAIL_REASON Reason);
        [[nodiscard]] bool DeleteUserFromCache(const std::string &UserName);
        [[nodiscard]] bool RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo);

        [[nodiscard]] static inline const std::string GetLogoAssetURI() {
            return MicroService::instance().PublicEndPoint() + "/wwwassets/the_logo.png";
        }

        [[nodiscard]] static inline const std::string GetLogoAssetFileName() {
            return MicroService::instance().DataDir() + "/wwwassets/the_logo.png";
        }

    private:
		static AuthService *instance_;
		bool    			Secure_ = false ;
		std::string     	DefaultUserName_;
		std::string			DefaultPassword_;
		std::string     	Mechanism_;
		Poco::JWT::Signer	Signer_;
		Poco::SHA2Engine	SHA2_;
		SecurityObjects::UserInfoCache UserCache_;
        std::string          PasswordValidationStr_;
		std::regex          PasswordValidation_;
		uint64_t            TokenAging_ = 30 * 24 * 60 * 60;
        uint64_t            HowManyOldPassword_=5;

        class SHA256Engine : public Poco::Crypto::DigestEngine
                {
                public:
                    enum
                    {
                        BLOCK_SIZE = 64,
                        DIGEST_SIZE = 32
                    };

                    SHA256Engine()
                    : DigestEngine("SHA256")
                    {
                    }

                };

        Poco::HMACEngine<SHA256Engine> HMAC_{"tipopenwifi"};

        AuthService() noexcept:
            SubSystemServer("Authentication", "AUTH-SVR", "authentication")
        {
        }
    };

    inline AuthService * AuthService() { return AuthService::instance(); }

    [[nodiscard]] inline bool AuthServiceIsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo ) {
        return AuthService()->IsAuthorized(Request, SessionToken, UInfo );
    }

} // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
