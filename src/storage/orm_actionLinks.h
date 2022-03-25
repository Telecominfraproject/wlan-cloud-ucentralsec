//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    typedef Poco::Tuple <
            std::string,    // id
            uint64_t,       // action
            std::string,    // userId
            std::string,    // actionTemplate
            std::string,    // variables
            std::string,    // locale
            std::string,    // message
            uint64_t,       // sent
            uint64_t,       // created
            uint64_t,       // expires
            uint64_t,       // completed
            uint64_t,       // canceled
            bool            // userAction
    > ActionLinkRecordTuple;
    typedef std::vector <ActionLinkRecordTuple> ActionLinkRecordTupleList;

    class ActionLinkDB : public ORM::DB<ActionLinkRecordTuple, SecurityObjects::ActionLink> {
    public:
        ActionLinkDB( const std::string &name, const std::string &shortname, OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        virtual ~ActionLinkDB() {}
        bool CreateAction( SecurityObjects::ActionLink & A);
        bool DeleteAction(std::string &ActionId);
        bool CompleteAction(std::string &ActionId);
        bool CancelAction(std::string &ActionId);
        bool SentAction(std::string &ActionId);
        bool GetActionLink(std::string &ActionId, SecurityObjects::ActionLink &A);
        bool GetActions(std::vector<SecurityObjects::ActionLink> &Links, uint64_t Max=200);
        void CleanOldActionLinks();

        inline uint32_t Version() override { return 1;}
        bool Upgrade(uint32_t from, uint32_t &to) override;

    private:

    };
}

