cmake_minimum_required(VERSION 3.13)
project(owsec VERSION 3.2.0)

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
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/build BUILD_NUM)
    if(BUILD_INCREMENT)
        MATH(EXPR BUILD_NUM "${BUILD_NUM}+1")
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
    endif()
else()
    set(BUILD_NUM 1)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build ${BUILD_NUM})
endif()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_RESULT
            OUTPUT_VARIABLE GIT_HASH)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "git rev-parse --short HEAD failed with ${GIT_RESULT}")
    endif()
    string(REGEX REPLACE "\n$" "" GIT_HASH "${GIT_HASH}")
endif()

add_definitions(-DAWS_CUSTOM_MEMORY_MANAGEMENT -DBOOST_NO_CXX98_FUNCTION_BASE=1)

set(BUILD_SHARED_LIBS 1)

add_definitions(-DTIP_SECURITY_SERVICE="1")
add_definitions(-DPOCO_LOG_DEBUG="1")

add_compile_options(-Wall -Wextra)
if(ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(fmt  REQUIRED)
find_package(AWSSDK     REQUIRED COMPONENTS sns)
find_package(nlohmann_json  REQUIRED)
find_package(CppKafka REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)

include_directories(/usr/local/include  /usr/local/opt/openssl/include src include/kafka /usr/local/opt/mysql-client/include)

configure_file(src/ow_version.h.in ${PROJECT_SOURCE_DIR}/src/ow_version.h @ONLY)

add_executable( owsec
        build
        src/ow_version.h.in
        src/framework/CountryCodes.h
        src/framework/KafkaTopics.h
        src/framework/MicroService.h
        src/framework/OpenWifiTypes.h
        src/framework/orm.h
        src/framework/StorageClass.h
        src/framework/MicroServiceErrorHandler.h
        src/framework/UI_WebSocketClientServer.cpp
        src/framework/UI_WebSocketClientServer.h
        src/framework/UI_WebSocketClientNotifications.cpp
        src/framework/UI_WebSocketClientNotifications.h
        src/framework/utils.h
        src/framework/utils.cpp
        src/framework/AppServiceRegistry.h
        src/framework/SubSystemServer.cpp
        src/framework/SubSystemServer.h
        src/framework/RESTAPI_utils.h
        src/framework/AuthClient.cpp
        src/framework/AuthClient.h
        src/framework/MicroServiceNames.h
        src/framework/MicroServiceFuncs.h
        src/framework/OpenAPIRequests.cpp
        src/framework/OpenAPIRequests.h
        src/framework/MicroServiceFuncs.cpp
        src/framework/ALBserver.cpp
        src/framework/ALBserver.h
        src/framework/KafkaManager.cpp
        src/framework/KafkaManager.h
        src/framework/RESTAPI_RateLimiter.h
        src/framework/WebSocketLogger.h
        src/framework/RESTAPI_GenericServerAccounting.h
        src/framework/RESTAPI_SystemConfiguration.h
        src/framework/CIDR.h
        src/framework/RESTAPI_Handler.cpp
        src/framework/RESTAPI_Handler.h
        src/framework/RESTAPI_ExtServer.h
        src/framework/RESTAPI_ExtServer.cpp
        src/framework/RESTAPI_IntServer.cpp
        src/framework/RESTAPI_IntServer.h
        src/framework/RESTAPI_SystemCommand.h
        src/framework/RESTAPI_WebSocketServer.h
        src/framework/EventBusManager.cpp
        src/framework/EventBusManager.h
        src/framework/RESTAPI_PartHandler.h
        src/framework/MicroService.cpp
        src/framework/MicroServiceExtra.h
        src/framework/default_device_types.h
        src/RESTObjects/RESTAPI_SecurityObjects.h src/RESTObjects/RESTAPI_SecurityObjects.cpp
        src/RESTObjects/RESTAPI_GWobjects.h src/RESTObjects/RESTAPI_GWobjects.cpp
        src/RESTObjects/RESTAPI_FMSObjects.h src/RESTObjects/RESTAPI_FMSObjects.cpp
        src/RESTObjects/RESTAPI_CertObjects.cpp src/RESTObjects/RESTAPI_CertObjects.h
        src/RESTObjects/RESTAPI_OWLSobjects.cpp src/RESTObjects/RESTAPI_OWLSobjects.h
        src/RESTObjects/RESTAPI_ProvObjects.cpp src/RESTObjects/RESTAPI_ProvObjects.h
        src/RESTObjects/RESTAPI_AnalyticsObjects.cpp src/RESTObjects/RESTAPI_AnalyticsObjects.h
        src/RESTObjects/RESTAPI_SubObjects.cpp src/RESTObjects/RESTAPI_SubObjects.h

        src/seclibs/qrcode/qrcodegen.hpp src/seclibs/qrcode/qrcodegen.cpp
        src/seclibs/cpptotp/bytes.cpp src/seclibs/cpptotp/bytes.h
        src/seclibs/cpptotp/otp.cpp src/seclibs/cpptotp/otp.h
        src/seclibs/cpptotp/sha1.cpp src/seclibs/cpptotp/sha1.h
        src/RESTAPI/RESTAPI_oauth2_handler.h src/RESTAPI/RESTAPI_oauth2_handler.cpp
        src/RESTAPI/RESTAPI_users_handler.cpp src/RESTAPI/RESTAPI_users_handler.h
        src/RESTAPI/RESTAPI_user_handler.cpp src/RESTAPI/RESTAPI_user_handler.h
        src/RESTAPI/RESTAPI_action_links.cpp src/RESTAPI/RESTAPI_action_links.h
        src/RESTAPI/RESTAPI_validate_token_handler.cpp src/RESTAPI/RESTAPI_validate_token_handler.h
        src/RESTAPI/RESTAPI_system_endpoints_handler.cpp src/RESTAPI/RESTAPI_system_endpoints_handler.h
        src/RESTAPI/RESTAPI_asset_server.cpp src/RESTAPI/RESTAPI_asset_server.h
        src/RESTAPI/RESTAPI_avatar_handler.cpp src/RESTAPI/RESTAPI_avatar_handler.h
        src/RESTAPI/RESTAPI_subavatar_handler.cpp src/RESTAPI/RESTAPI_subavatar_handler.h
        src/RESTAPI/RESTAPI_email_handler.cpp src/RESTAPI/RESTAPI_email_handler.h
        src/RESTAPI/RESTAPI_sms_handler.cpp src/RESTAPI/RESTAPI_sms_handler.h
        src/RESTAPI/RESTAPI_suboauth2_handler.h src/RESTAPI/RESTAPI_suboauth2_handler.cpp
        src/RESTAPI/RESTAPI_subuser_handler.h src/RESTAPI/RESTAPI_subuser_handler.cpp
        src/RESTAPI/RESTAPI_subusers_handler.h src/RESTAPI/RESTAPI_subusers_handler.cpp
        src/RESTAPI/RESTAPI_validate_sub_token_handler.cpp src/RESTAPI/RESTAPI_validate_sub_token_handler.h
        src/RESTAPI/RESTAPI_submfa_handler.cpp src/RESTAPI/RESTAPI_submfa_handler.h
        src/RESTAPI/RESTAPI_preferences.cpp src/RESTAPI/RESTAPI_preferences.h
        src/RESTAPI/RESTAPI_subpreferences.cpp src/RESTAPI/RESTAPI_subpreferences.h
        src/RESTAPI/RESTAPI_routers.cpp
        src/Daemon.h src/Daemon.cpp
        src/SpecialUserHelpers.h
        src/AuthService.h src/AuthService.cpp
        src/StorageService.cpp src/StorageService.h
        src/SMTPMailerService.cpp src/SMTPMailerService.h
        src/SMSSender.cpp src/SMSSender.h
        src/MFAServer.cpp src/MFAServer.h
        src/SMS_provider_aws.cpp src/SMS_provider_aws.h
        src/SMS_provider.cpp src/SMS_provider.h
        src/SMS_provider_twilio.cpp src/SMS_provider_twilio.h
        src/ActionLinkManager.cpp src/ActionLinkManager.h
        src/ACLProcessor.h
        src/storage/orm_users.cpp src/storage/orm_users.h
        src/storage/orm_tokens.cpp src/storage/orm_tokens.h
        src/storage/orm_preferences.cpp src/storage/orm_preferences.h
        src/storage/orm_actionLinks.cpp src/storage/orm_actionLinks.h
        src/storage/orm_avatar.cpp src/storage/orm_avatar.h
        src/SpecialUserHelpers.h
        src/RESTAPI/RESTAPI_db_helpers.h src/storage/orm_logins.cpp src/storage/orm_logins.h
        src/RESTAPI/RESTAPI_totp_handler.cpp
        src/RESTAPI/RESTAPI_totp_handler.h
        src/TotpCache.h
        src/RESTAPI/RESTAPI_subtotp_handler.cpp src/RESTAPI/RESTAPI_subtotp_handler.h
        src/RESTAPI/RESTAPI_signup_handler.cpp src/RESTAPI/RESTAPI_signup_handler.h
        src/MessagingTemplates.h src/RESTAPI/RESTAPI_apiKey_handler.cpp src/RESTAPI/RESTAPI_apiKey_handler.h src/storage/orm_apikeys.cpp src/storage/orm_apikeys.h src/RESTAPI/RESTAPI_validate_apikey.cpp src/RESTAPI/RESTAPI_validate_apikey.h src/RESTAPI/RESTAPI_systemSecret_handler.cpp src/RESTAPI/RESTAPI_systemSecret_handler.h src/SecretStore.cpp src/SecretStore.h)

if(NOT SMALL_BUILD)
    target_link_libraries(owsec PUBLIC
            ${Poco_LIBRARIES}
            ${MySQL_LIBRARIES}
            ${ZLIB_LIBRARIES}
            CppKafka::cppkafka
            ${AWSSDK_LINK_LIBRARIES}
            fmt::fmt
            resolv
            )
    if(UNIX AND NOT APPLE)
        target_link_libraries(owsec PUBLIC PocoJSON)
    endif()
endif()
