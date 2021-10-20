cmake_minimum_required(VERSION 3.13)
project(owsec VERSION 2.2.0)

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

find_package(CppKafka REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

add_executable( owsec
        build
        src/Daemon.h src/Daemon.cpp
        src/framework/MicroService.cpp src/framework/MicroService.h
        src/framework/SubSystemServer.cpp src/framework/SubSystemServer.h
        src/RESTAPI/RESTAPI_oauth2Handler.h src/RESTAPI/RESTAPI_oauth2Handler.cpp
        src/framework/RESTAPI_handler.h src/framework/RESTAPI_handler.cpp
        src/RESTAPI/RESTAPI_server.cpp src/RESTAPI/RESTAPI_server.h
        src/RESTAPI/RESTAPI_SecurityObjects.cpp src/RESTAPI/RESTAPI_SecurityObjects.h
        src/framework/RESTAPI_system_command.h src/framework/RESTAPI_system_command.cpp
        src/AuthService.h src/AuthService.cpp
        src/framework/KafkaManager.h src/framework/KafkaManager.cpp
        src/StorageService.cpp src/StorageService.h
        src/framework/Utils.cpp src/framework/Utils.h
        src/SMTPMailerService.cpp src/SMTPMailerService.h
        src/RESTAPI/RESTAPI_users_handler.cpp src/RESTAPI/RESTAPI_users_handler.h
        src/RESTAPI/RESTAPI_user_handler.cpp src/RESTAPI/RESTAPI_user_handler.h
        src/RESTAPI/RESTAPI_action_links.cpp src/RESTAPI/RESTAPI_action_links.h
        src/RESTAPI/RESTAPI_InternalServer.cpp src/RESTAPI/RESTAPI_InternalServer.h
        src/RESTAPI/RESTAPI_validateToken_handler.cpp src/RESTAPI/RESTAPI_validateToken_handler.h
        src/RESTAPI/RESTAPI_systemEndpoints_handler.cpp src/RESTAPI/RESTAPI_systemEndpoints_handler.h
        src/RESTAPI/RESTAPI_AssetServer.cpp src/RESTAPI/RESTAPI_AssetServer.h
        src/RESTAPI/RESTAPI_avatarHandler.cpp src/RESTAPI/RESTAPI_avatarHandler.h
        src/storage/storage_avatar.cpp src/storage/storage_avatar.h src/storage/storage_users.h
        src/RESTAPI/RESTAPI_email_handler.cpp src/RESTAPI/RESTAPI_email_handler.h
        src/framework/RESTAPI_GenericServer.h src/framework/RESTAPI_GenericServer.cpp
        src/framework/storage_setup.cpp
        src/storage/storage_tables.cpp
        src/framework/RESTAPI_protocol.h
        src/framework/OpenWifiTypes.h
        src/storage/storage_users.cpp
        src/storage/storage_tokens.cpp
        src/framework/RESTAPI_errors.h
        src/framework/Storage.h
        src/SMSSender.cpp src/SMSSender.h
        src/RESTAPI/RESTAPI_sms_handler.cpp src/RESTAPI/RESTAPI_sms_handler.h
        src/MFAServer.cpp src/MFAServer.h
        src/SMS_provider_aws.cpp src/SMS_provider_aws.h
        src/SMS_provider.cpp src/SMS_provider.h
        src/SMS_provider_twilio.cpp src/SMS_provider_twilio.h)

if(NOT SMALL_BUILD)
    target_link_libraries(owsec PUBLIC
            ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${MySQL_LIBRARIES}  ${ZLIB_LIBRARIES}
            CppKafka::cppkafka ${AWSSDK_LINK_LIBRARIES}
            )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owsec PUBLIC PocoJSON)
    endif()
endif()