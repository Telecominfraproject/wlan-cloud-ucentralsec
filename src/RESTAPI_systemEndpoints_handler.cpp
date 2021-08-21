//
// Created by stephane bourque on 2021-07-01.
//

#include "RESTAPI_systemEndpoints_handler.h"
#include "Daemon.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {
    void RESTAPI_systemEndpoints_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                        Poco::Net::HTTPServerResponse &Response) {

        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        try {
            if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                auto Services = Daemon()->GetServices();

                SecurityObjects::SystemEndpointList L;

                for(const auto &i:Services) {
                    SecurityObjects::SystemEndpoint S{
                        .type = i.Type,
                        .id = i.Id,
                        .uri = i.PublicEndPoint};
                    L.endpoints.push_back(S);
                }
                Poco::JSON::Object  Obj;
                L.to_json(Obj);

                ReturnObject(Request, Obj, Response);
                return;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

}
