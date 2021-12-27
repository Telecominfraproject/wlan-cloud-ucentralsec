//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

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

    typedef Poco::Tuple<
            std::string,    // token
            std::string,    // RefreshToken;
            std::string,    // TokenType;
            std::string,    // UserId;
            uint64_t,       // Created = 0;
            uint64_t,       // Expires = 0;
            uint64_t,       // IdleTimeOut = 0;
            uint64_t        // RevocationDate = 0;
    > TokenRecordTuple;

    typedef std::vector <TokenRecordTuple> TokenRecordTupleList;

    class BaseTokenDB : public ORM::DB<TokenRecordTuple, SecurityObjects::Token> {
    public:
        BaseTokenDB( const std::string &name, const std::string &shortname, OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);


        bool AddToken(std::string &UserId, std::string &Token, std::string &RefreshToken, std::string & TokenType, uint64_t Expires, uint64_t TimeOut);
        bool RevokeToken( std::string & Token );
        bool IsTokenRevoked( std::string & Token );
        bool CleanExpiredTokens();
        bool RevokeAllTokens( std::string & UserName );
        bool GetToken(std::string &Token, SecurityObjects::WebToken &WT, std::string & UserId, uint64_t &RevocationDate);

    private:
    };

}
