//
// Created by stephane bourque on 2021-06-25.
//

#include "StorageService.h"

namespace uCentral {

    bool Storage::CreateUser(const std::string & Admin, SecurityObjects::UserInfo & NewUser) {
        return true;
    }

    bool Storage::DeleteUser(const std::string & Admin, uint64_t Id)  {
        return true;
    }

    bool Storage::SetOwner(const std::string & Admin, uint64_t Id, const std::string &Owner)  {
        return true;
    }

    bool Storage::SetLocation(const std::string & Admin, uint64_t Id, const std::string &Location)  {
        return true;
    }

    Storage::AUTH_ERROR Storage::ChangePassword(const std::string & Admin, uint64_t Id, const std::string &OldPassword, const std::string &NewPassword)  {
        return SUCCESS;
    }

    bool Storage::AddNotes(const std::string & Admin, uint64_t Id, const std::string &Notes)   {
        return true;
    }

    bool Storage::SetPolicyChange(const std::string & Admin, const std::string &NewPolicy)   {
        return true;
    }

}

