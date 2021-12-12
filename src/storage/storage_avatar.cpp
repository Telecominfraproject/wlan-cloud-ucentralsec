//
// Created by stephane bourque on 2021-07-15.
//

#include <iostream>
#include <fstream>

#include "storage_avatar.h"
#include "../StorageService.h"

#include "Poco/File.h"
#include "Poco/Data/LOBStream.h"
#include "../Daemon.h"

namespace OpenWifi {

    /*
                            "Id			    VARCHAR(36) PRIMARY KEY, "
                            "Type			VARCHAR, "
                            "Created 		BIGINT, "
                            "Name           VARCHAR, "
                            "Avatar     	BLOB"
     */

    bool Storage::SetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string &Type, std::string & Name) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);

            Poco::Data::LOB<char> L;
            Poco::Data::LOBOutputStream OL(L);

            std::ifstream f(FileName.path(), std::ios::binary);
            Poco::StreamCopier::copyStream(f, OL);

            uint64_t Now = std::time(nullptr);

            std::string St2{"INSERT INTO Avatars (" + AllAvatarFieldsForSelect + ") VALUES( " + AllAvatarValuesForSelect + " )"};

            Insert << ConvertParams(St2),
                    Poco::Data::Keywords::use(Id),
                    Poco::Data::Keywords::use(Type),
                    Poco::Data::Keywords::use(Now),
                    Poco::Data::Keywords::use(Name),
                    Poco::Data::Keywords::use(L);
            Insert.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool Storage::GetAvatar(const std::string & Admin, std::string &Id, Poco::TemporaryFile &FileName, std::string & Type, std::string & Name) {
        try {
            Poco::Data::LOB<char> L;
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            std::string St2{"SELECT " + AllAvatarFieldsForSelect + " FROM Avatars WHERE Id=?"};

            Poco::Data::Statement Select2(Sess);

            std::string TId;
            uint64_t    Created;

            Select2 << ConvertParams(St2),
                    Poco::Data::Keywords::into(TId),
                    Poco::Data::Keywords::into(Type),
                    Poco::Data::Keywords::into(Created),
                    Poco::Data::Keywords::into(Name),
                    Poco::Data::Keywords::into(L),
                    Poco::Data::Keywords::use(Id);
            Select2.execute();

            Poco::Data::LOBInputStream IL(L);
            std::ofstream f(FileName.path(), std::ios::binary);
            Poco::StreamCopier::copyStream(IL, f);
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }

    bool Storage::DeleteAvatar(const std::string & Admin, std::string &Id) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            std::string     St1{"delete from avatars where id=?"};

            Delete <<   ConvertParams(St1),
                        Poco::Data::Keywords::use(Id);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }


}