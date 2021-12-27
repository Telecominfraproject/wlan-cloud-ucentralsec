
#include "StorageService.h"
#include "storage/storage_tokens.h"

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

    bool StorageService::AddToken(std::string &UserID, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);
            uint64_t Now = std::time(nullptr);
            uint64_t Z = 0;

            std::string St2{
                "INSERT INTO Tokens (" + AllTokensFieldsForSelect + ") VALUES(" + AllTokensValuesForSelect + ")"};

            Insert << ConvertParams(St2),
                Poco::Data::Keywords::use(Token),
                Poco::Data::Keywords::use(RefreshToken),
                Poco::Data::Keywords::use(TokenType),
                Poco::Data::Keywords::use(UserID),
                Poco::Data::Keywords::use(Now),
                Poco::Data::Keywords::use(Expires),
                Poco::Data::Keywords::use(TimeOut),
                Poco::Data::Keywords::use(Z);
            Insert.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool StorageService::GetToken(std::string &Token, SecurityObjects::WebToken &WT, std::string & UserId, uint64_t &RevocationDate) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);
            RevocationDate = 0 ;
            std::string St2{"SELECT " + AllTokensFieldsForSelect + " From Tokens WHERE Token=?"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(WT.access_token_),
                Poco::Data::Keywords::into(WT.refresh_token_),
                Poco::Data::Keywords::into(WT.token_type_),
                Poco::Data::Keywords::into(UserId),
                Poco::Data::Keywords::into(WT.created_),
                Poco::Data::Keywords::into(WT.expires_in_),
                Poco::Data::Keywords::into(WT.idle_timeout_),
                Poco::Data::Keywords::into(RevocationDate),
                Poco::Data::Keywords::use(Token);
            Select.execute();

            if(Select.rowsExtracted()!=1)
                return false;
            return true;

        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool StorageService::IsTokenRevoked(std::string &Token) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            uint32_t RevocationDate = 0 ;

            std::string St2{"SELECT RevocationDate From Tokens WHERE Token=?"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(RevocationDate),
                Poco::Data::Keywords::use(Token);
            Select.execute();

            if(Select.columnsExtracted()==0)
                return false;
            return RevocationDate>0 ;

        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool StorageService::RevokeToken(std::string &Token) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint64_t Now = std::time(nullptr);

            // update users set lastLogin=? where id=?
            std::string St2{"UPDATE Tokens  Set RevocationDate=? WHERE Token=?"};
            Update << ConvertParams(St2),
                Poco::Data::Keywords::use(Now),
                Poco::Data::Keywords::use(Token);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool StorageService::CleanExpiredTokens() {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);
            uint64_t Now = std::time(nullptr);

            std::string St2{"DELETE From Tokens WHERE (Created+Expires) <= ?"};
            Delete << ConvertParams(St2),
                Poco::Data::Keywords::use(Now);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool StorageService::RevokeAllTokens(std::string & UserId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string St2{"DELETE From Tokens WHERE Username=?"};
            Delete << ConvertParams(St2),
            Poco::Data::Keywords::use(UserId);
            Delete.execute();
            return true;
        } catch(const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

}