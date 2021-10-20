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

#include "Daemon.h"
#include "framework/RESTAPI_handler.h"
#include "StorageService.h"
#include "AuthService.h"
#include "framework/Utils.h"
#include "framework/KafkaManager.h"
#include "framework/Kafka_topics.h"

#include "SMTPMailerService.h"
#include "MFAServer.h"

namespace OpenWifi {
    class AuthService *AuthService::instance_ = nullptr;

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
		Signer_.setRSAKey(Daemon()->Key());
		Signer_.addAllAlgorithms();
		Logger_.notice("Starting...");
        Secure_ = Daemon()->ConfigGetBool("authentication.enabled",true);
        DefaultPassword_ = Daemon()->ConfigGetString("authentication.default.password","");
        DefaultUserName_ = Daemon()->ConfigGetString("authentication.default.username","");
        Mechanism_ = Daemon()->ConfigGetString("authentication.service.type","internal");
        PasswordValidation_ = PasswordValidationStr_ = Daemon()->ConfigGetString("authentication.validation.expression","^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$");
        TokenAging_ = (uint64_t) Daemon()->ConfigGetInt("authentication.token.ageing", 30 * 24 * 60 * 60);
        HowManyOldPassword_ = Daemon()->ConfigGetInt("authentication.oldpasswords", 5);
        return 0;
    }

    void AuthService::Stop() {
		Logger_.notice("Stopping...");
    }

	bool AuthService::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo )
    {
        if(!Secure_)
            return true;

        std::lock_guard		Guard(Mutex_);

		std::string CallToken;

		try {
			Poco::Net::OAuth20Credentials Auth(Request);

			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}
		} catch(const Poco::Exception &E) {
		}

		if(!CallToken.empty()) {
		    if(Storage()->IsTokenRevoked(CallToken))
		        return false;
		    auto Client = UserCache_.find(CallToken);
		    if( Client == UserCache_.end() )
		        return ValidateToken(CallToken, CallToken, UInfo);

		    if((Client->second.webtoken.created_ + Client->second.webtoken.expires_in_) > time(nullptr)) {
		        SessionToken = CallToken;
		        UInfo = Client->second ;
		        return true;
		    }
		    UserCache_.erase(CallToken);
		    Storage()->RevokeToken(CallToken);
		    return false;
		}

		return false;
    }

    bool AuthService::DeleteUserFromCache(const std::string &UserName) {
        std::lock_guard		Guard(Mutex_);

        for(auto i=UserCache_.begin();i!=UserCache_.end();) {
            if (i->second.userinfo.email==UserName) {
                Logout(i->first);
                i = UserCache_.erase(i);
            } else {
                ++i;
            }
        }
        return true;
    }

    bool AuthService::RequiresMFA(const SecurityObjects::UserInfoAndPolicy &UInfo) {
        return (UInfo.userinfo.userTypeProprietaryInfo.mfa.enabled && MFAServer().MethodEnabled(UInfo.userinfo.userTypeProprietaryInfo.mfa.method));
    }

    bool AuthService::ValidatePassword(const std::string &Password) {
        return std::regex_match(Password, PasswordValidation_);
    }

    void AuthService::Logout(const std::string &token) {
		std::lock_guard		Guard(Mutex_);

		UserCache_.erase(token);

        try {
            Poco::JSON::Object Obj;
            Obj.set("event", "remove-token");
            Obj.set("id", Daemon()->ID());
            Obj.set("token", token);
            std::stringstream ResultText;
            Poco::JSON::Stringifier::stringify(Obj, ResultText);
            std::string Tmp{token};
            Storage()->RevokeToken(Tmp);
            KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS, Daemon()->PrivateEndPoint(), ResultText.str(),
                                        false);
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
    }

    std::string AuthService::GenerateToken(const std::string & Identity, ACCESS_TYPE Type) {
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

	bool AuthService::ValidateToken(const std::string & Token, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo  ) {
        std::lock_guard		Guard(Mutex_);
		Poco::JWT::Token	DecryptedToken;

		try {
            auto E = UserCache_.find(SessionToken);
            if(E == UserCache_.end()) {
                if(Storage()->GetToken(SessionToken,UInfo)) {
                    if(Storage()->GetUserById(UInfo.userinfo.email,UInfo.userinfo)) {
                        UserCache_[UInfo.webtoken.access_token_] = UInfo;
                        return true;
                    }
                }
            } else {
                UInfo = E->second;
                return true;
            }
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
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
        UInfo.webtoken.access_token_ = GenerateToken(UInfo.userinfo.Id,USERNAME);
        UInfo.webtoken.id_token_ = GenerateToken(UInfo.userinfo.Id,USERNAME);
        UInfo.webtoken.refresh_token_ = GenerateToken(UInfo.userinfo.Id,CUSTOM);
        UInfo.webtoken.created_ = time(nullptr);
        UInfo.webtoken.username_ = UserName;
        UInfo.webtoken.errorCode = 0;
        UInfo.webtoken.userMustChangePassword = false;
        UserCache_[UInfo.webtoken.access_token_] = UInfo;
        Storage()->SetLastLogin(UInfo.userinfo.Id);
        Storage()->AddToken(UInfo.webtoken.username_, UInfo.webtoken.access_token_,
                            UInfo.webtoken.refresh_token_, UInfo.webtoken.token_type_,
                                UInfo.webtoken.expires_in_, UInfo.webtoken.idle_timeout_);
    }

    bool AuthService::SetPassword(const std::string &NewPassword, SecurityObjects::UserInfo & UInfo) {
        auto NewPasswordHash = ComputePasswordHash(UInfo.email, NewPassword);
        for (auto const &i:UInfo.lastPasswords) {
            if (i == NewPasswordHash) {
                return false;
            }
        }
        if(UInfo.lastPasswords.size()==HowManyOldPassword_) {
            UInfo.lastPasswords.erase(UInfo.lastPasswords.begin());
        }
        UInfo.lastPasswords.push_back(NewPasswordHash);
        UInfo.currentPassword = NewPasswordHash;
        UInfo.changePassword = false;
        return true;
    }

    AuthService::AUTH_ERROR AuthService::Authorize( std::string & UserName, const std::string & Password, const std::string & NewPassword, SecurityObjects::UserInfoAndPolicy & UInfo )
    {
        std::lock_guard		Guard(Mutex_);
        SecurityObjects::AclTemplate	ACL;

        Poco::toLowerInPlace(UserName);
        auto PasswordHash = ComputePasswordHash(UserName, Password);

        if(Storage()->GetUserByEmail(UserName,UInfo.userinfo)) {
            if(UInfo.userinfo.waitingForEmailCheck) {
                return USERNAME_PENDING_VERIFICATION;
            }

            if(PasswordHash != UInfo.userinfo.currentPassword) {
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
                Storage()->UpdateUserInfo(AUTHENTICATION_SYSTEM, UInfo.userinfo.Id,UInfo.userinfo);
            }

            //  so we have a good password, password up date has taken place if need be, now generate the token.
            UInfo.userinfo.lastLogin=std::time(nullptr);
            Storage()->SetLastLogin(UInfo.userinfo.Id);
            CreateToken(UserName, UInfo );
            return SUCCESS;
        }

        if(((UserName == DefaultUserName_) && (DefaultPassword_== ComputePasswordHash(UserName,Password))) || !Secure_)
        {
            ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
            UInfo.webtoken.acl_template_ = ACL;
            UInfo.userinfo.email = DefaultUserName_;
            UInfo.userinfo.currentPassword = DefaultPassword_;
            UInfo.userinfo.name = DefaultUserName_;
            CreateToken(UserName, UInfo );
            return SUCCESS;
        }
        return INVALID_CREDENTIALS;
    }

    std::string AuthService::ComputePasswordHash(const std::string &UserName, const std::string &Password) {
        std::string UName = Poco::trim(Poco::toLower(UserName));
        SHA2_.update(Password + UName);
        return Utils::ToHex(SHA2_.digest());
    }

    bool AuthService::SendEmailToUser(std::string &Email, EMAIL_REASON Reason) {
        SecurityObjects::UserInfo   UInfo;

        if(Storage()->GetUserByEmail(Email,UInfo)) {
            switch (Reason) {
                case FORGOT_PASSWORD: {
                        MessageAttributes Attrs;

                        Attrs[RECIPIENT_EMAIL] = UInfo.email;
                        Attrs[LOGO] = "logo.jpg";
                        Attrs[SUBJECT] = "Password reset link";
                        Attrs[ACTION_LINK] =
                                Daemon()->GetPublicAPIEndPoint() + "/actionLink?action=password_reset&id=" + UInfo.Id ;
                        SMTPMailerService()->SendMessage(UInfo.email, "password_reset.txt", Attrs);
                    }
                    break;

                case EMAIL_VERIFICATION: {
                        MessageAttributes Attrs;

                        Attrs[RECIPIENT_EMAIL] = UInfo.email;
                        Attrs[LOGO] = "logo.jpg";
                        Attrs[SUBJECT] = "EMail Address Verification";
                        Attrs[ACTION_LINK] =
                                Daemon()->GetPublicAPIEndPoint() + "/actionLink?action=email_verification&id=" + UInfo.Id ;
                        SMTPMailerService()->SendMessage(UInfo.email, "email_verification.txt", Attrs);
                        UInfo.waitingForEmailCheck = true;
                    }
                    break;

                default:
                    break;
            }
        }
        return false;
    }

    bool AuthService::VerifyEmail(SecurityObjects::UserInfo &UInfo) {
        MessageAttributes Attrs;

        Attrs[RECIPIENT_EMAIL] = UInfo.email;
        Attrs[LOGO] = "logo.jpg";
        Attrs[SUBJECT] = "EMail Address Verification";
        Attrs[ACTION_LINK] =
                Daemon()->GetPublicAPIEndPoint() + "/actionLink?action=email_verification&id=" + UInfo.Id ;
        SMTPMailerService()->SendMessage(UInfo.email, "email_verification.txt", Attrs);
        UInfo.waitingForEmailCheck = true;
        return true;
    }

    bool AuthService::IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo) {
        std::lock_guard G(Mutex_);
        auto It = UserCache_.find(Token);

        if(It==UserCache_.end())
            return false;
        WebToken = It->second.webtoken;
        UserInfo = It->second.userinfo;
        return true;
    }


}  // end of namespace
