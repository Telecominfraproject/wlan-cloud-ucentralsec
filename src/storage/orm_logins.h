//
// Created by stephane bourque on 2022-01-05.
//

#pragma once

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/orm.h"

namespace OpenWifi {

	typedef Poco::Tuple<std::string, // SessionId
						std::string, // UserID
						std::string, // UserEMail
						uint64_t,	 // login = 0;
						uint64_t	 // logout = 0;
						>
		LoginInfoRecordTuple;
	typedef std::vector<LoginInfoRecordTuple> LoginInfoRecordTupleList;

	class LoginDB : public ORM::DB<LoginInfoRecordTuple, SecurityObjects::LoginRecordInfo> {
	  public:
		LoginDB(const std::string &name, const std::string &shortname, OpenWifi::DBType T,
				Poco::Data::SessionPool &P, Poco::Logger &L);
		virtual ~LoginDB() {}

		void AddLogin(const std::string &id, const std::string &email, const std::string &token);
		void AddLogout(const std::string &token);
	};

} // namespace OpenWifi
