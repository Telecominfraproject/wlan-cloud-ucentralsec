//
// Created by stephane bourque on 2021-11-12.
//

#ifndef OWSEC_ACLPROCESSOR_H
#define OWSEC_ACLPROCESSOR_H

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

	class ACLProcessor {
	  public:
		enum ACL_OPS { READ, MODIFY, DELETE, CREATE };
		/*
		 *  0) You can only delete yourself if you are a subscriber
			1) You cannot delete yourself
			2) If you are root, you can do anything.
			3) You can do anything to yourself
			4) Nobody can touch a root, unless they are a root, unless it is to get information on a
		 ROOT 5) Creation rules: ROOT -> create anything PARTNER -> (multi-tenant owner)
		 admin,subs,csr,installer,noc,accounting - matches to an entity in provisioning ADMIN ->
		 admin-subs-csr-installer-noc-accounting ACCOUNTING -> subs-installer-csr

		 */
		static inline bool Can(const SecurityObjects::UserInfo &User,
							   const SecurityObjects::UserInfo &Target, ACL_OPS Op) {

			switch (Op) {
			case DELETE: {
				//  can a user delete themselves - yes - only if not root. We do not want a system
				//  to end up rootless
				if (User.id == Target.id) {
					return User.userRole != SecurityObjects::ROOT;
				}
				//  Root can delete anyone
				switch (User.userRole) {
				case SecurityObjects::ROOT:
					return true;
				case SecurityObjects::ADMIN:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::SUBSCRIBER:
					return User.id == Target.id;
				case SecurityObjects::CSR:
					return false;
				case SecurityObjects::SYSTEM:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::INSTALLER:
					return User.id == Target.id;
				case SecurityObjects::NOC:
					return Target.userRole == SecurityObjects::NOC;
				case SecurityObjects::ACCOUNTING:
					return Target.userRole == SecurityObjects::ACCOUNTING;
				case SecurityObjects::PARTNER:
					return Target.userRole != SecurityObjects::ROOT;
				default:
					return false;
				}
			} break;

			case READ: {
				return User.userRole == SecurityObjects::ROOT ||
					   User.userRole == SecurityObjects::ADMIN ||
					   User.userRole == SecurityObjects::PARTNER;
			} break;

			case CREATE: {
				switch (User.userRole) {
				case SecurityObjects::ROOT:
					return true;
				case SecurityObjects::ADMIN:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::SUBSCRIBER:
					return false;
				case SecurityObjects::CSR:
					return Target.userRole == SecurityObjects::CSR;
				case SecurityObjects::SYSTEM:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::INSTALLER:
					return Target.userRole == SecurityObjects::INSTALLER;
				case SecurityObjects::NOC:
					return Target.userRole == SecurityObjects::NOC;
				case SecurityObjects::ACCOUNTING:
					return Target.userRole == SecurityObjects::ACCOUNTING;
				case SecurityObjects::PARTNER:
					return Target.userRole != SecurityObjects::ROOT;
				default:
					return false;
				}
			} break;

			case MODIFY: {
				switch (User.userRole) {
				case SecurityObjects::ROOT:
					return true;
				case SecurityObjects::ADMIN:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::SUBSCRIBER:
					return User.id == Target.id;
				case SecurityObjects::CSR:
					return Target.userRole == SecurityObjects::CSR;
				case SecurityObjects::SYSTEM:
					return Target.userRole != SecurityObjects::ROOT &&
						   Target.userRole != SecurityObjects::PARTNER;
				case SecurityObjects::INSTALLER:
					return Target.userRole == SecurityObjects::INSTALLER;
				case SecurityObjects::NOC:
					return Target.userRole == SecurityObjects::NOC;
				case SecurityObjects::ACCOUNTING:
					return Target.userRole == SecurityObjects::ACCOUNTING;
				case SecurityObjects::PARTNER:
					return Target.userRole != SecurityObjects::ROOT;
				default:
					return false;
				}
			} break;
			default:
				return false;
			}
		}

	  private:
	};

} // namespace OpenWifi

#endif // OWSEC_ACLPROCESSOR_H
