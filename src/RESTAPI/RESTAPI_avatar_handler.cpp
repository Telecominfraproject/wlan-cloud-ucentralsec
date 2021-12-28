//
// Created by stephane bourque on 2021-07-15.
//

#include <fstream>
#include <iostream>

#include "RESTAPI_avatar_handler.h"
#include "StorageService.h"
#include "Poco/Net/HTMLForm.h"
#include "framework/RESTAPI_protocol.h"
#include "framework/MicroService.h"

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

    void RESTAPI_avatar_handler::DoPost() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        SecurityObjects::UserInfo UInfo;

        if (Id.empty() || !StorageService()->UserDB().GetUserById(Id, UInfo)) {
            return NotFound();
        }

        //  if there is an avatar, just remove it...
        StorageService()->AvatarDB().DeleteAvatar(UserInfo_.userinfo.email,Id);

        Poco::TemporaryFile TmpFile;
        AvatarPartHandler partHandler(Id, Logger_, TmpFile);

        Poco::Net::HTMLForm form(*Request, Request->stream(), partHandler);
        Poco::JSON::Object Answer;
        if (!partHandler.Name().empty() && partHandler.Length()< MicroService::instance().ConfigGetInt("openwifi.avatar.maxsize",2000000)) {
            Answer.set(RESTAPI::Protocol::AVATARID, Id);
            Answer.set(RESTAPI::Protocol::ERRORCODE, 0);
            Logger_.information(Poco::format("Uploaded avatar: %s Type: %s", partHandler.Name(), partHandler.ContentType()));
            StorageService()->AvatarDB().SetAvatar(UserInfo_.userinfo.email,
                                 Id, TmpFile, partHandler.ContentType(), partHandler.Name());
        } else {
            Answer.set(RESTAPI::Protocol::AVATARID, Id);
            Answer.set(RESTAPI::Protocol::ERRORCODE, 13);
            Answer.set(RESTAPI::Protocol::ERRORTEXT, "Avatar upload could not complete.");
        }
        ReturnObject(Answer);
    }

    void RESTAPI_avatar_handler::DoGet() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        if (Id.empty()) {
            return NotFound();
        }
        Poco::TemporaryFile TempAvatar;
        std::string Type, Name;
        if (!StorageService()->AvatarDB().GetAvatar(UserInfo_.userinfo.email, Id, TempAvatar, Type, Name)) {
            return NotFound();
        }
        std::cout << "Sending avatar" << std::endl;
        SendFile(TempAvatar, Type, Name);
    }

    void RESTAPI_avatar_handler::DoDelete() {
        std::string Id = GetBinding(RESTAPI::Protocol::ID, "");
        if (Id.empty()) {
            return NotFound();
        }
        if (!StorageService()->AvatarDB().DeleteAvatar(UserInfo_.userinfo.email, Id)) {
            return NotFound();
        }
        OK();
    }
}
