//
// Created by stephane bourque on 2021-11-08.
//

#include "storage_actionLinks.h"

#include <string>

#include "StorageService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    bool Convert(const ActionLinkRecord &T, SecurityObjects::ActionLink &U) {
        U.id = T.get<0>();
        U.action = T.get<1>();
        U.userId = T.get<2>();
        U.actionTemplate = T.get<3>();
        U.variables = RESTAPI_utils::to_stringpair_array(T.get<4>());
        U.locale = T.get<5>();
        U.message = T.get<6>();
        U.sent = T.get<7>();
        U.created = T.get<8>();
        U.expires = T.get<9>();
        U.completed = T.get<10>();
        U.canceled = T.get<11>();
        return true;
    }

    bool Convert(const SecurityObjects::ActionLink &U, ActionLinkRecord &T) {
        T.set<0>(U.id);
        T.set<1>(U.action);
        T.set<2>(U.userId);
        T.set<3>(U.actionTemplate);
        T.set<4>(RESTAPI_utils::to_string(U.variables));
        T.set<5>(U.locale);
        T.set<6>(U.message);
        T.set<7>(U.sent);
        T.set<8>(U.created);
        T.set<9>(U.expires);
        T.set<10>(U.completed);
        T.set<11>(U.canceled);
        return true;
    }

    bool Storage::CreateAction( SecurityObjects::ActionLink & A) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Insert(Sess);
            std::string St2{
                "INSERT INTO ActionLinks (" + AllActionLinksFieldsForSelect + ") VALUES(" + AllActionLinksValuesForSelect + ")"};
            ActionLinkRecord AR;
            Convert(A, AR);
            Insert << ConvertParams(St2),
                Poco::Data::Keywords::use(AR);
            Insert.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetActions(std::vector<SecurityObjects::ActionLink> &Links, uint64_t Max) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            ActionLinkRecordList ARL;

            std::string St2{
                "SELECT " + AllActionLinksFieldsForSelect + " From ActionLinks where sent=0 and canceled=0"};
            Select << ConvertParams(St2),
                Poco::Data::Keywords::into(ARL);
            Select.execute();

            for(const auto &i:ARL) {
                SecurityObjects::ActionLink L;
                Convert(i,L);
                Links.emplace_back(L);
            }

            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::GetActionLink(std::string &ActionId, SecurityObjects::ActionLink &A) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Select(Sess);

            ActionLinkRecord AR;

            std::string St2{
                "SELECT " + AllActionLinksFieldsForSelect + " From ActionLinks where id=?"};
            Select << ConvertParams(St2),
            Poco::Data::Keywords::into(AR),
            Poco::Data::Keywords::use(ActionId);
            Select.execute();

            if(Select.rowsExtracted()==1) {
                Convert(AR, A);
                return true;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::SentAction(std::string &ActionId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint64_t Sent = std::time(nullptr);
            std::string St{"UPDATE ActionLinks set Sent=? where id=?"};
            Update << ConvertParams(St),
                Poco::Data::Keywords::use(Sent),
                Poco::Data::Keywords::use(ActionId);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::DeleteAction(std::string &ActionId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Delete(Sess);

            uint64_t Sent = std::time(nullptr);
            std::string St{"DELETE FROM ActionLinks where id=?"};
            Delete << ConvertParams(St),
                Poco::Data::Keywords::use(ActionId);
            Delete.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::CompleteAction(std::string &ActionId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint64_t completed = std::time(nullptr);
            std::string St{"UPDATE ActionLinks set completed=? where id=?"};
            Update << ConvertParams(St),
                Poco::Data::Keywords::use(completed),
                Poco::Data::Keywords::use(ActionId);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool Storage::CancelAction(std::string &ActionId) {
        try {
            Poco::Data::Session Sess = Pool_->get();
            Poco::Data::Statement Update(Sess);

            uint64_t canceled = std::time(nullptr);
            std::string St{"UPDATE ActionLinks set canceled=? where id=?"};
            Update << ConvertParams(St),
                Poco::Data::Keywords::use(canceled),
                Poco::Data::Keywords::use(ActionId);
            Update.execute();
            return true;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

}