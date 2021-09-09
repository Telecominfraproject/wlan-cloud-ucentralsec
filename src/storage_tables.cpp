//
// Created by stephane bourque on 2021-06-13.
//

#include "StorageService.h"
#include "Utils.h"
#include "storage_users.h"
#include "storage_avatar.h"

namespace OpenWifi {

    int Storage::Create_Tables() {
        Create_UserTable();
        Create_AvatarTable();
        Create_RevocationTable();
        return 0;
    }

    int Storage::Create_UserTable() {
        Poco::Data::Session Sess = Pool_->get();

        try {
            if (dbType_ == mysql) {
                Sess << "CREATE TABLE IF NOT EXISTS Users (" +
                        AllUsersFieldsForCreation +
                        " ,INDEX emailindex (email ASC)"
                        " ,INDEX nameindex (name ASC))",
                        Poco::Data::Keywords::now;
            } else {
                Sess << "CREATE TABLE IF NOT EXISTS Users (" +
                        AllUsersFieldsForCreation +
                        ")",
                        Poco::Data::Keywords::now;
                Sess << "CREATE INDEX IF NOT EXISTS emailindex ON Users (email ASC)", Poco::Data::Keywords::now;
                Sess << "CREATE INDEX IF NOT EXISTS nameindex ON Users (name ASC)", Poco::Data::Keywords::now;
            }
            return 0;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }

    int Storage::Create_AvatarTable() {
            try {
                Poco::Data::Session Sess = Pool_->get();

                if(dbType_==sqlite) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars ("
                            "Id			    VARCHAR(36) PRIMARY KEY, "
                            "Type			VARCHAR, "
                            "Created 		BIGINT, "
                            "Name           VARCHAR, "
                            "Avatar     	BLOB"
                            ") ", Poco::Data::Keywords::now;
                } else if(dbType_==mysql) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars ("
                            "Id			    VARCHAR(36) PRIMARY KEY, "
                            "Type			VARCHAR, "
                            "Created 		BIGINT, "
                            "Name           VARCHAR, "
                            "Avatar     	LONGBLOB"
                            ") ", Poco::Data::Keywords::now;
                } else if(dbType_==pgsql) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars ("
                            "Id			    VARCHAR(36) PRIMARY KEY, "
                            "Type			VARCHAR, "
                            "Created 		BIGINT, "
                            "Name           VARCHAR, "
                            "Avatar     	BYTEA"
                            ") ", Poco::Data::Keywords::now;
                }
                return 0;
            } catch(const Poco::Exception &E) {
                Logger_.log(E);
            }
            return 0;
        }

    int Storage::Create_RevocationTable() {
        try {
            Poco::Data::Session Sess = Pool_->get();
            if(dbType_==sqlite) {
                Sess << "CREATE TABLE IF NOT EXISTS Revocations ("
                        "Token			TEXT PRIMARY KEY, "
                        "Created 		BIGINT "
                        ") ", Poco::Data::Keywords::now;
            } else if(dbType_==mysql) {
                Sess << "CREATE TABLE IF NOT EXISTS Revocations ("
                        "Token			TEXT PRIMARY KEY, "
                        "Created 		BIGINT "
                        ") ", Poco::Data::Keywords::now;
            } else if(dbType_==pgsql) {
                Sess << "CREATE TABLE IF NOT EXISTS Revocations ("
                        "Token			TEXT PRIMARY KEY, "
                        "Created 		BIGINT "
                        ") ", Poco::Data::Keywords::now;
            }
            return 0;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 0;
    }
}
