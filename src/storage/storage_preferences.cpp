//
// Created by stephane bourque on 2021-11-16.
//

#include "StorageService.h"
#include "storage_preferences.h"

namespace OpenWifi {
    void Convert(const PreferencesRecord &R,SecurityObjects::Preferences &P ) {
        P.id = R.get<0>();
        P.modified = R.get<1>();
        P.data = RESTAPI_utils::to_stringpair_array(R.get<2>());
    }

    void Convert(const SecurityObjects::Preferences &P, PreferencesRecord &R ) {
        R.set<0>(P.id);
        R.set<1>(P.modified);
        R.set<2>(RESTAPI_utils::to_string(P.data));
    }

    bool Storage::GetPreferences(std::string &Id, SecurityObjects::Preferences &P) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            P.data.clear();
            std::string St2{"SELECT " + AllPreferencesFieldsForSelect + " From Preferences WHERE Id=?"};

            PreferencesRecord   R;

            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(R),
                Poco::Data::Keywords::use(Id);
            Select.execute();
            Convert(R,P);
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SetPreferences(SecurityObjects::Preferences &P) {
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   InsertOrUpdate(Sess);

            std::string InsertOrReplace{
                "insert into Preferences (Id, Modified, Data) VALUES(?,?,?) on conflict(Id) do  "
                "update set Modified=?, Data=?"
            };

            P.modified = time(nullptr);
            std::string     Data = RESTAPI_utils::to_string(P.data);

            InsertOrUpdate << 	ConvertParams(InsertOrReplace),
                Poco::Data::Keywords::use(P.id),
                Poco::Data::Keywords::use(P.modified),
                Poco::Data::Keywords::use(Data),
                Poco::Data::Keywords::use(P.modified),
                Poco::Data::Keywords::use(Data);
            InsertOrUpdate.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::DeletePreferences(const std::string &AdminId, std::string & Id) {
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

            std::string St{ "delete from Preferences where id=?" };

            Delete << 	ConvertParams(St),
                Poco::Data::Keywords::use(Id);
            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

};
