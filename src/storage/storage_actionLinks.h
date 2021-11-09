//
// Created by stephane bourque on 2021-11-08.
//

#ifndef OWSEC_STORAGE_ACTIONLINKS_H
#define OWSEC_STORAGE_ACTIONLINKS_H

#include <string>
#include <vector>
#include "Poco/Tuple.h"

namespace OpenWifi {
    static const std::string AllActionLinksFieldsForCreation{
        "Id          varchar(36),"
        "Action         text,"
        "UserId         text,"
        "template       text,"
        "variables      text,"
        "locale         varchar,"
        "message        text,"
        "sent           bigint,"
        "created        bigint,"
        "expires        bigint,"
        "completed      bigint,"
        "canceled       bigint"
    };

    static const std::string AllActionLinksFieldsForSelect {
        "Id, "
        "Action,"
        "UserId,"
        "template,"
        "variables,"
        "locale,"
        "message,"
        "sent,"
        "created,"
        "expires,"
        "completed,"
        "canceled"
    };

    static const std::string AllActionLinksValuesForSelect{ "?,?,?,?,?,?,?,?,?,?,?,?" };

    static const std::string AllActionLinksFieldsForUpdate {
        "Id=?, "
        "Action=?,"
        "UserId=?,"
        "template=?,"
        "variables=?,"
        "locale=?,"
        "message=?,"
        "sent=?,"
        "created=?,"
        "expires=?,"
        "completed=?,"
        "canceled=?"
    };

    typedef Poco::Tuple <
                std::string,    // id
                std::string,    // action
                std::string,    // userId
                std::string,    // actionTemplate
                std::string,    // variables
                std::string,    // locale
                std::string,    // message
                uint64_t,       // sent
                uint64_t,       // created
                uint64_t,       // expires
                uint64_t,       // completed
                uint64_t        // canceled
            > ActionLinkRecord;
    typedef std::vector <ActionLinkRecord> ActionLinkRecordList;

}


#endif //OWSEC_STORAGE_ACTIONLINKS_H
