<p align="center">
    <img src="images/project/logo.svg" width="200"/>
</p>

# OpenWiFi Security  (OWSEC)

## What is it?
The OWSEC is a service for the TIP OpenWiFi CloudSDK (OWSDK).
OWSEC is the Authentication and Resource Policy Access service for the TIP
OpenWiFi Cloud SDK (OWSDK). OWSEC,
like all other OWSDK microservices, is defined using an OpenAPI definition and uses the ucentral communication
protocol to interact with Access Points. To use the OWSUB, you either need to [build it](#building) or use the
[Docker version](#docker).

## Building
To build the microservice from source, please follow the instructions in [here](./BUILDING.md)

## Docker
To use the CLoudSDK deployment please follow [here](https://github.com/Telecominfraproject/wlan-cloud-ucentral-deploy)

## OpenAPI
You may get static page with OpenAPI docs generated from the definition on [GitHub Page](https://telecominfraproject.github.io/wlan-cloud-ucentralsec/).
Also, you may use [Swagger UI](https://petstore.swagger.io/#/) with OpenAPI definition file raw link (i.e. [latest version file](https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentralsec/main/openapi/owsec.yaml)) to get interactive docs page.

## Usage
Like all other OWSDK services, OWSEC is defined through an OpenAPI. You can use this API to build your own 
applications or integration modules into your own systems. If all you need it to access the OWGW for 
example (the service that manages the APs), you will need to:
- get a token (`/oauth2`)
- find the endpoints on the system (`/systemEndpoints`) 
- choose a microservice to manage (pick an endpoint that matches what you are trying to do by looking at its 
`type`. For the Cloud SDK Controller, type = owgw)
- make your calls (use the PublicEndPoint of the corresponding entry to make your calls, 
do not forget to add `/api/v1` as the root os the call)

The CLI for the [OWGW](https://github.com/telecominfraproject/wlan-cloud-ucentralsec/blob/main/test_scripts/curl/cli) has 
a very good example of this. Look for the `setgateway` function.

You may get static page with OpenAPI docs generated from the definition on [GitHub Page](https://telecominfraproject.github.io/wlan-cloud-ucentralsec/).

Also, you may use [Swagger UI](https://petstore.swagger.io/#/) with OpenAPI definition file raw link (i.e. [latest version file](https://validator.swagger.io/validator?url=https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentralsec/main/openpapi/owsec.yaml)) to get interactive docs page.

## Firewall Considerations
The entire uCentral systems uses several MicroServices. In order for the whole system to work, you should provide the following port
access:

- Security
  - Properties file: owsec.properties
  - Ports
    - Public: 16001
    - Private: 17001
    - ALB: 16101

- Gateway:
  - Properties file: owgw.properties
  - Ports
    - Public: 16002
    - Private: 17002
    - ALB: 16102

- Firmware:
  - Properties file: owfms.properties
  - Ports
    - Public: 16004
    - Private: 17004
    - ALB: 16104

- Provisioning:
  - Properties file: owprov.properties
  - Ports
    - Public: 16004
    - Private: 17004
    - ALB: 16104

### OWSEC Service Configuration
The configuration is kept in a file called `owsec.properties`. To understand the content of this file,
please look [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec/blob/main/CONFIGURATION.md)

### Default username and password
The default username and password are set in `owsec.properties` file. The following entries manage the username and password
```properties
authentication.default.username = tip@ucentral.com
authentication.default.password = XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```
The password is a long sequence of hexadecimal digits. It is the result of hashing the `username` and the `password`.
In order to create the password, please follow these simple instructions.
```bash
echo -n "<password><username>" | shasum -a 256
```
Here is a complete example for username "root@system.com" and the password being "weLoveWifi".
```bash
echo -n "weLoveWifiroot@system.com" | shasum -a 256
b5bfed31e2a272e52973a57b95042ab842db3999475f3d79f1ce0f45f465e34c  -
```
Then you need to modify your properties file like this
```properties
authentication.default.username = root@system.com
authentication.default.password = b5bfed31e2a272e52973a57b95042ab842db3999475f3d79f1ce0f45f465e34c
```
Remember, when you login, use `root@system.com` with the password `weLoveWifi`, not this monster digit sequence.

### Changing default password
On the first startup of the service new user will be created with the default credentials from properties `authentication.default.username` and `authentication.default.password`, but **you will have to change the password** before making any real requests.
You can this using [owgw-ui](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw-ui/) on first login or using the following script:

```bash
export OWSEC=openwifi.wlan.local:16001 # endpoint to your owsec RESTAPI endpoint
#export FLAGS="-k" # uncomment and add curl flags that you would like to pass for the request (for example '-k' may be used to pass errors with self-signed certificates)
export OWSEC_DEFAULT_USERNAME=root@system.com # default username that you've set in property 'authentication.default.username'
export OWSEC_DEFAULT_PASSWORD=weLoveWifi # default password __in cleartext__ from property 'authentication.default.password'
export OWSEC_NEW_PASSWORD=NewPass123% # new password that must be set for the user (must comply with 'authentication.validation.expression')
test_scripts/curl/cli testlogin $OWSEC_DEFAULT_USERNAME $OWSEC_DEFAULT_PASSWORD $OWSEC_NEW_PASSWORD
```

CLI is also included in Docker image if you want to run it this way:

```bash
export OWSEC=openwifi.wlan.local:16001
#export FLAGS="-k"
export OWSEC_DEFAULT_USERNAME=root@system.com
export OWSEC_DEFAULT_PASSWORD=weLoveWifi
export OWSEC_NEW_PASSWORD=NewPass123%
docker run --rm -ti \
  --network=host \
  --env OWSEC \
  --env FLAGS \
  --env OWSEC_DEFAULT_USERNAME \
  --env OWSEC_DEFAULT_PASSWORD \
  --env OWSEC_NEW_PASSWORD \
  tip-tip-wlan-cloud-ucentral.jfrog.io/owsec:main \
  /cli testlogin $OWSEC_DEFAULT_USERNAME $OWSEC_DEFAULT_PASSWORD $OWSEC_NEW_PASSWORD
```

## Firewall Considerations
| Port  | Description                                   | Configurable |
|:------|:----------------------------------------------|:------------:|
| 16001 | Default port for REST API Access to the OWSEC |     yes      |

It is very important that you not use spaces in your OrgName.
## Kafka topics
Toe read more about Kafka, follow the [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/KAFKA.md)

## Contributions
We need more contributors. Should you wish to contribute,
please follow the [contributions](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONTRIBUTING.md) document.

## Pull Requests
Please create a branch with the Jira addressing the issue you are fixing or the feature you are implementing.
Create a pull-request from the branch into master.

## Additional OWSDK Microservices
Here is a list of additional OWSDK microservices
| Name | Description | Link | OpenAPI |
| :--- | :--- | :---: | :---: |
| OWSEC | Security Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec/blob/main/openpapi/owsec.yaml) |
| OWGW | Controller Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/openapi/owgw.yaml) |
| OWFMS | Firmware Management Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms/blob/main/openapi/owfms.yaml) |
| OWPROV | Provisioning Service | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov) | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/openapi/owprov.yaml) |
| OWANALYTICS | Analytics Service | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics) | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics/blob/main/openapi/owanalytics.yaml) |
| OWSUB | Subscriber Service | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal) | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal/blob/main/openapi/userportal.yaml) |
