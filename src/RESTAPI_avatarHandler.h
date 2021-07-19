//
// Created by stephane bourque on 2021-07-15.
//

#ifndef UCENTRALSEC_RESTAPI_AVATARHANDLER_H
#define UCENTRALSEC_RESTAPI_AVATARHANDLER_H


#include "RESTAPI_handler.h"

namespace uCentral {

    class AvatarPartHandler : public Poco::Net::PartHandler {
    public:
        AvatarPartHandler(std::string Id, Poco::Logger &Logger, Poco::TemporaryFile &TmpFile) :
                Id_(std::move(Id)),
                Logger_(Logger),
                TempFile_(TmpFile){
        }
        void handlePart(const Poco::Net::MessageHeader &Header, std::istream &Stream);
        [[nodiscard]] uint64_t Length() const { return Length_; }
        [[nodiscard]] std::string &Name() { return Name_; }
        [[nodiscard]] std::string &ContentType() { return FileType_; }
        [[nodiscard]] std::string FileName() const { return TempFile_.path(); }
    private:
        uint64_t        Length_ = 0;
        std::string     FileType_;
        std::string     Name_;
        std::string     Id_;
        Poco::Logger    &Logger_;
        Poco::TemporaryFile &TempFile_;
    };

    class RESTAPI_avatarHandler : public RESTAPIHandler {
    public:
        RESTAPI_avatarHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_GET,
                                         Poco::Net::HTTPRequest::HTTP_POST,
                                         Poco::Net::HTTPRequest::HTTP_DELETE,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                         Internal) {}

        void handleRequest(Poco::Net::HTTPServerRequest &Request,
                           Poco::Net::HTTPServerResponse &Response) override;

        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/avatar/{id}"}; };

        void DoGet(     Poco::Net::HTTPServerRequest &Request,
                        Poco::Net::HTTPServerResponse &Response);
        void DoPost(     Poco::Net::HTTPServerRequest &Request,
                        Poco::Net::HTTPServerResponse &Response);
        void DoDelete(     Poco::Net::HTTPServerRequest &Request,
                        Poco::Net::HTTPServerResponse &Response);
    };
}
#endif //UCENTRALSEC_RESTAPI_AVATARHANDLER_H
