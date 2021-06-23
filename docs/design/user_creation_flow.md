# USER creation flow

## pre-requisite
To create a user in the system, someone must first login as the super user as configured in the properties file. Once logged in as super user,
thata person should create another user and convey the super user bit to them too. From that point on, only that second 
created super user should be used. We will call superuser root0 and the created superuser root1

## About usernames

### email is your username
Your email address is your username. The username is case-insensitive.

### ASCII characters only
Usernames must only use ASCII characters.

### forcing domain names
You can allow only certain domain names by configuring the service with `email.includeonly` parameter
```asm
email.includeonly = mycorporatedomain.com
```

### excluding email domains
You may exclude e-mail domains you will not accept emails from in the configuration. 
You could, for example, not allow people in gmail by adding 
```asm
email.exclude = gmail.com
```

### precedence
If `email.includeonly` is used, `email.exclude` is ignored.

## Creating a username
In order to create a username, root1 must use the `/user/0` API call. The creation of a username involves:
- the service will email the new user to verify her email address
- the username remains dormant until the email verification completes
- the email verification maybe canceled anytime by deleting the username
- the email verification process times-out after `email.verification.timeout` in minutes
- the new user must change her password using the `/oauth2?changePassword=true` and filling in a `WebTokenRequestChangePassword` request
- the system will not accept any other calls until the user has changed her password

## Values accepted in user creation
The user creation request must provide the following in the `UserInfo` of the `post`. 

```asm
        id                  required = 0
        name                optional = a string for the user display
        description:        optional = a description of this user
        avatar:             optional = an avatar URI
        email               required = valid email address used as user name
        validated:          ignored
        validationEmail:    ignored
        validationDate:     ignored
        created:            ignored
        valiadationURI:     ignored
        changePassword:     ignored
        lastLogin:          ignored
        currentLoginURI:    ignored
        lastPasswordChange: ignored
        lastEmailCheck:     ignored
        currentPassword:        ignored
        lastPasswords:          ignored
        waitingForEmailCheck:   ignored
        notes:              optional = cumulative notes that may be added in for this user
        location:           optionsl = UUID of a provisioning server location
        owner:              optional = UUID of a providioning server owner 
        suspended:          optional = if true, the user can change password but not do anything else
        blackListed:        ignored
        locale:             optional = 2 letter code of country language, default to EN. If the language specified is not supported, EN  is assumed.
        userType:           required = root/admin/csr/sub/system/special, defaults to sub
        oauthType:          optional = if using oauth, a recognized oauth provider
        oauthUserInfo:      ignored
```

## Values accepted during user update
When doing a `put`, these are the accepted fields.

```asm
        id                  required = must match the ID in the path
        name                optional = a string for the user display
        description:        optional = a description of this user
        avatar:             optional = an avatar URI
        email               ignored 
        validated:          ignored
        validationEmail:    ignored
        validationDate:     ignored
        created:            ignored
        valiadationURI:     ignored
        changePassword:     optonal = set to true to force a password change for the user
        lastLogin:          ignored
        currentLoginURI:    ignored
        lastPasswordChange: ignored
        lastEmailCheck:     ignored
        currentPassword:        ignored
        lastPasswords:          ignored
        waitingForEmailCheck:   ignored
        notes:              optional = cumulative notes that may be added in for this user
        location:           optionsl = UUID of a provisioning server location
        owner:              optional = UUID of a providioning server owner 
        suspended:          optional = if true, the user can change password but not do anything else
        blackListed:        optional = if true, user cannot login/deleted
        locale:             optional = 2 letter code of country language, default to EN. If the language specified is not supported, EN  is assumed.
        userType:           required = root/admin/csr/sub/system/special, defaults to sub
        oauthType:          optional = if using oauth, a recognized oauth provider
        oauthUserInfo:      ignored
```




