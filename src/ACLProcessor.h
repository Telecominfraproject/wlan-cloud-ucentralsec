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
/*
 *  0) You can only delete yourself if you are a subscriber
    1) You cannot delete yourself
    2) If you are root, you can do anything.
    3) You can do anything to yourself
    4) Nobody can touch a root, unless they are a root, unless it is to get information on a ROOT
    5) Creation rules:
        ROOT -> create anything
        PARTNER -> (multi-tenant owner) admin,subs,csr,installer,noc,accounting - matches to an entity in provisioning
        ADMIN -> admin-subs-csr-installer-noc-accounting
        ACCOUNTING -> subs-installer-csr

 */
        static inline bool Can( const SecurityObjects::UserInfo & User, const SecurityObjects::UserInfo & Target, ACL_OPS Op) {

            // rule 0
            if(User.id == Target.id && User.userRole == SecurityObjects::SUBSCRIBER && Op == DELETE)
                return true;

            //  rule 1
            if(User.id == Target.id && Op==DELETE)
                return false;

            //  rule 2
            if(User.userRole==SecurityObjects::ROOT)
                return true;

            //  rule 3
            if(User.id == Target.id)
                return true;

            //  rule 4
            if(Target.userRole==SecurityObjects::ROOT && Op!=READ)
                return false;

            if(Op==CREATE) {
                if(User.userRole==SecurityObjects::ROOT)
                    return true;
                if(User.userRole==SecurityObjects::PARTNER && (Target.userRole==SecurityObjects::ADMIN ||
                    Target.userRole==SecurityObjects::SUBSCRIBER ||
                    Target.userRole==SecurityObjects::CSR ||
                    Target.userRole==SecurityObjects::INSTALLER ||
                    Target.userRole==SecurityObjects::NOC ||
                    Target.userRole==SecurityObjects::ACCOUNTING))
                    return true;
                if(User.userRole==SecurityObjects::ADMIN &&
                    (Target.userRole==SecurityObjects::ADMIN ||
                    Target.userRole==SecurityObjects::SUBSCRIBER ||
                    Target.userRole==SecurityObjects::CSR ||
                    Target.userRole==SecurityObjects::INSTALLER ||
                    Target.userRole==SecurityObjects::NOC ||
                    Target.userRole==SecurityObjects::ACCOUNTING))
                    return true;
                if(User.userRole==SecurityObjects::ACCOUNTING &&
                    (Target.userRole==SecurityObjects::SUBSCRIBER ||
                    Target.userRole==SecurityObjects::INSTALLER ||
                    Target.userRole==SecurityObjects::CSR))
                    return true;
                return false;
            }

            return true;
        }
    private:

    };

}


#endif //OWSEC_ACLPROCESSOR_H
