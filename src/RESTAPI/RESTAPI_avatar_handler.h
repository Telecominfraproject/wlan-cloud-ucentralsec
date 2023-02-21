//
// Created by stephane bourque on 2021-07-15.
//
#pragma once

#include "Poco/Net/PartHandler.h"
#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {

	class AvatarPartHandler : public Poco::Net::PartHandler {
	  public:
		AvatarPartHandler(std::string Id, Poco::Logger &Logger, std::stringstream &ofs)
			: Id_(std::move(Id)), Logger_(Logger), OutputStream_(ofs) {}
		void handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream);
		[[nodiscard]] uint64_t Length() const { return Length_; }
		[[nodiscard]] std::string &Name() { return Name_; }
		[[nodiscard]] std::string &ContentType() { return FileType_; }

	  private:
		uint64_t Length_ = 0;
		std::string FileType_;
		std::string Name_;
		std::string Id_;
		Poco::Logger &Logger_;
		std::stringstream &OutputStream_;

		inline Poco::Logger &Logger() { return Logger_; };
	};

	class RESTAPI_avatar_handler : public RESTAPIHandler {
	  public:
		RESTAPI_avatar_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
							   RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId,
							   bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_POST,
													  Poco::Net::HTTPRequest::HTTP_DELETE,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/avatar/{id}"}; };

		void DoGet() final;
		void DoPost() final;
		void DoDelete() final;
		void DoPut() final{};
	};
} // namespace OpenWifi
