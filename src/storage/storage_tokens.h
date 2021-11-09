//
// Created by stephane bourque on 2021-11-08.
//

#ifndef OWSEC_STORAGE_TOKENS_H
#define OWSEC_STORAGE_TOKENS_H

#include <string>
#include <vector>
#include "Poco/Tuple.h"

namespace OpenWifi {

    static std::string AllTokensFieldsForCreation{                        "Token			    TEXT PRIMARY KEY, "
                                                                          "RefreshToken       TEXT, "
                                                                          "TokenType          TEXT, "
                                                                          "UserName           TEXT, "
                                                                          "Created 		    BIGINT, "
                                                                          "Expires 		    BIGINT, "
                                                                          "IdleTimeOut        BIGINT, "
                                                                          "RevocationDate 	BIGINT "
    };
    static std::string AllTokensFieldsForSelect {"Token, RefreshToken, TokenType, Username, Created, Expires, IdleTimeOut, RevocationDate"};
    static std::string AllTokensValuesForSelect{"?,?,?,?,?,?,?,?"};



}

#endif //OWSEC_STORAGE_TOKENS_H
