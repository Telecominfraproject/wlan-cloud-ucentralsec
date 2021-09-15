//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_OAUTH2HANDLER_H
#define UCENTRAL_RESTAPI_OAUTH2HANDLER_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
	class RESTAPI_oauth2Handler : public RESTAPIHandler {
	  public:
	    RESTAPI_oauth2Handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
													  Poco::Net::HTTPRequest::HTTP_DELETE,
                                                      Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
													  Server,
													  Internal, false) {}
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/oauth2/{token}","/api/v1/oauth2"}; };
		void DoGet() final;
		void DoPost() final;
		void DoDelete() final;
		void DoPut() final {};
	};
}
#endif //UCENTRAL_RESTAPI_OAUTH2HANDLER_H
