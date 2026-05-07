
#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)
#include <rtl_glue.h>
#include <rtk_rg_apolloPro_fpga.h>
extern rtk_fb_debug_level_t debug_level;
#include <rtk_rg_internal.h>
#elif defined(CONFIG_RTK_L34_XPON_PLATFORM)
#include <rtk_fc_internal.h>
#include <rtk_fc_debug.h>

#if defined(CONFIG_RTK_L34_FLOWBASED_DYN_MODE)
#include <rtk_fc_struct.h>
#endif

#else
#include <rtk_rg_internal.h>
#endif
#include <rtk_rg_apolloPro_internal.h>

#if defined(CONFIG_RTL9603CVD_SERIES)
#include <dal/rtl9603cvd/dal_rtl9603cvd.h>
#endif

#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)
#error
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#error
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
// PASS
#endif

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
// to enable FPGA platform DRAM simulation
#if !defined(LINUX_KERNEL_SPI_IO) 
#define LINUX_KERNEL_SPI_IO
#endif
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
__SRAM_FWDENG_DATA rtk_rg_fbDatabase_t rgpro_db;
extern uint32 ASIC_CHIP_ID;
extern uint32 ASICDRIVERVER;


#define FLOWCMP_P1P2(pP1Data, pP2Data) \
	do{\
			if(pP1Data->valid != TRUE) break;\
			if(pP1Data->in_path != 0) break;\
			if(pP1Data->in_multiple_act != 0) break;\
			if(pP1Data->in_spa_check != pP2Data->in_spa) break;\
			if(pP1Data->in_ctagif != pP2Data->in_ctagif) break;\
			if(pP1Data->in_stagif != pP2Data->in_stagif) break;\
			if(pP1Data->in_pppoeif != pP2Data->in_pppoeif) break;\
			if(pP1Data->in_tos != pP2Data->in_tos) break;\
			if(pP1Data->in_protocol != pP2Data->in_protocol) break;\
			if(pP1Data->in_smac_lut_idx != pP2Data->in_smac_lut_idx) break;\
			if(pP1Data->in_dmac_lut_idx != pP2Data->in_dmac_lut_idx) break;\
			if(pP1Data->in_svlan_id != pP2Data->in_svlan_id) break;\
			if(pP1Data->in_cvlan_id != pP2Data->in_cvlan_id) break;\
			if(pP1Data->in_spa != pP2Data->in_spa) break;\
			if(pP1Data->in_ext_spa != pP2Data->in_ext_spa) break;\
			if(pP1Data->in_pppoe_sid != pP2Data->in_pppoe_sid) break;\
			if(pP1Data->in_pppoe_sid_check != pP2Data->in_pppoe_sid_check) break;\
			if(pP1Data->in_tos_check != pP2Data->in_tos_check) break;\
			if(pP1Data->in_cvlan_pri != pP2Data->in_cvlan_pri) break;\
			found = TRUE;\
	}while(0)

#define FLOWCMP_P3P4(pP3Data, pP4Data)\
	do{\
		if(pP3Data->valid != TRUE) break;\
		if(pP3Data->in_path != 1) break;\
		if(pP3Data->in_intf_idx != pP4Data->in_intf_idx) break;\
		if(pP3Data->in_multiple_act != 0) break;\
		if(pP3Data->in_ipv4_or_ipv6 != pP4Data->in_ipv4_or_ipv6) break;\
		if(pP3Data->in_ctagif != pP4Data->in_ctagif) break;\
		if(pP3Data->in_stagif != pP4Data->in_stagif) break;\
		if(pP3Data->in_pppoeif != pP4Data->in_pppoeif) break;\
		if(pP3Data->in_tos != pP4Data->in_tos) break;\
		if(pP3Data->in_src_ipv4_addr != pP4Data->in_src_ipv4_addr) break;\
		if(pP3Data->in_dst_ipv4_addr != pP4Data->in_dst_ipv4_addr) break;\
		if(pP3Data->in_l4_src_port != pP4Data->in_l4_src_port) break;\
		if(pP3Data->in_l4_dst_port != pP4Data->in_l4_dst_port) break;\
		if(pP3Data->in_pppoe_sid_check != pP4Data->in_pppoe_sid_check) break;\
		if(pP3Data->in_l4proto != pP4Data->in_l4proto) break;\
		if(pP3Data->in_tos_check != pP4Data->in_tos_check) break;\
		if(pP3Data->in_cvlan_pri != pP3Data->in_cvlan_pri) break;\
		found = TRUE;\
	}while(0)

#if defined(CONFIG_APOLLOPRO_FPGA) || defined(CONFIG_APOLLO_FPGA_PHY_TEST)

#if !defined(CONFIG_APOLLOPRO_ASICMODE)	// CONFIG_APOLLOPRO_ASICMODE was defined if run on ApolloPro ASIC
#include <osal/spl.h>
#include <ioal/io_spi.h>
#include <dal/rtl9607c/dal_rtl9607c.h>

extern osal_spinlock_t mii_spinlock;
#endif

#define FPGA_PONSID_ADDR 12
#define FPGA_DRAMWR_ADDR 14
#define FPGA_DRAMRD_ADDR 15

int io_spi_ponDsSID_write(uint8 ponSID)
{
#if !defined(CONFIG_APOLLOPRO_ASICMODE)
	// write PON SID rx register(R/W) first before virtual mac input to simulate transmitting PON STREAM IDX
	io_spi_phyReg_write(FPGA_PONSID_ADDR, 6, ponSID&0x7f);
	DEBUG("[PON SID] write DS SID: 0x%x", ponSID&0x7f);
#endif
	return RT_ERR_RG_OK;
}

int io_spi_ponUsSID_read(uint8 *ponSID)
{
#if !defined(CONFIG_APOLLOPRO_ASICMODE)
	// read PON SID tx register(R) first before virtual mac output to simulate receiving PON STREAM IDX
	unsigned short value;

	io_spi_phyReg_read(FPGA_PONSID_ADDR, 7, &value);
	DEBUG("[PON SID] read US SID: 0x%x", value);
	*ponSID = value&0x7f;
#endif	
	return RT_ERR_RG_OK;
}

int io_spi_ponUsSID_ctrl(rtk_rg_enable_t status)
{
	if(status == RTK_RG_ENABLED)
	{
		// tun on following register to enable the latch function.  (save phy12reg8 to phy12reg7)
		// so that we could read phy12reg7 after reading egress packet.
		//Reg set 0x248 0x50000
		//Reg set 0x24c 0x900
		//ioal_mem32_write(0x264, 0x50000);
		//ioal_mem32_write(0x268, 0x900);
#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTL9603CVD_SERIES)
		if(ASIC_CHIP_ID==RTL9607C_CHIP_ID)
		{
			rtk_rg_asic_reg_write(RTL9607C_DEBUG_SELr, 0x50000);
			rtk_rg_asic_reg_write(RTL9607C_DEBUG_SEL_TOPr, 0x900);
		}
		else
		{
			rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SELr, 0x40000);
			rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SEL_TOPr, 0x900);
		}
#elif defined(CONFIG_RTL9607C_SERIES)
		rtk_rg_asic_reg_write(RTL9607C_DEBUG_SELr, 0x50000);
		rtk_rg_asic_reg_write(RTL9607C_DEBUG_SEL_TOPr, 0x900);
#elif defined(CONFIG_RTL9603CVD_SERIES)
		rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SELr, 0x40000);
		rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SEL_TOPr, 0x900);
#endif
	}
	else
	{
#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTL9603CVD_SERIES)
		if(ASIC_CHIP_ID==RTL9607C_CHIP_ID)
		{
			rtk_rg_asic_reg_write(RTL9607C_DEBUG_SELr, 0x0);
			rtk_rg_asic_reg_write(RTL9607C_DEBUG_SEL_TOPr, 0x0);
		}
		else
		{
			rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SELr, 0x0);
			rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SEL_TOPr, 0x0);
		}
#elif defined(CONFIG_RTL9607C_SERIES)
		rtk_rg_asic_reg_write(RTL9607C_DEBUG_SELr, 0x0);
		rtk_rg_asic_reg_write(RTL9607C_DEBUG_SEL_TOPr, 0x0);
#elif defined(CONFIG_RTL9603CVD_SERIES)
		rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SELr, 0x0);
		rtk_rg_asic_reg_write(RTL9603CVD_DEBUG_SEL_TOPr, 0x0);
#endif
	}

	return RT_ERR_RG_OK;
}

int io_spi_dramResponseState_ctrl(rtk_rg_enable_t state)
{
#if defined(LINUX_KERNEL_SPI_IO) && !defined(CONFIG_APOLLOPRO_ASICMODE)
	uint16 tmpVal = 0;
	
	io_spi_phyReg_read(FPGA_DRAMWR_ADDR, 17, &tmpVal);
		
	if(state == RTK_RG_ENABLED){
		// enable dram response (dafault state)
		tmpVal &=  ~0x8;
		io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, tmpVal);
	}else{
		// disable dram response
		tmpVal |=  0x8;
		io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, tmpVal);
	}
#else
//#error_compiler_flag	// for FPGA test, we need LINUX_KERNEL_SPI_IO to turn on spi io
#endif
	return RT_ERR_RG_OK;
}

int io_spi_dramFlowEntry_write(uint32 memaddr, void *data)
{
#if defined(LINUX_KERNEL_SPI_IO) && !defined(CONFIG_APOLLOPRO_ASICMODE)
	unsigned short value;
	uint32 i;
	uint32 *pData = (uint32 *)data;

	osal_spl_spin_lock(&mii_spinlock);
	
	/*wr_data reg: reg[0]: flowdata[15:0]...reg[15]: flowdata[255:240]*/
	for(i = 0; i < (sizeof(rtk_rg_asic_path1_entry_t)>>1); i++)	// write word 0~6 to reg0~reg13
	{
		if(i%2 == 0)
			value = (unsigned short)(pData[i/2]&0xffff);
		else
			value = (unsigned short)((pData[i/2]>>16)&0xffff);

		// IO_MII_PHYREG_WRITE
		io_spi_phyReg_write(FPGA_DRAMWR_ADDR, i, value);
	}
	for( ; i < 16; i++)										// reg14~reg15
	{
		value = 0;
		io_spi_phyReg_write(FPGA_DRAMWR_ADDR, i, value);
	}

	/*address reg: reg[16]*/		
	value = (unsigned short)(memaddr & 0x00007FFF);			// [14:0], [15]:reserved
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 16, value);

	/*cmd reg: set reg[17] to 0x0003 trigger write procedure*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0003);
	
	/* Wait operation completed */
	do
	{
		io_spi_phyReg_read(FPGA_DRAMRD_ADDR, 16, &value);
	}while (value&0x1);
	
	/*cmd reg: set reg[17] to 0x000 clear previous access*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0000);

	osal_spl_spin_unlock(&mii_spinlock);
#else
//#error_compiler_flag	// for FPGA test, we need LINUX_KERNEL_SPI_IO to turn on spi io
#endif
	return RT_ERR_RG_OK;
}

uint32 io_spi_dramFlowEntry_read(uint32 memaddr, void *data)
{
#if defined(LINUX_KERNEL_SPI_IO) && !defined(CONFIG_APOLLOPRO_ASICMODE)
	unsigned short value;
	uint32 i;
	uint32 *pData = (uint32 *)data;

	osal_spl_spin_lock(&mii_spinlock);

	/*address reg: reg[16]*/		
	value = (unsigned short)(memaddr & 0x00007FFF);			// [14:0], [15]:reserved
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 16, value);

	/*cmd reg: set reg[17] to 0x0002 trigger read procedure*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0002);
	
	/* Wait operation completed */
	do
	{
		io_spi_phyReg_read(FPGA_DRAMRD_ADDR, 16, &value);
	}while (value&0x1);
	
	/*rd_data reg: reg[0]: flowdata[15:0]...reg[15]: flowdata[255:240]*/
	for(i = 0; i < (sizeof(rtk_rg_asic_path1_entry_t)>>1); i++)	// reg0~reg13
	{		
		io_spi_phyReg_read(FPGA_DRAMRD_ADDR, i, &value);
		if(i%2 == 0)
		{
			pData[i/2] &=0xffff0000;
			pData[i/2] |= value;
		}else
		{
			pData[i/2] &=0x0000ffff;
			pData[i/2] |= value<<16;
		}
		
	}
	
	/*cmd reg: set reg[17] to 0x000 disable read execute*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0000);
	
	osal_spl_spin_unlock(&mii_spinlock);
#endif
	return RT_ERR_RG_OK;
}

int io_spi_dramFlowEntry_reset(uint32 memaddr)
{
#if defined(LINUX_KERNEL_SPI_IO) && !defined(CONFIG_APOLLOPRO_ASICMODE)
	unsigned short value;

	osal_spl_spin_lock(&mii_spinlock);


	/*address reg: reg[16]*/		
	value = (unsigned short)(memaddr & 0x00007FFF);			// [14:0], [15]:reserved
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 16, value);

	/*cmd reg: set reg[17] to 0x0004 trigger reset procedure*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0004);
	
	/* Wait operation completed */
	do
	{
		io_spi_phyReg_read(FPGA_DRAMRD_ADDR, 16, &value);
	}while (value&0x1);
	
	/*cmd reg: set reg[17] to 0x000 clear previous access*/
	io_spi_phyReg_write(FPGA_DRAMWR_ADDR, 17, 0x0000);

	osal_spl_spin_unlock(&mii_spinlock);
#endif
	return RT_ERR_RG_OK;
}

#endif //CONFIG_APOLLOPRO_FPGA


#include <ioal/mem32.h>

static void _rtk_rg_clearDDRMemBlock(void)
{
#if defined(CONFIG_RTK_L34_FLOWBASED_DYN_MODE)
	int entryNum = _rtk_rg_flowEntryNum_get();
	memset(rgpro_db.ddrMemBase, 0, entryNum<<5);	// 32 bytes per one flow entry.

#elif !defined(CONFIG_RG_FLOW_4K_MODE)

#if defined(CONFIG_APOLLOPRO_FPGA)/* || defined(CONFIG_APOLLO_FPGA_PHY_TEST) */ /// to reduce RG init time cost (_rtk_rg_flow_clear)
#if !defined(CONFIG_APOLLOPRO_ASICMODE)
	int entryNum = _rtk_rg_flowEntryNum_get();
	int i = 0;
	for(i = 0; i < entryNum; i++)
	{
		io_spi_dramFlowEntry_reset(i);
	}
#endif	
#endif

	memset(&rgpro_db.ddrMemBlock, 0, sizeof(rgpro_db.ddrMemBlock));

#endif// support dram mode
}

static void *_rtk_rg_memoryAlignAlloc(uint32 alignedSize, uint32 size)
{
	void *ptr = NULL;	
	
#if defined(CONFIG_RTK_L34_FLOWBASED_DYN_MODE)
	uint32 mask = ~(alignedSize -1);

	rgpro_db.ddrMemAlloc = &fc_db.flowEntryDataPool;		// get from static data buffer of cache driver
	
	ptr = (void *) (((long)rgpro_db.ddrMemAlloc + (alignedSize - 1)) & mask);

#elif !defined(CONFIG_RG_FLOW_4K_MODE)
	uint32 mask = ~(alignedSize -1);

	rgpro_db.ddrMemAlloc = &rgpro_db.ddrMemBlock;

	TRACE("ddr memory block @ %p", &rgpro_db.ddrMemBlock);
	ptr = (void *) (((long)rgpro_db.ddrMemAlloc + (alignedSize - 1)) & mask);

#endif// support DRAM mode
	return ptr;
}

static uint16 _rtk_rg_cksum(uint16 *ptr, uint32 len, uint16 resid) 
{
	uint32 csum = resid;
	uint32 odd = 0;
 	if(len & 1) odd = 1;
	len = len >> 1;
 	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);
 
	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;
 
	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);
 
	//if(csum == 0xffff)	csum = 0;		//FIXME:luke
 
	return((uint16)csum);
}

uint16 _rtk_rg_outerHdrIPv4Checksum(struct rgip *pip)
{
	uint16 *ptr = (uint16 *)pip;
	uint32 len = ((pip->ip_vhl & 0xf) << 2);	
	uint32 csum = 0;
	uint32 odd = 0;
	uint16 orgcsum=0;

	orgcsum=pip->ip_sum;
 	pip->ip_sum = 0;

	if(len & 1) odd = 1;
	len = len >> 1;
	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);

	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;

	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	//if(csum == 0xffff)	csum = 0;		//FIXME:luke
 	pip->ip_sum=orgcsum;
	return((uint16)htons(~csum));
}

uint16 _rtk_rg_outerHdrUdpChecksum(struct rgip *pip, rgudpHdr_t *pudp, uint16 udpLen)
{
	uint32 len = 0;//ntohs(pip->ip_len) - ((pip->ip_vhl & 0xf) << 2);	
	uint32 csum = 0;
	uint16 tmp[2] = {0};
	uint16 orgcsum=0;

	csum = _rtk_rg_cksum((uint16 *)&pip->ip_src, 8, 0);
	tmp[0] = htons(17);	// 0x11: UDP
	csum = _rtk_rg_cksum((uint16 *)tmp, 2, csum);
	tmp[0] = htons(len);
	csum = _rtk_rg_cksum((uint16 *)tmp, 2, csum);
	orgcsum = pudp->uh_sum;
	pudp->uh_sum = 0;
	csum = _rtk_rg_cksum((uint16 *)pudp, udpLen, csum);

	if(csum == 0xffff)	csum = 0;		//FIXME:luke

	pudp->uh_sum = orgcsum;

	return htons(~csum);
}



uint32 _rtk_rg_flowEntryValidBitIgnoreRst(rtk_enable_t state)
{
	if((state != DISABLED) && (state != ENABLED))
		return FAIL;

#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)	// only for testing
	rgpro_db.ignoreFlowValidTableRst = state;
#endif

	return SUCCESS;
}

uint32 _rtk_rg_flowEntryValidBit_reset(rtk_rg_flowEntryValidBitScope_t vBitScope)
{
	if(rgpro_db.fbMode == FB_MODE_4K)
	{
		switch(vBitScope)
		{
			case FB_flowEtnryScope_SRAM:
#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)	// only for testing
				if(!rgpro_db.ignoreFlowValidTableRst)
#endif
				{
					memset(&rgpro_db.flowEntryValidBits[0], 0, (FLOWBASED_TABLESIZE_FLOWSRAM>>5)<<2);
				}	
				break;
			case FB_flowEtnryScope_TCAM:	
				if(FLOWBASED_TABLESIZE_FLOWTCAM<32)
				{
					memset(&rgpro_db.flowEntryValidBits[FLOWBASED_TABLESIZE_FLOWSRAM>>5], 0, 1<<2);
				}	
				else
				{
					memset(&rgpro_db.flowEntryValidBits[FLOWBASED_TABLESIZE_FLOWSRAM>>5], 0, (FLOWBASED_TABLESIZE_FLOWTCAM>>5)<<2);
				}
				break;
			case FB_flowEtnryScope_ALL:
#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)	// only for testing
				if(rgpro_db.ignoreFlowValidTableRst)
				{
					if(FLOWBASED_TABLESIZE_FLOWTCAM<32)
					{
						memset(&rgpro_db.flowEntryValidBits[FLOWBASED_TABLESIZE_FLOWSRAM>>5], 0, 1<<2);
					}
					else
					{
						memset(&rgpro_db.flowEntryValidBits[FLOWBASED_TABLESIZE_FLOWSRAM>>5], 0, (FLOWBASED_TABLESIZE_FLOWTCAM>>5)<<2);
					}

				}	
				else
#endif
				{
					memset(&rgpro_db.flowEntryValidBits[0], 0, sizeof(rgpro_db.flowEntryValidBits));
				}
				break;
		}
	}
	else // not 4K mode
	{	
		// synchronize reset ddr memory block
		_rtk_rg_clearDDRMemBlock();
	}

	return SUCCESS;
}

uint32 _rtk_rg_flowEntryValidBit_set(uint8 isValid, uint32 entryIdx)
{
	// Support set 4K mode SRAM(4K+64) valid bit, maintain array in main memory
	uint32 validBits = 0;
	ASSERT(entryIdx < (FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWTCAM));

	validBits = rgpro_db.flowEntryValidBits[entryIdx>>5];	
	validBits &= ~(1 << (entryIdx&0x1f));
	validBits |= (isValid << (entryIdx&0x1f));
	rgpro_db.flowEntryValidBits[entryIdx>>5] =  validBits;
	
	return SUCCESS;
}

uint32 _rtk_rg_flowEntryValidBit_get(uint32 entryIdx)
{
	// Support get 
	// 	1. 4K mode SRAM(4K+64) valid bit
	//	2. DDR mode DDR(max: 32K) valid bit
	if(rgpro_db.fbMode == FB_MODE_4K)
	{
		ASSERT(entryIdx < (FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWTCAM));
		
		return (rgpro_db.flowEntryValidBits[entryIdx>>5]>>(entryIdx&0x1f)) & 0x1;
	}else
	{
		rtk_rg_asic_path1_entry_t flowData;
		bzero(&flowData, sizeof(flowData));
		ASSERT(entryIdx < _rtk_rg_flowEntryNum_get());
		
		ASSERT_EQ(rtk_rg_asic_flowPath1_get(entryIdx, &flowData), SUCCESS);
		
		return flowData.valid;
		
	}
}

int32 _rtk_rg_flowEntryAvailableIdx_get(u32 baseEntIdx)
{
	u8 i=0, found=FALSE;
	u8 isValid = FALSE;
	u8 way;
	rtk_rg_asic_fbMode_t fbMode;
	rtk_rg_asic_path1_entry_t flowData;

    	//ASSERT_EQ(rtk_rg_asic_fbModeCtrl_get(FB_MODE_FB_MOD, &mode), RT_ERR_RG_OK);
    	fbMode = rgpro_db.fbMode;
	if(fbMode == FB_MODE_4K) way = 4;
	else way = 1;
	
	for(i=0; i<way; i++){
		// driver need to handle valid/invalid entry in local memory
		if(fbMode == FB_MODE_4K){
			//SRAM mode: get valid information from valid table in memory.
			isValid = _rtk_rg_flowEntryValidBit_get(baseEntIdx);
		}else{
			//DDR mode: get valid information from flow entry in memory.
			ASSERT_EQ(rtk_rg_asic_flowPath1_get(baseEntIdx, &flowData), SUCCESS);
			isValid = flowData.valid;
		}
		//DEBUG("entry idx = %d, valid value = %d", baseEntIdx, isValid);
		if(isValid == FALSE){
			found = TRUE;
			break;
		}
		baseEntIdx++;
	}

	if(!found){
		if((fbMode == FB_MODE_4K)){
			// search TCAM SRAM
			baseEntIdx = FLOWBASED_TABLESIZE_FLOWSRAM;
			for(i=0; i<FLOWBASED_TABLESIZE_FLOWTCAM; i++){
				isValid = _rtk_rg_flowEntryValidBit_get(baseEntIdx);
				if(isValid == 0){
					found = TRUE;
					break;
				}
				baseEntIdx++;
			}
		}
	}
	DEBUG("Search avaliable entry: %s (%d)", found?"Found":"Not found", baseEntIdx);
	if(!found)
		return FAILED;
	else
		return baseEntIdx;
}

/*
 * Return:
 *      SUCCESS: No conflict egress port was configured.
 *      FAILED: find conflict flow entry, delete it.
 */
int32 _rtk_rg_flowEntryConflictEgrCPUPortChecking(void *pFlowData, uint32 s1EntryIdx)
{
#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE) && !defined(RTK_RG_ALL_MAC_CPU_PORTMASK) 
	#define RTK_RG_ALL_MAC_CPU_PORTMASK RTK_FC_ALL_MAC_CPU_PORTMASK
#endif
	rtk_rg_asic_path1_entry_t *pP1Data = NULL;
	rtk_rg_asic_path2_entry_t *pP2Data = NULL;
	rtk_rg_asic_path3_entry_t *pP3Data = NULL;
	rtk_rg_asic_path4_entry_t *pP4Data = NULL;
	rtk_rg_asic_path3_entry_t flowData;
	rtk_rg_asic_extPortMask_entry_t s1ExtPMask, s2ExtPMask;
	u8 way = 0, i = 0, found = FALSE, conflict = FALSE;
	uint32 cpupmask = RTK_RG_ALL_MAC_CPU_PORTMASK;

	if(pFlowData == NULL)
		return SUCCESS;
	
	pP2Data = (rtk_rg_asic_path2_entry_t *)pFlowData;
	if((pP2Data->in_path == 0) && (pP2Data->in_multiple_act == 1))
	{
		DEBUG("Conflict checking for path2");
		if(((pP2Data->out_portmask & cpupmask)==0) && (pP2Data->out_ext_portmask_idx==0))
			return SUCCESS;
		pP2Data = (rtk_rg_asic_path2_entry_t *)pFlowData;
		pP4Data = NULL;
		pP1Data = (rtk_rg_asic_path1_entry_t *)&flowData;
	}else if((pP2Data->in_path == 1) && (pP2Data->in_multiple_act == 1))
	{
		DEBUG("Conflict checking for path4");
		if(((pP2Data->out_portmask & cpupmask)==0) && (pP2Data->out_ext_portmask_idx==0))
			return SUCCESS;
		pP2Data = NULL;
		pP4Data = (rtk_rg_asic_path4_entry_t *)pFlowData;
		pP3Data = (rtk_rg_asic_path3_entry_t *)&flowData;
	}else
	{
		DEBUG("Conflict checking for unsupported flow entry");
		pP2Data = NULL;
		pP4Data = NULL;
		return SUCCESS;
	}

	// search step 1 entry in SRAM
	if(rgpro_db.fbMode == FB_MODE_4K) way = 4;
	else way = 1;

	DEBUG("Conflict checking, search step 1 entry in #%d", s1EntryIdx);
	
	for(i=0; i<way; i++, s1EntryIdx++){
		
		if(_rtk_rg_flowEntryValidBit_get(s1EntryIdx) == 0)	continue;
		
		if(pP2Data)
		{	
			ASSERT_EQ(rtk_rg_asic_flowPath1_get(s1EntryIdx, pP1Data), SUCCESS);

			FLOWCMP_P1P2(pP1Data, pP2Data);
			
			if (found == TRUE)
			{
				if((pP1Data->out_portmask & pP2Data->out_portmask)!=0)
					conflict = TRUE;
				else{
					//do extension port conflict checking.
					ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP1Data->out_ext_portmask_idx, &s1ExtPMask), SUCCESS);
					ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP2Data->out_ext_portmask_idx, &s2ExtPMask), SUCCESS);
					if((s1ExtPMask.extpmask & s2ExtPMask.extpmask)!=0)
						conflict = TRUE;
				}
				
				break;	// find target, break the searching no matter the multiple actions is conflict or not. 
			}
		}else if(pP4Data)
		{
			ASSERT_EQ(rtk_rg_asic_flowPath3_get(s1EntryIdx, pP3Data), SUCCESS);
			
			FLOWCMP_P3P4(pP3Data, pP4Data);
			
			if (found == TRUE)
			{
				if((pP3Data->out_portmask & pP4Data->out_portmask)!=0)
					conflict = TRUE;
				else{
					//do extension port conflict checking.
					ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP3Data->out_ext_portmask_idx, &s1ExtPMask), SUCCESS);
					ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP4Data->out_ext_portmask_idx, &s2ExtPMask), SUCCESS);
					if((s1ExtPMask.extpmask & s2ExtPMask.extpmask)!=0)
						conflict = TRUE;
				}
				
				break;	// find target, break the searching no matter the multiple actions is conflict or not. 
			}
		}
		
	}

	// search step 1 entry in TCAM
	if(!found){
		if((rgpro_db.fbMode == FB_MODE_4K)){
			// search TCAM SRAM
			s1EntryIdx = FLOWBASED_TABLESIZE_FLOWSRAM;
			for(i=0; i<FLOWBASED_TABLESIZE_FLOWTCAM; i++, s1EntryIdx++){
				if(_rtk_rg_flowEntryValidBit_get(s1EntryIdx) == 0)	continue;

				if(pP2Data)
				{	
					ASSERT_EQ(rtk_rg_asic_flowPath1_get(s1EntryIdx, pP1Data), SUCCESS);

					FLOWCMP_P1P2(pP1Data, pP2Data);
							
					if (found == TRUE)
					{
						if((pP1Data->out_portmask & pP2Data->out_portmask)!=0)
							conflict = TRUE;
						else{
							//do extension port conflict checking.
							ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP1Data->out_ext_portmask_idx, &s1ExtPMask), SUCCESS);
							ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP2Data->out_ext_portmask_idx, &s2ExtPMask), SUCCESS);
							if((s1ExtPMask.extpmask & s2ExtPMask.extpmask)!=0)
								conflict = TRUE;
						}
						
						break;	// find target, break the searching no matter the multiple actions is conflict or not. 
					}
				}else if(pP4Data)
				{
					ASSERT_EQ(rtk_rg_asic_flowPath3_get(s1EntryIdx, pP3Data), SUCCESS);
					
					FLOWCMP_P3P4(pP3Data, pP4Data);
					
					if (found == TRUE)
					{
						if((pP3Data->out_portmask & pP4Data->out_portmask)!=0)
							conflict = TRUE;
						else{
							//do extension port conflict checking.
							ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP3Data->out_ext_portmask_idx, &s1ExtPMask), SUCCESS);
							ASSERT_EQ(rtk_rg_asic_extPortMaskTable_get(pP4Data->out_ext_portmask_idx, &s2ExtPMask), SUCCESS);
							if((s1ExtPMask.extpmask & s2ExtPMask.extpmask)!=0)
								conflict = TRUE;
						}
						
						break;	// find target, break the searching no matter the multiple actions is conflict or not. 
					}
				}
				
			}
		}
	}


	if(conflict)
	{
		DEBUG("Find multiple egress action to same CPU port, delete step1 flow[%d] and handle flow by sw", s1EntryIdx);
		ASSERT_EQ(rtk_rg_asic_flowPath_del(s1EntryIdx), SUCCESS);
		return FAILED;		// Maintain flow step 1 & step 2 by SW.
	}else
		return SUCCESS;		// Set Flow step 2 to flow table then.
	
}

uint32 _rtk_rg_flowEntryWriteToDDR(uint32 entryIdx, void *pFlowData)
{	
	// Write flow entry to DRAM 
	// Note: reg.FBA32==0, memory align 4 bytes. (Now 28 bytes per entry)
	//		reg.FBA32==1, memory align 32 bytes.
	int memOffset = (entryIdx << 5);

	if(rgpro_db.ddrBusAlign32==DISABLED)
		memOffset -= (entryIdx << 2);		//entryIdx * sizeof(rtk_rg_asic_path1_entry_t)
	
	if(rgpro_db.ddrMemBase==NULL){
		WARNING("DDR mode memory block was not initialized yet.");
		return FAILED;
	}

	memcpy(rgpro_db.ddrMemBase+memOffset, pFlowData, sizeof(rtk_rg_asic_path1_entry_t));

	
#if defined(CONFIG_APOLLOPRO_FPGA) || defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#if !defined(CONFIG_APOLLOPRO_ASICMODE) // skip fake dram access if build model case in ASIC
	// Write flow entry to SRAM (fake DRAM) for FPGA HW access
	io_spi_dramFlowEntry_write(entryIdx, pFlowData);
	return SUCCESS;
#endif
#endif

	
	dma_cache_wback_inv((unsigned long)rgpro_db.ddrMemBase+memOffset, sizeof(rtk_rg_asic_path1_entry_t));

	if((rgpro_db.fbMode == FB_MODE_8K) || (rgpro_db.fbMode == FB_MODE_16K))
	{
		// if FB_CC_GLOBAL_FLOW_VALID_EN was turned on, request cache controller to update valid table
		ASSERT_EQ(rtk_rg_asic_ccAddFlow_cmd(DISABLED, DISABLED, ENABLED, DISABLED, DISABLED, entryIdx, NULL), SUCCESS);
	}
	
	if(rgpro_db.fbMode != FB_MODE_4K)
	{	
		// if delete dram flow entry, delete cached entry and valid bit
		// if update dram flow entry, delete cached entry only
		rtk_rg_asic_path1_entry_t *pFlow = (rtk_rg_asic_path1_entry_t *)pFlowData;
		if(pFlow->valid==0)
			rtk_rg_asic_ccInvalidFlow_cmd(ENABLED, entryIdx);
		else
			rtk_rg_asic_ccInvalidFlow_cmd(DISABLED, entryIdx);

	}

	return SUCCESS;
}

uint32 _rtk_rg_flowEntryReadFromDDR(uint32 entryIdx, void *pFlowData)
{
	// Note: reg.FBA32==0, memory align 4 bytes. (Now 28 bytes per entry)
	//		reg.FBA32==1, memory align 32 bytes.
	int memOffset = (entryIdx << 5);		//entryIdx * sizeof(rtk_rg_asic_path1_entry_t)

 // since write function write flow data to both simulated DRAM and main memory, read function could get flow from main memory directly 	
#if defined(CONFIG_APOLLOPRO_FPGA) || defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#if !defined(CONFIG_APOLLOPRO_ASICMODE) // skip fake dram access if build model case in ASIC
	// Read actual flow data from SRAM (fake DRAM)
	io_spi_dramFlowEntry_read(entryIdx, pFlowData);
	return SUCCESS;
#endif	
#endif

	if(rgpro_db.ddrBusAlign32==DISABLED)
		memOffset  -= (entryIdx << 2);
	
	if(rgpro_db.ddrMemBase==NULL){
		WARNING("DDR mode memory block was not initialized yet.");
		return FAILED;
	}
	
	memcpy(pFlowData, rgpro_db.ddrMemBase+memOffset, sizeof(rtk_rg_asic_path1_entry_t));

	return SUCCESS;
}

uint32 _rtk_rg_flowEntryNum_get(void)
{	
	rtk_rg_asic_fbMode_t fbMode = FB_MODE_4K;

	//rtk_rg_asic_fbModeCtrl_get(FB_MODE_FB_MOD, &fbMode);
	fbMode = rgpro_db.fbMode;
	
	//  4K: 2<<1 * 1024 entries
	//  8K: 2<<2 * 1024 entries
	// 16K: 2<<3 * 1024 entries
	// 32K: 2<<4 * 1024 entries
	if(fbMode==FB_MODE_4K)
		return FLOWBASED_TABLESIZE_FLOWSRAM;
	else
		return ((2 << (fbMode + 1)) << 10);
}

uint32 _rtk_rg_flowHashPreProcessPort(uint16 port, uint32 cf_ptn){

	uint32 shiftdir = (cf_ptn >> 19) & 0x1;
	uint32 shiftbits = (cf_ptn >> 16) & 0x7;
	uint32 xoroperatedvalue = cf_ptn & 0xffff;
	uint32 port_tmp = 0;
	//DEBUG("Hash - preprocess: shift dir: %d, bits %d, xor value = 0x%x", shiftdir, shiftbits, xoroperatedvalue);
	if(shiftdir){	// shift left
		port_tmp = ((port << shiftbits) & 0xffff) | (port >> (16 - shiftbits));
	}else{		// shift right
		port_tmp = ((port << (16 - shiftbits)) & 0xffff) | (port >> shiftbits);
	}
	//DEBUG("Hash - preprocess: 0x%x ^ 0x%x = 0x%x", port_tmp, xoroperatedvalue, (port_tmp ^ xoroperatedvalue));
	return (port_tmp ^ xoroperatedvalue) & 0xffff;	// sport, dport: (16 bits)

}

uint32 _rtk_rg_flowHashPreProcessIP(uint32 ip, uint32 cf_ptn){
	uint32 shiftdir = (cf_ptn >> 23) & 0x1;
	uint32 shiftbits = (cf_ptn >> 20) & 0x7;
	uint32 xoroperatedvalue = cf_ptn & 0xfffff;
	uint32 msb_12bits = ip & 0xfff00000;
	uint32 lsb_20bits = ip & 0x000fffff;
	uint32 ip_tmp = 0;
	ip &= 0xfffff;
	//DEBUG("Hash - preprocess: shift dir: %d, bits %d, xor value = 0x%x", shiftdir, shiftbits, xoroperatedvalue);
	if(shiftdir){	// shift left
		ip_tmp = ((lsb_20bits << shiftbits) & 0xfffff) | (lsb_20bits>> (20 - shiftbits));
	}else{		// shift right
		ip_tmp = ((lsb_20bits << (20 - shiftbits)) & 0xfffff) | (lsb_20bits>> shiftbits);
	}
	//DEBUG("Hash - preprocess: 0x%x | (0x%x ^ 0x%x) = 0x%x", msb_12bits, ip_tmp, xoroperatedvalue, msb_12bits | (ip_tmp ^ xoroperatedvalue));
	return (msb_12bits |( (ip_tmp ^ xoroperatedvalue) & 0xfffff));	// sip, dip: (msb12bits + 20 bits)

}

uint32 _rtk_rg_flowHashPath12ExtraItem_get(void *pFlowData)
{
	u32 extraItem = 0;
	rtk_rg_asic_path1_entry_t *pP1Data = pFlowData;
	rtk_enable_t enabled = DISABLED;
	uint8 igrCPRI = 0, igrDSCP = 0;
	
	if(pP1Data==NULL)
	{
		WARNING("flow data is NULL");
		return 0;
	}

	igrCPRI = pP1Data->in_cvlan_pri;
	igrDSCP = pP1Data->in_tos >> 2;

#if defined(CONFIG_RTL9607C_SERIES)
	if(ASIC_CHIP_ID==RTL9607C_CHIP_ID && ASICDRIVERVER==CHIP_REV_ID_A)
	{
		extraItem = 0x0;
	}else
#endif
	{
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATHALL_SKIP_DSCP, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = ((igrDSCP&0x3f)<<3) ^ extraItem;	
			DEBUG("flow hash with DSCP %d", igrDSCP);
		}	
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH12_SKIP_CPRI, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = (igrCPRI&0x7) ^ extraItem;	
			DEBUG("flow hash with CPRI %d", igrCPRI);
		}		
	}

	return extraItem;
}


uint32 _rtk_rg_flowHashPath34ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx_gmacChk)
{
	u32 extraItem = 0;
	rtk_rg_asic_path3_entry_t *pP3Data = pFlowData;
	u8 isMulticast = FALSE;
	rtk_enable_t enabled = DISABLED;
	uint8 igrCPRI = 0, igrDSCP = 0;

	if(pP3Data==NULL)
	{
		WARNING("flow data is NULL");
		return 0;
	}

	igrCPRI = pP3Data->in_cvlan_pri;
	igrDSCP = pP3Data->in_tos >> 2;
	
	if(pP3Data->in_ipv4_or_ipv6 == 1){
		/* IPv6 */
		isMulticast = (pP3Data->in_dst_ipv6_addr_hash& FLOW_V6HASHADDR_MC_BIT)?TRUE:FALSE;
	}else{
		/* IPv4 */
		if((pP3Data->in_dst_ipv4_addr >= FLOW_V4ADDR_MC_LO_BOUND) && (pP3Data->in_dst_ipv4_addr < FLOW_V4ADDR_MC_UP_BOUND))
			isMulticast = TRUE;
	}

	if(isMulticast){
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_MC_SKIP_SVID, &enabled), RT_ERR_RG_OK);
		if(!enabled) extraItem |= (igrSVID<<12); 
		else extraItem &= 0x000fff;
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_MC_SKIP_CVID, &enabled), RT_ERR_RG_OK);
		if(!enabled) extraItem |= igrCVID;
		else extraItem &= 0xfff000;
	}else{
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_UCBC_SKIP_SVID, &enabled), RT_ERR_RG_OK);
		if(!enabled) extraItem |= (igrSVID<<12); 
		else extraItem &= 0x000fff;
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_UCBC_SKIP_CVID, &enabled), RT_ERR_RG_OK);
		if(!enabled) extraItem |= igrCVID;
		else extraItem &= 0xfff000;
	}
	
	ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_SKIP_DA, &enabled), RT_ERR_RG_OK);
	/* Extraitem: Consider L4 protocol only for path 3/4/5 */

#if defined(CONFIG_RTL9607C_SERIES)	
	if(ASIC_CHIP_ID==RTL9607C_CHIP_ID && ASICDRIVERVER==CHIP_REV_ID_A)
	{
		if(!enabled)
			extraItem = (pP3Data->in_l4proto<<23 | (lutDaIdx_gmacChk&0xfff)) ^ extraItem;
		else
			extraItem = (pP3Data->in_l4proto<<23) ^ extraItem;
	}else
#endif	
	{
		if(!enabled)
			extraItem = (pP3Data->in_l4proto<<23 | ((lutDaIdx_gmacChk&0x1)<<9)) ^ extraItem;		
		else
			extraItem = (pP3Data->in_l4proto<<23) ^ extraItem;
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATHALL_SKIP_DSCP, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = ((igrDSCP&0x3f)<<3) ^ extraItem;	
			DEBUG("flow hash with DSCP %d", igrDSCP);
		}

		if(isMulticast){
			ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_MC_SKIP_CPRI, &enabled), RT_ERR_RG_OK);
			if(!enabled){
				extraItem = (igrCPRI&0x7) ^ extraItem;		
				DEBUG("flow hash with CPRI %d", igrCPRI);
			}
		}else{
			ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH34_UCBC_SKIP_CPRI, &enabled), RT_ERR_RG_OK);
			if(!enabled){
				extraItem = (igrCPRI&0x7) ^ extraItem;		
				DEBUG("flow hash with CPRI %d", igrCPRI);
			}
		}
		
	}
	
	return extraItem;
}


uint32 _rtk_rg_flowHashPath5ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID)
{
	u32 extraItem = 0;
	rtk_rg_asic_path5_entry_t *pP5Data = pFlowData;
	rtk_enable_t enabled = DISABLED;
	uint8 igrCPRI = 0, igrDSCP = 0;

	if(pP5Data==NULL)
	{
		WARNING("flow data is NULL");
		return 0;
	}

	igrCPRI = pP5Data->in_cvlan_pri;
	igrDSCP = pP5Data->in_tos >> 2;
	
	ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH5_SKIP_SVID, &enabled), RT_ERR_RG_OK);
	if(!enabled) extraItem |= (igrSVID<<12); 
	else extraItem &= 0x000fff;
			
	ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH5_SKIP_CVID, &enabled), RT_ERR_RG_OK);
	if(!enabled) extraItem |= igrCVID;
	else extraItem &= 0xfff000;

	/* Extraitem: Consider L4 protocol only for path 3/4/5 */

#if defined(CONFIG_RTL9607C_SERIES)	
	if(ASIC_CHIP_ID==RTL9607C_CHIP_ID && ASICDRIVERVER==CHIP_REV_ID_A)
	{
		extraItem = (pP5Data->in_l4proto<<23) ^ extraItem;
	}else
#endif	
	{
		extraItem = (pP5Data->in_l4proto<<23) ^ extraItem;
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATHALL_SKIP_DSCP, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = ((igrDSCP&0x3f)<<3) ^ extraItem;
			DEBUG("flow hash with DSCP %d", igrDSCP);
		}	
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH5_SKIP_CPRI, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = (igrCPRI&0x7) ^ extraItem;		
			DEBUG("flow hash with CPRI %d", igrCPRI);
		}
	}

	return extraItem;
}

uint32 _rtk_rg_flowHashPath6ExtraItem_get(void *pFlowData, uint16 igrSVID, uint16 igrCVID)
{
	u32 extraItem = 0;
	rtk_rg_asic_path6_entry_t *pP6Data = pFlowData;
	rtk_enable_t enabled = DISABLED;
	uint8 igrCPRI = 0, igrDSCP = 0;

	if(pP6Data==NULL)
	{
		WARNING("flow data is NULL");
		return 0;
	}

	igrCPRI = pP6Data->in_cvlan_pri;
	igrDSCP = pP6Data->in_tos >> 2;

	ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH6_SKIP_SVID, &enabled), RT_ERR_RG_OK);
	if(!enabled) extraItem |= (igrSVID<<12);
	else extraItem &= 0x000fff;
	
	ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH6_SKIP_CVID, &enabled), RT_ERR_RG_OK);
	if(!enabled) extraItem |= igrCVID;
	else extraItem &= 0xfff000;

#if defined(CONFIG_RTL9607C_SERIES)	
	if(ASIC_CHIP_ID==RTL9607C_CHIP_ID && ASICDRIVERVER==CHIP_REV_ID_A)
	{
		// NA
	}else
#endif	
	{
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATHALL_SKIP_DSCP, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = ((igrDSCP&0x3f)<<3) ^ extraItem;	
			DEBUG("flow hash with DSCP %d", igrDSCP);
		}	
		
		ASSERT_EQ(rtk_rg_asic_globalState_get(FB_GLOBAL_PATH6_SKIP_CPRI, &enabled), RT_ERR_RG_OK);
		if(!enabled){
			extraItem = (igrCPRI&0x7) ^ extraItem;	
			DEBUG("flow hash with CPRI %d", igrCPRI);
		}		
	}
	
	return extraItem;
}


/* 
	[WARNING]:If change this function should sync to  _rtk_rg_flowHashIndexStep1_get() and _rtk_fc_sw_flowHashIndexStep1_get()
	return value: HashIdx
		- 4K mode: (10 bits hash index)<<2 (4-way, need to search entry from (HashIdx<<2+0) to (HashIdx<<2+3) );
		- 8K/16K/32K mode: 13bits/14bits/15bits entry index (1-way, directly mapping)						
	Input: (Path1) SVID, CVID, SMACIDX, DMACIDX, EXTRA(0)
	Input: (Path3) SPORT, DPORT, SIP, DIP, EXTRA(_rtk_rg_flowHashPath34ExtraItem_get)
	Input: (Path5) SPORT, DPORT, SIP, DIP, EXTRA(_rtk_rg_flowHashPath5ExtraItem_get)			*/
uint32 _rtk_rg_flowHashIndexStep1_get(uint16 param1, uint16 param2, uint32 param3, uint32 param4, uint32 extraItem){
	uint32 sum1=0, sum2=0, sum=0, sum_nk=0, hashIdx=0;
	uint32 sport, dport, sip, dip;
	rtk_rg_asic_fbMode_t fbMode;
	uint32 preHashPtn;
	
	//ASSERT_EQ(rtk_rg_asic_fbModeCtrl_get(FB_MODE_FB_MOD, &fbMode), SUCCESS);
	fbMode = rgpro_db.fbMode;

	ASSERT_EQ(rtk_rg_asic_preHashPtn_get(FB_PREHASH_PTN_SPORT, &preHashPtn), SUCCESS);
	sport = _rtk_rg_flowHashPreProcessPort(param1, preHashPtn);
	ASSERT_EQ(rtk_rg_asic_preHashPtn_get(FB_PREHASH_PTN_DPORT, &preHashPtn), SUCCESS);
	dport = _rtk_rg_flowHashPreProcessPort(param2, preHashPtn);
	ASSERT_EQ(rtk_rg_asic_preHashPtn_get(FB_PREHASH_PTN_SIP, &preHashPtn), SUCCESS);
	sip = _rtk_rg_flowHashPreProcessIP(param3, preHashPtn);
	ASSERT_EQ(rtk_rg_asic_preHashPtn_get(FB_PREHASH_PTN_DIP, &preHashPtn), SUCCESS);
	dip = _rtk_rg_flowHashPreProcessIP(param4, preHashPtn);

	sum1 = ((sip&0xfffff)  + (sip>>20) + (dip&0xfffff) + (dip>>20) + sport + dport) & 0x7fffff;		// sum1[22:0]
	sum2 = ((sum1&0xfffff) + (sum1>>20)) & 0x1fffff;						// sum2[20:0]
	sum	  = ((sum2&0xfffff) + (sum2>>20)) & 0x1fffff;						// sum[20:0]

	if(ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)
	{
		switch (fbMode) {
		case FB_MODE_1K:
			// 1k mode: 4-way (8 bits index)
			sum_nk = ((sum&0xff) + ((sum>>8)&0xff) + ((sum>>16)&0x1f))&0xff;		// sum_4k[7:0]
			hashIdx = sum_nk ^ (extraItem&0xff) ^ ((extraItem>>8)&0xff) ^ ((extraItem>>16)&0xff);
			hashIdx = hashIdx<<2;												// Get base entry index
			break;
		case FB_MODE_8K:
			// 8k mode: 1-way (13 bits index)
			sum1 = (sum&0x1fff) + ((sum>>13)&0xff);
			sum2 = (sum1&0x3ff) ^ ((sum1>>10)&0x7);
			sum_nk = (sum1&0x1c00) | (sum2&0x3ff);							// sum_8k[12:0]
			hashIdx = sum_nk ^ (extraItem&0x1fff) ^ ((extraItem>>13)&0x7ff);
			break;
		case FB_MODE_16K:
			// 16k mode: 1-way (14 bits index)
			sum1 = (sum&0x3fff) + ((sum>>14)&0x7f);
			sum2 = (sum1&0x3ff) ^ ((sum1>>10)&0xf);
			sum_nk = (sum1&0x3c00) | (sum2&0x3ff);							// sum_16k[13:0]
			hashIdx = sum_nk ^ (extraItem&0x3fff) ^ ((extraItem>>14)&0x3ff);
			break;
		case FB_MODE_32K:
			// 32k mode: 1-way (15 bits index)
			sum1 = (sum&0x7fff) + ((sum>>15)&0x3f);
			sum2 = (sum1&0x3ff) ^ ((sum1>>10)&0x1f);
			sum_nk = (sum1&0x7c00) | (sum2&0x3ff);							// sum_32k[14:0]
			hashIdx = sum_nk ^ (extraItem&0x7fff) ^ ((extraItem>>15)&0x1ff);
			break;
		}
	}
	else	// not 9603CVD
	{
		switch (fbMode) {
		case FB_MODE_4K:
			// 4k mode: 4-way (10 bits index)
			sum_nk = ((sum&0x3ff) + ((sum>>10)&0x3ff) + ((sum>>20)&0x1))&0x3ff;		// sum_4k[9:0]
			hashIdx = sum_nk ^ (extraItem&0x3ff) ^ ((extraItem>>10)&0x3ff) ^ ((extraItem>>20)&0xf);
			hashIdx = hashIdx<<2;												// Get base entry index
			break;
		case FB_MODE_8K:
			// 8k mode: 1-way (13 bits index)
			sum1 = (sum&0x1fff) + ((sum>>13)&0xff);
			sum2 = (sum1&0xfff) ^ ((sum1>>12)&0x1);
			sum_nk = (sum1&0x1000) | (sum2&0xfff);							// sum_8k[12:0]
			hashIdx = sum_nk ^ (extraItem&0x1fff) ^ ((extraItem>>13)&0x7ff);
			break;
		case FB_MODE_16K:
			// 16k mode: 1-way (14 bits index)
			sum1 = (sum&0x3fff) + ((sum>>14)&0x7f);
			sum2 = (sum1&0xfff) ^ ((sum1>>12)&0x3);
			sum_nk = (sum1&0x3000) | (sum2&0xfff);							// sum_16k[13:0]
			hashIdx = sum_nk ^ (extraItem&0x3fff) ^ ((extraItem>>14)&0x3ff);
			break;
		case FB_MODE_32K:
			// 32k mode: 1-way (15 bits index)
			sum1 = (sum&0x7fff) + ((sum>>15)&0x3f);
			sum2 = (sum1&0xfff) ^ ((sum1>>12)&0x7);
			sum_nk = (sum1&0x7000) | (sum2&0xfff);							// sum_32k[14:0]
			hashIdx = sum_nk ^ (extraItem&0x7fff) ^ ((extraItem>>15)&0x1ff);
			break;
		}
	}

	DEBUG("[FLOWHASH] index=%d, fbMode=%d, params: [0x%x,0x%x,0x%x,0x%x], extra:0x%x", hashIdx, fbMode, param1, param2, param3, param4, extraItem);
	//DEBUG("[FLOWHASH] index=%d, fbMode=%d, after preprocess: [0x%x,0x%x,0x%x,0x%x], sum1:0x%x", hashIdx, fbMode, sport, dport, sip, dip, sum);
	return hashIdx;
}

/*
[WARNING]:If change this function should sync to  _rtk_rg_flowHashIndexStep2_get() and _rtk_fc_sw_flowHashIndexStep2_get()
*/
uint32 _rtk_rg_flowHashIndexStep2_get(uint32 step1Idx)
{
	uint32 hashidx = 0;
	uint32 mask;
	rtk_rg_asic_fbMode_t fbMode;

	// For 9603CVD, Hid_s2[14:0] = {~hid[14:8], ~hid[7], hid[6:2], ~hid[1:0]}	0x7E03= 111_1111_1000_0011
	// For 9607C, Hid_s2[14:0] = {~hid[14:10], ~hid[9], hid[8:2], ~hid[1:0]}	0x7E03= 111_1110_0000_0011
	mask = (ASIC_CHIP_ID==RTL9603CVD_CHIP_ID) ? 0x7F83 : 0x7E03;
	
	//ASSERT_EQ(rtk_rg_asic_fbModeCtrl_get(FB_MODE_FB_MOD, &fbMode), SUCCESS);
	fbMode = rgpro_db.fbMode;
	switch (fbMode) {				
	case FB_MODE_4K:	//FB_MODE_1K
		if(ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)
		{
			// 4k mode: 4-way (8 bits index)
			hashidx = step1Idx >> 2;			// shift back to 8 bits
			mask = mask & 0xff;				// Hid_s2[7:0] = { ~hid[7], hid[6:2], ~hid[1:0]} => 8 bits mask
			hashidx = hashidx ^ mask;
			hashidx = hashidx << 2;			// shift to 12 bits 4 way entry index
		}
		else	// not 9603CVD
		{
			// 4k mode: 4-way (10 bits index)
			hashidx = step1Idx >> 2;			// shift back to 10 bits
			mask = mask & 0x3ff;				// Hid_s2[9:0] = { ~hid[9], hid[8:2], ~hid[1:0]} => 10 bits mask
			hashidx = hashidx ^ mask;
			hashidx = hashidx << 2;			// shift to 12 bits 4 way entry index
		}
		break;
	case FB_MODE_8K:
		// 8k mode: 1-way (13 bits index)
		mask = mask & 0x1fff;
		hashidx = step1Idx ^ mask;			
		break;
	case FB_MODE_16K:
		// 16k mode: 1-way (14 bits index)
		mask = mask & 0x3fff;
		hashidx = step1Idx ^ mask;
		break;
	case FB_MODE_32K:
		// 32k mode: 1-way (15 bits index)
		mask = mask & 0x7fff;
		hashidx = step1Idx ^ mask;
		break;	
	}
	
	DEBUG("[FLOWHASH] index=%d (Step2), fbMode=%d", hashidx, fbMode);
	return hashidx;
}

uint32 _rtk_rg_flowHashIPv6DstAddr_get(uint8 ipDes[16])
{
	/* Dst hashidx = {MC_bit, v6hsh[30:0]} */
	uint32 hashIdx = ntohl((*(uint32*)&ipDes[0])) ^ ntohl((*(uint32*)&ipDes[4])) ^ ntohl((*(uint32*)&ipDes[8])) ^ ntohl((*(uint32*)&ipDes[12]));
	hashIdx = (hashIdx >> 31) ^ (hashIdx & 0x7fffffff);
	/* Set MC bit to 1 if ipv6 address is started with ffxx*/
	if(ipDes[0] == 0xff)
		hashIdx |= (1<<31);
	//DEBUG("hashIdx = 0x%x", hashIdx);
	return hashIdx;
}

uint32 _rtk_rg_flowHashIPv6SrcAddr_get(uint8 ipSrc[16])
{
	/* Dst hashidx = {MC_bit, v6hsh[30:0]} */
	uint32 hashIdx = ntohl((*(uint32*)&ipSrc[0])) ^ ntohl((*(uint32*)&ipSrc[4])) ^ ntohl((*(uint32*)&ipSrc[8])) ^ ntohl((*(uint32*)&ipSrc[12]));

	//DEBUG("hashIdx = 0x%x", hashIdx);
	return hashIdx;
}

uint32 _rtk_rg_flowHashBuckets_get(void)
{
	switch(rgpro_db.fbMode)
	{
		case FB_MODE_4K:
			return 1024;
		case FB_MODE_8K:
			return 8192;
		case FB_MODE_16K:
			return 16384;
		case FB_MODE_32K:
			return 32768;
		default:
			WARNING("ERROR");
			return 0;
	}
}

uint32 _rtk_rg_hashTrapBandwidthSharing_get(uint32 SPA, uint64 SMAC, uint64 DMAC, uint32 innerSIP, uint32 innerDIP, uint32 innerSrcPort, uint32 innerDstPort)
{
	rtk_enable_t enabled;
	uint32 hashValue = 0, i =0;

	//DEBUG("SPA(%d), SMAC(%lld), DMAC(%lld), SIP(%d), DIP(%d), SPROT(%d), DPORT(%d)", SPA, SMAC, DMAC, innerSIP, innerDIP, innerSrcPort, innerDstPort);
	
	//TRAP_HASH_SPA
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_SPA, &enabled);
	if(enabled)
	{
		hashValue ^= (SPA&0x7);											// R2~R0
		hashValue ^= (((SPA&0x1) ^ ((SPA>>1)&0x1) ^ ((SPA>>2)&0x1)) << 3);	// R3 = SPA0 ^ SPA1 ^ SPA2
	}
	//TRAP_HASH_SMAC
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_SMAC, &enabled);
	if(enabled)
	{
		for(i=0; i< 48; i+=4)
			hashValue ^= ((SMAC>>i)&0xf);
	}
	//TRAP_HASH_DMAC
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_DMAC, &enabled);
	if(enabled)
	{
		for(i=0; i< 48; i+=4)
			hashValue ^= ((DMAC>>i)&0xf);
	}
	//TRAP_HASH_SIP_INNER
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_SIP_INNER, &enabled);
	if(enabled)
	{
		for(i=0; i< 32; i+=4)
			hashValue ^= ((innerSIP>>i)&0xf);
	}
	//TRAP_HASH_DIP_INNER
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_DIP_INNER, &enabled);
	if(enabled)
	{			
		for(i=0; i< 32; i+=4)
			hashValue ^= ((innerDIP>>i)&0xf);
	}
	//TRAP_HASH_SPORT_INNER
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_SPORT_INNER, &enabled);
	if(enabled)	// tcp or udp
	{
		for(i=0; i< 16; i+=4)
			hashValue ^= ((innerSrcPort>>i)&0xf);
	}
	//TRAP_HASH_DPORT_INNER
	rtk_trap_cpuTrapHashMask_get(TRAP_HASH_DPORT_INNER, &enabled);
	if(enabled)
	{
		for(i=0; i< 16; i+=4)
			hashValue ^= ((innerDstPort>>i)&0xf);
	}

	return (hashValue&0xf);	// possible value: 0~15
}

uint32 _rtk_rg_fbMode_set(rtk_rg_asic_fbMode_t fbMode)
{
	uint32 numFlowEntry = 0;
	uint32 memSize = 0;
	void *ddrMemBase = NULL;
	uint32 supportedModeMask = 0xff;	// allow setting fbMode if no any related compiler flag, e.g. model code.

	TRACE("Config FB mode: %d(%s)", fbMode, (fbMode==0)?"SRAM 4K":((fbMode==1)?"DDR 8K":((fbMode==2)?"DDR 16K":((fbMode==3)?"DDR 32K":"UNKNOWN"))));


#if defined(CONFIG_RTK_L34_FLOWBASED_DYN_MODE)
	supportedModeMask = (1<<FB_MODE_4K)|(1<<FB_MODE_8K)|(1<<FB_MODE_16K)|(1<<FB_MODE_32K);
#elif defined(CONFIG_RG_FLOW_4K_MODE)
	supportedModeMask = (1<<FB_MODE_4K);
#elif defined(CONFIG_RG_FLOW_8K_MODE)
	supportedModeMask = (1<<FB_MODE_8K);
#elif defined(CONFIG_RG_FLOW_16K_MODE)
	supportedModeMask = (1<<FB_MODE_16K);
#elif defined(CONFIG_RG_FLOW_32K_MODE)
	supportedModeMask = (1<<FB_MODE_32K);
#endif

	if((supportedModeMask & (1<<fbMode))==0x0)
	{
		WARNING("fbMode %d was not supported! please check compiler flag.", fbMode);
		return FAILED;
	}


	// Init FB ASIC / reset tables
	ASSERT_EQ(rtk_rg_asic_fb_init(), SUCCESS);
	// Init FbMode
	ASSERT_EQ(rtk_rg_asic_fbModeCtrl_set(FB_MODE_FB_MOD, fbMode), SUCCESS);
	rgpro_db.fbMode = fbMode;

	numFlowEntry = _rtk_rg_flowEntryNum_get();

#if defined(CONFIG_RTK_L34_FLOWBASED_DYN_MODE)
	memSize = (numFlowEntry << 5);								// Note: one entry occupy 32 bytes.
#elif !defined(CONFIG_RG_FLOW_4K_MODE)
	memSize = sizeof(rgpro_db.ddrMemBlock) - 1023;				//maximun allocation but ignore alignment
#endif//	!CONFIG_RG_FLOW_4K_MODE


	if(rgpro_db.ddrMemAlloc != NULL){	
		rgpro_db.ddrMemAlloc = NULL;
	}
	if(fbMode != FB_MODE_4K){
		/* Allocate memory and set base address to configure cache controller */		
		ddrMemBase = _rtk_rg_memoryAlignAlloc(1024, memSize);

		if(ddrMemBase == NULL)
			return RT_ERR_RG_FAILED;

		//TRACE("Config as DDR mode %d: flow table alloc mem %d bytes @ %p", fbMode, memSize, ddrMemBase);

		if(fbMode == FB_MODE_32K)
		{
			// For 32K mode, support traffic bits only.
			ASSERT_EQ(rtk_rg_asic_ccGlobalState_set(FB_CC_GLOBAL_FLOW_VALID_EN, DISABLED), SUCCESS);
		}else
		{
			// traffic bits and valid bits share the same register pool.
			ASSERT_EQ(rtk_rg_asic_ccGlobalState_set(FB_CC_GLOBAL_FLOW_VALID_EN, ENABLED), SUCCESS);
		}
		// the struct support 32 bytes alignment only.
		ASSERT_EQ(rtk_rg_asic_ccGlobalState_set(FB_CC_GLOBAL_BUS_ALIGN, ENABLED), SUCCESS);
		rgpro_db.ddrBusAlign32 = ENABLED;

		ASSERT_EQ(rtk_rg_asic_ccMemAddr_set(&ddrMemBase), SUCCESS);
		rgpro_db.ddrMemBase = ddrMemBase;

		
		// reset dram flow buffer
		_rtk_rg_clearDDRMemBlock();
	}else
	{
		//TRACE("Config as SRAM mode");
		
		ASSERT_EQ(rtk_rg_asic_ccMemAddr_set(&ddrMemBase), SUCCESS);
		rgpro_db.ddrMemBase = NULL;
	}

#if !defined(CONFIG_RG_FLOW_4K_MODE) && defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#if !defined(CONFIG_APOLLOPRO_ASICMODE)
	{
		//1only used in FPGA platform to clear simulated DRAM.
		// in normal case, DRAM should be cleared by _rtk_rg_clearDDRMemBlock()/
		// but it took too much time, so we skip the original action and do clear by each rg_init/mode_set
		int i = 0, entryNum = _rtk_rg_flowEntryNum_get();
		for(i = 0; i < entryNum; i++)
		{
			io_spi_dramFlowEntry_reset(i);
		}
	}	
#endif
#endif
		
	return SUCCESS;
}
rtk_rg_asic_fbMode_t _rtk_rg_fbMode_get(void)
{
	return rgpro_db.fbMode;
}

uint32 _rtk_rg_init_rgProDB()
{
	static bool rgProInitDone = FALSE;
	if(rgProInitDone==FALSE)
	{
		uint8 u8Sta = 0;
	    	ASSERT_EQ(rtk_rg_asic_fbModeCtrl_get(FB_MODE_FB_MOD, &u8Sta), RT_ERR_RG_OK);
		rgpro_db.fbMode = u8Sta;
		rgpro_db.ddrMemAlloc = NULL;
		rgpro_db.ddrMemBase = NULL;
		memset(&rgpro_db.flowEntryValidBits, 0, sizeof(rgpro_db.flowEntryValidBits));
		rgProInitDone = TRUE;
	}
	
#if defined(CONFIG_APOLLOPRO_MODEL) || defined(CONFIG_APOLLOPRO_FPGA)	// only for testing
	rgpro_db.ignoreFlowValidTableRst = FALSE;
#endif

	return SUCCESS;
}

uint32 _rtk_rg_fb_hw_init(void)
{
	int i;

	i = 0;
	
#if defined(CONFIG_RG_FLOW_4K_MODE)
	ASSERT_EQ(_rtk_rg_fbMode_set(FB_MODE_4K), RT_ERR_OK);
#elif defined(CONFIG_RG_FLOW_8K_MODE)
	ASSERT_EQ(_rtk_rg_fbMode_set(FB_MODE_8K), RT_ERR_OK);
#elif defined(CONFIG_RG_FLOW_16K_MODE)
	ASSERT_EQ(_rtk_rg_fbMode_set(FB_MODE_16K), RT_ERR_OK);
#elif defined(CONFIG_RG_FLOW_32K_MODE)
	ASSERT_EQ(_rtk_rg_fbMode_set(FB_MODE_32K), RT_ERR_OK);
#endif // end CONFIG_RG_FLOW_4K_MODE


#if defined(CONFIG_RTL9607C_SERIES)
	if(ASIC_CHIP_ID==RTL9607C_CHIP_ID && ASICDRIVERVER==CHIP_REV_ID_A)
		rtk_rg_asic_l2tpUdpSport_set(0);
#endif	
	
	return RT_ERR_OK;
}

#endif //CONFIG_RG_RTL9607C_SERIES

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

rtk_rg_err_code_t _rtk_rg_lut_ucLookupAction_init()
{
	ASSERT_EQ(rtk_rg_asic_l2UcAct_set(FB_L2UCACT_FB), RT_ERR_RG_OK);

	return RT_ERR_RG_OK;
}

#endif
