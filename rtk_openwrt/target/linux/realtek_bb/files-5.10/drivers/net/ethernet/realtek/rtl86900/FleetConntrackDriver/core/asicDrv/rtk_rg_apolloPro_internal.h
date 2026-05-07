#ifndef __RTK_RG_APOLLOPRO_INTERNAL_H__
#define __RTK_RG_APOLLOPRO_INTERNAL_H__

#include <common/rt_type.h>
#include <common/rt_error.h>
#if defined(CONFIG_RTL9607C_SERIES)
#include <rtk_rg_apolloPro_asicDriver.h>
#elif defined(CONFIG_RTL9603CVD_SERIES)
#include <rtk_rg_rtl9603cvd_asicDriver.h>
#endif

#if !defined(CONFIG_RTK_L34_XPON_PLATFORM) && defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_struct.h>
#endif
#include <rtk_rg_asic_tblSize.h>

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES) || defined(CONFIG_RG_G3_SERIES)

#ifndef SUCCESS
#define SUCCESS	0
#endif
#ifndef FAILED
#define FAILED	-1
#endif

#ifndef ASSERT
#define ASSERT(value)\
do {\
	if (!((int32)(value))) {\
		rtlglue_printf("\t%s(%d): Fail, so abort!\n", __FUNCTION__, __LINE__);\
		return FAILED; \
	}\
}while (0)
#endif


#ifndef IS_CLASSD_ADDR
#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)
#endif

#define IS_V6FLOWHASH_MCADDR(ipv6hashaddr)		((((uint32)(ipv6hashaddr)) & FLOW_V6HASHADDR_MC_BIT) == FLOW_V6HASHADDR_MC_BIT)



typedef struct rgip {
	/* replace bit field */
	uint8 ip_vhl;

	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	uint32 		ip_src,ip_dst;	/* source and dest address */
} rgip_t;


typedef struct rgudpHdr_s {
	uint16	uh_sport;		/* source port */
	uint16	uh_dport;		/* destination port */
	uint16	uh_ulen;		/* udp length */
	uint16	uh_sum;			/* udp checksum */
} rgudpHdr_t;

typedef enum rtk_rg_flowEntryValidBitScope_s
{
	FB_flowEtnryScope_SRAM,		// 4096
	FB_flowEtnryScope_TCAM,		// 64
	FB_flowEtnryScope_ALL,			// 4096+64
}rtk_rg_flowEntryValidBitScope_t;

typedef struct rtk_rg_fbDatabase_s
{
	/* Register status */
	uint8 fbMode;
	uint8 ddrBusAlign32;
	/***************/
	void *ddrMemAlloc;
	void *ddrMemBase;
#if defined(CONFIG_RG_FLOW_8K_MODE)
	int8 ddrMemBlock[(32<<13) + 1023];		// maximum occupied memory size: 32 byte align * 8K mode + 1023 (prepare for 1k aligned base address)
#elif defined(CONFIG_RG_FLOW_16K_MODE)
	int8 ddrMemBlock[(32<<14) + 1023];		// maximum occupied memory size: 32 byte align * 16K mode + 1023 (prepare for 1k aligned base address)
#elif defined(CONFIG_RG_FLOW_32K_MODE) || defined(CONFIG_APOLLOPRO_FPGA)
	int8 ddrMemBlock[(32<<15) + 1023];		// maximum occupied memory size: 32 byte align * 32K mode + 1023 (prepare for 1k aligned base address)
#endif
#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)	// only for testing
	uint8 ignoreFlowValidTableRst;				// ignore the reset action of SRAM flow entries
#endif
	uint32 flowEntryValidBits[(FLOWBASED_TABLESIZE_FLOWSRAM_ARYSIZE+FLOWBASED_TABLESIZE_FLOWTCAM_ARYSIZE)>>5];		// 4K+64 SRAM / 32bit = 130 dwords	

}rtk_rg_fbDatabase_t;

#define DDRMODE_HASHIDX2CACHEIDX(hashIdx)	(hashIdx&(FLOWBASED_TABLESIZE_FLOWTAG-1))
#define DDRMODE_HASHIDX2MSB(hashIdx)		((ASIC_CHIP_ID==RTL9607C_CHIP_ID) ? (hashIdx>>12) : (hashIdx>>10))


#if !defined(CONFIG_RG_G3_SERIES)
uint16 _rtk_rg_outerHdrIPv4Checksum(struct rgip *pip);
uint16 _rtk_rg_outerHdrUdpChecksum(struct rgip *pip, rgudpHdr_t *pudp, uint16 udpLen);
uint32 _rtk_rg_flowEntryValidBitIgnoreRst(rtk_enable_t state);
uint32 _rtk_rg_flowEntryValidBit_reset(rtk_rg_flowEntryValidBitScope_t vBitScope);
uint32 _rtk_rg_flowEntryValidBit_set(uint8 isValid, uint32 entryIdx);
uint32 _rtk_rg_flowEntryValidBit_get(uint32 entryIdx);
int32 _rtk_rg_flowEntryAvailableIdx_get(uint32 baseEntIdx);
int32 _rtk_rg_flowEntryConflictEgrCPUPortChecking(void *pFlowData, uint32 s1EntryIdx);
uint32 _rtk_rg_flowEntryWriteToDDR(uint32 entryIdx, void *pFlowData);
uint32 _rtk_rg_flowEntryReadFromDDR(uint32 entryIdx, void *pFlowData);
uint32 _rtk_rg_flowEntryNum_get(void);

uint32 _rtk_rg_flowHashPreProcessPort(uint16 port, uint32 cf_ptn);
uint32 _rtk_rg_flowHashPreProcessIP(uint32 ip, uint32 cf_ptn);
uint32 _rtk_rg_flowHashPath12ExtraItem_get(void *pFlowData);
uint32 _rtk_rg_flowHashPath34ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx_gmacChk);
uint32 _rtk_rg_flowHashPath5ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID);
uint32 _rtk_rg_flowHashPath6ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID);
uint32 _rtk_rg_flowHashIndexStep1_get(uint16 param1, uint16 param2, uint32 param3, uint32 param4, uint32 extraItem);
uint32 _rtk_rg_flowHashIndexStep2_get(uint32 step1Idx);
uint32 _rtk_rg_flowHashIPv6DstAddr_get(uint8 ipDes[16]);
uint32 _rtk_rg_flowHashIPv6SrcAddr_get(uint8 ipSrc[16]);
uint32 _rtk_rg_flowHashBuckets_get(void);

uint32 _rtk_rg_hashTrapBandwidthSharing_get(uint32 SPA, uint64 SMAC, uint64 DMAC, uint32 innerSIP, uint32 innerDIP, uint32 innerSrcPort, uint32 innerDstPort);

uint32 _rtk_rg_fbMode_set(rtk_rg_asic_fbMode_t fbMode);
uint32 _rtk_rg_fbMode_get(void);
uint32 _rtk_rg_init_rgProDB(void);
uint32 _rtk_rg_fb_hw_init(void);

#endif //CONFIG_RG_RTL9607C_SERIES

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
rtk_rg_err_code_t _rtk_rg_lut_ucLookupAction_init(void);
#endif
#endif
#endif //__RTK_RG_APOLLOPRO_INTERNAL_H__

