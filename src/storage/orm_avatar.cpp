//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_avatar.h"
#include "Poco/Data/LOBStream.h"
#include "StorageService.h"

/*
    std::string             id;
    std::string             type;
    uint64_t                created=0;
    std::string             name;
    Poco::Data::LOB<char>   avatar;

 */
namespace OpenWifi {

    static ORM::FieldVec AvatarDB_Fields{
            ORM::Field{"id", 36, true},
            ORM::Field{"type", ORM::FieldType::FT_TEXT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"avatar", ORM::FieldType::FT_BLOB}
    };

    AvatarDB::AvatarDB(const std::string &Name, const std::string &ShortName, OpenWifi::DBType T,
                               Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, Name.c_str(), AvatarDB_Fields,{}, P, L, ShortName.c_str()) {
    }

    bool AvatarDB::SetAvatar([[maybe_unused]] const std::string &Admin, std::string &Id, const std::string & AvatarContent,
                             std::string &Type, std::string &Name) {

        try {
            SecurityObjects::Avatar A;
            A.id = Id;
            A.type = Type;
            A.name = Name;
            A.created = std::time(nullptr);
            // Poco::Data::LOBOutputStream OL(A.avatar);
            // OL << AvatarContent;
            A.avatar.appendRaw((const unsigned char *)AvatarContent.c_str(),AvatarContent.size());
//            A.avatar.assignRaw(AvatarContent.c_str(), AvatarContent.size());

            if (Exists("id", Id)) {
                return UpdateRecord("id", Id, A);
            }
            return CreateRecord(A);
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        } catch (...) {

        }
        return false;
    }

    inline uint8_t fromhex(const char c) {
        if(c>='0' && c<='9')
            return c-'0';
        if(c>='a' && c<='f')
            return c-'a'+10;
        if(c>='A' && c<='F')
            return c-'A'+10;
        return 0;
    }

    bool AvatarDB::GetAvatar([[maybe_unused]] const std::string &Admin, std::string &Id, std::string & AvatarContent,
                             std::string &Type, std::string &Name) {
        SecurityObjects::Avatar A;
        try {
            if(GetRecord("id",Id,A)) {
                Type = A.type;
                Name = A.name;
                if(StorageService()->Type() == DBType::pgsql) {
//                    Poco::Data::BLOBInputStream IL(A.avatar);
                    auto Content = A.avatar.content();
                    for(const auto &c:Content) {
                        AvatarContent += (char) c;
                    }
/*                    while(IL.std::basic_ios<char>::good()) {
                        char buf[16000];
                        std::size_t Size = IL.readsome((unsigned char *)buf,16000);
                        AvatarContent += std::string{buf,Size};
                    }
*/
/*
                    Poco::StreamCopier::copyStream(IL, os);
                    IL.
                    std::string tmp = os.str().substr(2);
                    AvatarContent.reserve(tmp.size()/2);
                    for(size_t i=0;i<tmp.size(); i+=2) {
                        AvatarContent += (char) (fromhex(tmp[i])*16 + fromhex(tmp[i+1]));
                    }
*/
                    Logger().information(fmt::format("Avatar size:4 {}", AvatarContent.size()));
                } else {
/*                    Poco::Data::BLOBInputStream      IL(A.avatar);
                    std::ostringstream              os("",std::ios_base::binary);
                    Poco::StreamCopier::copyStream(IL, os);
                    AvatarContent = os.str();
                    Logger().information(fmt::format("Avatar size: {}", os.str().size()));
*/
                }
                return true;
            }
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        } catch (...) {

        }
        return false;
    }

    bool AvatarDB::DeleteAvatar([[maybe_unused]] const std::string &Admin, std::string &Id) {
        return DeleteRecord("id",Id);
    }

}

template<> void ORM::DB<OpenWifi::AvatarRecordTuple,
        OpenWifi::SecurityObjects::Avatar>::Convert(const OpenWifi::AvatarRecordTuple &T,
                                                    OpenWifi::SecurityObjects::Avatar &U) {
    U.id = T.get<0>();
    U.type = T.get<1>();
    U.created = T.get<2>();
    U.name = T.get<3>();
    U.avatar = T.get<4>();
}

template<> void ORM::DB<OpenWifi::AvatarRecordTuple,
        OpenWifi::SecurityObjects::Avatar>::Convert(const OpenWifi::SecurityObjects::Avatar &U,
                                                    OpenWifi::AvatarRecordTuple &T) {
    T.set<0>(U.id);
    T.set<1>(U.type);
    T.set<2>(U.created);
    T.set<3>(U.name);
    T.set<4>(U.avatar);
}
