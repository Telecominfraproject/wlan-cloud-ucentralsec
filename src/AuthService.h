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
#include "Poco/ExpireLRUCache.h"

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

        enum EMAIL_REASON {
            FORGOT_PASSWORD,
            EMAIL_VERIFICATION
        };

        static ACCESS_TYPE IntToAccessType(int C);
        static int AccessTypeToInt(ACCESS_TYPE T);

        static auto instance() {
            static auto instance_ = new AuthService;
            return instance_;
        }

        int Start() override;
        void Stop() override;

        [[nodiscard]] bool IsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired);
        [[nodiscard]] UNAUTHORIZED_REASON Authorize( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired );
        void CreateToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
        [[nodiscard]] bool SetPassword(const std::string &Password, SecurityObjects::UserInfo & UInfo);
        [[nodiscard]] const std:: string & PasswordValidationExpression() const { return PasswordValidationStr_;};
        void Logout(const std::string &token, bool EraseFromCache=true);

        [[nodiscard]] bool IsSubAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired);
        [[nodiscard]] UNAUTHORIZED_REASON AuthorizeSub( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired );
        void CreateSubToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo);
        [[nodiscard]] bool SetSubPassword(const std::string &Password, SecurityObjects::UserInfo & UInfo);
        [[nodiscard]] const std:: string & SubPasswordValidationExpression() const { return PasswordValidationStr_;};
        void SubLogout(const std::string &token, bool EraseFromCache=true);

        void RemoveTokenSystemWide(const std::string &token);

        bool ValidatePassword(const std::string &pwd);
        bool ValidateSubPassword(const std::string &pwd);

        [[nodiscard]] bool IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo, bool & Expired);
        [[nodiscard]] bool IsValidSubToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo, bool & Expired);
        [[nodiscard]] std::string GenerateTokenJWT(const std::string & UserName, ACCESS_TYPE Type);
        [[nodiscard]] std::string GenerateTokenHMAC(const std::string & UserName, ACCESS_TYPE Type);

        [[nodiscard]] std::string ComputeNewPasswordHash(const std::string &UserName, const std::string &Password);
        [[nodiscard]] bool ValidatePasswordHash(const std::string & UserName, const std::string & Password, const std::string &StoredPassword);
        [[nodiscard]] bool ValidateSubPasswordHash(const std::string & UserName, const std::string & Password, const std::string &StoredPassword);

        [[nodiscard]] bool UpdatePassword(const std::string &Admin, const std::string &UserName, const std::string & OldPassword, const std::string &NewPassword);
        [[nodiscard]] std::string ResetPassword(const std::string &Admin, const std::string &UserName);

        [[nodiscard]] bool UpdateSubPassword(const std::string &Admin, const std::string &UserName, const std::string & OldPassword, const std::string &NewPassword);
        [[nodiscard]] std::string ResetSubPassword(const std::string &Admin, const std::string &UserName);

        [[nodiscard]] static bool VerifyEmail(SecurityObjects::UserInfo &UInfo);
        [[nodiscard]] static bool VerifySubEmail(SecurityObjects::UserInfo &UInfo);

        [[nodiscard]] static bool SendEmailToUser(const std::string &LinkId, std::string &Email, EMAIL_REASON Reason);
        [[nodiscard]] static bool SendEmailToSubUser(const std::string &LinkId, std::string &Email, EMAIL_REASON Reason);
        [[nodiscard]] bool RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo);

        bool DeleteUserFromCache(const std::string &UserName);
        bool DeleteSubUserFromCache(const std::string &UserName);
        void RevokeToken(std::string & Token);
        void RevokeSubToken(std::string & Token);

        [[nodiscard]] static inline const std::string GetLogoAssetURI() {
            return MicroService::instance().PublicEndPoint() + "/wwwassets/the_logo.png";
        }

        [[nodiscard]] static inline const std::string GetLogoAssetFileName() {
            return MicroService::instance().WWWAssetsDir() + "/the_logo.png";
        }

        inline const std::string & GetPasswordPolicy() const { return PasswordPolicy_; }
        inline const std::string & GetAccessPolicy() const { return AccessPolicy_; }

        inline const std::string & GetSubPasswordPolicy() const { return SubPasswordPolicy_; }
        inline const std::string & GetSubAccessPolicy() const { return SubAccessPolicy_; }

    private:
		Poco::JWT::Signer	Signer_;
		Poco::SHA2Engine	SHA2_;

		std::string         AccessPolicy_;
		std::string         PasswordPolicy_;
		std::string         SubAccessPolicy_;
		std::string         SubPasswordPolicy_;
		std::string         PasswordValidationStr_;
        std::string         SubPasswordValidationStr_;
        std::regex          PasswordValidation_;
        std::regex          SubPasswordValidation_;

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

    inline auto AuthService() { return AuthService::instance(); }

    [[nodiscard]] inline bool AuthServiceIsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo , bool & Expired, bool Sub ) {
        if(Sub)
            return AuthService()->IsSubAuthorized(Request, SessionToken, UInfo, Expired );
        else
            return AuthService()->IsAuthorized(Request, SessionToken, UInfo, Expired );
    }

} // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
