#ifndef _OMCI_DRV_MIC_H_
#define _OMCI_DRV_MIC_H_

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

unsigned int omci_gen_mic( unsigned char *omciIk, unsigned char dir, unsigned char *omciMsg, unsigned int omciMsgLen);

#endif //_OMCI_DRV_MIC_H_
