//
// Created by stephane bourque on 2021-10-11.
//

#ifndef OWSEC_MFASERVER_H
#define OWSEC_MFASERVER_H

#include "framework/MicroService.h"
#include "Poco/JSON/Object.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {
    struct MFACacheEntry {
        SecurityObjects::UserInfoAndPolicy  UInfo;
        std::string                         Answer;
        uint64_t                            Created;
        std::string                         Method;
    };

    typedef std::map<std::string,MFACacheEntry>     MFAChallengeCache;

    class MFAServer : public SubSystemServer{
    public:
        int Start() override;
        void Stop() override;
        static MFAServer *instance() {
            if (instance_ == nullptr) {
                instance_ = new MFAServer;
            }
            return instance_;
        }

        bool StartMFAChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo, Poco::JSON::Object &Challenge);
        bool CompleteMFAChallenge(Poco::JSON::Object::Ptr &ChallengeResponse, SecurityObjects::UserInfoAndPolicy &UInfo);
        bool MethodEnabled(const std::string &Method);
        bool ResendCode(const std::string &uuid);
        bool SendChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo, const std::string &Method, const std::string &Challenge);

        static inline std::string MakeChallenge() {
            return std::to_string(rand() % 999999);
        }

    private:
        static MFAServer *  instance_;
        MFAChallengeCache   Cache_;
        MFAServer() noexcept:
            SubSystemServer("MFServer", "MFA-SVR", "mfa")
            {
            }

        void CleanCache();
    };

    inline MFAServer & MFAServer() { return *MFAServer::instance(); }
}

#endif //OWSEC_MFASERVER_H
