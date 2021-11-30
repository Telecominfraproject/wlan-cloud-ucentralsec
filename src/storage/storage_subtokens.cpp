//
// Created by stephane bourque on 2021-11-30.
//

#include "storage/storage_subtokens.h"
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

    bool Storage::AddSubToken(std::string &UserID, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);
            uint64_t Now = std::time(nullptr);
            uint64_t Z = 0;

            std::string St2{
                "INSERT INTO SubTokens (" + AllSubTokensFieldsForSelect + ") VALUES(" + AllSubTokensValuesForSelect + ")"};

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
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetSubToken(std::string &Token, SecurityObjects::UserInfoAndPolicy &UInfo, uint64_t &RevocationDate) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);
            RevocationDate = 0 ;
            std::string St2{"SELECT " + AllSubTokensFieldsForSelect + " From SubTokens WHERE Token=?"};
            Select << ConvertParams(St2),
            Poco::Data::Keywords::into(UInfo.webtoken.access_token_),
            Poco::Data::Keywords::into(UInfo.webtoken.refresh_token_),
            Poco::Data::Keywords::into(UInfo.webtoken.token_type_),
            Poco::Data::Keywords::into(UInfo.userinfo.Id),
            Poco::Data::Keywords::into(UInfo.webtoken.created_),
            Poco::Data::Keywords::into(UInfo.webtoken.expires_in_),
            Poco::Data::Keywords::into(UInfo.webtoken.idle_timeout_),
            Poco::Data::Keywords::into(RevocationDate),
            Poco::Data::Keywords::use(Token);
            Select.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::IsSubTokenRevoked(std::string &Token) {
        try {

            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            uint32_t RevocationDate = 0 ;

            std::string St2{"SELECT RevocationDate From SubTokens WHERE Token=?"};
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

    bool Storage::RevokeSubToken(std::string &Token) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint64_t Now = std::time(nullptr);

            // update users set lastLogin=? where id=?
            std::string St2{"UPDATE SubTokens  Set RevocationDate=? WHERE Token=?"};
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

    bool Storage::CleanExpiredSubTokens() {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);
            uint64_t Now = std::time(nullptr);

            std::string St2{"DELETE From SubTokens WHERE (Created+Expires) <= ?"};
            Delete << ConvertParams(St2),
            Poco::Data::Keywords::use(Now);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::RevokeAllSubTokens(std::string & UserId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string St2{"DELETE SubFrom Tokens WHERE Username=?"};
            Delete << ConvertParams(St2),
            Poco::Data::Keywords::use(UserId);
            Delete.execute();
            return true;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

}