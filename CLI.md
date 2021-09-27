# Security Service CLI Documentation

## Before using the CLI
You must set the environment variable `OWSEC`. You must specify the host and port for the security service. Here is an example
```csh
export OWSEC=mysecurityservice,example.com:16001
```
Once set, you can start using the `CLI`.

## General format
Most commands will take from 0 to 2 parameters. You should include all parameters in double quotes when possible.

## The commands

### listendpoints                          
Get all the system endpoints.

### emailtest                              
Generate a forgot Password e-amil to the logged in user.

### me
Show information about the logged user.

### createuser <email> <password>          
Create a user with an initial password and force the user to change password.

### createuser_v <email> <password>        
Same as create user but also force an e-mail verification.

### deleteuser <user UUID>                 
Delete the user.
    
### getuser <user UUID>                    
Get the user information.

### listusers
List users.

### policies
List the login and access policies.

### setavatar <user UUID> <filename>       
Sets the avatar for user to the image in filename.

### getavatar <user UUID>                  
Get the avatar for the user.

### deleteavatar <user UUID>               
Remove the avatar for a user.

### sendemail <recipient> <from>           
Sends a test email to see if the e-mail system is working.

### setloglevel <subsystem> <loglevel>
Set the log level for s specific subsystem.

### getloglevels
Get the current log levels for all subsystems.

### getloglevelnames
Get the log level names available.

### getsubsystemnames
Get the list of subsystems.

### systeminfo
Get basic system information.

### reloadsubsystem <subsystem name>
Reload the configuration for a subsystem.
