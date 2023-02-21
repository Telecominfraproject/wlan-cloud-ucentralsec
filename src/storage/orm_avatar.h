//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/orm.h"

namespace OpenWifi {

	/*
		std::string             id;
		std::string             type;
		uint64_t                created=0;
		std::string             name;
		Poco::Data::LOB<char>   avatar;
	*/

	typedef Poco::Tuple<std::string,	 // id
						std::string,	 // type
						uint64_t,		 // created
						std::string,	 // name
						Poco::Data::BLOB // avatar
						>
		AvatarRecordTuple;
	typedef std::vector<AvatarRecordTuple> AvatarRecordTupleList;

	class AvatarDB : public ORM::DB<AvatarRecordTuple, SecurityObjects::Avatar> {
	  public:
		AvatarDB(const std::string &name, const std::string &shortname, OpenWifi::DBType T,
				 Poco::Data::SessionPool &P, Poco::Logger &L);
		virtual ~AvatarDB() {}

		bool SetAvatar(const std::string &Admin, std::string &Id, const std::string &AvatarContent,
					   std::string &Type, std::string &Name);
		bool GetAvatar(const std::string &Admin, std::string &Id, std::string &AvatarContent,
					   std::string &Type, std::string &Name);
		bool DeleteAvatar(const std::string &Admin, std::string &Id);

	  private:
	};
} // namespace OpenWifi
