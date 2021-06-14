//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "Daemon.h"
#include "RESTAPI_handler.h"
#include "RESTAPI_objects.h"
#include "Utils.h"

namespace uCentral::Objects {

	void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
		std::string D = ObjStr.empty() ? "{}" : ObjStr;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set(ObjName, DetailsObj);
	}

	void AclTemplate::to_json(Poco::JSON::Object &Obj) const {
		Obj.set("Read",Read_);
		Obj.set("ReadWrite",ReadWrite_);
		Obj.set("ReadWriteCreate",ReadWriteCreate_);
		Obj.set("Delete",Delete_);
		Obj.set("PortalLogin",PortalLogin_);
	}

	void WebToken::to_json(Poco::JSON::Object & Obj) const {
		Poco::JSON::Object  AclTemplateObj;
		acl_template_.to_json(AclTemplateObj);
		Obj.set("access_token",access_token_);
		Obj.set("refresh_token",refresh_token_);
		Obj.set("token_type",token_type_);
		Obj.set("expires_in",expires_in_);
		Obj.set("idle_timeout",idle_timeout_);
		Obj.set("created",created_);
		Obj.set("username",username_);
		Obj.set("aclTemplate",AclTemplateObj);
	}
}

