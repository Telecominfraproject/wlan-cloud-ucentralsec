//
// Created by stephane bourque on 2021-11-12.
//

#ifndef OWSEC_ACLPROCESSOR_H
#define OWSEC_ACLPROCESSOR_H

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    class ACLProcessor {
    public:
        enum ACL_OPS {
            READ,
            MODIFY,
            DELETE,
            CREATE
        };
        static inline bool Can( const SecurityObjects::UserInfo & User, const SecurityObjects::UserInfo & Target, ACL_OPS Op) {
            if(User.Id == Target.Id && Op==DELETE)
                return false;

            if(User.userRole==SecurityObjects::ROOT)
                return true;

            if((User.userRole!=SecurityObjects::ADMIN || Target.userRole!=SecurityObjects::ROOT) && Op!=READ)
                return false;

            if(User.userRole==SecurityObjects::ADMIN && Target.userRole==SecurityObjects::ROOT && Op!=READ)
                return false;

            return true;
        }
    private:

    };

}


#endif //OWSEC_ACLPROCESSOR_H
