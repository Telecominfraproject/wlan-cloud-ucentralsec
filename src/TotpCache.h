//
// Created by stephane bourque on 2022-01-31.
//

#ifndef OWSEC_TOTPCACHE_H
#define OWSEC_TOTPCACHE_H

#include "framework/MicroService.h"
#include "seclibs/cpptotp/bytes.h"
#include "seclibs/qrcode/qrcodegen.hpp"
#include "seclibs/cpptotp/otp.h"

namespace OpenWifi {

    class TotpCache : public SubSystemServer {
    public:

        struct Entry {
            bool        Subscriber=false;
            uint64_t    Start = 0;
            uint64_t    Done = 0 ;
            uint64_t    Verifications = 0 ;
            std::string Secret;
            std::string QRCode;
            std::string LastCode;
        };

        static auto instance() {
            static auto instance = new TotpCache;
            return instance;
        }

        static std::string GenerateSecret(uint Size, std::string & Base32Secret) {
            std::string R;

            for(;Size;Size--) {
                R += (char) MicroService::instance().Random(33,127);
            }
            Base32Secret = CppTotp::Bytes::toBase32( CppTotp::Bytes::ByteString{ (const u_char *)R.c_str()});
            return R;
        }

        std::string GenerateQRCode(const std::string &Secret, const std::string &email) {

            std::string uri{
                "otpauth://totp/" + Issuer_ + ":" +
                email + "?secret=" + Secret + "&issuer=" + Issuer_
            };

            qrcodegen::QrCode qr0 = qrcodegen::QrCode::encodeText(uri.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
            std::string svg = qrcodegen::toSvgString(qr0, 4);  // See QrCodeGeneratorDemo
            return svg;
        }

        static bool ValidateCode( const std::string &Secret, const std::string &Code, std::string & Expecting) {
            uint64_t Now = std::time(nullptr);
            uint32_t p = CppTotp::totp(CppTotp::Bytes::ByteString{ (const u_char *)Secret.c_str()}, Now, 0, 30, 6);
            char buffer[16];
            sprintf(buffer,"%06u",p);
            Expecting = buffer;
            return Code == buffer;
        }

        int Start() override {
            Issuer_ = MicroService::instance().ConfigGetString("totp.issuer","OpenWiFi");
            return 0;
        };

        void Stop() override {

        };

        inline bool StartValidation(const SecurityObjects::UserInfo &User, bool Subscriber, std::string & QRCode, bool Reset) {
            auto Hint = Cache_.find(User.id);
            if(Hint!=Cache_.end() && Hint->second.Subscriber==Subscriber) {
                if(Reset) {
                    std::string Base32Secret;
                    Hint->second.Subscriber = Subscriber;
                    Hint->second.Start = std::time(nullptr);
                    Hint->second.Done = 0;
                    Hint->second.Verifications = 0;
                    Hint->second.Secret = GenerateSecret(20,Base32Secret);
                    Hint->second.QRCode = QRCode = GenerateQRCode(Base32Secret, User.email);
                    Hint->second.LastCode.clear();
                } else {
                    QRCode = Hint->second.QRCode;
                }
                return true;
            }

            std::string Base32Secret;
            auto Secret = GenerateSecret(20, Base32Secret);
            QRCode = GenerateQRCode(Base32Secret, User.email);

            Entry E{ .Subscriber = Subscriber,
                     .Start = (uint64_t )std::time(nullptr),
                     .Done = 0,
                     .Verifications = 0,
                     .Secret = Secret,
                     .QRCode = QRCode
                     };
            Cache_[User.id] = E;
            return true;
        }

        inline bool ContinueValidation(const SecurityObjects::UserInfo &User, bool Subscriber, const std::string & Code,
                                       uint64_t &NextIndex, bool &MoreCodes, uint64_t & ErrorCode, std::string & ErrorText ) {
            auto Hint = Cache_.find(User.id);
            uint64_t Now = std::time(nullptr);
            ErrorCode = 0;
            if(Hint!=Cache_.end() && Subscriber==Hint->second.Subscriber && (Now-Hint->second.Start)<(15*60)) {
                std::string Expecting;
                if (NextIndex == 1 && Hint->second.Verifications == 0 && ValidateCode(Hint->second.Secret, Code, Expecting)) {
                    NextIndex++;
                    Hint->second.Verifications++;
                    MoreCodes = true;
                    Hint->second.LastCode = Code;
                    return true;
                } else if (NextIndex == 2 && Hint->second.Verifications == 1 && Code != Hint->second.LastCode &&
                            ValidateCode(Hint->second.Secret, Code, Expecting) ) {
                    MoreCodes = false;
                    Hint->second.Done = Now;
                    return true;
                } else {
                    if(!ValidateCode(Hint->second.Secret, Code, Expecting)) {
                        ErrorCode = 1;
                        ErrorText = "Invalid code.";
                        return false;
                    } else if(NextIndex!=1 && NextIndex != 2) {
                        ErrorCode = 2;
                        ErrorText = "Invalid Index";
                        return false;
                    } else if(Code == Hint->second.LastCode) {
                        ErrorCode = 3;
                        ErrorText = "Code is repeated. Must be new code.";
                        return false;
                    }
                    ErrorCode = 5;
                    ErrorText = "Invalid protocol sequence.";
                    return false;
                }
            } else {
                ErrorCode = 4;
                ErrorText = "No validation session present.";
            }
            return false;
        }

        inline bool CompleteValidation(const SecurityObjects::UserInfo &User, bool Subscriber, std::string & Secret) {
            auto Hint = Cache_.find(User.id);
            uint64_t Now = std::time(nullptr);
            if(Hint!=Cache_.end() && Subscriber==Hint->second.Subscriber && (Now-Hint->second.Start)<(15*60) && Hint->second.Done!=0) {
                Secret = Hint->second.Secret;
                std::cout << "completed validation: " << Secret << std::endl;
                Cache_.erase(Hint);
                return true;
            }
            return false;
        }

    private:
        std::map<std::string,Entry>     Cache_;
        std::string                     Issuer_;

        TotpCache() noexcept:
            SubSystemServer("TOTP-system", "TOTP-SVR", "totp") {
        }

    };

    inline auto TotpCache() { return TotpCache::instance(); }
}

#endif //OWSEC_TOTPCACHE_H
