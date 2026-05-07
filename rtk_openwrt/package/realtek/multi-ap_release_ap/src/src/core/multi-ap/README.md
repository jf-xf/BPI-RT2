# Reaktek EashMesh Daemon

## File Structure

The following files are contained in this folder.
- `lib` : Additional libraries.
    - `libmultiap.so` : Shared library for EasyMesh core service.
- `map_agent.*` : EasyMesh Agent daemon.
- `map_controller.*` : EasyMesh Controller daemon.
- `map_checker.*` : A background daemon that checks whether EasyMesh daemon is still alive.
- `map_reinit.*` : Platform specific reintialization tool to sync non-volatile storage.
- `map_reset.*` : Reset device to WFA test controller or agent.
- `multiap.conf` : Configuration file for EasyMesh, contains some parameters that user can change.

## Build Image

To build EasyMesh with Realtek SDK, the following features need to be enabled.

### Enable ebtables support
Kernel config > Networking support > Networking options > Network packet filtering framework (Netfilter) > Ethernet Bridge tables (ebtables) support > **ebt: filter table support**

### Enable inotify support
Kernel Config > File systems > Inotify support for userspace

### Enable message queue
Kernel Config > General Setup > POSIX Message Queue

### Disable WDS from Wi-Fi driver
Kernel Config > Device Drivers > Network device support > Wireless LAN > WDS Support

### Enable Multi-AP driver support
Kernel Config > Device Drivers > Network device support > Wireless LAN > Multi-AP Support

### Enable multi-ap user daemon
users config > Multi-AP

### Change openssl version to 1.02d
users config > openssl -> openssl (openssl-1.0.2d)

### Enable ebtable user daemon
users config > ebtables

## Enable logging

The EasyMesh service can write log to file if the busybox logger is enabled. The default logging verbosity is 0.

- 0: No Log
- 1: Error
- 2: Warning
- 3: Info
- 4: Debug
- 5: Trace
- 6: Detail
- 7: Full

### enable log support
Busybox > System Logging Utilities > syslogd
Busybox > System Logging Utilities > logger

## Enable Wi-Fi Alliance EasyMesh Certification Related Features/Tools

### Enable sed
Busybox > Editors > sed

### Enable telnetd
Busybox > Networking Utilities > telnetd
Then add `telnet &` at the end of `users/script/cinit/init.sh`.

### Enable login
Busybox > Login/Password Management Utilities > login

### Increase busybox FEATURE_EDITING_MAX_LEN (8192)
Busybox > Busybox Settings -> Busybox Library Tuning > Command line editing

### Set Board Region to Global
```
flash set HW_WLAN0_REG_DOMAIN 14

flash set HW_WLAN1_REG_DOMAIN 14
```
