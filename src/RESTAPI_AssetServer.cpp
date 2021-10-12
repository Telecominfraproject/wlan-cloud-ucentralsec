//
// Created by stephane bourque on 2021-07-10.
//

#include "RESTAPI_AssetServer.h"
#include "Poco/File.h"
#include "Daemon.h"
#include "RESTAPI_server.h"
#include "Utils.h"
#include "RESTAPI_protocol.h"

namespace OpenWifi {
    void RESTAPI_AssetServer::DoGet() {
        Poco::File  AssetFile;

        if(Request->getURI().find("/favicon.ico") != std::string::npos) {
            AssetFile = RESTAPI_Server()->AssetDir() + "/favicon.ico";
        } else {
            std::string AssetName = GetBinding(RESTAPI::Protocol::ID, "");
            AssetFile = RESTAPI_Server()->AssetDir() + "/" + AssetName;
        }
        if(!AssetFile.isFile()) {
            return NotFound();
        }
        SendFile(AssetFile);
    }
}