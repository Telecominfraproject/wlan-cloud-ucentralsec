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
  - Properties file: owsec.properties
  - Ports
    - Public: 16001
    - Private: 17001
    - ALB: 16101

- Gateway:
  - Properties file: ucentralgw.properties
  - Ports
    - Public: 16002
    - Private: 17002
    - ALB: 16102

- Firmware:
  - Properties file: ucentralfms.properties
  - Ports
    - Public: 16004
    - Private: 17004
    - ALB: 16104

## Security Configuration
The service relies on a properties configuration file called `owsec.properties`. In this file, you should configure several entries. Many values are optional 
and you can rely on the defaults. Here are some values of note:

### `authentication.default.password`
Set the hash of the default username and password. Please look below on how to do this. 

### `authentication.default.username`
Set the default username to use to login.

### Default username and password
The default username and password are set in `owsec.properties` file. The following entries manage the username and password
```text
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
```text
authentication.default.username = root@system.com
authentication.default.password = b5bfed31e2a272e52973a57b95042ab842db3999475f3d79f1ce0f45f465e34c
```
Remember, when you login, use `root@system.com` with the password `weLoveWifi`, not this monster digit sequence.

#### Is this safe?
Is this safe to show the hash in a text file? Let me put it this way, if you can find a way to break this encryption, you
would have control over the entire internet. It's incredibly safe. If you love math, you can find a lot of videos explaining
how hashes work and why they are safe.


### `authentication.validation.expression`
This is a regular expression (regex) to verify the incoming password. You can find many examples on the internet on how to create these expressions. I suggest 
that using Google is your friend. Someone has figured out what you want to do already. Click [here](https://stackoverflow.com/questions/19605150/regex-for-password-must-contain-at-least-eight-characters-at-least-one-number-a)
to get a sample. The default is

```
^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$
```

### `authentication.oldpasswords`
The number of older passwords to keep. Default is 5.

### Kafka integration
This security service uses Kafka to coordinate security with other services that are part of the system. You must have a Kafka service running
in order to use this. You can find several examples of Kafka services available with Docker. Here are the values you need to configure.

```asm
openwifi.kafka.group.id = security
openwifi.kafka.client.id = security1
openwifi.kafka.enable = true
openwifi.kafka.brokerlist = my.kafkaserver.arilia.com:9092
openwifi.kafka.auto.commit = false
openwifi.kafka.queue.buffering.max.ms = 50
```

#### `openwifi.kafka.brokerlist`
This is the list of your kafka brokers. This is a comma separated list. You should use IP addresses or FQDNs and the relevant ports, usually 9092 is the 
default.

#### `openwifi.kafka.group.id`
Every service on the Kafka bux must have a unique value (at least in our case). This should be a string. We suggest using a name corresponding to the 
function provided. In this case, security.

### Certificates
Of course we need certificates. In our case, we already have existing certificates we have. You should find out how your file name correspond
to our names. We suggest reusing the same names we use so it is easier to use our default configuration files. We suggest using proper certificates 
for the publicly visible interfaces. For private interfaces, self-signed certificates are OK. We will not describe how to use/create private certificates 
here.

#### The public interface
Here are the parameters for the public interface. The important files are:
- `restapi-ca.pem` : the CA of your certificate
- `restapi-cert.pem` : the certificate for the public interface
- `restapi-key.pem` : the key associated with this certificate
- `openwifi.restapi.host.0.key.password` : if you key is password protected, you may supply that password here.

```asm
openwifi.restapi.host.0.backlog = 100
openwifi.restapi.host.0.security = relaxed
openwifi.restapi.host.0.rootca = $OWSEC_ROOT/certs/restapi-ca.pem
openwifi.restapi.host.0.address = *
openwifi.restapi.host.0.port = 16001
openwifi.restapi.host.0.cert = $OWSEC_ROOT/certs/restapi-cert.pem
openwifi.restapi.host.0.key = $OWSEC_ROOT/certs/restapi-key.pem
openwifi.restapi.host.0.key.password = mypassword
```

#### The private interface
The private interface is used for service-to-service communication. You can use self-signed certificates here or letsencrypt. The file names are similar 
to the filenames used in the previous section.

```asm
openwifi.internal.restapi.host.0.backlog = 100
openwifi.internal.restapi.host.0.security = relaxed
openwifi.internal.restapi.host.0.rootca = $OWSEC_ROOT/certs/restapi-ca.pem
openwifi.internal.restapi.host.0.address = *
openwifi.internal.restapi.host.0.port = 17001
openwifi.internal.restapi.host.0.cert = $OWSEC_ROOT/certs/restapi-cert.pem
openwifi.internal.restapi.host.0.key = $OWSEC_ROOT/certs/restapi-key.pem
openwifi.internal.restapi.host.0.key.password = mypassword
```

### Other important values
Here are other important values you must set.


```asm
openwifi.system.data = $OWSEC_ROOT/data
openwifi.system.uri.private = https://localhost:17001
openwifi.system.uri.public = https://openwifi.dpaas.arilia.com:16001
openwifi.system.commandchannel = /tmp/app.ucentralsec
openwifi.service.key = $OWSEC_ROOT/certs/restapi-key.pem
```

#### `openwifi.system.data`
The location of some important data files including the user name database.

#### `openwifi.system.uri.private`
This is the FQDN used internally between services.

#### `openwifi.system.uri.public`
This is the FQDN used externally serving the OpenAPI interface.

