//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_OBJECTS_H
#define UCENTRAL_RESTAPI_OBJECTS_H

#include "Poco/JSON/Object.h"

namespace uCentral::Objects {

	struct AclTemplate {
		bool Read_ = true ;
		bool ReadWrite_ = true ;
		bool ReadWriteCreate_ = true ;
		bool Delete_ = true ;
		bool PortalLogin_ = true ;
		void to_json(Poco::JSON::Object &Obj) const ;
	};

	struct WebToken {
		std::string access_token_;
		std::string refresh_token_;
		std::string id_token_;
		std::string token_type_;
		std::string username_;
		unsigned int expires_in_;
		unsigned int idle_timeout_;
		AclTemplate acl_template_;
		uint64_t    created_;
		void to_json(Poco::JSON::Object &Obj) const ;
	};
}

#endif //UCENTRAL_RESTAPI_OBJECTS_H
