cmake_minimum_required(VERSION 3.13)
project(ucentralsec VERSION 2.0.0)

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

find_package(CppKafka REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED COMPONENTS JSON Crypto JWT Net Util NetSSL Data DataSQLite DataPostgreSQL DataMySQL)

add_executable( ucentralsec
                build
                src/Daemon.h src/Daemon.cpp
                src/MicroService.cpp src/MicroService.h
                src/SubSystemServer.cpp src/SubSystemServer.h
                src/RESTAPI_oauth2Handler.h src/RESTAPI_oauth2Handler.cpp
                src/RESTAPI_handler.h src/RESTAPI_handler.cpp
                src/RESTAPI_server.cpp src/RESTAPI_server.h
                src/RESTAPI_SecurityObjects.cpp src/RESTAPI_SecurityObjects.h
                src/RESTAPI_system_command.h src/RESTAPI_system_command.cpp
                src/RESTAPI_protocol.h
                src/AuthService.h src/AuthService.cpp
                src/KafkaManager.h src/KafkaManager.cpp
                src/StorageService.cpp src/StorageService.h
                src/Utils.cpp src/Utils.h
                src/storage_sqlite.cpp src/storage_sqlite.cpp src/storage_pgql.cpp src/storage_mysql.cpp
                src/storage_tables.cpp src/SMTPMailerService.cpp src/SMTPMailerService.h
                src/RESTAPI_users_handler.cpp src/RESTAPI_users_handler.h
                src/RESTAPI_user_handler.cpp src/RESTAPI_user_handler.h
                src/RESTAPI_action_links.cpp src/RESTAPI_action_links.h src/storage_users.cpp
                src/RESTAPI_InternalServer.cpp src/RESTAPI_InternalServer.h
                src/RESTAPI_validateToken_handler.cpp src/RESTAPI_validateToken_handler.h
                src/RESTAPI_systemEndpoints_handler.cpp src/RESTAPI_systemEndpoints_handler.h src/RESTAPI_AssetServer.cpp src/RESTAPI_AssetServer.h src/RESTAPI_avatarHandler.cpp src/RESTAPI_avatarHandler.h src/storage_avatar.cpp src/storage_avatar.h src/storage_users.h)

if(NOT SMALL_BUILD)
    target_link_libraries(ucentralsec PUBLIC
            ${Poco_LIBRARIES} ${Boost_LIBRARIES} ${MySQL_LIBRARIES}  ${ZLIB_LIBRARIES}
            CppKafka::cppkafka
            )
    if(UNIX AND NOT APPLE)
        target_link_libraries(ucentralsec PUBLIC PocoJSON)
    endif()
endif()