//
// Created by stephane bourque on 2021-07-15.
//

#include <fstream>
#include <iostream>

#include "Poco/CountingStream.h"
#include "Poco/Net/HTMLForm.h"
#include "RESTAPI_avatar_handler.h"
#include "StorageService.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	void AvatarPartHandler::handlePart(const Poco::Net::MessageHeader &Header,
									   std::istream &Stream) {
		FileType_ = Header.get(RESTAPI::Protocol::CONTENTTYPE, RESTAPI::Protocol::UNSPECIFIED);
		if (Header.has(RESTAPI::Protocol::CONTENTDISPOSITION)) {
			std::string Disposition;
			Poco::Net::NameValueCollection Parameters;
			Poco::Net::MessageHeader::splitParameters(Header[RESTAPI::Protocol::CONTENTDISPOSITION],
													  Disposition, Parameters);
			Name_ = Parameters.get(RESTAPI::Protocol::NAME, RESTAPI::Protocol::UNNAMED);
		}
		Poco::CountingInputStream InputStream(Stream);
		Poco::StreamCopier::copyStream(InputStream, OutputStream_);
		Length_ = OutputStream_.str().size();
	};

	void RESTAPI_avatar_handler::DoPost() {
		std::string Id = UserInfo_.userinfo.id;
		SecurityObjects::UserInfo UInfo;

		std::stringstream SS;
		AvatarPartHandler partHandler(Id, Logger_, SS);
		Poco::Net::HTMLForm form(*Request, Request->stream(), partHandler);
		Poco::JSON::Object Answer;

		if (!partHandler.Name().empty() &&
			partHandler.Length() < MicroServiceConfigGetInt("openwifi.avatar.maxsize", 2000000)) {
			Answer.set(RESTAPI::Protocol::AVATARID, Id);
			Answer.set(RESTAPI::Protocol::ERRORCODE, 0);
			Logger_.information(fmt::format("Uploaded avatar: {} Type: {}", partHandler.Name(),
											partHandler.ContentType()));
			StorageService()->AvatarDB().SetAvatar(UserInfo_.userinfo.email, Id, SS.str(),
												   partHandler.ContentType(), partHandler.Name());
			StorageService()->UserDB().SetAvatar(Id, "1");
			Logger().information(fmt::format("Adding avatar for {}", UserInfo_.userinfo.email));
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

		std::string Type, Name, AvatarContent;
		if (!StorageService()->AvatarDB().GetAvatar(UserInfo_.userinfo.email, Id, AvatarContent,
													Type, Name)) {
			return NotFound();
		}
		Logger().information(fmt::format("Retrieving avatar for {}, size:{}",
										 UserInfo_.userinfo.email, AvatarContent.size()));
		return SendFileContent(AvatarContent, Type, Name);
	}

	void RESTAPI_avatar_handler::DoDelete() {
		std::string Id = GetBinding(RESTAPI::Protocol::ID, "");

		if (UserInfo_.userinfo.userRole != SecurityObjects::ROOT && Id != UserInfo_.userinfo.id) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if (!StorageService()->AvatarDB().DeleteAvatar(UserInfo_.userinfo.email, Id)) {
			return NotFound();
		}

		Logger().information(fmt::format("Deleted avatar for {}", UserInfo_.userinfo.email));
		StorageService()->UserDB().SetAvatar(Id, "");
		OK();
	}
} // namespace OpenWifi
