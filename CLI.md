# Security Service CLI Documentation

## Before using the CLI
You must set the environment variable `UCENTRALSEC`. You must specify the host and port for the security service. Here is an example
```csh
export UCENTRALSEC=mysecurityservice,example.com:16001
```
Once set, you can start using the `CLI`.

## General format
Most commands will take from 0 to 2 parameters. You should include all parameters in double quotes when possible.

## The commands

### `cli createuser <email> <initial password>`
This will create a simple user as admin using the email as login ID and setting the initial password.

### `cli createuser_v <email> <initial password>`
This will create a simple user and force email verification. 

### `cli deleteuser <id>`
Delete the specified user using the user's UUID.

### `cli getuser <id>`
Get the specified user using the user's UUID.

### `cli listusers`
Get a list of users.

### `cli policies`
List the link used to display password and usage policies for the management site.

### `cli setavatar <id> <filename>`
Sets the avatar for the user with ID. The file should be gif, png, svg.

### `cli deleteavatar <id>`
Remove the avatar fort the specified user ID.

### `cli secversion`
Get the vewrsion of the secufiry service.

### `cli sectimes`
Get the starttime and uptime for the security service.




### `cli revisions`
Get the list of currently available revisions.

### `cli devicetypes`
Retrieve the list of known `device_types`

### `cli firmwareage <device_type> <revision>`
If you specify your `device_type` and `revision`, the system will do its best to estimate how
far in the past you `revision` is compared to the latest revision.

### `cli gethistory <serialNumber>`
Get the revision history for a given device.

### `cli connecteddevices`
Get a list of the currently known devices and the last connection information we have about the,

### `cli connecteddevice <serialNumber>`
Get the information relevant to a specific device.

### `cli devicereport`
Give a simplified dashboard report of the data in the service.

### `cli fmsversion`
Display the version of the service.

### `cli fmstimes`
Display the uptime and start time of the service.

