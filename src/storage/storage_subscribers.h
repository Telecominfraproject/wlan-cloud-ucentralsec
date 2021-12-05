//
// Created by stephane bourque on 2021-11-30.
//

#pragma once

#include <string>
#include <vector>
#include "Poco/Tuple.h"

namespace OpenWifi {
    static const std::string AllSubUsersFieldsForCreation{
        " Id             varchar(36) UNIQUE PRIMARY KEY,"
        "name               varchar,"
        "description        varchar,"
        "avatar             varchar,"
        "email              varchar,"
        "validated          boolean,"
        "validationEmail    varchar,"
        "validationDate     bigint,"
        "creationDate       bigint,"
        "validationURI      varchar,"
        "changePassword     boolean,"
        "lastLogin          bigint,"
        "currentLoginURI    varchar,"
        "lastPasswordChange bigint,"
        "lastEmailCheck     bigint,"
        "waitingForEmailCheck   boolean,"
        "locale             varchar,"
        "notes              text,"
        "location           varchar,"
        "owner              varchar,"
        "suspended          boolean,"
        "blackListed        boolean,"
        "userRole           varchar,"
        "userTypeProprietaryInfo    text,"
        "securityPolicy     text,"
        "securityPolicyChange   bigint,"
        "currentPassword    varchar,"
        "lastPasswords      varchar,"
        "oauthType          varchar,"
        "oauthUserInfo      text"};

    static const std::string AllSubUsersFieldsForSelect{
        "Id,"
        "name,"
        "description,"
        "avatar,"
        "email,"
        "validated,"
        "validationEmail,"
        "validationDate,"
        "creationDate,"
        "validationURI,"
        "changePassword,"
        "lastLogin,"
        "currentLoginURI,"
        "lastPasswordChange,"
        "lastEmailCheck,"
        "waitingForEmailCheck,"
        "locale,"
        "notes,"
        "location,"
        "owner,"
        "suspended,"
        "blackListed,"
        "userRole,"
        "userTypeProprietaryInfo,"
        "securityPolicy,"
        "securityPolicyChange,"
        "currentPassword,"
        "lastPasswords,"
        "oauthType,"
        "oauthUserInfo"};

    static const std::string AllSubUsersFieldsForUpdate{
        " Id=?, "
        "name=?, "
        "description=?, "
        "avatar=?, "
        "email=?, "
        "validated=?, "
        "validationEmail=?, "
        "validationDate=?, "
        "creationDate=?, "
        "validationURI=?, "
        "changePassword=?, "
        "lastLogin=?, "
        "currentLoginURI=?, "
        "lastPasswordChange=?, "
        "lastEmailCheck=?, "
        "waitingForEmailCheck=?, "
        "locale=?, "
        "notes=?, "
        "location=?, "
        "owner=?, "
        "suspended=?, "
        "blackListed=?, "
        "userRole=?, "
        "userTypeProprietaryInfo=?, "
        "securityPolicy=?, "
        "securityPolicyChange=?, "
        "currentPassword=?, "
        "lastPasswords=?, "
        "oauthType=?, "
        "oauthUserInfo=? "};

    typedef Poco::Tuple <
        std::string,    // Id = 0;
        std::string,    // name;
        std::string,    // description;
        std::string,    // avatar;
        std::string,    // email;
        bool,       // bool validated = false;
        std::string,    // validationEmail;
        uint64_t,       // validationDate = 0;
        uint64_t,       // creationDate = 0;
        std::string,    // validationURI;
        bool,       // bool changePassword = true;
        uint64_t,       // lastLogin = 0;
        std::string,    // currentLoginURI;
        uint64_t,       // lastPasswordChange = 0;
        uint64_t,       // lastEmailCheck = 0;
        bool,      // bool waitingForEmailCheck = false;
        std::string,    // locale;
        std::string,    // notes;
        std::string,    // location;
        std::string,    // owner;
        bool,       // bool suspended = false;
        bool,       // bool blackListed = false;
        std::string,    // userRole;
        std::string,    // userTypeProprietaryInfo;
        std::string,    // securityPolicy;
        uint64_t,       // securityPolicyChange;
        std::string,    // currentPassword;
        std::string,    // lastPasswords;
        std::string,    // oauthType;
        std::string    // oauthUserInfo;
    > UserInfoRecord;

    typedef std::vector <UserInfoRecord> UserInfoRecordList;
}
