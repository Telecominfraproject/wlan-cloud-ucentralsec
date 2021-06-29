# Kafka topics for micro-services.
Here are some of the topics going on the kafka bus to maintain the microservice architecture

## `service_events` topic

### Payload

#### Service joins the stack
Upon starting up, a service needs to send this message.
```json
{
  "event" : "join",
  "id" : 1,
  "type" : "ucentral/security/firmware/prov/topology/other",
  "endPoint" : "https://endpoint.com:16001",
  "key" : "access-key",
  "version": "1.0.0"
}
```

#### Service leaves the stack
Upon going down, a service should send this notice.
```json
{
  "event" : "leave",
  "id" : 1,
  "type" : "ucentral/security/firmware/prov/topology/other",
  "endPoint" : "https://endpoint.com:16001",
  "key" : "access-key",
  "version": "1.0.0"
}
```

#### Service heartbeat
Sent every minute by all services.
```json
{
  "event" : "keep-alive",
  "id" : 1,
  "type" : "ucentral/security/firmware/prov/topology/other",
  "endPoint" : "https://endpoint.com:16001",
  "key" : "access-key",
  "version": "1.0.0"
}
```

