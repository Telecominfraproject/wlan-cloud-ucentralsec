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
#include "RESTAPI_handler.h"
#include "StorageService.h"
#include "AuthService.h"
#include "Utils.h"
#include "KafkaManager.h"
#include "Kafka_topics.h"

namespace uCentral {
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
        return 0;
    }

    void AuthService::Stop() {
		Logger_.notice("Stopping...");
    }

	bool AuthService::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo )
    {
        if(!Secure_)
            return true;

		SubMutexGuard		Guard(Mutex_);

		std::string CallToken;

		try {
			Poco::Net::OAuth20Credentials Auth(Request);

			if (Auth.getScheme() == "Bearer") {
				CallToken = Auth.getBearerToken();
			}
		} catch(const Poco::Exception &E) {
		}

		if(CallToken.empty())
			CallToken = Request.get("X-API-KEY ", "");

		if(CallToken.empty())
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
		return false;
    }

    void AuthService::Logout(const std::string &token) {
		SubMutexGuard		Guard(Mutex_);
        UserCache_.erase(token);

        try {
            Poco::JSON::Object Obj;
            Obj.set("event", "remove-token");
            Obj.set("id", Daemon()->ID());
            Obj.set("token", token);
            std::stringstream ResultText;
            Poco::JSON::Stringifier::stringify(Obj, ResultText);
            KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS, Daemon()->PrivateEndPoint(), ResultText.str(),
                                        false);
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
    }

    std::string AuthService::GenerateToken(const std::string & Identity, ACCESS_TYPE Type, int NumberOfDays) {
		SubMutexGuard		Guard(Mutex_);

		Poco::JWT::Token	T;

		T.setType("JWT");
		switch(Type) {
			case USERNAME:	T.setSubject("usertoken"); break;
			case SERVER: 	T.setSubject("servertoken"); break;
			case CUSTOM:	T.setSubject("customtoken"); break;
		}

		T.payload().set("identity", Identity);
		T.setIssuedAt(Poco::Timestamp());
		T.setExpiration(Poco::Timestamp() + Poco::Timespan(NumberOfDays,0,0,0,0));
		std::string JWT = Signer_.sign(T,Poco::JWT::Signer::ALGO_RS256);

		return JWT;
    }

	bool AuthService::ValidateToken(const std::string & Token, std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo  ) {
		SubMutexGuard		Guard(Mutex_);
		Poco::JWT::Token	DecryptedToken;

		try {
			if (Signer_.tryVerify(Token, DecryptedToken)) {
				auto Expires = DecryptedToken.getExpiration();
				if (Expires > Poco::Timestamp()) {
					auto Identity = DecryptedToken.payload().get("identity").toString();
					auto IssuedAt = DecryptedToken.getIssuedAt();
					auto Subject = DecryptedToken.getSubject();

                    UInfo.webtoken.access_token_ = Token;
                    UInfo.webtoken.refresh_token_= Token;
                    UInfo.webtoken.username_ = Identity;
                    UInfo.webtoken.id_token_ = Token;
                    UInfo.webtoken.token_type_ = "Bearer";
                    UInfo.webtoken.created_ = IssuedAt.epochTime();
                    UInfo.webtoken.expires_in_ = Expires.epochTime() - IssuedAt.epochTime();
                    UInfo.webtoken.idle_timeout_ = 5*60;

					if(Storage()->GetIdentityRights(Identity, UInfo.webtoken.acl_template_)) {
					} else {
						//	we can get in but we have no given rights... something is very wrong
						UInfo.webtoken.acl_template_.Read_ = true ;
                        UInfo.webtoken.acl_template_.ReadWriteCreate_ =
                        UInfo.webtoken.acl_template_.ReadWrite_ =
                        UInfo.webtoken.acl_template_.Delete_ = false;
                        UInfo.webtoken.acl_template_.PortalLogin_ = true;
					}

                    UserCache_[UInfo.webtoken.access_token_] = UInfo;

					return true;
				}
			}
		} catch (const Poco::Exception &E ) {
			Logger_.log(E);
		}
		return false;
	}

    void AuthService::CreateToken(const std::string & UserName, SecurityObjects::UserInfoAndPolicy &UInfo)
    {
		SubMutexGuard		Guard(Mutex_);

		std::string Token = GenerateToken(UserName,USERNAME,30);
        UInfo.webtoken.expires_in_ = 30 * 24 * 60 * 60 ;
        UInfo.webtoken.idle_timeout_ = 5 * 60;
        UInfo.webtoken.token_type_ = "Bearer";
        UInfo.webtoken.access_token_ = Token;
        UInfo.webtoken.id_token_ = Token;
        UInfo.webtoken.refresh_token_ = Token;
        UInfo.webtoken.created_ = time(nullptr);
        UInfo.webtoken.username_ = UserName;

        UserCache_[Token] = UInfo;
    }

    bool AuthService::Authorize( const std::string & UserName, const std::string & Password, SecurityObjects::UserInfoAndPolicy & UInfo )
    {
		SubMutexGuard					Guard(Mutex_);
        SecurityObjects::AclTemplate	ACL;

		if(Mechanism_=="internal")
        {
            if(((UserName == DefaultUserName_) && (DefaultPassword_== ComputePasswordHash(UserName,Password))) || !Secure_)
            {
                ACL.PortalLogin_ = ACL.Read_ = ACL.ReadWrite_ = ACL.ReadWriteCreate_ = ACL.Delete_ = true;
                UInfo.webtoken.acl_template_ = ACL;
                UInfo.userinfo.email = DefaultUserName_;
                UInfo.userinfo.currentPassword = DefaultPassword_;
                UInfo.userinfo.name = DefaultUserName_;
                CreateToken(UserName, UInfo );
                return true;
            }
        } else if (Mechanism_=="db") {
			auto PasswordHash = ComputePasswordHash(UserName, Password);

			std::string TUser{UserName};
			if(Storage()->GetIdentity(TUser,PasswordHash,USERNAME,ACL)) {
				CreateToken(UserName, UInfo);
				return true;
			}
		}
        return false;
    }

    std::string AuthService::ComputePasswordHash(const std::string &UserName, const std::string &Password) {
        std::string UName = Poco::trim(Poco::toLower(UserName));
        SHA2_.update(Password + UName);
        return uCentral::Utils::ToHex(SHA2_.digest());
    }

    bool AuthService::IsValidToken(const std::string &Token, SecurityObjects::WebToken &WebToken, SecurityObjects::UserInfo &UserInfo) {

        return true;
    }


}  // end of namespace
