//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "Poco/Net/OAuth20Credentials.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"
#include "Poco/StringTokenizer.h"

#include "framework/MicroService.h"
#include "StorageService.h"
#include "AuthService.h"
#include "framework/KafkaTopics.h"

#include "SMTPMailerService.h"
#include "MFAServer.h"

namespace OpenWifi {

    AuthService::ACCESS_TYPE AuthService::IntToAccessType(int C) {
		switch (C) {
		case 1: return USERNAME;
		case 2: return SERVER;
		case 3: return CUSTOM;
		default:
			return USERNAME;
		}
	}

	int AuthService::AccessTypeToInt(ACCESS_TYPE T) {
		switch (T) {
		case USERNAME: return 1;
		case SERVER: return 2;
		case CUSTOM: return 3;
		}
		return 1;	// some compilers complain...
	}

    int AuthService::Start() {
		Signer_.setRSAKey(MicroService::instance().Key());
		Signer_.addAllAlgorithms();
		Logger().notice("Starting...");
        TokenAging_ = (uint64_t) MicroService::instance().ConfigGetInt("authentication.token.ageing", 30 * 24 * 60 * 60);
        HowManyOldPassword_ = MicroService::instance().ConfigGetInt("authentication.oldpasswords", 5);

        AccessPolicy_ = MicroService::instance().ConfigPath("openwifi.document.policy.access", "/wwwassets/access_policy.html");
        PasswordPolicy_ = MicroService::instance().ConfigPath("openwifi.document.policy.password", "/wwwassets/password_policy.html");
        PasswordValidation_ = PasswordValidationStr_ = MicroService::instance().ConfigGetString("authentication.validation.expression","^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$");

        SubPasswordValidation_ = SubPasswordValidationStr_ = MicroService::instance().ConfigGetString("subscriber.validation.expression","^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$");
        SubAccessPolicy_ = MicroService::instance().ConfigPath("subscriber.policy.access", "/wwwassets/access_policy.html");
        SubPasswordPolicy_ = MicroService::instance().ConfigPath("subscriber.policy.password", "/wwwassets/password_policy.html");

        return 0;
    }

    void AuthService::Stop() {
		Logger().notice("Stopping...");
    }

	bool AuthService::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired )
    {
        std::lock_guard	Guard(Mutex_);
        Expired = false;
		try {
		    std::string CallToken;
		    Poco::Net::OAuth20Credentials Auth(Request);
		    if (Auth.getScheme() == "Bearer") {
		        CallToken = Auth.getBearerToken();
		    }

            if(CallToken.empty()) {
                return false;
            }

            SecurityObjects::WebToken   WT;
            uint64_t                    RevocationDate=0;
            std::string                 UserId;
            if(StorageService()->UserTokenDB().GetToken(CallToken, WT, UserId, RevocationDate)) {
                if(RevocationDate!=0)
                    return false;
                Expired = (WT.created_ + WT.expires_in_) < time(nullptr);
                if(StorageService()->UserDB().GetUserById(UserId,UInfo.userinfo)) {
                    UInfo.webtoken = WT;
                    SessionToken = CallToken;
                    return true;
                }
            }
            return false;
		} catch(const Poco::Exception &E) {
		    Logger().log(E);
		}
		return false;
    }

    bool AuthService::IsSubAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired )
    {
        std::lock_guard	Guard(Mutex_);
        Expired = false;
        try {
            std::string CallToken;
            Poco::Net::OAuth20Credentials Auth(Request);
            if (Auth.getScheme() == "Bearer") {
                CallToken = Auth.getBearerToken();
            }

            if(CallToken.empty()) {
                return false;
            }

            SecurityObjects::WebToken   WT;
            uint64_t                    RevocationDate=0;
            std::string                 UserId;
            if(StorageService()->SubTokenDB().GetToken(CallToken, WT, UserId, RevocationDate)) {
                if(RevocationDate!=0)
                    return false;
                Expired = (WT.created_ + WT.expires_in_) < time(nullptr);
                if(StorageService()->SubDB().GetUserById(UserId,UInfo.userinfo)) {
                    UInfo.webtoken = WT;
                    SessionToken = CallToken;
                    return true;
                }
            }
            return false;
        } catch(const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    void AuthService::RevokeToken(std::string & Token) {
        StorageService()->UserTokenDB().RevokeToken(Token);
    }

    void AuthService::RevokeSubToken(std::string & Token) {
        StorageService()->SubTokenDB().RevokeToken(Token);
    }

    bool AuthService::DeleteUserFromCache(const std::string &Id) {
        return StorageService()->UserTokenDB().DeleteRecordsFromCache("userName",Id);
    }

    bool AuthService::DeleteSubUserFromCache(const std::string &Id) {
        return StorageService()->SubTokenDB().DeleteRecordsFromCache("userName",Id);
    }

    bool AuthService::RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo) {
        return (UInfo.userinfo.userTypeProprietaryInfo.mfa.enabled && MFAServer::MethodEnabled(UInfo.userinfo.userTypeProprietaryInfo.mfa.method));
    }

    bool AuthService::ValidatePassword(const std::string &Password) {
        return std::regex_match(Password, PasswordValidation_);
    }

    bool AuthService::ValidateSubPassword(const std::string &Password) {
        return std::regex_match(Password, SubPasswordValidation_);
    }

    void AuthService::RemoveTokenSystemWide(const std::string &token) {
        try {
            if(KafkaManager()->Enabled()) {
                Poco::JSON::Object Obj;
                Obj.set("event", "remove-token");
                Obj.set("id", MicroService::instance().ID());
                Obj.set("token", token);
                std::stringstream ResultText;
                Poco::JSON::Stringifier::stringify(Obj, ResultText);
                KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS, MicroService::instance().PrivateEndPoint(),
                                            ResultText.str(),
                                            false);
            }
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
    }

    void AuthService::Logout(const std::string &Token, bool EraseFromCache) {
		std::lock_guard		Guard(Mutex_);

        try {
            auto tToken{Token};
            StorageService()->UserTokenDB().DeleteRecord("token",tToken);
            StorageService()->LoginDB().AddLogout(Token);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
    }

    void AuthService::SubLogout(const std::string &Token, bool EraseFromCache) {
        std::lock_guard		Guard(Mutex_);

        try {
            auto tToken{Token};
            StorageService()->SubTokenDB().DeleteRecord("token",tToken);
            StorageService()->SubLoginDB().AddLogout(Token);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
    }

    [[nodiscard]] std::string AuthService::GenerateTokenHMAC(const std::string & UserName, ACCESS_TYPE Type) {
        std::string Identity(UserName + ":" + Poco::format("%d",(int)std::time(nullptr)) + ":" + std::to_string(rand()));
        HMAC_.update(Identity);
        return Poco::DigestEngine::digestToHex(HMAC_.digest());
    }

    std::string AuthService::GenerateTokenJWT(const std::string & Identity, ACCESS_TYPE Type) {
        std::lock_guard		Guard(Mutex_);

		Poco::JWT::Token	T;

		T.setType("JWT");
		switch(Type) {
			case USERNAME:	T.setSubject("usertoken"); break;
			case SERVER: 	T.setSubject("servertoken"); break;
			case CUSTOM:	T.setSubject("customtoken"); break;
		}

		T.payload().set("identity", Identity);
		T.setIssuedAt(Poco::Timestamp());
		T.setExpiration(Poco::Timestamp() + (long long)TokenAging_);
		std::string JWT = Signer_.sign(T,Poco::JWT::Signer::ALGO_RS256);

		return JWT;
    }

    void AuthService::CreateToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo)
    {
        std::lock_guard		Guard(Mutex_);

        SecurityObjects::AclTemplate	ACL;
        ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
        UInfo.webtoken.acl_template_ = ACL;
        UInfo.webtoken.expires_in_ = TokenAging_ ;
        UInfo.webtoken.idle_timeout_ = 5 * 60;
        UInfo.webtoken.token_type_ = "Bearer";
        UInfo.webtoken.access_token_ = GenerateTokenHMAC(UInfo.userinfo.id,USERNAME);
        UInfo.webtoken.id_token_ = GenerateTokenHMAC(UInfo.userinfo.id,USERNAME);
        UInfo.webtoken.refresh_token_ = GenerateTokenHMAC(UInfo.userinfo.id,CUSTOM);
        UInfo.webtoken.created_ = time(nullptr);
        UInfo.webtoken.username_ = UserName;
        UInfo.webtoken.errorCode = 0;
        UInfo.webtoken.userMustChangePassword = false;
        StorageService()->UserDB().SetLastLogin(UInfo.userinfo.id);
        StorageService()->UserTokenDB().AddToken(UInfo.userinfo.id, UInfo.webtoken.access_token_,
                            UInfo.webtoken.refresh_token_, UInfo.webtoken.token_type_,
                                UInfo.webtoken.expires_in_, UInfo.webtoken.idle_timeout_);
        StorageService()->LoginDB().AddLogin(UInfo.userinfo.id, UInfo.userinfo.email,UInfo.webtoken.access_token_ );
    }

    void AuthService::CreateSubToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo)
    {
        std::lock_guard		Guard(Mutex_);

        SecurityObjects::AclTemplate	ACL;
        ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
        UInfo.webtoken.acl_template_ = ACL;
        UInfo.webtoken.expires_in_ = TokenAging_ ;
        UInfo.webtoken.idle_timeout_ = 5 * 60;
        UInfo.webtoken.token_type_ = "Bearer";
        UInfo.webtoken.access_token_ = GenerateTokenHMAC(UInfo.userinfo.id,USERNAME);
        UInfo.webtoken.id_token_ = GenerateTokenHMAC(UInfo.userinfo.id,USERNAME);
        UInfo.webtoken.refresh_token_ = GenerateTokenHMAC(UInfo.userinfo.id,CUSTOM);
        UInfo.webtoken.created_ = time(nullptr);
        UInfo.webtoken.username_ = UserName;
        UInfo.webtoken.errorCode = 0;
        UInfo.webtoken.userMustChangePassword = false;
        StorageService()->SubDB().SetLastLogin(UInfo.userinfo.id);
        StorageService()->SubTokenDB().AddToken(UInfo.userinfo.id, UInfo.webtoken.access_token_,
                                   UInfo.webtoken.refresh_token_, UInfo.webtoken.token_type_,
                                   UInfo.webtoken.expires_in_, UInfo.webtoken.idle_timeout_);
        StorageService()->SubLoginDB().AddLogin(UInfo.userinfo.id, UInfo.userinfo.email,UInfo.webtoken.access_token_ );
    }

    bool AuthService::SetPassword(const std::string &NewPassword, SecurityObjects::UserInfo & UInfo) {
        std::lock_guard     G(Mutex_);

        Poco::toLowerInPlace(UInfo.email);
        for (const auto &i:UInfo.lastPasswords) {
            auto Tokens = Poco::StringTokenizer(i,"|");
            if(Tokens.count()==2) {
                const auto & Salt = Tokens[0];
                for(const auto &j:UInfo.lastPasswords) {
                    auto OldTokens = Poco::StringTokenizer(j,"|");
                    if(OldTokens.count()==2) {
                        SHA2_.update(Salt+NewPassword+UInfo.email);
                        if(OldTokens[1]==Utils::ToHex(SHA2_.digest()))
                            return false;
                    }
                }
            } else {
                SHA2_.update(NewPassword+UInfo.email);
                if(Tokens[0]==Utils::ToHex(SHA2_.digest()))
                    return false;
            }
        }

        if(UInfo.lastPasswords.size()==HowManyOldPassword_) {
            UInfo.lastPasswords.erase(UInfo.lastPasswords.begin());
        }

        auto NewHash = ComputeNewPasswordHash(UInfo.email,NewPassword);
        UInfo.lastPasswords.push_back(NewHash);
        UInfo.currentPassword = NewHash;
        UInfo.changePassword = false;
        return true;
    }

    bool AuthService::SetSubPassword(const std::string &NewPassword, SecurityObjects::UserInfo & UInfo) {
        std::lock_guard     G(Mutex_);

        Poco::toLowerInPlace(UInfo.email);
        for (const auto &i:UInfo.lastPasswords) {
            auto Tokens = Poco::StringTokenizer(i,"|");
            if(Tokens.count()==2) {
                const auto & Salt = Tokens[0];
                for(const auto &j:UInfo.lastPasswords) {
                    auto OldTokens = Poco::StringTokenizer(j,"|");
                    if(OldTokens.count()==2) {
                        SHA2_.update(Salt+NewPassword+UInfo.email);
                        if(OldTokens[1]==Utils::ToHex(SHA2_.digest()))
                            return false;
                    }
                }
            } else {
                SHA2_.update(NewPassword+UInfo.email);
                if(Tokens[0]==Utils::ToHex(SHA2_.digest()))
                    return false;
            }
        }

        if(UInfo.lastPasswords.size()==HowManyOldPassword_) {
            UInfo.lastPasswords.erase(UInfo.lastPasswords.begin());
        }

        auto NewHash = ComputeNewPasswordHash(UInfo.email,NewPassword);
        UInfo.lastPasswords.push_back(NewHash);
        UInfo.currentPassword = NewHash;
        UInfo.changePassword = false;
        return true;
    }

    static std::string GetMeSomeSalt() {
        auto start = std::chrono::high_resolution_clock::now();
        return std::to_string(start.time_since_epoch().count());
    }

    std::string AuthService::ComputeNewPasswordHash(const std::string &UserName, const std::string &Password) {
        std::string UName = Poco::trim(Poco::toLower(UserName));
        auto Salt = GetMeSomeSalt();
        SHA2_.update(Salt + Password + UName );
        return Salt + "|" + Utils::ToHex(SHA2_.digest());
    }

    bool AuthService::ValidatePasswordHash(const std::string & UserName, const std::string & Password, const std::string &StoredPassword) {
        std::lock_guard G(Mutex_);

        std::string UName = Poco::trim(Poco::toLower(UserName));
        auto Tokens = Poco::StringTokenizer(StoredPassword,"|");
        if(Tokens.count()==1) {
            SHA2_.update(Password+UName);
            if(Tokens[0]==Utils::ToHex(SHA2_.digest()))
                return true;
        } else if (Tokens.count()==2) {
            SHA2_.update(Tokens[0]+Password+UName);
            if(Tokens[1]==Utils::ToHex(SHA2_.digest()))
                return true;
        }
        return false;
    }

    bool AuthService::ValidateSubPasswordHash(const std::string & UserName, const std::string & Password, const std::string &StoredPassword) {
        std::lock_guard G(Mutex_);

        std::string UName = Poco::trim(Poco::toLower(UserName));
        auto Tokens = Poco::StringTokenizer(StoredPassword,"|");
        if(Tokens.count()==1) {
            SHA2_.update(Password+UName);
            if(Tokens[0]==Utils::ToHex(SHA2_.digest()))
                return true;
        } else if (Tokens.count()==2) {
            SHA2_.update(Tokens[0]+Password+UName);
            if(Tokens[1]==Utils::ToHex(SHA2_.digest()))
                return true;
        }
        return false;
    }

    UNAUTHORIZED_REASON AuthService::Authorize( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo , bool & Expired )
    {
        std::lock_guard		Guard(Mutex_);

        Poco::toLowerInPlace(UserName);

        if(StorageService()->UserDB().GetUserByEmail(UserName,UInfo.userinfo)) {
            if(UInfo.userinfo.waitingForEmailCheck) {
                return USERNAME_PENDING_VERIFICATION;
            }

            if(!ValidatePasswordHash(UserName,Password,UInfo.userinfo.currentPassword)) {
                return INVALID_CREDENTIALS;
            }

            if(UInfo.userinfo.changePassword && NewPassword.empty()) {
                UInfo.webtoken.userMustChangePassword = true ;
                return PASSWORD_CHANGE_REQUIRED;
            }

            if(!NewPassword.empty() && !ValidatePassword(NewPassword)) {
                return PASSWORD_INVALID;
            }

            if(UInfo.userinfo.changePassword || !NewPassword.empty()) {
                if(!SetPassword(NewPassword,UInfo.userinfo)) {
                    UInfo.webtoken.errorCode = 1;
                    return PASSWORD_ALREADY_USED;
                }
                UInfo.userinfo.lastPasswordChange = std::time(nullptr);
                UInfo.userinfo.changePassword = false;
                UInfo.userinfo.modified = std::time(nullptr);
                StorageService()->UserDB().UpdateUserInfo(AUTHENTICATION_SYSTEM, UInfo.userinfo.id,UInfo.userinfo);
            }

            //  so we have a good password, password up date has taken place if need be, now generate the token.
            UInfo.userinfo.lastLogin=std::time(nullptr);
            StorageService()->UserDB().SetLastLogin(UInfo.userinfo.id);
            CreateToken(UserName, UInfo );

            return SUCCESS;
        }

        return INVALID_CREDENTIALS;
    }

    UNAUTHORIZED_REASON AuthService::AuthorizeSub( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo , bool & Expired )
    {
        std::lock_guard		Guard(Mutex_);

        Poco::toLowerInPlace(UserName);

        if(StorageService()->SubDB().GetUserByEmail(UserName,UInfo.userinfo)) {
            if(UInfo.userinfo.waitingForEmailCheck) {
                return USERNAME_PENDING_VERIFICATION;
            }

            if(!ValidateSubPasswordHash(UserName,Password,UInfo.userinfo.currentPassword)) {
                return INVALID_CREDENTIALS;
            }

            if(UInfo.userinfo.changePassword && NewPassword.empty()) {
                UInfo.webtoken.userMustChangePassword = true ;
                return PASSWORD_CHANGE_REQUIRED;
            }

            if(!NewPassword.empty() && !ValidateSubPassword(NewPassword)) {
                return PASSWORD_INVALID;
            }

            if(UInfo.userinfo.changePassword || !NewPassword.empty()) {
                if(!SetSubPassword(NewPassword,UInfo.userinfo)) {
                    UInfo.webtoken.errorCode = 1;
                    return PASSWORD_ALREADY_USED;
                }
                UInfo.userinfo.lastPasswordChange = std::time(nullptr);
                UInfo.userinfo.changePassword = false;
                UInfo.userinfo.modified = std::time(nullptr);
                StorageService()->SubDB().UpdateUserInfo(AUTHENTICATION_SYSTEM, UInfo.userinfo.id,UInfo.userinfo);
            }

            //  so we have a good password, password up date has taken place if need be, now generate the token.
            UInfo.userinfo.lastLogin=std::time(nullptr);
            StorageService()->SubDB().SetLastLogin(UInfo.userinfo.id);
            CreateSubToken(UserName, UInfo );

            return SUCCESS;
        }

        return INVALID_CREDENTIALS;
    }

    bool AuthService::SendEmailToUser(const std::string &LinkId, std::string &Email, EMAIL_REASON Reason) {
        SecurityObjects::UserInfo   UInfo;

        if(StorageService()->UserDB().GetUserByEmail(Email,UInfo)) {
            switch (Reason) {

                case FORGOT_PASSWORD: {
                        MessageAttributes Attrs;
                        Attrs[RECIPIENT_EMAIL] = UInfo.email;
                        Attrs[LOGO] = GetLogoAssetURI();
                        Attrs[SUBJECT] = "Password reset link";
                        Attrs[ACTION_LINK] = MicroService::instance().GetPublicAPIEndPoint() + "/actionLink?action=password_reset&id=" + LinkId ;
                        SMTPMailerService()->SendMessage(UInfo.email, "password_reset.txt", Attrs);
                    }
                    break;

                case EMAIL_VERIFICATION: {
                        MessageAttributes Attrs;
                        Attrs[RECIPIENT_EMAIL] = UInfo.email;
                        Attrs[LOGO] = GetLogoAssetURI();
                        Attrs[SUBJECT] = "EMail Address Verification";
                        Attrs[ACTION_LINK] = MicroService::instance().GetPublicAPIEndPoint() + "/actionLink?action=email_verification&id=" + LinkId ;
                        SMTPMailerService()->SendMessage(UInfo.email, "email_verification.txt", Attrs);
                        UInfo.waitingForEmailCheck = true;
                    }
                    break;

                default:
                    break;
            }
            return true;
        }
        return false;
    }

    bool AuthService::SendEmailToSubUser(const std::string &LinkId, std::string &Email, EMAIL_REASON Reason) {
        SecurityObjects::UserInfo   UInfo;

        if(StorageService()->SubDB().GetUserByEmail(Email,UInfo)) {
            switch (Reason) {

                case FORGOT_PASSWORD: {
                    MessageAttributes Attrs;
                    Attrs[RECIPIENT_EMAIL] = UInfo.email;
                    Attrs[LOGO] = GetLogoAssetURI();
                    Attrs[SUBJECT] = "Password reset link";
                    Attrs[ACTION_LINK] = MicroService::instance().GetPublicAPIEndPoint() + "/actionLink?action=password_reset&id=" + LinkId ;
                    SMTPMailerService()->SendMessage(UInfo.email, "password_reset.txt", Attrs);
                }
                break;

                case EMAIL_VERIFICATION: {
                    MessageAttributes Attrs;
                    Attrs[RECIPIENT_EMAIL] = UInfo.email;
                    Attrs[LOGO] = GetLogoAssetURI();
                    Attrs[SUBJECT] = "EMail Address Verification";
                    Attrs[ACTION_LINK] = MicroService::instance().GetPublicAPIEndPoint() + "/actionLink?action=email_verification&id=" + LinkId ;
                    SMTPMailerService()->SendMessage(UInfo.email, "email_verification.txt", Attrs);
                    UInfo.waitingForEmailCheck = true;
                }
                break;

                case SIGNUP_VERIFICATION: {
                    MessageAttributes Attrs;
                    std::cout << "Signup verification " << __LINE__ << std::endl;
                    Attrs[RECIPIENT_EMAIL] = UInfo.email;
                    Attrs[LOGO] = GetLogoAssetURI();
                    Attrs[SUBJECT] = "EMail Address Verification";
                    Attrs[ACTION_LINK] = MicroService::instance().GetPublicAPIEndPoint() + "/actionLink?action=signup_verification&id=" + LinkId ;
                    std::cout << "Signup verification " << __LINE__ << std::endl;
                    SMTPMailerService()->SendMessage(UInfo.email, "signup_verification.txt", Attrs);
                    UInfo.waitingForEmailCheck = true;
                    std::cout << "Signup verification " << __LINE__ << std::endl;
                }
                break;

                default:
                    break;
            }
            return true;
        }
        return false;
    }

    bool AuthService::VerifyEmail(SecurityObjects::UserInfo &UInfo) {
        SecurityObjects::ActionLink A;

        A.action = OpenWifi::SecurityObjects::LinkActions::VERIFY_EMAIL;
        A.userId = UInfo.email;
        A.id = MicroService::CreateUUID();
        A.created = std::time(nullptr);
        A.expires = A.created + 24*60*60;
        A.userAction = true;
        StorageService()->ActionLinksDB().CreateAction(A);
        UInfo.waitingForEmailCheck = true;
        return true;
    }

    bool AuthService::VerifySubEmail(SecurityObjects::UserInfo &UInfo) {
        SecurityObjects::ActionLink A;

        A.action = OpenWifi::SecurityObjects::LinkActions::SUB_VERIFY_EMAIL;
        A.userId = UInfo.email;
        A.id = MicroService::CreateUUID();
        A.created = std::time(nullptr);
        A.expires = A.created + 24*60*60;
        A.userAction = false;
        StorageService()->ActionLinksDB().CreateAction(A);
        UInfo.waitingForEmailCheck = true;
        return true;
    }

    bool AuthService::IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo, bool & Expired) {

        std::lock_guard G(Mutex_);
        Expired = false;

        std::string TToken{Token}, UserId;
        SecurityObjects::WebToken   WT;
        uint64_t RevocationDate=0;
        if(StorageService()->UserTokenDB().GetToken(TToken, WT, UserId, RevocationDate)) {
            if(RevocationDate!=0)
                return false;
            Expired = (WT.created_ + WT.expires_in_) < std::time(nullptr);
            if(StorageService()->UserDB().GetUserById(UserId,UserInfo)) {
                WebToken = WT;
                return true;
            }
            return false;
        }
        return IsValidSubToken(Token, WebToken, UserInfo, Expired);
    }

    bool AuthService::IsValidSubToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo, bool & Expired) {
        std::lock_guard G(Mutex_);
        Expired = false;

        std::string TToken{Token}, UserId;
        SecurityObjects::WebToken   WT;
        uint64_t RevocationDate=0;
        if(StorageService()->SubTokenDB().GetToken(TToken, WT, UserId, RevocationDate)) {
            if(RevocationDate!=0)
                return false;
            Expired = (WT.created_ + WT.expires_in_) < std::time(nullptr);
            if(StorageService()->SubDB().GetUserById(UserId,UserInfo)) {
                WebToken = WT;
                return true;
            }
            return false;
        }
        return false;
    }

}  // end of namespace
