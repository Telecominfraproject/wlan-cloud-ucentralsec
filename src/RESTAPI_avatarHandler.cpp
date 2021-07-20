//
// Created by stephane bourque on 2021-07-15.
//

#include <fstream>
#include <iostream>

#include "RESTAPI_avatarHandler.h"
#include "StorageService.h"
#include "Daemon.h"
#include "Poco/Net/HTMLForm.h"
#include "Utils.h"

namespace uCentral {

    void AvatarPartHandler::handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream) {
        FileType_ = Header.get("Content-Type", "(unspecified)");
        if (Header.has("Content-Disposition")) {
            std::string Disposition;
            Poco::Net::NameValueCollection Parameters;
            Poco::Net::MessageHeader::splitParameters(Header["Content-Disposition"], Disposition, Parameters);
            Name_ = Parameters.get("name", "(unnamed)");
        }
        Poco::CountingInputStream InputStream(Stream);
        std::ofstream OutputStream(TempFile_.path(), std::ofstream::out);
        Poco::StreamCopier::copyStream(InputStream, OutputStream);
        Length_ = InputStream.chars();
    };

    void RESTAPI_avatarHandler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                              Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            DoPost(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
            DoDelete(Request, Response);
        else
            BadRequest(Request, Response);
    }

    void RESTAPI_avatarHandler::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            SecurityObjects::UserInfo UInfo;

            if (Id.empty() || !Storage()->GetUserById(Id, UInfo)) {
                NotFound(Request, Response);
                return;
            }

            //  if there is an avatar, just remove it...
            Storage()->DeleteAvatar(UserInfo_.userinfo.email,Id);

            Poco::TemporaryFile TmpFile;
            AvatarPartHandler partHandler(Id, Logger_, TmpFile);

            Poco::Net::HTMLForm form(Request, Request.stream(), partHandler);
            Poco::JSON::Object Answer;
            if (!partHandler.Name().empty() && partHandler.Length()<Daemon()->ConfigGetInt("ucentral.avatar.maxsize",2000000)) {
                Answer.set("avatarId", Id);
                Answer.set("errorCode", 0);
                Logger_.information(Poco::format("Uploaded avatar: %s Type: %s", partHandler.Name(), partHandler.ContentType()));
                Storage()->SetAvatar(UserInfo_.userinfo.email,
                                     Id, TmpFile, partHandler.ContentType(), partHandler.Name());
            } else {
                Answer.set("avatarId", Id);
                Answer.set("errorCode", 13);
                Answer.set("ErrorText", "Avatar upload could not complete.");
            }
            ReturnObject(Request, Answer, Response);
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_avatarHandler::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if (Id.empty()) {
                NotFound(Request, Response);
                return;
            }
            Poco::TemporaryFile TempAvatar;
            std::string Type, Name;
            if (!Storage()->GetAvatar(UserInfo_.userinfo.email, Id, TempAvatar, Type, Name)) {
                NotFound(Request, Response);
                return;
            }
            SendFile(TempAvatar, Type, Name, Request, Response);
            return;
        } catch (const Poco::Exception&E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_avatarHandler::DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Id = GetBinding("id", "");
            if (Id.empty()) {
                NotFound(Request, Response);
                return;
            }
            if (!Storage()->DeleteAvatar(UserInfo_.userinfo.email, Id)) {
                NotFound(Request, Response);
                return;
            }
            OK(Request, Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}
