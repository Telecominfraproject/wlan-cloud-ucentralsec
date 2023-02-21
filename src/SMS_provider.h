//
// Created by stephane bourque on 2021-10-15.
//

#ifndef OWSEC_SMS_PROVIDER_H
#define OWSEC_SMS_PROVIDER_H

#include "Poco/Logger.h"

namespace OpenWifi {
	class SMS_provider {
	  public:
		virtual bool Initialize() = 0;
		virtual bool Start() = 0;
		virtual bool Stop() = 0;
		virtual bool Running() = 0;
		virtual bool Send(const std::string &Number, const std::string &Message) = 0;
		virtual ~SMS_provider(){};

	  private:
	};
} // namespace OpenWifi

#endif // OWSEC_SMS_PROVIDER_H
