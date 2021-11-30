//
// Created by stephane bourque on 2021-06-13.
//

#include "StorageService.h"
#include "storage_users.h"
#include "storage_avatar.h"
#include "storage_actionLinks.h"
#include "storage_tokens.h"
#include "storage_preferences.h"

namespace OpenWifi {

    int Storage::Create_Tables() {
        Create_UserTable();
        Create_AvatarTable();
        Create_TokensTable();
        Create_ActionLinkTable();
        Create_Preferences();
        Create_SubTokensTable();
        Create_SubscriberTable();
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

    int Storage::Create_SubscriberTable() {
        Poco::Data::Session Sess = Pool_->get();

        try {
            if (dbType_ == mysql) {
                Sess << "CREATE TABLE IF NOT EXISTS Subscribers (" +
                AllUsersFieldsForCreation +
                " ,INDEX emailindex (email ASC)"
                " ,INDEX nameindex (name ASC))",
                Poco::Data::Keywords::now;
            } else {
                Sess << "CREATE TABLE IF NOT EXISTS Subscribers (" +
                AllUsersFieldsForCreation +
                ")",
                Poco::Data::Keywords::now;
                Sess << "CREATE INDEX IF NOT EXISTS emailindex ON Subscribers (email ASC)", Poco::Data::Keywords::now;
                Sess << "CREATE INDEX IF NOT EXISTS nameindex ON Subscribers (name ASC)", Poco::Data::Keywords::now;
            }
            return 0;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }

    int Storage::Create_ActionLinkTable() {
        try {
            Poco::Data::Session Sess = Pool_->get();

            Sess << "CREATE TABLE IF NOT EXISTS ActionLinks ( "
                    + AllActionLinksFieldsForCreation + " ) ",
            Poco::Data::Keywords::now;
            return 0;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }

    int Storage::Create_AvatarTable() {
            try {
                Poco::Data::Session Sess = Pool_->get();

                if(dbType_==sqlite) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars (" + AllAvatarFieldsForCreation_sqlite +
                            ") ", Poco::Data::Keywords::now;
                } else if(dbType_==mysql) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars (" + AllAvatarFieldsForCreation_mysql +
                            ") ", Poco::Data::Keywords::now;
                } else if(dbType_==pgsql) {
                    Sess << "CREATE TABLE IF NOT EXISTS Avatars (" + AllAvatarFieldsForCreation_pgsql +
                            ") ", Poco::Data::Keywords::now;
                }
                return 0;
            } catch(const Poco::Exception &E) {
                Logger_.log(E);
            }
            return 1;
        }

    int Storage::Create_TokensTable() {
        try {
            Poco::Data::Session Sess = Pool_->get();
                Sess << "CREATE TABLE IF NOT EXISTS Tokens (" +
                            AllTokensFieldsForCreation +
                        ") ", Poco::Data::Keywords::now;
            return 0;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }

    int Storage::Create_SubTokensTable() {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Sess << "CREATE TABLE IF NOT EXISTS SubTokens (" +
            AllTokensFieldsForCreation +
            ") ", Poco::Data::Keywords::now;
            return 0;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }

    int Storage::Create_Preferences() {
        try {
            Poco::Data::Session Sess = Pool_->get();

            Sess << "CREATE TABLE IF NOT EXISTS Preferences (" +
                AllPreferencesFieldsForCreation +
            ") ", Poco::Data::Keywords::now;
            return 0;
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        return 1;
    }
}
