//
// Created by stephane bourque on 2021-11-16.
//

#pragma once

#include <string>
#include <vector>
#include "Poco/Tuple.h"

namespace OpenWifi {

    static const std::string AllPreferencesFieldsForCreation{
        "Id          varchar(36) UNIQUE PRIMARY KEY,"
        "modified       bigint,"
        "data           text"};

    static const std::string AllPreferencesFieldsForSelect{
        "Id, "
        "modified, "
        "data "};

    static const std::string AllPreferencesFieldsForUpdate{
        " Id=?, "
        "modified=?, "
        "data=? "};

    typedef Poco::Tuple <
        std::string,        // id
        uint64_t,           // modified
        std::string         // data
    >  PreferencesRecord;

    typedef std::vector <PreferencesRecord> PreferencesRecordList;

}
