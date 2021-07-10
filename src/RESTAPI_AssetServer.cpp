//
// Created by stephane bourque on 2021-07-10.
//

#include "RESTAPI_AssetServer.h"
#include "Poco/File.h"
#include "Daemon.h"
#include "RESTAPI_server.h"
#include "Utils.h"

namespace uCentral {
    void RESTAPI_AssetServer::handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else
            NotFound(Request, Response);
    }

    void RESTAPI_AssetServer::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        std::string AssetName = GetBinding("id", "");
        Poco::File  AssetFile(RESTAPI_Server()->AssetDir()+"/"+AssetName);
        if(!AssetFile.isFile()) {
            NotFound(Request, Response);
            return;
        }
        SendFile(AssetFile,Request, Response);
    }
}