/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_PING_H_
#define _MAP_PING_H_
#include <stdio.h>
typedef enum {false = 0, true = 1} bool;


#define BUFSIZE					8192
#define	MAX_PING_TIME			500				//in 0.1ms
#define INTERNET_CHECK_INTERVAL	5		//in seconds

#ifdef CONFIG_RTK_PON_XDSL
#else
bool stp_blocked;
char gGatewayIp[255];
char PingCmd[128];
FILE *fp_cmd;
int InternetCheckTimerID;//0 invalid? from capwap
unsigned int gPingSpeed;
#endif // CONFIG_RTK_PON_XDSL

extern bool start_internet_check();
extern void internet_check_expire();

#endif
