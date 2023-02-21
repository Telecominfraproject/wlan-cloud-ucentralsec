//
// Created by stephane bourque on 2021-12-27.
//

#pragma once

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/orm.h"

namespace OpenWifi {

	typedef Poco::Tuple<std::string, // id
						uint64_t,	 // modified
						std::string	 // data
						>
		PreferencesRecordTuple;
	typedef std::vector<PreferencesRecordTuple> PreferencesRecordTupleList;

	class PreferencesDB : public ORM::DB<PreferencesRecordTuple, SecurityObjects::Preferences> {
	  public:
		PreferencesDB(const std::string &name, const std::string &shortname, OpenWifi::DBType T,
					  Poco::Data::SessionPool &P, Poco::Logger &L);

		bool GetPreferences(std::string &Id, SecurityObjects::Preferences &P);
		bool SetPreferences(SecurityObjects::Preferences &P);
		bool DeletePreferences(const std::string &AdminId, std::string &Id);
		virtual ~PreferencesDB() {}

	  private:
	};
} // namespace OpenWifi
