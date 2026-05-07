#ifndef __RTK_RG_ALG_FCC_H__
#define __RTK_RG_ALG_FCC_H__

#include <rtk_rg_alg_tool.h>

#define ALG_FCC_FMT_REQUEST		0x2
#define ALG_FCC_FMT_RESPONSE	0x3

#define ALG_FCC_TYPE_NORMAL		0x2
#define ALG_FCC_TYPE_REDIRECT	0x3

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_fcc(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);

#endif
