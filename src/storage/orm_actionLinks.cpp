//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_actionLinks.h"

/*
"Id             varchar(36),"
"Action         bigint,"
"UserId         text,"
"template       text,"
"variables      text,"
"locale         varchar,"
"message        text,"
"sent           bigint,"
"created        bigint,"
"expires        bigint,"
"completed      bigint,"
"canceled       bigint,
 userAction     boolean"
*/

namespace OpenWifi {
    static ORM::FieldVec ActionLinksDB_Fields{
            ORM::Field{"id", 36, true},
            ORM::Field{"action", ORM::FieldType::FT_BIGINT},
            ORM::Field{"userId", ORM::FieldType::FT_TEXT},
            ORM::Field{"actionTemplate", ORM::FieldType::FT_TEXT},
            ORM::Field{"variables", ORM::FieldType::FT_TEXT},
            ORM::Field{"locale", ORM::FieldType::FT_TEXT},
            ORM::Field{"message", ORM::FieldType::FT_TEXT},
            ORM::Field{"sent", ORM::FieldType::FT_BIGINT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"expires", ORM::FieldType::FT_BIGINT},
            ORM::Field{"completed", ORM::FieldType::FT_BIGINT},
            ORM::Field{"canceled", ORM::FieldType::FT_BIGINT},
            ORM::Field{"userAction", ORM::FieldType::FT_BOOLEAN}
    };

    ActionLinkDB::ActionLinkDB(const std::string &Name, const std::string &ShortName, OpenWifi::DBType T,
                             Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, Name.c_str(), ActionLinksDB_Fields,{}, P, L, ShortName.c_str()) {
    }

    bool ActionLinkDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        std::vector<std::string> Statements{
                "alter table " + TableName_ + " add column userAction BOOLEAN default true;"
        };
        RunScript(Statements);
        to = 1;
        return true;

        return true;
    }
    bool ActionLinkDB::CreateAction( SecurityObjects::ActionLink & A) {
        return CreateRecord(A);
    }

    bool ActionLinkDB::GetActions(std::vector<SecurityObjects::ActionLink> &Links, uint64_t Max) {
        return GetRecords(0,Max,Links," sent=0 ");
    }

    bool ActionLinkDB::GetActionLink(std::string &ActionId, SecurityObjects::ActionLink &A) {
        return GetRecord("id",ActionId,A);
    }

    bool ActionLinkDB::SentAction(std::string &ActionId) {
        SecurityObjects::ActionLink A;
        if(GetRecord("id",ActionId,A)) {
            A.sent = OpenWifi::Now();
            return UpdateRecord("id",ActionId,A);
        }
        return false;
    }

    bool ActionLinkDB::DeleteAction(std::string &ActionId) {
        return DeleteRecord("id",ActionId);
    }

    bool ActionLinkDB::CompleteAction(std::string &ActionId) {
        SecurityObjects::ActionLink A;
        if(GetRecord("id",ActionId,A)) {
            A.completed = OpenWifi::Now();
            return UpdateRecord("id",ActionId,A);
        }
        return false;
    }

    bool ActionLinkDB::CancelAction(std::string &ActionId) {
        SecurityObjects::ActionLink A;
        if(GetRecord("id",ActionId,A)) {
            A.canceled = OpenWifi::Now();
            return UpdateRecord("id",ActionId,A);
        }
        return false;
    }

    void ActionLinkDB::CleanOldActionLinks() {
        uint64_t CutOff = OpenWifi::Now() - (30 * 24 * 60 * 60);
        std::string WhereClause{" Created <= " + std::to_string(CutOff) + " "};
        DeleteRecords(WhereClause);
    }

}

template<> void ORM::DB<OpenWifi::ActionLinkRecordTuple,
        OpenWifi::SecurityObjects::ActionLink>::Convert(const OpenWifi::ActionLinkRecordTuple &T, OpenWifi::SecurityObjects::ActionLink &U) {
    U.id = T.get<0>();
    U.action = T.get<1>();
    U.userId = T.get<2>();
    U.actionTemplate = T.get<3>();
    U.variables = OpenWifi::RESTAPI_utils::to_stringpair_array(T.get<4>());
    U.locale = T.get<5>();
    U.message = T.get<6>();
    U.sent = T.get<7>();
    U.created = T.get<8>();
    U.expires = T.get<9>();
    U.completed = T.get<10>();
    U.canceled = T.get<11>();
    U.userAction = T.get<12>();
}

template<> void ORM::DB<OpenWifi::ActionLinkRecordTuple,
        OpenWifi::SecurityObjects::ActionLink>::Convert(const OpenWifi::SecurityObjects::ActionLink &U, OpenWifi::ActionLinkRecordTuple &T) {
    T.set<0>(U.id);
    T.set<1>(U.action);
    T.set<2>(U.userId);
    T.set<3>(U.actionTemplate);
    T.set<4>(OpenWifi::RESTAPI_utils::to_string(U.variables));
    T.set<5>(U.locale);
    T.set<6>(U.message);
    T.set<7>(U.sent);
    T.set<8>(U.created);
    T.set<9>(U.expires);
    T.set<10>(U.completed);
    T.set<11>(U.canceled);
    T.set<12>(U.userAction);
}
