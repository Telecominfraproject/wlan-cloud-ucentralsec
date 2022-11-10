//
// Created by stephane bourque on 2022-11-04.
//

#include "orm_apikeys.h"
#include "framework/RESTAPI_utils.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/orm.h"
#include "AuthService.h"
#include "StorageService.h"
#include "fmt/format.h"

namespace OpenWifi {
    static ORM::FieldVec ApiKeyDB_Fields{
            ORM::Field{"id", 36, true},
            ORM::Field{"userUuid", ORM::FieldType::FT_TEXT},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"description", ORM::FieldType::FT_TEXT},
            ORM::Field{"apiKey", ORM::FieldType::FT_TEXT},
            ORM::Field{"salt", ORM::FieldType::FT_TEXT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"expiresOn", ORM::FieldType::FT_BIGINT},
            ORM::Field{"rights", ORM::FieldType::FT_TEXT},
            ORM::Field{"lastUse", ORM::FieldType::FT_BIGINT}
    };

    static ORM::IndexVec MakeIndices(const std::string & shortname) {
        return ORM::IndexVec{
                {std::string(shortname + "_username_index"),
                    ORM::IndexEntryVec{
                         {
                                 std::string("userUuid"),
                                 ORM::Indextype::ASC }}},
                {std::string(shortname + "_apikey_index"),
                        ORM::IndexEntryVec{
                                {
                                        std::string("apiKey"),
                                        ORM::Indextype::ASC}
                        }
                }
        };
    };

    ApiKeyDB::ApiKeyDB( const std::string &TableName, const std::string &Shortname ,OpenWifi::DBType T,
                        Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, TableName.c_str(), ApiKeyDB_Fields, MakeIndices(Shortname), P, L, Shortname.c_str()) {
    }

    bool ApiKeyDB::RemoveAllApiKeys(const std::string & user_uuid) {
        SecurityObjects::ApiKeyEntryList    Keys;
        if(StorageService()->ApiKeyDB().GetRecords(0,500,Keys.apiKeys,fmt::format(" userUuid='{} ", user_uuid))) {
            for(const auto &key:Keys.apiKeys) {
                AuthService()->RemoveTokenSystemWide(key.apiKey);
            }
        }
        return true;
    }

    bool ApiKeyDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        to = Version();
        std::vector<std::string>    Script{
        };

        for(const auto &i:Script) {
            try {
                auto Session = Pool_.get();
                Session << i , Poco::Data::Keywords::now;
            } catch (...) {

            }
        }
        return true;
    }

    bool ApiKeyDB::RemoveExpiredAPIKeys() {
        std::string WhereClause{" expiresOn <= " + std::to_string(OpenWifi::Now())};
        DeleteRecords( WhereClause );
        return true;
    }

} // OpenWifi

template<> void ORM::DB<OpenWifi::ApiKeyRecordTuple, OpenWifi::SecurityObjects::ApiKeyEntry>::Convert(const OpenWifi::ApiKeyRecordTuple &In, OpenWifi::SecurityObjects::ApiKeyEntry &Out) {
    Out.id = In.get<0>();
    Out.userUuid = In.get<1>();
    Out.name = In.get<2>();
    Out.description = In.get<3>();
    Out.apiKey = In.get<4>();
    Out.salt = In.get<5>();
    Out.created = In.get<6>();
    Out.expiresOn = In.get<7>();
    Out.rights.acls = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::ApiKeyAccessRight>(In.get<8>());
    Out.lastUse = In.get<9>();
}

template<> void ORM::DB<OpenWifi::ApiKeyRecordTuple, OpenWifi::SecurityObjects::ApiKeyEntry>::Convert(const OpenWifi::SecurityObjects::ApiKeyEntry &In, OpenWifi::ApiKeyRecordTuple &Out) {
    Out.set<0>(In.id);
    Out.set<1>(In.userUuid);
    Out.set<2>(In.name);
    Out.set<3>(In.description);
    Out.set<4>(In.apiKey);
    Out.set<5>(In.salt);
    Out.set<6>(In.created);
    Out.set<7>(In.expiresOn);
    Out.set<8>(OpenWifi::RESTAPI_utils::to_string(In.rights.acls));
    Out.set<9>(In.lastUse);
}
