# User Validation Actions 
The system uses email and links to offer user validation. Validation may include email verification, password reset
requests, etc.

## Action links
All action links will come back to the endpoint
```
https://site.com:port/api/v1/actions?command=payload
```

## Payload 
The system encrypts the payload with its private key. A base64 encoder convert the encrypted message to something
valid for a URI.

```json
{
  "id" : "uuid",
  "email" : "email@host.com",
  "type" : "verificationType"
}
```

- `id` : UUID of the request that should be outstanding
- `email` : email address of the user under verification
- `type` : could be one of "emailValidation", "passwordResetRequest", or other.

## Templates
Email templates come in 2 flavors: txt and html. The system replaces the following occurrences with system variables:

- {{action}} : the link that the user should press. That link should include the payload as above.
- {{name}} : the name of the user field.
- {{recipient}} : user email, as-is

 


