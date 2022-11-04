//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_preferences.h"
#include "framework/RESTAPI_utils.h"

/*
        "Id          varchar(36) UNIQUE PRIMARY KEY,"
        "modified       bigint,"
        "data           text"};
*/

namespace OpenWifi {

    static ORM::FieldVec PreferencesDB_Fields{
            ORM::Field{"id", 36, true},
            ORM::Field{"modified", ORM::FieldType::FT_BIGINT},
            ORM::Field{"data", ORM::FieldType::FT_TEXT}
    };

    PreferencesDB::PreferencesDB( const std::string &TableName, const std::string &Shortname ,OpenWifi::DBType T,
                                 Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, TableName.c_str(), PreferencesDB_Fields, {}, P, L, Shortname.c_str()) {
    }

    bool PreferencesDB::GetPreferences(std::string &Id, SecurityObjects::Preferences &P) {
        return GetRecord("id", Id, P);
    }

    bool PreferencesDB::SetPreferences(SecurityObjects::Preferences &P) {
        return ReplaceRecord("id", P.id, P);
    }

    bool PreferencesDB::DeletePreferences([[maybe_unused]] const std::string &AdminId, std::string &Id) {
        return DeleteRecord("id",Id);
    }

}

template<> void ORM::DB<OpenWifi::PreferencesRecordTuple, OpenWifi::SecurityObjects::Preferences>::Convert(const OpenWifi::PreferencesRecordTuple &R, OpenWifi::SecurityObjects::Preferences &P ) {
    P.id = R.get<0>();
    P.modified = R.get<1>();
    P.data = OpenWifi::RESTAPI_utils::to_stringpair_array(R.get<2>());
}

template<> void ORM::DB<OpenWifi::PreferencesRecordTuple, OpenWifi::SecurityObjects::Preferences>::Convert(const OpenWifi::SecurityObjects::Preferences &P, OpenWifi::PreferencesRecordTuple &R ) {
    R.set<0>(P.id);
    R.set<1>(P.modified);
    R.set<2>(OpenWifi::RESTAPI_utils::to_string(P.data));
}
