//
// Created by stephane bourque on 2021-07-15.
//

#pragma once

namespace OpenWifi {

    static const std::string AllAvatarFieldsForCreation_sqlite{
        "Id			    VARCHAR(36) PRIMARY KEY, "
        "Type			VARCHAR, "
        "Created 		BIGINT, "
        "Name           VARCHAR, "
        "Avatar     	BLOB"
    };

    static const std::string AllAvatarFieldsForCreation_mysql{
        "Id			    VARCHAR(36) PRIMARY KEY, "
        "Type			VARCHAR, "
        "Created 		BIGINT, "
        "Name           VARCHAR, "
        "Avatar     	LONGBLOB"
    };

    static const std::string AllAvatarFieldsForCreation_pgsql{
        "Id			    VARCHAR(36) PRIMARY KEY, "
        "Type			VARCHAR, "
        "Created 		BIGINT, "
        "Name           VARCHAR, "
        "Avatar     	BYTEA"
    };

    static const std::string AllAvatarFieldsForSelect{ " Id,Type,Created,Name,Avatar " };
    static const std::string AllAvatarValuesForSelect{ "?,?,?,?,?" };



}
