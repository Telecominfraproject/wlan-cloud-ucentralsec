//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "RESTAPI_objects.h"

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

    void UserInfo::to_json(Poco::JSON::Object &Obj) const {
	    Obj.set("Id",Id);
	    Obj.set("name",name);
        Obj.set("description", description);
        Obj.set("avatar", avatar);
        Obj.set("email", email);
        Obj.set("validated", validated);
        Obj.set("validationEmail", validationEmail);
        Obj.set("validationDate", validationDate);
        Obj.set("creationDate", creationDate);
        Obj.set("validationURI", validationURI);
        Obj.set("changePassword", changePassword);
        Obj.set("lastLogin", lastLogin);
        Obj.set("currentLoginURI", currentLoginURI);
        Obj.set("lastPasswordChange", lastPasswordChange);
        Obj.set("lastEmailCheck", lastEmailCheck);
        Obj.set("waitingForEmailCheck", waitingForEmailCheck);
        Obj.set("locale", locale);
        Obj.set("notes", notes);
        Obj.set("location", location);
        Obj.set("owner", owner);
        Obj.set("suspended", suspended);
        Obj.set("blackListed", blackListed);
        Obj.set("userRole", userRole);
        Obj.set("userTypeProprietaryInfo", userTypeProprietaryInfo);
        Obj.set("securityPolicy", securityPolicy);
        Obj.set("securityPolicyChange", securityPolicyChange);
    };

    bool UserInfo::from_json(Poco::JSON::Object::Ptr Obj) {
        try {
            if(Obj->has("Id"))
                Id = Obj->get("Id");
            if(Obj->has("name"))
                name = Obj->get("name").toString();
            if(Obj->has("description"))
                description = Obj->get("description").toString();
            if(Obj->has("avatar"))
                avatar = Obj->get("avatar").toString();
            if(Obj->has("email"))
                email = Obj->get("email").toString();
            if(Obj->has("validationEmail"))
                validationEmail = Obj->get("validationEmail").toString();
            if(Obj->has("validationURI"))
                validationURI = Obj->get("validationURI").toString();
            if(Obj->has("currentLoginURI"))
                currentLoginURI = Obj->get("currentLoginURI").toString();
            if(Obj->has("locale"))
                locale = Obj->get("locale").toString();
            if(Obj->has("notes"))
                notes = Obj->get("notes").toString();
            if(Obj->has("userRole"))
                userRole = Obj->get("userRole").toString();
            if(Obj->has("securityPolicy"))
                securityPolicy = Obj->get("securityPolicy").toString();
            if(Obj->has("userTypeProprietaryInfo"))
                description = Obj->get("userTypeProprietaryInfo").toString();
            if(Obj->has("description"))
                description = Obj->get("description").toString();
            if(Obj->has("validationDate"))
                validationDate = Obj->get("validationDate");
            if(Obj->has("creationDate"))
                creationDate = Obj->get("creationDate");
            if(Obj->has("lastLogin"))
                lastLogin = Obj->get("lastLogin");
            if(Obj->has("lastPasswordChange"))
                lastPasswordChange = Obj->get("lastPasswordChange");
            if(Obj->has("lastEmailCheck"))
                lastEmailCheck = Obj->get("lastEmailCheck");
            if(Obj->has("securityPolicyChange"))
                securityPolicyChange = Obj->get("securityPolicyChange");
            if(Obj->has("validated"))
                validated = (Obj->get("validated").toString() == "true");
            if(Obj->has("changePassword"))
                changePassword = (Obj->get("changePassword").toString() == "true");
            if(Obj->has("waitingForEmailCheck"))
                waitingForEmailCheck = (Obj->get("waitingForEmailCheck").toString() == "true");
            if(Obj->has("suspended"))
                suspended = (Obj->get("suspended").toString() == "true");
            if(Obj->has("blackListed"))
                blackListed = (Obj->get("blackListed").toString() == "true");
            return true;
        } catch (const Poco::Exception &E) {

        }
        return false;
    }

}

