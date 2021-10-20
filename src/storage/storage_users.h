//
// Created by stephane bourque on 2021-07-15.
//

#ifndef UCENTRALSEC_STORAGE_USERS_H
#define UCENTRALSEC_STORAGE_USERS_H

namespace OpenWifi {
    static const std::string AllUsersFieldsForCreation{
        " Id             varchar(36) UNIQUE PRIMARY KEY,"
            "name           varchar,"
            "description    varchar,"
            "avatar         varchar,"
            "email          varchar,"
            "validated      int,"
            "validationEmail    varchar,"
            "validationDate bigint,"
            "creationDate   bigint,"
            "validationURI  varchar,"
            "changePassword int,"
            "lastLogin      bigint,"
            "currentLoginURI    varchar,"
            "lastPasswordChange bigint,"
            "lastEmailCheck     bigint,"
            "waitingForEmailCheck   int,"
            "locale             varchar,"
            "notes              text,"
            "location           varchar,"
            "owner              varchar,"
            "suspended          int,"
            "blackListed        int,"
            "userRole           varchar,"
            "userTypeProprietaryInfo    text,"
            "securityPolicy     text,"
            "securityPolicyChange   bigint,"
            "currentPassword    varchar,"
            "lastPasswords      varchar,"
            "oauthType          varchar,"
            "oauthUserInfo      text"};

    static const std::string AllUsersFieldsForSelect{
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

    static const std::string AllUsersFieldsForUpdate{
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

    static const std::string AllActionLinksFieldsForCreation{
            "Id          varchar(36),"
            "Action         varchar,"
            "UserId         varchar,"
            "template       varchar,"
            "locale         varchar,"
            "message        text,"
            "sent           bigint,"
            "created        bigint,"
            "expires        bigint,"
            "completed      bigint,"
            "canceled       bigint"
    };




    typedef Poco::Tuple <
        std::string,    // Id = 0;
        std::string,    // name;
        std::string,    // description;
        std::string,    // avatar;
        std::string,    // email;
        uint64_t,       // bool validated = false;
        std::string,    // validationEmail;
        uint64_t,       // validationDate = 0;
        uint64_t,       // creationDate = 0;
        std::string,    // validationURI;
        uint64_t,       // bool changePassword = true;
        uint64_t,       // lastLogin = 0;
        std::string,    // currentLoginURI;
        uint64_t,       // lastPasswordChange = 0;
        uint64_t,       // lastEmailCheck = 0;
        uint64_t,      // bool waitingForEmailCheck = false;
        std::string,    // locale;
        std::string,    // notes;
        std::string,    // location;
        std::string,    // owner;
        uint64_t,       // bool suspended = false;
        uint64_t,       // bool blackListed = false;
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

#endif //UCENTRALSEC_STORAGE_USERS_H
