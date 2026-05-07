# Index

- [Multi-AP acknowledgment and resending mechanism](#multi-ap-acknowledgment-and-resending-mechanism)
- [How to enable Multi-AP in Realtek SDK](#how-to-enable-multi-ap-in-realtek-sdk)
  - [enable ebtables support](#enable-ebtables-support)
  - [enable inotify support](#enable-inotify-support)
  - [enable message queue](#enable-message-queue)
  - [enable Multi-AP support](#enable-multi-ap-support)
  - [enable openssl](#enable-openssl)
  - [enable multi-ap](#enable-multi-ap)
  - [enable log support](#enable-log-support)
- [Enable Test Related Features/Tools](#enable-test-related-featurestools)
  - [Enable sed](#enable-sed)
  - [Enable touch](#enable-touch)
  - [Enable telnetd](#enable-telnetd)
  - [Enable login](#enable-login)
  - [increase busybox FEATURE_EDITING_MAX_LEN (8192)](#increase-busybox-feature_editing_max_len-8192)
- [Multi-AP Setup](#multi-ap-setup)
  - [Set Region to Global](#set-region-to-global)
  - [Enable 11k and 11v](#enable-11k-and-11v)
  - [Controller Setup](#controller-setup)
  - [Agent Setup](#agent-setup)
  - [Onboarding](#onboarding)
- [ALME to Trigger CMDU](#alme-to-trigger-cmdu)
  - [MAP Related MIBs](#map-related-mibs)

# Multi-AP acknowledgment and resending mechanism
As stated in the multi-ap specification (Chapter 15 Multi-AP control messaging reliability), for the unicast message that does not get acknowledged by 1905 ACK or response message in one second, the multi-ap daemon will resend the message stream using different message id.
1. The streams forged from CMDU message that needs acknowledging are saved for later use.
2. Timeout is set to 1-second according to the multi-ap specification.
3. The saved streams will be resent 3 times at most if it doesn't get acknowledged in time.
4. The acknowledge can come from 1905 ACK message or the paired responding message.

```
------------------------------------------------------------------------------------------------------
     ##################                                     ##################
     # sending thread #                      |              # timeout thread #
     ##################                      |              ##################
                                             |
  send1905RawPacket                          |         +----------+
         |                                   |         |   Timer  |
  +------v-----+  N   +-------------+        |         |  timeout |
  |  Need ACK? +----->+ Free Stream |        |         +----+-----+
  +------+-----+      +------+------+        |              |
       Y |                   |               |              v
         v                   |               |        +-----+----------+     +-------------------+
  +------+-----------+       |               |        |  Timeout       | N   |   Resend streams  |
  | Setup 1s timer   |       |               |        |  counter > 3   +---->+   Counter+=1      |
  | Save streams(wms)|       |               |        +------+---------+     |   Restart timer   |
  +------+-----------+       |               |               |               +-------------------+
         |                   |               |             Y |
         |                   |               |               v
         |                   |               |      +--------+-----------------+
         +-------------------+               |      |   Free the streams(wms)  |
         |                                   |      | Delete the agent from DB |
         |                                   |      |      Log the error       |
         v                                   |      +--------------------------+
                                             |
------------------------------------------------------------------------------------------------------
   ####################
   # receiving thread #
   ####################

 +-------------------+
 | 1905 ACK/Response |
 +-------------------+
           |
           |
           v
+-----------------------+
| Free the streams(wms) |
|    Stop the timer     |
+-----------------------+

-------------------------------------------------------------------------------------------------------
```

# How to enable Multi-AP in Realtek SDK

## enable ebtables support
Kernel config > Networking support > Networking options > Network packet filtering framework (Netfilter) > Ethernet Bridge tables (ebtables) support > **ebt: filter table support**

## enable inotify support
Kernel Config > File systems > Inotify support for userspace

## enable message queue
Kernel Config > General Setup > POSIX Message Queue

## enable Multi-AP support
Kernel Config > Device Drivers > Network device support > Wireless LAN > Multi-AP Support

<!-- ## disable WDS
Kernel Config > Device Drivers > Network device support > Wireless LAN > WDS Support

## enable A4 support
Kernel Config > Device Drivers > Network device support > Wireless LAN > RTL A4_STA Support

## enable 11k support
Kernel Config > Device Drivers > Network device support > Wireless LAN > IEEE 802.11k Support

## enable 11v support
Kernel Config > Device Drivers > Network device support > Wireless LAN > IEEE 802.11V Support(BSS Transition) -->

## enable openssl
users config > openssl-1.0.2d

## enable multi-ap
users config > Multi-AP

## enable log support
Busybox > System Logging Utilities > syslogd
Busybox > System Logging Utilities > logger

# Enable Test Related Features/Tools

## Enable sed
Busybox > Editors > sed

## Enable touch
Busybox > Coreutils > touch

## Enable telnetd
Busybox > Networking Utilities > telnetd
Then add `telnetd &` at the end of `users/script/cinit/init.sh`.

## Enable login
Busybox > Login/Password Management Utilities > login

## increase busybox FEATURE_EDITING_MAX_LEN (8192)
Busybox > Busybox Settings -> Busybox Library Tuning > Command line editing

# Multi-AP Setup

## Set Region to Global
```
flash set HW_WLAN0_REG_DOMAIN 14

flash set HW_WLAN1_REG_DOMAIN 14
```

## Enable 11k and 11v
```
flash set WLAN0_DOT11V_ENABLE 1 && flash set WLAN0_VAP0_DOT11V_ENABLE 1 && flash set WLAN0_VAP1_DOT11V_ENABLE 1 && flash set WLAN0_VAP2_DOT11V_ENABLE 1 && flash set WLAN0_VAP3_DOT11V_ENABLE 1 && flash set WLAN0_VAP4_DOT11V_ENABLE 1

flash set WLAN1_DOT11K_ENABLE 1 && flash set WLAN1_VAP0_DOT11K_ENABLE 1 && flash set WLAN1_VAP1_DOT11K_ENABLE 1 && flash set WLAN1_VAP2_DOT11K_ENABLE 1 && flash set WLAN1_VAP3_DOT11K_ENABLE 1 && flash set WLAN1_VAP4_DOT11K_ENABLE 1
```

## Controller Setup

1. Set device to controller with command `flash set MAP_CONTROLLER 1`.
2. Manually setup backhaul BSS with WPA2-Personal.
3. Enable va0 (VAP1) on wlan0 and set the security to WPA2.
4. Disable vxd on controller with `flash set REPEATER_ENABLED1 0` and `flash set REPEATER_ENABLED2 0`.

## Agent Setup
1. Enable vxd with `flash set REPEATER_ENABLED1 1 && flash set WLAN0_VAP4_WLAN_DISABLED 0` or `flash set REPEATER_ENABLED2 1 && flash set WLAN1_VAP4_WLAN_DISABLED 0` (wlan0 or wlan1).
2. Set vxd mode with `flash set WLAN0_VAP4_MODE 1 && flash set WLAN1_VAP4_MODE 1`.
3. Set device to agent with command `flash set MAP_CONTROLLER 2`.
4. Enable wscd support with `flash set WLAN0_VAP4_WSC_DISABLE 0` or `flash set WLAN1_VAP4_WSC_DISABLE 0`.

## Onboarding
Press push button on both registrar and enrollee. Or `touch /tmp/virtual_push_button`.

# ALME to Trigger CMDU

`hle_entity -a <alme_target_ip>:<port> -m ALME-MULTIAP-COMMAND.request <cmdu_target_al_mac> <cmdu_type> <tlv_payload_data_file_path>`

example:
`hle_entity -a 127.0.0.1:8888 -m ALME-MULTIAP-COMMAND.request 00:e0:4c:ff:02:c0 0x0002 /tmp/tlv`

tlv_payload data needs to contain at least 0000 (end of tlv) in order to be parsed, the data could be echoed with `echo -n -e \\x00\\x00 > /tmp/tlv`


## MAP Related MIBs

- **MAP_CONTROLLER** <br/>
0: disable <br/>
1: controller <br/>
2: agent
- **MAP_BSS_TYPE**  <br/>
Bit 7: Backhaul STA <br/>
Bit 6: Backhaul BSS <br/>
Bit 5: Fronthaul BSS <br/>
- **MAP_CONFIGURED_BAND** <br/>
Bit 0: 2.4G <br/>
Bit 1: 5G <br/>
Bit 5 -8: Store autoconfig state 0: configured, 1: M1 sent, not configured yet <br/>
