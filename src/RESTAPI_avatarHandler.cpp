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
#include "RESTAPI_protocol.h"

namespace OpenWifi {

    void AvatarPartHandler::handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream) {
        FileType_ = Header.get(RESTAPI::Protocol::CONTENTTYPE, RESTAPI::Protocol::UNSPECIFIED);
        if (Header.has(RESTAPI::Protocol::CONTENTDISPOSITION)) {
            std::string Disposition;
            Poco::Net::NameValueCollection Parameters;
            Poco::Net::MessageHeader::splitParameters(Header[RESTAPI::Protocol::CONTENTDISPOSITION], Disposition, Parameters);
            Name_ = Parameters.get(RESTAPI::Protocol::NAME, RESTAPI::Protocol::UNNAMED);
        }
        Poco::CountingInputStream InputStream(Stream);
        std::ofstream OutputStream(TempFile_.path(), std::ofstream::out);
        Poco::StreamCopier::copyStream(InputStream, OutputStream);
        Length_ = InputStream.chars();
    };

    void RESTAPI_avatarHandler::DoPost() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        SecurityObjects::UserInfo UInfo;

        if (Id.empty() || !Storage()->GetUserById(Id, UInfo)) {
            return NotFound();
        }

        //  if there is an avatar, just remove it...
        Storage()->DeleteAvatar(UserInfo_.userinfo.email,Id);

        Poco::TemporaryFile TmpFile;
        AvatarPartHandler partHandler(Id, Logger_, TmpFile);

        Poco::Net::HTMLForm form(*Request, Request->stream(), partHandler);
        Poco::JSON::Object Answer;
        if (!partHandler.Name().empty() && partHandler.Length()<Daemon()->ConfigGetInt("openwifi.avatar.maxsize",2000000)) {
            Answer.set(RESTAPI::Protocol::AVATARID, Id);
            Answer.set(RESTAPI::Protocol::ERRORCODE, 0);
            Logger_.information(Poco::format("Uploaded avatar: %s Type: %s", partHandler.Name(), partHandler.ContentType()));
            Storage()->SetAvatar(UserInfo_.userinfo.email,
                                 Id, TmpFile, partHandler.ContentType(), partHandler.Name());
        } else {
            Answer.set(RESTAPI::Protocol::AVATARID, Id);
            Answer.set(RESTAPI::Protocol::ERRORCODE, 13);
            Answer.set(RESTAPI::Protocol::ERRORTEXT, "Avatar upload could not complete.");
        }
        ReturnObject(Answer);
    }

    void RESTAPI_avatarHandler::DoGet() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        if (Id.empty()) {
            return NotFound();
        }
        Poco::TemporaryFile TempAvatar;
        std::string Type, Name;
        if (!Storage()->GetAvatar(UserInfo_.userinfo.email, Id, TempAvatar, Type, Name)) {
            return NotFound();
        }
        SendFile(TempAvatar, Type, Name);
    }

    void RESTAPI_avatarHandler::DoDelete() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        if (Id.empty()) {
            return NotFound();
        }
        if (!Storage()->DeleteAvatar(UserInfo_.userinfo.email, Id)) {
            return NotFound();
        }
        OK();
    }
}
