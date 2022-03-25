//
// Created by stephane bourque on 2021-07-15.
//
#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

    class SubAvatarPartHandler : public Poco::Net::PartHandler {
    public:
        SubAvatarPartHandler(std::string Id, Poco::Logger &Logger, std::stringstream & ofs) :
                Id_(std::move(Id)),
                Logger_(Logger),
                OutputStream_(ofs){
        }
        void handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream);
        [[nodiscard]] uint64_t Length() const { return Length_; }
        [[nodiscard]] std::string &Name() { return Name_; }
        [[nodiscard]] std::string &ContentType() { return FileType_; }

    private:
        uint64_t        Length_ = 0;
        std::string     FileType_;
        std::string     Name_;
        std::string     Id_;
        [[maybe_unused]] Poco::Logger    &Logger_;
        std::stringstream &OutputStream_;
    };

    class RESTAPI_subavatar_handler : public RESTAPIHandler {
    public:
        RESTAPI_subavatar_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_GET,
                                         Poco::Net::HTTPRequest::HTTP_POST,
                                         Poco::Net::HTTPRequest::HTTP_DELETE,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                         Server,
                                         TransactionId,
                                         Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/subavatar/{id}"}; };

        void DoGet() final;
        void DoPost() final;
        void DoDelete() final;
        void DoPut() final {};

    };
}
