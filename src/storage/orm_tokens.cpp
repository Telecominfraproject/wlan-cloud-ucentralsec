//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_tokens.h"

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

namespace OpenWifi {
    static ORM::FieldVec BaseTokenDB_Fields{
            ORM::Field{"token", ORM::FT_TEXT, 0, true},
            ORM::Field{"refreshToken", ORM::FieldType::FT_TEXT},
            ORM::Field{"tokenType", ORM::FieldType::FT_TEXT},
            ORM::Field{"userName", ORM::FieldType::FT_TEXT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"expires", ORM::FieldType::FT_BIGINT},
            ORM::Field{"idleTimeOut", ORM::FieldType::FT_BIGINT},
            ORM::Field{"revocationDate", ORM::FieldType::FT_BIGINT}
    };

    static ORM::IndexVec MakeIndices(const std::string &shortname) {
        return ORM::IndexVec{
                {std::string(shortname + "_user_id_index"),
                        ORM::IndexEntryVec{
                                {std::string("userName"),
                                 ORM::Indextype::ASC}}},
                {std::string(shortname + "_refresh_index"),
                        ORM::IndexEntryVec{
                                {std::string("refreshToken"),
                                 ORM::Indextype::ASC}}}
        };
    }

    BaseTokenDB::BaseTokenDB(const std::string &Name, const std::string &ShortName, OpenWifi::DBType T,
                             Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, Name.c_str(), BaseTokenDB_Fields, MakeIndices(ShortName), P, L, ShortName.c_str()) {
    }

    bool BaseTokenDB::AddToken(std::string &UserID, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut) {
        SecurityObjects::Token  T{.token=Token, .refreshToken=RefreshToken, .tokenType="Bearer", .userName=UserID,
                                  .created=(uint64_t) std::time(nullptr), .expires=Expires, .idleTimeout=TimeOut,.revocationDate=0};
        return CreateRecord(T);
    }

    bool BaseTokenDB::GetToken(std::string &Token, SecurityObjects::WebToken &WT, std::string & UserId, uint64_t &RevocationDate) {
        SecurityObjects::Token  T;

        if(GetRecord("token", Token, T)) {
            WT.access_token_ = T.token;
            WT.refresh_token_ = T.refreshToken;
            WT.token_type_ = T.tokenType;
            WT.username_ = T.userName;
            WT.created_ = T.created;
            WT.expires_in_ = T.expires;
            WT.idle_timeout_ = T.idleTimeout;
            RevocationDate = T.revocationDate;
            UserId = T.userName;
            return true;
        }
        return false;
    }

    bool BaseTokenDB::IsTokenRevoked(std::string &Token) {
        SecurityObjects::Token  T;

        if(GetRecord("token",Token,T)) {
            return T.revocationDate!=0;
        }
        return false;
    }

    bool BaseTokenDB::RevokeToken(std::string &Token) {
        SecurityObjects::Token  T;

        if(GetRecord("token", Token, T)) {
            T.revocationDate = std::time(nullptr);
            return UpdateRecord("token", Token, T);
        }
        return false;
    }

    bool BaseTokenDB::CleanExpiredTokens() {
        std::string WhereClause{" (created + expires) <= " + std::to_string(std::time(nullptr))};
        DeleteRecords( WhereClause );
        return true;
    }

    bool BaseTokenDB::RevokeAllTokens(std::string & UserId) {
        std::string WhereClause{" userName='" + UserId + "' "};
        DeleteRecords( WhereClause );
        return true;
    }

}

template<> void ORM::DB<OpenWifi::TokenRecordTuple,
        OpenWifi::SecurityObjects::Token>::Convert(OpenWifi::TokenRecordTuple &T,
                                                      OpenWifi::SecurityObjects::Token &U) {
    U.token = T.get<0>();
    U.refreshToken = T.get<1>();
    U.tokenType = T.get<2>();
    U.userName = T.get<3>();
    U.created = T.get<4>();
    U.expires = T.get<5>();
    U.idleTimeout = T.get<6>();
    U.revocationDate = T.get<7>();
}

template<> void ORM::DB< OpenWifi::TokenRecordTuple,
        OpenWifi::SecurityObjects::Token>::Convert(OpenWifi::SecurityObjects::Token &U,
                                                         OpenWifi::TokenRecordTuple &T) {
    T.set<0>(U.token);
    T.set<1>(U.refreshToken);
    T.set<2>(U.tokenType);
    T.set<3>(U.userName);
    T.set<4>(U.created);
    T.set<5>(U.expires);
    T.set<6>(U.idleTimeout);
    T.set<7>(U.revocationDate);
}
