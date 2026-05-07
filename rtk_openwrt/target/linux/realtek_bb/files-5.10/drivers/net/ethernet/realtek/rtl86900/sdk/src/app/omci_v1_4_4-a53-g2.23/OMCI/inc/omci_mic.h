/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI MIB related info
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI MIC definition
 */

#ifndef __OMCI_MIC_H__
#define __OMCI_MIC_H__

#define OMCI_DEF_MIC_LEN    4   /* 4 octets */
#define OMCI_IK_LEN         16  /* 16 octets */
#define OMCI_ONU_REG_ID_LEN 36  /* 36 octets */
#define OMCI_MSK_LEN        16  /* 16 octets */
#define OMCI_PON_TAG_LEN    8   /* 8 octets */
#define OMCI_SK_STR_LEN     8   /* 8 octets */
#define OMCI_SN_LEN           8   /* 8 octets */
#define OMCI_SK_MSG_LEN     (OMCI_PON_TAG_LEN + OMCI_SN_LEN + OMCI_SK_STR_LEN)
#define OMCI_SK_LEN         16  /* 16 octets */
#define OMCI_IK_MSG_LEN     16  /* 16 octets */
#define OMCI_MIC_DIR_DS     0x01
#define OMCI_MIC_DIR_US     0x02


int omci_gen_omci_ik(unsigned char *msk, unsigned char *sn, unsigned char *ponTag, unsigned char *omci_ik);
void omci_mic_init(unsigned char *sn, unsigned char *gponPwd, unsigned char *ponTag, unsigned char *omciIk);
//void omci_update_omci_msk(unsigned char *pMsk);
//void omci_mic_build(unsigned char *pdu, unsigned int ext);
#endif /* __OMCI_MIC_H__ */

