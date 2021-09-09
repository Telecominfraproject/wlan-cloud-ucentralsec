
#include "StorageService.h"

namespace OpenWifi {

    bool Storage::AddRevocatedToken(std::string &Token) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);
            uint64_t Now = std::time(nullptr);

            std::string St2{
                "INSERT INTO Revocations (Token,Created) VALUES(?,?)"};
            Insert << ConvertParams(St2),
                Poco::Data::Keywords::use(Token),
                Poco::Data::Keywords::use(Now);
            Insert.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::IsTokenRevocated(std::string &Token) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);
            uint64_t Now = std::time(nullptr);

            std::string T;
            std::string St2{"SELECT Token From Revocations WHERE Token=?"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(T),
                Poco::Data::Keywords::use(Token);
            Select.execute();
            if(T.empty())
                return false;
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::CleanRevocatedTokens( uint64_t Oldest ) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);
            uint64_t Now = std::time(nullptr);

            std::string St2{"DELETE From Revocations WHERE Created <= ?"};
            Delete << ConvertParams(St2),
                Poco::Data::Keywords::use(Oldest);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

}