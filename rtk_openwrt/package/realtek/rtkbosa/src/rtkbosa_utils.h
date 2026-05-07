/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 */
#ifndef _RTKBOSA_UTIL_H
#define _RTKBOSA_UTIL_H

#include <common/util/rt_util.h>
#include <osal/lib.h>
#include <rtk/i2c.h>
#include <rtk/ponmac.h>

#define dbg(f, a...) if (debug) printf("[rtkbosa]" f, ##a)

#define RTKBOSA_MSG(fmt,args...)  printf("rtkbosa: "fmt"\n", ##args)
#define RTKBOSA_INFO(fmt,args...)  printf("\033[1;33;46m""rtkbosa: "fmt"\033[m""\n", ##args)
#define RTKBOSA_ERROR(fmt,args...)  printf("\033[1;33;41m""rtkbosa: "fmt"\033[m""\n", ##args)

#define msleep(a)	usleep(a * 1000)

#ifndef TRANSCEIVER_A0
#define TRANSCEIVER_A0 0x50
#endif
#ifndef TRANSCEIVER_A2
#define TRANSCEIVER_A2 0x51
#endif

#define RTKBOSA_NONE			(0)

#define RTKBOSA_REALTEK_OTHER		(100)
#define RTKBOSA_REALTEK_RTL8290B	(101)

#define RTKBOSA_SEMTECH_OTHER		(200)
#define RTKBOSA_SEMTECH_GN25L95		(201)
#define RTKBOSA_SEMTECH_GN28L95		(202)
#define RTKBOSA_SEMTECH_GN28L96		(203)
#define RTKBOSA_SEMTECH_GN28L97		(204)

#define RTKBOSA_UXFASTIC_OTHER		(300)
#define RTKBOSA_UXFASTIC_UX3320		(301)
#define RTKBOSA_UXFASTIC_UX3360		(302)
#define RTKBOSA_UXFASTIC_UX3320S	(303)
#define RTKBOSA_UXFASTIC_UX3361		(304)

#define RTKBOSA_UNKNOWN			(999)

typedef struct ld_data_filed_s{
    uint32	address;
    uint8	table;
    uint32	byte;
    uint8	value;
    char	*description;
} ld_data_filed_t;

int32 rtkbosa_i2c_init();
int32 rtkbosa_byte_read(uint32 devID, uint32 addr, uint8 *data);
int32 rtkbosa_byte_write(uint32 devID, uint32 addr, uint8 data);
uint32 rtkbosa_vendor_name_pn_sn(int bosa);
uint32 rtkbosa_checksum(uint8 *addr, int32 len);

uint32 polynomialfit(int obs, int degree, double *dx, double *dy, double *store); /* n, p */
#if defined(CONFIG_RTK_LIB_MIB) || defined(CONFIG_USER_BOA_SRC_BOA)
void rtkbosa_print_rtk_pcb_id(void);
#endif
#endif	/* _RTKBOSA_UTIL_H */

