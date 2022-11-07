//
// Created by stephane bourque on 2022-11-04.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    typedef Poco::Tuple<
            std::string,    //  id
            std::string,    //  userUuid
            std::string,    //  name
            std::string,    //  description
            std::string,    //  apiKey
            std::string,    //  salt
            uint64_t,       //  created = 0;
            uint64_t,       //  expiresOn = 0;
            std::string,    //  rights
            std::uint64_t   //  lastUse
    > ApiKeyRecordTuple;
    typedef std::vector <ApiKeyRecordTuple> ApiKeyRecordTupleTupleList;

    class ApiKeyDB : public ORM::DB<ApiKeyRecordTuple, SecurityObjects::ApiKeyEntry> {
    public:
        ApiKeyDB( const std::string &name, const std::string &shortname, OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        virtual ~ApiKeyDB() {}
        inline uint32_t Version() override {
            return 1;
        }

        bool Upgrade(uint32_t from, uint32_t &to) override;
        bool RemoveAllApiKeys(const std::string & user_uuid);

    };

}
