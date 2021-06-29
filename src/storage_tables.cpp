//
// Created by stephane bourque on 2021-06-13.
//

#include "StorageService.h"

namespace uCentral {

    int Storage::Create_Tables() {
        return 0;
    }

    int Storage::Create_UserTable() {
        Poco::Data::Session Sess = Pool_->get();

        try {

            if (dbType_ == mysql) {
                Sess << "CREATE TABLE IF NOT EXISTS Users ("
                        "Id     Id unique primary key, "
                        "name   varchar, "
                        "description varchar, "
                        "avatar varchar, "
                        "email  varchar, "
                        "validated int, "
                        "validationEmail varchar, "
                        "validationDate bigint, "
                        "creationDate bigint, "
                        "validationURI text, "
                        "changePassword int, "
                        "lastLogin bigint, "
                        "currentLoginURI varchar, "
                        "lastPasswordChange bigint, "
                        "lastEmailCheck bigint, "
                        "currentPassword varchar, "
                        "lastPasswords varchar,"
                        "waitingForEmailCheck int, "
                        "locale varchar, "
                        "notes text, "
                        "location text, "
                        "owner varchar, "
                        "suspended int, "
                        "blackListed int, "
                        "userRole varchar, "
                        "securityPolicy text, "
                        "securityPolicyChange bigint, "
                        "userTypeProprietaryInfo text"
                        " ,INDEX emailindex (email ASC)"
                        " ,INDEX nameindex (name ASC))",
                        Poco::Data::Keywords::now;
            } else {
                Sess << "CREATE TABLE IF NOT EXISTS Users ("
                        "Id     Id unique primary key, "
                        "name   varchar, "
                        "description varchar, "
                        "avatar varchar, "
                        "email  varchar, "
                        "validated int, "
                        "validationEmail varchar, "
                        "validationDate bigint, "
                        "creationDate bigint, "
                        "validationURI text, "
                        "changePassword int, "
                        "lastLogin bigint, "
                        "currentLoginURI varchar, "
                        "lastPasswordChange bigint, "
                        "lastEmailCheck bigint, "
                        "currentPassword varchar, "
                        "lastPasswords varchar,"
                        "waitingForEmailCheck int, "
                        "locale varchar, "
                        "notes text, "
                        "location text, "
                        "owner varchar, "
                        "suspended int, "
                        "blackListed int, "
                        "userRole varchar, "
                        "securityPolicy text, "
                        "securityPolicyChange bigint, "
                        "userTypeProprietaryInfo text"
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

    int Storage::Create_APIKeyTable() {

        return 0;
    }
}
