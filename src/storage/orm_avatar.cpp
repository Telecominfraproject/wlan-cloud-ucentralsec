//
// Created by stephane bourque on 2021-12-27.
//

#include "orm_avatar.h"

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
            ORM::Field{"type", ORM::FieldType::FT_BIGINT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"avatar", ORM::FieldType::FT_BLOB}
    };

    AvatarDB::AvatarDB(const std::string &Name, const std::string &ShortName, OpenWifi::DBType T,
                               Poco::Data::SessionPool &P, Poco::Logger &L) :
            DB(T, Name.c_str(), AvatarDB_Fields,{}, P, L, ShortName.c_str()) {
    }

    bool AvatarDB::SetAvatar(const std::string &Admin, std::string &Id, Poco::TemporaryFile &FileName,
                             std::string &Type, std::string &Name) {

        try {
            std::stringstream ss;
            std::ifstream ifs(FileName.path().c_str(), std::ios_base::in | std::ios_base::binary);
            Poco::StreamCopier::copyStream(ifs, ss);

            SecurityObjects::Avatar A;
            A.id = Id;
            A.type = Type;
            A.name = Name;
            A.created = std::time(nullptr);
            A.avatar.assignRaw(ss.str().c_str(), ss.str().size());

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

    bool AvatarDB::GetAvatar(const std::string &Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type,
                   std::string &Name) {
        SecurityObjects::Avatar A;
        try {
            if(GetRecord("id",Id,A)) {
                Type = A.type;
                Name = A.name;

                Poco::Data::LOBInputStream IL(A.avatar);
                std::ofstream f(FileName.path(), std::ios::binary);
                Poco::StreamCopier::copyStream(IL, f);

                return true;
            }
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        } catch (...) {

        }
        return false;
    }

    bool AvatarDB::DeleteAvatar(const std::string &Admin, std::string &Id) {
        return DeleteRecord("id",Id);
    }

}

template<> void ORM::DB<OpenWifi::AvatarRecordTuple,
        OpenWifi::SecurityObjects::Avatar>::Convert(OpenWifi::AvatarRecordTuple &T, OpenWifi::SecurityObjects::Avatar &U) {
    U.id = T.get<0>();
    U.type = T.get<1>();
    U.created = T.get<2>();
    U.name = T.get<3>();
    U.avatar = T.get<4>();
}

template<> void ORM::DB<OpenWifi::AvatarRecordTuple,
        OpenWifi::SecurityObjects::Avatar>::Convert(OpenWifi::SecurityObjects::Avatar &U, OpenWifi::AvatarRecordTuple &T) {
    T.set<0>(U.id);
    T.set<1>(U.type);
    T.set<2>(U.created);
    T.set<3>(U.name);
    T.set<4>(U.avatar);
}
