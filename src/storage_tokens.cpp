
#include "StorageService.h"

namespace OpenWifi {

/*
    "Token			    TEXT PRIMARY KEY, "
    "RefreshToken       TEXT, "
    "TokenType          TEXT, "
    "UserName           TEXT, "
    "Created 		    BIGINT, "
    "Expires 		    BIGINT, "
    "IdleTimeOut        BIGINT, "
    "RevocationDate 	BIGINT "
*/

    bool Storage::AddToken(std::string &UserName, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);
            uint64_t Now = std::time(nullptr);
            uint64_t Z = 0;

            std::string St2{
                "INSERT INTO Tokens (Token, RefreshToken, TokenType, Username, Created, Expires, IdleTimeOut, RevocationDate) VALUES(?,?,?,?,?,?,?,?)"};

            Insert << ConvertParams(St2),
                Poco::Data::Keywords::use(Token),
                Poco::Data::Keywords::use(RefreshToken),
                Poco::Data::Keywords::use(TokenType),
                Poco::Data::Keywords::use(UserName),
                Poco::Data::Keywords::use(Now),
                Poco::Data::Keywords::use(Expires),
                Poco::Data::Keywords::use(TimeOut),
                Poco::Data::Keywords::use(Z);
            Insert.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetToken(std::string &Token, SecurityObjects::UserInfoAndPolicy &UInfo) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            uint32_t RevocationDate = 0 ;

            std::string St2{"SELECT Token, RefreshToken, TokenType, Username, Created, Expires, IdleTimeOut, RevocationDate From Tokens WHERE Token=?"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(UInfo.webtoken.access_token_),
                Poco::Data::Keywords::into(UInfo.webtoken.refresh_token_),
                Poco::Data::Keywords::into(UInfo.webtoken.token_type_),
                Poco::Data::Keywords::into(UInfo.userinfo.email),
                Poco::Data::Keywords::into(UInfo.webtoken.created_),
                Poco::Data::Keywords::into(UInfo.webtoken.expires_in_),
                Poco::Data::Keywords::into(UInfo.webtoken.idle_timeout_),
                Poco::Data::Keywords::into(RevocationDate),
                Poco::Data::Keywords::use(Token);
            Select.execute();

            if(RevocationDate>0)
               return false;
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::IsTokenRevoked(std::string &Token) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            uint32_t RevocationDate = 0 ;

            std::string St2{"SELECT Revoked From Tokens WHERE Token=?"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(RevocationDate),
                Poco::Data::Keywords::use(Token);
            Select.execute();

            if(Select.columnsExtracted()==0)
                return false;
            return RevocationDate>0 ;

        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::RevokeToken(std::string &Token) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint32_t Revoked = 1 ;
            uint64_t Now = std::time(nullptr);

            // update users set lastLogin=? where id=?
            std::string St2{"UPDATE Tokens  Set RevocationDate=? WHERE Token=?"};
            Update << ConvertParams(St2),
                Poco::Data::Keywords::use(Now),
                Poco::Data::Keywords::use(Token);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::CleanRevokedTokens(uint64_t Oldest) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);
            uint64_t Now = std::time(nullptr);

            std::string St2{"DELETE From Tokens WHERE Created <= ?"};
            Delete << ConvertParams(St2),
                Poco::Data::Keywords::use(Oldest);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::RevokeAllTokens(std::string & username) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string St2{"DELETE From Tokens WHERE Username=?"};
            Delete << ConvertParams(St2),
                Poco::Data::Keywords::use(username);
            Delete.execute();
            return true;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

}