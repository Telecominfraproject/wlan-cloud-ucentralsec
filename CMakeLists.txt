cmake_minimum_required(VERSION 3.13)
project(owsec VERSION 2.5.0)

set(CMAKE_CXX_STANDARD 17)

if(UNIX AND APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(MYSQL_ROOT_DIR /usr/local/opt/mysql-client)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(UNIX AND NOT APPLE)
    set(PostgreSQL_TYPE_INCLUDE_DIR /usr/include/postgresql)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
endif()

if(SMALL_BUILD)
    add_definitions(-DSMALL_BUILD)
endif()

# Auto build increment. You must define BUILD_INCREMENT with cmake -DBUILD_INCREMENT=1
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build)
    file(READ build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE build ${BUILD_NUM})
endif()

set(BUILD_SHARED_LIBS 1)

add_definitions(-DAPP_VERSION="${CMAKE_PROJECT_VERSION}" -DBUILD_NUMBER="${BUILD_NUM}")
add_definitions(-DTIP_SECURITY_SERVICE="1")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED system)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(AWSSDK     REQUIRED COMPONENTS sns)
find_package(nlohmann_json  REQUIRED)
find_package(CppKafka REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

add_executable( owsec
        build
        src/framework/CountryCodes.h
        src/framework/KafkaTopics.h
        src/framework/MicroService.h
        src/framework/orm.h
        src/framework/RESTAPI_errors.h
        src/framework/RESTAPI_protocol.h
        src/framework/StorageClass.h
        src/framework/uCentral_Protocol.h
        src/RESTObjects/RESTAPI_SecurityObjects.h src/RESTObjects/RESTAPI_SecurityObjects.cpp
        src/RESTObjects/RESTAPI_ProvObjects.cpp src/RESTObjects/RESTAPI_ProvObjects.h
        src/RESTObjects/RESTAPI_GWobjects.h src/RESTObjects/RESTAPI_GWobjects.cpp
        src/RESTObjects/RESTAPI_FMSObjects.h src/RESTObjects/RESTAPI_FMSObjects.cpp
        src/RESTAPI/RESTAPI_oauth2_handler.h src/RESTAPI/RESTAPI_oauth2_handler.cpp
        src/RESTAPI/RESTAPI_users_handler.cpp src/RESTAPI/RESTAPI_users_handler.h
        src/RESTAPI/RESTAPI_user_handler.cpp src/RESTAPI/RESTAPI_user_handler.h
        src/RESTAPI/RESTAPI_action_links.cpp src/RESTAPI/RESTAPI_action_links.h
        src/RESTAPI/RESTAPI_validate_token_handler.cpp src/RESTAPI/RESTAPI_validate_token_handler.h
        src/RESTAPI/RESTAPI_system_endpoints_handler.cpp src/RESTAPI/RESTAPI_system_endpoints_handler.h
        src/RESTAPI/RESTAPI_asset_server.cpp src/RESTAPI/RESTAPI_asset_server.h
        src/RESTAPI/RESTAPI_avatar_handler.cpp src/RESTAPI/RESTAPI_avatar_handler.h
        src/RESTAPI/RESTAPI_email_handler.cpp src/RESTAPI/RESTAPI_email_handler.h
        src/RESTAPI/RESTAPI_sms_handler.cpp src/RESTAPI/RESTAPI_sms_handler.h
        src/storage/storage_avatar.cpp src/storage/storage_avatar.h
        src/storage/storage_users.h src/storage/storage_users.cpp
        src/storage/storage_actionLinks.cpp src/storage/storage_actionLinks.h
        src/storage/storage_tokens.h src/storage/storage_tokens.cpp
        src/storage/storage_subtokens.h src/storage/storage_subtokens.cpp
        src/storage/storage_preferences.cpp src/storage/storage_preferences.h
        src/storage/storage_subscribers.cpp src/storage/storage_subscribers.h
        src/storage/storage_tables.cpp
        src/RESTAPI/RESTAPI_suboauth2_handler.h src/RESTAPI/RESTAPI_suboauth2_handler.cpp
        src/RESTAPI/RESTAPI_subuser_handler.h src/RESTAPI/RESTAPI_subuser_handler.cpp
        src/RESTAPI/RESTAPI_subusers_handler.h src/RESTAPI/RESTAPI_subusers_handler.cpp
        src/APIServers.cpp
        src/Daemon.h src/Daemon.cpp
        src/AuthService.h src/AuthService.cpp
        src/StorageService.cpp src/StorageService.h
        src/SMTPMailerService.cpp src/SMTPMailerService.h
        src/SMSSender.cpp src/SMSSender.h
        src/MFAServer.cpp src/MFAServer.h
        src/SMS_provider_aws.cpp src/SMS_provider_aws.h
        src/SMS_provider.cpp src/SMS_provider.h
        src/SMS_provider_twilio.cpp src/SMS_provider_twilio.h
        src/ActionLinkManager.cpp src/ActionLinkManager.h
        src/ACLProcessor.h src/RESTAPI/RESTAPI_preferences.cpp src/RESTAPI/RESTAPI_preferences.h
        src/framework/OpenWifiTypes.h)

if(NOT SMALL_BUILD)
    target_link_libraries(owsec PUBLIC
            ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${MySQL_LIBRARIES}  ${ZLIB_LIBRARIES}
            CppKafka::cppkafka ${AWSSDK_LINK_LIBRARIES}
            )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owsec PUBLIC PocoJSON)
    endif()
endif()