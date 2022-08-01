//
// Created by stephane bourque on 2022-01-05.
//

#include "orm_logins.h"

namespace OpenWifi {

    static ORM::FieldVec LoginDB_Fields{
            ORM::Field{"session", 64, true},
            ORM::Field{"userId", ORM::FieldType::FT_TEXT},
            ORM::Field{"email", ORM::FieldType::FT_TEXT},
            ORM::Field{"login", ORM::FieldType::FT_BIGINT},
            ORM::Field{"logout", ORM::FieldType::FT_BIGINT}
    };

    static ORM::IndexVec MakeIndices(const std::string & shortname) {
        return ORM::IndexVec{
                {std::string(shortname + "_userid_index"),
                        ORM::IndexEntryVec{
                                {std::string("userId"),
                                 ORM::Indextype::ASC}}}
        };
    }

    LoginDB::LoginDB( const std::string &TableName, const std::string &Shortname ,OpenWifi::DBType T,
                                  Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, TableName.c_str(), LoginDB_Fields, MakeIndices(Shortname), P, L, Shortname.c_str()) {
    }

    static std::string MakeSessionId(const std::string & token) {
        return Utils::ComputeHash(token);
    }

    void LoginDB::AddLogin( const std::string & userId, const std::string & email, const std::string &token) {
        SecurityObjects::LoginRecordInfo    R;

        R.sessionId = MakeSessionId(token);
        R.userId = userId;
        R.email = email;
        R.login = OpenWifi::Now();
        R.logout = 0;
        CreateRecord(R);
    }

    void LoginDB::AddLogout(const std::string &token) {
        auto Session = MakeSessionId(token);
        SecurityObjects::LoginRecordInfo    R;

        if(GetRecord("session", Session, R)) {
            R.logout = OpenWifi::Now();
            UpdateRecord("session", Session, R);
        }
    }
}

template<> void ORM::DB<OpenWifi::LoginInfoRecordTuple, OpenWifi::SecurityObjects::LoginRecordInfo>::Convert(const OpenWifi::LoginInfoRecordTuple &R, OpenWifi::SecurityObjects::LoginRecordInfo &P ) {
    P.sessionId = R.get<0>();
    P.userId = R.get<1>();
    P.email = R.get<2>();
    P.login = R.get<3>();
    P.logout = R.get<4>();
}

template<> void ORM::DB<OpenWifi::LoginInfoRecordTuple, OpenWifi::SecurityObjects::LoginRecordInfo>::Convert(const OpenWifi::SecurityObjects::LoginRecordInfo &P, OpenWifi::LoginInfoRecordTuple &R ) {
    R.set<0>(P.sessionId);
    R.set<1>(P.userId);
    R.set<2>(P.email);
    R.set<3>(P.login);
    R.set<4>(P.logout);
}
