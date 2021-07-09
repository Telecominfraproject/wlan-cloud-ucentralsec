//
// Created by stephane bourque on 2021-06-13.
//

#include "StorageService.h"
#include "Utils.h"

namespace uCentral {

    int Storage::Create_Tables() {
        Create_UserTable();
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

    int Storage::Create_APIKeyTable() {

        return 0;
    }
}
