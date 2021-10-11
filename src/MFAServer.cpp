//
// Created by stephane bourque on 2021-10-11.
//

#include "MFAServer.h"
#include "Daemon.h"
#include "SMSSender.h"
#include "SMTPMailerService.h"

namespace OpenWifi {

    class MFAServer * MFAServer::instance_ = nullptr;

    int MFAServer::Start() {
        return 0;
    }

    void MFAServer::Stop() {
    }

    bool MFAServer::StartMFAChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo, Poco::JSON::Object &ChallengeStart) {
        std::lock_guard G(Mutex_);

        CleanCache();

        if(!MethodEnabled(UInfo.userinfo.userTypeProprietaryInfo.mfa.method))
            return false;

        std::string Challenge = MakeChallenge();
        std::string uuid = Daemon()->CreateUUID();
        uint64_t Created = std::time(nullptr);

        ChallengeStart.set("uuid",uuid);
        ChallengeStart.set("created", Created);
        ChallengeStart.set("method", UInfo.userinfo.userTypeProprietaryInfo.mfa.method);

        Cache_[uuid] = MFACacheEntry{ .UInfo = UInfo, .Answer=Challenge, .Created=Created, .Method=UInfo.userinfo.userTypeProprietaryInfo.mfa.method };
        return SendChallenge(UInfo, UInfo.userinfo.userTypeProprietaryInfo.mfa.method, Challenge);
    }

    bool MFAServer::SendChallenge(const SecurityObjects::UserInfoAndPolicy &UInfo, const std::string &Method, const std::string &Challenge) {
        if(Method=="sms" && SMSSender()->Enabled() && !UInfo.userinfo.userTypeProprietaryInfo.mobiles.empty()) {
            std::string Message = "This is your login code: " + Challenge + " Please enter this in your login screen.";
            return SMSSender()->Send(UInfo.userinfo.userTypeProprietaryInfo.mobiles[0].number, Message);
        }

        if(Method=="email" && SMTPMailerService()->Enabled() && !UInfo.userinfo.email.empty()) {
            MessageAttributes Attrs;
            Attrs[RECIPIENT_EMAIL] = UInfo.userinfo.email;
            Attrs[LOGO] = "logo.jpg";
            Attrs[SUBJECT] = "Login validation code";
            Attrs[CHALLENGE_CODE] = Challenge;
            return SMTPMailerService()->SendMessage(UInfo.userinfo.email, "verification_code.txt", Attrs);
        }

        return false;
    }

    bool MFAServer::ResendCode(const std::string &uuid) {
        std::lock_guard G(Mutex_);
        auto Hint = Cache_.find(uuid);
        if(Hint==Cache_.end())
            return false;
        return SendChallenge(Hint->second.UInfo, Hint->second.Method, Hint->second.Answer);
    }

    bool MFAServer::CompleteMFAChallenge(Poco::JSON::Object::Ptr &ChallengeResponse, SecurityObjects::UserInfoAndPolicy &UInfo) {
        std::lock_guard G(Mutex_);

        if(!ChallengeResponse->has("uuid") || !ChallengeResponse->has("answer"))
            return false;

        auto uuid = ChallengeResponse->get("uuid").toString();
        auto Hint = Cache_.find(uuid);
        if(Hint == end(Cache_))
            return false;

        auto answer = ChallengeResponse->get("answer").toString();
        if(Hint->second.Answer!=answer) {
            return false;
        }

        UInfo = Hint->second.UInfo;
        Cache_.erase(Hint);
        return true;
    }

    bool MFAServer::MethodEnabled(const std::string &Method) {
        if(Method=="sms")
            return SMSSender()->Enabled();

        if(Method=="email")
            return SMTPMailerService()->Enabled();

        return false;
    }

    void MFAServer::CleanCache() {
        // it is assumed that you have locked Cache_ at this point.
        uint64_t Now = std::time(nullptr);
        for(auto i=begin(Cache_);i!=end(Cache_);) {
            if((Now-i->second.Created)>300) {
                i = Cache_.erase(i);
            } else {
                ++i;
            }
        }
    }
}