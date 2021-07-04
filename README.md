# ucentralsec

uCentralSec is the Authentication & Resource Policy Access service for the uCentral system. In order to use the uCentral system
you must have at least 1 uCentralSec. uCentralSec is the first point of contact for the entire architecture. We strongly recommend using Docker 
to deploy all the uCentral services. If you would like to develop and play with the source, please do.

## OpenAPI
Like all other uCentral services, uCentralSec is defined through an OpenAPI. You can use this API to build your own applications or integration modules
into your own systems. If all you need it to access the uCentralGW for example (the service that manages the APs), you will need to:

- get a token (`/oauth2`)
- find the endpoints on the system (`/systemEndpoints`) 
- choose one to manage (pick an endpoint that matches what you are trying to do by looking at its `type`. For the gateway, type = ucentrtalgw)
- make your calls (use the PublicEndPoint of the corresponding entry to make your calls, do not forget to add `/api/v1` as the root os the call)

The CLI for the [uCentralGW](https://github.com/telecominfraproject/wlan-cloud-ucentralgw/blob/main/test_scripts/curl/cli) has a very good example of this. Loog for the `setgateway` 
function.

## Firewall Considerations
The entire uCentral systems uses several MicroServices. In order for the whole system to work, you should provide the following port
access

- Security
    - Public: 16001
    - Private: 17001
    - ALB: 16101

- Gateway:
    - Public: 16002
    - Private: 17002
    - ALB: 16102

- Firmware:
    - Public: 16004
    - Private: 17004
    - ALB: 16104
