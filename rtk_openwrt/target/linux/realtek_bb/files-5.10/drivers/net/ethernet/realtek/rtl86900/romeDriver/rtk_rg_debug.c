
#include <stdarg.h>
#include <rtk_rg_internal.h>
#include <rtk_rg_debug.h>

#include <linux/sched.h>
#include <linux/tty.h>

#define COLOR_Y "\033[1;33m"
#define COLOR_NM "\033[0m"
#define COLOR_H "\033[1;37;41m"
#define COLOR_G "\033[1;32m"


#ifdef CONFIG_RG_DEBUG
#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_acl.h>
//#include <rtk_rg_liteRomeDriver.h>
//#include <rtk_rg_internal.h>
#include <rtk_rg_fwdEngine.h>
#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
//#include <rtk_rg_debug.h>
#include <rtk_rg_alg_tool.h>
#include <rtk_rg_callback.h>

#include <rtk/acl.h>
#include <rtk/classify.h>
#include <rtk/l2.h>
#include <rtk/l34.h>
#include <rtk/qos.h>
#include <rtk/svlan.h>
#include <ioal/mem32.h>


#if defined(CONFIG_RG_RTL9600_SERIES)
#include <dal/apollomp/raw/apollomp_raw_hwmisc.h>
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#include <dal/rtl9602c/dal_rtl9602c_hwmisc.h>
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#include <dal/rtl9607c/dal_rtl9607c_hwmisc.h>
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#include <dal/rtl9603cvd/dal_rtl9603cvd_hwmisc.h>
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#include <rtk_rg_apolloPro_internal.h>
#include <rtk_rg_apolloPro_liteRomeDriver.h>
#endif
#if defined(CONFIG_RG_G3_SERIES)
#include <rtk_rg_g3_internal.h>

#include <aal_l2_vlan.h>
#include <aal_l3fe.h>
#include <aal_l3_cls.h>
#include <aal_l3_specpkt.h>
#include <classifier.h>
#include <aal_l3_cam.h>		//L3_CAM_PKT_LEN_TBL_ENTRY_MAX
#include <aal_l3_pe.h>
#include <aal_l2_te.h>
#include <aal_l2_qm.h>
#include <aal_l2_tm_cb.h>
#include <aal_l3_te.h>
#include <aal_l3_te_cb.h>
#endif

#include <linux/proc_fs.h>
#include <asm/stacktrace.h> //for dump_stack()


#ifdef CONFIG_RG_WMUX_SUPPORT
#include <rtk_rg_wmux.h>
#endif



#include <rtk_rg_igmpsnooping.h>
extern struct rtl_mCastTimerParameters rtl_mCastTimerParas;
extern struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];



#if defined(CONFIG_RG_RTL9600_SERIES)
#define DIAG_UTIL_CHIP_TYPE APOLLOMP_CHIP_ID
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define DIAG_UTIL_CHIP_TYPE RTL9602C_CHIP_ID
#elif defined(CONFIG_RG_G3_SERIES)
#define DIAG_UTIL_CHIP_TYPE 0
#endif

extern rtk_mac_t g_whiteList[WHITELISTSIZE];
extern unsigned int g_whiteList_idx;

void _rtk_rg_dump_stack(void)
{
#if defined(CONFIG_RG_G3_SERIES)
#else
	struct pt_regs regs;
	unsigned long sp,ra,pc;
//	prepare_frametrace(&regs);

    memset(&regs, 0, sizeof(regs));

	__asm__ __volatile__(
	".set push\n\t"
	".set noat\n\t"
	"1: la $1, 1b\n\t"
	"sw $1, %0\n\t"
	"sw $29, %1\n\t"
	"sw $31, %2\n\t"
	".set pop\n\t"
	: "=m" (regs.cp0_epc),
	"=m" (regs.regs[29]), "=m" (regs.regs[31])
	: : "memory");


	sp = regs.regs[29];
	ra = regs.regs[31];
	pc = regs.cp0_epc;

	if (!__kernel_text_address(pc))
	{
		return;
	}

	rtlglue_printf("\033[1;33;41m");
	pc = unwind_stack(current, &sp, pc, &ra);
	while(1)
	{
		if(!pc) break;
		pc = unwind_stack(current, &sp, pc, &ra);
		if(!pc) break;
		rtlglue_printf("[%p][%pS]\n", (void *)pc, (void *)pc);
		//printk("[%p:%pS]\n", (void *) pc, (void *) pc);
	}
	rtlglue_printf("\033[0m\n");
#endif
}



void assert_ok_sub_func(int assert_ret, char *assert_str,const char *func,int line)
{
	if(assert_ret!=0)
	{
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN)
		{
        	rtlglue_printf("\033[1;33;41m[ASSERT_FAIL]'%s=0x%x'\033[1;30m @%s:%d\033[0m\n",assert_str,assert_ret,func,line);
#ifdef CONFIG_RG_DEBUG
			_rtk_rg_dump_stack();
#endif
		}
	}
}

__IRAM_FWDENG void assert_sub_func(int assert_ret, char *assert_str,const char *func,int line)
{
	if(!(assert_ret))
	{
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN)
		{
			rtlglue_printf("\033[1;33;41m[ASSERT]'%s'\033[1;30m @%s:%d\033[0m\n", assert_str,func,line);
#ifdef CONFIG_RG_DEBUG
			_rtk_rg_dump_stack();
#endif
		}
	}
}




//#define diag_util_inet_mactoa mactoa
int8 *diag_util_inet_mactoa (const uint8 *mac)
{
        static int8 str[6*sizeof "123"];

    if (NULL == mac)
    {
        sprintf(str,"NULL");
        return str;
    }

    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return str;
} /* end of diag_util_mac2str */


/*IPv6 address to string*/
int8 *diag_util_inet_n6toa(const uint8 *ipv6)
{
	static int8 buf[8*sizeof "FFFF:"];
    uint32  i;
    uint16  ipv6_ptr[8] = {0};

    for (i = 0; i < 8 ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }

    sprintf(buf, "%X:%X:%X:%X:%X:%X:%X:%X", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);
	return (buf);
}

#define diag_util_printf(fmt, args...)    printk( fmt, ## args)
#include <rtk/debug.h>
#include <diag_display.h>

/*internal function*/
void hs_displayL4Hsb_S(rtk_l34_hsb_t * hsbWatch,struct seq_file *s)
{
    //rtk_ip_addr_t addr;

    // PROC_PRINTF("L4HSB(");
    PROC_PRINTF("\tspa:%d\n",hsbWatch->spa);
    PROC_PRINTF("\tda:%02x-%02x-%02x-%02x-%02x-%02x\n",hsbWatch->dmac.octet[0],hsbWatch->dmac.octet[1],hsbWatch->dmac.octet[2],hsbWatch->dmac.octet[3],hsbWatch->dmac.octet[4],hsbWatch->dmac.octet[5]);
    //addr =ntohl( hsbWatch->sip);
#ifdef CONFIG_APOLLO_TESTING
    PROC_PRINTF("\tsip:%s(hex:%08x)   ",inet_ntoa(addr),hsbWatch->sip);
#else
    //PROC_PRINTF("\tsip:%08x ",hsbWatch->sip);
	PROC_PRINTF("\tsip:");

    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[0]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[1]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[2]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[3]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[4]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[5]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[6]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[7]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[8]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[9]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[10]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[11]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[12]));
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[13]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->sip.ipv6_addr[14]));
    PROC_PRINTF("%02x",hsbWatch->sip.ipv6_addr[15]);
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
    PROC_PRINTF("\tis_policy:%d policy_nh_idx:%d\n ",(int)hsbWatch->is_policy_route,hsbWatch->policy_nh_idx);
#endif
    PROC_PRINTF("\tsprt:%d(hex:%08x)\n ",(int)hsbWatch->sport_icmpid_chksum,hsbWatch->sport_icmpid_chksum);
    PROC_PRINTF("\tdip:");

    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[0]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[1]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[2]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[3]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[4]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[5]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[6]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[7]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[8]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[9]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[10]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[11]);
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[12]));
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[13]));
    PROC_PRINTF(":");
    PROC_PRINTF("%02x",(hsbWatch->dip.ipv6_addr[14]));
    PROC_PRINTF("%02x",hsbWatch->dip.ipv6_addr[15]);

    PROC_PRINTF("\tdprt:%d(hex:%08x)\n",hsbWatch->dport_l4chksum,hsbWatch->dport_l4chksum);

    PROC_PRINTF("\ttype:%d",hsbWatch->type);

    PROC_PRINTF("\tttlst:0x%x\n",hsbWatch->ttls);
    PROC_PRINTF("\tpppoetagif:%d\tpppoeId:%d\n",hsbWatch->pppoe_if,hsbWatch->pppoe_id);
    PROC_PRINTF("\tctagif:%d",hsbWatch->cvlan_if);
    PROC_PRINTF("\tvid :%d\n",hsbWatch->cvid);
    PROC_PRINTF("\tstagif:%d",hsbWatch->svlan_if);
    PROC_PRINTF("\tlen:%d\n",hsbWatch->len);

    PROC_PRINTF("\tudp_nocs:%d",hsbWatch->udp_no_chksum);
    PROC_PRINTF("\ttcpflg:%x\n",hsbWatch->tcp_flag);
    PROC_PRINTF("\tparsfail:%d",hsbWatch->parse_fail);
    PROC_PRINTF("\tdirtx:%d\n",hsbWatch->cpu_direct_tx);
    PROC_PRINTF("\tl3csok:%d\tl4csok:%d\n",hsbWatch->l3_chksum_ok,hsbWatch->l4_chksum_ok);
    PROC_PRINTF("\tipmf:%d\tipfragif:%d\n",hsbWatch->ipmf,hsbWatch->ipfrag_s);
    PROC_PRINTF("\tisFromWan:%d\n",hsbWatch->isFromWan);

//    PROC_PRINTF("\tl2bridge:%d\n)\n",hsbWatch->extl2);
    PROC_PRINTF("\tl2bridge:%d\n",hsbWatch->l2bridge);

#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("\tis_policy_route:%d policy_nh_idx:%d\n",hsbWatch->is_policy_route,hsbWatch->policy_nh_idx);
#endif

}

void hs_displayL4Hsa_S(rtk_l34_hsa_t *hsaWatch,struct seq_file *s)
{
    rtk_ip_addr_t addr;


//   PROC_PRINTF(("L4HSA("));
    addr =ntohl( hsaWatch->ip);
#ifdef CONFIG_APOLLO_TESTING
	PROC_PRINTF("\ttrip:%s(hex:%08x)",inet_ntoa(addr),hsaWatch->ip);
#else
    PROC_PRINTF("\ttrip:%08x",hsaWatch->ip);
#endif
    PROC_PRINTF("\tprt:%d\n",hsaWatch->port);
    PROC_PRINTF("\tl3cs:0x%x",hsaWatch->l3_chksum);
    PROC_PRINTF("\tl4cs:0x%x\n",hsaWatch->l4_chksum);
    PROC_PRINTF("\tmacidx:%d\n",hsaWatch->nexthop_mac_idx);
    PROC_PRINTF("\tpppif:%d",hsaWatch->pppoe_if);
    PROC_PRINTF("\tpppid:%d",hsaWatch->pppid_idx);
	PROC_PRINTF("\tpppoeKeep:%d\n",hsaWatch->pppoeKeep);
    PROC_PRINTF("\tdvid:%d(0x%x)",hsaWatch->dvid,hsaWatch->dvid);
    PROC_PRINTF("\tdestination interface :%d\n",hsaWatch->difid);

    PROC_PRINTF("\tfrag:%d\n",hsaWatch->frag);
    PROC_PRINTF("\tpriority:%d\tvalid:%d\n",hsaWatch->l4_pri_sel,hsaWatch->l4_pri_valid);
    PROC_PRINTF("\taction:%d\n",hsaWatch->action);
    PROC_PRINTF("\tInternal NETIF:%d",hsaWatch->interVlanIf);
    PROC_PRINTF("\tl2tr:%d",hsaWatch->l2trans);
    PROC_PRINTF("\tl34tr:%d\n",hsaWatch->l34trans);
    PROC_PRINTF("\tbindvidtrans:%d\n",hsaWatch->bindVidTrans);
    PROC_PRINTF("\treason:%d\n",hsaWatch->reason);

//    PROC_PRINTF(")\n");
	return ;


}






extern void _diag_debug_hsb_display(rtk_hsb_t *hsaDatb);
extern void _diag_debug_hsa_display(rtk_hsa_t *hsaData);
extern void _diag_debug_hsd_display(rtk_hsa_debug_t *hsaDatd);

#if defined(CONFIG_RG_RTL9602C_SERIES)

int32 dump_ipmv6_l3mcr_table(struct seq_file *s, void *v)
{
	int i,j;
	int len=0;
	uint32 getVal=0;

	PROC_PRINTF("ASIC IPMv6 L3MCR Table : \n");
	for(i=0;i<MAX_IPMV6_L3MCR_TABL_SIZE;i++){
		PROC_PRINTF("[%d]",i);
		for(j=0;j<RTK_RG_MAC_PORT_MAX;j++){
			if(RG_INVALID_MAC_PORT(j)) continue;
			getVal=0;
			rtk_l34_ip6mcRoutingTransIdx_get(i,j,&getVal);
			PROC_PRINTF("   port[%d]->idx=%d",j,getVal);
		}
		PROC_PRINTF("\n");
	}
	return len;

}



int32 dump_dslite_status(struct seq_file *s, void *v)
{
	int len =0;
	uint32 status;
    PROC_PRINTF(">>DSLITE Status:\n\n");
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_DSLITE_STATE,&status);
    PROC_PRINTF("DSLITE Enable:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_MC_PREFIX_UNMATCH,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_MC_PREFIX_UNMATCH:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_DS_UNMATCH_ACT,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_DS_UNMATCH_ACT:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP6_NH_ACTION,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP6_NH_ACTION:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP6_FRAGMENT_ACTION,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP6_FRAGMENT_ACTION:%d \n",status);
	rtk_l34_dsliteControl_get(L34_DSLITE_CTRL_IP4_FRAGMENT_ACTION,&status);
    PROC_PRINTF("DSLITE L34_DSLITE_CTRL_IP4_FRAGMENT_ACTION:%d \n",status);
	return len;

}


int32 dump_dslite_table(struct seq_file *s, void *v)
{
	int len =0;
	int i,j;
	rtk_l34_dsliteInf_entry_t dslite;
    PROC_PRINTF(">>DSLITE Table:\n\n");
	for(i=0;i<4;i++){
		dslite.index=i;
		rtk_l34_dsliteInfTable_get(&dslite);
		if(dslite.valid){
			PROC_PRINTF("[%d] flowLabel:%x hopLimit:%d tc:%x tcOpt:%s \n",i,dslite.flowLabel,dslite.hopLimit,dslite.tc,dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_ASSIGN?"Assign":"Copy from IPv4 TOS");

			PROC_PRINTF("\tAFTRIP:\t");
			for(j=0;j<8;j++)
				PROC_PRINTF("%02x%02x ",dslite.ipAftr.ipv6_addr[j*2],dslite.ipAftr.ipv6_addr[j*2+1]);
			PROC_PRINTF("\n");

			PROC_PRINTF("\tB4IP:\t");
			for(j=0;j<8;j++)
				PROC_PRINTF("%02x%02x ",dslite.ipB4.ipv6_addr[j*2],dslite.ipB4.ipv6_addr[j*2+1]);
			PROC_PRINTF("\n");
		}
	}

	return len;

}

int32 dump_dsliteMc_table(struct seq_file *s, void *v)
{
	int len=0;
	int i,j;
	rtk_l34_dsliteMc_entry_t dsliteMc;
    PROC_PRINTF(">>DSLITEMC Table:\n\n");
	for(i=0;i<MAX_DSLITEMC_SW_TABLE_SIZE;i++){
		dsliteMc.index=i;
		rtk_l34_dsliteMcTable_get(&dsliteMc);

		PROC_PRINTF("[%d] \n",i);
		PROC_PRINTF("\tipMPrefix64:\t\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipMPrefix64.ipv6_addr[j*2],dsliteMc.ipMPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipMPrefix64Mask:\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2],dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipUPrefix64:\t\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipUPrefix64.ipv6_addr[j*2],dsliteMc.ipUPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipUPrefix64Mask:\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2],dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

	}
	return len;


}

int _rtk_rg_proc_host_policing_get(struct seq_file *s, void *v)
{

	int len=0;
    int i;
	uint32 value;
	rtk_rg_hostPoliceControl_t hostPoliceControl;
	rtk_rg_hostPoliceLogging_t hostPoliceLogging;
	rtk_enable_t ifgInclude;
	rtk_rate_metet_mode_t meterMode;

	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);

    for(i = 0;i < HOST_POLICING_TABLE_SIZE; i++) {
        ASSERT_EQ((pf.rtk_rg_hostPoliceControl_get)(&hostPoliceControl,i),RT_ERR_RG_OK);
		if(!(hostPoliceControl.egressLimitCtrl|hostPoliceControl.ingressLimitCtrl|hostPoliceControl.mibCountCtrl))continue;
		PROC_PRINTF("-------------------------------------------------\n");
        PROC_PRINTF("host[%d]: %02X:%02X:%02X:%02X:%02X:%02X  \n", i,
			hostPoliceControl.macAddr.octet[0], hostPoliceControl.macAddr.octet[1], hostPoliceControl.macAddr.octet[2],
			hostPoliceControl.macAddr.octet[3], hostPoliceControl.macAddr.octet[4], hostPoliceControl.macAddr.octet[5]);
        PROC_PRINTF("egress_en: %s  \n", hostPoliceControl.egressLimitCtrl==ENABLED?"En":"Dis");
        PROC_PRINTF("ingress_en: %s  \n", hostPoliceControl.ingressLimitCtrl==ENABLED?"En":"Dis");
		if(hostPoliceControl.egressLimitCtrl==ENABLED||hostPoliceControl.ingressLimitCtrl==ENABLED){
	        PROC_PRINTF("meter index: %d  \n", hostPoliceControl.limitMeterIdx);
			ASSERT_EQ(rtk_rg_rate_shareMeter_get(i,&value,&ifgInclude),RT_ERR_OK);
			PROC_PRINTF("  meter rate: %d, ifg: %d  \n", value, ifgInclude);
			ASSERT_EQ(rtk_rg_rate_shareMeterBucket_get(i,&value),RT_ERR_OK);
			PROC_PRINTF("  bucket size: %d  \n", value);
			ASSERT_EQ(rtk_rg_rate_shareMeterMode_get(i,&meterMode),RT_ERR_OK);
			PROC_PRINTF("  meter mode: %s  \n", meterMode==METER_MODE_BIT_RATE?"BIT":"PACKET");
			ASSERT_EQ(rtk_rg_rate_shareMeterExceed_get(i,&value),RT_ERR_OK);
			PROC_PRINTF("  meter exceed: %d  \n", value);
		}
		ASSERT_EQ((pf.rtk_rg_hostPoliceLogging_get)(&hostPoliceLogging,i),RT_ERR_RG_OK);
		PROC_PRINTF("mib_en: %s  \n", hostPoliceControl.mibCountCtrl==ENABLED?"En":"Dis");
		if(hostPoliceControl.mibCountCtrl==ENABLED){
			PROC_PRINTF("mib_rx: %llu  \n", hostPoliceLogging.rx_count);
			PROC_PRINTF("mib_tx: %llu  \n", hostPoliceLogging.tx_count);
		}
    }

    return len;
}
#endif


int32 dump_napt(struct seq_file *s, void *v)
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int len=0;
#else
	uint32 idx=0, entry=0, retval=0;
	int len=0;
    rtk_l34_naptOutbound_entry_t asic_outtcpudp;
    rtk_l34_naptInbound_entry_t asic_intcpudp;
	char *napt_state[11]={"INVALID","SYN_RECV","UDP_FIRST","SYN_ACK_RECV","UDP_SECOND","TCP_CONNECTED","UDP_CONNECTED","FIRST_FIN","RST_RECV","FIN_SEND_AND_RECV","LAST_ACK"};

    PROC_PRINTF(">>ASIC NAPT TCP/UDP Table:\n");
    PROC_PRINTF(" ---------------- Outbound ----------------\n");
    for(idx=0; idx<MAX_NAPT_OUT_HW_TABLE_SIZE; idx++)
    {
        retval = rtk_l34_naptOutboundTable_get(idx, &asic_outtcpudp);
        if (retval != RT_ERR_OK)
            continue;

        if (asic_outtcpudp.valid == 1)
        {
            PROC_PRINTF("  [%4d] INIDX(%d) priValid(%d) priority(%d) - extPort(0x%x) state(%s) idle(%d) replace(%d) noAddHw(%d) recordedInLimitCount(%d) naptrLookupHit(%d) forceExtPort(%d)\n",
                           idx,
                           asic_outtcpudp.hashIdx,
                           asic_outtcpudp.priValid,
                           asic_outtcpudp.priValue,
                           rg_db.naptOut[idx].extPort,
                           napt_state[rg_db.naptOut[idx].state],
                           rg_db.naptOut[idx].idleSecs,
                           rg_db.naptOut[idx].canBeReplaced,
                           rg_db.naptOut[idx].cannotAddToHw,
                           rg_db.naptOut[idx].recordedInLimitCount,
                           rg_db.naptOut[idx].naptrLookupHit,
                           rg_db.naptOut[idx].forceExtPort
                           );
            entry++;
        }
    }

    entry=0;
    PROC_PRINTF(" ---------------- Inbound ----------------\n");
    for(idx=0; idx<MAX_NAPT_IN_HW_TABLE_SIZE; idx++)
    {
        retval = rtk_l34_naptInboundTable_get(idx, &asic_intcpudp);
        if (retval == FAIL)
            continue;

        if (asic_intcpudp.valid != 0)
        {
        	uint32 remoteIp=0;
			uint16 remotePort=0;
        	if(rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)
        	{
        		int outIdx=rg_db.naptIn[idx].symmetricNaptOutIdx;
				remoteIp=rg_db.naptOut[outIdx].remoteIp;
				remotePort=rg_db.naptOut[outIdx].remotePort;
        	}
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF("  [%4d] %d.%d.%d.%d:%d V(%d), IPIDX(%d} REMHASH(%d) EXTPORT(0x%x) TCP(%d) PRI_EN(%d) PRI(%d) - remote(%d.%d.%d.%d:%d) idle(%d) replace(%d) noAddHw(%d) sw_valid(%d) %s\n",
                           idx,
                           asic_intcpudp.intIp>>24, (asic_intcpudp.intIp&0x00ff0000) >> 16,
                           (asic_intcpudp.intIp&0x0000ff00)>>8, asic_intcpudp.intIp&0x000000ff,
                           asic_intcpudp.intPort,
                           asic_intcpudp.valid, asic_intcpudp.extIpIdx,
                           asic_intcpudp.remHash,
                           ((uint32)asic_intcpudp.extPortHSB<<8 | (uint32)asic_intcpudp.extPortLSB), asic_intcpudp.isTcp, asic_intcpudp.priValid, asic_intcpudp.priId,
                           remoteIp>>24, (remoteIp&0x00ff0000) >> 16,
                           (remoteIp&0x0000ff00)>>8, remoteIp&0x000000ff,
                           remotePort,
                           rg_db.naptIn[idx].idleSecs,
                           rg_db.naptIn[idx].canBeReplaced,
                           rg_db.naptIn[idx].cannotAddToHw,
                           rg_db.naptIn[idx].rtk_naptIn.valid,
                           (rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE")
                           );
#elif defined(CONFIG_RG_RTL9600_SERIES)
            PROC_PRINTF("  [%4d] %d.%d.%d.%d:%d V(%d), IPIDX(%d} REMHASH(%d) EPLSB(0x%x) TCP(%d) PRI_EN(%d) PRI(%d) - remote(%d.%d.%d.%d:%d) idle(%d) replace(%d) noAddHw(%d) sw_valid(%d) %s\n",
                           idx,
                           asic_intcpudp.intIp>>24, (asic_intcpudp.intIp&0x00ff0000) >> 16,
                           (asic_intcpudp.intIp&0x0000ff00)>>8, asic_intcpudp.intIp&0x000000ff,
                           asic_intcpudp.intPort,
                           asic_intcpudp.valid, asic_intcpudp.extIpIdx,
                           asic_intcpudp.remHash,
                           asic_intcpudp.extPortLSB, asic_intcpudp.isTcp, asic_intcpudp.priValid, asic_intcpudp.priId,
                           remoteIp>>24, (remoteIp&0x00ff0000) >> 16,
                           (remoteIp&0x0000ff00)>>8, remoteIp&0x000000ff,
                           remotePort,
                           rg_db.naptIn[idx].idleSecs,
                           rg_db.naptIn[idx].canBeReplaced,
                           rg_db.naptIn[idx].cannotAddToHw,
                           rg_db.naptIn[idx].rtk_naptIn.valid,
                           (rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[idx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE")
                           );
#endif
            entry++;
        }
    }
    PROC_PRINTF("Total entry: %d\n", entry);
#endif

    return len;
}

int32 dump_napt_entry(int outIdx,int outHashIdx,struct seq_file *s)
{
	char *napt_state[11]={"INVALID","SYN_RECV","UDP_FIRST","SYN_ACK_RECV","UDP_SECOND","TCP_CONNECTED","UDP_CONNECTED","FIRST_FIN","RST_RECV","FIN_SEND_AND_RECV","LAST_ACK"};

	if(rg_db.naptOut[outIdx].state!=INVALID)
	{
		int inIdx;
		inIdx=rg_db.naptOut[outIdx].rtk_naptOut.hashIdx;
		if(rg_db.naptIn[inIdx].rtk_naptIn.valid !=0)
		{
			uint32 extip=rg_db.extip[rg_db.naptIn[inIdx].rtk_naptIn.extIpIdx].rtk_extip.extIpAddr;
			int inHash=_rtk_rg_naptTcpUdpInHashIndex(rg_db.naptIn[inIdx].rtk_naptIn.isTcp,extip,rg_db.naptOut[outIdx].extPort);
			PROC_PRINTF("[O:%04d(%03d),I:%04d(%03d)] %s %03d.%03d.%03d.%03d:%05d[PRI:%d(%s)] <-> %03d.%03d.%03d.%03d:%05d <-> %03d.%03d.%03d.%03d:%05d[PRI:%d(%s)] %s IDLE:%d TYPE:%s, recordedInLimitCount(%d), naptrLookupHit(%d), forceExtPort(%d)\n",
						   outIdx,outHashIdx,
						   inIdx,inHash,
						   (rg_db.naptIn[inIdx].rtk_naptIn.isTcp==1)?"TCP":"UDP",
						   rg_db.naptIn[inIdx].rtk_naptIn.intIp>>24, (rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x00ff0000) >> 16,
						   (rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x0000ff00)>>8, rg_db.naptIn[inIdx].rtk_naptIn.intIp&0x000000ff,
						   rg_db.naptIn[inIdx].rtk_naptIn.intPort,
						   rg_db.naptIn[inIdx].rtk_naptIn.priId,
						   (rg_db.naptIn[inIdx].rtk_naptIn.priValid==1)?"on":"off",
							extip>>24,(extip&0x00ff0000) >> 16,
							(extip&0x0000ff00)>>8, extip&0x000000ff,
						   rg_db.naptOut[outIdx].extPort,
						   rg_db.naptOut[outIdx].remoteIp>>24,(rg_db.naptOut[outIdx].remoteIp&0x00ff0000) >> 16,
						   (rg_db.naptOut[outIdx].remoteIp&0x0000ff00)>>8, rg_db.naptOut[outIdx].remoteIp&0x000000ff,
						   rg_db.naptOut[outIdx].remotePort,
							rg_db.naptOut[outIdx].rtk_naptOut.priValue,
							(rg_db.naptOut[outIdx].rtk_naptOut.priValid==1)?"on":"off",

						   napt_state[rg_db.naptOut[outIdx].state],
						   rg_db.naptOut[outIdx].idleSecs,
                           (rg_db.naptIn[inIdx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)?"SYMMETRIC":
                           ((rg_db.naptIn[inIdx].coneType==NAPT_IN_TYPE_RESTRICTED_CONE)?"RESTRICTED_CONE":"FULL_CONE"),
						   	rg_db.naptOut[outIdx].recordedInLimitCount,
						   	rg_db.naptOut[outIdx].naptrLookupHit,
						   	rg_db.naptOut[outIdx].forceExtPort
						   );
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
{
			rtk_rg_naptRecorded_flow_linkList_t *pFlowEntry, *pNextFlowEntry;
			ptrdiff_t flowEntryIdx;

			PROC_PRINTF("src_cpri_en: %d, src_cpri: %d, dst_cpri_en: %d, dst_cpri: %d\n", rg_db.naptOut[outIdx].src_cpri_en, rg_db.naptOut[outIdx].src_cpri, rg_db.naptOut[outIdx].dst_cpri_en, rg_db.naptOut[outIdx].dst_cpri);
			if(!_rtk_rg_list_head_empty(&rg_db.naptOut[outIdx].flowListHead))
			{
				_rtk_rg_list_for_each_entry_safe(pFlowEntry, pNextFlowEntry, &(rg_db.naptRecordedFlowList[0]), &rg_db.naptOut[outIdx].flowListHead, flow_idxList) //just return the first entry right behind of head
				{
					flowEntryIdx = _rtk_rg_list_entry_idx(pFlowEntry, &(rg_db.naptRecordedFlowList[0]));//use address-based method to get naptRecordedFlowList entry index. (naptRecordedFlowList[i] mapping to flow[i])
					if(rg_db.flow[flowEntryIdx].sw_valid==0)
						PROC_PRINTF("\033[1;33;41m[WARNING] Flow[%d] of naptOut[%d] is invalid. \033[0m\n", flowEntryIdx, outIdx);
					if(RG_PFLOW(flowEntryIdx)->path1.in_path==FB_PATH_5 && RG_PFLOW(flowEntryIdx)->path5.out_l4_act)
						PROC_PRINTF("%s flow [Idx: %u]\n", (RG_PFLOW(flowEntryIdx)->path5.out_l4_direction)?"Outbound":"Inbound", flowEntryIdx);
					else
					{
						PROC_PRINTF("\033[1;33;41m[WARNING] Flow[%d] of naptOut[%d] is not path5(napt). \033[0m\n", flowEntryIdx, outIdx);
						if(rg_db.flow[flowEntryIdx].sw_valid)
						{
							rtk_rg_asic_path1_entry_t *pP1Data = &RG_PFLOW(flowEntryIdx)->path1;
							uint32 idx = flowEntryIdx;
							if(pP1Data->in_path == FB_PATH_12)
							{
								if(pP1Data->in_multiple_act == 0)
									dump_flow_p1Rawdata(idx, (void*)pP1Data);
								else
									dump_flow_p2Rawdata(idx, (void*)pP1Data);
							}else if (pP1Data->in_path == FB_PATH_34)
							{
								if(pP1Data->in_multiple_act == 0)
									dump_flow_p3Rawdata(idx, (void*)pP1Data);
								else
									dump_flow_p4Rawdata(idx, (void*)pP1Data);
							}else if (pP1Data->in_path == FB_PATH_5)
							{
									dump_flow_p5Rawdata(idx, (void*)pP1Data);
							}else if (pP1Data->in_path == FB_PATH_6)
							{
									dump_flow_p6Rawdata(idx, (void*)pP1Data);
							}
							PROC_PRINTF("idleSecs: %u, static: %u, canBeReplaced: %u, isHairpinNat: %u, isLocalInNapt: %u, isTrapToPs: %u, isMulticast: %u, isUsedByFrag: %u, isSwOnly: %u\n", rg_db.flow[idx].idleSecs, rg_db.flow[idx].staticEntry, rg_db.flow[idx].canBeReplaced, rg_db.flow[idx].isHairpinNat, rg_db.flow[idx].isLocalInNapt, rg_db.flow[idx].isTrapToPs, rg_db.flow[idx].isMulticast, rg_db.flow[idx].isUsedByFrag, rg_db.flow[idx].isSwOnly);
							PROC_PRINTF("smacL2Idx: %u, dmacL2Idx: %u, naptOrTcpUdpGroupIdx: %d, arpOrNeighborIdx_src: %d, arpOrNeighborIdx_dst: %d, ingressPort: %u, egressTagAccType: %u\r\n\n", rg_db.flow[idx].smacL2Idx, rg_db.flow[idx].dmacL2Idx, rg_db.flow[idx].naptOrTcpUdpGroupIdx, rg_db.flow[idx].arpOrNeighborIdx_src, rg_db.flow[idx].arpOrNeighborIdx_dst, rg_db.flow[idx].ingressPort, rg_db.flow[idx].egressTagAccType);
							PROC_PRINTF("srcWlanDevIdx: %d, byDmac2cvid: %u\r\n\n", rg_db.flow[idx].srcWlanDevIdx, rg_db.flow[idx].byDmac2cvid);

						}
						PROC_PRINTF("\n");
					}
				}
			}
			else
				PROC_PRINTF("No any flow exist\n");
}
#endif
			if(	outIdx>=MAX_NAPT_OUT_HW_TABLE_SIZE)
				return 1; //SW entry
			if(	inIdx>=MAX_NAPT_IN_HW_TABLE_SIZE)
				return 1; //SW entry
			if((rg_db.naptOut[outIdx].state == TCP_CONNECTED)||(rg_db.naptOut[outIdx].state == UDP_CONNECTED)||
				(rg_db.naptOut[outIdx].state == FIRST_FIN)||(rg_db.naptOut[outIdx].state == RST_RECV))
				return 2; //HW entry
			return 1;

		}

	}
	return 0;
}

int32 dump_netstat(struct seq_file *s, void *v)
{
#if defined(CONFIG_RG_NAPT_NEW_EXTPORT_MECHANISM)
	int len=0, i;
	ptrdiff_t extPortIdx;
	rtk_rg_napt_extPort_record_t *pNaptExtPortRecord;

	PROC_PRINTF(">>Software netstat:\n");
	//TCP
	PROC_PRINTF(">>TCP\n");
	for(i=0; i<65536; i++)
	{
		if(rg_db.naptTcpExtPortRecord[i].refCnt==0 && rg_db.naptTcpExtPortRecord[i].refCnt_force==0)
			continue;
		PROC_PRINTF(" ExtPort[%5d]:\n", i);
		PROC_PRINTF("	 refCnt %d, internalIp %pI4, internalPort %d, refCnt_force %d\n",
					rg_db.naptTcpExtPortRecord[i].refCnt,
					&rg_db.naptTcpExtPortRecord[i].internalIp,
					rg_db.naptTcpExtPortRecord[i].internalPort,
					rg_db.naptTcpExtPortRecord[i].refCnt_force);
	}
	PROC_PRINTF("\n>>TCP list\n");
	for(i=0; i<4096; i++)
	{
		if(!list_empty(&rg_db.naptTcpExtPortRecord_head[i]))
		{
			PROC_PRINTF(" Hash idx[%4d]:\n", i);
			list_for_each_entry(pNaptExtPortRecord, &rg_db.naptTcpExtPortRecord_head[i], extPort_list)
			{
				extPortIdx = _rtk_rg_list_entry_idx(pNaptExtPortRecord, &rg_db.naptTcpExtPortRecord[0]);
				if(pNaptExtPortRecord->extPort_list.next != &rg_db.naptTcpExtPortRecord_head[i])
					PROC_PRINTF("	 ExtPort[%5d] ->\n", extPortIdx);
				else
					PROC_PRINTF("	 ExtPort[%5d]\n", extPortIdx);
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF("\n");
	//UDP
	PROC_PRINTF(">>UDP\n");
	for(i=0; i<65536; i++)
	{
		if(rg_db.naptUdpExtPortRecord[i].refCnt==0 && rg_db.naptUdpExtPortRecord[i].refCnt_force==0)
			continue;
		PROC_PRINTF(" ExtPort[%5d]:\n", i);
		PROC_PRINTF("	 refCnt %d, internalIp %pI4, internalPort %d, refCnt_force %d\n",
					rg_db.naptUdpExtPortRecord[i].refCnt,
					&rg_db.naptUdpExtPortRecord[i].internalIp,
					rg_db.naptUdpExtPortRecord[i].internalPort,
					rg_db.naptUdpExtPortRecord[i].refCnt_force);
	}
	PROC_PRINTF("\n>>UDP list\n");
	for(i=0; i<4096; i++)
	{
		if(!list_empty(&rg_db.naptUdpExtPortRecord_head[i]))
		{
			PROC_PRINTF(" Hash idx[%4d]:\n", i);
			list_for_each_entry(pNaptExtPortRecord, &rg_db.naptUdpExtPortRecord_head[i], extPort_list)
			{
				extPortIdx = _rtk_rg_list_entry_idx(pNaptExtPortRecord, &rg_db.naptUdpExtPortRecord[0]);
				if(pNaptExtPortRecord->extPort_list.next != &rg_db.naptUdpExtPortRecord_head[i])
					PROC_PRINTF("	 ExtPort[%5d] ->\n", extPortIdx);
				else
					PROC_PRINTF("	 ExtPort[%5d]\n", extPortIdx);
			}
			PROC_PRINTF("\n");
		}
	}

	PROC_PRINTF("\n\nTotal Entry: {Fwd:%d} {PS:%d}\n",atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]),atomic_read(&rg_db.naptProtcolStackEntryNumber[0])+atomic_read(&rg_db.naptProtcolStackEntryNumber[1]));

	return len;
#else
	int i;
	int portIdx;
	int validBit;
	int cnt=0;
	int len=0;

	PROC_PRINTF(">>Software netstat:\n");

	//TCP
	PROC_PRINTF("\nTCP\n");
	for(i=0;i<65536;i++)
	{
		portIdx=i>>5;
		validBit=i&31;
		if(rg_db.naptTcpExternPortUsed[portIdx]&(0x1<<validBit))
		{
			PROC_PRINTF("%d(%d),\t",i,rg_db.naptTcpExternPortUsedRefCount[i]);
			cnt++;
		}
	}
	PROC_PRINTF("\nTCP ref_cnt=%d\nUDP\n",cnt);
	cnt=0;
	//UDP
	for(i=0;i<65536;i++)
	{
		portIdx=i>>5;
		validBit=i&31;
		if(rg_db.naptUdpExternPortUsed[portIdx]&(0x1<<validBit))
		{
			PROC_PRINTF("%d(%d),\t",i,rg_db.naptUdpExternPortUsedRefCount[i]);
			cnt++;
		}
	}
	PROC_PRINTF("\nUDP ref_cnt=%d\n",cnt);
#if 1
	for(i=0;i<65536/32;i++)
	{
		if((rg_db.naptTcpExternPortUsed[i]!=0) || (rg_db.naptUdpExternPortUsed[i]!=0))
			PROC_PRINTF("[%d=>%x,%x]\t",i,rg_db.naptTcpExternPortUsed[i],rg_db.naptUdpExternPortUsed[i]);
	}
#endif

	PROC_PRINTF("\n\nTotal Entry: {Fwd:%d} {PS:%d}\n",atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]),atomic_read(&rg_db.naptProtcolStackEntryNumber[0])+atomic_read(&rg_db.naptProtcolStackEntryNumber[1]));

	return len;
#endif
}

#if 0 // debug only,  for field index width of shortcut
int32 dump_table_size(struct seq_file *s, void *v)
{
	PROC_PRINTF("rg_db: %d\n", sizeof(rg_db));

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("---Flow TABLE-------------------------------------------------------\n");
	PROC_PRINTF("MAX_FLOW_SW_TABLE_SIZE: %d\n", MAX_FLOW_SW_TABLE_SIZE);
	PROC_PRINTF("per flow entry size: %d + %d\n", sizeof(rtk_rg_table_flow_t), sizeof(rtk_rg_table_flowEntry_t));
	PROC_PRINTF("total flow talbe size: %d (%d x %d)\n", (sizeof(rtk_rg_table_flow_t)+sizeof(rtk_rg_table_flowEntry_t))*MAX_FLOW_SW_TABLE_SIZE, sizeof(rtk_rg_table_flow_t)+sizeof(rtk_rg_table_flowEntry_t), MAX_FLOW_SW_TABLE_SIZE);

	PROC_PRINTF("---Flow related TABLE-------------------------------------------------------\n");
	PROC_PRINTF("total sw flow list size: %d (%d x %d)\n", sizeof(rg_db.flowList), sizeof(rtk_rg_flow_linkList_t), MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE);
	PROC_PRINTF("total sw napt recorded flow list size: %d (%d x %d)\n", sizeof(rg_db.naptRecordedFlowList), sizeof(rtk_rg_naptRecorded_flow_linkList_t), MAX_FLOW_SW_TABLE_SIZE);
	PROC_PRINTF("total sw L2/L3 tracking group list size: %d (%d x %d)\n", sizeof(rg_db.flowTcpUdpTrackingGroupList), sizeof(rtk_rg_flow_tcpUdpTracking_group_linkList_t), MAX_FLOW_TCP_UDP_TRACKING_GROUP_SIZE);
	PROC_PRINTF("total sw L2/L3 tracking flow list size: %d (%d x %d)\n", sizeof(rg_db.flowTcpUdpTrackingList), sizeof(rtk_rg_flow_tcpUdpTracking_linkList_t), MAX_FLOW_SW_TABLE_SIZE);
#else	//1  not CONFIG_RG_FLOW_BASED_PLATFORM

	PROC_PRINTF("---SHORTCUT TABLE-------------------------------------------------------\n");
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	PROC_PRINTF("MAX_NAPT_SHORTCUT_SIZE_SHFIT: %d\n", MAX_NAPT_SHORTCUT_SIZE_SHIFT);
	PROC_PRINTF("MAX_NAPT_SHORTCUT_SIZE: %d\n", MAX_NAPT_SHORTCUT_SIZE);
	PROC_PRINTF("per v4 shortcut entry size: %d\n", sizeof(rtk_rg_napt_shortcut_t));
	PROC_PRINTF("total v4 shortcut talbe size: %d (%d x %d)\n", sizeof(rg_db.naptShortCut), sizeof(rtk_rg_napt_shortcut_t), MAX_NAPT_SHORTCUT_SIZE);
#endif

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	PROC_PRINTF("MAX_NAPT_V6_SHORTCUT_SIZE: %d\n", MAX_NAPT_V6_SHORTCUT_SIZE);
	PROC_PRINTF("per v6 shortcut entry size: %d\n", sizeof(rtk_rg_naptv6_shortcut_t));
	PROC_PRINTF("total v6 shortcut talbe size: %d (%d x %d)\n", sizeof(rg_db.naptv6ShortCut), sizeof(rtk_rg_naptv6_shortcut_t), MAX_NAPT_V6_SHORTCUT_SIZE);
#endif
	PROC_PRINTF("---SHORTCUT related TABLE-------------------------------------------------------\n");
	PROC_PRINTF("entries of v4ShortCutValidSet table: %d\n", MAX_NAPT_SHORTCUT_SIZE/32);
	PROC_PRINTF("per entry size of v4ShortCutValidSet: %d\n", sizeof(uint32));
	PROC_PRINTF("sizeof(v4ShortCutValidSet): %d (%d x %d)\n", sizeof(rg_db.v4ShortCutValidSet), sizeof(uint32), MAX_NAPT_SHORTCUT_SIZE/32);

	PROC_PRINTF("entries of v4ShortCut_lastAddIdx table: %d\n", MAX_NAPT_SHORTCUT_SIZE>>MAX_NAPT_SHORTCUT_WAYS_SHIFT);
	PROC_PRINTF("per entry size of v4ShortCut_lastAddIdx: %d\n", sizeof(uint8));
	PROC_PRINTF("sizeof(v4ShortCut_lastAddIdx): %d (%d x %d)\n", sizeof(rg_db.v4ShortCut_lastAddIdx), sizeof(uint8), MAX_NAPT_SHORTCUT_SIZE>>MAX_NAPT_SHORTCUT_WAYS_SHIFT);

	PROC_PRINTF("entries of v6ShortCutValidSet table: %d\n", MAX_NAPT_V6_SHORTCUT_SIZE/32);
	PROC_PRINTF("per entry size of v6ShortCutValidSet: %d\n", sizeof(uint32));
	PROC_PRINTF("sizeof(v6ShortCutValidSet): %d (%d x %d)\n", sizeof(rg_db.v6ShortCutValidSet), sizeof(uint32), MAX_NAPT_V6_SHORTCUT_SIZE/32);

	PROC_PRINTF("entries of v6ShortCut_lastAddIdx table: %d\n", MAX_NAPT_V6_SHORTCUT_SIZE>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT);
	PROC_PRINTF("per entry size of v6ShortCut_lastAddIdx: %d\n", sizeof(uint8));
	PROC_PRINTF("sizeof(v6ShortCut_lastAddIdx): %d (%d x %d)\n", sizeof(rg_db.v6ShortCut_lastAddIdx), sizeof(uint8), MAX_NAPT_V6_SHORTCUT_SIZE>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT);
#endif	// end CONFIG_RG_FLOW_BASED_PLATFORM

	PROC_PRINTF("---NAPT TABLE-------------------------------------------------------\n");
#ifdef CONFIG_RG_SW_NAPT_SIZE_L
	PROC_PRINTF("CONFIG_RG_SW_NAPT_SIZE_L is defined\n");
#else
	PROC_PRINTF("CONFIG_RG_SW_NAPT_SIZE_L is undefined\n");
#endif
	PROC_PRINTF("per napt-in entry size: %d\n", sizeof(rtk_rg_table_naptIn_t));
	PROC_PRINTF("MAX_NAPT_IN_SW_TABLE_SIZE: %d\n", MAX_NAPT_IN_SW_TABLE_SIZE);
	PROC_PRINTF("total napt-in table size: %d (%d x %d)\n", sizeof(rg_db.naptIn), sizeof(rtk_rg_table_naptIn_t), MAX_NAPT_IN_SW_TABLE_SIZE);

	PROC_PRINTF("per napt-out entry size: %d\n", sizeof(rtk_rg_table_naptOut_t));
	PROC_PRINTF("MAX_NAPT_OUT_SW_TABLE_SIZE: %d\n", MAX_NAPT_OUT_SW_TABLE_SIZE);
	PROC_PRINTF("total napt-out table size: %d (%d x %d)\n", sizeof(rg_db.naptOut), sizeof(rtk_rg_table_naptOut_t), MAX_NAPT_OUT_SW_TABLE_SIZE);
	PROC_PRINTF("---NAPT related TABLE-------------------------------------------------------\n");
	PROC_PRINTF("CONFIG_RG_SW_NAPT_SIZE_L is defined\n");

	PROC_PRINTF("entries of NAPT-in free list: %d\n", MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE);
	PROC_PRINTF("per entry size of NAPT-in free list: %d\n", sizeof(rtk_rg_table_naptIn_linkList_t));
	PROC_PRINTF("naptInFreeList talbe size: %d (%d x %d)\n", sizeof(rg_db.naptInFreeList), sizeof(rtk_rg_table_naptIn_linkList_t), MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE);

	PROC_PRINTF("entries of NAPT-out free list: %d\n", MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE);
	PROC_PRINTF("per entry size of NAPT-out free list: %d\n", sizeof(rtk_rg_table_naptOut_linkList_t));
	PROC_PRINTF("naptOutFreeList talbe size: %d (%d x %d)\n", sizeof(rg_db.naptOutFreeList), sizeof(rtk_rg_table_naptOut_linkList_t), MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE);

	PROC_PRINTF("entries of NAPT valid set table: %d\n", MAX_NAPT_OUT_SW_TABLE_SIZE/32);
	PROC_PRINTF("sizeof(naptValidSet): %d\n", sizeof(rg_db.naptValidSet));

	PROC_PRINTF("entries of tcpShortTimeoutRing: %d\n", MAX_NAPT_OUT_SW_TABLE_SIZE);
	PROC_PRINTF("per entry size of tcpShortTimeoutRing: %d\n", sizeof(rtk_rg_tcpShotTimeout_t));
	PROC_PRINTF("sizeof(tcpShortTimeoutRing): %d\n", sizeof(rg_db.tcpShortTimeoutRing));

	PROC_PRINTF("-------------------------------------------------------\n");
#if 0
	PROC_PRINTF("naptShortCut+naptv6ShortCut+v4ShortCutValidSet+v4ShortCut_lastAddIdx+v6ShortCutValidSet+v6ShortCut_lastAddIdx: %d\n",
		sizeof(rg_db.naptShortCut)+sizeof(rg_db.naptv6ShortCut)+sizeof(rg_db.v4ShortCutValidSet)+\
		sizeof(rg_db.v4ShortCut_lastAddIdx)+sizeof(rg_db.v6ShortCutValidSet)+sizeof(rg_db.v6ShortCut_lastAddIdx));

	PROC_PRINTF("naptIn+naptOut+naptInFreeList+naptOutFreeList+naptValidSet+tcpShortTimeoutRing: %d\n",
		sizeof(rg_db.naptIn)+sizeof(rg_db.naptOut)+sizeof(rg_db.naptInFreeList)+\
		sizeof(rg_db.naptOutFreeList)+sizeof(rg_db.naptValidSet)+sizeof(rg_db.tcpShortTimeoutRing));
#else
#if 0
	PROC_PRINTF("total size of above tables: %d\n",
		sizeof(rg_db.naptShortCut)+sizeof(rg_db.naptv6ShortCut)+sizeof(rg_db.v4ShortCutValidSet)+\
		sizeof(rg_db.v4ShortCut_lastAddIdx)+sizeof(rg_db.v6ShortCutValidSet)+sizeof(rg_db.v6ShortCut_lastAddIdx)+\
		sizeof(rg_db.naptIn)+sizeof(rg_db.naptOut)+sizeof(rg_db.naptInFreeList)+\
		sizeof(rg_db.naptOutFreeList)+sizeof(rg_db.naptValidSet)+sizeof(rg_db.tcpShortTimeoutRing));
#endif
#endif
	PROC_PRINTF("-------------------------------------------------------\n");
	PROC_PRINTF("MAX_VLAN_SW_TABLE_SIZE: %d\n", MAX_VLAN_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_LUT_SW_TABLE_SIZE: %d\n", MAX_LUT_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_NETIF_SW_TABLE_SIZE: %d\n", MAX_NETIF_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_L3_SW_TABLE_SIZE: %d\n", MAX_L3_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_EXTIP_SW_TABLE_SIZE: %d\n", MAX_EXTIP_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_NEXTHOP_SW_TABLE_SIZE: %d\n", MAX_NEXTHOP_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_PPPOE_SW_TABLE_SIZE: %d\n", MAX_PPPOE_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_ARP_SW_TABLE_SIZE: %d\n", MAX_ARP_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_WANTYPE_SW_TABLE_SIZE: %d\n", MAX_WANTYPE_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_BIND_SW_TABLE_SIZE: %d\n", MAX_BIND_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_IPV6_ROUTING_SW_TABLE_SIZE: %d\n", MAX_IPV6_ROUTING_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_NETIF_SW_TABLE_SIZE: %d\n", MAX_NETIF_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE: %d\n", MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE);
#if !defined(CONFIG_RG_RTL9607C_SERIES)
	PROC_PRINTF("MAX_IPMCFILTER_HW_TABLE_SIZE: %d\n", MAX_IPMCFILTER_HW_TABLE_SIZE);
#endif
	PROC_PRINTF("MAX_IPMCGRP_SW_TABLE_SIZE: %d\n", MAX_IPMCGRP_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_WLAN_MBSSID_SW_TABLE_SIZE: %d\n", MAX_WLAN_MBSSID_SW_TABLE_SIZE);

#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("MAX_DSLITE_SW_TABLE_SIZE: %d\n", MAX_DSLITE_SW_TABLE_SIZE);
#endif
	PROC_PRINTF("MAX_DSLITEMC_SW_TABLE_SIZE: %d\n", MAX_DSLITEMC_SW_TABLE_SIZE);
	PROC_PRINTF("MAX_STATIC_ROUTE_SIZE: %d\n", MAX_STATIC_ROUTE_SIZE);
	PROC_PRINTF("MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE: %d\n", MAX_NAPT_OUT_SW_TABLE_SIZE-MAX_NAPT_OUT_HW_TABLE_SIZE);
	PROC_PRINTF("MAX_NAPT_OUT_HW_TABLE_SIZE: %d\n", MAX_NAPT_OUT_HW_TABLE_SIZE);
	PROC_PRINTF("MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE: %d\n", MAX_NAPT_IN_SW_TABLE_SIZE-MAX_NAPT_IN_HW_TABLE_SIZE);
	PROC_PRINTF("MAX_NAPT_IN_HW_TABLE_SIZE: %d\n", MAX_NAPT_IN_HW_TABLE_SIZE);
	PROC_PRINTF("MAX_ICMPCTRLFLOW_SIZE: %d\n", MAX_ICMPCTRLFLOW_SIZE);
	PROC_PRINTF("MAX_VLAN_SW_TABLE_SIZE: %d\n", MAX_VLAN_SW_TABLE_SIZE);

	PROC_PRINTF("-----field width of shortcut entry--------------------------------------------------------\n");
	PROC_PRINTF("BFW_SPA			: %d\n", BFW_SPA);
	PROC_PRINTF("BFW_DIR			: %d\n", BFW_DIR);
	PROC_PRINTF("BFW_ISTCP			: %d\n", BFW_ISTCP);
	PROC_PRINTF("BFW_ISNAPT 		: %d\n", BFW_ISNAPT);
	PROC_PRINTF("BFW_ISBRIDGE		: %d\n", BFW_ISBRIDGE);
	PROC_PRINTF("BFW_ISHAIRPINNAT	: %d\n", BFW_ISHAIRPINNAT);
	PROC_PRINTF("BFW_NOTFINUPDATE	: %d\n", BFW_NOTFINUPDATE);
	PROC_PRINTF("BFW_VLANTAGIF		: %d\n", BFW_VLANTAGIF);
	PROC_PRINTF("BFW_SVLANTAGIF 	: %d\n", BFW_SVLANTAGIF);
	PROC_PRINTF("BFW_DMAC2CVLANTAGIF: %d\n", BFW_DMAC2CVLANTAGIF);
	PROC_PRINTF("BFW_TAGACCTYPE 	: %d\n", BFW_TAGACCTYPE);
	PROC_PRINTF("BFW_INTERVLANID	: %d\n", BFW_INTERVLANID);
	PROC_PRINTF("BFW_VLANID 		: %d\n", BFW_VLANID);
	PROC_PRINTF("BFW_DMAC2CVLANID	: %d\n", BFW_DMAC2CVLANID);
	PROC_PRINTF("BFW_DSCP			: %d\n", BFW_DSCP);
	PROC_PRINTF("BFW_PRIORITY		: %d\n", BFW_PRIORITY);
	PROC_PRINTF("BFW_SPRIORITY		: %d\n", BFW_SPRIORITY);
	PROC_PRINTF("BFW_INTERCFPRI 	: %d\n", BFW_INTERCFPRI);
	PROC_PRINTF("BFW_SVLANID		: %d\n", BFW_SVLANID);
	PROC_PRINTF("BFW_STREAMID		: %d\n", BFW_STREAMID);
	PROC_PRINTF("BFW_SMACL2IDX		: %d\n", BFW_SMACL2IDX);
	PROC_PRINTF("BFW_MACPORT		: %d\n", BFW_MACPORT);
	PROC_PRINTF("BFW_EXTPORT		: %d\n", BFW_EXTPORT);
	PROC_PRINTF("BFW_NEIGHBORIDX	: %d\n", BFW_NEIGHBORIDX);
	PROC_PRINTF("BFW_IDLETIME		: %d\n", BFW_IDLETIME);
	PROC_PRINTF("BFW_NAPTIDX		: %d\n", BFW_NAPTIDX);
	PROC_PRINTF("BFW_INTFIDX		: %d\n", BFW_INTFIDX);
	PROC_PRINTF("BFW_EIPIDX 		: %d\n", BFW_EIPIDX);
	PROC_PRINTF("BFW_LUTIDX 		: %d\n", BFW_LUTIDX);
	PROC_PRINTF("BFW_ARPIDX 		: %d\n", BFW_ARPIDX);
	PROC_PRINTF("BFW_UNIPORTMASK	: %d\n", BFW_UNIPORTMASK);

    return 0;
}
#endif
int32 dump_meminfo(struct seq_file *s, void *v)
{
	uint32 temp;
	PROC_PRINTF("rg_db:     %10d (%5d KB)\n", sizeof(rg_db), (sizeof(rg_db)/1024));
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("rgpro_db:  %10d (%5d KB)\n", sizeof(rgpro_db), sizeof(rgpro_db)/1024);
#endif
	PROC_PRINTF("rg_kernel: %10d (%5d KB)\n", sizeof(rg_kernel), sizeof(rg_kernel)/1024);


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("===Flow TABLE======================================================\n");
	temp = sizeof(rtk_rg_table_flow_t) + sizeof(rtk_rg_table_flowEntry_t);
	PROC_PRINTF("each flow entry needs %d (%d + %d) bytes\n", temp, sizeof(rtk_rg_table_flow_t), sizeof(rtk_rg_table_flowEntry_t));
	PROC_PRINTF("flow:                          %8d (%4d btyes * %6d entrys)\n", temp * MAX_FLOW_SW_TABLE_SIZE, temp, MAX_FLOW_SW_TABLE_SIZE);
	temp *= MAX_FLOW_SW_TABLE_SIZE;
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);


	PROC_PRINTF("===Flow Related TABLE/LIST=========================================\n");
	temp = 0;
	PROC_PRINTF("table size is related to SW flow entry count:\n");
#if defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("flowValidSet:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowValidSet), sizeof(rg_db.flowValidSet[0]), sizeof(rg_db.flowValidSet)/sizeof(rg_db.flowValidSet[0]));
	temp += sizeof(rg_db.flowValidSet);
#endif //end of defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("flowList:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowList), sizeof(rg_db.flowList[0]), sizeof(rg_db.flowList)/sizeof(rg_db.flowList[0]));
	PROC_PRINTF("naptRecordedFlowList:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptRecordedFlowList), sizeof(rg_db.naptRecordedFlowList[0]), sizeof(rg_db.naptRecordedFlowList)/sizeof(rg_db.naptRecordedFlowList[0]));
	PROC_PRINTF("flowTcpUdpTrackingList:        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowTcpUdpTrackingList), sizeof(rg_db.flowTcpUdpTrackingList[0]), sizeof(rg_db.flowTcpUdpTrackingList)/sizeof(rg_db.flowTcpUdpTrackingList[0]));
	temp += sizeof(rg_db.flowList) + sizeof(rg_db.naptRecordedFlowList) + sizeof(rg_db.flowTcpUdpTrackingList);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);

	temp = 0;
	PROC_PRINTF("table size is NOT related to SW flow entry count:\n");
#if!defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("flowValidSet:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowValidSet), sizeof(rg_db.flowValidSet[0]), sizeof(rg_db.flowValidSet)/sizeof(rg_db.flowValidSet[0]));
	temp += sizeof(rg_db.flowValidSet);
#endif //end of !defined(CONFIG_RG_G3_SERIES)
#if defined(CONFIG_RG_FLOW_4K_MODE)
	PROC_PRINTF("flowTcamListHead:              %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowTcamListHead), sizeof(rg_db.flowTcamListHead[0]), sizeof(rg_db.flowTcamListHead)/sizeof(rg_db.flowTcamListHead[0]));
	PROC_PRINTF("flowTcamList:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowTcamList), sizeof(rg_db.flowTcamList[0]), sizeof(rg_db.flowTcamList)/sizeof(rg_db.flowTcamList[0]));
	temp += sizeof(rg_db.flowTcamListHead) + sizeof(rg_db.flowTcamList);
#endif  //CONFIG_RG_FLOW_4K_MODE
	PROC_PRINTF("flowListHead:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowListHead), sizeof(rg_db.flowListHead[0]), sizeof(rg_db.flowListHead)/sizeof(rg_db.flowListHead[0]));
	PROC_PRINTF("flowTcpUdpTrackingGroupHead:   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowTcpUdpTrackingGroupHead), sizeof(rg_db.flowTcpUdpTrackingGroupHead[0]), sizeof(rg_db.flowTcpUdpTrackingGroupHead)/sizeof(rg_db.flowTcpUdpTrackingGroupHead[0]));
	PROC_PRINTF("flowTcpUdpTrackingGroupList:   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowTcpUdpTrackingGroupList), sizeof(rg_db.flowTcpUdpTrackingGroupList[0]), sizeof(rg_db.flowTcpUdpTrackingGroupList)/sizeof(rg_db.flowTcpUdpTrackingGroupList[0]));
	temp += sizeof(rg_db.flowListHead) + sizeof(rg_db.flowTcpUdpTrackingGroupHead) + sizeof(rg_db.flowTcpUdpTrackingGroupList);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);

#else //!defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("===SHORTCUT TABLE==================================================\n");
	temp = 0;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	PROC_PRINTF("v4ShortCutValidSet:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v4ShortCutValidSet), sizeof(rg_db.v4ShortCutValidSet[0]), sizeof(rg_db.v4ShortCutValidSet)/sizeof(rg_db.v4ShortCutValidSet[0]));
	PROC_PRINTF("v4ShortCut_lastAddIdx:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v4ShortCut_lastAddIdx), sizeof(rg_db.v4ShortCut_lastAddIdx[0]), sizeof(rg_db.v4ShortCut_lastAddIdx)/ sizeof(rg_db.v4ShortCut_lastAddIdx[0]));
	PROC_PRINTF("naptShortCut:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptShortCut), sizeof(rg_db.naptShortCut[0]), sizeof(rg_db.naptShortCut)/sizeof(rg_db.naptShortCut[0]));
	temp += sizeof(rg_db.v4ShortCutValidSet) + sizeof(rg_db.v4ShortCut_lastAddIdx) + sizeof(rg_db.naptShortCut);
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	PROC_PRINTF("v6ShortCutValidSet:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v6ShortCutValidSet), sizeof(rg_db.v6ShortCutValidSet[0]), sizeof(rg_db.v6ShortCutValidSet)/sizeof(rg_db.v6ShortCutValidSet[0]));
	PROC_PRINTF("v6ShortCut_lastAddIdx:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v6ShortCut_lastAddIdx), sizeof(rg_db.v6ShortCut_lastAddIdx[0]), sizeof(rg_db.v6ShortCut_lastAddIdx)/sizeof(rg_db.v6ShortCut_lastAddIdx[0]));
	PROC_PRINTF("naptv6ShortCut:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptv6ShortCut), sizeof(rg_db.naptv6ShortCut[0]), sizeof(rg_db.naptv6ShortCut)/sizeof(rg_db.naptv6ShortCut[0]));
	temp += sizeof(rg_db.v6ShortCutValidSet) + sizeof(rg_db.v6ShortCut_lastAddIdx) + sizeof(rg_db.naptv6ShortCut);
#endif
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);
#endif //end of defined(CONFIG_RG_FLOW_BASED_PLATFORM)


	PROC_PRINTF("===NAPT TABLE======================================================\n");
	temp = 0;
	PROC_PRINTF("naptIn:                        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptIn), sizeof(rg_db.naptIn[0]), sizeof(rg_db.naptIn)/sizeof(rg_db.naptIn[0]));
	PROC_PRINTF("naptOut:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptOut), sizeof(rg_db.naptOut[0]), sizeof(rg_db.naptOut)/sizeof(rg_db.naptOut[0]));
	temp += sizeof(rg_db.naptIn) + sizeof(rg_db.naptOut);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);


	PROC_PRINTF("===NAPT Related TABLE/LIST==============================================\n");
	temp = 0;
	PROC_PRINTF("table size is related to SW napt entry count:\n");
	PROC_PRINTF("naptValidSet:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptValidSet), sizeof(rg_db.naptValidSet[0]), sizeof(rg_db.naptValidSet)/sizeof(rg_db.naptValidSet[0]));
	PROC_PRINTF("naptInFreeList:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptInFreeList), sizeof(rg_db.naptInFreeList[0]), sizeof(rg_db.naptInFreeList)/sizeof(rg_db.naptInFreeList[0]));
	PROC_PRINTF("naptOutFreeList:               %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptOutFreeList), sizeof(rg_db.naptOutFreeList[0]), sizeof(rg_db.naptOutFreeList)/sizeof(rg_db.naptOutFreeList[0]));
	PROC_PRINTF("tcpShortTimeoutRing:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.tcpShortTimeoutRing), sizeof(rg_db.tcpShortTimeoutRing[0]), sizeof(rg_db.tcpShortTimeoutRing)/sizeof(rg_db.tcpShortTimeoutRing[0]));
	temp += sizeof(rg_db.naptValidSet) + sizeof(rg_db.naptInFreeList) + sizeof(rg_db.naptOutFreeList) + sizeof(rg_db.tcpShortTimeoutRing);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);

	temp = 0;
	PROC_PRINTF("table size is NOT related to SW napt entry count:\n");
#if defined(CONFIG_RG_NAPT_NEW_EXTPORT_MECHANISM)
	PROC_PRINTF("naptTcpExtPortRecord:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptTcpExtPortRecord), sizeof(rg_db.naptTcpExtPortRecord[0]), sizeof(rg_db.naptTcpExtPortRecord)/sizeof(rg_db.naptTcpExtPortRecord[0]));
	PROC_PRINTF("naptUdpExtPortRecord:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptUdpExtPortRecord), sizeof(rg_db.naptUdpExtPortRecord[0]), sizeof(rg_db.naptUdpExtPortRecord)/sizeof(rg_db.naptUdpExtPortRecord[0]));
	PROC_PRINTF("naptTcpExtPortRecord_head:    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptTcpExtPortRecord_head), sizeof(rg_db.naptTcpExtPortRecord_head[0]),  sizeof(rg_db.naptTcpExtPortRecord_head)/sizeof(rg_db.naptTcpExtPortRecord_head[0]));
	PROC_PRINTF("naptUdpExtPortRecord_head:    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptUdpExtPortRecord_head), sizeof(rg_db.naptUdpExtPortRecord_head[0]), sizeof(rg_db.naptUdpExtPortRecord_head)/sizeof(rg_db.naptUdpExtPortRecord_head[0]));
	temp += sizeof(rg_db.naptTcpExtPortRecord) + sizeof(rg_db.naptUdpExtPortRecord) + sizeof(rg_db.naptTcpExtPortRecord_head) + sizeof(rg_db.naptUdpExtPortRecord_head);
#else
	PROC_PRINTF("naptTcpExternPortUsed:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptTcpExternPortUsed), sizeof(rg_db.naptTcpExternPortUsed[0]), sizeof(rg_db.naptTcpExternPortUsed)/sizeof(rg_db.naptTcpExternPortUsed[0]));
	PROC_PRINTF("naptUdpExternPortUsed:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptUdpExternPortUsed), sizeof(rg_db.naptUdpExternPortUsed[0]), sizeof(rg_db.naptUdpExternPortUsed)/sizeof(rg_db.naptUdpExternPortUsed[0]));
	PROC_PRINTF("naptTcpExternPortUsedRefCount: %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptTcpExternPortUsedRefCount), sizeof(rg_db.naptTcpExternPortUsedRefCount[0]),  sizeof(rg_db.naptTcpExternPortUsedRefCount)/sizeof(rg_db.naptTcpExternPortUsedRefCount[0]));
	PROC_PRINTF("naptUdpExternPortUsedRefCount: %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.naptUdpExternPortUsedRefCount), sizeof(rg_db.naptUdpExternPortUsedRefCount[0]), sizeof(rg_db.naptUdpExternPortUsedRefCount)/sizeof(rg_db.naptUdpExternPortUsedRefCount[0]));
	temp += sizeof(rg_db.naptTcpExternPortUsed) + sizeof(rg_db.naptUdpExternPortUsed) + sizeof(rg_db.naptTcpExternPortUsedRefCount) + sizeof(rg_db.naptUdpExternPortUsedRefCount);
#endif
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	PROC_PRINTF("ipv6naptTcpExternPortUsed:     %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv6naptTcpExternPortUsed), sizeof(rg_db.ipv6naptTcpExternPortUsed[0]), sizeof(rg_db.ipv6naptTcpExternPortUsed)/sizeof(rg_db.ipv6naptTcpExternPortUsed[0]));
	PROC_PRINTF("ipv6naptUdpExternPortUsed:     %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv6naptUdpExternPortUsed), sizeof(rg_db.ipv6naptUdpExternPortUsed[0]), sizeof(rg_db.ipv6naptUdpExternPortUsed)/sizeof(rg_db.ipv6naptUdpExternPortUsed[0]));
	temp += sizeof(rg_db.ipv6naptTcpExternPortUsed) + sizeof(rg_db.ipv6naptUdpExternPortUsed);
#endif
	PROC_PRINTF("pNaptInHashListHead:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.pNaptInHashListHead), sizeof(rg_db.pNaptInHashListHead[0]), sizeof(rg_db.pNaptInHashListHead)/sizeof(rg_db.pNaptInHashListHead[0]));
	PROC_PRINTF("pNaptOutHashListHead:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.pNaptOutHashListHead), sizeof(rg_db.pNaptOutHashListHead[0]), sizeof(rg_db.pNaptOutHashListHead)/sizeof(rg_db.pNaptOutHashListHead[0]));
	temp += sizeof(rg_db.pNaptInHashListHead) + sizeof(rg_db.pNaptOutHashListHead);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);


	PROC_PRINTF("===OTHER TABLE=====================================================\n");
	temp = 0;
	PROC_PRINTF("netif:                         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.netif), sizeof(rg_db.netif[0]), sizeof(rg_db.netif)/sizeof(rg_db.netif[0]));
	PROC_PRINTF("extip:                         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.extip), sizeof(rg_db.extip[0]), sizeof(rg_db.extip)/sizeof(rg_db.extip[0]));
	PROC_PRINTF("l3:                            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.l3), sizeof(rg_db.l3[0]), sizeof(rg_db.l3)/sizeof(rg_db.l3[0]));
	PROC_PRINTF("arp:                           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.arp), sizeof(rg_db.arp[0]), sizeof(rg_db.arp)/sizeof(rg_db.arp[0]));
	PROC_PRINTF("nexthop:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.nexthop), sizeof(rg_db.nexthop[0]), sizeof(rg_db.nexthop)/sizeof(rg_db.nexthop[0]));
	PROC_PRINTF("pppoe:                         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.pppoe), sizeof(rg_db.pppoe[0]), sizeof(rg_db.pppoe)/sizeof(rg_db.pppoe[0]));
	PROC_PRINTF("wantype:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.wantype), sizeof(rg_db.wantype[0]), sizeof(rg_db.wantype)/sizeof(rg_db.wantype[0]));
	PROC_PRINTF("bind:                          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.bind), sizeof(rg_db.bind[0]), sizeof(rg_db.bind)/sizeof(rg_db.bind[0]));
	PROC_PRINTF("v6route:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v6route), sizeof(rg_db.v6route[0]), sizeof(rg_db.v6route)/sizeof(rg_db.v6route[0]));
	PROC_PRINTF("v6Extip:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v6Extip), sizeof(rg_db.v6Extip[0]), sizeof(rg_db.v6Extip)/sizeof(rg_db.v6Extip[0]));
	PROC_PRINTF("v6neighbor:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.v6neighbor), sizeof(rg_db.v6neighbor[0]), sizeof(rg_db.v6neighbor)/sizeof(rg_db.v6neighbor[0]));
	PROC_PRINTF("vlan:                          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.vlan), sizeof(rg_db.vlan[0]), sizeof(rg_db.vlan)/sizeof(rg_db.vlan[0]));
	PROC_PRINTF("lut:                           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lut), sizeof(rg_db.lut[0]), sizeof(rg_db.lut)/sizeof(rg_db.lut[0]));
	PROC_PRINTF("tempL3Table:                   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.tempL3Table), sizeof(rg_db.tempL3Table[0]), sizeof(rg_db.tempL3Table)/sizeof(rg_db.tempL3Table[0]));
	PROC_PRINTF("arpTrfIndicator:               %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.arpTrfIndicator), sizeof(rg_db.arpTrfIndicator[0]), sizeof(rg_db.arpTrfIndicator)/sizeof(rg_db.arpTrfIndicator[0]));
	temp += sizeof(rg_db.netif) + sizeof(rg_db.extip) + sizeof(rg_db.l3) + sizeof(rg_db.arp) + sizeof(rg_db.nexthop) + sizeof(rg_db.pppoe) + sizeof(rg_db.wantype) + sizeof(rg_db.bind) + sizeof(rg_db.v6route) + sizeof(rg_db.v6Extip) + sizeof(rg_db.v6neighbor) + sizeof(rg_db.vlan) + sizeof(rg_db.lut) +  sizeof(rg_db.tempL3Table) + sizeof(rg_db.arpTrfIndicator);
#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("dslite:                        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.dslite), sizeof(rg_db.dslite[0]), sizeof(rg_db.dslite)/sizeof(rg_db.dslite[0]));
	temp += sizeof(rg_db.dslite);
#endif
	PROC_PRINTF("wlanMbssid:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.wlanMbssid), sizeof(rg_db.wlanMbssid[0]), sizeof(rg_db.wlanMbssid)/sizeof(rg_db.wlanMbssid[0]));
	PROC_PRINTF("staticRoute:                   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.staticRoute), sizeof(rg_db.staticRoute[0]), sizeof(rg_db.staticRoute)/sizeof(rg_db.staticRoute[0]));
#if defined(CONFIG_RG_WAN_MSS_CACHE)
	PROC_PRINTF("msscache:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.msscache), sizeof(rg_db.msscache[0]), sizeof(rg_db.msscache)/sizeof(rg_db.msscache[0]));
	temp += sizeof(rg_db.msscache);
#endif
	PROC_PRINTF("routingVlanInfoArray_1:        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.routingVlanInfoArray_1), sizeof(rg_db.routingVlanInfoArray_1[0]), sizeof(rg_db.routingVlanInfoArray_1)/sizeof(rg_db.routingVlanInfoArray_1[0]));
	PROC_PRINTF("routingVlanInfoArray_2:        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.routingVlanInfoArray_2), sizeof(rg_db.routingVlanInfoArray_2[0]), sizeof(rg_db.routingVlanInfoArray_2)/sizeof(rg_db.routingVlanInfoArray_2[0]));
	PROC_PRINTF("routingArpInfoArray_1:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.routingArpInfoArray_1), sizeof(rg_db.routingArpInfoArray_1[0]), sizeof(rg_db.routingArpInfoArray_1)/sizeof(rg_db.routingArpInfoArray_1[0]));
	PROC_PRINTF("routingArpInfoArray_2:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.routingArpInfoArray_2), sizeof(rg_db.routingArpInfoArray_2[0]), sizeof(rg_db.routingArpInfoArray_2)/sizeof(rg_db.routingArpInfoArray_2[0]));
	temp += sizeof(rg_db.wlanMbssid) + sizeof(rg_db.staticRoute) + sizeof(rg_db.routingVlanInfoArray_1) + sizeof(rg_db.routingVlanInfoArray_2) + sizeof(rg_db.routingArpInfoArray_1) + sizeof(rg_db.routingArpInfoArray_2);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("extPortTbl:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.extPortTbl), sizeof(rg_db.extPortTbl[0]), sizeof(rg_db.extPortTbl)/sizeof(rg_db.extPortTbl[0]));
	PROC_PRINTF("indMacTbl:                     %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.indMacTbl), sizeof(rg_db.indMacTbl[0]), sizeof(rg_db.indMacTbl)/sizeof(rg_db.indMacTbl[0]));
	PROC_PRINTF("ethTypeTbl:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ethTypeTbl), sizeof(rg_db.ethTypeTbl[0]), sizeof(rg_db.ethTypeTbl)/sizeof(rg_db.ethTypeTbl[0]));
	PROC_PRINTF("wanAccessLimit:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.wanAccessLimit), sizeof(rg_db.wanAccessLimit[0]), sizeof(rg_db.wanAccessLimit)/sizeof(rg_db.wanAccessLimit[0]));
	PROC_PRINTF("flowMIBTbl:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.flowMIBTbl), sizeof(rg_db.flowMIBTbl[0]), sizeof(rg_db.flowMIBTbl)/sizeof(rg_db.flowMIBTbl[0]));
	temp += sizeof(rg_db.extPortTbl) + sizeof(rg_db.indMacTbl) + sizeof(rg_db.ethTypeTbl) + sizeof(rg_db.wanAccessLimit) + sizeof(rg_db.flowMIBTbl);
#else
	PROC_PRINTF("arpValidSet:                   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.arpValidSet), sizeof(rg_db.arpValidSet[0]), sizeof(rg_db.arpValidSet)/sizeof(rg_db.arpValidSet[0]));
	PROC_PRINTF("neighborValidSet:              %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.neighborValidSet), sizeof(rg_db.neighborValidSet[0]), sizeof(rg_db.neighborValidSet)/sizeof(rg_db.neighborValidSet[0]));
	PROC_PRINTF("arpTableCopied:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.arpTableCopied), sizeof(rg_db.arpTableCopied[0]), sizeof(rg_db.arpTableCopied)/sizeof(rg_db.arpTableCopied[0]));
	PROC_PRINTF("tempArpTable:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.tempArpTable), sizeof(rg_db.tempArpTable[0]), sizeof(rg_db.tempArpTable)/sizeof(rg_db.tempArpTable[0]));
	temp += sizeof(rg_db.arpValidSet) + sizeof(rg_db.neighborValidSet) + sizeof(rg_db.arpTableCopied) + sizeof(rg_db.tempArpTable);
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	PROC_PRINTF("ipv6FragmentQueue:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv6FragmentQueue), sizeof(rg_db.ipv6FragmentQueue[0]), sizeof(rg_db.ipv6FragmentQueue)/sizeof(rg_db.ipv6FragmentQueue[0]));
	temp += sizeof(rg_db.ipv6FragmentQueue);
#endif
	PROC_PRINTF("lutValidSet:                   %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lutValidSet), sizeof(rg_db.lutValidSet[0]), sizeof(rg_db.lutValidSet)/sizeof(rg_db.lutValidSet[0]));
	PROC_PRINTF("virtualServer:                 %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.virtualServer), sizeof(rg_db.virtualServer[0]), sizeof(rg_db.virtualServer)/sizeof(rg_db.virtualServer[0]));
	PROC_PRINTF("upnp:                          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.upnp), sizeof(rg_db.upnp[0]), sizeof(rg_db.upnp)/sizeof(rg_db.upnp[0]));
	temp += sizeof(rg_db.lutValidSet) + sizeof(rg_db.virtualServer) + sizeof(rg_db.upnp);
#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
	PROC_PRINTF("dmzInfo:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.dmzInfo),sizeof(rg_db.dmzInfo[0]), sizeof(rg_db.dmzInfo)/sizeof(rg_db.dmzInfo[0]));
	temp += sizeof(rg_db.dmzInfo);
#endif
	PROC_PRINTF("gatewayServicePortEntry:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.gatewayServicePortEntry), sizeof(rg_db.gatewayServicePortEntry[0]), sizeof(rg_db.gatewayServicePortEntry)/sizeof(rg_db.gatewayServicePortEntry[0]));
	PROC_PRINTF("ipv4FragmentQueue:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv4FragmentQueue), sizeof(rg_db.ipv4FragmentQueue[0]), sizeof(rg_db.ipv4FragmentQueue)/sizeof(rg_db.ipv4FragmentQueue[0]));
	temp += sizeof(rg_db.gatewayServicePortEntry) + sizeof(rg_db.ipv4FragmentQueue);
	PROC_PRINTF("layer2NextOfNewestCountIdx:    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.layer2NextOfNewestCountIdx), sizeof(rg_db.layer2NextOfNewestCountIdx[0]), sizeof(rg_db.layer2NextOfNewestCountIdx)/sizeof(rg_db.layer2NextOfNewestCountIdx[0]));
	temp += sizeof(rg_db.layer2NextOfNewestCountIdx);
#if defined(CONFIG_RG_RTL9600_SERIES)
	PROC_PRINTF("congestionCtrlRing:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.congestionCtrlRing), sizeof(rg_db.congestionCtrlRing[0]), sizeof(rg_db.congestionCtrlRing)/sizeof(rg_db.congestionCtrlRing[0]));
	temp += sizeof(rg_db.congestionCtrlRing);
#endif
	PROC_PRINTF("nexthop_lan_table:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.nexthop_lan_table), sizeof(rg_db.nexthop_lan_table[0]), sizeof(rg_db.nexthop_lan_table)/sizeof(rg_db.nexthop_lan_table[0]));
	PROC_PRINTF("algL2TPExternTulIDUsed:        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algL2TPExternTulIDUsed), sizeof(rg_db.algL2TPExternTulIDUsed[0]), sizeof(rg_db.algL2TPExternTulIDUsed)/sizeof(rg_db.algL2TPExternTulIDUsed[0]));
	temp += sizeof(rg_db.nexthop_lan_table) + sizeof(rg_db.algL2TPExternTulIDUsed);
	PROC_PRINTF("aclDecisionBackup:             %8d \n", sizeof(rg_db.aclDecisionBackup));
	temp += sizeof(rg_db.aclDecisionBackup);
#if !defined(CONFIG_RG_RTL9607C_SERIES) && !defined(CONFIG_RG_G3_SERIES) && !defined(CONFIG_RG_RTL9603CVD_SERIES)
	PROC_PRINTF("ipmcfilter          :          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipmcfilter), sizeof(rg_db.ipmcfilter[0]), sizeof(rg_db.ipmcfilter)/sizeof(rg_db.ipmcfilter[0]));
	temp += sizeof(rg_db.ipmcfilter);
#endif
	PROC_PRINTF("ipmcgrp:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipmcgrp), sizeof(rg_db.ipmcgrp[0]), sizeof(rg_db.ipmcgrp)/sizeof(rg_db.ipmcgrp[0]));
	PROC_PRINTF("dsliteMc:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.dsliteMc), sizeof(rg_db.dsliteMc[0]), sizeof(rg_db.dsliteMc)/sizeof(rg_db.dsliteMc[0]));
	temp += sizeof(rg_db.ipmcgrp) + sizeof(rg_db.dsliteMc);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("mcflowIdxtbl:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.mcflowIdxtbl),sizeof(rg_db.mcflowIdxtbl[0]), sizeof(rg_db.mcflowIdxtbl)/sizeof(rg_db.mcflowIdxtbl[0]));
	temp += sizeof(rg_db.mcflowIdxtbl);
#if defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("mcHwEngineInfoTbl:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.mcHwEngineInfoTbl), sizeof(rg_db.mcHwEngineInfoTbl[0]), sizeof(rg_db.mcHwEngineInfoTbl)/sizeof(rg_db.mcHwEngineInfoTbl[0]));
	temp += sizeof(rg_db.mcHwEngineInfoTbl);
#endif
#endif
	PROC_PRINTF("algServInLanIpMapping:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algServInLanIpMapping), sizeof(rg_db.algServInLanIpMapping[0]), sizeof(rg_db.algServInLanIpMapping)/sizeof(rg_db.algServInLanIpMapping[0]));
	PROC_PRINTF("algTcpExternPortEnabled:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algTcpExternPortEnabled), sizeof(rg_db.algTcpExternPortEnabled[0]), sizeof(rg_db.algTcpExternPortEnabled)/sizeof(rg_db.algTcpExternPortEnabled[0]));
	PROC_PRINTF("algUdpExternPortEnabled:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algUdpExternPortEnabled), sizeof(rg_db.algUdpExternPortEnabled[0]), sizeof(rg_db.algUdpExternPortEnabled)/sizeof(rg_db.algUdpExternPortEnabled[0]));
	PROC_PRINTF("algTcpExtPortEnabled_SrvInLan: %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algTcpExternPortEnabled_SrvInLan), sizeof(rg_db.algTcpExternPortEnabled_SrvInLan[0]), sizeof(rg_db.algTcpExternPortEnabled_SrvInLan)/sizeof(rg_db.algTcpExternPortEnabled_SrvInLan[0]));
	PROC_PRINTF("algUdpExtPortEnabled_SrvInLan: %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algUdpExternPortEnabled_SrvInLan), sizeof(rg_db.algUdpExternPortEnabled_SrvInLan[0]), sizeof(rg_db.algUdpExternPortEnabled_SrvInLan)/sizeof(rg_db.algUdpExternPortEnabled_SrvInLan[0]));
	PROC_PRINTF("algTcpFunctionMapping:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algTcpFunctionMapping), sizeof(rg_db.algTcpFunctionMapping[0]), sizeof(rg_db.algTcpFunctionMapping)/sizeof(rg_db.algTcpFunctionMapping[0]));
	PROC_PRINTF("algUdpFunctionMapping:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algUdpFunctionMapping), sizeof(rg_db.algUdpFunctionMapping[0]), sizeof(rg_db.algUdpFunctionMapping)/sizeof(rg_db.algUdpFunctionMapping[0]));
	PROC_PRINTF("algUserDefinedPort:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algUserDefinedPort), sizeof(rg_db.algUserDefinedPort[0]), sizeof(rg_db.algUserDefinedPort)/sizeof(rg_db.algUserDefinedPort[0]));
	PROC_PRINTF("algUserDefinedTimeout:         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algUserDefinedTimeout), sizeof(rg_db.algUserDefinedTimeout[0]), sizeof(rg_db.algUserDefinedTimeout)/sizeof(rg_db.algUserDefinedTimeout[0]));
	temp += sizeof(rg_db.algServInLanIpMapping) + sizeof(rg_db.algTcpExternPortEnabled) + sizeof(rg_db.algUdpExternPortEnabled) + sizeof(rg_db.algTcpExternPortEnabled_SrvInLan) + sizeof(rg_db.algUdpExternPortEnabled_SrvInLan);
	temp += sizeof(rg_db.algTcpFunctionMapping) + sizeof(rg_db.algUdpFunctionMapping) + sizeof(rg_db.algUserDefinedPort) + sizeof(rg_db.algUserDefinedTimeout);
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	PROC_PRINTF("algBnetSCClient:               %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algBnetSCClient), sizeof(rg_db.algBnetSCClient[0]), sizeof(rg_db.algBnetSCClient)/sizeof(rg_db.algBnetSCClient[0]));
	temp += sizeof(rg_db.algBnetSCClient);
#endif
#if defined(CONFIG_RG_RTL9600_SERIES)
	PROC_PRINTF("hw_v6route:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_v6route), sizeof(rg_db.hw_v6route[0]), sizeof(rg_db.hw_v6route)/sizeof(rg_db.hw_v6route[0]));
	PROC_PRINTF("hw_netif:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_netif), sizeof(rg_db.hw_netif[0]), sizeof(rg_db.hw_netif)/sizeof(rg_db.hw_netif[0]));
	PROC_PRINTF("hw_extip:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_extip), sizeof(rg_db.hw_extip[0]), sizeof(rg_db.hw_extip)/sizeof(rg_db.hw_extip[0]));
	PROC_PRINTF("hw_l3:                         %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_l3), sizeof(rg_db.hw_l3[0]), sizeof(rg_db.hw_l3)/sizeof(rg_db.hw_l3[0]));
	PROC_PRINTF("hw_pppoe:                      %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_pppoe), sizeof(rg_db.hw_pppoe[0]), sizeof(rg_db.hw_pppoe)/sizeof(rg_db.hw_pppoe[0]));
	PROC_PRINTF("hw_wantype:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_wantype), sizeof(rg_db.hw_wantype[0]), sizeof(rg_db.hw_wantype)/sizeof(rg_db.hw_wantype[0]));
	PROC_PRINTF("hw_nexthop:                    %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_nexthop), sizeof(rg_db.hw_nexthop[0]), sizeof(rg_db.hw_nexthop)/sizeof(rg_db.hw_nexthop[0]));
	PROC_PRINTF("hw_bind:                       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hw_bind), sizeof(rg_db.hw_bind[0]), sizeof(rg_db.hw_bind)/sizeof(rg_db.hw_bind[0]));
	temp += sizeof(rg_db.hw_v6route) + sizeof(rg_db.hw_netif) + sizeof(rg_db.hw_extip) + sizeof(rg_db.hw_l3) + sizeof(rg_db.hw_pppoe) + sizeof(rg_db.hw_wantype) + sizeof(rg_db.hw_nexthop) + sizeof(rg_db.hw_bind);
#endif
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);


	PROC_PRINTF("===OTHER LIST=====================================================\n");
	temp = 0;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	PROC_PRINTF("L2L3FragHashListHead:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.L2L3FragHashListHead), sizeof(rg_db.L2L3FragHashListHead[0]), sizeof(rg_db.L2L3FragHashListHead)/sizeof(rg_db.L2L3FragHashListHead[0]));
	PROC_PRINTF("L2L3FragList:                  %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.L2L3FragList), sizeof(rg_db.L2L3FragList[0]), sizeof(rg_db.L2L3FragList)/sizeof(rg_db.L2L3FragList[0]));
	PROC_PRINTF("L2L3FragQueueList:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.L2L3FragQueueList), sizeof(rg_db.L2L3FragQueueList[0]), sizeof(rg_db.L2L3FragQueueList)/sizeof(rg_db.L2L3FragQueueList[0]));
	temp +=  sizeof(rg_db.L2L3FragHashListHead) + sizeof(rg_db.L2L3FragList) + sizeof(rg_db.L2L3FragQueueList);
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
		PROC_PRINTF("ipv6Layer4FreeList:			%8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv6Layer4FreeList), sizeof(rg_db.ipv6Layer4FreeList[0]), sizeof(rg_db.ipv6Layer4FreeList)/sizeof(rg_db.ipv6Layer4FreeList[0]));
		PROC_PRINTF("ipv6Layer4HashListHead:		%8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.ipv6Layer4HashListHead), sizeof(rg_db.ipv6Layer4HashListHead[0]), sizeof(rg_db.ipv6Layer4HashListHead)/sizeof(rg_db.ipv6Layer4HashListHead[0]));
		temp += sizeof(rg_db.ipv6Layer4FreeList) + sizeof(rg_db.ipv6Layer4HashListHead);
#endif
	PROC_PRINTF("fragOutFreeList:               %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.fragOutFreeList), sizeof(rg_db.fragOutFreeList[0]), sizeof(rg_db.fragOutFreeList)/sizeof(rg_db.fragOutFreeList[0]));
	PROC_PRINTF("fragOutHashListHead:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.fragOutHashListHead), sizeof(rg_db.fragOutHashListHead[0]), sizeof(rg_db.fragOutHashListHead)/sizeof(rg_db.fragOutHashListHead[0]));
	PROC_PRINTF("fragInFreeList:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.fragInFreeList), sizeof(rg_db.fragInFreeList[0]), sizeof(rg_db.fragInFreeList)/sizeof(rg_db.fragInFreeList[0]));
	PROC_PRINTF("fragInHashListHead:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.fragInHashListHead), sizeof(rg_db.fragInHashListHead[0]), sizeof(rg_db.fragInHashListHead)/sizeof(rg_db.fragInHashListHead[0]));
	temp += sizeof(rg_db.fragOutFreeList) + sizeof(rg_db.fragOutHashListHead) + sizeof(rg_db.fragInFreeList) + sizeof(rg_db.fragInHashListHead);
	PROC_PRINTF("softwareArpTableHead:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.softwareArpTableHead), sizeof(rg_db.softwareArpTableHead[0]), sizeof(rg_db.softwareArpTableHead)/sizeof(rg_db.softwareArpTableHead[0]));
	PROC_PRINTF("softwareArpFreeList:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.softwareArpFreeList), sizeof(rg_db.softwareArpFreeList[0]), sizeof(rg_db.softwareArpFreeList)/sizeof(rg_db.softwareArpFreeList[0]));
	temp += sizeof(rg_db.softwareArpTableHead) + sizeof(rg_db.softwareArpFreeList);
#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("hardwareArpTableHead:			%8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hardwareArpTableHead), sizeof(rg_db.hardwareArpTableHead[0]), sizeof(rg_db.hardwareArpTableHead)/sizeof(rg_db.hardwareArpTableHead[0]));
	PROC_PRINTF("hardwareArpFreeList:			%8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hardwareArpFreeList), sizeof(rg_db.hardwareArpFreeList[0]), sizeof(rg_db.hardwareArpFreeList)/sizeof(rg_db.hardwareArpFreeList[0]));
	temp += sizeof(rg_db.hardwareArpTableHead) + sizeof(rg_db.hardwareArpFreeList);
#endif
	PROC_PRINTF("softwareLutTableHead:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.softwareLutTableHead),  sizeof(rg_db.softwareLutTableHead[0]),  sizeof(rg_db.softwareLutTableHead)/sizeof(rg_db.softwareLutTableHead[0]));
	PROC_PRINTF("softwareLutFreeList:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.softwareLutFreeList), sizeof(rg_db.softwareLutFreeList[0]), sizeof(rg_db.softwareLutFreeList)/sizeof(rg_db.softwareLutFreeList[0]));
	temp += sizeof(rg_db.softwareLutTableHead) + sizeof(rg_db.softwareLutFreeList);
#if !defined(CONFIG_RG_RTL9600_SERIES) //support lut traffic bit
	PROC_PRINTF("lutBCAMTableHead:              %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lutBCAMTableHead), sizeof(rg_db.lutBCAMTableHead[0]), sizeof(rg_db.lutBCAMTableHead)/sizeof(rg_db.lutBCAMTableHead[0]));
	temp += sizeof(rg_db.lutBCAMTableHead);
#endif
	PROC_PRINTF("lutBCAMLinkList:               %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lutBCAMLinkList), sizeof(rg_db.lutBCAMLinkList[0]), sizeof(rg_db.lutBCAMLinkList)/sizeof(rg_db.lutBCAMLinkList[0]));
	PROC_PRINTF("lutGroupLinkList:              %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lutGroupLinkList), sizeof(rg_db.lutGroupLinkList[0]), sizeof(rg_db.lutGroupLinkList)/sizeof(rg_db.lutGroupLinkList[0]));
	PROC_PRINTF("lutGroupTableHead:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.lutGroupTableHead), sizeof(rg_db.lutGroupTableHead[0]), sizeof(rg_db.lutGroupTableHead)/sizeof(rg_db.lutGroupTableHead[0]));
	PROC_PRINTF("vlanBindingListHead:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.vlanBindingListHead), sizeof(rg_db.vlanBindingListHead[0]), sizeof(rg_db.vlanBindingListHead)/sizeof(rg_db.vlanBindingListHead[0]));
	temp += sizeof(rg_db.lutBCAMLinkList) + sizeof(rg_db.lutGroupTableHead) + sizeof(rg_db.lutGroupLinkList) + sizeof(rg_db.vlanBindingListHead);
	PROC_PRINTF("redirectHttpURLFreeList:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.redirectHttpURLFreeList),sizeof(rg_db.redirectHttpURLFreeList[0]), sizeof(rg_db.redirectHttpURLFreeList)/sizeof(rg_db.redirectHttpURLFreeList[0]));
	PROC_PRINTF("redirectHttpWhiteListFreeList: %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.redirectHttpWhiteListFreeList),  sizeof(rg_db.redirectHttpWhiteListFreeList[0]),  sizeof(rg_db.redirectHttpWhiteListFreeList)/sizeof(rg_db.redirectHttpWhiteListFreeList[0]));
	PROC_PRINTF("avoidPortalURLFreeList:        %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.avoidPortalURLFreeList), sizeof(rg_db.avoidPortalURLFreeList[0]), sizeof(rg_db.avoidPortalURLFreeList)/sizeof(rg_db.avoidPortalURLFreeList[0]));
	temp +=  sizeof(rg_db.redirectHttpURLFreeList) + sizeof(rg_db.redirectHttpWhiteListFreeList) + sizeof(rg_db.avoidPortalURLFreeList);
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	PROC_PRINTF("vmacSkbFreeList:				%8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.vmacSkbFreeList), sizeof(rg_db.vmacSkbFreeList[0]), sizeof(rg_db.vmacSkbFreeList)/sizeof(rg_db.vmacSkbFreeList[0]));
	temp += sizeof(rg_db.vmacSkbFreeList);
#endif
#if !defined(CONFIG_RG_RTL9600_SERIES)//support host policy
	PROC_PRINTF("hostPoliceList:                %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.hostPoliceList), sizeof(rg_db.hostPoliceList[0]), sizeof(rg_db.hostPoliceList)/sizeof(rg_db.hostPoliceList[0]));
	temp += sizeof(rg_db.hostPoliceList);
#endif
	PROC_PRINTF("algL2TPCtrlFlowLinkList:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algL2TPCtrlFlowLinkList), sizeof(rg_db.algL2TPCtrlFlowLinkList[0]), sizeof(rg_db.algL2TPCtrlFlowLinkList)/sizeof(rg_db.algL2TPCtrlFlowLinkList[0]));
	PROC_PRINTF("igmpWhiteList:                 %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.igmpWhiteList), sizeof(rg_db.igmpWhiteList[0]), sizeof(rg_db.igmpWhiteList)/sizeof(rg_db.igmpWhiteList[0]));
	PROC_PRINTF("igmpBlackList:                 %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.igmpBlackList), sizeof(rg_db.igmpBlackList[0]), sizeof(rg_db.igmpBlackList)/sizeof(rg_db.igmpBlackList[0]));
	temp += sizeof(rg_db.algL2TPCtrlFlowLinkList) + sizeof(rg_db.igmpWhiteList) + sizeof(rg_db.igmpBlackList);
	PROC_PRINTF("icmpCtrlFlowLinkList:          %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.icmpCtrlFlowLinkList), sizeof(rg_db.icmpCtrlFlowLinkList[0]), sizeof(rg_db.icmpCtrlFlowLinkList)/sizeof(rg_db.icmpCtrlFlowLinkList[0]));
	temp += sizeof(rg_db.icmpCtrlFlowLinkList);
	PROC_PRINTF("vlanBindingFreeList:           %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.vlanBindingFreeList), sizeof(rg_db.vlanBindingFreeList[0]), sizeof(rg_db.vlanBindingFreeList)/sizeof(rg_db.vlanBindingFreeList[0]));
	temp += sizeof(rg_db.vlanBindingFreeList);
	PROC_PRINTF("algDynamicFreeList:            %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algDynamicFreeList), sizeof(rg_db.algDynamicFreeList[0]), sizeof(rg_db.algDynamicFreeList)/sizeof(rg_db.algDynamicFreeList[0]));
	PROC_PRINTF("algPreExtFreeList:             %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.algPreExtFreeList), sizeof(rg_db.algPreExtFreeList[0]), sizeof(rg_db.algPreExtFreeList)/sizeof(rg_db.algPreExtFreeList[0]));
	temp += sizeof(rg_db.algDynamicFreeList) + sizeof(rg_db.algPreExtFreeList);
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);


	PROC_PRINTF("===OTHER VARIABLE=================================================\n");
	temp = 0;
	PROC_PRINTF("systemGlobal:                  %8d \n", sizeof(rg_db.systemGlobal));
	temp += sizeof(rg_db.systemGlobal);
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==1)
	PROC_PRINTF("  => ipset_entry:              %8d (%4d btyes * %6d entrys * %6d sets)\n", sizeof(rg_db.systemGlobal.ipset_entry), sizeof(rg_db.systemGlobal.ipset_entry[0][0]), sizeof(rg_db.systemGlobal.ipset_entry[0])/sizeof(rg_db.systemGlobal.ipset_entry[0][0]), sizeof(rg_db.systemGlobal.ipset_entry)/sizeof(rg_db.systemGlobal.ipset_entry[0]));
	PROC_PRINTF("  => ipset_list:               %8d (%4d btyes * %6d entrys * %6d sets)\n", (sizeof(rg_db.systemGlobal.ipsetListHead)+sizeof(rg_db.systemGlobal.ipsetHashListHead)+sizeof(rg_db.systemGlobal.ipsetFreeListHead)), sizeof(rg_db.systemGlobal.ipsetHashListHead[0][0]), ((sizeof(rg_db.systemGlobal.ipsetHashListHead[0])/sizeof(rg_db.systemGlobal.ipsetHashListHead[0][0]))+2), (sizeof(rg_db.systemGlobal.ipsetHashListHead)/sizeof(rg_db.systemGlobal.ipsetHashListHead[0])));
#endif
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
	PROC_PRINTF("  => ipsets_entry:             %8d (%4d btyes * %6d entrys * %3d sets)\n", sizeof(rg_db.systemGlobal.ipsets_entry), sizeof(rg_db.systemGlobal.ipsets_entry[0][0]), sizeof(rg_db.systemGlobal.ipsets_entry[0])/sizeof(rg_db.systemGlobal.ipsets_entry[0][0]), sizeof(rg_db.systemGlobal.ipsets_entry)/sizeof(rg_db.systemGlobal.ipsets_entry[0]));
	PROC_PRINTF("  => ipsets_setCfg:            %8d (%4d btyes * %6d sets)\n", sizeof(rg_db.systemGlobal.ipsets_setCfg), sizeof(rg_db.systemGlobal.ipsets_setCfg[0]), sizeof(rg_db.systemGlobal.ipsets_setCfg)/sizeof(rg_db.systemGlobal.ipsets_setCfg[0]));
	PROC_PRINTF("  => ipsets_grpCfg:            %8d (%4d btyes * %6d groups)\n", sizeof(rg_db.systemGlobal.ipsets_grpCfg), sizeof(rg_db.systemGlobal.ipsets_grpCfg[0]), sizeof(rg_db.systemGlobal.ipsets_grpCfg)/sizeof(rg_db.systemGlobal.ipsets_grpCfg[0]));
	PROC_PRINTF("  => ipsets_list:              %8d (%4d btyes * (%d groups + %d sets * (%d hash +1)))\n", (sizeof(rg_db.systemGlobal.ipsetsGroupListHead)+sizeof(rg_db.systemGlobal.ipsetsFreeListHead)+sizeof(rg_db.systemGlobal.ipsetsHashListHead)), sizeof(rg_db.systemGlobal.ipsetsGroupListHead[0]), MAX_IPSETS_GROUP_SIZE, MAX_IPSETS_IDX_SIZE, MAX_IPSETS_HASH_SIZE);
#endif
	PROC_PRINTF("  => acl_SW_table_entry:       %8d (%4d btyes * %6d entrys)\n", sizeof(rg_db.systemGlobal.acl_SW_table_entry), sizeof(rg_db.systemGlobal.acl_SW_table_entry[0]), sizeof(rg_db.systemGlobal.acl_SW_table_entry)/sizeof(rg_db.systemGlobal.acl_SW_table_entry[0]));
	PROC_PRINTF("  => acl_filter_temp+valid:    %8d (%4d btyes * %6d entrys)\n", (sizeof(rg_db.systemGlobal.acl_filter_temp) + sizeof(rg_db.systemGlobal.acl_filter_temp_valid)), sizeof(rg_db.systemGlobal.acl_filter_temp[0]), sizeof(rg_db.systemGlobal.acl_filter_temp)/sizeof(rg_db.systemGlobal.acl_filter_temp[0]));
	PROC_PRINTF("  => acl_SWindex_sorting:      %8d (%4d btyes * %6d entrys)\n", (sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight) + sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action)), (sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[0]) + sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[0])), sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight)/sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[0]));
	PROC_PRINTF("                                    Total: %8d bytes (%5d KB)\n", temp, temp/1024);

	return 0;
}

int32 dump_rg_shareMeter(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval = RT_ERR_RG_OK;
	int i;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;
	rtk_rate_metet_mode_t meterMode;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	PROC_PRINTF("funcbasedMeter Mode: %s \n", (rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_NOT_INIT)?"RTK_RG_METERMODE_NOT_INIT":((rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)?"RTK_RG_METERMODE_HW_INDEX":"RTK_RG_METERMODE_SW_INDEX"));
	if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_SW_INDEX)
	{
		PROC_PRINTF("\033[1;33mfuncbasedMeter Mode is RTK_RG_METERMODE_SW_INDEX. Please use /proc/dump/funcbasedMeter to get meter setting.\033[0m\n");
		return retval;
	}
#endif

	PROC_PRINTF("=== HW meter ===\n");
	for(i = 0 ; i < MAX_SHAREMETER_TABLE_SIZE; i++)
	{
		retval = rtk_rg_apollo_shareMeter_get(i, &rate, &ifgInclude);
		if(retval== RT_ERR_RG_OK)
		{
			PROC_PRINTF("shareMeter[%2d]: rate=%d, ifgInclude=%s", i, rate, ifgInclude?"Include":"Exclude");

		}
		else
		{
			PROC_PRINTF("Get shareMeter[%d] failed. return value = %x", i, retval);
		}

		retval = rtk_rg_apollo_shareMeterMode_get(i, &meterMode);
		if(retval== RT_ERR_RG_OK)
		{
			if(meterMode > METER_MODE_PACKET_RATE)
				PROC_PRINTF(" (meterMode: Unknown meterMode %d)", i, meterMode);
			else
				PROC_PRINTF(" (meterMode: %s)", (meterMode==METER_MODE_BIT_RATE)?"BIT RATE MODE":"PACKET RATE MODE");
		}
		else
		{
			PROC_PRINTF(" (Get meterMode failed. return value = %x)", i, retval);
		}
		PROC_PRINTF("\n");
	}


	PROC_PRINTF("=== SW meter ===\n");
	for(i = 0 ; i < PURE_SW_SHAREMETER_TABLE_SIZE; i++)
	{
		retval = rtk_rg_apollo_shareMeter_get(i+PURE_SW_METER_IDX_OFFSET, &rate, &ifgInclude);
		if(retval== RT_ERR_RG_OK)
		{
			PROC_PRINTF("shareMeter[%2d]: rate=%d, ifgInclude=%s", i+PURE_SW_METER_IDX_OFFSET, rate, ifgInclude?"Include":"Exclude");

		}
		else
		{
			PROC_PRINTF("Get shareMeter[%d] failed. return value = %x", i+PURE_SW_METER_IDX_OFFSET, retval);
		}

		retval = rtk_rg_apollo_shareMeterMode_get(i+PURE_SW_METER_IDX_OFFSET, &meterMode);
		if(retval== RT_ERR_RG_OK)
		{
			if(meterMode > METER_MODE_PACKET_RATE)
				PROC_PRINTF(" (meterMode: Unknown meterMode %d)", i+PURE_SW_METER_IDX_OFFSET, meterMode);
			else
				PROC_PRINTF(" (meterMode: %s)", (meterMode==METER_MODE_BIT_RATE)?"BIT RATE MODE":"PACKET RATE MODE");

		}
		else
		{
			PROC_PRINTF(" (Get meterMode failed. return value = %x)", i+PURE_SW_METER_IDX_OFFSET, retval);
		}
		PROC_PRINTF("\n");
	}
	PROC_PRINTF("\n");
	return retval;
}

int dump_swShaper_table(struct seq_file *s, void *v)
{
	int i = 0, j = 0;

	for(i=0; i<MAX_SW_SHAPER_TABLE_SIZE; i++)
	{
		if(rg_db.systemGlobal.systemMeter.shapingCtrl[i].enququcnt != 0)
		{
			PROC_PRINTF("========== shaper[%d] ========== rate:%d kbps\n", i, rg_db.systemGlobal.systemMeter.swMeter[i].rate);
			PROC_PRINTF("\tjiffies=%lu lastJiffies=%lu lastJiffiesPersec=%lu timerCnt=%d\n", jiffies, 
							rg_db.systemGlobal.systemMeter.swMeter[i].lastJiffies, 
							rg_db.systemGlobal.systemMeter.swMeter[i].lastJiffiesPersec, 
							rg_db.systemGlobal.systemMeter.shapingCtrl[i].timerCnt);
			PROC_PRINTF("\tfree:%d sched:%d congestion:%d enqcnt:%d deqcnt:%d\n", 
						atomic_read(&rg_db.systemGlobal.systemMeter.shapingCtrl[i].free_idx), atomic_read(&rg_db.systemGlobal.systemMeter.shapingCtrl[i].sched_idx), 
						atomic_read(&rg_db.systemGlobal.systemMeter.shapingCtrl[i].congestion), rg_db.systemGlobal.systemMeter.shapingCtrl[i].enququcnt, rg_db.systemGlobal.systemMeter.shapingCtrl[i].pausetxcnt);
			for(j=0; j<MAX_SW_SHAPER_QUEUE_SIZE; j++)
				PROC_PRINTF("queue[%d]: wifiTx %d skbLen %d state %d\n", j, 
							rg_db.systemGlobal.systemMeter.shapingCtrl[i].queue[j].wifiTx,
							rg_db.systemGlobal.systemMeter.shapingCtrl[i].queue[j].skbLen,
							atomic_read(&rg_db.systemGlobal.systemMeter.shapingCtrl[i].queue[j].state));
		}
	}
	
	return SUCCESS;
}

int32 dump_sw_napt(struct seq_file *s, void *v)
{
    uint32 idx=0,ret,naptHWNum=0,naptSWNum=0, naptSWListNum=0, naptSWListFreeNum=0;
	rtk_rg_table_naptOut_linkList_t	*pNaptOutLinkList;
	int len=0;
	ptrdiff_t naptOutLinkListIdx;

    PROC_PRINTF(">>Software NAPT OUT TCP/UDP Table:\n");
    for(idx=0; idx<MAX_NAPT_OUT_HW_TABLE_SIZE; idx++)
    {
		int outIdx=idx;
		ret = dump_napt_entry(outIdx,outIdx>>2,s);
		if(ret==2) naptHWNum++;
		if(ret==1) naptSWNum++;
    }

	PROC_PRINTF(">>Software Link List NAPT OUT TCP/UDP Table:\n");

	for(idx=0;idx<MAX_NAPT_OUT_HASH_SIZE;idx++)
	{
		pNaptOutLinkList=rg_db.pNaptOutHashListHead[idx];
		while(pNaptOutLinkList!=NULL)
		{
			naptOutLinkListIdx = _rtk_rg_list_entry_idx(pNaptOutLinkList, &(rg_db.naptOutFreeList[0])) + MAX_NAPT_OUT_HW_TABLE_SIZE; //use address-based method to get naptOutFreeList entry index. (naptOutFreeList[i] mapping to naptOut[i+MAX_NAPT_OUT_HW_TABLE_SIZE])
			naptSWListNum++;
			ret = dump_napt_entry(naptOutLinkListIdx,idx,s);
			pNaptOutLinkList=pNaptOutLinkList->pNext;
			if(ret==2) naptHWNum++;
			if(ret==1) naptSWNum++;
		}
	}

	//20160707LUKE: display software free napt statistic.
	pNaptOutLinkList = rg_db.pNaptOutFreeListHead;
	while(pNaptOutLinkList!=NULL)
	{
		naptSWListFreeNum++;
		pNaptOutLinkList = pNaptOutLinkList->pNext;
	}


	PROC_PRINTF("Total Entry: {OUT_HW:%d} {OUT_SW:%d} {OUT_LIST_SW:%d, OUT_LIST_SW_FREE:%d} {Fwd:%d} {PS:%d}\n",naptHWNum,naptSWNum,naptSWListNum,naptSWListFreeNum,atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]),atomic_read(&rg_db.naptProtcolStackEntryNumber[0])+atomic_read(&rg_db.naptProtcolStackEntryNumber[1]));

    return len;
}

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#ifdef CONFIG_ROME_NAPT_SHORTCUT
void dump_ipv4_shortcut(uint32 idx, struct seq_file *s)
{
	rtk_rg_napt_shortcut_t *pNaptSc=&rg_db.naptShortCut[idx];

	if(RG_V4SC_VALID(idx)) //valid
	{
		PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n 	 sip=%d.%d.%d.%d dip=%d.%d.%d.%d sport=%d dport=%d isTcp=%d spa=%d notFinishUpdated=%d hairpin=%d\n"
			,idx
			,(pNaptSc->isLocalInNapt==1)?"Local in napt/routing":(pNaptSc->isBridge==1)?"L2":((pNaptSc->isNapt==1?"L4":"L3"))
			,pNaptSc->idleSecs
			,(pNaptSc->sip>>24)&0xff,(pNaptSc->sip>>16)&0xff,(pNaptSc->sip>>8)&0xff,(pNaptSc->sip)&0xff
			,(pNaptSc->dip>>24)&0xff,(pNaptSc->dip>>16)&0xff,(pNaptSc->dip>>8)&0xff,(pNaptSc->dip)&0xff
			,pNaptSc->sport
			,pNaptSc->dport
			,pNaptSc->isTcp
			,pNaptSc->spa
			,pNaptSc->notFinishUpdated
			,pNaptSc->isHairpinNat);
		PROC_PRINTF("	   [ACT] dir=%s smacL2Idx=%d lut_idx=%d intf_idx=%d eip_idx=%d naptIdx=%d pri=%d spri=%d intVlanId=%d intCFPri=%d dscp=%d uniPortmask=0x%x naptFilterRateIdx=%d naptFilterPktCntIdx=%d naptFilterByteCntIdx=%d naptFilterCopyCnt=%d\n"
			,(pNaptSc->direction==NAPT_DIRECTION_OUTBOUND)?"OUT":((pNaptSc->direction==NAPT_DIRECTION_INBOUND)?"IN":"X")
			,pNaptSc->smacL2Idx
			,pNaptSc->new_lut_idx
			,pNaptSc->new_intf_idx
			,pNaptSc->new_eip_idx
			,pNaptSc->naptIdx
			,pNaptSc->priority
			,pNaptSc->servicePriority
			,pNaptSc->internalVlanID
			,pNaptSc->internalCFPri
			,pNaptSc->dscp
			,pNaptSc->uniPortmask
			,pNaptSc->naptFilterRateLimitIdx
			,pNaptSc->naptFilterPktCntIdx
			,pNaptSc->naptFilterByteCnttIdx
			,pNaptSc->naptFilterCopyCnt);


		PROC_PRINTF("	   vlanID=%d vlanTagif=%d svlanID=%d svlanTagif=%d dmac2cvlanID=%d dmac2cvlanTagif=%d tagAccType=%d arpIdx=%d streamID=%d\n"
			,pNaptSc->vlanID
			,pNaptSc->vlanTagif
			,pNaptSc->serviceVlanID
			,pNaptSc->serviceVlanTagif
			,pNaptSc->dmac2cvlanID
			,pNaptSc->dmac2cvlanTagif
			,pNaptSc->tagAccType
			,pNaptSc->arpIdx
#ifdef CONFIG_GPON_FEATURE
			,pNaptSc->streamID
#else
			,0
#endif
			);
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF("	   mibDirect=%d mibNetifIdx=%d mibTagDelta=%d\n"
			,pNaptSc->mibDirect
			,pNaptSc->mibNetifIdx
			,pNaptSc->mibTagDelta);
#endif
	}
}

#endif

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
void dump_ipv6_shortcut(uint32 idx, struct seq_file *s)
{
	rtk_rg_naptv6_shortcut_t *pV6NaptSc=&rg_db.naptv6ShortCut[idx];

	if(RG_V6SC_VALID(idx)) //valid
	{
		PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n 	 sip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x dip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x sport=%d dport=%d isTcp=%d spa=%d notFinishUpdated=%d\n"
			,idx
			,(pV6NaptSc->isBridge==1)?"L2":"L3"
			,pV6NaptSc->idleSecs
			,pV6NaptSc->sip.ipv6_addr[0],pV6NaptSc->sip.ipv6_addr[1],pV6NaptSc->sip.ipv6_addr[2],pV6NaptSc->sip.ipv6_addr[3]
			,pV6NaptSc->sip.ipv6_addr[4],pV6NaptSc->sip.ipv6_addr[5],pV6NaptSc->sip.ipv6_addr[6],pV6NaptSc->sip.ipv6_addr[7]
			,pV6NaptSc->sip.ipv6_addr[8],pV6NaptSc->sip.ipv6_addr[9],pV6NaptSc->sip.ipv6_addr[10],pV6NaptSc->sip.ipv6_addr[11]
			,pV6NaptSc->sip.ipv6_addr[12],pV6NaptSc->sip.ipv6_addr[13],pV6NaptSc->sip.ipv6_addr[14],pV6NaptSc->sip.ipv6_addr[15]
			,pV6NaptSc->dip.ipv6_addr[0],pV6NaptSc->dip.ipv6_addr[1],pV6NaptSc->dip.ipv6_addr[2],pV6NaptSc->dip.ipv6_addr[3]
			,pV6NaptSc->dip.ipv6_addr[4],pV6NaptSc->dip.ipv6_addr[5],pV6NaptSc->dip.ipv6_addr[6],pV6NaptSc->dip.ipv6_addr[7]
			,pV6NaptSc->dip.ipv6_addr[8],pV6NaptSc->dip.ipv6_addr[9],pV6NaptSc->dip.ipv6_addr[10],pV6NaptSc->dip.ipv6_addr[11]
			,pV6NaptSc->dip.ipv6_addr[12],pV6NaptSc->dip.ipv6_addr[13],pV6NaptSc->dip.ipv6_addr[14],pV6NaptSc->dip.ipv6_addr[15]
			,pV6NaptSc->sport
			,pV6NaptSc->dport
			,pV6NaptSc->isTcp
			,pV6NaptSc->spa
			,pV6NaptSc->notFinishUpdated);
		PROC_PRINTF("	   [ACT] lut_idx=%d intf_idx=%d pri=%d spri=%d intVlanId=%d intCFPri=%d dscp=%d uniPortmask=0x%x\n"
			,pV6NaptSc->new_lut_idx
			,pV6NaptSc->new_intf_idx
			,pV6NaptSc->priority
			,pV6NaptSc->servicePriority
			,pV6NaptSc->internalVlanID
			,pV6NaptSc->internalCFPri
			,pV6NaptSc->dscp
			,pV6NaptSc->uniPortmask);
		PROC_PRINTF("	   vlanID=%d vlanTagif=%d svlanID=%d svlanTagif=%d dmac2cvlanID=%d dmac2cvlanTagif=%d tagAccType=%d neighborIdx=%d streamID=%d\n"
			,pV6NaptSc->vlanID
			,pV6NaptSc->vlanTagif
			,pV6NaptSc->serviceVlanID
			,pV6NaptSc->serviceVlanTagif
			,pV6NaptSc->dmac2cvlanID
			,pV6NaptSc->dmac2cvlanTagif
			,pV6NaptSc->tagAccType
			,pV6NaptSc->neighborIdx
#ifdef CONFIG_GPON_FEATURE
			,pV6NaptSc->streamID
#else
			,0
#endif
			);
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF("	   smacL2Idx=%d mibDirect=%d mibNetifIdx=%d mibTagDelta=%d\n"
			,pV6NaptSc->smacL2Idx
			,pV6NaptSc->mibDirect
			,pV6NaptSc->mibNetifIdx
			,pV6NaptSc->mibTagDelta);
#endif
	}
}
#endif
#endif

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
void dump_ipv6_statefulList(uint32 idx, struct seq_file *s)
{
	rtk_rg_ipv6_layer4_linkList_t *pV6StatefulList=&rg_db.ipv6Layer4FreeList[idx];

	if(pV6StatefulList->valid) //valid
	{
		PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n 	 sip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x dip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x sport=%d dport=%d isTcp=%d spa=%d notFinishUpdated=%d\n"
			,idx
			,"L3"
			,pV6StatefulList->idleSecs
			,pV6StatefulList->srcIP.ipv6_addr[0],pV6StatefulList->srcIP.ipv6_addr[1],pV6StatefulList->srcIP.ipv6_addr[2],pV6StatefulList->srcIP.ipv6_addr[3]
			,pV6StatefulList->srcIP.ipv6_addr[4],pV6StatefulList->srcIP.ipv6_addr[5],pV6StatefulList->srcIP.ipv6_addr[6],pV6StatefulList->srcIP.ipv6_addr[7]
			,pV6StatefulList->srcIP.ipv6_addr[8],pV6StatefulList->srcIP.ipv6_addr[9],pV6StatefulList->srcIP.ipv6_addr[10],pV6StatefulList->srcIP.ipv6_addr[11]
			,pV6StatefulList->srcIP.ipv6_addr[12],pV6StatefulList->srcIP.ipv6_addr[13],pV6StatefulList->srcIP.ipv6_addr[14],pV6StatefulList->srcIP.ipv6_addr[15]
			,pV6StatefulList->destIP.ipv6_addr[0],pV6StatefulList->destIP.ipv6_addr[1],pV6StatefulList->destIP.ipv6_addr[2],pV6StatefulList->destIP.ipv6_addr[3]
			,pV6StatefulList->destIP.ipv6_addr[4],pV6StatefulList->destIP.ipv6_addr[5],pV6StatefulList->destIP.ipv6_addr[6],pV6StatefulList->destIP.ipv6_addr[7]
			,pV6StatefulList->destIP.ipv6_addr[8],pV6StatefulList->destIP.ipv6_addr[9],pV6StatefulList->destIP.ipv6_addr[10],pV6StatefulList->destIP.ipv6_addr[11]
			,pV6StatefulList->destIP.ipv6_addr[12],pV6StatefulList->destIP.ipv6_addr[13],pV6StatefulList->destIP.ipv6_addr[14],pV6StatefulList->destIP.ipv6_addr[15]
			,pV6StatefulList->srcPort
			,pV6StatefulList->destPort
			,pV6StatefulList->isTCP
			,pV6StatefulList->spa
			,pV6StatefulList->notFinishUpdated);
		PROC_PRINTF("	   [ACT] lut_idx=%d intf_idx=%d pri=%d spri=%d intVlanId=%d intCFPri=%d dscp=%d uniPortmask=0x%x\n"
			,pV6StatefulList->dmacL2Idx
			,pV6StatefulList->netifIdx
			,pV6StatefulList->priority
			,pV6StatefulList->servicePriority
			,pV6StatefulList->internalVlanID
			,pV6StatefulList->internalCFPri
			,pV6StatefulList->dscp
			,pV6StatefulList->uniPortmask);
		PROC_PRINTF("	   vlanID=%d vlanTagif=%d svlanID=%d svlanTagif=%d dmac2cvlanID=%d dmac2cvlanTagif=%d tagAccType=%d neighborIdx=%d streamID=%d\n"
			,pV6StatefulList->vlanID
			,pV6StatefulList->vlanTagif
			,pV6StatefulList->serviceVlanID
			,pV6StatefulList->serviceVlanTagif
			,pV6StatefulList->dmac2cvlanID
			,pV6StatefulList->dmac2cvlanTagif
			,pV6StatefulList->tagAccType
			,pV6StatefulList->neighborIdx
#ifdef CONFIG_GPON_FEATURE
			,pV6StatefulList->streamID
#else
			,0
#endif
			);
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF("	   smacL2Idx=%d mibDirect=%d mibNetifIdx=%d mibTagDelta=%d\n"
			,pV6StatefulList->smacL2Idx
			,pV6StatefulList->mibDirect
			,pV6StatefulList->mibNetifIdx
			,pV6StatefulList->mibTagDelta);
#endif
	}
}

#endif

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int32 dump_sw_shortcut(struct seq_file *s, void *v)
{
	int len=0;

#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
	int i;
#endif
#ifdef CONFIG_ROME_NAPT_SHORTCUT
{
	PROC_PRINTF("-------------------IPv4 SW shortcut-------------------\n");
	for(i=0;i<MAX_NAPT_SHORTCUT_SIZE;i++)
	{
		dump_ipv4_shortcut(i, s);
	}
}
#endif

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
{
	PROC_PRINTF("-------------------IPv6 SW shortcut-------------------\n");
	for(i=0;i<MAX_NAPT_V6_SHORTCUT_SIZE;i++)
	{
		dump_ipv6_shortcut(i, s);
	}
}
#endif
	return len;
}
#endif

int32 mibdump_frag(struct seq_file *s, void *v)
{
    uint32 idx=0;
	rtk_rg_ipv4_fragment_out_t *pFragOut;
	rtk_rg_ipv4_fragment_in_t *pFragIn;
	int len=0;

    PROC_PRINTF(">>SOFTWARE Fragment Link-List:\n");

	//count free out list number
	idx=0;
	list_for_each_entry(pFragOut, &rg_db.fragOutFreeListHead, fragout_list)
	{
		idx++;
	}
	PROC_PRINTF("free Frag Out list has %d elements\n",idx);

	//display used hash head
	for(idx=0;idx<IPV4_FRAGMENT_OUT_HASH_MASK_SHIFT;idx++)
	{
		if(!list_empty(&rg_db.fragOutHashListHead[idx]))
		{
			PROC_PRINTF("fragOutHashListHead[%d]",idx);
			list_for_each_entry(pFragOut, &rg_db.fragOutHashListHead[idx], fragout_list)
			{
				PROC_PRINTF("->%p(%s,idle:%lu,act:%d,queueCnt:%d, rcv_len/total_len: %d/%d)",pFragOut,pFragOut->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",
					pFragOut->beginIdleTime,pFragOut->fragAction,pFragOut->queueCount,
					pFragOut->receivedLength, pFragOut->totalLength);
			}
			PROC_PRINTF("->NULL\n");
		}
	}
	PROC_PRINTF("===================================\n");

	//count free in list number
	idx=0;
	list_for_each_entry(pFragIn, &rg_db.fragInFreeListHead, fragin_list)
	{
		idx++;
	}
	PROC_PRINTF("free Frag In list has %d elements\n",idx);

	//display used hash head
	for(idx=0;idx<IPV4_FRAGMENT_IN_HASH_MASK_SHIFT;idx++)
	{
		if(!list_empty(&rg_db.fragInHashListHead[idx]))
		{
			PROC_PRINTF("fragInHashListHead[%d]",idx);
			list_for_each_entry(pFragIn, &rg_db.fragInHashListHead[idx], fragin_list)
			{
				PROC_PRINTF("->%p(%s,id:%x,idle:%lu,act:%d,queueCnt:%d, rcv_len/total_len: %d/%d)",pFragIn,pFragIn->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",pFragIn->identification,
					pFragIn->beginIdleTime,pFragIn->fragAction,pFragIn->queueCount,
					pFragIn->receivedLength, pFragIn->totalLength);
			}
			PROC_PRINTF("->NULL\n");
		}
	}

	PROC_PRINTF(">>IPv4 fragmentsQueue:\n");
	//rg_lock(&rg_kernel.ipv4FragQueueLock);
	//------------------ Critical Section start -----------------------//
	for(idx=0;idx<MAX_IPV4_FRAGMENT_QUEUE_SIZE;idx++)
	{
		if(rg_db.ipv4FragmentQueue[idx].occupied)
		{
			PROC_PRINTF("  queue[%d]: queueTime=%lu, direction=%d, skb[%p]\n",
				idx,rg_db.ipv4FragmentQueue[idx].queue_time,rg_db.ipv4FragmentQueue[idx].direction,rg_db.ipv4FragmentQueue[idx].queue_skb);
		}
	}
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.ipv4FragQueueLock);

    return len;
}
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
int32 mibdump_v6Frag(struct seq_file *s, void *v)
{
	int i,_count;
	rtk_rg_ipv6_layer4_linkList_t *pLayer4List;
	int len=0;

	PROC_PRINTF(">>IPv6 Stateful fragments:\n");
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
	{
		if(!list_empty(&rg_db.ipv6Layer4HashListHead[i]))
		{
			_count=0;
			list_for_each_entry(pLayer4List,&rg_db.ipv6Layer4HashListHead[i],layer4_list)
			{
				if(!pLayer4List->isFrag)continue;
				if(_count==0)PROC_PRINTF("[%d]========================================================\n",i);

				if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND)
					PROC_PRINTF("   Outbound[%p]",pLayer4List);
				else
					PROC_PRINTF("   Inbound [%p]",pLayer4List);

				PROC_PRINTF("  idle:%lu action:%d, queueCount:%d\n",
					(jiffies-pLayer4List->beginIdleTime)/TICKTIME_PERIOD,pLayer4List->fragAction,pLayer4List->queueCount);
				PROC_PRINTF("	  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x --> \n	 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ID:%04x%04x\n",
					pLayer4List->srcIP.ipv6_addr[0],pLayer4List->srcIP.ipv6_addr[1],pLayer4List->srcIP.ipv6_addr[2],pLayer4List->srcIP.ipv6_addr[3],
					pLayer4List->srcIP.ipv6_addr[4],pLayer4List->srcIP.ipv6_addr[5],pLayer4List->srcIP.ipv6_addr[6],pLayer4List->srcIP.ipv6_addr[7],
					pLayer4List->srcIP.ipv6_addr[8],pLayer4List->srcIP.ipv6_addr[9],pLayer4List->srcIP.ipv6_addr[10],pLayer4List->srcIP.ipv6_addr[11],
					pLayer4List->srcIP.ipv6_addr[12],pLayer4List->srcIP.ipv6_addr[13],pLayer4List->srcIP.ipv6_addr[14],pLayer4List->srcIP.ipv6_addr[15],
					pLayer4List->destIP.ipv6_addr[0],pLayer4List->destIP.ipv6_addr[1],pLayer4List->destIP.ipv6_addr[2],pLayer4List->destIP.ipv6_addr[3],
					pLayer4List->destIP.ipv6_addr[4],pLayer4List->destIP.ipv6_addr[5],pLayer4List->destIP.ipv6_addr[6],pLayer4List->destIP.ipv6_addr[7],
					pLayer4List->destIP.ipv6_addr[8],pLayer4List->destIP.ipv6_addr[9],pLayer4List->destIP.ipv6_addr[10],pLayer4List->destIP.ipv6_addr[11],
					pLayer4List->destIP.ipv6_addr[12],pLayer4List->destIP.ipv6_addr[13],pLayer4List->destIP.ipv6_addr[14],pLayer4List->destIP.ipv6_addr[15],
					pLayer4List->srcPort,pLayer4List->destPort);

				_count++;
			}
			if(_count>0)PROC_PRINTF("[%d] has %d Fragment.\n",i,_count);
		}

	}

	PROC_PRINTF(">>IPv6 Stateful fragmentsQueue:\n");
	for(i=0;i<MAX_IPV6_FRAGMENT_QUEUE_SIZE;i++)
	{
		//------------------ Critical Section start -----------------------//
		rg_lock(&rg_kernel.ipv6FragQueueLock);
		if(rg_db.ipv6FragmentQueue[i].occupied)
		{
			PROC_PRINTF("  queue[%d]: queueTime=%lu, direction=%d, wanType=%s, skb[%p]\n",
				i,(jiffies-rg_db.ipv6FragmentQueue[i].queue_time)/TICKTIME_PERIOD,rg_db.ipv6FragmentQueue[i].direction,
				rg_db.ipv6FragmentQueue[i].wanType==L34_NH_ETHER?"ETHER":"PPPOE",rg_db.ipv6FragmentQueue[i].queue_skb);
		}
		//------------------ Critical Section End -----------------------//
		rg_unlock(&rg_kernel.ipv6FragQueueLock);
	}

    return len;
}
#endif

#if defined(CONFIG_APOLLO_GPON_FPGATEST)

int32 mibclean_vmac_skb( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtk_rg_vmac_skb_linlList_t *pVmacEntry,*pNextEntry;

	if(!list_empty(&rg_db.vmacSkbListHead))
	{
		list_for_each_entry_safe(pVmacEntry,pNextEntry,&rg_db.vmacSkbListHead,vmac_list)
		{
			//Delete from head list
			list_del_init(&pVmacEntry->vmac_list);

			dev_kfree_skb_any(pVmacEntry->skb);
			pVmacEntry->skb=NULL;

			//Add back to free list
			list_add(&pVmacEntry->vmac_list,&rg_db.vmacSkbListFreeListHead);
		}
	}

	return len;
}


int32 mibdump_vmac_skb(struct seq_file *s, void *v)
{
	int len=0,i=0;
	rtk_rg_vmac_skb_linlList_t *pVmacEntry;

	PROC_PRINTF(">>virtualmac with PON:\n\n");

	if(!list_empty(&rg_db.vmacSkbListHead))
	{
		list_for_each_entry(pVmacEntry,&rg_db.vmacSkbListHead,vmac_list)
		{
			PROC_PRINTF("-->skb_%d[%x] len=%d %08x %08x %08x %08x\n",i++,(unsigned int)pVmacEntry->skb&0xffff,pVmacEntry->skb->len,
				*(unsigned int *)pVmacEntry->skb->data,
				*(unsigned int *)(pVmacEntry->skb->data+1),
				*(unsigned int *)(pVmacEntry->skb->data+2),
				*(unsigned int *)(pVmacEntry->skb->data+3));
			memDump(pVmacEntry->skb,pVmacEntry->skb->len,"PACKET");
		}
	}

	return len;
}
#endif


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int32 rtk_rg_asic_display_netif_table_idx(uint32 idx)
{
#if !defined(CONFIG_RG_G3_SERIES)
	uint8	*mac;
	//int32	i;
	//for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
	//{
		rtk_rg_asic_netif_entry_t intf;
		rtk_rg_asic_netifMib_entry_t intfMib;
		int retval  = 0;

		memset(&intf,0,sizeof(rtk_rg_asic_netif_entry_t));
		retval = rtk_rg_asic_netifTable_get(idx, &intf);
		if (retval == FAIL) return FAIL; //continue;
#if 1
		if (intf.valid)
		{
			mac = (uint8 *)&intf.gateway_mac_addr.octet[0];
			rtlglue_printf("  [%d]- %02x:%02x:%02x:%02x:%02x:%02x    IP: %d.%d.%d.%d",
				idx, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
				(intf.gateway_ipv4_addr>>24)&0xff, (intf.gateway_ipv4_addr>>16)&0xff, (intf.gateway_ipv4_addr>>8)&0xff, intf.gateway_ipv4_addr&0xff);
			rtlglue_printf("\n      MTU check: %d, MTU %d Bytes", intf.intf_mtu_check, intf.intf_mtu);
			rtlglue_printf("\n      IgrAct: %d, EgrAct: %d, denyv4: %d, denyv6: %d", intf.ingress_action, intf.egress_action, intf.deny_ipv4, intf.deny_ipv6);
			rtlglue_printf("\n      Allow IgrPMask: 0x%x, IgrExtPmask: 0x%x", intf.allow_ingress_portmask.bits[0], intf.allow_ingress_ext_portmask.bits[0]);
			rtlglue_printf("\n      Out PPPoE Act: %d, sid: 0x%x", intf.out_pppoe_act, intf.out_pppoe_sid);


			retval = rtk_rg_asic_netifMib_get(idx, &intfMib);

			rtlglue_printf("\n      Igr pkt count : ");
			rtlglue_printf("uc(%u), mc(%u), bc(%u)", intfMib.in_intf_uc_packet_cnt, intfMib.in_intf_mc_packet_cnt, intfMib.in_intf_bc_packet_cnt);
			rtlglue_printf("\n      Igr byte count: ");
			rtlglue_printf("uc(%llu), mc(%llu), bc(%llu)", intfMib.in_intf_uc_byte_cnt, intfMib.in_intf_mc_byte_cnt, intfMib.in_intf_bc_byte_cnt);

			rtlglue_printf("\n      Egr pkt count : ");
			rtlglue_printf("uc(%u), mc(%u), bc(%u)", intfMib.out_intf_uc_packet_cnt, intfMib.out_intf_mc_packet_cnt, intfMib.out_intf_bc_packet_cnt);
			rtlglue_printf("\n      Egr byte count: ");
			rtlglue_printf("uc(%llu), mc(%llu), bc(%llu)", intfMib.out_intf_uc_byte_cnt, intfMib.out_intf_mc_byte_cnt, intfMib.out_intf_bc_byte_cnt);

			rtlglue_printf("\n\n");
		}
#endif
	//}
#endif //!defined(CONFIG_RG_G3_SERIES)
	return SUCCESS;
}

int32 rtk_rg_asic_dump_netif_table_idx(uint32 idx)
{
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_netif_entry_t intf;
	rtk_rg_asic_netifMib_entry_t intfMib;
	int retval  = 0;

	memset(&intf,0,sizeof(rtk_rg_asic_netif_entry_t));
	retval = rtk_rg_asic_netifTable_get(idx, &intf);

	memset(&intfMib,0,sizeof(rtk_rg_asic_netifMib_entry_t));
	retval = rtk_rg_asic_netifMib_get(idx, &intfMib);
	return display_netif(idx, &intf, &intfMib);
#else //defined(CONFIG_RG_G3_SERIES)
	struct ca_int_l3_intf_s intf;
	ca_l3_intf_stats_t intfMib;
	int retval  = 0;

	rtlglue_printf("--------------------------------------------------------------------------------\n");

	retval = ca_generic_intf_get(G3_DEF_DEVID, idx, &intf);
	if(retval == CA_E_OK)
	{
		memset(&intfMib,0,sizeof(ca_l3_intf_stats_t));
		ca_l3_intf_stats_get(G3_DEF_DEVID, idx, FALSE, &intfMib);
		return display_netif(idx, &intf, &intfMib);
	}
	else if(retval == CA_E_NOT_FOUND)
		rtlglue_printf("generic interface[%d] does not exist\n", idx);

	return SUCCESS;
#endif
}

int32 rtk_rg_asic_dump_netif_table_all(void)
{
	int i = 0;

	for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
		rtk_rg_asic_dump_netif_table_idx(i);

	return SUCCESS;
}
#endif

#if defined(CONFIG_RG_WAN_MSS_CACHE)
int32 mibdump_mssCache(struct seq_file *s, void *v)
{
	int i,j;
	rtk_rg_netif_mssCache_t *pCache;
	rtk_rg_netif_mssCacheEntry_t *pCur;

	PROC_PRINTF(">>MSS Cache List:\nHASH=%d,POOL=%d\n",MAX_NETIF_MSSCACHE_HASH,MAX_NETIF_MSSCACHE_POOL);

	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
	{
		if(!(rg_db.systemGlobal.interfaceInfo[i].valid && rg_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan))continue;
		PROC_PRINTF("WAN[%d]:\n",i);
		pCache=&rg_db.msscache[i];
		for(j=0;j<MAX_NETIF_MSSCACHE_HASH;j++)
		{
			if(!list_empty(&pCache->hash[j]))
			{
				list_for_each_entry(pCur, &pCache->hash[j], msscache_list)
				{
					if(*(unsigned int *)(pCur->v6dest+4)==0&&*(unsigned int *)(pCur->v6dest+8)==0&&*(unsigned int *)(pCur->v6dest+12)==0)
						PROC_PRINTF("\t%d.%d.%d.%d advmss:%d\n",
							(pCur->dest&0xff000000)>>24,
							(pCur->dest&0xff0000)>>16,
							(pCur->dest&0xff00)>>8,
							(pCur->dest&0xff),
							pCur->advmss);
					else
						PROC_PRINTF("\t%08x:%08x:%08x:%08x advmss:%d\n",
							*((unsigned int *)pCur->v6dest),
							*((unsigned int *)(pCur->v6dest+4)),
							*((unsigned int *)(pCur->v6dest+8)),
							*((unsigned int *)(pCur->v6dest+12)),
							pCur->advmss);
				}
			}
		}
	}

	return SUCCESS;
}
#endif
int32 dump_netif(struct seq_file *s, void *v)
{

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	uint8	*mac;
	int32	i, j;
	rtk_portmask_t mbr,untag;
	int len=0;

    PROC_PRINTF(">>ASIC Netif Table:\n\n");
    for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
    {
    	rtk_l34_netif_entry_t intf;
#if defined(CONFIG_RG_RTL9602C_SERIES)
		rtk_l34_mib_t wanIfMibt;
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		rtk_rg_asic_netifMib_entry_t asicNetifMib;
#endif
        int retval  = 0;
		memset(&intf,0,sizeof(rtk_l34_netif_entry_t));
		retval = rtk_l34_netifTable_get(i,&intf);
        if (retval == FAIL)
            continue;

        if (intf.valid)
        {
            mac = (uint8 *)&intf.gateway_mac.octet[0];
            PROC_PRINTF("  [%d]-vid[%d] %02x:%02x:%02x:%02x:%02x:%02x (l2Idx=%d)",
                           i, intf.vlan_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],rg_db.netif[i].l2_idx);
            PROC_PRINTF("  L3/4 HW acc %s ",
                           intf.enable_rounting==TRUE? "enabled": "disabled" );
            PROC_PRINTF("\n      %d MAC Addresses, MTU %d Bytes", intf.mac_mask, intf.mtu);
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF("\n 	DSlite state[%s], Index[%d]", intf.dslite_state==1?"enabled": "disabled", intf.dslite_idx);
			PROC_PRINTF("\n 	For mib counter lookup, isL34:%d, isCtagIf:%d, IPv4 Addr: %d.%d.%d.%d", intf.isL34, intf.isCtagIf, (intf.ipAddr>>24),
						   ((intf.ipAddr&0x00ff0000)>>16), ((intf.ipAddr&0x0000ff00)>>8), (intf.ipAddr&0xff));
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT1))
				PROC_PRINTF(", isIpv6:%d", intf.isIpv6);

			memset(&wanIfMibt,0,sizeof(rtk_l34_mib_t));
			wanIfMibt.ifIndex = i;
			if(rtk_l34_mib_get(&wanIfMibt)==RT_ERR_OK)
				PROC_PRINTF("\n 	HW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)", wanIfMibt.ifInUcstPkts, wanIfMibt.ifInOctets, wanIfMibt.ifOutUcstPkts, wanIfMibt.ifOutOctets);
			PROC_PRINTF("\n 	SW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)", rg_db.netif[i].netifMib.in_intf_uc_packet_cnt, rg_db.netif[i].netifMib.in_intf_uc_byte_cnt, rg_db.netif[i].netifMib.out_intf_uc_packet_cnt, rg_db.netif[i].netifMib.out_intf_uc_byte_cnt);
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
			if(rtk_rg_asic_netifMib_get(i, &asicNetifMib)==RT_ERR_RG_OK)
			{
				PROC_PRINTF("\n 	HW:inUcPktCnt(%u), inUcByteCnt(%llu), outUcPktCnt(%u), outUcByteCnt(%llu)", asicNetifMib.in_intf_uc_packet_cnt, asicNetifMib.in_intf_uc_byte_cnt, asicNetifMib.out_intf_uc_packet_cnt, asicNetifMib.out_intf_uc_byte_cnt);
				PROC_PRINTF("\n 	   inMcPktCnt(%u), inMcByteCnt(%llu), outMcPktCnt(%u), outMcByteCnt(%llu)", asicNetifMib.in_intf_mc_packet_cnt, asicNetifMib.in_intf_mc_byte_cnt, asicNetifMib.out_intf_mc_packet_cnt, asicNetifMib.out_intf_mc_byte_cnt);
				PROC_PRINTF("\n 	   inBcPktCnt(%u), inBcByteCnt(%llu), outBcPktCnt(%u), outBcByteCnt(%llu)", asicNetifMib.in_intf_bc_packet_cnt, asicNetifMib.in_intf_bc_byte_cnt, asicNetifMib.out_intf_bc_packet_cnt, asicNetifMib.out_intf_bc_byte_cnt);
			}
			PROC_PRINTF("\n 	SW:inUcPktCnt(%u), inUcByteCnt(%llu), outUcPktCnt(%u), outUcByteCnt(%llu)", rg_db.netif[i].netifMib.in_intf_uc_packet_cnt, rg_db.netif[i].netifMib.in_intf_uc_byte_cnt, rg_db.netif[i].netifMib.out_intf_uc_packet_cnt, rg_db.netif[i].netifMib.out_intf_uc_byte_cnt);
			PROC_PRINTF("\n 	   inMcPktCnt(%u), inMcByteCnt(%llu), outMcPktCnt(%u), outMcByteCnt(%llu)", rg_db.netif[i].netifMib.in_intf_mc_packet_cnt, rg_db.netif[i].netifMib.in_intf_mc_byte_cnt, rg_db.netif[i].netifMib.out_intf_mc_packet_cnt, rg_db.netif[i].netifMib.out_intf_mc_byte_cnt);
			PROC_PRINTF("\n 	   inBcPktCnt(%u), inBcByteCnt(%llu), outBcPktCnt(%u), outBcByteCnt(%llu)", rg_db.netif[i].netifMib.in_intf_bc_packet_cnt, rg_db.netif[i].netifMib.in_intf_bc_byte_cnt, rg_db.netif[i].netifMib.out_intf_bc_packet_cnt, rg_db.netif[i].netifMib.out_intf_bc_byte_cnt);
#endif

            mbr.bits[0]=0;
            untag.bits[0]=0;
            retval = rtk_vlan_port_get(intf.vlan_id,&mbr,&untag);
            //assert_ok(retval==RT_ERR_OK);
            PROC_PRINTF("\n		Untag member ports:");
            for(j=0; j<RTK_RG_MAC_PORT_MAX; j++)
            {
            	if(RG_INVALID_MAC_PORT(j)) continue;
                if(untag.bits[0] & (1<<j))
                    PROC_PRINTF("%d ", j);
            }
            PROC_PRINTF("\n		Active member ports:");
            for(j=0; j<RTK_RG_MAC_PORT_MAX; j++)
            {
            	if(RG_INVALID_MAC_PORT(j)) continue;
                if(mbr.bits[0] & (1<<j))
                    PROC_PRINTF("%d ", j);
            }
            mbr.bits[0]=0;
            retval = rtk_vlan_extPort_get(intf.vlan_id,&mbr);
			//assert_ok(retval==RT_ERR_OK);
            for(j=1; j<RTK_RG_MAX_EXT_PORT; j++)
                if(mbr.bits[0] & (1<<j))
                    PROC_PRINTF("EXT%d ", j);
            PROC_PRINTF("\n\n");
        }
    }

    return len;
#else
	// CONFIG_RG_FLOW_BASED_PLATFORM
	int32	i;
	PROC_PRINTF(">>ASIC Netif Table:\n\n");

#if defined(CONFIG_RG_G3_SERIES)
	for(i=NETIF_START_IDX; i<MAX_GENERIC_INTERFACE_SIZE; i++)
		rtk_rg_asic_dump_netif_table_idx(i);
#else
	for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
		rtk_rg_asic_dump_netif_table_idx(i+NETIF_START_IDX);
#endif

	return SUCCESS;
#endif
}

int32 dump_sw_netif(struct seq_file *s, void *v)
{
	int len=0;
	uint8	*mac;
    int32	i;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	int32 retval, j;
    rtk_portmask_t mbr,untag;
#endif

	PROC_PRINTF(">>SW Netif Table:\n\n");

	for(i=0; i<MAX_NETIF_SW_TABLE_SIZE; i++)
	{
#if defined(CONFIG_RG_RTL9602C_SERIES)
		rtk_l34_mib_t wanIfMibt;
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		rtk_rg_asic_netifMib_entry_t asicNetifMib;
#endif



		if (rg_db.netif[i].rtk_netif.valid)
		{
			mac = (uint8 *)&rg_db.netif[i].rtk_netif.gateway_mac.octet[0];
			PROC_PRINTF("  [%d](%s)-vid[%d] %02x:%02x:%02x:%02x:%02x:%02x (l2Idx=%d)",
						   i,(i>=MAX_NETIF_HW_TABLE_SIZE)?"SW_ONLY":"HW_SW_SYNC" ,rg_db.netif[i].rtk_netif.vlan_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],rg_db.netif[i].l2_idx);
			PROC_PRINTF("  L3/4 HW acc %s \n",
						   rg_db.netif[i].rtk_netif.enable_rounting==TRUE? "enabled": "disabled" );
			PROC_PRINTF(" 	%d MAC Addresses, MTU %d Bytes\n", rg_db.netif[i].rtk_netif.mac_mask, rg_db.netif[i].rtk_netif.mtu);
#if defined(CONFIG_RG_RTL9602C_SERIES)
			PROC_PRINTF(" 	DSlite state[%s], Index[%d]\n", rg_db.netif[i].rtk_netif.dslite_state==1?"enabled": "disabled", rg_db.netif[i].rtk_netif.dslite_idx);
			PROC_PRINTF(" 	For mib counter lookup, isL34:%d, isCtagIf:%d, IPv4 Addr: %d.%d.%d.%d\n", rg_db.netif[i].rtk_netif.isL34, rg_db.netif[i].rtk_netif.isCtagIf, (rg_db.netif[i].rtk_netif.ipAddr>>24),
						   ((rg_db.netif[i].rtk_netif.ipAddr&0x00ff0000)>>16), ((rg_db.netif[i].rtk_netif.ipAddr&0x0000ff00)>>8), (rg_db.netif[i].rtk_netif.ipAddr&0xff));

			memset(&wanIfMibt,0,sizeof(rtk_l34_mib_t));
			wanIfMibt.ifIndex = i;
			if(rtk_l34_mib_get(&wanIfMibt)==RT_ERR_OK)
				PROC_PRINTF(" 	HW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)\n", wanIfMibt.ifInUcstPkts, wanIfMibt.ifInOctets, wanIfMibt.ifOutUcstPkts, wanIfMibt.ifOutOctets);
			PROC_PRINTF(" 	SW:inPktCnt(%u), inByteCnt(%llu), outPktCnt(%u), outByteCnt(%llu)\n", rg_db.netif[i].netifMib.in_intf_uc_packet_cnt, rg_db.netif[i].netifMib.in_intf_uc_byte_cnt, rg_db.netif[i].netifMib.out_intf_uc_packet_cnt, rg_db.netif[i].netifMib.out_intf_uc_byte_cnt);
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
			PROC_PRINTF(" 	isL34:%d, isCtagIf:%d, IPv4 Addr: %d.%d.%d.%d\n", rg_db.netif[i].rtk_netif.isL34, rg_db.netif[i].rtk_netif.isCtagIf, (rg_db.netif[i].rtk_netif.ipAddr>>24),
						   ((rg_db.netif[i].rtk_netif.ipAddr&0x00ff0000)>>16), ((rg_db.netif[i].rtk_netif.ipAddr&0x0000ff00)>>8), (rg_db.netif[i].rtk_netif.ipAddr&0xff));
#if defined(CONFIG_RG_G3_SERIES)
			if(rg_db.systemGlobal.interfaceInfo[i].valid)
			{
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan==0)
				{
					PROC_PRINTF(" 	LAN groupIdx[%d], generic intf[%d~%d]\n",
						rg_db.systemGlobal.interfaceInfo[i].lanWan_groupIdx_for_genericIntf,
						(rg_db.systemGlobal.interfaceInfo[i].lanWan_groupIdx_for_genericIntf*MAX_GENERIC_INTERFACE_PER_LAN)+NETIF_START_IDX,
						((rg_db.systemGlobal.interfaceInfo[i].lanWan_groupIdx_for_genericIntf+1)*MAX_GENERIC_INTERFACE_PER_LAN)+NETIF_START_IDX-1);
				}
				else
				{
					int genIntf_idx = _rtk_rg_g3_generic_intf_index_get(FALSE, rg_db.systemGlobal.interfaceInfo[i].lanWan_groupIdx_for_genericIntf, rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx, FAIL);
					struct ca_int_l3_intf_s intf;

					if(ca_generic_intf_get(G3_DEF_DEVID, genIntf_idx, &intf) != CA_E_OK)
						genIntf_idx = FAIL;
						
					PROC_PRINTF(" 	WAN groupIdx[%d], generic intf[%d]\n", 
						rg_db.systemGlobal.interfaceInfo[i].lanWan_groupIdx_for_genericIntf,
						genIntf_idx);
				}
			}
			else
			{
				PROC_PRINTF("\033[1;33;41m 	[WARNING] Sw interface[%d] is invalid \033[0m\n", i);
			}
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
			if(rtk_rg_asic_netifMib_get(i, &asicNetifMib)==RT_ERR_RG_OK)
			{
				PROC_PRINTF(" 	HW:Igr pkt count : uc(%u), mc(%u), bc(%u)\n", asicNetifMib.in_intf_uc_packet_cnt, asicNetifMib.in_intf_mc_packet_cnt, asicNetifMib.in_intf_bc_packet_cnt);
				PROC_PRINTF(" 	   Igr byte count: uc(%llu), mc(%llu), bc(%llu)\n", asicNetifMib.in_intf_uc_byte_cnt, asicNetifMib.in_intf_mc_byte_cnt, asicNetifMib.in_intf_bc_byte_cnt);
				PROC_PRINTF(" 	   Egr pkt count : uc(%u), mc(%u), bc(%u)\n", asicNetifMib.out_intf_uc_packet_cnt, asicNetifMib.out_intf_mc_packet_cnt, asicNetifMib.out_intf_bc_packet_cnt);
				PROC_PRINTF(" 	   Egr byte count: uc(%llu), mc(%llu), bc(%llu)\n", asicNetifMib.out_intf_uc_byte_cnt, asicNetifMib.out_intf_mc_byte_cnt, asicNetifMib.out_intf_bc_byte_cnt);
			}
			PROC_PRINTF(" 	SW:Igr pkt count : uc(%u), mc(%u), bc(%u)\n", rg_db.netif[i].netifMib.in_intf_uc_packet_cnt, rg_db.netif[i].netifMib.in_intf_mc_packet_cnt, rg_db.netif[i].netifMib.in_intf_bc_packet_cnt);
			PROC_PRINTF(" 	   Igr byte count: uc(%llu), mc(%llu), bc(%llu)\n", rg_db.netif[i].netifMib.in_intf_uc_byte_cnt, rg_db.netif[i].netifMib.in_intf_mc_byte_cnt, rg_db.netif[i].netifMib.in_intf_bc_byte_cnt);
			PROC_PRINTF(" 	   Egr pkt count : uc(%u), mc(%u), bc(%u)\n", rg_db.netif[i].netifMib.out_intf_uc_packet_cnt, rg_db.netif[i].netifMib.out_intf_mc_packet_cnt, rg_db.netif[i].netifMib.out_intf_bc_packet_cnt);
			PROC_PRINTF(" 	   Egr byte count: uc(%llu), mc(%llu), bc(%llu)\n", rg_db.netif[i].netifMib.out_intf_uc_byte_cnt, rg_db.netif[i].netifMib.out_intf_mc_byte_cnt, rg_db.netif[i].netifMib.out_intf_bc_byte_cnt);
#else //defined(CONFIG_RG_G3_SERIES)
			//G3 HW only supports for uc packets counting now, thus shows SW uc counter only.
			PROC_PRINTF(" 	SW:Igr pkt count : uc(%u)\n", rg_db.netif[i].netifMib.in_intf_uc_packet_cnt);
			PROC_PRINTF(" 	   Igr byte count: uc(%llu)\n", rg_db.netif[i].netifMib.in_intf_uc_byte_cnt);
			PROC_PRINTF(" 	   Egr pkt count : uc(%u)\n", rg_db.netif[i].netifMib.out_intf_uc_packet_cnt);
			PROC_PRINTF(" 	   Egr byte count: uc(%llu)\n", rg_db.netif[i].netifMib.out_intf_uc_byte_cnt);
#endif
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
			//no need to access rtk api
#else
			mbr.bits[0]=0;
			untag.bits[0]=0;
			retval = rtk_vlan_port_get(rg_db.netif[i].rtk_netif.vlan_id,&mbr,&untag);
			//assert_ok(retval==RT_ERR_OK);
			PROC_PRINTF(" 	Untag member ports:");
			for(j=0; j<RTK_RG_MAC_PORT_MAX; j++)
			{
				if(RG_INVALID_MAC_PORT(j)) continue;
				if(untag.bits[0] & (1<<j))
					PROC_PRINTF("%d ", j);
			}
			PROC_PRINTF("\n 	Active member ports:");
			for(j=0; j<RTK_RG_MAC_PORT_MAX; j++)
			{
				if(RG_INVALID_MAC_PORT(j)) continue;
				if(mbr.bits[0] & (1<<j))
					PROC_PRINTF("%d ", j);
			}
			mbr.bits[0]=0;
			retval = rtk_vlan_extPort_get(rg_db.netif[i].rtk_netif.vlan_id,&mbr);
			//assert_ok(retval==RT_ERR_OK);
			for(j=1; j<RTK_RG_MAX_EXT_PORT; j++)
				if(mbr.bits[0] & (1<<j))
					PROC_PRINTF("EXT%d ", j);
#endif
			PROC_PRINTF("\n\n");
		}
	}

    return len;
}


int32 dump_pppoe(struct seq_file *s, void *v)
{
	rtk_l34_pppoe_entry_t pppEntry;
	int32	i=0,retval=0;
	int len=0;

	PROC_PRINTF(">>PPPOE Table:\n");
	for(i=0; i<MAX_PPPOE_HW_TABLE_SIZE; i++)
	{
		memset(&pppEntry,0,sizeof(rtk_l34_pppoe_entry_t));
		retval = rtk_l34_pppoeTable_get(i,  &pppEntry);
		if (retval == FAIL)
			continue;
		PROC_PRINTF("  [%d]	sessionID(%d) hex:0x%x\n", i, pppEntry.sessionID, pppEntry.sessionID);
	}

    return len;
}

int32 dump_sw_pppoe(struct seq_file *s, void *v)
{

	int32	i=0;
	int len=0;
	char validType[4][15] = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};

	PROC_PRINTF(">>PPPOE SW Table:\n");
	for(i=0; i<MAX_PPPOE_SW_TABLE_SIZE; i++)
	{
		if(rg_db.pppoe[i].valid)
			PROC_PRINTF("  [%d](%s)\t	sessionID(%d) hex:0x%x  idleSecs=%d\n", i,validType[rg_db.pppoe[i].valid], rg_db.pppoe[i].rtk_pppoe.sessionID, rg_db.pppoe[i].rtk_pppoe.sessionID,rg_db.pppoe[i].idleSecs);
	}

    return len;
}


int32 dump_nexthop(struct seq_file *s, void *v)
{
	rtk_l34_nexthop_entry_t nextHopEntry;
	uint32 idx, refcnt, rt_flag, retval;

	uint8	isPppoe=0;
	uint8  	dvid=0;
	uint8  	pppoeIdx=0;
	uint16 	nextHopRow=0;
	uint16 	nextHopColumn=0;
	uint8  	pppoekeep=0;
	uint8 	extIntIpIdx=0;
	int len=0;


	PROC_PRINTF(">>ASIC Next Hop Table:\n");

	for(idx=0; idx<MAX_NEXTHOP_HW_TABLE_SIZE; idx++)
	{
		refcnt = rt_flag = 0;
		retval = rtk_l34_nexthopTable_get(idx, &nextHopEntry);
		ASSERT_EQ(retval,RT_ERR_OK);


		dvid = nextHopEntry.ifIdx;
		nextHopRow = nextHopEntry.nhIdx>>2;
		nextHopColumn = nextHopEntry.nhIdx&0x3;
		pppoeIdx = nextHopEntry.pppoeIdx;
		isPppoe = nextHopEntry.type;
		pppoekeep = nextHopEntry.keepPppoe;

		if((nextHopRow==0) && (nextHopColumn==0) && (isPppoe==0) && (extIntIpIdx==0) && (dvid==0) && (pppoeIdx==0))
			continue;

		PROC_PRINTF("  [%d]\ttype(%s) keep(%d) IFIdx(%d) pppoeIdx(%d) nextHop(%d,%d) refCount(%d)\n", idx,
					   (isPppoe==TRUE? "pppoe": "ethernet"),pppoekeep,
					   dvid, pppoeIdx, (nextHopRow),nextHopColumn,rg_db.systemGlobal.nxpRefCount[idx]);
	}


    return len;
}


int32 sw_dump_dsliteMc_table(struct seq_file *s, void *v)
{
	int len=0;
	int i,j;

    PROC_PRINTF(">>DSLITEMC Table:\n\n");
	for(i=0;i<MAX_DSLITEMC_SW_TABLE_SIZE;i++){

		PROC_PRINTF("[%d] \n",i);
		PROC_PRINTF("\tipMPrefix64:\t\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[j*2],rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipMPrefix64Mask:\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2],rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipUPrefix64:\t\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[j*2],rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

		PROC_PRINTF("\tipUPrefix64Mask:\t");
		for(j=0;j<4;j++)
			PROC_PRINTF("%02x%02x ",rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2],rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[j*2+1]);
		PROC_PRINTF("XXXX XXXX XXXX XXXX ");
		PROC_PRINTF("\n");

	}
	return len;


}


int32 dump_sw_nexthop(struct seq_file *s, void *v)
{
	uint32 idx;
	char validType[4][15] = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};

	uint8	isPppoe=0;
	uint8  	dvid=0;
	uint8  	pppoeIdx=0;
	uint16 	nextHopRow=0;
	uint16 	nextHopColumn=0;
	uint8  	pppoekeep=0;

	int len=0;

	PROC_PRINTF(">>Sw NextHop Table:\n");

	for(idx=0; idx<MAX_NEXTHOP_SW_TABLE_SIZE; idx++)
	{

		if(rg_db.nexthop[idx].valid)
		{

			dvid = rg_db.nexthop[idx].rtk_nexthop.ifIdx;
			nextHopRow = rg_db.nexthop[idx].rtk_nexthop.nhIdx>>2;
			nextHopColumn = rg_db.nexthop[idx].rtk_nexthop.nhIdx&0x3;
			pppoeIdx = rg_db.nexthop[idx].rtk_nexthop.pppoeIdx;
			isPppoe = rg_db.nexthop[idx].rtk_nexthop.type;
			pppoekeep = rg_db.nexthop[idx].rtk_nexthop.keepPppoe;

			PROC_PRINTF("  [%d](%s)\ttype(%s) keep(%d) IFIdx(%d) pppoeIdx(%d) nextHop(%d,%d) refCount(%d) extIPMask(%x) origIfMask(%x)\n", idx,validType[rg_db.nexthop[idx].valid],
					   (isPppoe==TRUE? "pppoe": "ethernet"),pppoekeep,
					   dvid, pppoeIdx, (nextHopRow),nextHopColumn,rg_db.systemGlobal.nxpRefCount[idx],rg_db.nexthop[idx].extIPMask,rg_db.nexthop[idx].staticRouteWanIdxMask);
		}
	}

    return len;
}


int32 dump_arp(struct seq_file *s, void *v)
{

#if defined(CONFIG_RG_RTL9600_SERIES)
	rtk_l34_routing_entry_t asic_l3;
	uint32 j;
	ipaddr_t ipAddr;
#endif
	uint32	i, retval;
	rtk_l34_arp_entry_t asic_arp;
	rtk_rg_arp_linkList_t *pSoftwareArp;
	int len=0;

	int8 ipBuf[sizeof"255.255.255.255"];

	PROC_PRINTF(">>Arp Table:\n");
	for(i=0; i<MAX_ARP_HW_TABLE_SIZE; i++)
	{
		retval = rtk_l34_arpTable_get(i,  &asic_arp);
		if ((retval != RT_ERR_OK) || asic_arp.valid!=1)
			continue;
		{
#if defined(CONFIG_RG_RTL9600_SERIES)
			for(j=0; j<MAX_L3_HW_TABLE_SIZE; j++)
			{
				memset(&asic_l3,0,sizeof(asic_l3));
				retval = rtk_l34_routingTable_get(j, &asic_l3);
				if ( retval == FAIL || asic_l3.process!= 2 || asic_l3.valid!=1)
					continue;

				if(asic_l3.arpStart <= (i>>2) &&  (i>>2) <= asic_l3.arpEnd)
				{
					ipAddr= (i-(asic_l3.arpStart<<2))+(asic_l3.ipAddr & (~((1<<(31-asic_l3.ipMask))-1)));
					inet_ntoa_r(ipAddr, ipBuf);
					if(rg_db.arp[i].staticEntry)
						{PROC_PRINTF("  [%3d] : %-16s (STATIC) : ",i, ipBuf);}
					else
						{PROC_PRINTF("  [%3d] : %-16s (DYNAMIC) : ",i, ipBuf);}
					if(rg_db.systemGlobal.gatherLanNetInfo && rg_db.arp[i].lanNetInfo.checked)
						PROC_PRINTF("Name:%s DevType:%s Brand:%s Model:%s OS:%s ConnType:%s",
							rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].dev_name,
							rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
							rg_lanNet_brand[(int)rg_db.arp[i].lanNetInfo.brand][0],
							rg_lanNet_model[(int)rg_db.arp[i].lanNetInfo.model][0],
							rg_lanNet_os[(int)rg_db.arp[i].lanNetInfo.os][0],
							rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
					PROC_PRINTF(" -> L2:%d (L3Idx:%d Idle %d Secs P:%d)\n", asic_arp.nhIdx,rg_db.arp[i].routingIdx,rg_db.arp[i].idleSecs,rg_db.arp[i].permit_for_l34_forward);
					break;
				}
			}
#elif defined(CONFIG_RG_RTL9602C_SERIES)
			inet_ntoa_r(asic_arp.ipAddr, ipBuf);
			if(rg_db.arp[i].staticEntry){
				PROC_PRINTF("  [%3d] : %-16s (STATIC) : ",i, ipBuf);
			}else{
				PROC_PRINTF("  [%3d] : %-16s (DYNAMIC) : ",i, ipBuf);
			}
			if(rg_db.systemGlobal.gatherLanNetInfo)
				PROC_PRINTF("Name:%s DevType:%s Brand:%s Model:%s OS:%s ConnType:%s\n",
					rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].dev_name,
					rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[i].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
					rg_lanNet_brand[(int)rg_db.arp[i].lanNetInfo.brand][0],
					rg_lanNet_model[(int)rg_db.arp[i].lanNetInfo.model][0],
					rg_lanNet_os[(int)rg_db.arp[i].lanNetInfo.os][0],
					rg_db.lut[rg_db.arp[i].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			PROC_PRINTF("\t -> L2:%d (Idle %d Secs P:%d)\n", asic_arp.nhIdx, rg_db.arp[i].idleSecs,rg_db.arp[i].permit_for_l34_forward);
#endif
		}

	}

#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF(">>Arp Hardware list:\n");
	for(i=0;i<MAX_ARP_HW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.hardwareArpTableHead[i]))
		{
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpTableHead[i],arp_list)
			{
				inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s P:%d) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC",rg_db.arp[pSoftwareArp->idx].permit_for_l34_forward);
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF(">>Arp Hardware Free list:\n");
	list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpFreeListHead,arp_list)
	{
		inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
		PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s) \n",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC");
	}
	PROC_PRINTF("\n");

	PROC_PRINTF(">> ARP valid:\n");
	for(i=0;i<(MAX_ARP_HW_TABLE_SIZE/32);i++) PROC_PRINTF("[%08x]",rg_db.arpValidSet[i]);
	PROC_PRINTF("\n");
#endif


	PROC_PRINTF(">>Arp Software Table:\n");
	for(i=0;i<MAX_ARP_SW_TABLE_HEAD;i++){
		if(!list_empty(&rg_db.softwareArpTableHead[i])){
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.softwareArpTableHead[i],arp_list){
				inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s P:%d) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC",rg_db.arp[pSoftwareArp->idx].permit_for_l34_forward);
				if(rg_db.systemGlobal.gatherLanNetInfo)
					PROC_PRINTF("Name:%s DevType:%s Brand:%s Model:%s OS:%s ConnType:%s\n",
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].dev_name,
						rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
						rg_lanNet_brand[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.brand][0],
						rg_lanNet_model[(int)rg_db.arp[i].lanNetInfo.model][0],
						rg_lanNet_os[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.os][0],
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			}
			PROC_PRINTF("\n");
		}
	}

    return len;
}

int32 dump_sw_arp(struct seq_file *s, void *v)
{
	uint32	i;
	rtk_rg_arp_linkList_t *pSoftwareArp;
	int len=0;

	int8 ipBuf[sizeof"255.255.255.255"];

#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF(">>Arp Hardware list:\n");
	for(i=0;i<MAX_ARP_HW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.hardwareArpTableHead[i]))
		{
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpTableHead[i],arp_list)
			{
				inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s P:%d) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC",rg_db.arp[pSoftwareArp->idx].permit_for_l34_forward);
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF(">>Arp Hardware Free list:\n");
	list_for_each_entry(pSoftwareArp,&rg_db.hardwareArpFreeListHead,arp_list)
	{
		inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
		PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s) \n",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC");
	}
	PROC_PRINTF("\n");

	PROC_PRINTF(">> ARP valid:\n");
	for(i=0;i<(MAX_ARP_HW_TABLE_SIZE/32);i++) PROC_PRINTF("[%08x]",rg_db.arpValidSet[i]);
	PROC_PRINTF("\n");
#endif

	PROC_PRINTF(">>Arp Software Table:\n");
	for(i=0;i<MAX_ARP_SW_TABLE_HEAD;i++){
		if(!list_empty(&rg_db.softwareArpTableHead[i])){
			PROC_PRINTF("  [%3d] : ",i);
			list_for_each_entry(pSoftwareArp,&rg_db.softwareArpTableHead[i],arp_list){
				inet_ntoa_r(rg_db.arp[pSoftwareArp->idx].ipv4Addr, ipBuf);
				PROC_PRINTF("->%-16s([%d],l2:%d,l3:%d,idle:%d%s P:%d) ",ipBuf,pSoftwareArp->idx,rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx,rg_db.arp[pSoftwareArp->idx].routingIdx,rg_db.arp[pSoftwareArp->idx].idleSecs,rg_db.arp[pSoftwareArp->idx].staticEntry==1?",STATIC":",DYNAMIC",rg_db.arp[pSoftwareArp->idx].permit_for_l34_forward);
				if(rg_db.systemGlobal.gatherLanNetInfo)
					PROC_PRINTF("Name:%s DevType:%s Brand:%s Model:%s OS:%s ConnType:%s\n",
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].dev_name,
						rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[pSoftwareArp->idx].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
						rg_lanNet_brand[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.brand][0],
						rg_lanNet_model[(int)rg_db.arp[i].lanNetInfo.model][0],
						rg_lanNet_os[(int)rg_db.arp[pSoftwareArp->idx].lanNetInfo.os][0],
						rg_db.lut[rg_db.arp[pSoftwareArp->idx].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			}
			PROC_PRINTF("\n");
		}
	}

    return len;
}


int32 dump_ip(struct seq_file *s, void *v)
{

	rtk_l34_ext_intip_entry_t asic_ip;
    int32	i;
    int8 intIpBuf[sizeof"255.255.255.255"];
    int8 extIpBuf[sizeof"255.255.255.255"];
	int len=0;

    PROC_PRINTF(">>IP ASIC Table:\n");
    for(i=0; i<MAX_EXTIP_HW_TABLE_SIZE; i++)
    {
        int retval = 0;
		memset(&asic_ip,0,sizeof(rtk_l34_ext_intip_entry_t));
		retval = rtk_l34_extIntIPTable_get(i,&asic_ip);
        if ( retval != RT_ERR_OK)
        {
            PROC_PRINTF("  [%d] (Invalid) --> %x.\n", i,retval);
            continue;
        }
        else if (asic_ip.valid==0)
        {
            PROC_PRINTF("  [%d] (Invalid)\n", i);
            continue;
        }
        else
        {
            inet_ntoa_r(asic_ip.intIpAddr, intIpBuf);
            inet_ntoa_r(asic_ip.extIpAddr,extIpBuf);
            PROC_PRINTF("  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d) PriValid(%d) Priority(%d)\n",
                           i, intIpBuf,extIpBuf,
                           ((asic_ip.type&0x2)!=0? "LP" : ((asic_ip.type&0x1)!=0 ? "NAT" : "NAPT")), asic_ip.nhIdx, asic_ip.prival, asic_ip.pri);
        }
    }


    return len;
}

int32 dump_sw_ip(struct seq_file *s, void *v)
{
	char validType[4][15] = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};
    int32	i;
    int8 intIpBuf[sizeof"255.255.255.255"];
    int8 extIpBuf[sizeof"255.255.255.255"];
	int len=0;

    PROC_PRINTF(">>IP SW Table:\n");
    for(i=0; i<MAX_EXTIP_SW_TABLE_SIZE; i++)
    {

        if (rg_db.extip[i].rtk_extip.valid==0)
        {
            //PROC_PRINTF("  [%d] (Invalid)\n", i);
            continue;
        }
        else
        {
            inet_ntoa_r(rg_db.extip[i].rtk_extip.intIpAddr, intIpBuf);
            inet_ntoa_r(rg_db.extip[i].rtk_extip.extIpAddr,extIpBuf);
            PROC_PRINTF("  [%d](%s)\t intip(%-14s) extip(%-14s) type(%s) nhIdx(%d) PriValid(%d) Priority(%d) refCount(%d)\n",
                           i,validType[rg_db.extip[i].valid], intIpBuf,extIpBuf,
                           ((rg_db.extip[i].rtk_extip.type&0x2)!=0? "LP" : ((rg_db.extip[i].rtk_extip.type&0x1)!=0 ? "NAT" : "NAPT")), rg_db.extip[i].rtk_extip.nhIdx, rg_db.extip[i].rtk_extip.prival, rg_db.extip[i].rtk_extip.pri, rg_db.systemGlobal.eipRefCount[i]);
        }
    }


    return len;
}


int32 dump_ipv6_extIp(struct seq_file *s, void *v){
	int len=0;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	int i;
	rtk_rg_table_v6ExtIp_t v6ExtIp;


	for(i=0;i<MAX_IPV6_ROUTING_SW_TABLE_SIZE;i++){
		bzero(&v6ExtIp,sizeof(v6ExtIp));
		_rtk_rg_ipv6_externalIp_get(i,&v6ExtIp);

		if(v6ExtIp.valid==1){
			PROC_PRINTF("[%d] EXT_IP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)  NextHop[%d]\n",i,
			v6ExtIp.externalIp.ipv6_addr[0],v6ExtIp.externalIp.ipv6_addr[1],v6ExtIp.externalIp.ipv6_addr[2],v6ExtIp.externalIp.ipv6_addr[3],
			v6ExtIp.externalIp.ipv6_addr[4],v6ExtIp.externalIp.ipv6_addr[5],v6ExtIp.externalIp.ipv6_addr[6],v6ExtIp.externalIp.ipv6_addr[7],
			v6ExtIp.externalIp.ipv6_addr[8],v6ExtIp.externalIp.ipv6_addr[9],v6ExtIp.externalIp.ipv6_addr[10],v6ExtIp.externalIp.ipv6_addr[11],
			v6ExtIp.externalIp.ipv6_addr[12],v6ExtIp.externalIp.ipv6_addr[13],v6ExtIp.externalIp.ipv6_addr[14],v6ExtIp.externalIp.ipv6_addr[15],
			v6ExtIp.nextHopIdx);
		}else{
			PROC_PRINTF("[%d] (Invalid)\n",i);
		}
	}
#endif
	return len;
}

int32 dump_l3(struct seq_file *s, void *v)
{

	rtk_l34_routing_entry_t asic_l3;
	uint32 idx, retval;
	int8 *str[4] = { "CPU", "DROP", "ARP", "NxtHop"};
#if defined(CONFIG_RG_RTL9600_SERIES)
	int8 *nhstr[3] = {"PER-PACKET","PER-SESSION","PER-SIP"};
#endif
	int len=0;
	PROC_PRINTF(">>L3 Asic Routing Table:\n");
	for(idx=0; idx<MAX_L3_HW_TABLE_SIZE; idx++)
	{
	    retval = rtk_l34_routingTable_get(idx, &asic_l3);
	    if ( retval != RT_ERR_OK)
	    {
	        PROC_PRINTF("[%d]  (Invalid)\n", idx);
	        continue;
	    }
	    else
	    {
			if(asic_l3.ipAddr!=0) asic_l3.ipMask++;

			if(asic_l3.valid==1)
			{
		        PROC_PRINTF("[%d] Valid %d.%d.%d.%d/%d Internal(%d) RT2WAN(%d)\n", idx, (asic_l3.ipAddr>>24)&0xff,
		        (asic_l3.ipAddr>>16)&0xff, (asic_l3.ipAddr>>8)&0xff, (asic_l3.ipAddr&0xff),
		        asic_l3.ipMask,asic_l3.internal,asic_l3.rt2waninf);
			}
			else
			{
				PROC_PRINTF("[%d]  (Invalid)\n", idx);
	        	continue;
			}

		    switch(asic_l3.process)
		    {
		    case 0x00:	/* CPU */
		        PROC_PRINTF("            [%s PROCESS] \n\n", str[asic_l3.process]);
		        break;

		    case 0x01:	/* DROP */
		        PROC_PRINTF("            [%s PROCESS] \n\n", str[asic_l3.process]);
		        break;

#if defined(CONFIG_RG_RTL9602C_SERIES)
			case 0x02:	/* ARP */
				PROC_PRINTF("			[%s PROCESS]: NET_IF(%d)\n\n", str[asic_l3.process],asic_l3.netifIdx);
				break;
			case 0x03:	/* NextHop */
				PROC_PRINTF("			[%s PROCESS]: NEXT_HOP(%d)\n\n", str[asic_l3.process],asic_l3.nhNxt);


#elif defined(CONFIG_RG_RTL9600_SERIES)
			case 0x02:	/* ARP */
				PROC_PRINTF("			[%s PROCESS]: NETIF(%d) ARPSTA(%d) ARPEND(%d) \n\n", str[asic_l3.process],asic_l3.netifIdx,asic_l3.arpStart, asic_l3.arpEnd);
				break;
			case 0x03:	/* NextHop */
				PROC_PRINTF("			[%s PROCESS]: NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%s) IPDOMAIN(%d)\n\n", str[asic_l3.process],asic_l3.nhStart,
							   asic_l3.nhNum, asic_l3.nhNxt, nhstr[asic_l3.nhAlgo], asic_l3.ipDomain);

#endif



		        break;
		    default:
		        assert(0);
		    }
		}
	}



    return len;
}


int32 dump_sw_l3(struct seq_file *s, void *v)
{
	char validType[4][15] = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};
	uint32 idx;
	int8 *str[4] = { "CPU", "DROP", "ARP", "NxtHop"};
#if defined(CONFIG_RG_RTL9600_SERIES)
	int8 *nhstr[3] = {"PER-PACKET","PER-SESSION","PER-SIP"};
#endif
	int len=0;


	PROC_PRINTF(">>L3 SW Routing Table:\n");
	for(idx=0; idx<MAX_L3_SW_TABLE_SIZE; idx++)
	{
		int addOne=0;

		if(rg_db.l3[idx].rtk_l3.ipAddr!=0) addOne=1;

		if(rg_db.l3[idx].valid)
		{
			PROC_PRINTF("[%d](%s)\t  %d.%d.%d.%d/%d Internal(%d) RT2WAN(%d)\n", idx,validType[rg_db.l3[idx].valid], (rg_db.l3[idx].rtk_l3.ipAddr>>24)&0xff,
			(rg_db.l3[idx].rtk_l3.ipAddr>>16)&0xff, (rg_db.l3[idx].rtk_l3.ipAddr>>8)&0xff, (rg_db.l3[idx].rtk_l3.ipAddr&0xff),
			rg_db.l3[idx].rtk_l3.ipMask+addOne,rg_db.l3[idx].rtk_l3.internal,rg_db.l3[idx].rtk_l3.rt2waninf);
		}
		else
		{
			//PROC_PRINTF("[%d]  (Invalid)\n", idx);
			continue;
		}

		switch(rg_db.l3[idx].rtk_l3.process)
		{
		case 0x00:	/* CPU */
			PROC_PRINTF("			 [%s PROCESS] \n\n", str[rg_db.l3[idx].rtk_l3.process]);
			break;

		case 0x01:	/* DROP */
			PROC_PRINTF("			 [%s PROCESS] \n\n", str[rg_db.l3[idx].rtk_l3.process]);
			break;

#if defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		case 0x02:	/* ARP */
			PROC_PRINTF("			[%s PROCESS]: NET_IF(%d)\n\n", str[rg_db.l3[idx].rtk_l3.process],rg_db.l3[idx].rtk_l3.netifIdx);
			break;
		case 0x03:	/* NextHop */
			PROC_PRINTF("			[%s PROCESS]: NEXT_HOP(%d)\n\n", str[rg_db.l3[idx].rtk_l3.process],rg_db.l3[idx].rtk_l3.nhNxt);


#elif defined(CONFIG_RG_RTL9600_SERIES)
		case 0x02:	/* ARP */
			PROC_PRINTF("			[%s PROCESS]: NETIF(%d) ARPSTA(%d) ARPEND(%d) \n\n", str[rg_db.l3[idx].rtk_l3.process],rg_db.l3[idx].rtk_l3.netifIdx,rg_db.l3[idx].rtk_l3.arpStart, rg_db.l3[idx].rtk_l3.arpEnd);
			break;
		case 0x03:	/* NextHop */
			PROC_PRINTF("			[%s PROCESS]: NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%s) IPDOMAIN(%d)\n\n", str[rg_db.l3[idx].rtk_l3.process],rg_db.l3[idx].rtk_l3.nhStart,
						   rg_db.l3[idx].rtk_l3.nhNum, rg_db.l3[idx].rtk_l3.nhNxt, nhstr[rg_db.l3[idx].rtk_l3.nhAlgo], rg_db.l3[idx].rtk_l3.ipDomain);

#endif

			break;
		default:
			assert(0);
		}

	}




    return len;
}




int32 dump_l4hs(struct seq_file *s, void *v)
{
	int retval = 0;
	rtk_l34_hsb_t hsb_r;
	rtk_l34_hsa_t hsa_r;
	int len=0;

	memset(&hsb_r,0,sizeof(rtk_l34_hsb_t));
	memset(&hsa_r,0,sizeof(rtk_l34_hsa_t));
	retval = rtk_l34_hsbData_get(&hsb_r);
	ASSERT_EQ(retval,RT_ERR_OK);
	retval = rtk_l34_hsaData_get(&hsa_r);
	ASSERT_EQ(retval,RT_ERR_OK);

	PROC_PRINTF("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsb_S(&hsb_r,s);
	PROC_PRINTF("----------------------------------------------\n");
	PROC_PRINTF("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsa_S(&hsa_r,s);
	PROC_PRINTF("----------------------------------------------\n");

#if 0
	l4_hsb_param_t hsb_r;
	l4_hsa_param_t hsa_r;
	memset((void*)&hsb_r,0,sizeof(hsb_r));
	memset((void*)&hsa_r,0,sizeof(hsa_r));

	apollo_virtualMacGetL4Hsb( &hsb_r );
	apollo_virtualMacGetL4Hsa( &hsa_r );

	PROC_PRINTF("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsb_S(&hsb_r);
	PROC_PRINTF("----------------------------------------------\n");
	PROC_PRINTF("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
	hs_displayL4Hsa_S(&hsa_r);
	PROC_PRINTF("----------------------------------------------\n");
#endif
    return len;
}

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES) || defined(CONFIG_RG_G3_SERIES)
int _rtk_rg_aclHeaderInfo_get(char *acl_dbg_info, int acl_dbg_info_size, int ref_info)
{
	bzero(&rg_db.systemGlobal.aclSWEntry_for_find, sizeof(rg_db.systemGlobal.aclSWEntry_for_find));
		
	if(ref_info&RXINFO_REF_ACL_RSN_CTRL_RSV)
	{
		if(ref_info&RXINFO_REF_ACL_RSN_CTRL_TRAP)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap by RSV_ACL[%d])", (ref_info&RXINFO_REF_ACL_RSN_VAL));
		else
			snprintf(acl_dbg_info, acl_dbg_info_size, "(hit RSV_ACL[%d])", (ref_info&RXINFO_REF_ACL_RSN_VAL));
		return RT_ERR_RG_OK;
	}
	ref_info &= RXINFO_REF_ACL_RSN_VAL;
	if(_rtk_rg_aclSWEntry_get(ref_info, &rg_db.systemGlobal.aclSWEntry_for_find) == RT_ERR_RG_OK)
	{
		if(rg_db.systemGlobal.aclSWEntry_for_find.acl_filter.action_type==ACL_ACTION_TYPE_TRAP)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap by RG_ACL[%d])", ref_info);
		else if(rg_db.systemGlobal.aclSWEntry_for_find.acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap to ps by RG_ACL[%d])", ref_info);
		else if(rg_db.systemGlobal.aclSWEntry_for_find.acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap+pri%d by RG_ACL[%d])", rg_db.systemGlobal.aclSWEntry_for_find.acl_filter.action_trap_with_priority, ref_info);
#if defined(CONFIG_RG_G3_SERIES)		
		else
			snprintf(acl_dbg_info, acl_dbg_info_size, "(hit RG_ACL[%d])", ref_info);
#endif
	}

	return RT_ERR_RG_OK;
}
#endif

rtk_hsb_t rawHsb;
rtk_hsa_t rawHsa;
rtk_hsa_debug_t rawHsd;

int32 dump_hs(struct seq_file *s, void *v)
{
	int32 ret,value;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	char acl_dbg_info[64];
#endif

	ret=0;
    memset((void*)&rawHsb,0,sizeof(rawHsb));
    memset((void*)&rawHsa,0,sizeof(rawHsa));
    memset((void*)&rawHsd,0,sizeof(rawHsd));


#if defined(CONFIG_RG_RTL9600_SERIES)
	//disable hsa latch ,stop hab update
	ioal_mem32_read((uint32)(0x00028000),&value);
	value = (value & (~0x1C)) | (0x1<<2);
	ioal_mem32_write((uint32)(0x00028000),value);

	ret = apollomp_raw_hsbData_get(&rawHsb);
    ASSERT_EQ(ret,RT_ERR_OK);
    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
    ret = apollomp_raw_hsaData_get(&rawHsa);
    ASSERT_EQ(ret,RT_ERR_OK);
    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
    ret = apollomp_raw_hsdData_get(&rawHsd);
    ASSERT_EQ(ret,RT_ERR_OK);

	//enable hab update
	ioal_mem32_read((uint32)(0x00028000),&value);
	value = (value & (~0x1C)) ;
	ioal_mem32_write((uint32)(0x00028000),value);

#elif defined(CONFIG_RG_RTL9602C_SERIES)
	//disable hsa latch ,stop hab update
	ioal_mem32_read((uint32)(0x00028000),&value);
	value = (value & (~0x1C)) | (0x1<<2);
	ioal_mem32_write((uint32)(0x00028000),value);

	ret = rtl9602c_hsbData_get(&rawHsb);
    ASSERT_EQ(ret,RT_ERR_OK);
    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
    ret = rtl9602c_hsaData_get(&rawHsa);
    ASSERT_EQ(ret,RT_ERR_OK);
    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
    ret = rtl9602c_hsdData_get(&rawHsd);
    ASSERT_EQ(ret,RT_ERR_OK);

	//enable hab update
	ioal_mem32_read((uint32)(0x00028000),&value);
	value = (value & (~0x1C)) ;
	ioal_mem32_write((uint32)(0x00028000),value);
#elif defined(CONFIG_RG_RTL9607C_SERIES)
	//disable hsa latch ,stop hab update (config as non-latch mode)
	rtk_rg_asic_l2HsbaLatchMode_set(DISABLED);

	ret = rtl9607c_hsbData_get(&rawHsb);
	ASSERT_EQ(ret,RT_ERR_OK);
	ret = rtl9607c_hsbPar_get(&rawHsb);
	ASSERT_EQ(ret,RT_ERR_OK);

	ret = rtl9607c_hsaData_get(&rawHsa);
	ASSERT_EQ(ret,RT_ERR_OK);

	ret = rtl9607c_hsdData_get(&rawHsd);
	ASSERT_EQ(ret,RT_ERR_OK);

	if(rawHsa.dbghsa_oq_fb){ // via FB
		ret = rtl9607c_fbData_get(&rawHsa);
		ASSERT_EQ(ret,RT_ERR_OK);
		value = 1;
	}

	//enable hab update (config as all-latch mode)
	rtk_rg_asic_l2HsbaLatchMode_set(ENABLED);

	if((rawHsb.bgdsc != rawHsa.dbghsa_bgdsc) || (rawHsb.endsc != rawHsa.dbghsa_oq_endsc))
	{
		rtlglue_printf("HSB/HSA not match! please try again ...\n");

		return SUCCESS;
	}
#elif  defined(CONFIG_RG_RTL9603CVD_SERIES)
	//disable hsa latch ,stop hab update (config as non-latch mode)
	rtk_rg_asic_l2HsbaLatchMode_set(DISABLED);

	ret = rtl9603cvd_hsbData_get(&rawHsb);
	ASSERT_EQ(ret,RT_ERR_OK);
	ret = rtl9603cvd_hsbPar_get(&rawHsb);
	ASSERT_EQ(ret,RT_ERR_OK);

	ret = rtl9603cvd_hsaData_get(&rawHsa);
	ASSERT_EQ(ret,RT_ERR_OK);

	ret = rtl9603cvd_hsdData_get(&rawHsd);
	ASSERT_EQ(ret,RT_ERR_OK);

	if(rawHsa.dbghsa_oq_fb){ // via FB
		ret = rtl9603cvd_fbData_get(&rawHsa);
		ASSERT_EQ(ret,RT_ERR_OK);
		value = 1;
	}

	//enable hab update (config as all-latch mode)
	rtk_rg_asic_l2HsbaLatchMode_set(ENABLED);

	if((rawHsb.bgdsc != rawHsa.dbghsa_bgdsc) || (rawHsb.endsc != rawHsa.dbghsa_oq_endsc))
	{
		rtlglue_printf("HSB/HSA not match! please try again ...\n");

		return SUCCESS;
	}
#elif defined(CONFIG_RG_G3_SERIES)
	ret = RT_ERR_OK;
	value = 1;
#else
#error "please add "
#endif

	/* HSB display */
	rtlglue_printf("---- "COLOR_Y "[HSB:]" COLOR_NM "------------------------------------\n");
	_diag_debug_hsb_display(&rawHsb);
	rtlglue_printf("----------------------------------------------\n");

	/* HSA display */
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	memset(acl_dbg_info, 0, sizeof(acl_dbg_info));
	if(rawHsa.dbghsa_epcom_fwdrsn == CPU_REASON_ACL)
		_rtk_rg_aclHeaderInfo_get(acl_dbg_info, sizeof(acl_dbg_info), (rawHsa.dbghsa_grp0_cvlan_info&RXINFO_REF_ACL_RSN_BIT));
	rtlglue_printf("---- "COLOR_Y "[HSA: (by %s)]" COLOR_NM COLOR_G "%s" COLOR_NM "------------------------------------\n", value?"FB":"switch", acl_dbg_info);
#else
	rtlglue_printf("---- "COLOR_Y "[HSA:]" COLOR_NM "------------------------------------\n");
#endif
	_diag_debug_hsa_display(&rawHsa);
	rtlglue_printf("----------------------------------------------\n");

	/* HSD display */
	rtlglue_printf("---- "COLOR_Y "[HSD:]" COLOR_NM "------------------------------------\n");
	_diag_debug_hsd_display(&rawHsd);
	rtlglue_printf("----------------------------------------------\n");

    return SUCCESS;

}



int tracefilterRULE0_hs_period_time=0;
int initTimer=0;
#ifdef TIMER_AGG
rtk_rg_timer_t dumphs_timer;
#else
struct timer_list dumphs_timer;
#endif



void tracefilterRULE0_dump_hs(unsigned long data )
{
#if !defined(CONFIG_RG_G3_SERIES)
	int ret=0,value;
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_l34_hsb_t hsb_r;
	rtk_l34_hsa_t hsa_r;
#else
	rtk_rg_asic_hsb_entry_t hsb_r;
	rtk_rg_asic_hsa_entry_t hsa_r;
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	char acl_dbg_info[64];
#endif

	if(tracefilterRULE0_hs_period_time==0)
	{
#if defined(TIMER_AGG)
		rtlglue_printf("set dump_hs timer period to long time  msec=%lu\n",msecs_to_jiffies(-1));
		_rtk_rg_mod_timer(&dumphs_timer, jiffies+msecs_to_jiffies(-1));
		initTimer=0;
#else
		rtlglue_printf("Disable dump_hs timer period!\n");
		_rtk_rg_del_timer(&dumphs_timer);
		initTimer=0;
#endif
		return ;
	}

	if(rg_kernel.traceFilterRuleMask&0x1)
	{

		if(rg_kernel.trace_filter_bitmask[0]==RTK_RG_DEBUG_TRACE_FILTER_SHOWNUMBEROFTIMES || rg_kernel.trace_filter_bitmask[0]==0)
			goto NEXTTIMER;

		memset((void*)&rawHsb,0,sizeof(rawHsb));
		memset((void*)&rawHsa,0,sizeof(rawHsa));
		memset((void*)&rawHsd,0,sizeof(rawHsd));
		memset(&hsb_r,0,sizeof(hsb_r));
		memset(&hsa_r,0,sizeof(hsa_r));

#if defined(CONFIG_RG_RTL9600_SERIES)
		//disable hsa latch ,stop hab update
		ioal_mem32_read((uint32)(0x00028000),&value);
		value = (value & (~0x1C)) | (0x1<<2);
		ioal_mem32_write((uint32)(0x00028000),value);

		ret = apollomp_raw_hsbData_get(&rawHsb);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
	    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
	    ret = apollomp_raw_hsaData_get(&rawHsa);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
	    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
	    ret = apollomp_raw_hsdData_get(&rawHsd);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
#elif defined(CONFIG_RG_RTL9602C_SERIES)
		//disable hsa latch ,stop hab update
		ioal_mem32_read((uint32)(0x00028000),&value);
		value = (value & (~0x1C)) | (0x1<<2);
		ioal_mem32_write((uint32)(0x00028000),value);

		ret = rtl9602c_hsbData_get(&rawHsb);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
	    //convertAsicHsbToSoftware(&rawHsb,&hsb_r);
	    ret = rtl9602c_hsaData_get(&rawHsa);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
	    //convertAsicHsaToSoftware(&rawHsa,&hsa_r);
	    ret = rtl9602c_hsdData_get(&rawHsd);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
#elif defined(CONFIG_RG_RTL9607C_SERIES)
		//disable hsa latch ,stop hab update (config as non-latch mode)
		rtk_rg_asic_l2HsbaLatchMode_set(DISABLED);

		ret = rtl9607c_hsbData_get(&rawHsb);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}

		ret = rtl9607c_hsaData_get(&rawHsa);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}


		if(rawHsa.dbghsa_oq_fb){ // via FB
			ret = rtl9607c_fbData_get(&rawHsa);
			if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
			value = 1;
		}

		if((rawHsb.bgdsc != rawHsa.dbghsa_bgdsc) || (rawHsb.endsc != rawHsa.dbghsa_oq_endsc))
		{
			rtlglue_printf("HSB/HSA not match! please wait next timer expired ...\n");
			goto NEXTTIMER;
		}

		ret = rtl9607c_hsdData_get(&rawHsd);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
#elif  defined(CONFIG_RG_RTL9603CVD_SERIES)
		//disable hsa latch ,stop hab update (config as non-latch mode)
		rtk_rg_asic_l2HsbaLatchMode_set(DISABLED);

		ret = rtl9603cvd_hsbData_get(&rawHsb);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}

		ret = rtl9603cvd_hsaData_get(&rawHsa);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}


		if(rawHsa.dbghsa_oq_fb){ // via FB
			ret = rtl9603cvd_fbData_get(&rawHsa);
			if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
			value = 1;
		}

		if((rawHsb.bgdsc != rawHsa.dbghsa_bgdsc) || (rawHsb.endsc != rawHsa.dbghsa_oq_endsc))
		{
			rtlglue_printf("HSB/HSA not match! please wait next timer expired ...\n");
			goto NEXTTIMER;
		}

		ret = rtl9603cvd_hsdData_get(&rawHsd);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}

#else
#error "please add "
#endif

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		ret = rtk_l34_hsbData_get(&hsb_r);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
		ret = rtk_l34_hsaData_get(&hsa_r);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
#else
		ret = rtk_rg_asic_hsbData_get(&hsb_r);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
		ret = rtk_rg_asic_hsaData_get(&hsa_r);
		if(ret!=RT_ERR_OK) {rtlglue_printf("ret failed %s:%d\n",__FUNCTION__, __LINE__); goto NEXTTIMER;}
#endif


#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
		//enable hab update
		ioal_mem32_read((uint32)(0x00028000),&value);
		value = (value & (~0x1C)) ;
		ioal_mem32_write((uint32)(0x00028000),value);
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		rtk_rg_asic_l2HsbaLatchMode_set(ENABLED);
#endif


		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_SPA)
		{
				if(rg_kernel.trace_filter[0].spa!=rawHsb.spa) goto NEXTTIMER;

		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_DA)
		{
			if(memcmp(&rg_kernel.trace_filter[0].dmac.octet[0],&rawHsb.da.octet[0],6)!=0) goto NEXTTIMER;
		}

		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_SA)
		{
			if(memcmp(&rg_kernel.trace_filter[0].smac.octet[0],&rawHsb.sa.octet[0],6)!=0) goto NEXTTIMER;
		}

		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_SIP )
		{
			if(rg_kernel.trace_filter[0].sip!=rawHsb.sip)
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_DIP )
		{
			if(rg_kernel.trace_filter[0].dip!=rawHsb.dip)
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_V6SIP)
		{

			if(memcmp(&rg_kernel.trace_filter[0].sipv6[12],&rawHsb.sip,4)!=0)
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_V6DIP)
		{
			if(memcmp(&rg_kernel.trace_filter[0].dipv6[12],&rawHsb.dip,4)!=0)
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_CVLAN )
		{
			if(rawHsb.ctag_if)
			{
				if( rg_kernel.trace_filter[0].cvlanid != rawHsb.ctag)
					goto NEXTTIMER;
			}else
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_SVLAN )
		{
			if(rawHsb.stag_if)
			{
				if( rg_kernel.trace_filter[0].svlanid != rawHsb.stag)
					goto NEXTTIMER;
			}else
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_PPPOESESSIONID )
		{
			if(rawHsb.pppoe_if)
			{
				if( rg_kernel.trace_filter[0].sessionid!= rawHsb.pppoe_session)
					goto NEXTTIMER;
			}else
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_ETH )
		{
			if( rg_kernel.trace_filter[0].ethertype!= rawHsb.ether_type)
				goto NEXTTIMER;
		}
		if(rg_kernel.trace_filter_bitmask[0]&RTK_RG_DEBUG_TRACE_FILTER_REASON )
		{
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
			if( rg_kernel.trace_filter[0].reason!= rawHsa.dbghsa_epcom_fwdrsn)
				goto NEXTTIMER;
#else
			if( rg_kernel.trace_filter[0].reason!= rawHsa.rng_nhsab_fwdrsn)
				goto NEXTTIMER;
#endif
		}



		rtlglue_printf("---- "COLOR_Y "[HSB:]" COLOR_NM "------------------------------------\n");
		_diag_debug_hsb_display(&rawHsb);
		rtlglue_printf("----------------------------------------------\n");
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		memset(acl_dbg_info, 0, sizeof(acl_dbg_info));
		if(rawHsa.dbghsa_epcom_fwdrsn == CPU_REASON_ACL)
			_rtk_rg_aclHeaderInfo_get(acl_dbg_info, sizeof(acl_dbg_info), (rawHsa.dbghsa_grp0_cvlan_info&RXINFO_REF_ACL_RSN_BIT));
		rtlglue_printf("---- "COLOR_Y "[HSA: (by %s)]" COLOR_NM COLOR_G "%s" COLOR_NM "------------------------------------\n", value?"FB":"switch", acl_dbg_info);
#else
		rtlglue_printf("---- "COLOR_Y "[HSA:]" COLOR_NM "------------------------------------\n");
#endif
		_diag_debug_hsa_display(&rawHsa);
		rtlglue_printf("----------------------------------------------\n");
		rtlglue_printf("---- "COLOR_Y "[HSD:]" COLOR_NM "------------------------------------\n");
		_diag_debug_hsd_display(&rawHsd);
		rtlglue_printf("----------------------------------------------\n\n");

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		rtlglue_printf("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
		dump_l34hsb_display(&hsb_r);
		rtlglue_printf("----------------------------------------------\n");
		rtlglue_printf("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
		dump_l34hsa_display(&hsa_r);
		rtlglue_printf("----------------------------------------------\n");

#else

		rtlglue_printf("---- "COLOR_Y "[L4HSB:]" COLOR_NM "------------------------------------\n");
		hs_displayL4Hsb_S(&hsb_r,NULL);
		rtlglue_printf("----------------------------------------------\n");
		rtlglue_printf("---- "COLOR_Y "[L4HSA:]" COLOR_NM "------------------------------------\n");
		hs_displayL4Hsa_S(&hsa_r,NULL);
		rtlglue_printf("----------------------------------------------\n");
#endif

#if defined(TIMER_AGG)
		rtlglue_printf("set dump_hs timer period to long time  msec=%lu\n",msecs_to_jiffies(-1));
		_rtk_rg_mod_timer(&dumphs_timer, jiffies+msecs_to_jiffies(-1));
		initTimer=0;
#else
		rtlglue_printf("Disable dump_hs timer period!\n");
		_rtk_rg_del_timer(&dumphs_timer);
		initTimer=0;
#endif
		return ;



	}

NEXTTIMER:

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_asic_l2HsbaLatchMode_set(ENABLED);
#endif
	_rtk_rg_mod_timer(&dumphs_timer, jiffies+msecs_to_jiffies(tracefilterRULE0_hs_period_time));

#endif  //!defined(CONFIG_RG_G3_SERIES)
	return ;
}

int tracefilterRULE0_dump_hs_timer( struct file *filp, const char *buff,unsigned long len, void *data )
{

	tracefilterRULE0_hs_period_time=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	if(tracefilterRULE0_hs_period_time<0)
		tracefilterRULE0_hs_period_time=1;

	rtlglue_printf("SUPPORT tracefilterRULE0  SPA ETHTYPE CVLAN SVLAN PPPOESSID DA/48 SA/48 DIP/32 SIP/32 DIP6/32 SIP6/32 \n");

	if(tracefilterRULE0_hs_period_time==0)
	{
		rtlglue_printf("disable get dump hs period\n");
		return len;
	}
	else
	{
		rtlglue_printf("dumpHs period timer=%d (ms)\n",tracefilterRULE0_hs_period_time);
		if(initTimer==0){
#if defined(TIMER_AGG)
			_rtk_rg_del_timer(&dumphs_timer);
#endif
			//setup_timer(&dumphs_timer,tracefilterRULE0_dump_hs,0);
			bzero(&dumphs_timer,sizeof(dumphs_timer));
			dumphs_timer.function=tracefilterRULE0_dump_hs;
			_rtk_rg_init_timer(&dumphs_timer);
			_rtk_rg_mod_timer(&dumphs_timer,jiffies+msecs_to_jiffies(tracefilterRULE0_hs_period_time));
			initTimer=1;
			printk("TIMER INIT SUCCESS\n");
		}
	}

	return len;
}


int32 dump_lut_group_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	int i;
	rtk_rg_lut_linkList_t *pLutEntry;

	PROC_PRINTF(">>lut group table:\n");
	for(i=0; i<MAX_LUT_HW_TABLE_SIZE; i++)
	{
		if(!list_empty(&rg_db.lutGroupTableHead[i]))
		{
			PROC_PRINTF(" Lut group[%3d] : \n", i);
			list_for_each_entry(pLutEntry, &rg_db.lutGroupTableHead[i], lut_list)
			{
				if(pLutEntry->lut_list.next!=&rg_db.lutGroupTableHead[i])
					PROC_PRINTF("	 Lut[%d] ->\n", pLutEntry->idx);
				else
					PROC_PRINTF("	 Lut[%d]\n", pLutEntry->idx);
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF("\n");

	return retval;
}



int32 dump_lut(int idx,struct seq_file *s)
{
	rtk_l2_addr_table_t	data;
	int in_idx=idx;
	int out_idx=in_idx;
	int ret;

	data.method = LUT_READ_METHOD_NEXT_ADDRESS;
	ret=rtk_l2_nextValidEntry_get(&out_idx,&data);

	if(ret!=RT_ERR_OK)
	{
		PROC_PRINTF("rtk_l2_nextValidEntry_get Error at idx=%d\n",idx);
		return FAIL;
	}

	if(in_idx==out_idx)
	{
		dump_lut_display(idx, &data);
	}

	return SUCCESS;
}

int32 dump_lut_table(struct seq_file *s, void *v)
{
	int i = 0;
	int r = 0;
	int len=0;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rtk_l2_addr_table_t l2table;
	do
	{
		bzero(&l2table, sizeof(rtk_l2_addr_table_t));
		l2table.method = LUT_READ_METHOD_NEXT_ADDRESS;

		i =	r;
		if(RT_ERR_OK ==	(rtk_l2_nextValidEntry_get(&r, &l2table)))
		{
			/* Wrap	around */
			if(r < i)
			{
				break;
			}

			dump_lut_display(r, &l2table);
			r++;
		}
		else
		{
			break;
		}
	} while(1);
#else
	for(i=0; i<MAX_LUT_HW_TABLE_SIZE; i++)
	{
		r=dump_lut(i,s);
		if(r==FAIL) break;
	}
#endif

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
	{
		rtk_rg_lut_linkList_t *plutCamEntry;
		PROC_PRINTF("\n>>lutCam list:\n");
		for(i=0;i<(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)>>MAX_LUT_HASH_WAY_SHIFT;i++)
		{
			if(!list_empty(&rg_db.lutBCAMTableHead[i]))
			{
				PROC_PRINTF("  L2_Hash[%3d] : \n",i);
				list_for_each_entry(plutCamEntry,&rg_db.lutBCAMTableHead[i],lut_list)
				{
					PROC_PRINTF("	 LutCam idx[%d]\n", plutCamEntry->idx);
				}
				PROC_PRINTF("\n");
			}
		}
		PROC_PRINTF(">>lutCam Free list:\n");
		list_for_each_entry(plutCamEntry,&rg_db.lutBCAMFreeListHead,lut_list)
		{
			PROC_PRINTF("LutCam idx[%d]\n", plutCamEntry->idx);
		}
		PROC_PRINTF("\n");
	}
#endif

    return len;
}

int32 dump_lut_bcam_table(struct seq_file *s, void *v)
{
    int i;
	int len=0;
    for(i=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        dump_lut(i,s);
    }
    return len;
}

void _mibdump_lut(int idx,struct seq_file *s)
{
	rtk_l2_addr_table_t	*pdata;


   	pdata=&rg_db.lut[idx].rtk_lut;
	PROC_PRINTF("LUT idx=%d  Group idx=%d\n",idx, rg_db.lut[idx].lutGroupIdx);
    if(pdata->entryType==RTK_LUT_L2UC)
    {
    	//static entry won't show category
    	if((pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0)
    	{
			if(_rtK_rg_checkCategoryPortmask(&pdata->entry.l2UcEntry)==SUCCESS)
    			PROC_PRINTF(", Category<%d>",rg_db.lut[idx].category);

			//display wlan_dev_index, only used when this MAC is wifi host
			if((RTK_RG_ALL_MAC_CPU_PORTMASK & (0x1<<pdata->entry.l2UcEntry.port)) && pdata->entry.l2UcEntry.ext_port!=RTK_RG_MAC_EXT_CPU)
				PROC_PRINTF(", wlan_dev_idx=%d",rg_db.lut[idx].wlan_device_idx);

			//20160813LUKE: display if we are permited for l34 forward
			if(CONFIG_RG_ACCESSWAN_VERSION==2 && rg_db.lut[idx].permit_for_l34_forward)
				PROC_PRINTF(", l34Permited");
		}
		if(rg_db.systemGlobal.fix_l34_to_untag_enable==RG_HWNAT_ENABLE && rg_db.lut[idx].fix_l34_vlan)
			PROC_PRINTF(", remove L34 tag");
		if(rg_db.lut[idx].redirect_http_req){
			if(rg_db.lut[idx].redirect_http_req==1)PROC_PRINTF(", TRAP FIRST HTTP[Def]");	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
			else if(rg_db.lut[idx].redirect_http_req==0xFF)PROC_PRINTF(", DROP HTTP");
			else PROC_PRINTF(", TRAP FIRST HTTP[%d]",rg_db.lut[idx].redirect_http_req-2);	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
		}
#if defined(CONFIG_RG_RTL9600_SERIES)
#else
		if(rg_db.lut[idx].host_idx_valid)PROC_PRINTF(", HostLogging<%d>",rg_db.lut[idx].host_idx);
#endif
		PROC_PRINTF("\n");
        PROC_PRINTF("[P1] mac=%02x:%02x:%02x:%02x:%02x:%02x cvid=%d l3lookup=%d ivl=%d\n"
                       ,pdata->entry.l2UcEntry.mac.octet[0]
                       ,pdata->entry.l2UcEntry.mac.octet[1]
                       ,pdata->entry.l2UcEntry.mac.octet[2]
                       ,pdata->entry.l2UcEntry.mac.octet[3]
                       ,pdata->entry.l2UcEntry.mac.octet[4]
                       ,pdata->entry.l2UcEntry.mac.octet[5]
                       ,pdata->entry.l2UcEntry.vid
                       ,0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL)?1:0);

#if defined(CONFIG_RG_RTL9600_SERIES)
        PROC_PRINTF("efid=%d ", pdata->entry.l2UcEntry.efid);
#else
	PROC_PRINTF("ctagif=%d ", (pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0);
#endif

        PROC_PRINTF("fid=%d spa=%d age=%d auth1x=%d sablock=%d\n"
                       ,pdata->entry.l2UcEntry.fid
                       ,pdata->entry.l2UcEntry.port
                       ,pdata->entry.l2UcEntry.age
                       ,pdata->entry.l2UcEntry.auth
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0);

#if defined(CONFIG_RG_RTL9600_SERIES)
        PROC_PRINTF("dablock=%d ext_spa=%d arp_used=%d sapri_en=%d fwdpri_en=%d lutpri=%d static=%d \n"
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0
                       ,pdata->entry.l2UcEntry.ext_port
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)?1:0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_LOOKUP_PRI)?1:0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0
                       ,pdata->entry.l2UcEntry.priority
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0);
#else
        PROC_PRINTF("dablock=%d ext_spa=%d arp_used=%d static=%d \n"
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0
                       ,pdata->entry.l2UcEntry.ext_port
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)?1:0
                       ,(pdata->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0);
#endif

		PROC_PRINTF("mcStaticRefCnt=%d vxlanStaticRefCnt=%d\n", rg_db.lut[idx].mcStaticRefCnt, rg_db.lut[idx].vxlanStaticRefCnt);

		if(rg_db.lut[idx].dev_name[0])PROC_PRINTF("dev_namt=%s, type=%s \n",rg_db.lut[idx].dev_name,rg_db.lut[idx].conn_type==RG_CONN_WIFI?"WIFI":"MAC_PORT");

    }
    else if(pdata->entryType==RTK_LUT_L2MC)
    {
    	//MC entry won't show category
        PROC_PRINTF("\n[P2] mac=%02x:%02x:%02x:%02x:%02x:%02x ivl=%d vid=%d fid=%d l3lookup=%d ivl=%d\n"
                       ,pdata->entry.l2McEntry.mac.octet[0]
                       ,pdata->entry.l2McEntry.mac.octet[1]
                       ,pdata->entry.l2McEntry.mac.octet[2]
                       ,pdata->entry.l2McEntry.mac.octet[3]
                       ,pdata->entry.l2McEntry.mac.octet[4]
                       ,pdata->entry.l2McEntry.mac.octet[5]
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0
                       ,pdata->entry.l2McEntry.vid
                       ,pdata->entry.l2McEntry.fid
                       ,0
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0);

        PROC_PRINTF("mbr=0x%x extmbr=0x%x\n"
                       ,pdata->entry.l2McEntry.portmask.bits[0]
                       ,pdata->entry.l2McEntry.ext_portmask.bits[0]);

        PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                       ,pdata->entry.l2McEntry.priority
                       ,(pdata->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_FWD_PRI)?1:0);
		PROC_PRINTF("mcStaticRefCnt=%d vxlanStaticRefCnt=%d\n", rg_db.lut[idx].mcStaticRefCnt, rg_db.lut[idx].vxlanStaticRefCnt);

    }
    else if(pdata->entryType==RTK_LUT_L3MC)
    {

#if defined(CONFIG_RG_RTL9602C_SERIES)
		{
			PROC_PRINTF("\n");
			PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
			PROC_PRINTF("sip=%d.%d.%d.%d %s%d\n",(pdata->entry.ipmcEntry.sip>>24)&0xff,(pdata->entry.ipmcEntry.sip>>16)&0xff,(pdata->entry.ipmcEntry.sip>>8)&0xff,(pdata->entry.ipmcEntry.sip)&0xff,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?"IVL VID=":"SVL FID=",
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?pdata->entry.ipmcEntry.vid:pdata->entry.ipmcEntry.fid);
			PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0]);
			PROC_PRINTF("l3_trans_index = %x  sip_index=%d \n",pdata->entry.ipmcEntry.l3_trans_index,pdata->entry.ipmcEntry.sip_index);
			PROC_PRINTF("sip_filter_en=%d static=%d ipv6=%d  wan_sa=%d\n",(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IPV6)?1:0,
				(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0 );

		}

#else
		//MC entry won't show category
		PROC_PRINTF("\n");
        if(!(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY))
        {
            PROC_PRINTF("[P3] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
            PROC_PRINTF("sip=%d.%d.%d.%d vid=%d\n",(pdata->entry.ipmcEntry.sip>>24)&0xff,(pdata->entry.ipmcEntry.sip>>16)&0xff,(pdata->entry.ipmcEntry.sip>>8)&0xff,(pdata->entry.ipmcEntry.sip)&0xff,pdata->entry.ipmcEntry.vid);
            PROC_PRINTF("mbr=0x%x extmbr=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0]);
            PROC_PRINTF("lutpri=%d fwdpri_en=%d\n"
                           ,pdata->entry.ipmcEntry.priority
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0);
        }
        else
        {
            PROC_PRINTF("[P4] gip=%d.%d.%d.%d\n",((pdata->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(pdata->entry.ipmcEntry.dip>>16)&0xff,(pdata->entry.ipmcEntry.dip>>8)&0xff,(pdata->entry.ipmcEntry.dip)&0xff);
            PROC_PRINTF("mbr=0x%x extmbr=0x%x l3trans=0x%x\n",pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0],pdata->entry.ipmcEntry.l3_trans_index);
            PROC_PRINTF("lutpri=%d fwdpri_en=%d dip_only=%d ext_fr=%d wan_sa=%d notsalearn=%d\n"
                           ,pdata->entry.ipmcEntry.priority
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FORCE_EXT_ROUTE)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0
                           ,(pdata->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0
                          );
        }
#endif


    }
#if defined(CONFIG_RG_RTL9602C_SERIES)
	else if (pdata->entryType==RTK_LUT_L3V6MC)
	{
		PROC_PRINTF("[P4] v6ip= ---%02x(dip117_112)%02x(dip111_104)-----%02x(dip43_40)%02x:%02x%02x:%02x%02x \n",
			pdata->entry.ipmcEntry.dip6.ipv6_addr[1]&0x3f,pdata->entry.ipmcEntry.dip6.ipv6_addr[2],pdata->entry.ipmcEntry.dip6.ipv6_addr[10]&0xf,
			pdata->entry.ipmcEntry.dip6.ipv6_addr[11],pdata->entry.ipmcEntry.dip6.ipv6_addr[12],pdata->entry.ipmcEntry.dip6.ipv6_addr[13],
			pdata->entry.ipmcEntry.dip6.ipv6_addr[14],pdata->entry.ipmcEntry.dip6.ipv6_addr[15]);
		PROC_PRINTF("mbr=0x%x  ext_mbr=0x%x	l3mcr_idx=%d \n", pdata->entry.ipmcEntry.portmask.bits[0],pdata->entry.ipmcEntry.ext_portmask.bits[0],pdata->entry.ipmcEntry.l3_mcr_index);
		PROC_PRINTF("notsalearn=%d  l3lookup=%d  ip6=%d  wan_sa=%d\n",
			(pdata->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_STATIC)?1:0,1,
			(pdata->entry.ipmcEntry.flags &RTK_L2_IPMCAST_FLAG_IPV6)?1:0  ,
			(pdata->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0);
	}

#endif
}

void mibdump_lut(int idx,struct seq_file *s)
{
    if(rg_db.lut[idx].valid==1)
    {
#if 1
		dump_lut_display(idx, &rg_db.lut[idx].rtk_lut);
#else
#if defined(CONFIG_RG_RTL9600_SERIES)
		PROC_PRINTF("--------------- LUT TABLE (%d)----------------\n",idx);
#else	//support lut traffic bit
		if(rg_db.lut[idx].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)
			PROC_PRINTF("--------------- LUT TABLE (%d)----------------[Idle %d Secs], [arpRefCount: %d]\n", idx, rg_db.lut[idx].idleSecs, rg_db.lut[idx].arp_refCount);
		else
			PROC_PRINTF("--------------- LUT TABLE (%d)----------------[Idle %d Secs]\n", idx, rg_db.lut[idx].idleSecs);
#endif
		_mibdump_lut(idx,s);
#endif
    }
}
int32 mibdump_lut_table(struct seq_file *s, void *v)
{
    int i;
	rtk_rg_lut_linkList_t *pSoftwareLut;
	int len=0;

    for(i=0; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        mibdump_lut(i,s);
    }

	PROC_PRINTF(">>>>>>Lut Link List:<<<<<<\n");
	for(i=0;i<MAX_LUT_SW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.softwareLutTableHead[i]))
		{
			PROC_PRINTF("  hashIdx=[%3d] : \n",i);
			list_for_each_entry(pSoftwareLut,&rg_db.softwareLutTableHead[i],lut_list)
			{
				PROC_PRINTF("----------------------------------------------\n");
				_mibdump_lut(pSoftwareLut->idx,s);
			}
		}
	}
    return len;
}

int32 mac_ip_table(struct seq_file *s, void *v)
{
    int i;
	int len=0;
	uint32 ip4ZeroAddr=0;
	uint32 ip6ZeroAddr[4]={0};
	uint32 ipv4=0;
	uint32 ipv6=0;

	PROC_PRINTF(">> MAC IP INFO \n");

    for(i=0; i<MAX_LUT_SW_TABLE_SIZE; i++)
    {
		if(!rg_db.lut[i].valid)
			continue;
		if(rg_db.lut[i].rtk_lut.entryType!=RTK_LUT_L2UC)
			continue;

		ipv4=0;
		ipv6=0;
		
		if(ip4ZeroAddr != rg_db.lut[i].lutIP4addr)
			ipv4=1;
		if(memcmp(ip6ZeroAddr,rg_db.lut[i].lutIP6addr,16)!=0)
			ipv6=1;

		if(ipv6 || ipv4)
		{
	        PROC_PRINTF("[%d] MAC[%pM] Port[%d] EXTPORT[%d] WLANIDX[%d] ",i,rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet,rg_db.lut[i].rtk_lut.entry.l2UcEntry.port,rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port,rg_db.lut[i].wlan_device_idx);
			if(ipv4)
				PROC_PRINTF("IPV4[%pI4] ",&rg_db.lut[i].lutIP4addr);
			if(ipv6)
				PROC_PRINTF("IPV6[%pI6] ",rg_db.lut[i].lutIP6addr);
			PROC_PRINTF("\n",i,&rg_db.lut[i].lutIP4addr);
		}
    }

    return len;
}


int32 mibdump_lut_bcam_table(struct seq_file *s, void *v)
{
    int i;
	int len=0;
    for(i=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE; i<MAX_LUT_HW_TABLE_SIZE; i++)
    {
        mibdump_lut(i,s);
    }
    return len;
}

int32 mibdump_softwareLearningCategories(struct seq_file *s, void *v)
{
	int i,j;
	struct print_config
	{
		unsigned int printed:1;
		unsigned int software:1;
	}pconfig={0};
	int len=0;

	PROC_PRINTF(">>access WAN Category statistics:\n");
	for(i=0;i<WanAccessCategoryNum;i++)
	{
		pconfig.printed=0;
		pconfig.software=0;
		for(j=0;j<MAX_LUT_SW_TABLE_SIZE;j++)
		{
			if(rg_db.lut[j].valid && rg_db.lut[j].rtk_lut.entryType==RTK_LUT_L2UC && rg_db.lut[j].category==i &&
				(rg_db.lut[j].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0)
			{
				//Check if the MAC is under LAN portmask
				if(_rtK_rg_checkCategoryPortmask(&rg_db.lut[j].rtk_lut.entry.l2UcEntry)==SUCCESS)
				{
					//This category has at least one entry, print the title!!
					if(!pconfig.printed)
					{
						PROC_PRINTF("===============================================================Category <%d>\n",i);
						pconfig.printed=1;
					}
					if(!pconfig.software && j>=MAX_LUT_HW_TABLE_SIZE)
					{
						PROC_PRINTF(">>>>Software Lut:\n");
						pconfig.software=1;
					}
					mibdump_lut(j,s);
				}
			}
		}
	}

	return len;
}

int32 mibdump_softwareLearningStatistics(struct seq_file *s, void *v)
{
	int i;
	int len=0;


	PROC_PRINTF(">>access WAN Port learning statistics:\n");
	for(i=0;i<RTK_RG_EXT_PORT0;i++)
	{
#ifdef __KERNEL__
		PROC_PRINTF("  port[%d] ",i);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
		{
			PROC_PRINTF("limit is %04d, count is %04d, limitReachAction is %s\n",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i],
				atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]),
				rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
		}
		else
		{
			PROC_PRINTF("Unlimited, count is %04d\n",atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]));
		}
#else
		PROC_PRINTF("  port[%d] ",i);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
 			PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);
		else
			PROC_PRINTF("Unlimited");
		PROC_PRINTF(", limitReachAction is %s\n",
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#endif
	}
	//Extension port
	for(i=RTK_RG_EXT_PORT0;i<RTK_RG_PORT_MAX;i++)
	{
		if(RG_INVALID_PORT(i)) continue;
#ifdef __KERNEL__
		PROC_PRINTF("  EXTport[%d] ",i-RTK_RG_EXT_PORT0);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0){
			PROC_PRINTF("limit is %04d, count is %04d, limitReachAction is %s\n",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i],
				atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]),
				rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
		}else{
			PROC_PRINTF("Unlimited, count is %04d\n",atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[i]));
		}
#else
		PROC_PRINTF("  EXTport[%d] ",i-RTK_RG_EXT_PORT0);
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>=0)
			PROC_PRINTF("limit is %04d",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]);
		else
			PROC_PRINTF("Unlimited");
		PROC_PRINTF(", limitReachAction is %s\n",
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
			rg_db.systemGlobal.sourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
#endif
	}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Wlan0 device
	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("  Wlan0Dev[%d] ",i);
			if(rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[i]>=0){
				PROC_PRINTF("limit is %04d, count is %04d, limitReachAction is %s\n",rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[i],
					atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[i]),
					rg_db.systemGlobal.wlan0SourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
					rg_db.systemGlobal.wlan0SourceAddrLearningAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
			}else{
				PROC_PRINTF("Unlimited, count is %04d\n",atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[i]));
			}
		}
	}
#endif

	return len;
}
//reverse port mask translator
int _rtk_rg_portmask_re_translator(rtk_rg_portmask_t *out_pmask, rtk_portmask_t in_mac_pmask, rtk_portmask_t in_ext_pmask){
	int i;
	int EXT_CPU_PORT_flag = DISABLED;

	if(out_pmask==NULL)
		RETURN_ERR(RT_ERR_RG_NULL_POINTER);

	bzero(out_pmask,sizeof(rtk_portmask_t));
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++){
		if(RG_INVALID_MAC_PORT(i)) continue;
		if(in_mac_pmask.bits[0] & (1<<i))
		{
			out_pmask->portmask |= (1<<i);
			if(i==6) EXT_CPU_PORT_flag = ENABLED;	//CPU port is enable
		}
	}
	//set ext portmask
	for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
		if(in_ext_pmask.bits[0] & (1<<i)){
			out_pmask->portmask |= (1<<(i+RTK_RG_EXT_BASED_PORT));
			EXT_CPU_PORT_flag = ENABLED;//ENABLE MAC_CPU_PORT if any EXT_PORT is ENABLED
		}
	}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
	if(EXT_CPU_PORT_flag==ENABLED)
		out_pmask->portmask |= (1<<RTK_RG_MAC_PORT_CPU);	//check cpu port for all cases if CPU port is enable
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	/*Note!! For FB: bit'0 stands for ext0 not cpu port*/
#else
#error
#endif
	return (RT_ERR_RG_OK);

}

//rg command line should be close to the line-start.
static char tmpStr[255];
static char buf[64];
static char buf_2[64];
int32 diag_shell_dump(struct seq_file *s, void *v)
{
	int i,j;
	int len=0;
	rtk_enable_t state = ENABLED;

	PROC_PRINTF("#dump init\n");
	if((rg_db.systemGlobal.initParam.initByHwCallBack==_rtk_rg_initParameterSetByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.arpAddByHwCallBack==_rtk_rg_arpAddByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.arpDelByHwCallBack==_rtk_rg_arpDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.macAddByHwCallBack==_rtk_rg_macAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.macDelByHwCallBack==_rtk_rg_macDelByHwCallBack)
//5
		&& (rg_db.systemGlobal.initParam.routingAddByHwCallBack==_rtk_rg_routingAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.routingDelByHwCallBack==_rtk_rg_routingDelByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.naptAddByHwCallBack==_rtk_rg_naptAddByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.naptDelByHwCallBack==_rtk_rg_naptDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.bindingAddByHwCallBack==_rtk_rg_bindingAddByHwCallBack)
//10
		&& (rg_db.systemGlobal.initParam.bindingDelByHwCallBack==_rtk_rg_bindingDelByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceAddByHwCallBack==_rtk_rg_interfaceAddByHwCallBack)
		&& (rg_db.systemGlobal.initParam.interfaceDelByHwCallBack==_rtk_rg_interfaceDelByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.neighborAddByHwCallBack==_rtk_rg_neighborAddByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.neighborDelByHwCallBack==_rtk_rg_neighborDelByHwCallBack)
//15
		//&& (rg_db.systemGlobal.initParam.v6RoutingAddByHwCallBack==_rtk_rg_v6RoutingAddByHwCallBack)
		//&& (rg_db.systemGlobal.initParam.v6RoutingDelByHwCallBack==_rtk_rg_v6RoutingDelByHwCallBack)
		//rg_db.systemGlobal.initParam.naptInboundConnLookupFirstCallBack register at init if needed
		//rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBackregister at init if needed
		//rg_db.systemGlobal.initParam.naptInboundConnLookupThirdCallBackregister at init if needed
//20
		&& (rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack==_rtk_rg_dhcpRequestByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack==_rtk_rg_pppoeBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pptpBeforeDialByHwCallBack==_rtk_rg_pptpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.l2tpBeforeDialByHwCallBack==_rtk_rg_l2tpBeforeDialByHwCallBack)
		&& (rg_db.systemGlobal.initParam.pppoeDsliteBeforeDialByHwCallBack==_rtk_rg_pppoeDsliteBeforeDialByHwCallBack)
//25
		//rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupFirstCallBackregister at init if needed
		//rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupSecondCallBackat init if needed
		//rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupThirdCallBackat init if needed
		//&& (rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack==_rtk_rg_softwareNaptInfoAddCallBack)
		//&& (rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack==_rtk_rg_softwareNaptInfoDeleteCallBack)
//30
		&& ((rg_db.systemGlobal.initParam.naptPreRouteDPICallBack==_rtk_rg_naptPreRouteDPICallBack)||(rg_db.systemGlobal.initParam.naptPreRouteDPICallBack==NULL))
		&& ((rg_db.systemGlobal.initParam.naptForwardDPICallBack==_rtk_rg_naptForwardDPICallBack)||(rg_db.systemGlobal.initParam.naptForwardDPICallBack==NULL))
		&& (rg_db.systemGlobal.initParam.pppoeLCPStateCallBack==_rtk_rg_pppoeLCPStateCallBack)
		)
	{
		PROC_PRINTF("rg init callback default igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d hybridMode %d ivlMulticastSupport %d",
		rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
		rg_db.systemGlobal.initParam.hybridMode,
		rg_db.systemGlobal.initParam.ivlMulticastSupport);
	}else{
		PROC_PRINTF("rg init callback igmpSnoopingEnable %d macBasedTagDecision %d wanPortGponMode %d hybridMode %d ivlMulticastSupport %d",
		rg_db.systemGlobal.initParam.igmpSnoopingEnable,rg_db.systemGlobal.initParam.macBasedTagDecision,rg_db.systemGlobal.initParam.wanPortGponMode,
		rg_db.systemGlobal.initParam.hybridMode,
		rg_db.systemGlobal.initParam.ivlMulticastSupport);
	}
#if defined(CONFIG_RG_RTL9600_SERIES)
	if(rg_db.systemGlobal.initParam.fwdVLAN_CPU!=DEFAULT_CPU_VLAN ||
		rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block!=DEFAULT_PROTO_BLOCK_VLAN ||
		rg_db.systemGlobal.initParam.fwdVLAN_BIND_INTERNET!=DEFAULT_BIND_INTERNET ||
		rg_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER!=DEFAULT_BIND_OTHER)
		PROC_PRINTF(" fwdVlan_CPU %d fwdVlan_Proto_Block %d fwdVLAN_BIND_INTERNET %d fwdVLAN_BIND_OTHER %d\n\n",
		rg_db.systemGlobal.initParam.fwdVLAN_CPU,rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block,rg_db.systemGlobal.initParam.fwdVLAN_BIND_INTERNET,rg_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER);
	else PROC_PRINTF("\n\n");
#else
	if(rg_db.systemGlobal.initParam.fwdVLAN_CPU!=DEFAULT_CPU_VLAN ||
		rg_db.systemGlobal.initParam.fwdVLAN_CPU_SVLAN!=DEFAULT_CPU_SVLAN ||
		rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block!=DEFAULT_PROTO_BLOCK_VLAN ||
		rg_db.systemGlobal.initParam.fwdVLAN_BIND_INTERNET!=DEFAULT_BIND_INTERNET ||
		rg_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER!=DEFAULT_BIND_OTHER)
		PROC_PRINTF(" fwdVlan_CPU %d fwdVlan_CPU_SVLAN %d fwdVlan_Proto_Block %d fwdVLAN_BIND_INTERNET %d fwdVLAN_BIND_OTHER %d\n\n",
		rg_db.systemGlobal.initParam.fwdVLAN_CPU,rg_db.systemGlobal.initParam.fwdVLAN_CPU_SVLAN,rg_db.systemGlobal.initParam.fwdVLAN_Proto_Block,rg_db.systemGlobal.initParam.fwdVLAN_BIND_INTERNET,rg_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER);
	else PROC_PRINTF("\n\n");
#endif
	PROC_PRINTF("#wan/lan setting\n");
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
	{
		if(rg_db.systemGlobal.interfaceInfo[i].valid )
		{
			rtk_ipv6_addr_t zeroV6Addr={{0}};
			if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan == 0)	//LAN
			{
				if(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr!=0L ||
					memcmp(&rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr,&zeroV6Addr,sizeof(rtk_ipv6_addr_t)))	//if no ip, skip it
				{
					memset(tmpStr,0,255);
					if(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->vlan_based_pri_enable)
						sprintf(tmpStr,"enable vlan-based-pri %d",rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->vlan_based_pri);
					else
						sprintf(tmpStr,"disable");
#if defined(CONFIG_RG_RTL9600_SERIES)
PROC_PRINTF("rg set lan-intf ip-version %d gateway-mac %02x:%02x:%02x:%02x:%02x:%02x ip-addr %d.%d.%d.%d ip-mask %d.%d.%d.%d ipv6-addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_network_mask_length %d port-mask 0x%x untag-mask 0x%x intf-vlan_id %d vlan-based-pri-enable %s mtu %d isIVL %d add-sw-arp %d\n\n",
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_version,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[5],
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[0]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[1]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[2]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[3]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[4]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[5]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[6]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[7]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[8]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[9]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[10]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[11]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[12]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[13]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[14]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[15]),
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_network_mask_length,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->port_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->untag_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->intf_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->mtu,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->isIVL,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->add_sw_arp);

#else	// not CONFIG_RG_RTL9600_SERIES
PROC_PRINTF("rg set lan-intf ip-version %d gateway-mac %02x:%02x:%02x:%02x:%02x:%02x ip-addr %d.%d.%d.%d ip-mask %d.%d.%d.%d ipv6-addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_network_mask_length %d port-mask 0x%x untag-mask 0x%x intf-vlan_id %d vlan-based-pri-enable %s mtu %d isIVL %d\n\n",
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_version,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->gmac.octet[5],
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_addr & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff000000)>>24,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff0000)>>16,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff00)>>8,
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ip_network_mask & 0xff),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[0]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[1]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[2]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[3]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[4]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[5]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[6]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[7]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[8]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[9]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[10]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[11]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[12]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[13]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[14]),
				(rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_addr.ipv6_addr[15]),
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->ipv6_network_mask_length,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->port_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->untag_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->intf_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->mtu,
				rg_db.systemGlobal.interfaceInfo[i].p_lanIntfConf->isIVL);
#endif
PROC_PRINTF("rg add lan-intf entry\n\n");
				}
			}
			else	//WAN
			{
				memset(tmpStr,0,255);
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri_enable)
					sprintf(tmpStr,"enable vlan-based-pri %d",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri);
				else
					sprintf(tmpStr,"disable");
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.bridgeToBindingWanByProtocol)
PROC_PRINTF("rg set wan-intf wan-type %d gateway-mac  %02x:%02x:%02x:%02x:%02x:%02x wan-port %d port-binding-mask 0x%x egress-vlan-tag-on %d egress-vlan-id %d vlan-based-pri-enable %s isIVL %d %s wlan0-binding-mask 0x%x bridgeToBindingWanByProtocol %d\n",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD?RTK_RG_EXT_PORT2:
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD?RTK_RG_EXT_PORT3:rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.isIVL,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.none_internet ? "none_internet":"",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wlan0_dev_binding_mask,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.bridgeToBindingWanByProtocol);
				else
PROC_PRINTF("rg set wan-intf wan-type %d gateway-mac  %02x:%02x:%02x:%02x:%02x:%02x wan-port %d port-binding-mask 0x%x egress-vlan-tag-on %d egress-vlan-id %d vlan-based-pri-enable %s isIVL %d %s wlan0-binding-mask 0x%x\n",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5],
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD?RTK_RG_EXT_PORT2:
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD?RTK_RG_EXT_PORT3:rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id,
				tmpStr,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.isIVL,
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.none_internet ? "none_internet":"",
				rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wlan0_dev_binding_mask);
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.forbiddenUnbindIGMP)
PROC_PRINTF("rg set wan-intf forbiddenUnbindIGMP\n");
				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.forbiddenUnbindMLD)
PROC_PRINTF("rg set wan-intf forbiddenUnbindMLD\n");
PROC_PRINTF("\nrg add wan-intf entry\n\n");

				if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr!=0L ||
						memcmp(&rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr,&zeroV6Addr,sizeof(rtk_ipv6_addr_t))){
						setFlag = 1;
						snprintf(buf,64," ");
						snprintf(buf_2,64," ");
						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp)
							snprintf(buf_2,64," routing-type %d ", rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp);

PROC_PRINTF("rg set dhcpClientInfo stauts %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d%sip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d%sgw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.stauts,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_addr & 0xff),
					buf,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.mtu,
					buf_2,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv4.octet[5]);				

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
PROC_PRINTF("rg set dhcpClientInfo-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x nptv6_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nptv6_ipv6_mask_length %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_napt_enable,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.gateway_mac_addr_for_ipv6.octet[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dhcp_client_info.hw_info.nptv6_ipv6_mask_length);
						}
					}

					if(setFlag==1)
PROC_PRINTF("rg add dhcpClientInfo wan_intf_idx %d\n\n",i);

				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC)
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr!=0L ||
						memcmp(&rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr,&zeroV6Addr,sizeof(rtk_ipv6_addr_t))){
						setFlag = 1;
						snprintf(buf,64," ");
						snprintf(buf_2,64," ");
						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp)
							snprintf(buf_2,64," routing-type %d ", rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp);

PROC_PRINTF("rg set wan-intf-static-info ip-version %d napt_enable %d ip_addr %d.%d.%d.%d%sip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d%sgw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff),
					buf,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					buf_2,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[5]);

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
PROC_PRINTF("rg set wan-intf-static-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x nptv6_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nptv6_ipv6_mask_length %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[5],
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_mask_length);
						}
					}

					if(setFlag==1)
PROC_PRINTF("rg add wan-intf-static-info intf-index %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)	//pppoe
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.sessionId != 0L){
						setFlag = 1;
						snprintf(buf,64," ");
						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->remote_host_ip_addr!=0)
							snprintf(buf,64," remote_host_ip_addr %d.%d.%d.%d ",
							(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->remote_host_ip_addr & 0xff000000)>>24,
							(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->remote_host_ip_addr & 0xff0000)>>16,
							(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->remote_host_ip_addr & 0xff00)>>8,
							(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->remote_host_ip_addr & 0xff));

PROC_PRINTF("rg set pppoeClientInfoAfterDial sessionId %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d%sip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.sessionId,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_addr & 0xff),
					buf,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.mtu,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[5]);

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
PROC_PRINTF("rg set pppoeClientInfoAfterDial-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x nptv6_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nptv6_ipv6_mask_length %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_napt_enable,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.gateway_mac_addr_for_ipv6.octet[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_info.after_dial.hw_info.nptv6_ipv6_mask_length);
						}
					}

					if(setFlag == 1)
PROC_PRINTF("rg add pppoeClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)	//pptp
				{
#if 1
					PROC_PRINTF("rg set pptpClientInfoBeforeDial username %s password %s pptp_server_addr_for_ipv4 %d.%d.%d.%d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.username,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.password,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr & 0xff));
					PROC_PRINTF("rg add pptpClientInfoBeforeDial wan_intf_idx %d\n\n",i);
#endif
					PROC_PRINTF("rg set pptpClientInfoAfterDial callId %d gateway_callId %d ipv4_addr %d.%d.%d.%d ipv4_network_mask %d.%d.%d.%d gateway_ipv4_addr %d.%d.%d.%d mtu %d ipv4_napt_enable %d ipv4_default_gateway_on %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %02x:%02x:%02x:%02x:%02x:%02x\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.callId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.gateway_callId,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_addr  & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask& 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ip_network_mask  & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.mtu,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.napt_enable,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.ipv4_default_gateway_on,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv4,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[0],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[1],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[2],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[3],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[4],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pptp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[5]);
					PROC_PRINTF("rg add pptpClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP)	//l2tp
				{
#if 1
					PROC_PRINTF("rg set l2tpClientInfoBeforeDial username %s password %s l2tp_server_addr_for_ipv4 %d.%d.%d.%d\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.username,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.password,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.before_dial.l2tp_ipv4_addr & 0xff));
					PROC_PRINTF("rg add l2tpClientInfoBeforeDial wan_intf_idx %d\n\n",i);
#endif
					PROC_PRINTF("rg set l2tpClientInfoAfterDial outer_port %d gateway_outer_port %d tunnelId %d sessionId %d gateway_tunnelId %d gateway_sessionId %d ipv4_addr %d.%d.%d.%d ipv4_network_mask %d.%d.%d.%d gateway_ipv4_addr %d.%d.%d.%d mtu %d ipv4_napt_enable %d ipv4_default_gateway_on %d gw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %02x:%02x:%02x:%02x:%02x:%02x\n\n",
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.outer_port,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_outer_port,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.tunnelId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.sessionId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_tunnelId,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.gateway_sessionId,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_addr & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask& 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ip_network_mask & 0xff),
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff000000)>>24,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff0000)>>16,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff00)>>8,
						(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_ipv4_addr & 0xff),
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.mtu,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.napt_enable,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.ipv4_default_gateway_on,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gw_mac_auto_learn_for_ipv4,
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[0],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[1],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[2],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[3],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[4],
						rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.l2tp_info.after_dial.hw_info.gateway_mac_addr_for_ipv4.octet[5]);
					PROC_PRINTF("rg add l2tpClientInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE)	//dslite
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr != 0L ||
						memcmp(&rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr,&zeroV6Addr,sizeof(rtk_ipv6_addr_t))){
						setFlag = 1;
PROC_PRINTF("rg set wan-intf-dslite-info ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d mtu %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv4_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.mtu);

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
#if defined(CONFIG_RG_RTL9600_SERIES)
PROC_PRINTF("rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x\n\n",
#else

						if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS)
							snprintf(buf,64,"copy_from_IPv4_TOS");
						else
							snprintf(buf,64,"0x%x",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.tc);
PROC_PRINTF("rg set wan-intf-dslite-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x dslite_hoplimit %d dslite_flowlabel 0x%x dslite_tc %s\n\n",
#endif
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.ipv6_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.mtu,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.static_info.gateway_mac_addr_for_ipv6.octet[5],
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr[15]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_auto_learn,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.aftr_mac_addr.octet[5]
#if defined(CONFIG_RG_RTL9600_SERIES)
					);
#else
					,rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.hopLimit,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.dslite_info.rtk_dslite.flowLabel,
					buf);
#endif
						}
					}

					if(setFlag==1)
PROC_PRINTF("rg add wan-intf-dslite-info intf-index %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)	//pppoe dslite
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.sessionId != 0L){
						setFlag = 1;
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial sessionId %d ip-version %d napt_enable %d ip_addr %d.%d.%d.%d ip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d mtu %d \n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.sessionId,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_version,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_addr & 0xff),
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv4_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.mtu);

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
#if defined(CONFIG_RG_RTL9600_SERIES)
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x\n\n",
#else
						if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_COPY_FROM_TOS)
							snprintf(buf,64,"copy_from_IPv4_TOS");
						else
							snprintf(buf,64,"0x%x",rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tc);
PROC_PRINTF("rg set pppoeDsliteInfoAfterDial-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x b4_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x aftr_mac_auto_learn %d aftr_mac_addr %x:%x:%x:%x:%x:%x dslite_hoplimit %d dslite_flowlabel 0x%x dslite_tc %s\n\n",
#endif
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_napt_enable,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.ipv6_default_gateway_on,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[6],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[7],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[8],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[9],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[10],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[11],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[12],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[13],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[14],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr[15],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_auto_learn,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.aftr_mac_addr.octet[5]
#if defined(CONFIG_RG_RTL9600_SERIES)
					);
#else
					,rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.hopLimit,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.flowLabel,
					buf);
#endif
						}
					}

					if(setFlag == 1)
						PROC_PRINTF("rg add pppoeDsliteInfoAfterDial wan_intf_idx %d\n\n",i);
				}
				else if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_VXLAN)	//vxlan
				{
					uint8 setFlag = 0;
					if(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_addr!=0L){
						setFlag = 1;
PROC_PRINTF("rg set wan-intf-vxlan-info vxlan_mode %d vxlan_remote_ipv4_addr %d.%d.%d.%d vxlan_remote_ipv4_mac %x:%x:%x:%x:%x:%x vxlan_outer_udp_sport %d vxlan_outer_udp_dport %d vxlan_VNI 0x%x vxlan_baseIntf_idx %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_mode,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_remote_ipv4_gatewayMac.octet[5],
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.outer_udp_sport,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.outer_udp_dport,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_network_identifier,
					rg_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.vxlan_info.after_dial.vxlan_baseIntf_idx);
					}
					if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr!=0L ||
						memcmp(&rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr,&zeroV6Addr,sizeof(rtk_ipv6_addr_t))){
						setFlag = 1;
						snprintf(buf,64," ");
						snprintf(buf_2,64," ");
						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp)
							snprintf(buf_2,64," routing-type %d ", rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->static_route_with_arp);
PROC_PRINTF("rg set wan-intf-vxlan-info ip-version %d napt_enable %d ip_addr %d.%d.%d.%d%sip_network_mask %d.%d.%d.%d ipv4_default_gateway_on %d gateway_ipv4_addr %d.%d.%d.%d mtu %d%sgw_mac_auto_learn_for_ipv4 %d gateway_mac_addr_for_ipv4 %x:%x:%x:%x:%x:%x\n\n",
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_addr & 0xff),
					buf,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_network_mask & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv4_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff000000)>>24,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff0000)>>16,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff00)>>8,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv4_addr & 0xff),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					buf_2,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv4,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv4.octet[5]);

						if(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ip_version!=IPVER_V4ONLY){
PROC_PRINTF("rg set wan-intf-static-info-ipv6 ipv6_napt_enable %d ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ipv6_mask_length %d ipv6_default_gateway_on %d gateway_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mtu %d gw_mac_auto_learn_for_ipv6 %d gateway_mac_addr_for_ipv6 %x:%x:%x:%x:%x:%x nptv6_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nptv6_ipv6_mask_length %d\n\n",
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_napt_enable,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_mask_length,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->ipv6_default_gateway_on,
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->mtu,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gw_mac_auto_learn_for_ipv6,
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[0],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[1],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[2],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[3],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[4],
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->gateway_mac_addr_for_ipv6.octet[5],
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[0]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[1]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[2]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[3]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[4]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[5]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[6]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[7]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[8]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[9]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[10]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[11]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[12]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[13]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[14]),
					(rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_addr.ipv6_addr[15]),
					rg_db.systemGlobal.interfaceInfo[i].p_wanStaticInfo->nptv6_ipv6_mask_length);
						}
					}

					if(setFlag==1)
PROC_PRINTF("rg add wan-intf-vxlan-info intf-index %d\n\n",i);
				}
			}
		}
	}
	PROC_PRINTF("#add virtual server entry\n");
	for(i=0;i<rg_db.systemGlobal.virtualServerTotalNum;i++){
	//if(rg_db.systemGlobal.virtualServerGroup[i].local_ip!=0)
		{
			PROC_PRINTF("rg set virtualServer is_tcp %d wan_intf_idx %d gateway_port_start %d local_ip %d.%d.%d.%d remote_ip %d.%d.%d.%d local_port_start %d mappingPortRangeCnt %d mappingType %d valid %d hookAlgType 0x%x disable_wan_check %d enable_limit_remote_src_port %d remote_src_port_start %d remote_src_port_end %d\n\n",
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->is_tcp,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->wan_intf_idx,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->gateway_port_start,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff000000)>>24,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff0000)>>16,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff00)>>8,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_ip  & 0xff),
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_ip& 0xff000000)>>24,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_ip& 0xff0000)>>16,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_ip& 0xff00)>>8,
					(rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_ip& 0xff),
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->local_port_start,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->mappingPortRangeCnt,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->mappingType,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->valid,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->hookAlgType,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->disable_wan_check,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->enable_limit_remote_src_port,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_src_port_start,
					rg_db.systemGlobal.virtualServerGroup[i].p_virtualServer->remote_src_port_end
					);
			PROC_PRINTF("rg add virtualServer entry\n\n");
		}
	}
	PROC_PRINTF("#add port trigger entry\n");
	for(i=0;i<4;i++){
		if(!list_empty(rg_db.systemGlobal.pPortTrigger_matched_head[i])){
			rtk_rg_portTrigger_list_t *pTriggerEntry;
			list_for_each_entry(pTriggerEntry, rg_db.systemGlobal.pPortTrigger_matched_head[i], trigger_list){
				PROC_PRINTF("rg add portTrigger is_outgoing %d match_is_tcp %d match_port_start %d match_port_end %d relate_is_tcp %d relate_port_start %d relate_port_end %d\n",
							i>>1,
							i&0x1,
							pTriggerEntry->match_port_start,
							pTriggerEntry->match_port_end,
							pTriggerEntry->relate_is_tcp,
							pTriggerEntry->relate_port_start,
							pTriggerEntry->relate_port_end);
			}
		}
	}
	
	PROC_PRINTF("\n#set perPortBase Limit to WAN\n");
	for(i=0;i<RTK_RG_PORT_MAX;i++)
	{
		if(RG_INVALID_PORT(i)) continue;
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[i]>0){
			PROC_PRINTF("rg set softwareSourceAddrLearningLimit learningLimitNumber %d action %d\n",rg_db.systemGlobal.sourceAddrLearningLimitNumber[i],rg_db.systemGlobal.sourceAddrLearningAction[i]);
			PROC_PRINTF("rg add softwareSourceAddrLearningLimit port_idx %d\n",i);
		}
	}
	PROC_PRINTF("\n#set portMask Limit to WAN\n");
	if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask>0){
		//wlan_idx 0 is reserved for slave/master; only 0 could be used now
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member>0)
		{
			PROC_PRINTF("rg add wanAccessLimitPortMask port_mask 0x%x wlan_idx 0 wlan_dev_mask 0x%x learningLimitNumber %d action %d\n",rg_db.systemGlobal.accessWanLimitPortMask_member.portmask,
				rg_db.systemGlobal.accessWanLimitPortMask_wlan0member,
				rg_db.systemGlobal.accessWanLimitPortMask,
				rg_db.systemGlobal.accessWanLimitPortMaskAction);
		}
		else
#endif
		{
			PROC_PRINTF("rg add wanAccessLimitPortMask port_mask 0x%x learningLimitNumber %d action %d\n",
				rg_db.systemGlobal.accessWanLimitPortMask_member.portmask,
				rg_db.systemGlobal.accessWanLimitPortMask,
				rg_db.systemGlobal.accessWanLimitPortMaskAction);
		}
	}

	PROC_PRINTF("\n#set categoryBase Limit to WAN\n");
	for(i=0;i<WanAccessCategoryNum;i++){
		if(rg_db.systemGlobal.accessWanLimitCategory[i]>=0){
			PROC_PRINTF("rg add wanAccessLimitCategory category %d learningLimitNumber %d action %d\n",i,
			rg_db.systemGlobal.accessWanLimitCategory[i],
			rg_db.systemGlobal.accessWanLimitCategoryAction[i]);
		}
	}

	PROC_PRINTF("\n#set group Mac Limit\n");
	if(rg_db.systemGlobal.groupMACLimit>=0){
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		if(rg_db.systemGlobal.groupMACLimit_wlan0member){
			PROC_PRINTF("rg add groupMacLimit port_mask 0x%x wlan_idx 0 wlan_dev_mask 0x%x learningLimitNumber %d\n",
				rg_db.systemGlobal.groupMACLimit_member.portmask,
				rg_db.systemGlobal.groupMACLimit_wlan0member,
				rg_db.systemGlobal.groupMACLimit);
		}
		else
#endif
		{
			PROC_PRINTF("rg add groupMacLimit port_mask 0x%x learningLimitNumber %d\n",
				rg_db.systemGlobal.groupMACLimit_member.portmask,
				rg_db.systemGlobal.groupMACLimit);
		}
	}

	PROC_PRINTF("\n#set vlan group Mac Limit\n");
	for(i=0;i<MAX_VLAN_GROUP_MAC_LIMIT_NUMBER;i++)
	{
		if(rg_db.systemGlobal.vlanGroupMACLimit_group[i].group_info.valid)
		{
			PROC_PRINTF("rg add vlanGroupMacLimit port %d learningLimitNumber %d\n",rg_db.systemGlobal.vlanGroupMACLimit_group[i].group_info.port,rg_db.systemGlobal.vlanGroupMACLimit_group[i].group_info.mac_limit_number);
			if(rg_db.systemGlobal.vlanGroupMACLimit_group[i].group_info.untag)PROC_PRINTF("rg set vlanGroupMacLimit groupIndex %d vlan untag\n",i);
			for(j=0;j<MAX_VLAN_HW_TABLE_SIZE;j++)
			{
				if(rg_test_bit(j&0x1f,(void *)&(rg_db.systemGlobal.vlanGroupMACLimit_group[i].group_info.vlanMask[j>>5])))
				{
					PROC_PRINTF("rg set vlanGroupMacLimit groupIndex %d vlan %d\n",i,j);
				}
			}
		}
	}

	PROC_PRINTF("\n#set dmzHost\n");
	for(i=0;i<MAX_DMZ_TABLE_SIZE;i++){		//MAX_DMZ_TABLE_SIZE is defined as MAX_NETIF_SW_TABLE_SIZE
		if(rg_db.dmzInfo[i].enabled==1)
		{
			PROC_PRINTF("rg set dmzHost enabled 1 ipversion %d private_ip %d.%d.%d.%d private_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.dmzInfo[i].ipversion,
					(rg_db.dmzInfo[i].private_ip & 0xff000000)>>24,
					(rg_db.dmzInfo[i].private_ip & 0xff0000)>>16,
					(rg_db.dmzInfo[i].private_ip & 0xff00)>>8,
					(rg_db.dmzInfo[i].private_ip & 0xff),
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[0],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[1],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[2],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[3],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[4],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[5],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[6],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[7],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[8],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[9],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[10],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[11],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[12],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[13],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[14],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[15]);
			PROC_PRINTF("rg add dmzHost wan_intf_idx %d\n\n", i);
		}
	}
	PROC_PRINTF("#add cvlan\n");
	for(i=0;i<MAX_VLAN_SW_TABLE_SIZE;i++)
	{
		if(rg_db.vlan[i].valid == 1 && rg_db.vlan[i].addedAsCustomerVLAN == 1)
		{
			rtk_rg_portmask_t tmpmask;
			_rtk_rg_portmask_re_translator(&tmpmask,rg_db.vlan[i].MemberPortmask,rg_db.vlan[i].Ext_portmask);
			if(rg_db.vlan[i].priorityEn == 1)
			{
				PROC_PRINTF("rg set cvlan vlanId %d isIVL %d memberPortMask 0x%x untagPortMask 0x%x vlan-based-pri-enable enable vlan-based-pri %d\n\n",
						i,(rg_db.vlan[i].fidMode == VLAN_FID_IVL)? 1:0,
						tmpmask.portmask, rg_db.vlan[i].UntagPortmask.bits[0], rg_db.vlan[i].priority);
			}
			else
			{
				PROC_PRINTF("rg set cvlan vlanId %d isIVL %d memberPortMask 0x%x untagPortMask 0x%x vlan-based-pri-enable disable\n\n",
						i,(rg_db.vlan[i].fidMode == VLAN_FID_IVL)? 1:0,
						tmpmask.portmask, rg_db.vlan[i].UntagPortmask.bits[0]);
			}

			PROC_PRINTF("rg add cvlan entry\n");
		}
	}
	PROC_PRINTF("#add pvid\n");
	for(i=0;i<RTK_RG_PORT_MAX;i++)
	{
		if(RG_INVALID_PORT(i)) continue;
		if(rg_db.systemGlobal.portBasedVID[i] !=0)
			PROC_PRINTF("rg set port-based-vlan port_idx %d vlanId %d\n\n",
			i,rg_db.systemGlobal.portBasedVID[i]
			);
	}
	PROC_PRINTF("#add vlan binding\n");
	for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
	{
		if(rg_db.bind[i].rtk_bind.vidLan==0L) continue;
		if(rg_db.bind[i].valid)
		{
			int k,display=1;
			if(rg_db.bind[i].rtk_bind.portMask.bits[0])
			{
#if defined(CONFIG_RG_RTL9607C_SERIES)
				for(j=0;j<RTK_RG_PORT_MASTERCPU_CORE1;j++)
#else
				for(j=0;j<RTK_RG_PORT_CPU;j++)
#endif
				{
					if(RG_INVALID_PORT(j)) continue;
					if((rg_db.bind[i].rtk_bind.portMask.bits[0] & (1<<j))!=0) break;
				}
			}else{
				for(j=RTK_RG_EXT_PORT0;j<RTK_RG_PORT_MAX;j++)
				{
					if(RG_INVALID_PORT(j)) continue;
					if((rg_db.bind[i].rtk_bind.extPortMask.bits[0] & (1<<(j-RTK_RG_EXT_PORT0)))!=0) break;
				}
			}
			//20190315LUKE: prevent same rule be printed double time.
			for(k=0;k<i;k++)
			{
				if(rg_db.bind[k].valid && rg_db.bind[k].rtk_bind.vidLan==rg_db.bind[i].rtk_bind.vidLan &&
					!memcmp(&rg_db.bind[i].rtk_bind.portMask,&rg_db.bind[k].rtk_bind.portMask,sizeof(rtk_portmask_t)) &&
					!memcmp(&rg_db.bind[i].rtk_bind.extPortMask,&rg_db.bind[k].rtk_bind.extPortMask,sizeof(rtk_portmask_t)))
				{
					display=0;
					break;
				}
			}
			if(display)
			{
				PROC_PRINTF("rg set binding port_idx %d ingress_vid %d wan_intf_idx %d\n\n",j,rg_db.bind[i].rtk_bind.vidLan,rg_db.nexthop[rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.nhIdx].rtk_nexthop.ifIdx);
				PROC_PRINTF("rg add binding entry\n\n");
			}
		}
	}
	PROC_PRINTF("#add alg\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress !=0L)
		{
			PROC_PRINTF("rg set serverInLanAppsIpAddr algType 0x%x serverAddress %d.%d.%d.%d\n\n",
					rg_db.algServInLanIpMapping[i].algType,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff000000)>>24,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff0000)>>16,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff00)>>8,
					(rg_db.algServInLanIpMapping[i].serverAddress & 0xff)
		);
			PROC_PRINTF("rg add serverInLanAppsIpAddr entry\n\n");
		}
	}
	if(rg_db.algFunctionMask!=0)
	{
		PROC_PRINTF("rg add algApps app_mask 0x%x\n\n",rg_db.algFunctionMask);
	}

	PROC_PRINTF("#add url filter\n");
	PROC_PRINTF("rg clear url-filter entry\n\n");
	for(i=0;i<MAX_URL_FILTER_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.urlFilter_table_entry[i].valid)
		{
			PROC_PRINTF("rg set url-filter url %s path %s path-exactly-match %d wan-interface %d\n\n",
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.url_filter_string,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.path_filter_string,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.path_exactly_match,
			rg_db.systemGlobal.urlFilter_table_entry[i].urlFilter.wan_intf);
		PROC_PRINTF("rg add url-filter\n\n");
		}
	}
	PROC_PRINTF("#add httpRedirectAll\n");
	if(rg_db.redirectHttpAll.enable!=0){
		PROC_PRINTF("rg set redirectHttpAll enable %d count %d pushweb %s\n",rg_db.redirectHttpAll.enable,rg_db.redirectHttpAll.count,rg_db.redirectHttpAll.pushweb);
	}
	PROC_PRINTF("#add httpRedirectCount\n");
	if(rg_db.redirectHttpCount.enable!=0){
		PROC_PRINTF("rg set redirectHttpCount enable %d count %d denialSec %d pushweb %s\n",rg_db.redirectHttpCount.enable,rg_db.redirectHttpCount.count,rg_db.redirectHttpCount.denialSecs,rg_db.redirectHttpAll.pushweb);
	}
	PROC_PRINTF("\n#add httpRedirectURL\n");
	if(!list_empty(&rg_db.redirectHttpURLListHead)){
		rtk_rg_redirectHttpURL_linkList_t *pRedEntry;
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpURLListHead,url_list){
			PROC_PRINTF("rg add redirectHttpURL url %s dst_url %s count %d\n",pRedEntry->url_data.url_str,pRedEntry->url_data.dst_url_str,pRedEntry->url_data.count);
		}
	}
	PROC_PRINTF("\n#add httpRedirectWhiteList\n");
	if(!list_empty(&rg_db.redirectHttpWhiteListListHead)){
		rtk_rg_redirectHttpWhiteList_linkList_t *pRedEntry;
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpWhiteListListHead,white_list){
			PROC_PRINTF("rg add redirectHttpWhiteList url %s keyword %s\n",pRedEntry->white_data.url_str,pRedEntry->white_data.keyword_str);
		}
	}
	PROC_PRINTF("\n#add httpRedirectRsp\n");
	if(rg_db.redirectHttpRsp.enable!=0){
		PROC_PRINTF("rg set redirectHttpRsp enable %d statusCode %d url %s\n",rg_db.redirectHttpRsp.enable,rg_db.redirectHttpRsp.statusCode,rg_db.redirectHttpRsp.url_str);
	}

	PROC_PRINTF("\n#add static_route\n");
	for(i=0;i<MAX_STATIC_ROUTE_SIZE;i++){
		if(rg_db.staticRoute[i].valid){
			if(rg_db.staticRoute[i].info.ip_version){
				rtk_ipv6_addr_t zeroV6Addr={{0}};
				if(!memcmp(rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr,zeroV6Addr.ipv6_addr,IPV6_ADDR_LEN))
				{
					PROC_PRINTF("rg add staticRoute ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mask-length %d localRouteNetif %d\n",
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[1],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[3],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[5],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[7],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[9],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[11],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[13],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[15],
								rg_db.staticRoute[i].info.ipv6.mask_length,
								rg_db.staticRoute[i].info.ipv6.linklocal_intfidx);
				}else{
					if(rg_db.staticRoute[i].info.nexthop_mac_auto_learn)
					{
						if(rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[0]==0xfe&&rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[1]==0x80)
							PROC_PRINTF("rg add staticRoute ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mask-length %d nexthop %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nexthopMac autolearn nexthopNetif %d\n",
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[1],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[3],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[5],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[7],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[9],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[11],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[13],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[15],
								rg_db.staticRoute[i].info.ipv6.mask_length,
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[1],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[3],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[5],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[7],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[9],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[11],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[13],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[15],
								rg_db.staticRoute[i].nxtip_intfidx);
						else
							PROC_PRINTF("rg add staticRoute ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mask-length %d nexthop %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nexthopMac autolearn\n",
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[1],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[3],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[5],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[7],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[9],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[11],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[13],
								rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[15],
								rg_db.staticRoute[i].info.ipv6.mask_length,
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[1],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[3],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[5],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[7],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[9],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[11],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[13],
								rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[15]);
					}
					else
						PROC_PRINTF("rg add staticRoute ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x mask-length %d nexthop %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x nexthopMac %02x:%02x:%02x:%02x:%02x:%02x port %d nexthopNetif %d\n",
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[1],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[3],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[5],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[7],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[9],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[11],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[13],
							rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[15],
							rg_db.staticRoute[i].info.ipv6.mask_length,
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[1],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[3],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[5],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[7],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[9],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[11],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[13],
							rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[15],
							rg_db.staticRoute[i].info.nexthop_mac.octet[0],
							rg_db.staticRoute[i].info.nexthop_mac.octet[1],
							rg_db.staticRoute[i].info.nexthop_mac.octet[2],
							rg_db.staticRoute[i].info.nexthop_mac.octet[3],
							rg_db.staticRoute[i].info.nexthop_mac.octet[4],
							rg_db.staticRoute[i].info.nexthop_mac.octet[5],
							rg_db.staticRoute[i].info.nexthop_port,
							rg_db.staticRoute[i].nxtip_intfidx);
				}
			}else{
				if(rg_db.staticRoute[i].info.ipv4.nexthop==0){					
					PROC_PRINTF("rg add staticRoute ip %d.%d.%d.%d mask %d.%d.%d.%d localRouteNetif %d%s\n",
						(rg_db.staticRoute[i].info.ipv4.addr & 0xff000000)>>24,
						(rg_db.staticRoute[i].info.ipv4.addr & 0xff0000)>>16,
						(rg_db.staticRoute[i].info.ipv4.addr & 0xff00)>>8,
						(rg_db.staticRoute[i].info.ipv4.addr & 0xff),
						(rg_db.staticRoute[i].info.ipv4.mask & 0xff000000)>>24,
						(rg_db.staticRoute[i].info.ipv4.mask & 0xff0000)>>16,
						(rg_db.staticRoute[i].info.ipv4.mask & 0xff00)>>8,
						(rg_db.staticRoute[i].info.ipv4.mask & 0xff),
						rg_db.staticRoute[i].info.ipv4.localroute_intfidx,
						rg_db.staticRoute[i].info.ipv4.exclude_subnet?" excludeSubnet":"");
				}else{
					if(rg_db.staticRoute[i].info.nexthop_mac_auto_learn)
						PROC_PRINTF("rg add staticRoute ip %d.%d.%d.%d mask %d.%d.%d.%d nexthop %d.%d.%d.%d nexthopMac autolearn\n",
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff),
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff),
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff));
					else
						PROC_PRINTF("rg add staticRoute ip %d.%d.%d.%d mask %d.%d.%d.%d nexthop %d.%d.%d.%d nexthopMac %02x:%02x:%02x:%02x:%02x:%02x port %d\n",
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.addr & 0xff),
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.mask & 0xff),
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff000000)>>24,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff0000)>>16,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff00)>>8,
							(rg_db.staticRoute[i].info.ipv4.nexthop & 0xff),
							rg_db.staticRoute[i].info.nexthop_mac.octet[0],
							rg_db.staticRoute[i].info.nexthop_mac.octet[1],
							rg_db.staticRoute[i].info.nexthop_mac.octet[2],
							rg_db.staticRoute[i].info.nexthop_mac.octet[3],
							rg_db.staticRoute[i].info.nexthop_mac.octet[4],
							rg_db.staticRoute[i].info.nexthop_mac.octet[5],
							rg_db.staticRoute[i].info.nexthop_port);
				}
			}
		}
	}

	PROC_PRINTF("#add acl-filter\n");	//acl_weight, action(*), fwding_type_and_direction and pattern(*)
	{
		_dump_rg_acl_and_cf_diagshell(s);
	}

	PROC_PRINTF("#add napt-filter\n");
	{
		_dump_rg_napt_filterAndQos_diagshell(s);
	}

	{
		PROC_PRINTF("#add gponDsBcFilter\n");
		if(rg_db.systemGlobal.gponDsBCModuleEnable==1){
			PROC_PRINTF("rg set gponDsBcFilter module 1\n");
		}else{
			PROC_PRINTF("rg set gponDsBcFilter module 0\n");
		}


		for(i=0;i<MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE;i++){
			if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].valid==1){
				PROC_PRINTF("rg clear gponDsBcFilter\n");
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_EGRESS_PORT_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern egress_portmask 0x%x\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.egress_portmask.portmask);
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern ingress_ctagIf %d\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ingress_ctagIf);
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_INGRESS_CVID_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern ingress_ctag_cvid %d\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ingress_ctag_cvid);
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern ingress_stagIf %d\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ingress_stagIf);
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_INGRESS_SVID_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern ingress_stag_svid %d\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ingress_stag_svid);
				if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.filter_fields & GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT)
					PROC_PRINTF("rg set gponDsBcFilter pattern ingress_stream_id %d\n",rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ingress_stream_id);

				PROC_PRINTF("rg set gponDsBcFilter action action_type %d\n",
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.action_type);
				PROC_PRINTF("rg set gponDsBcFilter action tag_decision %d tag_cvid %d tag_cpri %d\n",
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ctag_action.ctag_decision,
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ctag_action.assigned_ctag_cvid,
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.ctag_action.assigned_ctag_cpri);
				PROC_PRINTF("rg set gponDsBcFilter action stag_decision %d tag_svid %d tag_spri %d\n",
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.stag_action.stag_decision,
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.stag_action.assigned_stag_svid,
					rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule.stag_action.assigned_stag_spri);
				PROC_PRINTF("rg add gponDsBcFilter entry %d\n",i);

			}
		}
	}

	{
		uint32 svlanTpid;
		uint enable;
		rtk_rg_apollo_svlanTpid_get(&svlanTpid);
PROC_PRINTF("rg add svlanTpid 0x%x\n",svlanTpid);
#if defined(CONFIG_RTL9602C_SERIES)
		(pf.rtk_rg_svlanTpid2_enable_get)(&enable);
		if(enable){
			PROC_PRINTF("rg add svlanTpid2_enable 1\n",svlanTpid);
			(pf.rtk_rg_svlanTpid2_get)(&svlanTpid);
			PROC_PRINTF("rg add svlanTpid2 0x%x\n",svlanTpid);
		}else
			PROC_PRINTF("rg add svlanTpid2_enable 0\n",svlanTpid);
#endif
		for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
		{
			if(RG_INVALID_MAC_PORT(i)) continue;
			rtk_rg_apollo_svlanServicePort_get(i,&enable);
PROC_PRINTF("rg add svlanServicePort port %d enable %d\n", i, enable);
		}
		PROC_PRINTF("\n");
	}


PROC_PRINTF("exit\n");
	{
		PROC_PRINTF("# Difference of rg init (default enable IGMP)\n");

#if defined(CONFIG_RG_RTL9602C_SERIES)
		if (rg_kernel.force_cf_pattern0_size_enable!=0){//default 0
			PROC_PRINTF("echo %d > proc/rg/force_cf_pattern0_size_state \n", rg_kernel.force_cf_pattern0_size_enable);
			PROC_PRINTF("echo %d > proc/rg/cf_pattern0_size \n", rg_kernel.cf_pattern0_size);
		}
#endif

		if (rg_kernel.autoTestMode) { //0
			PROC_PRINTF("echo 1 > proc/rg/autoTestMode\n", rg_kernel.autoTestMode);
		}

		for(i=0;i<MAX_ALG_FUNCTIONS;i++){
			switch(i){
		//Server in WAN
				case RTK_RG_ALG_SIP_TCP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_SIP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_SIP_TCP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_SIP_UDP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_SIP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_SIP_UDP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_H323_TCP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_H323_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_H323_TCP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_H323_UDP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_H323_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_H323_UDP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_RTSP_TCP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_RTSP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_RTSP_TCP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_RTSP_UDP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_RTSP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_RTSP_UDP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_FTP_TCP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_FTP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_FTP_TCP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_FTP_UDP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_FTP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_FTP_UDP_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_TFTP_UDP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_TFTP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_TFTP_UDP_BIT,rg_db.algUserDefinedPort[i]);
					break;

		//Server in LAN
				case RTK_RG_ALG_SIP_TCP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_SIP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_SIP_UDP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_SIP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_H323_TCP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_H323_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_H323_TCP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_H323_UDP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_H323_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_H323_UDP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_RTSP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_RTSP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_FTP_TCP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_FTP_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_FTP_UDP_SRV_IN_LAN:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_FTP_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT,rg_db.algUserDefinedPort[i]);
					break;

		//Pass through
				case RTK_RG_ALG_PPTP_TCP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_PPTP_PASSTHROUGH_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_PPTP_UDP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_PPTP_PASSTHROUGH_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_L2TP_TCP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_L2TP_PASSTHROUGH_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_L2TP_TCP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_L2TP_UDP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_L2TP_PASSTHROUGH_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_L2TP_UDP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_IPSEC_PASSTHROUGH_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
				case RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_IPSEC_PASSTHROUGH_UDP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH_BIT,rg_db.algUserDefinedPort[i]);
					break;
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
				case RTK_RG_ALG_BATTLENET_TCP:
					if(rg_db.algUserDefinedPort[i]!=RTK_RG_ALG_BATTLENET_TCP_PORT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_port_num\n",RTK_RG_ALG_BATTLENET_TCP_BIT,rg_db.algUserDefinedPort[i]);
					break;
#endif
				default:
					break;
			}
		}

		for(i=0;i<MAX_ALG_FUNCTIONS;i++){
			switch(i){
				case RTK_RG_ALG_SIP_TCP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_SIP_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_SIP_TCP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
				case RTK_RG_ALG_H323_TCP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_H323_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_H323_TCP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
				case RTK_RG_ALG_RTSP_TCP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_RTSP_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_RTSP_TCP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
				case RTK_RG_ALG_FTP_TCP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_FTP_ACTIVE_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_FTP_TCP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
				case RTK_RG_ALG_FTP_TCP_SRV_IN_LAN:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_FTP_PASV_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
				case RTK_RG_ALG_BATTLENET_TCP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_BATTLENET_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_BATTLENET_TCP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
#endif
				case RTK_RG_ALG_TFTP_UDP:
					if(rg_db.algUserDefinedTimeout[i]!=ALG_TFTP_UPNP_TIMEOUT)PROC_PRINTF("echo 0x%x %d > /proc/rg/alg_user_defined_time_out\n",RTK_RG_ALG_TFTP_UDP_BIT,rg_db.algUserDefinedTimeout[i]);
					break;
				default:
					break;
			}
		}

		if (rg_db.systemGlobal.BCRateLimitShareMeterIdx != RG_INIT_DEFAULT_BC_rate_limit) { //-1
			PROC_PRINTF("echo %d > proc/rg/BC_rate_limit\n", rg_db.systemGlobal.BCRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.BCRateLimitPortMask != RG_INIT_DEFAULT_BC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %x > proc/rg/BC_rate_limit_portMask\n", rg_db.systemGlobal.BCRateLimitPortMask);
		}

		if (rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx != RG_INIT_DEFAULT_IPv4_MC_rate_limit) { //-1 disable
			PROC_PRINTF("echo %d > proc/rg/IPv4_MC_rate_limit\n", rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.IPv4MCRateLimitPortMask != RG_INIT_DEFAULT_IPv4_MC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %x > proc/rg/IPv4_MC_rate_limit_portMask\n", rg_db.systemGlobal.IPv4MCRateLimitPortMask);
		}

		if (rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx != RG_INIT_DEFAULT_IPv6_MC_rate_limit) { //-1
			PROC_PRINTF("echo %d > proc/rg/IPv6_MC_rate_limit\n", rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.IPv6MCRateLimitPortMask != RG_INIT_DEFAULT_IPv6_MC_rate_limit_portMask) { //0x0
			PROC_PRINTF("echo %x > proc/rg/IPv6_MC_rate_limit_portMask\n", rg_db.systemGlobal.IPv6MCRateLimitPortMask);
		}

		if (rg_db.systemGlobal.aclDropIpRangeBySwEnable != RG_INIT_DEFAULT_acl_drop_ip_range_rule_handle_by_sw) { //0: disable
			PROC_PRINTF("echo %d > proc/rg/acl_drop_ip_range_rule_handle_by_sw\n", rg_db.systemGlobal.aclDropIpRangeBySwEnable);
		}

		if (rg_db.systemGlobal.aclPermitIpRangeBySwEnable != RG_INIT_DEFAULT_acl_permit_ip_range_rule_handle_by_sw) { //0: disable
			PROC_PRINTF("echo %d > proc/rg/acl_permit_ip_range_rule_handle_by_sw\n", rg_db.systemGlobal.aclPermitIpRangeBySwEnable);
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
		if(rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid!=RG_INIT_DEFAULT_acl_rearrange_force_mc_ingress_cvid){
			PROC_PRINTF("echo %d > proc/rg/acl_force_mc_cvid_when_rearrange\n", rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid);
		}
#endif
		if (rg_db.systemGlobal.aclPolicingMode != RG_INIT_DEFAULT_acl_policing_mode) { // 0: policing
			PROC_PRINTF("echo %d > proc/rg/turn_on_acl_counter\n", rg_db.systemGlobal.aclPolicingMode);
		}

		//PROC_PRINTF("echo %d > proc/rg/acl_reserved_arrange\n", rg_db.systemGlobal.acl_reserved_arrange);

		if (rg_db.systemGlobal.arp_max_request_count != RG_INIT_DEFAULT_arp_max_request_count) { //0
			PROC_PRINTF("echo %d > proc/rg/arp_max_request_count\n", rg_db.systemGlobal.arp_max_request_count);
		}

		if (rg_db.systemGlobal.arp_requset_interval_sec != RG_INIT_DEFAULT_arp_request_interval_sec) { //1
			PROC_PRINTF("echo %d > proc/rg/arp_request_interval_sec\n", rg_db.systemGlobal.arp_requset_interval_sec);
		}

		if (rg_db.systemGlobal.icmp_timeout != RTK_RG_DEFAULT_ICMP_TRACKING_TIMEOUT_INTERVAL_SECOND) { //300
			PROC_PRINTF("echo %d > proc/rg/icmp_timeout\n", rg_db.systemGlobal.icmp_timeout);
		}

		if (rg_db.systemGlobal.arp_timeout != RG_INIT_DEFAULT_arp_timeout) { //300
			PROC_PRINTF("echo %d > proc/rg/arp_timeout\n", rg_db.systemGlobal.arp_timeout);
		}

		if (rg_db.systemGlobal.antiIpSpoofStatus != RTK_RG_DISABLED) { // disable
			PROC_PRINTF("echo %d > proc/rg/ip_anti_spoofing_enable\n", rg_db.systemGlobal.antiIpSpoofStatus);
		}

		if (rg_db.systemGlobal.antiMacSpoofStatus != RTK_RG_DISABLED) { // disable
			PROC_PRINTF("echo %d > proc/rg/mac_anti_spoofing_enable\n", rg_db.systemGlobal.antiMacSpoofStatus);
		}


#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if(rg_db.systemGlobal.l2_timeout != RTK_RG_DEFAULT_L2_TIMEOUT) {
			PROC_PRINTF("echo %d > proc/rg/l2_timeout\n", rg_db.systemGlobal.l2_timeout);
		}
#endif
		if(rg_db.systemGlobal.fragment_timeout != RTK_RG_DEFAULT_FRAGMENT_LIST_TIMEOUT) {
			PROC_PRINTF("echo %d > proc/rg/fragment_timeout\n", rg_db.systemGlobal.fragment_timeout);
		}

		if (rg_db.systemGlobal.fragShortcut_off != 0) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_fragShortcut\n", rg_db.systemGlobal.fragShortcut_off);
		}


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY] != RG_INIT_DEFAULT_assign_ack_priority) //disable : 0
		{
			PROC_PRINTF("echo %d > proc/rg/assign_ack_priority\n",rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY]);
		}
#else
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY] != RG_INIT_DEFAULT_assign_ack_priority_and_disable_svlan) //disable : 0
		{
			PROC_PRINTF("echo %d > proc/rg/assign_ack_priority_and_disable_svlan\n",rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY]);
		}
#endif
		if(rg_db.systemGlobal.trapSpecificLengthAck.portMask)
		{
			PROC_PRINTF("echo Portmask 0x%x Length_start %d Length_end %d Ack_only %d Trap %d Priority %d > proc/rg/assign_ack_priority_or_trap\n",
				rg_db.systemGlobal.trapSpecificLengthAck.portMask,
				rg_db.systemGlobal.trapSpecificLengthAck.pktLenStart,
				rg_db.systemGlobal.trapSpecificLengthAck.pktLenEnd,
				rg_db.systemGlobal.trapSpecificLengthAck.ackOnly,
				rg_db.systemGlobal.trapSpecificLengthAck.isTrap,
				rg_db.systemGlobal.trapSpecificLengthAck.priority);
			// Another proc: echo Portmask 0x%x Length_start %d Length_end %d Priority %d > proc/rg/trap_specific_length_ack
		}

		if(rg_db.systemGlobal.trapSpecificLengthSyn.portMask)
		{
			PROC_PRINTF("echo Portmask 0x%x Length_start %d Length_end %d Syn_only %d Trap %d Priority %d > proc/rg/assign_syn_priority_or_trap\n",
				rg_db.systemGlobal.trapSpecificLengthSyn.portMask,
				rg_db.systemGlobal.trapSpecificLengthSyn.pktLenStart,
				rg_db.systemGlobal.trapSpecificLengthSyn.pktLenEnd,
				rg_db.systemGlobal.trapSpecificLengthSyn.synOnly,
				rg_db.systemGlobal.trapSpecificLengthSyn.isTrap,
				rg_db.systemGlobal.trapSpecificLengthSyn.priority);
		}

		if(rg_db.systemGlobal.wanAccessLimit_interval != CONFIG_RG_ACCESSWAN_TIMER_DELAY) {
			PROC_PRINTF("echo %d > proc/rg/wan_access_limit_request_interval\n", rg_db.systemGlobal.wanAccessLimit_interval);
		}

		if(rg_kernel.wanDmac2cvidDisabled!=0) //default : 0
		{
			PROC_PRINTF("echo 1 > proc/rg/wan_dmac2cvid_force_disabled\n");
		}

		for (i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++) {
			if (rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]) { //default : 0
				//PROC_PRINTF("Netif[%d]:%s\n",i,rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]==1?"Drop IPv6(IPv4 Pass)":"Drop IPv4(IPv46 Pass)");
				PROC_PRINTF("echo %d %d > proc/rg/bridgeWan_drop_by_protocal\n", i, rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[i]);
			}
		}

		for (i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++) {
			if (rg_db.systemGlobal.wanIntf_disable_linkLocal_rsvACL[i]) { //default : 0
				PROC_PRINTF("echo %d %d > proc/rg/wanIntf_disable_ipv6_linkLocal_rsvACL\n", i, rg_db.systemGlobal.wanIntf_disable_linkLocal_rsvACL[i]);
			}
		}

		/*
			PROC_PRINTF("echo %d > proc/rg/callback\n");
		*/

		if (rg_db.systemGlobal.congestionCtrlIntervalMicroSecs != RG_INIT_DEFAULT_congestion_ctrl_interval_usec) { //0
//			p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//			p->write_proc = (void *)_rtk_rg_proc_congestionCtrlHwTimerFunc_set;
			PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_interval_usec\n", rg_db.systemGlobal.congestionCtrlIntervalMicroSecs);

			//if (rg_db.systemGlobal.congestionCtrlInboundAckToHighQueue != 0) { //Always set again
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_inbound_ack_to_high_queue\n", rg_db.systemGlobal.congestionCtrlInboundAckToHighQueue);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlInboundAckToHighQueue_set;
			//}

			//if (rg_db.systemGlobal.congestionCtrlPortMask != 0) { //If it is 0, the effect as congestionCtrlIntervalMicroSecs to be 0
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_port_mask\n", rg_db.systemGlobal.congestionCtrlPortMask);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlPortMask_set;
			//}

			//if (rg_db.systemGlobal.congestionCtrlSendBytesPerSec[0] != 12500000) { //Use Port0
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_byte_per_sec\n", rg_db.systemGlobal.congestionCtrlSendBytesPerSec[0]);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendBytePerSec_set;
			//}

			//if (rg_db.systemGlobal.congestionCtrlSendBytesPerSec[RTK_RG_PORT_PON] != 12500000) { //Use PON port
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_byte_per_sec_for_wan\n", rg_db.systemGlobal.congestionCtrlSendBytesPerSec[RTK_RG_PORT_PON]);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendBytePerSecForWan_set;
			//}

			//if (rg_db.systemGlobal.congestionCtrlSendRemainderInNextGap != 0) {
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_remainder_in_next_gap\n", rg_db.systemGlobal.congestionCtrlSendRemainderInNextGap);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendRemainderInNextGap_set;
			//}

			//if (rg_db.systemGlobal.congestionCtrlSendTimesPerPort != 0) {
				PROC_PRINTF("echo %d > proc/rg/congestion_ctrl_send_times_per_port\n", rg_db.systemGlobal.congestionCtrlSendTimesPerPort);
//				p->read_proc = (void *)_rtk_rg_proc_congestionCtrl_get;
//				p->write_proc = (void *)_rtk_rg_proc_congestionCtrlSendTimesPerPort_set;
			//}

		}

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		ASSERT_EQ(rtk_l34_globalState_get(L34_GLOBAL_TTLMINUS_STATE, &state),RT_ERR_OK);
#else
		ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_GET(FB_GLOBAL_TTL_1, &state),RT_ERR_OK);
#endif
		if (state != ENABLED) { // 1
			PROC_PRINTF("echo 0 > proc/rg/control_hw_TTL_minus\n");
		}

		/*	PROC_PRINTF("echo %d > proc/rg/debug_level\n", rg_db.systemGlobal.debug_level);
			PROC_PRINTF("echo %d > proc/rg/dump_ps_rx_pkt\n", rg_db.systemGlobal.dump_ps_rx_pkt);
			PROC_PRINTF("echo %d > proc/rg/filter_level\n", rg_db.systemGlobal.filter_level);
			PROC_PRINTF("echo %d > proc/rg/fwd_statistic\n");
		*/
#if defined(CONFIG_SMP)
		if (rg_db.systemGlobal.distributedMechanism){
			PROC_PRINTF("echo %d > proc/rg/distributedMechanism\n", rg_db.systemGlobal.distributedMechanism);
		}

		if (rg_db.systemGlobal.disableNicTxDistributed){
			PROC_PRINTF("echo %d > proc/rg/disableNicTxDistributed\n", rg_db.systemGlobal.disableNicTxDistributed);
		}

		if (rg_db.systemGlobal.protocolStackBypassRxQueue){
			PROC_PRINTF("echo %d > proc/rg/protocolStackBypassRxQueue\n", rg_db.systemGlobal.protocolStackBypassRxQueue);
		}

		if (rg_db.systemGlobal.disableFwdEngineDistributed){
			PROC_PRINTF("echo %d > proc/rg/disableFwdEngineDistributed\n", rg_db.systemGlobal.disableFwdEngineDistributed);
		}

		if (rg_db.systemGlobal.criticalPacketBypassRxQueue!=GMAC_PRIORITY_CRITICAL){
			PROC_PRINTF("echo %d > proc/rg/criticalPriorityBypassRxQueue\n", rg_db.systemGlobal.criticalPacketBypassRxQueue);
		}
#endif
		if (rg_db.systemGlobal.inboundL4UnknownUdpConnDrop){
			PROC_PRINTF("echo %d > proc/rg/inboundL4UnknownUdpConnDrop\n", rg_db.systemGlobal.inboundL4UnknownUdpConnDrop);
		}

		if (rg_db.systemGlobal.forceWifiUntag != RG_INIT_DEFAULT_forceWifiUntag){
			PROC_PRINTF("echo %d > proc/rg/forceWifiUntag\n", rg_db.systemGlobal.forceWifiUntag);
		}

		if (rg_db.systemGlobal.keepWifiUntagToPS){
			PROC_PRINTF("echo %d > proc/rg/keepWifiUntagToPS\n", rg_db.systemGlobal.keepWifiUntagToPS);
		}

		if (rg_db.systemGlobal.gatherLanNetInfo != RG_INIT_DEFAULT_gatherLanNetInfo){
			PROC_PRINTF("echo %d > proc/rg/gather_lanNetInfo\n", rg_db.systemGlobal.gatherLanNetInfo);
		}

		if (rg_db.systemGlobal.dpi_accelerate_enable){
			PROC_PRINTF("echo %d > proc/rg/accelerateDPISeconds\n", rg_db.systemGlobal.dpi_accelerate_enable);
		}

		if (rg_db.systemGlobal.gponDsBCModuleEnable != RG_INIT_DEFAULT_gponDsBCModuleEnable) { //Default Disable(0)
			PROC_PRINTF("echo %d > proc/rg/gponDsBCModuleEnable\n", rg_db.systemGlobal.gponDsBCModuleEnable);
		}

		if (rg_db.systemGlobal.house_keep_sec != RG_INIT_DEFAULT_house_keep_sec) { //2
			PROC_PRINTF("echo %d > proc/rg/house_keep_sec\n", rg_db.systemGlobal.house_keep_sec);
		}

		if (rg_db.systemGlobal.hwnat_enable != RG_INIT_DEFAULT_hwnat) { // Default RG_HWNAT_ENABLE:1
			PROC_PRINTF("echo %d > proc/rg/hwnat\n", rg_db.systemGlobal.hwnat_enable);
		}

		if (rg_db.systemGlobal.initParam.igmpSnoopingEnable != RG_INIT_DEFAULT_igmpSnooping) { //default on:1
			PROC_PRINTF("echo %d > proc/rg/igmpSnooping\n", rg_db.systemGlobal.initParam.igmpSnoopingEnable);
//			p->read_proc = (void *)igmp_show;
//			p->write_proc = (void *)_rtk_rg_igmpSnooping_set;
		}


		if (rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave != RG_INIT_DEFAULT_igmp_fastLeave) { //default off
			PROC_PRINTF("echo %d > proc/rg/igmp_fastLeave\n", rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave);
		}

		/* just show status, can not be set
			PROC_PRINTF("echo %d > proc/rg/igmp_groupList_memDump\n");
		*/

		if (rtl_mCastTimerParas.groupMemberAgingTime != RG_INIT_DEFAULT_igmp_groupMemberAgingTime) { //260
			PROC_PRINTF("echo %d > proc/rg/igmp_groupMemberAgingTime\n", rtl_mCastTimerParas.groupMemberAgingTime);
		}

		if (rtl_mCastTimerParas.lastMemberAgingTime != RG_INIT_DEFAULT_igmp_lastMemberAgingTime) { //10
			PROC_PRINTF("echo %d > proc/rg/igmp_lastMemberAgingTime\n", rtl_mCastTimerParas.lastMemberAgingTime);
		}

		if (rg_db.systemGlobal.igmp_max_simultaneous_group_size != RG_INIT_DEFAULT_igmp_max_simultaneous_group_size) {
			PROC_PRINTF("echo %d > proc/rg/igmp_max_simultaneous_group_size\n", rg_db.systemGlobal.igmp_max_simultaneous_group_size);
		}

		if (rg_db.systemGlobal.igmp_max_system_client_size != 0) {
			PROC_PRINTF("echo %d > proc/rg/igmp_max_system_client_size\n", rg_db.systemGlobal.igmp_max_system_client_size);
		}
		if (rg_db.systemGlobal.mld_max_system_client_size != 0) {
			PROC_PRINTF("echo %d > proc/rg/mld_max_system_client_size\n", rg_db.systemGlobal.mld_max_system_client_size);
		}
		if (rg_db.systemGlobal.igmp_max_specific_client_join_group_size != 0) {
			PROC_PRINTF("echo %d > proc/rg/igmp_max_specific_client_join_group_size\n", rg_db.systemGlobal.igmp_max_specific_client_join_group_size);
		}
		if (rg_db.systemGlobal.mld_max_specific_client_join_group_size != 0) {
			PROC_PRINTF("echo %d > proc/rg/mld_max_specific_client_join_group_size\n", rg_db.systemGlobal.mld_max_specific_client_join_group_size);
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if (rg_db.systemGlobal.igmp_pppoe_passthrough_learning!= 0) {
			PROC_PRINTF("echo %d > proc/rg/igmp_pppoe_passthrough_learning\n", rg_db.systemGlobal.igmp_pppoe_passthrough_learning);
		}
#endif
		//igmp query filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpMldQueryPortmask != RG_INIT_DEFAULT_igmp_mld_query_filter_portmask) { //RTK_RG_ALL_PORTMASK
			PROC_PRINTF("echo %x > proc/rg/igmp_mld_query_filter_portmask\n", rg_db.systemGlobal.igmpMldQueryPortmask);
		}
	#if defined(CONFIG_MASTER_WLAN0_ENABLE)
		if (rg_db.systemGlobal.igmpWifiRefEnable != RG_INIT_DEFAULT_igmp_WifiRefEnable) { //0x0 disable
			PROC_PRINTF("echo %d > proc/rg/igmp_wifiRefEnable\n", rg_db.systemGlobal.igmpWifiRefEnable);
		}
	#endif

		if (rg_db.systemGlobal.icmpRedirectToDMZ != RG_INIT_DEFAULT_icmpRedirectToDMZ) { //0x0 disable
			PROC_PRINTF("echo %d > proc/rg/icmpRedirectToDMZ\n", rg_db.systemGlobal.icmpRedirectToDMZ);
		}


		if (rtl_mCastTimerParas.querierPresentInterval != RG_INIT_DEFAULT_igmp_querierPresentInterval) { //DEFAULT_LAST_MEMBER_INTERVAL
			PROC_PRINTF("echo %d > proc/rg/igmp_querierPresentInterval\n", rtl_mCastTimerParas.querierPresentInterval);
		}


		//igmp report ingress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpReportIngressPortmask != RG_INIT_DEFAULT_igmp_report_filter_portmask) { //RTK_RG_ALL_PORTMASK
			PROC_PRINTF("echo %x > proc/rg/igmp_report_ingress_filter_portmask\n", rg_db.systemGlobal.igmpReportIngressPortmask);
		}


		//igmp report ingress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpProxyOnly2Wifi!= 0) { //0
			PROC_PRINTF("echo %x > proc/rg/igmpProxyOnly2Wifi\n", rg_db.systemGlobal.igmpProxyOnly2Wifi);
		}


		//igmp report egress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpReportPortmask != RG_INIT_DEFAULT_igmp_report_filter_portmask) { //RTK_RG_ALL_PORTMASK
			PROC_PRINTF("echo %x > proc/rg/igmp_report_filter_portmask\n", rg_db.systemGlobal.igmpReportPortmask);
		}

		//igmp leave egress filter portmask, default enabled all port permit
		if (rg_db.systemGlobal.igmpLeavePortmask!= RG_INIT_DEFAULT_igmp_leave_filter_portmask) { //RTK_RG_ALL_PORTMASK
			PROC_PRINTF("echo %x > proc/rg/igmp_leave_filter_portmask\n", rg_db.systemGlobal.igmpLeavePortmask);
		}


		if (rg_db.systemGlobal.igmp_sys_timer_sec != RG_INIT_DEFAULT_igmp_sys_timer_sec) { //10
			PROC_PRINTF("echo %d > proc/rg/igmp_sys_timer_sec\n", rg_db.systemGlobal.igmp_sys_timer_sec);
		}

		if (rg_db.systemGlobal.igmp_Trap_to_PS_enable != RG_INIT_DEFAULT_igmp_trap_to_PS) { //Disable
			PROC_PRINTF("echo %d > proc/rg/igmp_trap_to_PS\n", rg_db.systemGlobal.igmp_Trap_to_PS_enable);
		}

		if (rg_db.systemGlobal.ipsec_passthru != RG_INIT_DEFAULT_ipsec_passthru) { //PASS_OLD
			PROC_PRINTF("echo %d > proc/rg/ipsec_passthru\n", rg_db.systemGlobal.ipsec_passthru);
		}


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		//apolloPro no need this proc to support avalanche
#else
		if (rg_db.systemGlobal.ipv6MC_translate_ingressVID_enable != RG_INIT_DEFAULT_ipv6MC_tranlate_ingressVid) { //ENABLED
			PROC_PRINTF("echo %d > proc/rg/ipv6MC_tranlate_ingressVid\n", rg_db.systemGlobal.ipv6MC_translate_ingressVID_enable);
		}
#endif
		if (rg_db.systemGlobal.keepPsOrigCvlan != RG_INIT_DEFAULT_keep_protocol_stack_packets_orig_cvlan) { //default 0: Follow Normal CVLAN Decision
			PROC_PRINTF("echo %d > proc/rg/keep_protocol_stack_packets_orig_cvlan\n", rg_db.systemGlobal.keepPsOrigCvlan);
		}

		if (rg_db.systemGlobal.enableL4ChoiceHwIn != RG_INIT_DEFAULT_l4_choice_hw_in) { //current 0
			PROC_PRINTF("echo %d > proc/rg/l4_choice_hw_in\n", rg_db.systemGlobal.enableL4ChoiceHwIn);
		}

		if (rg_db.systemGlobal.enableL4MaxWays != RG_INIT_DEFAULT_l4ways) { //current : disable
			PROC_PRINTF("echo %d > proc/rg/l4ways\n", rg_db.systemGlobal.enableL4MaxWays );
		}

		if (rg_db.systemGlobal.enableL4WaysList != RG_INIT_DEFAULT_l4ways_list) {//currrent : 0
			PROC_PRINTF("echo %d > proc/rg/l4ways_list\n", rg_db.systemGlobal.enableL4WaysList);
		}

		if (rg_kernel.layer2LookupMissFlood2CPU == RTK_RG_ENABLED) {//current:disable
			PROC_PRINTF("echo %d > proc/rg/layer2LookupMissFlood2CPU\n", rg_kernel.layer2LookupMissFlood2CPU);
		}

		if (rg_db.systemGlobal.forceReportResponseTime != RG_INIT_DEFAULT_mcast_force_report_sec) { //current : disable
			PROC_PRINTF("echo %d > proc/rg/mcast_force_report_sec\n",rg_db.systemGlobal.forceReportResponseTime);
		}

		if (rg_db.systemGlobal.multicastProtocol != RG_INIT_DEFAULT_mcast_protocol) { //current: RG_MC_BOTH_IGMP_MLD
			PROC_PRINTF("echo %d > proc/rg/mcast_protocol\n", rg_db.systemGlobal.multicastProtocol);
		}

		if (rg_db.systemGlobal.mcast_query_sec != RG_INIT_DEFAULT_mcast_query_sec) { //default : 30 sec
			PROC_PRINTF("echo %d > proc/rg/mcast_query_sec\n", rg_db.systemGlobal.mcast_query_sec);
		}

		if (rg_db.systemGlobal.mld_Trap_to_PS_enable != RG_INIT_DEFAULT_mld_trap_to_PS) { //DISABLED
			PROC_PRINTF("echo %d > proc/rg/mld_trap_to_PS\n", rg_db.systemGlobal.mld_Trap_to_PS_enable);
		}

		if (rg_db.systemGlobal.neighbor_timeout != RG_INIT_DEFAULT_neighbor_timeout) { //default : 300 sec
			PROC_PRINTF("echo %d > proc/rg/neighbor_timeout\n", rg_db.systemGlobal.neighbor_timeout);
		}
	#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	#else
		if (rg_db.systemGlobal.ponPortUnmatchCfDrop != RG_INIT_DEFAULT_pon_port_unmatch_cf_drop) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/pon_port_unmatch_cf_drop\n", rg_db.systemGlobal.ponPortUnmatchCfDrop);
		}
	#endif

		if (rg_db.systemGlobal.port_binding_by_protocal != RG_INIT_DEFAULT_portBindingByProtocal) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/portBindingByProtocal\n", rg_db.systemGlobal.port_binding_by_protocal);
		}

		if (rg_db.systemGlobal.port_binding_by_protocal_filter_vid != RG_INIT_DEFAULT_portBindingByProtocal_filter_downstream_vid) { //current: 0
			PROC_PRINTF("echo %d > proc/rg/portBindingByProtocal_filter_downstream_vid\n", rg_db.systemGlobal.port_binding_by_protocal_filter_vid);
		}

		if (rg_db.systemGlobal.pppoe_bc_passthrought_to_bindingWan_enable != RG_INIT_DEFAULT_pppoe_bc_passthrought_to_bindingWan) { //current: DISABLED
			PROC_PRINTF("echo %d > proc/rg/pppoe_bc_passthrought_to_bindingWan\n", rg_db.systemGlobal.pppoe_bc_passthrought_to_bindingWan_enable);
		}

		if (rg_db.systemGlobal.pppoe_mc_routing_trap != RG_INIT_DEFAULT_pppoe_mc_routing_trap) { //current: DISABLED
			PROC_PRINTF("echo %d > proc/rg/pppoe_mc_routing_trap\n", rg_db.systemGlobal.pppoe_mc_routing_trap);
		}

		if (rg_db.systemGlobal.pppoeProxyAllowBindingOnly != RG_INIT_DEFAULT_pppoe_proxy_only_for_binding_packet) { //current : 0/NORMAL
			PROC_PRINTF("echo %d > proc/rg/pppoe_proxy_only_for_binding_packet\n", rg_db.systemGlobal.pppoeProxyAllowBindingOnly);
		}

		if (rg_db.systemGlobal.pppoeGponSmallbandwithControl != RG_INIT_DEFAULT_pppoeGponSmallbandwithControl) { //current : 0/NORMAL
			PROC_PRINTF("echo %d > proc/rg/pppoe_gpon_small_bandwidth_control\n", rg_db.systemGlobal.pppoeGponSmallbandwithControl);
		}

		if (rg_db.systemGlobal.proc_to_pipe != RG_INIT_DEFAULT_proc_to_pipe ) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/proc_to_pipe\n", rg_db.systemGlobal.proc_to_pipe);
		}

		if (rg_db.systemGlobal.psRxMirrorToPort0 != RG_INIT_DEFAULT_ps_rx_mirror_to_port0) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/ps_rx_mirror_to_port0\n", rg_db.systemGlobal.psRxMirrorToPort0);
		}

		/* only read
			PROC_PRINTF("echo %d > proc/rg/qosInternalAndRemark\n");
		*/

	#if defined(RTK_RG_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RG_RTL9600_SERIES)
		/* only write, cat nothing
			PROC_PRINTF("echo %d > proc/rg/qos_type\n");
		*/
	#endif

		/*if (rg_db.systemGlobal.redirect_first_http_req_by_mac != ) {
			PROC_PRINTF("echo %d > proc/rg/redirect_first_http_req_by_mac\n");
		}*/
		if (rg_db.systemGlobal.forcePortal_url_list[0].valid)	//default
				PROC_PRINTF("echo a -1 %s > proc/rg/redirect_first_http_req_set_url\n", rg_db.systemGlobal.forcePortal_url_list[0].url_string);
		for (i=1;i<MAX_FORCE_PORTAL_URL_NUM;i++)
			if (rg_db.systemGlobal.forcePortal_url_list[i].valid) {
				PROC_PRINTF("echo a %d %s > proc/rg/redirect_first_http_req_set_url\n", i-1, rg_db.systemGlobal.forcePortal_url_list[i].url_string);
			}

		if(!list_empty(&rg_db.avoidPortalURLListHead)){
			rtk_rg_avoidPortalURL_linkList_t *pEntry;
			list_for_each_entry(pEntry,&rg_db.avoidPortalURLListHead,avoidPortal_list){
				PROC_PRINTF("echo %s > proc/rg/avoid_force_portal_set_url\n",pEntry->url_str);
			}
		}

		if (rg_db.systemGlobal.fix_l34_to_untag_enable != RG_INIT_DEFAULT_remove_l34_tag_for_same_mac) { //default : RG_HWNAT_DISABLE
			PROC_PRINTF("echo %d > proc/rg/remove_l34_tag_for_same_mac\n", rg_db.systemGlobal.fix_l34_to_untag_enable);
		}

#ifdef CONFIG_DUALBAND_CONCURRENT
		if (rg_db.systemGlobal.enableSlaveSSIDBind != RG_INIT_DEFAULT_enableSlaveSSIDBind){
			PROC_PRINTF("echo %d > proc/rg/slaveWifiBind\n", rg_db.systemGlobal.enableSlaveSSIDBind);
		}
#endif

		/* only for writing and cat nothing, just for debugging purpose
			PROC_PRINTF("echo %d > proc/rg/send_from_cpu\n");
		*/
		//if (rg_kernel.stag_enable != RG_INIT_DEFAULT_stag_enable) {//current : RTK_RG_DISABLED/0
			PROC_PRINTF("# rg_kernel.stag_enable default 0\n");
			PROC_PRINTF("echo %d > proc/rg/stag_enable\n", rg_kernel.stag_enable);
		//}

		if (rg_db.systemGlobal.strangeSA_drop != RG_INIT_DEFAULT_strange_packet_drop) { //current : RG_HWNAT_DISABLE / strangeSA_drop=0, PERMIT!
			PROC_PRINTF("echo %d > proc/rg/strange_packet_drop\n", rg_db.systemGlobal.strangeSA_drop);
		}

		if (rg_db.systemGlobal.tcpDisableSimultaneousSYN) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_disable_simultaneous_SYN\n", rg_db.systemGlobal.tcpDisableSimultaneousSYN);
		}

		if (rg_db.systemGlobal.tcpDisableStatefulTracking != RG_INIT_DEFAULT_tcp_disable_stateful_tracking) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_disable_stateful_tracking\n", rg_db.systemGlobal.tcpDisableStatefulTracking);
		}
		if (rg_db.systemGlobal.tcpDoNotDelWhenRstFin != RG_INIT_DEFAULT_tcp_do_not_del_when_rst_fin) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_do_not_del_when_rst_fin\n", rg_db.systemGlobal.tcpDoNotDelWhenRstFin);
		}

		if (rg_db.systemGlobal.tcpSwapFinDelRst != RG_INIT_DEFAULT_tcp_swap_fin_del_rst) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_swap_fin_del_rst\n", rg_db.systemGlobal.tcpSwapFinDelRst);
		}

		if (rg_db.systemGlobal.tcp_hw_learning_at_syn != RG_INIT_DEFAULT_tcp_hw_learning_at_syn) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_hw_learning_at_syn\n", rg_db.systemGlobal.tcp_hw_learning_at_syn);
		}

		if (rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn != RG_INIT_DEFAULT_tcp_in_shortcut_learning_at_syn) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/tcp_in_shortcut_learning_at_syn\n", rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn);
		}

		if (rg_db.systemGlobal.tcp_long_timeout != RG_INIT_DEFAULT_tcp_long_timeout) { //default : 3600
			PROC_PRINTF("echo %d > proc/rg/tcp_long_timeout\n", rg_db.systemGlobal.tcp_long_timeout);
		}

		if (rg_db.systemGlobal.tcp_short_timeout != RG_INIT_DEFAULT_tcp_short_timeout) { //default : 20
			PROC_PRINTF("echo %d > proc/rg/tcp_short_timeout\n", rg_db.systemGlobal.tcp_short_timeout);
		}

		if (rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies != RG_INIT_DEFAULT_tcp_short_timeout_housekeep_jiffies) {//current : 200
			PROC_PRINTF("echo %d > proc/rg/tcp_short_timeout_housekeep_jiffies\n", rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies);
		}

		/* not need trace filter, just for debugging purpose
			PROC_PRINTF("echo %d > proc/rg/trace_filter\n");
		*/

		/* just for special custom using, not general system usage for setting network interface
			PROC_PRINTF("echo %d > proc/rg/trap_lan_add_host\n");
			PROC_PRINTF("echo %d > proc/rg/trap_lan_del_host\n");
		*/

		PROC_PRINTF("# trap_lan_enable default 0:disable\n");
		if (rg_kernel.arp_number_for_LAN==0 && rg_kernel.arp_number_for_WAN==MAX_ARP_HW_TABLE_SIZE) {//current 0: disable
			PROC_PRINTF("echo 1 > proc/rg/trap_lan_enable\n");
		}else{
			PROC_PRINTF("echo 0 > proc/rg/trap_lan_enable\n");
		}


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		//apolloPro no need this proc to support avalanche
#else
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP] != RG_INIT_DEFAULT_trap_syn_and_disable_svlan) { //current disable:0
			PROC_PRINTF("echo %d > proc/rg/trap_syn_and_disable_svlan\n", 1);
		}
#endif
		if (rg_db.systemGlobal.arp_traffic_off != RG_INIT_DEFAULT_turn_off_arp_hw_traffic_info) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_arp_hw_traffic_info\n", rg_db.systemGlobal.arp_traffic_off);
		}

	#ifdef CONFIG_ROME_NAPT_SHORTCUT
		if (rg_db.systemGlobal.ipv4_shortcut_off != RG_INIT_DEFAULT_turn_off_ipv4_shortcut) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_ipv4_shortcut\n", rg_db.systemGlobal.ipv4_shortcut_off);
		}
		if (rg_db.systemGlobal.v4ShortCut_timeout != RTK_RG_DEFAULT_V4_SHORTCUT_TIMEOUT) {
			PROC_PRINTF("echo %d > proc/rg/ipv4_shortcut_timeout\n", rg_db.systemGlobal.v4ShortCut_timeout);
		}
	#endif

	#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
		if (rg_db.systemGlobal.ipv6_shortcut_off != RG_INIT_DEFAULT_turn_off_ipv6_shortcut) { //current : 0
			PROC_PRINTF("echo %d > proc/rg/turn_off_ipv6_shortcut\n", rg_db.systemGlobal.ipv6_shortcut_off);
		}
		if (rg_db.systemGlobal.v6ShortCut_timeout != RTK_RG_DEFAULT_V6_SHORTCUT_TIMEOUT) {
			PROC_PRINTF("echo %d > proc/rg/ipv6_shortcut_timeout\n", rg_db.systemGlobal.v6ShortCut_timeout);
		}
	#endif
	#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
		if (rg_db.systemGlobal.flow_timeout != RTK_RG_DEFAULT_FLOW_TIMEOUT) {
			PROC_PRINTF("echo %d > proc/rg/flow_timeout\n", rg_db.systemGlobal.flow_timeout);
		}
	#endif

		if (rg_db.systemGlobal.udp_long_timeout != RG_INIT_DEFAULT_udp_long_timeout) { //default : RTK_RG_DEFAULT_UDP_LONG_TIMEOUT/600
			PROC_PRINTF("echo %d > proc/rg/udp_long_timeout\n", rg_db.systemGlobal.udp_long_timeout);
		}

		if (rg_db.systemGlobal.udp_short_timeout != RG_INIT_DEFAULT_udp_short_timeout) { //default : RTK_RG_DEFAULT_UDP_SHORT_TIMEOUT/20
			PROC_PRINTF("echo %d > proc/rg/udp_short_timeout\n", rg_db.systemGlobal.udp_short_timeout);
		}

		if (rg_db.systemGlobal.unlearnedSALimit != RG_INIT_DEFAULT_unlearnedSA_rate_limit) { //current : RG_INIT_DEFAULT_unlearnedSA_rate_limit
			PROC_PRINTF("echo %d > proc/rg/unlearnedSA_rate_limit\n", rg_db.systemGlobal.unlearnedSALimit);
		}

		if (rg_db.systemGlobal.overMTURateLimitShareMeterIdx != RG_INIT_DEFAULT_overMTU_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/overMTU_rate_limit\n", rg_db.systemGlobal.overMTURateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.overMTURateLimitPortMask != RG_INIT_DEFAULT_overMTU_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo %d > proc/rg/overMTU_rate_limit_portMask\n", rg_db.systemGlobal.overMTURateLimitPortMask);
		}

		if (rg_db.systemGlobal.ArpReqRateLimitShareMeterIdx != RG_INIT_DEFAULT_ArpReq_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/ArpReq_rate_limit\n", rg_db.systemGlobal.ArpReqRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.ArpReqRateLimitPortMask != RG_INIT_DEFAULT_ArpReq_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo 0x%x > proc/rg/ArpReq_rate_limit_portMask\n", rg_db.systemGlobal.ArpReqRateLimitPortMask);
		}

		RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(i){
			if (i==0) {
				PROC_PRINTF("echo %d > proc/rg/dos_rate_limit\n", rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx);
				PROC_PRINTF("echo \"Portmask 0x%x Ctagif %d CtagVid %d DA %pM DIP %pI Tcp %d Length_start %d Length_end %d\" > proc/rg/dos_rate_limit_pattern\n",
							rg_db.systemGlobal.dosRateLimit[i].portmask,
							rg_db.systemGlobal.dosRateLimit[i].ctagif,
							rg_db.systemGlobal.dosRateLimit[i].ctagVid,
							rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet,
							rg_db.systemGlobal.dosRateLimit[i].ingressDip,
							rg_db.systemGlobal.dosRateLimit[i].isTcp,
							rg_db.systemGlobal.dosRateLimit[i].pktLenStart,
							rg_db.systemGlobal.dosRateLimit[i].pktLenEnd);
			}else{
				PROC_PRINTF("echo \"Rule %d MeterIdx %d\" > proc/rg/dos_rate_limit\n", i, rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx);
				PROC_PRINTF("echo \"Rule %d Portmask 0x%x Ctagif %d CtagVid %d DA %pM DIP %pI Tcp %d Length_start %d Length_end %d\" > proc/rg/dos_rate_limit_pattern\n",
							i,
							rg_db.systemGlobal.dosRateLimit[i].portmask,
							rg_db.systemGlobal.dosRateLimit[i].ctagif,
							rg_db.systemGlobal.dosRateLimit[i].ctagVid,
							rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet,
							rg_db.systemGlobal.dosRateLimit[i].ingressDip,
							rg_db.systemGlobal.dosRateLimit[i].isTcp,
							rg_db.systemGlobal.dosRateLimit[i].pktLenStart,
							rg_db.systemGlobal.dosRateLimit[i].pktLenEnd);
			}
		}

		if (rg_db.systemGlobal.igmpRateLimitShareMeterIdx != RG_INIT_DEFAULT_igmp_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/igmp_rate_limit\n", rg_db.systemGlobal.igmpRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.igmpRateLimitPortMask != RG_INIT_DEFAULT_igmp_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo 0x%x > proc/rg/igmp_rate_limit_portMask\n", rg_db.systemGlobal.igmpRateLimitPortMask);
		}

		if (rg_db.systemGlobal.dhcpRateLimitShareMeterIdx != RG_INIT_DEFAULT_dhcp_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/dhcp_rate_limit\n", rg_db.systemGlobal.dhcpRateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.dhcpRateLimitPortMask != RG_INIT_DEFAULT_dhcp_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo 0x%x > proc/rg/dhcp_rate_limit_portMask\n", rg_db.systemGlobal.dhcpRateLimitPortMask);
		}

		if (rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx != RG_INIT_DEFAULT_unknownDA_rate_limit) { //current : -1 / disable
			PROC_PRINTF("echo %d > proc/rg/unknownDA_rate_limit\n", rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx);
		}

		if (rg_db.systemGlobal.unKnownDARateLimitPortMask != RG_INIT_DEFAULT_unknownDA_rate_limit_portMask) { //current : 0x0
			PROC_PRINTF("echo %d > proc/rg/unknownDA_rate_limit_portMask\n", rg_db.systemGlobal.unKnownDARateLimitPortMask);
		}

		if (rg_db.systemGlobal.unknownDA_Trap_to_PS_enable != RG_INIT_DEFAULT_unknownDA_trap_to_PS) { //current : Disable
			PROC_PRINTF("echo %d > proc/rg/unknownDA_trap_to_PS\n", rg_db.systemGlobal.unknownDA_Trap_to_PS_enable);
		}

		if (rg_db.systemGlobal.bc_dip_with_non_bc_dmac_Trap_to_PS_enable != RG_INIT_DEFAULT_bc_dip_with_non_bc_dmac_Trap_to_PS) { //current : Enable
			PROC_PRINTF("echo %d > proc/rg/bc_dip_with_non_bc_dmac_trap_to_PS\n", rg_db.systemGlobal.bc_dip_with_non_bc_dmac_Trap_to_PS_enable);
		}

		if (rg_db.systemGlobal.urlFilterMode != RG_INIT_DEFAULT_urlFilter_mode) { //current : RG_FILTER_BLACK Mode.
			PROC_PRINTF("echo %d > proc/rg/urlFilter_mode\n", 1);
		}

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
		if (rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask != RG_INIT_DEFAULT_virtualMAC_with_PON) { //current : virtaulMAC with PON: 0/Turn Off.
			PROC_PRINTF("echo %d > proc/rg/virtualMAC_with_PON\n", rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask);
		}
#endif
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		for(i=0;i<MAX_WLAN_DEVICE_NUM;i++){
			if(rg_db.systemGlobal.wifiEgressRateLimitMeter[i]){
				PROC_PRINTF("echo %d %d > /proc/rg/wifi_egress_rate_limit\n",i,rg_db.systemGlobal.wifiEgressRateLimitMeter[i]);
			}
		}

		for(i=0;i<MAX_WLAN_DEVICE_NUM;i++){
			if(rg_db.systemGlobal.wifiIngressRateLimitMeter[i]){
				PROC_PRINTF("echo %d %d > /proc/rg/wifi_ingress_rate_limit\n",i,rg_db.systemGlobal.wifiIngressRateLimitMeter[i]);
			}
		}
#endif
		if (rg_db.systemGlobal.wifiTxRedirect != RG_INIT_DEFAULT_wifi_tx_redirect_to_port0) { // current: 0
			PROC_PRINTF("echo %d > proc/rg/wifi_tx_redirect_to_port0\n", rg_db.systemGlobal.wifiTxRedirect);
		}

		if(!(rg_kernel.lowerBoundPortUsedByPS==0 && rg_kernel.upperBoundPortUsedByPS==0)){
			PROC_PRINTF("echo %d %d > proc/rg/port_range_used_by_ps\n", rg_kernel.lowerBoundPortUsedByPS, rg_kernel.upperBoundPortUsedByPS);
		}

		if (rg_db.systemGlobal.log_rx_pcap != 0) {
			PROC_PRINTF("echo %d > proc/rg/log_rx_pcap\n", rg_db.systemGlobal.log_rx_pcap);
		}

		if (rg_db.systemGlobal.log_to_PS_pcap != 0) {
			PROC_PRINTF("echo %d > proc/rg/log_to_PS_pcap\n", rg_db.systemGlobal.log_to_PS_pcap);
		}

		if (rg_db.systemGlobal.localInNaptAddToShortcut != RTK_RG_DEFAULT_LOCAL_IN_NAPT_ADD_TO_SHORTCUT) {
			PROC_PRINTF("echo %d > proc/rg/local_in_napt_add_to_shortcut\n", rg_db.systemGlobal.localInNaptAddToShortcut);
		}

		if (rg_db.systemGlobal.whiteListState != RTK_RG_DEFAULT_WHITELIST_STATE) {
			PROC_PRINTF("echo %d > proc/rg/whiteListState\n", rg_db.systemGlobal.whiteListState);
		}
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if (rg_db.systemGlobal.forcedlyCFpattern1Override != RTK_RG_DEFAULT_CF1_OVERRIDE) {
			PROC_PRINTF("echo %d > proc/rg/forcedlyCFpattern1Override\n", rg_db.systemGlobal.forcedlyCFpattern1Override);
		}
		if (rg_db.systemGlobal.replaceHwFlow != RTK_RG_DEFAULT_REPLACE_HW_FLOW) {
			PROC_PRINTF("echo %d > proc/rg/replaceHwFlow\n", rg_db.systemGlobal.replaceHwFlow);
		}
        if (rg_db.systemGlobal.L2TcpUdpStatefulTracking != RG_STATEFUL_TRACKING_ENABLE) {
			PROC_PRINTF("echo %d > proc/rg/L2TcpUdpStatefulTracking\n", rg_db.systemGlobal.L2TcpUdpStatefulTracking);
		}
        if (rg_db.systemGlobal.L3TcpUdpStatefulTracking != RG_STATEFUL_TRACKING_ENABLE) {
			PROC_PRINTF("echo %d > proc/rg/L3TcpUdpStatefulTracking\n", rg_db.systemGlobal.L3TcpUdpStatefulTracking);
		}
		if (rg_db.systemGlobal.flow_not_update_in_real_time != 0) {
			PROC_PRINTF("echo %d > proc/rg/flow_not_update_in_real_time\n", rg_db.systemGlobal.flow_not_update_in_real_time);
		}
		if (rg_db.systemGlobal.flow_flush_disable_by_feature != 0) {
			PROC_PRINTF("echo 0x%x > proc/rg/flow_flush_disable\n", rg_db.systemGlobal.flow_flush_disable_by_feature);
		}
#else
		if (rg_db.systemGlobal.shortcut_flush_disable_by_feature != 0) {
			PROC_PRINTF("echo 0x%x > proc/rg/flush_shortcut_disable\n", rg_db.systemGlobal.shortcut_flush_disable_by_feature);
		}
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		if (rg_kernel.block_communication_between_internet_and_other != 0) {
			PROC_PRINTF("echo %d > proc/rg/block_communication_between_internet_and_other\n", rg_kernel.block_communication_between_internet_and_other);
		}
#endif
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
		if (rg_db.systemGlobal.trap_routing_wan_by_acl != 0) {
			PROC_PRINTF("echo %d > proc/rg/trap_routing_wan_ip_by_acl\n", rg_db.systemGlobal.trap_routing_wan_by_acl);
		}
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if (rg_db.systemGlobal.icmp_trap_by_acl_disable != 0) {
			PROC_PRINTF("echo %d > proc/rg/icmp_trap_by_acl_disable\n", rg_db.systemGlobal.icmp_trap_by_acl_disable);
		}
#endif
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if (rg_db.systemGlobal.disableWifiRxDistributed){
			PROC_PRINTF("echo %d > proc/rg/disableWifiRxDistributed\n", rg_db.systemGlobal.disableWifiRxDistributed);
		}
		if (rg_db.systemGlobal.disableWifiTxDistributed){
			PROC_PRINTF("echo %d > proc/rg/disableWifiTxDistributed\n", rg_db.systemGlobal.disableWifiTxDistributed);
		}
		if (rg_db.systemGlobal.disableWifiRxAcc != 0) {
			PROC_PRINTF("echo %d > proc/rg/disableWifiRxAcceleration\n", rg_db.systemGlobal.disableWifiRxAcc);
		}
#if defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
		if (rg_db.systemGlobal.disableSlaveWifiRxAcc != 0) {
			PROC_PRINTF("echo %d > proc/rg/disableSlaveWifiRxAcceleration\n", rg_db.systemGlobal.disableSlaveWifiRxAcc);
		}
		if(rg_db.systemGlobal.disableWifiRxHwlookupHash != 0) {
			PROC_PRINTF("echo %d > proc/rg/disableMasterWifiRxHwlookupHash\n", rg_db.systemGlobal.disableWifiRxHwlookupHash);
		}
#else
		if (rg_kernel.disableSlaveWifiRxAcc_and_enableForwardHash != 0) {
			PROC_PRINTF("echo %d > proc/rg/disableSlaveWifiRxAcceleration_and_enableForwardHash\n", rg_kernel.disableSlaveWifiRxAcc_and_enableForwardHash);
		}
#endif
#endif
#endif
		if (rg_db.systemGlobal.naptAccessLimitNumber >= 0) {
			PROC_PRINTF("echo %d > /proc/rg/napt_access_limit_number\n", rg_db.systemGlobal.naptAccessLimitNumber);
		}

		for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
		{
			if(rg_db.systemGlobal.wanVlanMemAppend[i].portmask)
				PROC_PRINTF("echo %d 0x%x > /proc/rg/wanVlanMemberAppend\n",i,rg_db.systemGlobal.wanVlanMemAppend[i].portmask);
		}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)	//backward compatible for trap with priority related feature on 9607C
		if(rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_arp_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.loop_detect_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_loop_detect_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.loop_detect_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.snmp_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_snmp_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.snmp_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.igmp_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_igmp_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.igmp_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_dhcp_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.dhcpv6_packet_assign_priority.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo %d > /proc/rg/assign_dhcpv6_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.dhcpv6_packet_assign_priority.priority);
		}

		if(rg_db.systemGlobal.aclAndCfReservedRule.accessIP_packet_assign_priority_and_addr.user_acl_idx>0) //default : Disable
		{
			PROC_PRINTF("echo 0x%x > /proc/rg/assign_access_ip\n", rg_db.systemGlobal.aclAndCfReservedRule.accessIP_packet_assign_priority_and_addr.ip_addr);
			PROC_PRINTF("echo %d > /proc/rg/assign_access_ip_priority\n", rg_db.systemGlobal.aclAndCfReservedRule.accessIP_packet_assign_priority_and_addr.priority);
		}
#endif
#endif // end CONFIG_RG_FLOW_BASED_PLATFORM

#if defined(CONFIG_RG_RTL9607C_SERIES)
		if (rg_db.systemGlobal.downstream_rate_limit_by_shaper!=RTK_RG_DISABLED) {
			PROC_PRINTF("echo %d > proc/rg/downstream_rate_limit_by_shaper\n", rg_db.systemGlobal.downstream_rate_limit_by_shaper);
		}
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if(rg_db.systemGlobal.synRateLimitShareMeterIdx != -1) {//default : Disable
			PROC_PRINTF("echo %d > proc/rg/assign_syn_share_meter\n", rg_db.systemGlobal.aclAndCfReservedRule.syn_packet_assign_share_meter.share_meter);
		}
		if(rg_db.systemGlobal.flow_based_cpri_decision_portmask){
			PROC_PRINTF("echo %d > proc/rg/flow_based_cpri_decision_portmask\n", rg_db.systemGlobal.flow_based_cpri_decision_portmask);
		}
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if (rg_db.systemGlobal.prevent_control_packet_drop != RTK_RG_DISABLED) {
			PROC_PRINTF("echo %d > /proc/rg/prevent_control_packet_drop\n", rg_db.systemGlobal.prevent_control_packet_drop);
		}
		if (rg_db.systemGlobal.tunnelHwAcceleration_enable != RTK_RG_DISABLED) {
			PROC_PRINTF("echo %d > /proc/rg/tunnelHwAcceleration_enable\n", rg_db.systemGlobal.tunnelHwAcceleration_enable);
		}
		if (rg_db.systemGlobal.dsliteHwAcceleration_disable != RTK_RG_DISABLED) {
			PROC_PRINTF("echo %d > /proc/rg/dsliteHwAcceleration_disable\n", rg_db.systemGlobal.dsliteHwAcceleration_disable);
		}
#endif
		if (rg_db.systemGlobal.trapICMPWhenNeighMiss != RG_INIT_DEFAULT_trapICMPWhenNeighMiss) { //0x0 disable
			PROC_PRINTF("echo %d > /proc/rg/trapICMPWhenNeighMiss\n", rg_db.systemGlobal.trapICMPWhenNeighMiss);
		}

		if (rg_db.systemGlobal.forcedlyTrapICMPv6Echo != RG_INIT_DEFAULT_forcedlyTrapICMPv6Echo) { //0x0 disable
			PROC_PRINTF("echo %d > /proc/rg/forcedlyTrapICMPv6Echo\n", rg_db.systemGlobal.forcedlyTrapICMPv6Echo);
		}

		if (rg_db.systemGlobal.forcedlyDropICMPv6RSFromWAN != RG_INIT_DEFAULT_forcedlyDropICMPv6RSFromWAN) { //0x0 disable
			PROC_PRINTF("echo %d > /proc/rg/forcedlyDropICMPv6RSFromWAN\n", rg_db.systemGlobal.forcedlyDropICMPv6RSFromWAN);
		}

		if(rg_db.systemGlobal.dnf_game_mechanism_local_port!=0 && rg_db.systemGlobal.dnf_game_mechanism_remote_port!=0){
			PROC_PRINTF("echo %d %d > /proc/rg/dnf_game_mechanism\n", rg_db.systemGlobal.dnf_game_mechanism_local_port, rg_db.systemGlobal.dnf_game_mechanism_remote_port);
		}
#if defined(CONFIG_RG_RTL9607C_SERIES)
		if (rg_db.systemGlobal.disable_lanIntf_overMtu_trap != 0) {
			PROC_PRINTF("echo %d > /proc/rg/disable_lanIntf_overMtu_trap\n", rg_db.systemGlobal.disable_lanIntf_overMtu_trap);
		}
#endif
#if	defined(CONFIG_RG_NAPT_DMZ_SUPPORT)
		if (rg_db.systemGlobal.enhanced_dmz != 0) {
			PROC_PRINTF("echo %d > /proc/rg/enhanced_dmz\n", rg_db.systemGlobal.enhanced_dmz);
		}
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if (rg_db.systemGlobal.phy2ExtPort_mappingMsk != 0) {
			PROC_PRINTF("echo 0x%x > /proc/rg/phyPort_to_extPort_mappingMsk\n", rg_db.systemGlobal.phy2ExtPort_mappingMsk);
		}
#endif

		if (rg_db_dynamic_sram.nptv6_acc.nptv6_acceleration_mechanism != RTK_RG_DISABLED) {
			PROC_PRINTF("echo %d > /proc/rg/nptv6_acceleration_mechanism\n", rg_db_dynamic_sram.nptv6_acc.nptv6_acceleration_mechanism);
		}

		if (rg_db.systemGlobal.forciblyMacBindingToWan != RTK_RG_DISABLED) {
			PROC_PRINTF("# forcibly mac binding:\n");
			PROC_PRINTF("echo %d > /proc/rg/forciblyMacBindingToWan\n", rg_db.systemGlobal.forciblyMacBindingWanIdx);
			PROC_PRINTF("echo 0x%x > /proc/rg/forciblyMacBindingToWanMode\n", rg_db.systemGlobal.forciblyMacBindingToWanModeMask);
			PROC_PRINTF("echo %d > /proc/rg/forciblyMacBindingToWanV4OptLen\n", rg_db.systemGlobal.forciblyMacBinding_dhcpV4Opt60_len);
			PROC_PRINTF("echo 0x%x > /proc/rg/forciblyMacBindingToWanV4OptLenOp\n", rg_db.systemGlobal.forciblyMacBinding_dhcpV4Opt60_opcode);
			PROC_PRINTF("echo %d > /proc/rg/forciblyMacBindingToWanV6OptLen\n", rg_db.systemGlobal.forciblyMacBinding_dhcpV6Opt16_len);
			PROC_PRINTF("echo 0x%x > /proc/rg/forciblyMacBindingToWanV6OptLenOp\n", rg_db.systemGlobal.forciblyMacBinding_dhcpV6Opt16_opcode);
			if(rg_db.systemGlobal.forciblyMacBindingToWanKeyMatch)
				PROC_PRINTF("echo %s > /proc/rg/forciblyMacBindingToWanKey\n", rg_db.systemGlobal.forciblyMacBinding_key);
		}
		
		/* only dump usage desc.
		 * reference diag rg init callback default .., please
			PROC_PRINTF("echo %d > proc/rg/wmux_add\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_del\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_flag\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_info\n");
			PROC_PRINTF("echo %d > proc/rg/wmux_init\n");
		*/
	}
	return len;
}

int32 mibdump_accessWanStatistics(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_UNLIMIT)
	{
		PROC_PRINTF(">>access WAN limit statistics:Unlimit\n");
	}
	else
	{
		if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
		{
			PROC_PRINTF(">>access WAN limit statistics:PortMask\n");
			//portmask
			PROC_PRINTF("  portmask 0x%x",rg_db.systemGlobal.accessWanLimitPortMask_member.portmask);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			PROC_PRINTF(" wlan0devmask 0x%x",rg_db.systemGlobal.accessWanLimitPortMask_wlan0member);
#endif
			if(rg_db.systemGlobal.accessWanLimitPortMask>=0)
				{PROC_PRINTF("\n  Limit is %04d",rg_db.systemGlobal.accessWanLimitPortMask);}
			else
				{PROC_PRINTF("\n  Unlimited");}
			PROC_PRINTF(", count is %04d, limitReachAction is %s, limitField:%s\n",
				atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount),
				rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2",
				rg_db.systemGlobal.activeLimitField==RG_ACCESSWAN_LIMIT_BY_SMAC?"By SMAC":"By SIP");
		}
		else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_CATEGORY)
		{
			PROC_PRINTF(">>access WAN limit statistics:Category\n");
			//category
			for(i=0;i<WanAccessCategoryNum;i++)
			{
				PROC_PRINTF("  Category[%d] ",i);
				if(rg_db.systemGlobal.accessWanLimitCategory[i]>=0)
					{PROC_PRINTF("Limit is %04d",rg_db.systemGlobal.accessWanLimitCategory[i]);}
				else
					{PROC_PRINTF("Unlimited");}
				PROC_PRINTF(", count is %04d, limitReachAction is %s\n",
					atomic_read(&rg_db.systemGlobal.accessWanLimitCategoryCount[i]),
					rg_db.systemGlobal.accessWanLimitCategoryAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
					rg_db.systemGlobal.accessWanLimitCategoryAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
			}
		}
		else
		{
			PROC_PRINTF(">>access WAN limit statistics:Port\n");
			PROC_PRINTF("Please refer to /proc/dump/sw_layer2LearningStatistics for more information.\n");
		}
	}

	return len;
}

int32 mibdump_groupMacStatistics(struct seq_file *s, void *v)
{
	int len=0;

	if(rg_db.systemGlobal.groupMACLimit>=0){
		PROC_PRINTF(">>group MAC limit statistics:\n");
		PROC_PRINTF(" portmask 0x%x",rg_db.systemGlobal.groupMACLimit_member.portmask);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		PROC_PRINTF(" wlan0devmask 0x%x",rg_db.systemGlobal.groupMACLimit_wlan0member);
#endif
		if(rg_db.systemGlobal.groupMACLimit>=0)
			{PROC_PRINTF("\n  Limit is %04d",rg_db.systemGlobal.groupMACLimit);}
		else
			{PROC_PRINTF("\n  Unlimited");}
		PROC_PRINTF(", count is %04d, limitReachAction is DROP\n",
			atomic_read(&rg_db.systemGlobal.groupMACLimitCount));
	}else{
		PROC_PRINTF(">>group MAC limit statistics:Unlimit\n");
	}
	return len;
}

int32 mibdump_vlanGroupMacLimitStatistics(struct seq_file *s, void *v)
{
	int i,j,k,len=0;
	PROC_PRINTF(">>vlan group MAC limit statistics:\n");
	//per port, per group information, showing all learned MAC if avalable
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		PROC_PRINTF("PORT[%d]:\n",i);
		for(j=0;j<MAX_VLAN_GROUP_MAC_LIMIT_NUMBER;j++)
		{
			if(rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.valid && rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.port==i)
			{
				PROC_PRINTF("  Group Index[%d] Count:%d Limit:%d\n",j,atomic_read(&rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.mac_count),rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.mac_limit_number);

				if(rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.untag)
				{
					PROC_PRINTF("    Untag MAC:\n");
					if(!list_empty(&rg_db.systemGlobal.vlanGroupMACLimit_group[j].mac_head))
					{
						int cnt=1;
						rtk_rg_vlanGroupMacLimit_mac_t *pMacEntry;
						list_for_each_entry(pMacEntry, &rg_db.systemGlobal.vlanGroupMACLimit_group[j].mac_head, group_list)
						{
							if(pMacEntry->vlanId==-1)
							{
								PROC_PRINTF("        %d. %02x:%02x:%02x:%02x:%02x:%02x\n",cnt++,
									pMacEntry->mac.octet[0],pMacEntry->mac.octet[1],pMacEntry->mac.octet[2],
									pMacEntry->mac.octet[3],pMacEntry->mac.octet[4],pMacEntry->mac.octet[5]);
							}
						}
					}
				}
				for(k=0;k<MAX_VLAN_HW_TABLE_SIZE;k++)
				{
					if(rg_test_bit(k&0x1f,(void *)&rg_db.systemGlobal.vlanGroupMACLimit_group[j].group_info.vlanMask[k>>5]))
					{
						PROC_PRINTF("    Vlan %d MAC:\n",k);
						if(!list_empty(&rg_db.systemGlobal.vlanGroupMACLimit_group[j].mac_head))
						{
							int cnt=1;
							rtk_rg_vlanGroupMacLimit_mac_t *pMacEntry;
							list_for_each_entry(pMacEntry, &rg_db.systemGlobal.vlanGroupMACLimit_group[j].mac_head, group_list)
							{
								if(pMacEntry->vlanId==k)
								{
									PROC_PRINTF("        %d. %02x:%02x:%02x:%02x:%02x:%02x\n",cnt++,
										pMacEntry->mac.octet[0],pMacEntry->mac.octet[1],pMacEntry->mac.octet[2],
										pMacEntry->mac.octet[3],pMacEntry->mac.octet[4],pMacEntry->mac.octet[5]);
								}
							}
						}
					}
				}
			}
		}
	}

	return len;
}

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
int32 mibdump_ipv6_stateful_connections(struct seq_file *s, void *v)
{
	int i,_count;
	rtk_rg_ipv6_layer4_linkList_t *pLayer4List;
	int len=0;

	PROC_PRINTF(">>IPv6 Stateful connections:\n");
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
	{
		if(!list_empty(&rg_db.ipv6Layer4HashListHead[i]))
		{
			_count=0;
			list_for_each_entry(pLayer4List,&rg_db.ipv6Layer4HashListHead[i],layer4_list)
			{
				if(pLayer4List->isFrag)continue;
				if(_count==0)PROC_PRINTF("[%d]========================================================\n",i);
				if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND || pLayer4List->direction==IPV6_ROUTE_OUTBOUND)
					PROC_PRINTF("   OutList");
				else
					PROC_PRINTF("   InList ");

				PROC_PRINTF("[%p]:(pair[%p]) %s ",pLayer4List,pLayer4List->pPair_list,pLayer4List->isTCP?"TCP":"UDP");

				switch(pLayer4List->state)
				{
					case INVALID:
						PROC_PRINTF("state:INVALID ");
						break;
					case SYN_RECV:
						PROC_PRINTF("state:SYN_RECV ");
						break;
					case UDP_FIRST:
						PROC_PRINTF("state:UDP_FIRST ");
						break;
					case SYN_ACK_RECV:
						PROC_PRINTF("state:SYN_ACK_RECV ");
						break;
					case UDP_SECOND:
						PROC_PRINTF("state:UDP_SECOND ");
						break;
					case TCP_CONNECTED:
						PROC_PRINTF("state:TCP_CONNECTED ");
						break;
					case UDP_CONNECTED:
						PROC_PRINTF("state:UDP_CONNECTED ");
						break;
					case FIRST_FIN:
						PROC_PRINTF("state:FIRST_FIN ");
						break;
					case RST_RECV:
						PROC_PRINTF("state:RST_RECV ");
						break;
					default:
						break;
				}

				PROC_PRINTF("idle:%d\n     %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d --> \n     %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d\n",
					pLayer4List->idleSecs,
					pLayer4List->srcIP.ipv6_addr[0],pLayer4List->srcIP.ipv6_addr[1],pLayer4List->srcIP.ipv6_addr[2],pLayer4List->srcIP.ipv6_addr[3],
					pLayer4List->srcIP.ipv6_addr[4],pLayer4List->srcIP.ipv6_addr[5],pLayer4List->srcIP.ipv6_addr[6],pLayer4List->srcIP.ipv6_addr[7],
					pLayer4List->srcIP.ipv6_addr[8],pLayer4List->srcIP.ipv6_addr[9],pLayer4List->srcIP.ipv6_addr[10],pLayer4List->srcIP.ipv6_addr[11],
					pLayer4List->srcIP.ipv6_addr[12],pLayer4List->srcIP.ipv6_addr[13],pLayer4List->srcIP.ipv6_addr[14],pLayer4List->srcIP.ipv6_addr[15],
					pLayer4List->srcPort,
					pLayer4List->destIP.ipv6_addr[0],pLayer4List->destIP.ipv6_addr[1],pLayer4List->destIP.ipv6_addr[2],pLayer4List->destIP.ipv6_addr[3],
					pLayer4List->destIP.ipv6_addr[4],pLayer4List->destIP.ipv6_addr[5],pLayer4List->destIP.ipv6_addr[6],pLayer4List->destIP.ipv6_addr[7],
					pLayer4List->destIP.ipv6_addr[8],pLayer4List->destIP.ipv6_addr[9],pLayer4List->destIP.ipv6_addr[10],pLayer4List->destIP.ipv6_addr[11],
					pLayer4List->destIP.ipv6_addr[12],pLayer4List->destIP.ipv6_addr[13],pLayer4List->destIP.ipv6_addr[14],pLayer4List->destIP.ipv6_addr[15],
					pLayer4List->destPort);

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
					if(pLayer4List->direction==NAPT_DIRECTION_OUTBOUND){
						PROC_PRINTF("     (ExtIp:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ExtPort:%d)\n",
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[0],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[1],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[2],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[3],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[4],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[5],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[6],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[7],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[8],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[9],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[10],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[11],
							rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[12],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[13],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[14],rg_db.v6Extip[pLayer4List->extipIdx].externalIp.ipv6_addr[15],
							pLayer4List->externalPort);

					}else if(pLayer4List->direction==NAPT_DIRECTION_INBOUND){
						PROC_PRINTF("     (IntIp:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x IntPort:%d)\n",
							pLayer4List->internalIP.ipv6_addr[0],pLayer4List->internalIP.ipv6_addr[1],pLayer4List->internalIP.ipv6_addr[2],pLayer4List->internalIP.ipv6_addr[3],
							pLayer4List->internalIP.ipv6_addr[4],pLayer4List->internalIP.ipv6_addr[5],pLayer4List->internalIP.ipv6_addr[6],pLayer4List->internalIP.ipv6_addr[7],
							pLayer4List->internalIP.ipv6_addr[8],pLayer4List->internalIP.ipv6_addr[9],pLayer4List->internalIP.ipv6_addr[10],pLayer4List->internalIP.ipv6_addr[11],
							pLayer4List->internalIP.ipv6_addr[12],pLayer4List->internalIP.ipv6_addr[13],pLayer4List->internalIP.ipv6_addr[14],pLayer4List->internalIP.ipv6_addr[15],
							pLayer4List->internalPort);
					}
#endif
				_count++;
			}
			if(_count!=0)PROC_PRINTF("[%d] has %d Connections.\n",i,_count);
		}

	}

	PROC_PRINTF(">>Overall System Connections:%d\n",atomic_read(&rg_db.systemGlobal.v6StatefulConnectionNum));

	return len;
}
#endif

#ifdef CONFIG_APOLLO_ROMEDRIVER
int32 mibdump_alg_dynamic_port(struct seq_file *s, void *v)
{
	rtk_rg_alg_dynamicPort_t *pList;
	int len=0;

	PROC_PRINTF(">>ALG Dynamic Port:\n");
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.algDynamicLock);
	if(!list_empty(&rg_db.algDynamicCheckListHead))
	{
		list_for_each_entry(pList,&rg_db.algDynamicCheckListHead,alg_list)
		{
			if(pList->serverInLan)
				{PROC_PRINTF("    Port[%d] %s ServerInLan(intIP:%x) timeout:%d\n",
					pList->portNum,pList->isTCP?"TCP":"UDP",pList->intIP,pList->timeout);}
			else
				{PROC_PRINTF("    Port[%d] %s ServerInWan timeout:%d\n",
					pList->portNum,pList->isTCP?"TCP":"UDP",pList->timeout);}
		}
	}
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.algDynamicLock);

	return len;
}

int32 mibdump_alg_preserve_port(struct seq_file *s, void *v)
{
	rtk_rg_alg_preserveExtPort_t *pList;
	int len=0;

	PROC_PRINTF(">>ALG Preserve Port:\n");
	if(!list_empty(&rg_db.algPreExtListHead))
	{
		list_for_each_entry(pList,&rg_db.algPreExtListHead,alg_list)
		{
			PROC_PRINTF("    Port[%d] %s timeout:%d\n",	pList->portNum,pList->isTCP?"TCP":"UDP",pList->timeout);
		}
	}

	return len;
}

int32 mibdump_alg_srvInLan_ip(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	PROC_PRINTF(">>ALG Server In Lan Internal IP:\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress)
			PROC_PRINTF("  algType=0x%06x : IP=%s\n",rg_db.algServInLanIpMapping[i].algType,	inet_ntoa(rg_db.algServInLanIpMapping[i].serverAddress));
	}

	return len;
}
#endif

#if defined(CONFIG_RG_RTL9602C_SERIES)

int32 dump_ipmc_sipFilter(struct seq_file *s, void *v)
{
	int i;
	int len=0;

    PROC_PRINTF("IDX\tSIP\t\tipFilterRefCount\n");

	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		if(rg_db.ipmcfilter[i].valid==TRUE)
		{
			PROC_PRINTF("[%02d]\t%d.%d.%d.%d\t%d\n",i,(rg_db.ipmcfilter[i].filterIp>>24)&0xff
				,(rg_db.ipmcfilter[i].filterIp>>16)&0xff
				,(rg_db.ipmcfilter[i].filterIp>>8)&0xff
				,(rg_db.ipmcfilter[i].filterIp)&0xff
				,rg_db.ipmcfilter[i].ipFilterRefCount);
		}
	}

    return len;
}
#endif

int32 dump_ipmc_group(struct seq_file *s, void *v)
{
	int i;
	int len=0;

    PROC_PRINTF("IDX\tGIP\t\tPMSK\tipmRefCount\n");

	for(i=0;i<MAX_IPMCGRP_SW_TABLE_SIZE;i++)
	{
		if(rg_db.ipmcgrp[i].valid==TRUE)
		{
			PROC_PRINTF("[%02d] \t%d.%d.%d.%d\t0x%x\t%d\n",i,(rg_db.ipmcgrp[i].groupIp>>24)&0xff
				,(rg_db.ipmcgrp[i].groupIp>>16)&0xff
				,(rg_db.ipmcgrp[i].groupIp>>8)&0xff
				,(rg_db.ipmcgrp[i].groupIp)&0xff
				,rg_db.ipmcgrp[i].portMsk.bits[0],rg_db.ipmcgrp[i].ipmRefCount);
		}
	}

    return len;
}
int32 dump_ipmc_routing(struct seq_file *s, void *v)
{
	int i,ret;
	rtk_l34_ipmcTrans_entry_t ipmc;
	int len=0;

	PROC_PRINTF("Idx\tNetif\tSIP_T_EN\tEXTIP_IDX\tPPPOE_ACT\tPPPOE_IDX\n");

	for(i=0; i<16; i++)
	{
		ret=rtk_l34_ipmcTransTable_get(i,&ipmc);
		if(ret != RT_ERR_OK) continue;
//		PROC_PRINTF("%02d\t%d\t%d\t\t%02d\t\t%d\t\t%d\n",i,ipmc.netifIdx,ipmc.sipTransEnable,ipmc.extipIdx,ipmc.isPppoeIf,ipmc.pppoeIdx);
		PROC_PRINTF("[%02d]\t%d\t%d\t\t%02d\t\t%d\t\t%d\n",i,ipmc.netifIdx,ipmc.sipTransEnable,ipmc.extipIdx,ipmc.pppoeAct,ipmc.pppoeIdx);
#if defined(CONFIG_RG_RTL9602C_SERIES)
		PROC_PRINTF("\tUntagSetPortMask =%x \n",ipmc.untagMbr.bits[0]);
#endif
	}
    return len;
}
int32 dump_rg_vlan_table(struct seq_file *s, void *v)
{

	int i,j;
	int len=0;

	PROC_PRINTF(">>RG VLAN Table:\n");

	for(i=0; i<MAX_VLAN_SW_TABLE_SIZE; i++)
	{
		// clean up
		if(!rg_db.vlan[i].valid) continue;	// ignore unconfigured entry

		PROC_PRINTF(" -- VID[%d] --", i);
		PROC_PRINTF("\n\tMember Ports:0x%x",rg_db.vlan[i].MemberPortmask.bits[0]);
		PROC_PRINTF("\n\tExtension Member Ports:0x%x",rg_db.vlan[i].Ext_portmask.bits[0]);
		PROC_PRINTF("\n\tUntag Member Ports:0x%x",rg_db.vlan[i].UntagPortmask.bits[0]);
		PROC_PRINTF("\n\tFID: %d,\tIVL_SVL: %s\n", rg_db.vlan[i].fid, (rg_db.vlan[i].fidMode==VLAN_FID_IVL)?"IVL":"SVL");    //0:SVL, 1:IVL but VLAN_FID_IVL = 0, VLAN_FID_SVL = 1
		PROC_PRINTF("\tBased Priority: %s, %d\n", (rg_db.vlan[i].priorityEn==ENABLED)?"enable":"disable", rg_db.vlan[i].priority);
		PROC_PRINTF("\tExtension Ports: ");
		if(rg_db.vlan[i].Ext_portmask.bits[0]) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(RTK_RG_MAX_EXT_PORT); j++)
			{
				if(rg_db.vlan[i].Ext_portmask.bits[0] & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		PROC_PRINTF("\tWlan0 Devices: ");
		if(rg_db.vlan[i].wlan0DevMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0DevMask & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
		PROC_PRINTF("\tWlan0 Untag: ");
		if(rg_db.vlan[i].wlan0UntagMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0UntagMask & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
#endif
	}

	//Display Port-based VLAN ID information
	PROC_PRINTF("\n>>RG Port-based VLAN settings:\n");
#if defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("PORT:\t      0       1       2       3    7PON 0x10CPU 0x12CPU 0x13CPU 0x14CPU 0x15CPU 0x16CPU 0x17CPU\nVID:\t");
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		if(RG_INVALID_MAC_PORT(i)) continue;
		PROC_PRINTF("%7d ",rg_db.systemGlobal.portBasedVID[i]);
	}

	PROC_PRINTF("\nEXTPORT:   0    1    2    3\nVID:\t");
	for(i=0;i<RTK_RG_MAX_EXT_PORT;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_BASED_PORT+i]);
	}
	PROC_PRINTF("\n");
#elif defined(CONFIG_RG_RTL9607C_SERIES)
	PROC_PRINTF("PORT:\t    0     1     2     3     4  5PON 6iNIC 7sCPU  8MII 9CPU0 10CPU1\nVID:\t");
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		PROC_PRINTF("%5d ",rg_db.systemGlobal.portBasedVID[i]);
	}

	PROC_PRINTF("\nEXTPORT: 7-0  7-1  7-2  7-3  7-4  7-5  9-0  9-1  9-2  9-3  9-4  9-5 10-0 10-1 10-2 10-3 10-4 10-5\nVID:\t");
	for(i=0;i<RTK_RG_MAX_EXT_PORT;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_BASED_PORT+i]);
	}
	PROC_PRINTF("\n");
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
	PROC_PRINTF("PORT:\t    0     1     2     3     4PON 5CPU\nVID:\t");
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		PROC_PRINTF("%5d ",rg_db.systemGlobal.portBasedVID[i]);
	}

	PROC_PRINTF("\nEXTPORT: 0   1   2   3   4   5\nVID:\t");
	for(i=0;i<RTK_RG_MAX_EXT_PORT;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_BASED_PORT+i]);
	}
	PROC_PRINTF("\n");
#elif defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("PORT:\t   0	1	 2PON 3CPU 4EX0 5EX1	6	7	8	 9\nVID:\t");

	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[i]);
	}

	for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_BASED_PORT+i]);
	}
	PROC_PRINTF("\n");
#elif defined(CONFIG_RG_RTL9600_SERIES)
	PROC_PRINTF("PORT:\t   0    1    2    3 4PON 5MII 6CPU 7EX0 8EX1    9   10   11\nVID:\t");

	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[i]);
	}

	for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		PROC_PRINTF("%4d ",rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_BASED_PORT+i]);
	}
	PROC_PRINTF("\n");
#endif

	//Display Port-and-protocol-based VLAN ID information
	PROC_PRINTF("\n>>RG Port-and-Protocol-based VLAN settings:\n");

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++) 	//port
	{
		int firstDump=TRUE;
		for(j=0; j<MAX_PORT_PROTO_GROUP_SIZE; j++)
		{
			if(rg_db.systemGlobal.protoBasedVID[i].protoVLANCfg[j].valid)
			{
				if(firstDump==TRUE) {PROC_PRINTF("  Port %d:\n",i); firstDump=FALSE; }
				switch(rg_db.systemGlobal.protoGroup[j].frametype)
				{
					case FRAME_TYPE_ETHERNET:
						PROC_PRINTF("    Group[%d] Ethertype:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_RFC1042:
						PROC_PRINTF("    Group[%d] frame_type:RFC1042 frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_SNAP8021H:
						PROC_PRINTF("    Group[%d] frame_type:SNAP8021H frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_SNAPOTHER:
						PROC_PRINTF("    Group[%d] frame_type:SNAPOTHER frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					case FRAME_TYPE_LLCOTHER:
						PROC_PRINTF("    Group[%d] frame_type:LLCOTHER frame_value:0x%04x",j,rg_db.systemGlobal.protoGroup[j].framevalue);
						break;
					default:
						break;
				}
				//PROC_PRINTF("\tVID:%d PRI:%d CFI:%d\n",ppbCfg.vid,ppbCfg.pri,ppbCfg.dei);
				PROC_PRINTF(" VID[%d]\n",rg_db.systemGlobal.protoBasedVID[i].protoVLANCfg[j].vid);
			}
		}
	}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Display WLAN0-device-based VLAN ID information
	PROC_PRINTF("\n>>WLAN0-device-based VLAN settings:\n");

	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("DEV[%d] DVID=%d\n",i,rg_db.systemGlobal.wlan0DeviceBasedVID[i]);
		}
	}
#endif


    return len;
}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
int32 dump_vlan_table(struct seq_file *s, void *v)
{

	int i,j,pvid;
	rtk_portmask_t mbr, untag, ext;
	rtk_fid_t fid;
	rtk_fidMode_t ivlsvl;
	rtk_pri_t pri;
	rtk_enable_t en;
	rtk_vlan_protoVlanCfg_t ppbCfg;
	rtk_vlan_protoGroup_t protoGroup;
	int len=0;

	PROC_PRINTF(">>ASIC VLAN Table:\n");

	for(i=0; i<MAX_VLAN_HW_TABLE_SIZE; i++)
	{
		// clean up
		mbr.bits[0] = 0;
		untag.bits[0] = 0;
		ext.bits[0] = 0;
		fid = 0;
		ivlsvl = 0;
		pri = 0;


		rtk_vlan_port_get(i, &mbr, &untag);
		if(mbr.bits[0]==0) continue;	// ignore unconfigured entry

		PROC_PRINTF(" -- VID[%d] --", i);
	#if 1
		PROC_PRINTF("\n\tMember Ports:0x%x",mbr.bits[0]);
	#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		rtk_vlan_extPortmaskIndex_get(i, &j);
		rtk_vlan_extPortmaskCfg_get(j, &mbr);
	#else
		rtk_vlan_extPort_get(i, &mbr);
	#endif
		PROC_PRINTF("\n\tExtension Member Ports:0x%x",mbr.bits[0]);
		PROC_PRINTF("\n\tUntag Member Ports:0x%x",untag.bits[0]);

	#else
		PROC_PRINTF("\n\tMember Ports: ");
		if(mbr.bits[0]==0) PROC_PRINTF("X");
		else
		{
			for(j=0; j<(MAX_APOLLO_PORT); j++)
			{
				if(mbr.bits[0] & (1<<j))  PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n\tUntag Member Ports: ");
		if(untag.bits[0]==0) PROC_PRINTF("X");
		else
		{
			for(j=0; j<(MAX_APOLLO_PORT); j++)
			{
				if(untag.bits[0] & (1<<j))	 PROC_PRINTF("%d ", j);
			}
		}
	#endif
	#ifdef CONFIG_RG_RTL9602C_SERIES
		rtk_vlan_lutSvlanHashState_get(i, &en);
		PROC_PRINTF("\n\tSVLAN_CHK_IVL_SVL: %s", (en==ENABLED)?"Force SVLAN":"Not Force SVLAN");
	#endif
		rtk_vlan_fid_get(i, &fid);
		rtk_vlan_fidMode_get(i, &ivlsvl);
		PROC_PRINTF("\n\tFID: %d,\tIVL_SVL: %s\n", fid, (ivlsvl==VLAN_FID_IVL)?"IVL":"SVL");    //0:SVL, 1:IVL but VLAN_FID_IVL = 0, VLAN_FID_SVL = 1
		rtk_vlan_priority_get(i, &pri);
		rtk_vlan_priorityEnable_get(i, &en);
		PROC_PRINTF("\tBased Priority: %s, %d\n", (en==ENABLED)?"enable":"disable", pri);
	#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		rtk_vlan_extPortmaskIndex_get(i, &j);
		PROC_PRINTF("\textpmask idx = %d",j);
		rtk_vlan_extPortmaskCfg_get(j, &ext);
	#else
		rtk_vlan_extPort_get(i, &ext);
	#endif
		PROC_PRINTF("\tExtension Ports: ");
		if(ext.bits[0]==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(RTK_RG_MAX_EXT_PORT); j++)
			{
				if(ext.bits[0] & (1<<j))  { PROC_PRINTF("%d ", j);}
			}
		}
		PROC_PRINTF("\n");
	#ifdef CONFIG_MASTER_WLAN0_ENABLE
		PROC_PRINTF("\tWlan0 Devices: ");
		if(rg_db.vlan[i].wlan0DevMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0DevMask & (1<<j))  { PROC_PRINTF("%d ", j);}
			}
		}
		PROC_PRINTF("\n");
		PROC_PRINTF("\tWlan0 Untag: ");
		if(rg_db.vlan[i].wlan0UntagMask==0) {PROC_PRINTF("X");}
		else
		{
			for(j=0; j<(MAX_WLAN_DEVICE_NUM); j++)
			{
				if(rg_db.vlan[i].wlan0UntagMask & (1<<j))   PROC_PRINTF("%d ", j);
			}
		}
		PROC_PRINTF("\n");
	#endif
	}

	//Display Port-based VLAN ID information
	PROC_PRINTF("\n>>ASIC Port-based VLAN settings:\n");
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)


	PROC_PRINTF("PORT:\t    0     1     2     3     4  5PON 6iNIC 7sCPU  8MII 9CPU0 10CPU1\nVID:\t");
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		rtk_vlan_portPvid_get(i,&pvid);
		PROC_PRINTF("%5d ",pvid);
	}

	PROC_PRINTF("\nEXTPORT: 7-0  7-1  7-2  7-3  7-4  7-5  9-0  9-1  9-2  9-3  9-4  9-5 10-0 10-1 10-2 10-3 10-4 10-5\nVID:\t");
	for(i=0;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		rtk_vlan_extPortPvid_get(i,&pvid);
		PROC_PRINTF("%4d ",pvid);
	}
	PROC_PRINTF("\n");

#else
	//!CONFIG_RG_FLOW_BASED_PLATFORM
#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF("PORT:\t   0    1 2PON 3CPU 4EX0 5EX1 6EX2 7EX3 8EX4 9EX5   10\nVID:\t");
#else
	PROC_PRINTF("PORT:\t   0    1    2    3 4PON 5MII 6CPU 7EX0 8EX1 9EX2   10   11\nVID:\t");
#endif

	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		rtk_vlan_portPvid_get(i,&pvid);
		PROC_PRINTF("%4d ",pvid);
	}

	for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
	{
		rtk_vlan_extPortPvid_get(i,&pvid);
		PROC_PRINTF("%4d ",pvid);
	}
	PROC_PRINTF("\n");

	//Display Port-based SVLAN ID information
#if defined(CONFIG_RG_RTL9602C_SERIES)
	if(rg_kernel.stag_enable){
		PROC_PRINTF("\n>>ASIC Port-based SVALN settings:\n");
		PROC_PRINTF("PORT:\t   0    1 2PON 3CPU 4EX0 5EX1 6EX2 7EX3 8EX4 9EX5 10X3\nSVID:\t");

		for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
		{
			rtk_svlan_portSvid_get(i, &pvid);
			PROC_PRINTF("%4d ",pvid);
		}

		/*for(i=1;i<RTK_RG_MAX_EXT_PORT;i++)	//extPort[0] is same as the CPU port
		{
			rtk_vlan_extPortSvid_get(i, &pvid);
			PROC_PRINTF("%5d ",pvid);
		}*/
		PROC_PRINTF("\n");
	}
#endif

#endif//CONFIG_RG_FLOW_BASED_PLATFORM

	//Display Port-and-protocol-based VLAN ID information
	PROC_PRINTF("\n>>ASIC Port-and-Protocol-based VLAN settings:\n");

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++) 	//port
	{
		int firstDump=TRUE;
		for(j=0; j<MAX_PORT_PROTO_GROUP_SIZE; j++)
		{
			bzero(&protoGroup,sizeof(protoGroup));
			rtk_vlan_protoGroup_get(j, &protoGroup);
			bzero(&ppbCfg,sizeof(ppbCfg));
			rtk_vlan_portProtoVlan_get(i,j,&ppbCfg);
			if(ppbCfg.valid)
			{

				if(firstDump==TRUE) {PROC_PRINTF("  Port %d:\n",i); firstDump=FALSE; }
				switch(protoGroup.frametype)
				{
					case FRAME_TYPE_ETHERNET:
						PROC_PRINTF("    Group[%d] Ethertype:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_RFC1042:
						PROC_PRINTF("    Group[%d] frame_type:RFC1042 frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_SNAP8021H:
						PROC_PRINTF("    Group[%d] frame_type:SNAP8021H frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_SNAPOTHER:
						PROC_PRINTF("    Group[%d] frame_type:SNAPOTHER frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					case FRAME_TYPE_LLCOTHER:
						PROC_PRINTF("    Group[%d] frame_type:LLCOTHER frame_value:0x%04x",j,protoGroup.framevalue);
						break;
					default:
						break;
				}
				//PROC_PRINTF("\tVID:%d PRI:%d CFI:%d\n",ppbCfg.vid,ppbCfg.pri,ppbCfg.dei);
				PROC_PRINTF(" VID[%d]\n",ppbCfg.vid);
			}
		}
	}

	ASSERT_EQ(rtk_vlan_vlanFunctionEnable_get(&en), RT_ERR_OK);
	PROC_PRINTF("\nCVLAN Filtering(%s)\n", en==ENABLED?"Enabled":"Disabled");
#if defined(CONFIG_RG_RTL9602C_SERIES)
	if(rg_kernel.stag_enable){
		ASSERT_EQ(rtk_svlan_svlanFunctionEnable_get(&en), RT_ERR_OK);
		PROC_PRINTF("\nSVLAN Filtering(%s)\n", en==ENABLED?"Enabled":"Disabled");
	}
#endif

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//Display WLAN0-device-based VLAN ID information
	PROC_PRINTF("\n>>WLAN0-device-based VLAN settings:\n");

	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[i].exist)
		{
			PROC_PRINTF("DEV[%d] DVID=%d\n",i,rg_db.systemGlobal.wlan0DeviceBasedVID[i]);
		}
	}
#endif

    return len;
}
#endif

int32 dump_vlan_mbr(struct seq_file *s, void *v)
{
	int i;
	uint32 val1, val2;
	int len=0;

	PROC_PRINTF(">>ASIC VLAN Member Configurations:\n\n");
	i=0;val1=0;val2=0;


#ifdef CONFIG_APOLLO_RLE0371
	for(i=0; i<32; i++)
	{
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, MBRf, &val2);
		if(val2==0) continue;

		PROC_PRINTF(" -- IDX[%d] --\n", i);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, EVIDf, &val1);
		PROC_PRINTF("\tCVID: %d\n", val1);
		PROC_PRINTF("\tMember Ports Mask: 0x%x\n", val2);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, EXT_MBRf, &val1);
		PROC_PRINTF("\tExtension Ports Mask: 0x%x\n", val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, VBPRIf, &val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, VBPENf, &val2);
		PROC_PRINTF("\tBased Priority: %s, %d\n",
					   val2?"enable":"disable", val1);
		reg_array_field_read(VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, FID_MSTIf, &val1);
		PROC_PRINTF("\tFID: %d\n", val1);
	}
#else
	switch(rg_kernel.apolloChipId)
	{



#if defined(CONFIG_RG_RTL9600_SERIES)

		case APOLLOMP_CHIP_ID:
			for(i=0; i<32; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_MBRf, &val2);
				if(val2==0) continue;

				PROC_PRINTF(" -- IDX[%d] --\n", i);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_EVIDf, &val1);
				PROC_PRINTF("\tCVID: %d\n", val1);
				PROC_PRINTF("\tMember Ports Mask: 0x%x\n", val2);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_EXT_MBRf, &val1);
				PROC_PRINTF("\tExtension Ports Mask: 0x%x\n", val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VBPRIf, &val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VBPENf, &val2);
				PROC_PRINTF("\tBased Priority: %s, %d\n",
							   val2?"enable":"disable", val1);
				reg_array_field_read(APOLLOMP_VLAN_MBR_CFGr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_FID_MSTIf, &val1);
				PROC_PRINTF("\tFID: %d\n", val1);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif
    return len;
}
int32 dump_svlan_mbr(struct seq_file *s, void *v)
{
    int i,j;
    rtk_svlan_memberCfg_t ent= {0};
    int res = 0, count = 0;
	int len=0;
    PROC_PRINTF(">>ASIC SVLAN Member Configurations:\n\n");

    for(i=0; i<MAX_VLAN_HW_TABLE_SIZE; i++)
    {
        ent.svid = i;
        res = rtk_svlan_memberPortEntry_get(&ent);
        if((res!=RT_ERR_OK) || (ent.memberport.bits[0] == 0)) continue;
        else
        {
            PROC_PRINTF(" -- COUNT[%d] --\n", ++count);	// can't get real index here with rtk API
            PROC_PRINTF("\tSVID: %d", ent.svid);
            PROC_PRINTF("\tS-Priority: %d\n", ent.priority);

            PROC_PRINTF("\tMember Ports: ");
            if(ent.memberport.bits[0]==0) {PROC_PRINTF("X");}
            else
            {
                for(j=0; j<(RTK_RG_MAC_PORT_MAX+RTK_RG_MAX_EXT_PORT); j++)
                {
                    if(ent.memberport.bits[0] & (1<<j)) { PROC_PRINTF("%d ", j);}
                }
            }
            PROC_PRINTF("\n\tUntag Member Ports: ");
            if(ent.untagport.bits[0]==0) {PROC_PRINTF("X");}
            else
            {
                for(j=0; j<(RTK_RG_MAC_PORT_MAX+RTK_RG_MAX_EXT_PORT); j++)
                {
                    if(ent.untagport.bits[0] & (1<<j))  {PROC_PRINTF("%d ", j);}
                }
            }
            PROC_PRINTF("\n\tForce FID: %s, %d\n", ent.fiden?"enabled":"disabled", ent.fid);
            PROC_PRINTF("\tEnhanced FID: %s, %d\n",ent.efiden?"enabled":"disabled", ent.efid);
        }
    }

    return len;
}
int32 dump_svlan_c2s(struct seq_file *s, void *v)
{
	int len=0;
	PROC_PRINTF("fix me");
#if 0
	// need too much time in FPGA
    /*    rtk_vlan_t svid=0, cvid;
        rtk_port_t port, pmsk=0;
        int res=0, count=0;
        for(cvid=0; cvid<(RTL865XC_VLAN_NUMBER*2-1); cvid++)
        {
            pmsk = 0;
            svid = 0;
            for(port=0; port<MAX_APOLLO_PORT; port++)
            {
                res=rtk_svlan_c2s_get(cvid, port, &svid);
                if(res==RT_ERR_OK) pmsk|=1<<port;
            }
            if(pmsk!=0)
            {
                PROC_PRINTF(" -- COUNT[%d] --\n", count++);
                PROC_PRINTF("\tEVID: %d\n", cvid);
                PROC_PRINTF("\tC2SENPMSK: 0x%x\n", pmsk);
                PROC_PRINTF("\tSVID: %d\n", svid);
            }
        }*/
    int i;
    uint32 val;
    PROC_PRINTF(">>ASIC SVLAN CVID/PVID to SVID Tables:\n\n");

    for(i=0; i<MAX_C2S_HW_TABLE_SIZE; i++)
    {
        ioal_mem32_read(0x014000+i*0x4,&val);
        if(((val>>0x6)&0x7F)==0) continue;
        PROC_PRINTF(" -- IDX[%d] --\n", i);
        PROC_PRINTF("\tSVIDX: 0x%x\n", (val)&0x1F);
        PROC_PRINTF("\tEVID: 0x%x\n", (val>>0xD)&0x1FFF);
        PROC_PRINTF("\tC2SENPMSK: 0x%x\n", (val>>0x6)&0x7F);
    }
#endif
    return len;
}
int32 dump_svlan_mc2s(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	int i;
	uint32 val;
	int len=0;
	PROC_PRINTF(">>ASIC SVLAN Multicast to SVLAN Tables:\n\n");
	i=0;val=0;

	//SVLAN_MC2S
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<SVLANMC2STBL_SIZE; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_VALIDf, &val);
				if(val==0) continue;

				PROC_PRINTF(" -- IDX[%d] --\n", i);
				//PROC_PRINTF("\tValid: yes\n");
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_FORMATf, &val);
				PROC_PRINTF("\tFormat: %s\n", (val!=0)?"DIP":"DMAC");
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_SVIDXf, &val);
				PROC_PRINTF("\tSVIDX: 0x%x\n", val);
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_DATAf, &val);
				PROC_PRINTF("\tData: 0x%x\n", val);
				reg_array_field_read(APOLLOMP_SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, i, APOLLOMP_MASKf, &val);
				PROC_PRINTF("\tMask: 0x%x\n", val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			PROC_PRINTF("9602C SERIES, RTL9601B_CHIP_ID=%d\n",RTL9601B_CHIP_ID);

			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif
    return len;
}
int32 dump_svlan_sp2c(struct seq_file *s, void *v)
{
    rtk_vlan_t svid, cvid;
    rtk_port_t port;
    int res=0, count=0;
	int len=0;
    for(svid=0; svid<MAX_VLAN_HW_TABLE_SIZE; svid++)
    {
        cvid = 0;
        for(port=0; port<RTK_RG_MAC_PORT_MAX; port++)
        {
            res=rtk_svlan_sp2c_get(svid, port, &cvid);
            if(res==RT_ERR_OK)
            {
                PROC_PRINTF(" -- COUNT[%d] --\n", count++);
                PROC_PRINTF("\tSVID: %d\n", svid);
                PROC_PRINTF("\tDestination Port: %d\n", port);
                PROC_PRINTF("\tExtened VID: %d\n", cvid);
            }
        }
    }

    return len;
}


int32 dump_acl_template(struct seq_file *s, void *v)
{
	int len =0;
	_dump_acl_template(s);
	return len;
}



int32 dump_acl_vidRangeTable(struct seq_file *s, void *v)
{
    int i,ret;
    rtk_acl_rangeCheck_vid_t vidRangeEntry;
	int len=0;

    PROC_PRINTF("------------ ACL VID RANGE TABLES -------------\n");
    for(i=0; i<8; i++)
    {
        memset(&vidRangeEntry,0,sizeof(vidRangeEntry));
        vidRangeEntry.index=i;
        ret=rtk_acl_vidRange_get(&vidRangeEntry);
		if(ret != RT_ERR_OK) continue;
        PROC_PRINTF("\tVIDRANGE[%d] upper:%d lower:%d type:0x%x\n",i,vidRangeEntry.upperVid,vidRangeEntry.lowerVid,vidRangeEntry.type);
    }
    return len;
}

int32 dump_acl_ipRangeTable(struct seq_file *s, void *v)
{
	int len =0;
	_dump_acl_ipRangeTable(s);

    return len;
}


int32 dump_acl_portRangeTable(struct seq_file *s, void *v)
{

	int len =0;
	_dump_acl_portRangeTable(s);
    return len;
}

int32 dump_acl_pktlenRangeTable(struct seq_file *s, void *v)
{
    int i;
	int len=0;

#if defined(CONFIG_RG_G3_SERIES)
	i = 0;
	len = L3_CAM_PKT_LEN_TBL_ENTRY_MAX;
	//Please check proc/dump/acl_ca and search pkt_len_range_idx to get usage purpose.
	rtk_rg_callback_pipe_cmd("echo %d %d %d > /proc/driver/cortina/aal/aal_table", AAL_TABLE_L3_CAM_PKT_LEN, i, len);
#else
    rtk_acl_rangeCheck_pktLength_t pktlenRangeEntry;
    PROC_PRINTF("------------ ACL PKTLEN RANGE TABLES -------------\n");
    for(i=0; i<8; i++)
    {
        memset(&pktlenRangeEntry,0,sizeof(pktlenRangeEntry));
        pktlenRangeEntry.index=i;
        rtk_acl_packetLengthRange_get(&pktlenRangeEntry);
        PROC_PRINTF("\tPKTLENRANGE[%d] upper:%d lower:%d type:0x%x\n",i,pktlenRangeEntry.upper_bound,pktlenRangeEntry.lower_bound,pktlenRangeEntry.type);
    }
#endif
    return len;
}


int32 dump_rg_naptPriority(struct seq_file *s, void *v){
	int i,j;
	int len=0;
	rtk_rg_naptFilterAndQos_t *naptFilterAndQos;
	rtk_rg_sw_naptFilterAndQos_t *pValidRule;


	for(i=0;i<MAX_NAPT_FILER_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.napt_SW_table_entry[i].valid==RTK_RG_ENABLED){
			naptFilterAndQos = &rg_db.systemGlobal.napt_SW_table_entry[i].naptFilter;
			PROC_PRINTF("==============[%d]=============\n",i);
			PROC_PRINTF("direction: %s \n",naptFilterAndQos->direction?"Downstream":"Upstream");
			PROC_PRINTF("followDirectionMatch: %s \n",naptFilterAndQos->followDirectionMatch?"FOLLOW_DIR":"BOTH_DIR");
			PROC_PRINTF("weight: %d \n",naptFilterAndQos->weight);
			PROC_PRINTF("patterns fields: 0x%x \n",naptFilterAndQos->filter_fields);
			if(naptFilterAndQos->filter_fields&INGRESS_SIP)
				PROC_PRINTF("ingress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_src_ipv4_addr);
			if(naptFilterAndQos->filter_fields&EGRESS_SIP)
				PROC_PRINTF("egress_src_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_src_ipv4_addr);
			if(naptFilterAndQos->filter_fields&INGRESS_DIP)
				PROC_PRINTF("ingress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->ingress_dest_ipv4_addr);
			if(naptFilterAndQos->filter_fields&EGRESS_DIP)
				PROC_PRINTF("egress_dest_ipv4_addr: 0x%x \n",naptFilterAndQos->egress_dest_ipv4_addr);
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT)
				PROC_PRINTF("ingress_src_l4_port: %d \n",naptFilterAndQos->ingress_src_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT)
				PROC_PRINTF("egress_src_l4_port: %d \n",naptFilterAndQos->egress_src_l4_port);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT)
				PROC_PRINTF("ingress_dest_l4_port: %d \n",naptFilterAndQos->ingress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT)
				PROC_PRINTF("egress_dest_l4_port: %d \n",naptFilterAndQos->egress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&INGRESS_SIP_RANGE)
				PROC_PRINTF("ingress_src_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->ingress_src_ipv4_addr_range_start,naptFilterAndQos->ingress_src_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_DIP_RANGE)
				PROC_PRINTF("ingress_dest_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->ingress_dest_ipv4_addr_range_start,naptFilterAndQos->ingress_dest_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT_RANGE)
				PROC_PRINTF("ingress_src_l4_port_range: %d ~ %d \n",naptFilterAndQos->ingress_src_l4_port_range_start,naptFilterAndQos->ingress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT_RANGE)
				PROC_PRINTF("ingress_dest_l4_port_range: %d ~ %d \n",naptFilterAndQos->ingress_dest_l4_port_range_start,naptFilterAndQos->ingress_dest_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_SIP_RANGE)
				PROC_PRINTF("egress_src_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->egress_src_ipv4_addr_range_start,naptFilterAndQos->egress_src_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_DIP_RANGE)
				PROC_PRINTF("egress_dest_ipv4_addr_range: 0x%x ~ 0x%x \n",naptFilterAndQos->egress_dest_ipv4_addr_range_start,naptFilterAndQos->egress_dest_ipv4_addr_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT_RANGE)
				PROC_PRINTF("egress_src_l4_port_range: %d ~ %d \n",naptFilterAndQos->egress_src_l4_port_range_start,naptFilterAndQos->egress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT_RANGE)
				PROC_PRINTF("egress_dest_l4_port_range: %d ~ %d \n",naptFilterAndQos->egress_dest_l4_port_range_start,naptFilterAndQos->egress_dest_l4_port_range_end);

			if(naptFilterAndQos->filter_fields&L4_PROTOCAL)
				PROC_PRINTF("ingress_l4_protocal: %d \n",naptFilterAndQos->ingress_l4_protocal);

			if(naptFilterAndQos->filter_fields&INGRESS_SMAC)
				PROC_PRINTF("ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X \n",
				naptFilterAndQos->ingress_smac.octet[0],naptFilterAndQos->ingress_smac.octet[1],naptFilterAndQos->ingress_smac.octet[2],
				naptFilterAndQos->ingress_smac.octet[3],naptFilterAndQos->ingress_smac.octet[4],naptFilterAndQos->ingress_smac.octet[5]);

			if(naptFilterAndQos->filter_fields&EGRESS_DMAC)
				PROC_PRINTF("egress_dmac: %02X:%02X:%02X:%02X:%02X:%02X \n",
				naptFilterAndQos->egress_dmac.octet[0],naptFilterAndQos->egress_dmac.octet[1],naptFilterAndQos->egress_dmac.octet[2],
				naptFilterAndQos->egress_dmac.octet[3],naptFilterAndQos->egress_dmac.octet[4],naptFilterAndQos->egress_dmac.octet[5]);

			PROC_PRINTF("action_bit: 0x%x\n",naptFilterAndQos->action_fields);
			if(naptFilterAndQos->action_fields&NAPT_DROP_BIT) PROC_PRINTF("drop: %s\n",(naptFilterAndQos->action_fields&NAPT_DROP_BIT)?"Enable":"Disable");
			if(naptFilterAndQos->action_fields&NAPT_PERMIT_BIT) PROC_PRINTF("permit: %s\n",(naptFilterAndQos->action_fields&NAPT_PERMIT_BIT)?"Enable":"Disable");
			if(naptFilterAndQos->action_fields&NAPT_SW_TRAP_TO_PS) PROC_PRINTF("trap to ps: %s\n",(naptFilterAndQos->action_fields&NAPT_SW_TRAP_TO_PS)?"Enable":"Disable");
			if(naptFilterAndQos->action_fields&NAPT_SW_COPY_TO_PS) PROC_PRINTF("copy to ps: %d times \n",naptFilterAndQos->assign_copy_num);
			if(naptFilterAndQos->action_fields&ASSIGN_NAPT_PRIORITY_BIT) PROC_PRINTF("assign_priority: %d \n",naptFilterAndQos->assign_priority);
			if(naptFilterAndQos->action_fields&NAPT_SW_RATE_LIMIT_BIT)PROC_PRINTF("rate limit: %d(Kbps)\n",naptFilterAndQos->assign_rate);
			if(naptFilterAndQos->action_fields&NAPT_SW_PACKET_COUNT)PROC_PRINTF("packet count: %d\n",naptFilterAndQos->packet_count);
			if(naptFilterAndQos->action_fields&NAPT_SW_BYTE_COUNT)PROC_PRINTF("byte count: %d\n",naptFilterAndQos->byte_count);

			PROC_PRINTF("ruleType: %s \n",naptFilterAndQos->ruleType?"One Shot":"Persist");

		}
	}


	PROC_PRINTF("==============[US sorting by weight]=============\n");
	pValidRule = NULL;
	if(rg_db.systemGlobal.pValidUsNaptPriorityRuleStart!=NULL)
		pValidRule = rg_db.systemGlobal.pValidUsNaptPriorityRuleStart;
	else
		PROC_PRINTF("pValidUsNaptPriorityRuleStart is NULL \n");

	if(pValidRule!=NULL){
		j=0;//avoid cycle
		while(pValidRule!=NULL && j<MAX_NAPT_FILER_SW_ENTRY_SIZE){
			PROC_PRINTF("rule[%d]>",pValidRule->sw_index);
			pValidRule=pValidRule->pNextValid;
			j++;
		}
	}
	PROC_PRINTF("\n");

	PROC_PRINTF("==============[DS sorting by weight]=============\n");
	pValidRule = NULL;
	if(rg_db.systemGlobal.pValidDsNaptPriorityRuleStart!=NULL)
		pValidRule = rg_db.systemGlobal.pValidDsNaptPriorityRuleStart;
	else
		PROC_PRINTF("pValidDsNaptPriorityRuleStart is NULL \n");

	if(pValidRule!=NULL){
		j=0;//avoid cycle
		while(pValidRule!=NULL && j<MAX_NAPT_FILER_SW_ENTRY_SIZE){
			PROC_PRINTF("rule[%d]>",pValidRule->sw_index);
			pValidRule=pValidRule->pNextValid;
			j++;
		}
	}
	PROC_PRINTF("\n");


	return len;
}

int32 _dump_rg_napt_filterAndQos_diagshell(struct seq_file *s){
	int i;
	int len=0;
	rtk_rg_naptFilterAndQos_t *naptFilterAndQos;

	for(i=0;i<MAX_NAPT_FILER_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.napt_SW_table_entry[i].valid==RTK_RG_ENABLED){
			naptFilterAndQos = &rg_db.systemGlobal.napt_SW_table_entry[i].naptFilter;

			PROC_PRINTF("rg clear naptFilterAndQos\n");
			if(naptFilterAndQos->direction)					PROC_PRINTF("rg set naptFilterAndQos direction %d\n", naptFilterAndQos->direction);
			if(naptFilterAndQos->followDirectionMatch)		PROC_PRINTF("rg set naptFilterAndQos followDirectionMatch %d\n", naptFilterAndQos->followDirectionMatch);
			if(naptFilterAndQos->weight)					PROC_PRINTF("rg set naptFilterAndQos weight %d\n", naptFilterAndQos->weight);
			if(naptFilterAndQos->ruleType)					PROC_PRINTF("rg set naptFilterAndQos ruleType %d \n",naptFilterAndQos->ruleType);

			if(naptFilterAndQos->filter_fields&INGRESS_SIP)	PROC_PRINTF("rg set naptFilterAndQos pattern ingress_src_ipv4_addr %s \n",diag_util_inet_ntoa(naptFilterAndQos->ingress_src_ipv4_addr));
			if(naptFilterAndQos->filter_fields&EGRESS_SIP)	PROC_PRINTF("rg set naptFilterAndQos pattern egress_src_ipv4_addr %s \n",diag_util_inet_ntoa(naptFilterAndQos->egress_src_ipv4_addr));
			if(naptFilterAndQos->filter_fields&INGRESS_DIP)	PROC_PRINTF("rg set naptFilterAndQos pattern ingress_dest_ipv4_addr %s \n",diag_util_inet_ntoa(naptFilterAndQos->ingress_dest_ipv4_addr));
			if(naptFilterAndQos->filter_fields&EGRESS_DIP)	PROC_PRINTF("rg set naptFilterAndQos pattern egress_dest_ipv4_addr %s \n",diag_util_inet_ntoa(naptFilterAndQos->egress_dest_ipv4_addr));
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT)	PROC_PRINTF("rg set naptFilterAndQos pattern ingress_src_l4_port %d \n",naptFilterAndQos->ingress_src_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT)	PROC_PRINTF("rg set naptFilterAndQos pattern egress_src_l4_port %d \n",naptFilterAndQos->egress_src_l4_port);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT)	PROC_PRINTF("rg set naptFilterAndQos pattern ingress_dest_l4_port %d \n",naptFilterAndQos->ingress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT)	PROC_PRINTF("rg set naptFilterAndQos pattern egress_dest_l4_port %d \n",naptFilterAndQos->egress_dest_l4_port);
			if(naptFilterAndQos->filter_fields&L4_PROTOCAL)		PROC_PRINTF("rg set naptFilterAndQos pattern ingress_l4_protocal %d \n",naptFilterAndQos->ingress_l4_protocal);
			if(naptFilterAndQos->filter_fields&INGRESS_SIP_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern ingress_src_ipv4_addr_range ingress_src_ipv4_addr_range_start %d.%d.%d.%d ingress_src_ipv4_addr_range_end %d.%d.%d.%d \n",
				(naptFilterAndQos->ingress_src_ipv4_addr_range_start & 0xff000000)>>24,(naptFilterAndQos->ingress_src_ipv4_addr_range_start & 0xff0000)>>16,(naptFilterAndQos->ingress_src_ipv4_addr_range_start & 0xff00)>>8,(naptFilterAndQos->ingress_src_ipv4_addr_range_start & 0xff),
				(naptFilterAndQos->ingress_src_ipv4_addr_range_end & 0xff000000)>>24,(naptFilterAndQos->ingress_src_ipv4_addr_range_end & 0xff0000)>>16,(naptFilterAndQos->ingress_src_ipv4_addr_range_end & 0xff00)>>8,(naptFilterAndQos->ingress_src_ipv4_addr_range_end & 0xff));
			if(naptFilterAndQos->filter_fields&INGRESS_DIP_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern ingress_dest_ipv4_addr_range ingress_dest_ipv4_addr_range_start %d.%d.%d.%d ingress_dest_ipv4_addr_range_end %d.%d.%d.%d \n",
				(naptFilterAndQos->ingress_dest_ipv4_addr_range_start & 0xff000000)>>24,(naptFilterAndQos->ingress_dest_ipv4_addr_range_start & 0xff0000)>>16,(naptFilterAndQos->ingress_dest_ipv4_addr_range_start & 0xff00)>>8,(naptFilterAndQos->ingress_dest_ipv4_addr_range_start & 0xff),
				(naptFilterAndQos->ingress_dest_ipv4_addr_range_end & 0xff000000)>>24,(naptFilterAndQos->ingress_dest_ipv4_addr_range_end & 0xff0000)>>16,(naptFilterAndQos->ingress_dest_ipv4_addr_range_end & 0xff00)>>8,(naptFilterAndQos->ingress_dest_ipv4_addr_range_end & 0xff));
			if(naptFilterAndQos->filter_fields&INGRESS_SPORT_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern ingress_src_l4_port_range ingress_src_l4_port_range_start %d ingress_src_l4_port_range_end %d \n",
				naptFilterAndQos->ingress_src_l4_port_range_start,naptFilterAndQos->ingress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_DPORT_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern ingress_dest_l4_port_range ingress_dest_l4_port_range_start %d ingress_dest_l4_port_range_end %d \n",
				naptFilterAndQos->ingress_dest_l4_port_range_start,naptFilterAndQos->ingress_dest_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_SIP_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern egress_src_ipv4_addr_range egress_src_ipv4_addr_range_start %d.%d.%d.%d egress_src_ipv4_addr_range_end %d.%d.%d.%d \n",
				(naptFilterAndQos->egress_src_ipv4_addr_range_start & 0xff000000)>>24,(naptFilterAndQos->egress_src_ipv4_addr_range_start & 0xff0000)>>16,(naptFilterAndQos->egress_src_ipv4_addr_range_start & 0xff00)>>8,(naptFilterAndQos->egress_src_ipv4_addr_range_start & 0xff),
				(naptFilterAndQos->egress_src_ipv4_addr_range_end & 0xff000000)>>24,(naptFilterAndQos->egress_src_ipv4_addr_range_end & 0xff0000)>>16,(naptFilterAndQos->egress_src_ipv4_addr_range_end & 0xff00)>>8,(naptFilterAndQos->egress_src_ipv4_addr_range_end & 0xff));
			if(naptFilterAndQos->filter_fields&EGRESS_DIP_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern egress_dest_ipv4_addr_range egress_dest_ipv4_addr_range_start %d.%d.%d.%d egress_dest_ipv4_addr_range_end %d.%d.%d.%d \n",
				(naptFilterAndQos->egress_dest_ipv4_addr_range_start & 0xff000000)>>24,(naptFilterAndQos->egress_dest_ipv4_addr_range_start & 0xff0000)>>16,(naptFilterAndQos->egress_dest_ipv4_addr_range_start & 0xff00)>>8,(naptFilterAndQos->egress_dest_ipv4_addr_range_start & 0xff),
				(naptFilterAndQos->egress_dest_ipv4_addr_range_end & 0xff000000)>>24,(naptFilterAndQos->egress_dest_ipv4_addr_range_end & 0xff0000)>>16,(naptFilterAndQos->egress_dest_ipv4_addr_range_end & 0xff00)>>8,(naptFilterAndQos->egress_dest_ipv4_addr_range_end & 0xff));
			if(naptFilterAndQos->filter_fields&EGRESS_SPORT_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern egress_src_l4_port_range egress_src_l4_port_range_start %d egress_src_l4_port_range_end %d \n",
				naptFilterAndQos->egress_src_l4_port_range_start,naptFilterAndQos->egress_src_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&EGRESS_DPORT_RANGE)
				PROC_PRINTF("rg set naptFilterAndQos pattern egress_dest_l4_port_range egress_dest_l4_port_range_start %d egress_dest_l4_port_range_end %d \n",
				naptFilterAndQos->egress_dest_l4_port_range_start,naptFilterAndQos->egress_dest_l4_port_range_end);
			if(naptFilterAndQos->filter_fields&INGRESS_SMAC)
				PROC_PRINTF("rg set naptFilterAndQos pattern ingress_smac %02X:%02X:%02X:%02X:%02X:%02X \n",
				naptFilterAndQos->ingress_smac.octet[0],naptFilterAndQos->ingress_smac.octet[1],naptFilterAndQos->ingress_smac.octet[2],
				naptFilterAndQos->ingress_smac.octet[3],naptFilterAndQos->ingress_smac.octet[4],naptFilterAndQos->ingress_smac.octet[5]);
			if(naptFilterAndQos->filter_fields&EGRESS_DMAC)
				PROC_PRINTF("rg set naptFilterAndQos pattern egress_dmac %02X:%02X:%02X:%02X:%02X:%02X \n",
				naptFilterAndQos->egress_dmac.octet[0],naptFilterAndQos->egress_dmac.octet[1],naptFilterAndQos->egress_dmac.octet[2],
				naptFilterAndQos->egress_dmac.octet[3],naptFilterAndQos->egress_dmac.octet[4],naptFilterAndQos->egress_dmac.octet[5]);

			if(naptFilterAndQos->action_fields&ASSIGN_NAPT_PRIORITY_BIT)	PROC_PRINTF("rg set naptFilterAndQos action assign_priority %d \n",naptFilterAndQos->assign_priority);
			if(naptFilterAndQos->action_fields&NAPT_DROP_BIT)			PROC_PRINTF("rg set naptFilterAndQos action drop\n");
			if(naptFilterAndQos->action_fields&NAPT_PERMIT_BIT)			PROC_PRINTF("rg set naptFilterAndQos action permit\n");
			if(naptFilterAndQos->action_fields&NAPT_SW_RATE_LIMIT_BIT)	PROC_PRINTF("rg set naptFilterAndQos action rate_limit %d\n",naptFilterAndQos->assign_rate);
			if(naptFilterAndQos->action_fields&NAPT_SW_TRAP_TO_PS)		PROC_PRINTF("rg set naptFilterAndQos action sw_trap_to_ps\n");
			if(naptFilterAndQos->action_fields&NAPT_SW_COPY_TO_PS)		PROC_PRINTF("rg set naptFilterAndQos action sw_copy_to_ps %d\n",naptFilterAndQos->assign_copy_num);
			if(naptFilterAndQos->action_fields&NAPT_SW_PACKET_COUNT)	PROC_PRINTF("rg set naptFilterAndQos action packet_count \n");
			if(naptFilterAndQos->action_fields&NAPT_SW_BYTE_COUNT)		PROC_PRINTF("rg set naptFilterAndQos action byte_count\n");
			PROC_PRINTF("rg add naptFilterAndQos entry\n");
		}
	}

	return len;
}

char *name_of_ds_bc_tagDecision[]={
	"FORCE_UNATG",
	"FORCE_TAGGIN_WITH_CVID",
	"FORCE_TAGGIN_WITH_CVID_CPRI",
	"FORCE_TRANSPARENT",
};

int dump_gpon_ds_bc_filterAndRemarkingRules(struct seq_file *s, void *v){
	int i;
	rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule;
	int len=0;

	for(i=0;i<MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].valid==RTK_RG_ENABLED){
			filterRule = &rg_db.systemGlobal.gpon_SW_ds_bc_filter_table_entry[i].filterRule;

			PROC_PRINTF("=======================[%d]=================================\n",i);
			PROC_PRINTF("filter_fields = 0x%x\n",filterRule->filter_fields);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STREAMID_BIT) PROC_PRINTF("ingress_stream_id= %d\n",filterRule->ingress_stream_id);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_STAGIf_BIT) PROC_PRINTF("%s ",filterRule->ingress_stagIf?"WITH_STAG":"WITHOUT_STAG");
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_SVID_BIT) PROC_PRINTF("svid=%d\n",filterRule->ingress_stag_svid);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CTAGIf_BIT) PROC_PRINTF("%s ",filterRule->ingress_ctagIf?"WITH_CTAG":"WITHOUT_CTAG");
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_INGRESS_CVID_BIT) PROC_PRINTF("cvid=%d\n",filterRule->ingress_ctag_cvid);
			if(filterRule->filter_fields & GPON_DS_BC_FILTER_EGRESS_PORT_BIT) PROC_PRINTF("egres_port_mask=0x%x\n",filterRule->egress_portmask.portmask);
			PROC_PRINTF("action: %s cvid=%d cpri=%d\n",name_of_ds_bc_tagDecision[filterRule->ctag_action.ctag_decision],filterRule->ctag_action.assigned_ctag_cvid,filterRule->ctag_action.assigned_ctag_cpri);
		}
	}

	return len;
}

//ipset feature
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==1)
char *name_of_ipset_filter[]={
	"IPV4_IP",
	"L4P_TCP",
	"L4P_UDP",
	"IPV6_IP",
	"FILTER_END",
};
char *name_of_ipset_action[]={
	"TO_PS",
	"ACTION_END",
};

void _dump_rg_ipset_rule(int set_idx, int index, rtk_rg_ipset_info_t ipset_rule, uint32 hash_idx){

	if(index >= MAX_IPSET_SW_ENTRY_SIZE){
		rtlglue_printf("IPSET[%d][invalid index %d]\n", set_idx, index);
		return;
	}

	if(ipset_rule.filter_type >= RTK_RG_IPSET_FILTER_TYPE_END){
		rtlglue_printf("IPSET[%d][%6d] disable\n", set_idx, index);
		return;
	}
	if(ipset_rule.action_type >= RTK_RG_IPSET_ACTION_TYPE_END)
		rtlglue_printf("IPSET[%d][%6d](filter %7s, unknown act %d)", set_idx, index, name_of_ipset_filter[ipset_rule.filter_type], ipset_rule.action_type);
	else
		rtlglue_printf("IPSET[%d][%6d](filter %7s, act %s)", set_idx, index, name_of_ipset_filter[ipset_rule.filter_type], name_of_ipset_action[ipset_rule.action_type]);

	if(hash_idx < MAX_IPSET_HASH_SIZE)
		rtlglue_printf("(hash %3d): ", hash_idx);
	else
		rtlglue_printf(": ");

	if(ipset_rule.filter_type == RTK_RG_IPSET_FILTER_TYPE_REMOTE_IPV4_IP)
		rtlglue_printf("addr %pI4/%d", &ipset_rule.ipv4_address, ipset_rule.ip_suffix_len);
	else if((ipset_rule.filter_type >= RTK_RG_IPSET_FILTER_TYPE_REMOTE_L4P_TCP) && (ipset_rule.filter_type <= RTK_RG_IPSET_FILTER_TYPE_REMOTE_L4P_UDP))
		rtlglue_printf("l4 port %d-%d", ipset_rule.l4_port_start, ipset_rule.l4_port_end);
	else if(ipset_rule.filter_type == RTK_RG_IPSET_FILTER_TYPE_REMOTE_IPV6_IP)
		rtlglue_printf("addr %pI6c/%d", ipset_rule.ipv6_address, ipset_rule.ip_suffix_len);

	rtlglue_printf("\n");

	return;
}

int dump_rg_ipset(struct seq_file *s, void *v){
	int len=0;
	rtk_rg_ipset_list_t *ipset_rule=NULL;
	int set_idx, loop_idx, hash_valid_entry;

	for(set_idx=0;set_idx<MAX_IPSET_IDX_SIZE;set_idx++){

	rtlglue_printf("<<<<<<<<<< IPSET[%d] %s check >>>>>>>>>>>>>>> \n", set_idx, (rg_db.systemGlobal.ipsetReverseCheck[(set_idx>>5)]&(1<<(set_idx&0x1f)))?"reverse":"obverse");

	rtlglue_printf("========== Hash List (total %d entry) ========== \n", rg_db.systemGlobal.ipsetHashRefCnt[set_idx]);
	for(loop_idx=0,hash_valid_entry=0;loop_idx<MAX_IPSET_HASH_SIZE;loop_idx++){
		if(!list_empty(&rg_db.systemGlobal.ipsetHashListHead[set_idx][loop_idx])){
			list_for_each_entry(ipset_rule,&rg_db.systemGlobal.ipsetHashListHead[set_idx][loop_idx],ipset_list)
			{
				_dump_rg_ipset_rule(set_idx, _rtk_rg_list_entry_idx(ipset_rule, &(rg_db.systemGlobal.ipset_entry[set_idx][0])), ipset_rule->ipsetRule, loop_idx);
				hash_valid_entry++;
			}
		}
		if(hash_valid_entry == rg_db.systemGlobal.ipsetHashRefCnt[set_idx])
			break;
	}
	rtlglue_printf("========== Non Hash List ========== \n");
	if(!list_empty(&rg_db.systemGlobal.ipsetListHead[set_idx])){
		list_for_each_entry(ipset_rule,&rg_db.systemGlobal.ipsetListHead[set_idx],ipset_list)
		{
			_dump_rg_ipset_rule(set_idx, _rtk_rg_list_entry_idx(ipset_rule, &(rg_db.systemGlobal.ipset_entry[set_idx][0])), ipset_rule->ipsetRule, MAX_IPSET_HASH_SIZE);
		}
	}

	}	//set_idx loop

	return len;
}

int32 dump_rg_ipset_by_index( struct file *filp, const char *buff,unsigned long len, void *data ){
	int set_idx;
	int index=_rtk_rg_pasring_proc_string_to_integer(buff,len);

	for(set_idx=0;set_idx<MAX_IPSET_IDX_SIZE;set_idx++)
		_dump_rg_ipset_rule(set_idx, index, rg_db.systemGlobal.ipset_entry[set_idx][index].ipsetRule, MAX_IPSET_HASH_SIZE);

    return len;
}
#endif
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
char *name_of_ipsets_setType[]={
	"MAC",
	"IP",
	"IP_SUBNET",
	"IP_L4PORT",
	"L4PORT_RNG",
	"SETTYPE_END",
};
char *name_of_ipsets_target[]={
	"ACCEPT",
	"DROP",
	"REGECT",
	"LEVEL",
	"TARGET_END",
};

void _dump_rg_ipsets_rule(int set_idx, int index, uint32 hash_idx){

	rtk_rg_ipsets_set_cfg_t *pEntrySet=NULL;
	rtk_rg_ipsets_info_t *pEntry=NULL;

	pEntrySet = &rg_db.systemGlobal.ipsets_setCfg[set_idx].ipsetsCfg;
	pEntry = &rg_db.systemGlobal.ipsets_entry[set_idx][index].ipsetsRule;

	rtlglue_printf("IPSETS[%3d][%4d](hash %3d)(%sfilter %s %s %s): ", set_idx, index, hash_idx, ((hash_idx == (MAX_IPSETS_HASH_SIZE-1)) && (pEntrySet->force_check == TRUE))?"force ":"",
		pEntrySet->protocol?"IPv6":"IPv4", pEntrySet->direction?"dest":"src", name_of_ipsets_setType[pEntrySet->set_type]);

	switch(pEntrySet->set_type){
		case RTK_RG_IPSETS_SET_TYPE_MAC:
			rtlglue_printf("mac %pM", pEntry->mac.octet);
			break;
		case RTK_RG_IPSETS_SET_TYPE_IP:
			if(pEntrySet->protocol){ //ipv6
				rtlglue_printf("addr %s", _rtk_rg_inet_n6toa(pEntry->ipv6_address));
			}else{
				rtlglue_printf("addr %s", inet_ntoa(pEntry->ipv4_address));
			}
			break;
		case RTK_RG_IPSETS_SET_TYPE_IP_SUBNET:
			if(pEntrySet->protocol){ //ipv6
				rtlglue_printf("addr %s/%d", _rtk_rg_inet_n6toa(pEntry->ipv6_address), pEntry->ip_suffix_len);
			}else{
				rtlglue_printf("addr %s/%d", inet_ntoa(pEntry->ipv4_address), pEntry->ip_suffix_len);
			}
			break;
		case RTK_RG_IPSETS_SET_TYPE_IP_L4PORT:
			if(pEntrySet->protocol){ //ipv6
				rtlglue_printf("addr %s:%d", _rtk_rg_inet_n6toa(pEntry->ipv6_address), pEntry->l4_port);
			}else{
				rtlglue_printf("addr %s:%d", inet_ntoa(pEntry->ipv4_address), pEntry->l4_port);
			}
			break;
		case RTK_RG_IPSETS_SET_TYPE_L4PORT_RANGE:
			rtlglue_printf("%sport %d-%d", ((pEntry->l4_proto==RTK_RG_IPSETS_RULE_PROTO_UDP)?"udp ":((pEntry->l4_proto==RTK_RG_IPSETS_RULE_PROTO_TCP)?"tcp ":"")), pEntry->l4_port_start, pEntry->l4_port_end);
			break;
		default:
			break;
	}

	rtlglue_printf("\n");

	return;
}

int dump_rg_ipsets(struct seq_file *s, void *v){
	int len=0;
	rtk_rg_ipsets_group_list_t *pEntrySet=NULL, *pNextEntrySet;
	rtk_rg_ipsets_list_t *pEntry=NULL, *pNextEntry;
	int grp_idx, set_idx, loop_idx;

	rtlglue_printf("Group sorting: \n");
	for(grp_idx=0;grp_idx<MAX_IPSETS_GROUP_SIZE;grp_idx++){
		rtlglue_printf("GRP[%d]:p(%d) ", rg_db.systemGlobal.ipsetsGroupPriority[grp_idx], rg_db.systemGlobal.ipsets_grpCfg[rg_db.systemGlobal.ipsetsGroupPriority[grp_idx]].priority);
		if(grp_idx < (MAX_IPSETS_GROUP_SIZE-1))
			rtlglue_printf("> ");
	}
	rtlglue_printf("\n");

	rtlglue_printf("\t\tSource Ref Cnt\t\t\t\t\tDestination Ref Cnt\n");
	rtlglue_printf("\tSMAC \tIP \tIP/sub  IP:port PortRng\t\tDMAC \tIP \tIP/sub  IP:port PortRng \n");
	for(grp_idx=0;grp_idx<RTK_RG_IPSETS_SET_PROTO_END;grp_idx++){
		rtlglue_printf("%s:\t", grp_idx?"IPv6":"IPv4");
		for(set_idx=0;set_idx<RTK_RG_IPSETS_SET_DIR_END;set_idx++){
			for(loop_idx=0;loop_idx<RTK_RG_IPSETS_SET_TYPE_END;loop_idx++){
				rtlglue_printf("%d \t", rg_db.systemGlobal.ipsetsRefCnt[grp_idx][set_idx][loop_idx]);
			}
			rtlglue_printf("\t");
		}
		rtlglue_printf("\n");
	}

	for(grp_idx=0;grp_idx<MAX_IPSETS_GROUP_SIZE;grp_idx++){

	if(!_rtk_rg_list_head_empty(&rg_db.systemGlobal.ipsetsGroupListHead[grp_idx])){
		if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_ACCEPT)
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, target %s to intf %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].netifIdx);
		else if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_LEVEL)
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, target %s to priority %d for egr wlan idx %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].level, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].egr_wlan_dev_idx);
		else
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, target %s >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target]);

		_rtk_rg_list_for_each_entry_safe(pEntrySet, pNextEntrySet, &(rg_db.systemGlobal.ipsets_setCfg[0]), &rg_db.systemGlobal.ipsetsGroupListHead[grp_idx], ipsets_group_idxList){
			set_idx = _rtk_rg_list_entry_idx(pEntrySet, &(rg_db.systemGlobal.ipsets_setCfg[0]));

			for(loop_idx=0;loop_idx<MAX_IPSETS_HASH_SIZE;loop_idx++){
				if(!_rtk_rg_list_head_empty(&rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx])){
					_rtk_rg_list_for_each_entry_safe(pEntry, pNextEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0]), &rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx], ipsets_idxList){
						_dump_rg_ipsets_rule(set_idx, _rtk_rg_list_entry_idx(pEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0])), loop_idx);
					}
				}
			}

		}
	}

	}	//grp_idx loop

	return len;
}

int32 dump_rg_ipsets_by_index( struct file *filp, const char *buff,unsigned long len, void *data ){
	int index=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	rtk_rg_ipsets_group_list_t *pEntrySet=NULL, *pNextEntrySet;
	rtk_rg_ipsets_list_t *pEntry=NULL, *pNextEntry;
	int set_idx, grp_idx, loop_idx;

	if(index >= MAX_IPSETS_IDX_SIZE){
		rtlglue_printf("IPSETS[invalid set index %d]\n", index);
		return len;
	}

	if(index >= 0){	//dump set info
		set_idx = index;
		grp_idx = rg_db.systemGlobal.ipsets_setCfg[set_idx].ipsetsCfg.group_idx;
		if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_ACCEPT)
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, %s to intf %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].netifIdx);
		else if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_LEVEL)
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, target %s to priority %d for egr wlan idx %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].level, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].egr_wlan_dev_idx);
		else
			rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, %s >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target]);
		for(loop_idx=0;loop_idx<MAX_IPSETS_HASH_SIZE;loop_idx++){
			if(!_rtk_rg_list_head_empty(&rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx])){
				_rtk_rg_list_for_each_entry_safe(pEntry, pNextEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0]), &rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx], ipsets_idxList){
					_dump_rg_ipsets_rule(set_idx, _rtk_rg_list_entry_idx(pEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0])), loop_idx);
				}
			}
		}
	}else{	//dump rule info

		for(grp_idx=0;grp_idx<MAX_IPSETS_GROUP_SIZE;grp_idx++){
			if(!_rtk_rg_list_head_empty(&rg_db.systemGlobal.ipsetsGroupListHead[grp_idx])){
				_rtk_rg_list_for_each_entry_safe(pEntrySet, pNextEntrySet, &(rg_db.systemGlobal.ipsets_setCfg[0]), &rg_db.systemGlobal.ipsetsGroupListHead[grp_idx], ipsets_group_idxList){
					if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_ACCEPT)
						rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, %s to intf %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].netifIdx);
					else if(rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target == RTK_RG_IPSETS_TARGET_LEVEL)
						rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, target %s to priority %d for egr wlan idx %d >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target], rg_db.systemGlobal.ipsets_grpCfg[grp_idx].level, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].egr_wlan_dev_idx);
					else
						rtlglue_printf("<<<<<<<<<< IPSETS Group[%d] sets %s check, %s >>>>>>>>>>>>>>> \n", grp_idx, rg_db.systemGlobal.ipsets_grpCfg[grp_idx].operation?"OR":"AND", name_of_ipsets_target[rg_db.systemGlobal.ipsets_grpCfg[grp_idx].target]);

					set_idx = _rtk_rg_list_entry_idx(pEntrySet, &(rg_db.systemGlobal.ipsets_setCfg[0]));
					for(loop_idx=0;loop_idx<MAX_IPSETS_HASH_SIZE;loop_idx++){
						if(!_rtk_rg_list_head_empty(&rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx])){
							_rtk_rg_list_for_each_entry_safe(pEntry, pNextEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0]), &rg_db.systemGlobal.ipsetsHashListHead[set_idx][loop_idx], ipsets_idxList){
								if((0-index) != _rtk_rg_list_entry_idx(pEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0])))
									continue;
								_dump_rg_ipsets_rule(set_idx, _rtk_rg_list_entry_idx(pEntry, &(rg_db.systemGlobal.ipsets_entry[set_idx][0])), loop_idx);
							}
						}
					}
				}
			}
		}	//grp_idx loop

	}

    return len;
}

#endif

int32 dump_rg_acl(struct seq_file *s, void *v){
	int len=0;
	_dump_rg_acl(s);
	return len;
}


int32 dump_gatewayServicePort(struct seq_file *s, void *v){
	int i;
	int len=0;
	for(i=0;i<MAX_GATEWAYSERVICEPORT_TABLE_SIZE;i++){
		if(rg_db.gatewayServicePortEntry[i].valid==ENABLED){
			PROC_PRINTF("gatewayServicePortEntry[%d] port_num=%d type=%s\n",i,rg_db.gatewayServicePortEntry[i].port_num,rg_db.gatewayServicePortEntry[i].type?"CLIENT(sport)":"SERVER(dport)");
		}
	}
	return len;
}


#if defined(CONFIG_RG_G3_SERIES)
int32 dump_acl_ca(struct seq_file *s, void *v)
{
	int len = 0;
	_dump_acl_ca(s);
    return len;
}

int32 dump_acl_ca_by_index( struct file *filp, const char *buff,unsigned long len, void *data )
{
	_dump_acl_ca_by_index(filp, buff, len, data);
    return len;
}
#endif


int32 dump_acl(struct seq_file *s, void *v)
{
	int len = 0;
	_dump_acl(s);
    return len;
}

int32 dump_acl_by_index( struct file *filp, const char *buff,unsigned long len, void *data )
{
#if defined(CONFIG_RG_G3_SERIES)
	_dump_acl_by_index(filp, buff, len, data);
#else
	rtlglue_printf("Not support.\n");
#endif
    return len;
}

int32 dump_cf(struct seq_file *s, void *v)
{
	int len = 0;
	_dump_cf(s);
    return len;
}


int32 dump_rg_cf(struct seq_file *s, void *v){
	int len=0;

	_dump_rg_cf(s);

	return len;
}


int32 dump_rg_acl_reserved_info(struct seq_file *s, void *v){
	int len=0;

	//record current rg_kernel.debug_level
	rtk_rg_debug_level_t debug_level;
	debug_level = rg_kernel.debug_level;
	rg_kernel.debug_level = RTK_RG_DEBUG_LEVEL_ACL_RRESERVED;

	//show reserved ACL info
	_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_TAIL_END, NULL);


	//roll-back rg_kernel.debug_level
	rg_kernel.debug_level = debug_level;

	return len;
}

int32 dump_rg_urlFilter(struct seq_file *s, void *v)
{
	rtk_rg_urlFilterEntry_t *urlEntry=NULL;
	int i;
	char *keyword; //urlfilter url_filter_string
	char *path;//urlfilter path_filter_string
	int len=0;
	PROC_PRINTF("%s Mode \n",rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK?"Black-list":"White_list");

	list_for_each_entry(urlEntry,&rg_db.systemGlobal.urlFilterByString,urlfilter_list)
	{
		keyword= urlEntry->urlFilter.url_filter_string;
		path = urlEntry->urlFilter.path_filter_string;
		PROC_PRINTF("========urlFilter[%d]=========\n",urlEntry->urlEntryIdx);
		PROC_PRINTF("keyword: %s\n",keyword);
		PROC_PRINTF("path: %s\n",path);
		PROC_PRINTF("BlockAllowTimes: %d\n",urlEntry->urlFilter.urlBlockAllowTimes);
		PROC_PRINTF("path_exactly_match: %d\n",urlEntry->urlFilter.path_exactly_match);
	}

	for(i=0 ; i<URL_FILTER_MAC_HASH_SIZE ; i++)
	{
		list_for_each_entry(urlEntry,&rg_db.systemGlobal.urlFilterBySmacHash[i],urlfilter_list)
		{
			keyword= urlEntry->urlFilter.url_filter_string;
			path = urlEntry->urlFilter.path_filter_string;
			PROC_PRINTF("========urlFilter[%d]=========\n",urlEntry->urlEntryIdx);
			PROC_PRINTF("SAMC %pM  Mode:%s\n",urlEntry->urlFilter.urlfilterSamc,urlEntry->urlFilter.urlfilterSmacMode==RG_FILTER_BLACK?"RG_FILTER_BLACK":"RG_FILTER_WHITE");
			PROC_PRINTF("keyword: %s\n",keyword);
			PROC_PRINTF("path: %s\n",path);
			PROC_PRINTF("BlockAllowTimes: %d\n",urlEntry->urlFilter.urlBlockAllowTimes);
			PROC_PRINTF("path_exactly_match: %d\n",urlEntry->urlFilter.path_exactly_match);
		}
	}

	return len;
}


int32 dump_rg_urlPri(struct seq_file *s, void *v)
{
	rtk_rg_hiPriEntryIdx_t* p_hiPriEntryIdx=NULL;
	int i;
	int len=0;

	for(i=0 ;i<MAX_URL_HiPri_ENTRY_SIZE ; i++)
	{
		if(!rg_db.systemGlobal.urlHiPri_table_entry[i].valid)
			continue;
		PROC_PRINTF("[%d] url:%s path:%s  path_exactly_match:%d AssignPri:%d \n",i,rg_db.systemGlobal.urlHiPri_table_entry[i].urlHighPriEt.url_filter_string,rg_db.systemGlobal.urlHiPri_table_entry[i].urlHighPriEt.path_filter_string,
		rg_db.systemGlobal.urlHiPri_table_entry[i].urlHighPriEt.path_exactly_match,rg_db.systemGlobal.urlHiPri_table_entry[i].urlHighPriEt.urlpri);
		PROC_PRINTF("hiPriEntryIdx:");
		list_for_each_entry(p_hiPriEntryIdx,&rg_db.systemGlobal.urlHiPri_table_entry[i].hiPriEntryIdxListHdr,hiPriEntryIdx_list)
		{
			PROC_PRINTF("naptIdx[%d]:[%d/%d]",p_hiPriEntryIdx->connectId,p_hiPriEntryIdx->hiPriEntryIdx,p_hiPriEntryIdx->hiPriEntryIdx2);
		}
		PROC_PRINTF("\n");
	}

	return len;
}


int32 mibdump_port_isolation(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	for(i=RTK_RG_PORT0;i<RTK_RG_PORT_MAX;i++)
	{
		if(RG_INVALID_PORT(i)) continue;
		if(i<=RTK_RG_PORT_LASTCPU)
			{PROC_PRINTF("Port[%d] isolation portmask %x\n",i,rg_db.systemGlobal.portIsolation[i].portmask);}
		else
			{PROC_PRINTF("extPort[%d] isolation portmask %x\n",i-RTK_RG_EXT_PORT0,rg_db.systemGlobal.portIsolation[i].portmask);}

	}
	return len;
}

int32 mibdump_redirect_httpAll(struct seq_file *s, void *v)
{
	int len=0;

	if(rg_db.redirectHttpAll.enable==0)
		PROC_PRINTF("Disable redirect.\n");
	else
		PROC_PRINTF("Redirect Http Info:enable=%u, count=%d\n%s\n",rg_db.redirectHttpAll.enable,rg_db.redirectHttpAll.count,rg_db.redirectHttpAll.pushweb);

	return len;
}

int32 mibdump_redirect_httpCount(struct seq_file *s, void *v)
{
	int len=0;

	if(rg_db.redirectHttpCount.enable==0)
		PROC_PRINTF("Disable redirect.\n");
	else
		PROC_PRINTF("Redirect Http Info:enable=%u count=%d, denialSec=%u\n%s\n",rg_db.redirectHttpCount.enable,rg_db.redirectHttpCount.count,rg_db.redirectHttpCount.denialSecs,rg_db.redirectHttpCount.pushweb);

	return len;
}

int32 mibdump_redirect_httpURL(struct seq_file *s, void *v)
{
	int len=0,count;
	rtk_rg_redirectHttpURL_linkList_t *pRedEntry;

	if(list_empty(&rg_db.redirectHttpURLListHead)){
		PROC_PRINTF("No redirect URL setting.\n");
	}else{
		PROC_PRINTF("Redirect URL setting:\n");
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpURLListHead,url_list){
			count=atomic_read(&pRedEntry->count);
			PROC_PRINTF("    URL:%s DST_URL:%s ",pRedEntry->url_data.url_str,pRedEntry->url_data.dst_url_str);
			if(count==0)
				PROC_PRINTF("disable redirect.\n");
			else if(count<0)
				PROC_PRINTF("Non-stop redirect.\n");
			else
				PROC_PRINTF("Count:%d\n",count);
		}
	}

	return len;
}

int32 mibdump_redirect_httpWhiteList(struct seq_file *s, void *v)
{
	int len=0;
	rtk_rg_redirectHttpWhiteList_linkList_t *pRedEntry;

	if(list_empty(&rg_db.redirectHttpWhiteListListHead)){
		PROC_PRINTF("No redirect WhiteList setting.\n");
	}else{
		PROC_PRINTF("Redirect WhiteList setting:\n");
		list_for_each_entry(pRedEntry,&rg_db.redirectHttpWhiteListListHead,white_list){
			PROC_PRINTF("    URL:%s Keyword:%s\n",pRedEntry->white_data.url_str,pRedEntry->white_data.keyword_str);
		}
	}
	return len;
}

#if 0 // for code size issue, this function is disabled.


int32 dump_reg(struct seq_file *s, void *v)
{
	int len=0;

#ifdef CONFIG_APOLLO_MODEL
	PROC_PRINTF("X86 Model do not support register dump!!\n");
#else
	//reg_array_field_read(APOLLOMP_XXXr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_XXXf, &val);
	int32 retv;
	uint32 val,val2;
	//rtk_portmask_t pmsk;
	//rtk_vlan_protoVlanCfg_t ppbCfg;
	int i,j;

	retv=0;val=0;val2=0;i=0;j=0;
#ifdef CONFIG_APOLLO_RLE0371
#else
	//rtk_vlan_protoGroup_t protoGroup;
#endif

#ifdef CONFIG_APOLLO_RLE0371
		//LUT regs
	PROC_PRINTF("\n-----LUT-----\n");

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_UNKN_UC_DA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_UC_DA_BEHAVE[%d]:0x%x\n",i,val);
	}
	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_UNKN_UC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_UNKN_UC_FLOOD[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_BC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_BC_FLOOD[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_UNKN_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_SA_BEHAVE[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_UNMATCHED_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNMATCHED_SA_BEHAVE[%d]:0x%x\n",i,val);
	}
	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(L2_SRC_PORT_PERMITr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_L2_SRC_PORT_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);
	}
	for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
	{
		reg_array_field_read(L2_SRC_EXT_PERMITr, REG_ARRAY_INDEX_NONE,i, ENf, &val);
		PROC_PRINTF("reg_L2_SRC_EXT_PERMIT[%d]:0x%x\n",i,(val>>(i-1))&0x1);
	}

	//Port Security regs
	PROC_PRINTF("\n-----Port Security-----\n");


	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_LRN_LIMITNOr, i, REG_ARRAY_INDEX_NONE, NUMf, &val);
		PROC_PRINTF("reg_LUT_LRN_LIMITNO[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_LEARN_OVER_CTRLr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_LUT_LEARN_OVER_ACT[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_DIS_AGEr, i, REG_ARRAY_INDEX_NONE, DIS_AGEf, &val);
		PROC_PRINTF("reg_LUT_DISABLE_AGE[%d]: %d\n",i,val);
	}


	reg_field_read(LUT_CFGr,  LUT_IPMC_HASHf, &val);
	PROC_PRINTF("reg_LUT_IPMC_HASH:0x%x\n",val);

	reg_field_read(LUT_CFGr,  LUT_IPMC_LOOKUP_OPf, &val);

	PROC_PRINTF("reg_LUT_IPMC_LOOKUP_OP:0x%x\n",val);

	//Multicast
	PROC_PRINTF("\n-----Multicast-----\n");

	reg_field_read(L34_GLB_CFGr,  L34_GLOBAL_CFGf, &val);
	PROC_PRINTF("reg_L34_Global_Enable: 0x%x\n",val);

	//Unknown Multicast Control
	PROC_PRINTF("\n-----Unknown Multicast Control-----\n");


	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(UNKN_IP4_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_IP4_MC_ACT[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(UNKN_IP6_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_IP6_MC_ACT[%d]:0x%x\n",i,val);
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(UNKN_L2_MCr, i, REG_ARRAY_INDEX_NONE, ACTf, &val);
		PROC_PRINTF("reg_UNKN_L2_MC_ACT[%d]:0x%x\n",i,val);
	}


	PROC_PRINTF("reg_UNKNOWN_MCAST_TRAP_PRIORITY: not found\n");
	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(LUT_UNKN_MC_FLOODr, i, REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_LUT_UNKN_MC_FLOOD[%d]:0x%x\n",i,val);
	}


	// QOS-Ingress Priority regs
	PROC_PRINTF("\n----- QOS-Ingress Priority-----\n");

	for(i=0; i<8; i++)
	{
		reg_array_field_read(QOS_1Q_PRI_REMAPr,REG_ARRAY_INDEX_NONE,i, INTPRI_1Qf, &val);
		PROC_PRINTF("reg_DOT1Q_PRI_PRIORITY[%d]:0x%x\n",i,val);//QOS_1Q_PRI_REMAP
	}

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(QOS_PB_PRIr, i, REG_ARRAY_INDEX_NONE, INTPRI_PBf, &val);
		PROC_PRINTF("reg_QOS_PORT_PRIORITY[%d]:0x%x\n",i,val);//QOS_PB_PRI
	}

	reg_field_read(PRI_SEL_TBL_CTRLr,	PORT_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_PORT_WEIGHT:0x%x\n",val); //PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	DOT1Q_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_1Q_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	DSCP_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_DSCP_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	ACL_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_ACL_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	CVLAN_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_CVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	LUTFWD_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_LUTFWD_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	SA_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_SA_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRLr,	SVLAN_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_SVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
	reg_field_read(PRI_SEL_TBL_CTRL2r,  L4_WEIGHTf, &val);
	PROC_PRINTF("reg_QOS_L4_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL2


	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(QOS_PORT_QMAP_CTRLr,i,REG_ARRAY_INDEX_NONE, IDXf, &val);
		PROC_PRINTF("reg_PORT_QTABLE_INDEX[%d]:0x%x\n",i,val);//QOS_PRI_REMAP_IN_CPU
	}

	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			reg_array_field_read(QOS_INTPRI_TO_QIDr,i,j, PRI_TO_QIDf, &val);
			PROC_PRINTF("reg_QOS_PRIORITY_TO_QID_TABLE[%d][%d]:0x%x\n",i,j,val);//QOS_INTPRI_TO_QID
		}
	}

	// QOS-Remarking
	PROC_PRINTF("\n----- QOS-Remarking-----\n");

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(RMK_DOT1Q_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_REMARKING_1Q_PORT_ENABLE[%d]:0x%x\n",i,val);//RMK_DOT1Q_RMK_EN_CTRL
	}

//ioal_mem32_read(0x0231CC,&val);
	for(i=0; i<8; i++)
	{
		reg_array_field_read(RMK_1Q_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_1Qf, &val);
		PROC_PRINTF("reg_INTPRI_1QPRI[%d]:0x%x\n",i,val); //RMK_1Q_CTRL
	}

	//Qos
for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
    {
            reg_array_field_read(RMK_DSCP_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
            PROC_PRINTF("reg_RMK_DSCP_RMK_EN_CTRL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
    {
            reg_array_field_read(RMK_P_DSCP_SELr,REG_ARRAY_INDEX_NONE,i, SELf, &val);
            PROC_PRINTF("reg_RMK_DSCP_CFG_SEL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<64; i++)
    {
            reg_array_field_read(QOS_DSCP_REMAPr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_QOS_DSCP_REMAP[%d]:%d\n",i,val);
    }

    for(i=0; i<8; i++)
    {
            reg_array_field_read(RMK_DSCP_INT_PRI_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_RMK_DSCP_INT_PRI_CTRL[%d]:0x%x\n",i,val);
    }

    for(i=0; i<64; i++)
    {
            reg_array_field_read(RMK_DSCP_CTRLr,REG_ARRAY_INDEX_NONE,i, INTPRI_DSCPf, &val);
            PROC_PRINTF("reg_RMK_DSCP_CTRL[%d]:0x%x\n",i,val);
    }
#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//LUT regs
			PROC_PRINTF("\n-----LUT-----\n");

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_UC_DA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_UC_DA_BEHAVE[%d]:0x%x\n",i,val);
			}
			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_UC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_UNKN_UC_FLOOD[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_BC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_BC_FLOOD[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_SA_BEHAVE[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNMATCHED_SA_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNMATCHED_SA_BEHAVE[%d]:0x%x\n",i,val);
			}
			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_L2_SRC_PORT_PERMITr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_L2_SRC_PORT_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);
			}
			for(i=0; i<RTK_RG_MAX_EXT_PORT-1; i++)
			{
				reg_array_field_read(APOLLOMP_L2_SRC_EXT_PERMITr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_L2_SRC_EXT_PERMIT[%d]:0x%x\n",i,(val>>(i-1))&0x1);
			}

			//Port Security regs
			PROC_PRINTF("\n-----Port Security-----\n");


			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_LRN_LIMITNOr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_NUMf, &val);
				PROC_PRINTF("reg_LUT_LRN_LIMITNO[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_LEARN_OVER_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_LUT_LEARN_OVER_ACT[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_AGEOUT_CTRLr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_AGEOUT_OUTf, &val);
				PROC_PRINTF("reg_LUT_DISABLE_AGE[%d]: %d\n",i,val);
			}


			reg_field_read(APOLLOMP_LUT_CFGr,  APOLLOMP_LUT_IPMC_HASHf, &val);
			PROC_PRINTF("reg_LUT_IPMC_HASH:0x%x\n",val);

			reg_field_read(APOLLOMP_LUT_CFGr,  APOLLOMP_LUT_IPMC_LOOKUP_OPf, &val);

			PROC_PRINTF("reg_LUT_IPMC_LOOKUP_OP:0x%x\n",val);

			//Multicast
			PROC_PRINTF("\n-----Multicast-----\n");

			reg_field_read(APOLLOMP_L34_GLB_CFGr,  APOLLOMP_L34_GLOBAL_CFGf, &val);
			PROC_PRINTF("reg_L34_Global_Enable: 0x%x\n",val);

			reg_field_read(APOLLOMP_L34_IPMC_TTL_CFGr,	APOLLOMP_IP_MCST_TTL_1f, &val);
			PROC_PRINTF("reg_IP_MULTICAST_ROUTE_TTL_1: %d\n",val);

			//Unknown Multicast Control
			PROC_PRINTF("\n-----Unknown Multicast Control-----\n");


			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_IP4_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_IP4_MC_ACT[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_IP6_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_IP6_MC_ACT[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_UNKN_L2_MCr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ACTf, &val);
				PROC_PRINTF("reg_UNKN_L2_MC_ACT[%d]:0x%x\n",i,val);
			}


			PROC_PRINTF("reg_UNKNOWN_MCAST_TRAP_PRIORITY: not found\n");
			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_LUT_UNKN_MC_FLOODr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_LUT_UNKN_MC_FLOOD[%d]:0x%x\n",i,val);
			}


			// QOS-Ingress Priority regs
			PROC_PRINTF("\n----- QOS-Ingress Priority-----\n");

			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_1Q_PRI_REMAPr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_1Qf, &val);
				PROC_PRINTF("reg_DOT1Q_PRI_PRIORITY[%d]:0x%x\n",i,val);//QOS_1Q_PRI_REMAP
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_PB_PRIr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_INTPRI_PBf, &val);
				PROC_PRINTF("reg_QOS_PORT_PRIORITY[%d]:0x%x\n",i,val);//QOS_PB_PRI
			}

			//QOS_DSCP_REMAP
			for(i=0; i<64; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_DSCP_REMAPr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_QOS_DSCP_REMAP[%d]:%d\n",i,val);
			}

			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_PORT_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_PORT_WEIGHT:0x%x\n",val); //PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_DOT1Q_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_1Q_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_DSCP_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_DSCP_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_ACL_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_ACL_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_CVLAN_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_CVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_LUTFWD_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_LUTFWD_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_SA_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_SA_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRLr,	APOLLOMP_SVLAN_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_SVLAN_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL
			reg_field_read(APOLLOMP_PRI_SEL_TBL_CTRL2r,  APOLLOMP_L4_WEIGHTf, &val);
			PROC_PRINTF("reg_QOS_L4_WEIGHT:0x%x\n",val);//PRI_SEL_TBL_CTRL2


			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_QOS_PORT_QMAP_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_IDXf, &val);
				PROC_PRINTF("reg_PORT_QTABLE_INDEX[%d]:0x%x\n",i,val);//QOS_PRI_REMAP_IN_CPU
			}

			for(i=0; i<4; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_QOS_INTPRI_TO_QIDr,i,j, APOLLOMP_PRI_TO_QIDf, &val);
					PROC_PRINTF("reg_QOS_PRIORITY_TO_QID_TABLE[%d][%d]:0x%x\n",i,j,val);//QOS_INTPRI_TO_QID
				}
			}

			// QOS-Remarking
			PROC_PRINTF("\n----- QOS-Remarking-----\n");

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DOT1Q_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_REMARKING_1Q_PORT_ENABLE[%d]:0x%x\n",i,val);//RMK_DOT1Q_RMK_EN_CTRL
			}

			ioal_mem32_read(0x0231CC,&val);
			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_1Q_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_1Qf, &val);
				PROC_PRINTF("reg_INTPRI_1QPRI[%d]:0x%x\n",i,val); //RMK_1Q_CTRL
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_RMK_EN_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_RMK_DSCP_RMK_EN_CTRL[%d]:0x%x\n",i,val);
			}

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_P_DSCP_SELr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_SELf, &val);
				PROC_PRINTF("reg_RMK_DSCP_CFG_SEL[%d]:0x%x\n",i,val);
			}

			for(i=0; i<8; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_INT_PRI_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_RMK_DSCP_INT_PRI_CTRL[%d]:0x%x\n",i,val);
			}

			for(i=0; i<64; i++)
			{
				reg_array_field_read(APOLLOMP_RMK_DSCP_CTRLr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_INTPRI_DSCPf, &val);
				PROC_PRINTF("reg_RMK_DSCP_CTRL[%d]:0x%x\n",i,val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

	//ACL regs
#ifdef CONFIG_APOLLO_RLE0371
	PROC_PRINTF("\n----- ACL-----\n");
	retv = reg_field_read(ACL_CFGr, MODEf, &val);
	assert(retv == RT_ERR_OK);
	PROC_PRINTF("reg_ACL_MODE:0x%x\n", val); /*0: 8 field template mode	 1: 4 field template + 3 field template */

	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(ACL_ENr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,val);	/*per port enable/disable ACL*/
	}
	for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
	{
		reg_array_field_read(ACL_PERMITr,i,REG_ARRAY_INDEX_NONE, PERMITf, &val);
		PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,val);	/*per port permit/drop frame while ACL rule unhit*/
	}

	retv = reg_field_read(CF_CFGr, CF_US_PERMITf, &val);
	assert(retv == RT_ERR_OK);
	PROC_PRINTF("reg_CF_US_PERMIT:0x%x\n", val); /*per port permit/drop frame while CF rule unhit*/

#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			PROC_PRINTF("\n----- ACL-----\n");
			//ioal_mem32_read(0x01530C,&val);
			//PROC_PRINTF("reg_ACL_MODE:0x%x\n",val&0x1); /*0: 8 field template mode   1: 4 field template + 3 field template */
			retv = reg_field_read(APOLLOMP_ACL_CFGr, APOLLOMP_MODEf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ACL_MODE:0x%x\n", val); /*0: 8 field template mode	 1: 4 field template + 3 field template */


			//ioal_mem32_read(0x015104,&val);
			//for(i=0; i<MAX_APOLLO_PORT; i++)
			//{
			//	  PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,(val>>i)&0x1);/*per port enable/disable ACL*/
			//}
			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_ACL_ENr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				PROC_PRINTF("reg_ACL_EN[%d]:0x%x\n",i,val);	/*per port enable/disable ACL*/
			}

			//ioal_mem32_read(0x015108,&val);
			//for(i=0; i<MAX_APOLLO_PORT; i++)
			//{
			//	  PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,(val>>i)&0x1);/*per port permit/drop frame while ACL rule unhit*/
			//}
			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_ACL_PERMITr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PERMITf, &val);
				PROC_PRINTF("reg_ACL_PERMIT[%d]:0x%x\n",i,val);	/*per port permit/drop frame while ACL rule unhit*/
			}


			//check hit reason
			//ioal_mem32_read(0x01C0CC,&val);
			//PROC_PRINTF("STAT_ACL_REASON(0x01C0CC):0x%x\n",val);
			for(i=0; i<6; i++)
			{
				reg_array_field_read(APOLLOMP_STAT_ACL_REASONr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_ACL_HIT_INFOf, &val);
				PROC_PRINTF("STAT_ACL_REASON[%d]:0x%x\n",i,val);
			}

			//ioal_mem32_read(0x01C0D0,&val_1);
			//PROC_PRINTF("STAT_ACL_REASON(0x01C0D0):0x%x\n",val_1);

		//Classification regs
			PROC_PRINTF("\n----- Classification-----\n");
			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_SEL_PON_EN:0x%x\n",(val&0x4)>>2);	/*enable/disable PON port as CF port*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_SEL_PON_ENf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_SEL_PON_EN:0x%x\n", val);	/*enable/disable PON port as CF port*/

			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_SEL_RGMII_EN:0x%x\n",(val&0x8)>>3);/*enable/disable RGMII port as CF port*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_SEL_RGMII_ENf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_SEL_RGMII_EN:0x%x\n", val);	/*enable/disable RGMII port as CF port*/


			//ioal_mem32_read(0x0150E0,&val);
			//PROC_PRINTF("reg_CF_PERMIT:0x%x\n",(val&0x3)); /*per port permit/drop frame while CF rule unhit*/
			retv = reg_field_read(APOLLOMP_CF_CFGr, APOLLOMP_CF_US_PERMITf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_CF_US_PERMIT:0x%x\n", val); /*per port permit/drop frame while CF rule unhit*/
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif


	//vlan
	PROC_PRINTF("\n-----vlan-----\n");
#ifdef CONFIG_APOLLO_RLE0371
	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PB_EFIDENr, i, REG_ARRAY_INDEX_NONE, PBFIDENf, &val);
		reg_array_field_read(VLAN_PB_EFIDr, i, REG_ARRAY_INDEX_NONE, PBFIDf, &val2);
		//assert(rtk_vlan_portFid_get(i, &val, &val2) == RT_ERR_OK);
		PROC_PRINTF("reg_PORTn_PBFIDEN[port%d]:0x%x,%s\n",i,val2,
			val==0?"DISABLED":"ENABLED");
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_EGRESS_TAGr, i, REG_ARRAY_INDEX_NONE, EGRESS_MODEf, &val);
		//assert(rtk_vlan_tagMode_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_EGRESS_MODE[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++)
	{
		//bzero(pmsk.bits,sizeof(pmsk.bits));
		reg_array_field_read(VLAN_EGRESS_KEEPr, i, REG_ARRAY_INDEX_NONE, MBRf, &val);
		//assert(rtk_vlan_portEgrTagKeepType_get(i, &pmsk, &val2)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_EGRESS_PORTn_VLAN_KEEP[port%d]:0x%x\n",i,val);
	}

	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VLAN_FILTERINGf, &val);
	//assert(rtk_vlan_vlanFunctionEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_FILTERING:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, TRANSPARENT_ENf, &val);
	//assert(rtk_vlan_transparentEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_TRANSPARENT_EN:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, CFI_KEEPf, &val);
	//assert(rtk_vlan_cfiKeepEnable_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_CFI_KEEP:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VID_0_TYPEf, &val);
	//assert(rtk_vlan_reservedVidAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VLAN_VID0_TYPE:0x%x\n",val);
	reg_array_field_read(VLAN_CTRLr, REG_ARRAY_INDEX_NONE,REG_ARRAY_INDEX_NONE, VID_4095_TYPEf, &val2);
	PROC_PRINTF("reg_VLAN_VID4095_TYPE:0x%x\n",val2);

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PORT_ACCEPT_FRAME_TYPEr, i,REG_ARRAY_INDEX_NONE, FRAME_TYPEf, &val);
		//assert(rtk_vlan_portAcceptFrameType_get(i, &val)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORT_ACCEPT_FRAME_TYPE[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_INGRESSr, i,REG_ARRAY_INDEX_NONE, INGRESSf, &val);
		//assert(rtk_vlan_portIgrFilterEnable_get(i, &val)== RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_INGRESS[port%d]:0x%x\n",i,val);
	}

// port-based vlan
	for(i=0; i<=6; i++)
	{
		reg_array_field_read(VLAN_PB_VIDXr, i,REG_ARRAY_INDEX_NONE, VIDXf, &val);
		//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_PORTn_VIDX[port%d]:0x%x\n",i,val);
	}

	for(i=0; i<5; i++)
	{
		reg_array_field_read(VLAN_EXT_VIDXr, i,REG_ARRAY_INDEX_NONE, VIDXf, &val);
		//assert(rtk_vlan_extPortPvid_get(i, &val) == RT_ERR_OK);
		PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort%d]:0x%x\n",i,val);
	}

// port-and-protocol-based vlan

	for(i=0; i<=3; i++)
	{

		reg_array_field_read(VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, FRAME_TYPEf, &val);
		//bzero(&protoGroup,sizeof(protoGroup));
		//assert(rtk_vlan_protoGroup_get(i, &protoGroup)== RT_ERR_OK);
		//PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,protoGroup.frametype);
		PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,val);
		reg_array_field_read(VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, ETHER_TYPEf, &val);
		//PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,protoGroup.framevalue);
		PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,val);
	}

	for(i=0; i<=6; i++) 	//port
	{
		PROC_PRINTF("Port %d:\n",i);
		for(j=0; j<=3; j++)
		{
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, VALIDf, &val);
			if(val==0)continue;	//invalid will pass
			//assert(rtk_vlan_portProtoVlan_get(i,j,&ppbCfg)== RT_ERR_OK);
			//PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,ppbCfg.valid);
			PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,val);
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, PPB_VIDXf, &val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,ppbCfg.vid);
			PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,val);
			reg_array_field_read(VLAN_PORT_PPB_VLANr, i,j, PPB_PRIf, &val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,ppbCfg.pri);
			PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,val);
			//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,ppbCfg.dei);
		}
	}

#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PB_FIDENr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_PBFIDENf, &val);
				reg_array_field_read(APOLLOMP_VLAN_PB_FIDr, i, REG_ARRAY_INDEX_NONE, APOLLOMP_PBFIDf, &val2);
				//assert(rtk_vlan_portFid_get(i, &val, &val2) == RT_ERR_OK);
				PROC_PRINTF("reg_PORTn_PBFIDEN[port%d]:0x%x,%s\n",i,val2,
					val==0?"DISABLED":"ENABLED");
			}

			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_EGRESS_TAGr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_EGRESS_MODEf, &val);
				//assert(rtk_vlan_tagMode_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_EGRESS_MODE[port%d]:0x%x\n",i,val);
			}

			for(i=0; i<=6; i++)
			{
				//bzero(pmsk.bits,sizeof(pmsk.bits));
				reg_array_field_read(APOLLOMP_VLAN_EGRESS_KEEPr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_MBRf, &val);
				//assert(rtk_vlan_portEgrTagKeepType_get(i, &pmsk, &val2)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_EGRESS_PORTn_VLAN_KEEP[port%d]:0x%x\n",i,val);
			}

			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VLAN_FILTERINGf, &val);
			//assert(rtk_vlan_vlanFunctionEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_FILTERING:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_TRANSPARENT_ENf, &val);
			//assert(rtk_vlan_transparentEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_TRANSPARENT_EN:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_CFI_KEEPf, &val);
			//assert(rtk_vlan_cfiKeepEnable_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_CFI_KEEP:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VID_0_TYPEf, &val);
			//assert(rtk_vlan_reservedVidAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_VID0_TYPE:0x%x\n",val);
			reg_field_read(APOLLOMP_VLAN_CTRLr, APOLLOMP_VID_4095_TYPEf, &val2);
			PROC_PRINTF("reg_VLAN_VID4095_TYPE:0x%x\n",val2);

			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PORT_ACCEPT_FRAME_TYPEr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_FRAME_TYPEf, &val);
				//assert(rtk_vlan_portAcceptFrameType_get(i, &val)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORT_ACCEPT_FRAME_TYPE[port%d]:0x%x\n",i,val);
			}

			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_INGRESSr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_INGRESSf, &val);
				//assert(rtk_vlan_portIgrFilterEnable_get(i, &val)== RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_INGRESS[port%d]:0x%x\n",i,val);
			}

		// port-based vlan
			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_PB_VIDXr, i,REG_ARRAY_INDEX_NONE, APOLLOMP_VIDXf, &val);
				//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_PORTn_VIDX[port%d]:0x%x\n",i,val);
			}

			//ext-port0 is cpu port, too
			reg_array_field_read(APOLLOMP_VLAN_PB_VIDXr, 6,REG_ARRAY_INDEX_NONE, APOLLOMP_VIDXf, &val);
			//assert(rtk_vlan_portPvid_get(i, &val) == RT_ERR_OK);
			PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort0]:0x%x\n",val);
			for(i=0; i<5; i++)
			{
				reg_array_field_read(APOLLOMP_VLAN_EXT_VIDXr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_VIDXf, &val);
				//assert(rtk_vlan_extPortPvid_get(i, &val) == RT_ERR_OK);
				PROC_PRINTF("reg_VLAN_EXTn_VIDX[extPort%d]:0x%x\n",i+1,val);
			}

		// port-and-protocol-based vlan

			for(i=0; i<=3; i++)
			{

				reg_array_field_read(APOLLOMP_VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_FRAME_TYPEf, &val);
				//bzero(&protoGroup,sizeof(protoGroup));
				//assert(rtk_vlan_protoGroup_get(i, &protoGroup)== RT_ERR_OK);
				//PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,protoGroup.frametype);
				PROC_PRINTF("reg_VLAN_PPBn_FRAME_TYPE[%d]:0x%x\n",i,val);
				reg_array_field_read(APOLLOMP_VLAN_PPB_VLAN_VALr, REG_ARRAY_INDEX_NONE,i, APOLLOMP_ETHER_TYPEf, &val);
				//PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,protoGroup.framevalue);
				PROC_PRINTF("reg_VLAN_PPBn_ETHERTYPE[%d]:0x%x\n",i,val);
			}

			for(i=0; i<=6; i++) 	//port
			{
				PROC_PRINTF("Port %d:\n",i);
				for(j=0; j<=3; j++)
				{
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_VALIDf, &val);
					//assert(rtk_vlan_portProtoVlan_get(i,j,&ppbCfg)== RT_ERR_OK);
					//PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,ppbCfg.valid);
					if(val==0)continue;
					PROC_PRINTF("	reg_VLAN_PPBn_VALID[%d]:0x%x\n",j,val);
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_PPB_VIDXf, &val);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,ppbCfg.vid);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_VIDX[%d]:0x%x\n",j,val);
					reg_array_field_read(APOLLOMP_VLAN_PORT_PPB_VLANr, i,j, APOLLOMP_PPB_PRIf, &val);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,ppbCfg.pri);
					//PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,ppbCfg.dei);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_PRIORITY[%d]:0x%x\n",j,val);
					PROC_PRINTF("	reg_VLAN_PPBm_PORTn_DEI[%d]:0x%x\n",j,0);
				}
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

	// svlan
	PROC_PRINTF("\n-----svlan-----\n");
#ifdef CONFIG_APOLLO_RLE0371
	reg_field_read(SVLAN_CFGr, VS_TPIDf, &val);
	//assert(rtk_svlan_tpidEntry_get(0,&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_TPID:0x%x\n",val);//SVLAN_CFG

	reg_field_read(SVLAN_CTRLr, VS_PRIf, &val);
	//assert(rtk_svlan_trapPri_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_TRAP_PRI:0x%x\n", val);//VS_TRAP_PRI

	PROC_PRINTF("reg_VS_PMSK: ");//SVLAN_UPLINK_PMSK
	for(i=0,j=0; i<=6; i++)
	{
		reg_array_field_read(SVLAN_UPLINK_PMSKr,REG_ARRAY_INDEX_NONE,i, ENf, &val);
		//assert(rtk_svlan_servicePort_get(i,&val)== RT_ERR_OK);
		if(val==1)
		{
			j=1;
			PROC_PRINTF("%d ",i);
		}
	}
	if(j==0) PROC_PRINTF("X\n");
	else PROC_PRINTF("\n");//SVLAN_UPLINK_PMSK

	reg_field_read(SVLAN_CTRLr, VS_SPRISELf, &val);
	//assert(rtk_svlan_priorityRef_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_SPRISEL:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNTAGf, &val);
	//assert(rtk_svlan_untagAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_UNTAG:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNTAG_SVIDXf, &val2);
	PROC_PRINTF("reg_VS_UNTAG_SVIDX:0x%x\n",val2);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNMATf, &val);
	//assert(rtk_svlan_unmatchAction_get(&val,&val2)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_UNMAT:0x%x\n",val);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_UNMAT_SVIDXf, &val2);
	PROC_PRINTF("reg_VS_UNMAT_SVIDX:0x%x\n",val2);//SVLAN_CTRL

	reg_field_read(SVLAN_CTRLr, VS_CFI_KEEPf, &val);
	//assert(rtk_svlan_deiKeepState_get(&val)== RT_ERR_OK);
	PROC_PRINTF("reg_VS_CFI_KEEP:0x%x\n",val);//SVLAN_CTRL

	for(i=0; i<6; i++)	 //VS_PORTn_SVIDX[0]~[4]
	{
		reg_array_field_read(SVLAN_P_SVIDXr,REG_ARRAY_INDEX_NONE,i, SVIDXf, &val);
		//ASSERT(rtk_svlan_portSvid_get(i,&val)== RT_ERR_OK);
		//val2 = rtk_svlan_portSvid_get(i,&val);
		//if(val2==RT_ERR_SVLAN_INVALID)
			//PROC_PRINTF("Port-based SVALN is invalid\n");
		//else if(val2==RT_ERR_OK)
			PROC_PRINTF("reg_VS_PORTn_SVIDX[%d]:0x%x\n",i,val);//SVLAN_P_SVIDX
	}

	for(i=0; i<=6; i++)
	{
		reg_array_field_read(SVLAN_EP_DMAC_CTRLr,i,REG_ARRAY_INDEX_NONE, ENf, &val);
		//assert(rtk_svlan_dmacVidSelState_get(i,&val)== RT_ERR_OK);
		PROC_PRINTF("reg_VS_PORTn_DMACVIDSE[%d]:0x%x\n", i, val);
	}

#else
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			reg_field_read(APOLLOMP_SVLAN_CFGr, APOLLOMP_VS_TPIDf, &val);
			//assert(rtk_svlan_tpidEntry_get(0,&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_TPID:0x%x\n",val);//SVLAN_CFG
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_TYPEf, &val);
			//assert(rtk_svlan_lookupType_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_LOOK_UP_TYPE:0x%x\n", val);//SVLAN_LOOK_UP_TYPE
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_PRIf, &val);
			//assert(rtk_svlan_trapPri_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_TRAP_PRI:0x%x\n", val);//VS_TRAP_PRI

			PROC_PRINTF("reg_VS_PMSK: ");//SVLAN_UPLINK_PMSK
			for(i=0,j=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_UPLINK_PMSKr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				//assert(rtk_svlan_servicePort_get(i,&val)== RT_ERR_OK);
				if(val==1)
				{
					j=1;
					PROC_PRINTF("%d ",i);
				}
			}
			if(j==0) {PROC_PRINTF("X\n");}
			else {PROC_PRINTF("\n");}//SVLAN_UPLINK_PMSK

			reg_field_read(APOLLOMP_SVLAN_CTRLr,APOLLOMP_FORCED_DMACVIDSELf,&val);
			//assert(rtk_svlan_dmacVidSelForcedState_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_FORCED_DMACVIDSEL:0x%x\n", val);

			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_SPRISELf, &val);
			//assert(rtk_svlan_priorityRef_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_SPRISEL:0x%x\n",val);//SVLAN_CTRL

			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNTAGf, &val);
			//assert(rtk_svlan_untagAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_UNTAG:0x%x\n",val);//SVLAN_CTRL
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNTAG_SVIDXf, &val2);
			PROC_PRINTF("reg_VS_UNTAG_SVIDX:0x%x\n",val2);//SVLAN_CTRL

			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNMATf, &val);
			//assert(rtk_svlan_unmatchAction_get(&val,&val2)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_UNMAT:0x%x\n",val);//SVLAN_CTRL
			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_UNMAT_SVIDXf, &val2);
			PROC_PRINTF("reg_VS_UNMAT_SVIDX:0x%x\n",val2);//SVLAN_CTRL

			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_SP2C_UNMATf, &val);
			//assert(rtk_svlan_sp2cUnmatchCtagging_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_SP2C_UNMAT:0x%x\n",val);//SVLAN_CTRL

			reg_field_read(APOLLOMP_SVLAN_CTRLr, APOLLOMP_VS_DEI_KEEPf, &val);
			//assert(rtk_svlan_deiKeepState_get(&val)== RT_ERR_OK);
			PROC_PRINTF("reg_VS_DEI_KEEP:0x%x\n",val);//SVLAN_CTRL

			for(i=0; i<6; i++)	 //VS_PORTn_SVIDX[0]~[4]
			{
				reg_array_field_read(APOLLOMP_SVLAN_P_SVIDXr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_SVIDXf, &val);
				//ASSERT(rtk_svlan_portSvid_get(i,&val)== RT_ERR_OK);
				//val2 = rtk_svlan_portSvid_get(i,&val);
				//if(val2==RT_ERR_SVLAN_INVALID)
					//PROC_PRINTF("Port-based SVALN is invalid\n");
				//else if(val2==RT_ERR_OK)
					PROC_PRINTF("reg_VS_PORTn_SVIDX[%d]:0x%x\n",i,val);//SVLAN_P_SVIDX
			}

			for(i=0; i<=6; i++)
			{
				reg_array_field_read(APOLLOMP_SVLAN_EP_DMAC_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_ENf, &val);
				//assert(rtk_svlan_dmacVidSelState_get(i,&val)== RT_ERR_OK);
				PROC_PRINTF("reg_VS_PORTn_DMACVIDSE[%d]:0x%x\n", i, val);
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else
	PROC_PRINTF("\n-----binding-----\n");

	//Binding
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:

			retv = reg_field_read(APOLLOMP_V6_BD_CTLr, APOLLOMP_PB_ENf, &val);
			//retv = rtk_l34_globalState_get(L34_GLOBAL_BIND_STATE,&val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_V6_BD_CTL:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L2L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L2L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L2L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L2L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L2L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L2L34:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L34:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L34L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L34L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L34L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L34L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L34L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L34L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_UNMATCHED_L3L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_UNMATCHED_L3L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_UNMATCHED_L3L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L2f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L2, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L2:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L3f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L3, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L3:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_BD_CFGr, APOLLOMP_WAN_BINDING_CUSTOMIZED_L34f, &val);
			//retv = rtk_l34_bindingAction_get(L34_BIND_CUSTOMIZED_L34, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_BINDING_WAN_BINDING_CUSTOMIZED_L34:0x%x\n", val);
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else
	// IPv6
	PROC_PRINTF("\n-----IPv6-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//retv = reg_field_read(APOLLOMP_V6_BD_CTLr, APOLLOMP_PB_ENf, &val);		//FIXME:no RTK api
			//assert(retv == RT_ERR_OK);
			//PROC_PRINTF("reg_IPV6_PB_EN:0x%x\n", val);		//binding control bit

			retv = reg_field_read(APOLLOMP_SWTCR0r, APOLLOMP_TTL_1ENABLEf, &val);	//FIXME:no RTK api
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_IPV6_TTL_1_EN:0x%x\n", val);		//TTL-1 control bit
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}
#endif

#ifdef CONFIG_APOLLO_RLE0371
#else
	// Port Isolation
	PROC_PRINTF("\n-----Port Isolation-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			retv = reg_field_read(APOLLOMP_PISO_CTRLr, APOLLOMP_CTAG_SELf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ISOLATION_CTAG_SEL:0x%x\n", val);

			retv = reg_field_read(APOLLOMP_PISO_CTRLr, APOLLOMP_L34_SELf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_ISOLATION_L34_SEL:0x%x\n", val);

			PROC_PRINTF("\n");

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_P_MODE0_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_PORT_MODE0[%d]:0x%x\n",i,val);
			}

			PROC_PRINTF("\n");

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_P_MODE1_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_PORT_MODE1[%d]:0x%x\n",i,val);
			}

			PROC_PRINTF("\n");

			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_EXT_MODE0_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_EXTPORT_MODE0[%d]:0x%x\n",i,val);
			}

			PROC_PRINTF("\n");

			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				reg_array_field_read(APOLLOMP_PISO_EXT_MODE1_CTRLr,i,REG_ARRAY_INDEX_NONE, APOLLOMP_PORTMASKf, &val);
				PROC_PRINTF("reg_ISOLATION_EXTPORT_MODE1[%d]:0x%x\n",i,val);
			}

			//PROC_PRINTF("reg_ISOLATION_EFID[MAX_APOLLO_PORT]:0x%x\n");
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			diag_util_printf("Chip Not Support.\n");
	}

	// ALE34
	PROC_PRINTF("\n-----ALE34-----\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			//PROC_PRINTF("reg_NAT_CTRL:0x%x\n");

			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP0f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P0]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP1f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P1]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP2f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P2]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP3f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P3]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP4f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P4]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFPr, APOLLOMP_INTP5f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_PORT_NETIF_MAPPING[P5]:0x%x\n",val);

			PROC_PRINTF("\n");

			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP0f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP0]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP1f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP1]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP2f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP2]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP3f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP3]:0x%x\n",val);
			retv = reg_field_read(APOLLOMP_NIFEPr, APOLLOMP_INTEXTP4f, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_EXTPORT_NETIF_MAPPING[EP4]:0x%x\n",val);

			PROC_PRINTF("\n");

			retv = reg_field_read(APOLLOMP_HSBA_CTRLr, APOLLOMP_TST_LOG_MDf, &val);
			assert(retv == RT_ERR_OK);
			PROC_PRINTF("reg_NAT_HSBA_TST_LOG_MODE:0x%x\n",val);

			PROC_PRINTF("\n");

			//PROC_PRINTF("reg_VS_FIDEN:0x%x\n");
			//PROC_PRINTF("reg_VS_FID:0x%x\n");

			for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_L34_PORT_TO_WANr,i,j, APOLLOMP_PORT_TO_WAN_PERMITf, &val);
					PROC_PRINTF("reg_L34_PORT[%d]_TO_WAN[%d]:0x%x\n",i,j,val);
				}
				PROC_PRINTF("\n");
			}

			for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
			{
				for(j=0; j<8; j++)
				{
					reg_array_field_read(APOLLOMP_L34_EXTPORT_TO_WANr,i,j, APOLLOMP_EXTPORT_TO_WAN_PERMITf, &val);
					PROC_PRINTF("reg_L34_EXTPORT[%d]_TO_WAN[%d]:0x%x\n",i,j,val);
				}
				PROC_PRINTF("\n");
			}

			PROC_PRINTF("\n");

			for(j=0; j<8; j++)
			{
				for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
				{
					reg_array_field_read(APOLLOMP_L34_WAN_TO_PORTr,i,j, APOLLOMP_WAN_TO_PORT_PERMITf, &val);
					PROC_PRINTF("reg_L34_WAN[%d]_TO_PORT[%d]:0x%x\n",j,i,val);
				}
				PROC_PRINTF("\n");
			}

			PROC_PRINTF("\n");

			for(j=0; j<8; j++)
			{
				for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
				{
					reg_array_field_read(APOLLOMP_L34_WAN_TO_EXTPORTr,i,j, APOLLOMP_WAN_TO_EXTPORT_PERMITf, &val);
					PROC_PRINTF("reg_L34_WAN[%d]_TO_EXTPORT[%d]:0x%x\n",j,i,val);
				}
				PROC_PRINTF("\n");
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif

#endif
		return len;

}
#endif


int32 mibdump_l34_bind_table(struct seq_file *s, void *v)
{
#ifndef CONFIG_APOLLO_RLE0371
	uint32 idx;
	rtk_rg_vbind_linkList_t *pVbdEntry;
	int len=0;
	char validType[4][15]  = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};

	PROC_PRINTF(">>L34 Binding Table:\n");

	for(idx=0; idx<MAX_BIND_SW_TABLE_SIZE; idx++)
	{
		if (!rg_db.bind[idx].valid)
			continue;
		PROC_PRINTF("  [%d](%s)\t	pmask(%02x) epmask(%02x) vid(%d) wt_idx(%d) bindPtl (%s)\n",
					   idx,validType[rg_db.bind[idx].valid],
					   rg_db.bind[idx].rtk_bind.portMask.bits[0],
					   rg_db.bind[idx].rtk_bind.extPortMask.bits[0],
					   rg_db.bind[idx].rtk_bind.vidLan,
					   rg_db.bind[idx].rtk_bind.wanTypeIdx,
					   rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV4_IPV6?"other":rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV6?"!v6":rg_db.bind[idx].rtk_bind.bindProto==L34_BIND_PROTO_NOT_IPV4?"!v4":"all"
					  );
	}

	PROC_PRINTF(">>Vlan-Binding Num=%d\n",rg_db.systemGlobal.vlanBindTotalNum);
	//diaplay vlan-bind
	for(idx=0;idx<RTK_RG_PORT_MAX;idx++)
	{
		if(RG_INVALID_PORT(idx)) continue;
		if(!list_empty(&rg_db.vlanBindingListHead[idx]))
		{
			if(idx>RTK_RG_PORT_LASTCPU)
				{PROC_PRINTF("  ExtPort[%d]:",idx-RTK_RG_PORT_LASTCPU-1);}
			else
				{PROC_PRINTF("  Port[%d]:",idx);}
			list_for_each_entry(pVbdEntry,&rg_db.vlanBindingListHead[idx],vbd_list)
			{
				PROC_PRINTF(" V%d->W%d",pVbdEntry->vlanId,pVbdEntry->wanIdx);
				if(list_is_last(&pVbdEntry->vbd_list,&rg_db.vlanBindingListHead[idx]))
					{PROC_PRINTF("\n");}
				else
					{PROC_PRINTF(",");}
			}

		}
	}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
	PROC_PRINTF(">>WLAN0 Device Binding:\n");
	for(idx=0;idx<MAX_WLAN_DEVICE_NUM;idx++)
	{
		if(rg_db.systemGlobal.wlan0BindDecision[idx].exist)
		{
			PROC_PRINTF("WLAN0 DEV[%d]:",idx);
			if(rg_db.systemGlobal.wlan0BindDecision[idx].set_bind)
				{PROC_PRINTF("Device-binding to WAN[%d].\n",rg_db.systemGlobal.wlan0BindDecision[idx].bind_wanIntf);}
			else
				{PROC_PRINTF("None-binding.\n");}
		}
	}
#endif
#endif
    return len;
}

int32 dump_l34_bind_table(struct seq_file *s, void *v)
{
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int len=0;
#else
	rtk_binding_entry_t entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>ASIC L34 Binding Table:\n");
	for(idx=0; idx<MAX_BIND_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_bindingTable_get(idx, &entry)!= RT_ERR_OK)
			continue;
		PROC_PRINTF("  [%d]	pmask(%02x) epmask(%02x) vid(%d) wt_idx(%d) bindPtl (%s)\n",
					   idx,
					   entry.portMask.bits[0],
					   entry.extPortMask.bits[0],
					   entry.vidLan,
					   entry.wanTypeIdx,
					   entry.bindProto==L34_BIND_PROTO_NOT_IPV4_IPV6?"other":entry.bindProto==L34_BIND_PROTO_NOT_IPV6?"!v6":entry.bindProto==L34_BIND_PROTO_NOT_IPV4?"!v4":"all"
					  );
	}
#endif
    return len;
}
int32 dump_l34_wantype_table(struct seq_file *s, void *v)
{
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int len=0;
#else
	rtk_wanType_entry_t entry;
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>ASIC L34 WAN Type Table:\n");
	for(idx=0; idx<MAX_WANTYPE_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_wanTypeTable_get(idx, &entry)!= RT_ERR_OK)
			continue;
		switch(entry.wanType)
		{
			case L34_WAN_TYPE_L2_BRIDGE:
			case L34_WAN_TYPE_L3_ROUTE:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   entry.wanType==L34_WAN_TYPE_L2_BRIDGE?"(L2_BRIDGE)     ":"(L3_ROUTE)      ",
					   entry.nhIdx
					  );
				break;
			case L34_WAN_TYPE_L34NAT_ROUTE:
			case L34_WAN_TYPE_L34_CUSTOMIZED:
				PROC_PRINTF("  [%d]	wanType%s nexthopIdx(%d)\n",
					   idx,
					   entry.wanType==L34_WAN_TYPE_L34NAT_ROUTE?"(L34NAT_ROUTE)  ":"(L34_CUSTOMIZED)",
					   entry.nhIdx
					  );
				break;
			default:
				break;
		}
	}

#endif
    return len;
}


int32 dump_l34_sw_wantype_table(struct seq_file *s, void *v)
{
	char validType[4][15]  = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};

	uint32 idx;
	int len=0;

	PROC_PRINTF(">>Software L34 WAN Type Table:\n");
	for(idx=0; idx<MAX_WANTYPE_SW_TABLE_SIZE; idx++)
	{
		if(!rg_db.wantype[idx].valid)
			continue;
		switch(rg_db.wantype[idx].rtk_wantype.wanType)
		{
			case L34_WAN_TYPE_L2_BRIDGE:
			case L34_WAN_TYPE_L3_ROUTE:
				PROC_PRINTF("  [%d](%s)\t	wanType%s nexthopIdx(%d)\n",
					   idx,validType[rg_db.wantype[idx].valid],
					   rg_db.wantype[idx].rtk_wantype.wanType==L34_WAN_TYPE_L2_BRIDGE?"(L2_BRIDGE)     ":"(L3_ROUTE)      ",
					   rg_db.wantype[idx].rtk_wantype.nhIdx
					  );
				break;
			case L34_WAN_TYPE_L34NAT_ROUTE:
			case L34_WAN_TYPE_L34_CUSTOMIZED:
				PROC_PRINTF("  [%d](%s)\t	wanType%s nexthopIdx(%d)\n",
					   idx,validType[rg_db.wantype[idx].valid],
					   rg_db.wantype[idx].rtk_wantype.wanType==L34_WAN_TYPE_L34NAT_ROUTE?"(L34NAT_ROUTE)  ":"(L34_CUSTOMIZED)",
					   rg_db.wantype[idx].rtk_wantype.nhIdx
					  );
				break;
			default:
				break;
		}
	}

    return len;
}



int32 dump_ipv6_route_table(struct seq_file *s, void *v)
{
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int len=0;
#else
	rtk_ipv6Routing_entry_t entry;
	uint32 idx;
	int len=0;

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	PROC_PRINTF(">>ASIC IPv6 Routing Table:(Stateful Routing support:on)\n");
#else
	PROC_PRINTF(">>ASIC IPv6 Routing Table:\n");
#endif

	for(idx=0; idx<MAX_IPV6_ROUTING_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_ipv6RoutingTable_get(idx, &entry) != RT_ERR_OK)
			continue;
		if(entry.valid!=TRUE)
			continue;

		switch(entry.type)
		{
			case L34_IPV6_ROUTE_TYPE_TRAP:
					PROC_PRINTF("  [%d]	valid type(TRAP)", idx);
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
					if(rg_db.v6route[idx].rtk_v6route.type!=L34_IPV6_ROUTE_TYPE_TRAP)
					{
						switch(rg_db.v6route[idx].rtk_v6route.type)
						{
							case L34_IPV6_ROUTE_TYPE_DROP:
								PROC_PRINTF("=>orig:DROP");
								break;
							case L34_IPV6_ROUTE_TYPE_LOCAL:
								PROC_PRINTF("=>orig:LOCAL DNET_IF(%d)",entry.nhOrIfidIdx);
								break;
							case L34_IPV6_ROUTE_TYPE_GLOBAL:
								PROC_PRINTF("=>orig:GLOBAL NEXTHOP(%d)",entry.nhOrIfidIdx);
								break;
							default:
								break;
						}
					}
#endif
				break;
    		case L34_IPV6_ROUTE_TYPE_DROP:
				PROC_PRINTF("  [%d]	valid type(DROP)", idx);
				break;
    		case L34_IPV6_ROUTE_TYPE_LOCAL:
				PROC_PRINTF("  [%d]	valid type(LOCAL) DNET_IF(%d)", idx,entry.nhOrIfidIdx);
				break;
    		case L34_IPV6_ROUTE_TYPE_GLOBAL:
				PROC_PRINTF("  [%d]	valid type(GLOBAL) NEXTHOP(%d)", idx,entry.nhOrIfidIdx);
				break;
			default:
				break;
		}

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		PROC_PRINTF("%s prefixLen(%d) \n	   DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
				(entry.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
				entry.ipv6PrefixLen,
				entry.ipv6Addr.ipv6_addr[0],entry.ipv6Addr.ipv6_addr[1],entry.ipv6Addr.ipv6_addr[2],entry.ipv6Addr.ipv6_addr[3],
				entry.ipv6Addr.ipv6_addr[4],entry.ipv6Addr.ipv6_addr[5],entry.ipv6Addr.ipv6_addr[6],entry.ipv6Addr.ipv6_addr[7],
				entry.ipv6Addr.ipv6_addr[8],entry.ipv6Addr.ipv6_addr[9],entry.ipv6Addr.ipv6_addr[10],entry.ipv6Addr.ipv6_addr[11],
				entry.ipv6Addr.ipv6_addr[12],entry.ipv6Addr.ipv6_addr[13],entry.ipv6Addr.ipv6_addr[14],entry.ipv6Addr.ipv6_addr[15]);

#else
		PROC_PRINTF("%s prefixLen(%d)\n     DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
						(entry.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
						entry.ipv6PrefixLen,
						entry.ipv6Addr.ipv6_addr[0],entry.ipv6Addr.ipv6_addr[1],entry.ipv6Addr.ipv6_addr[2],entry.ipv6Addr.ipv6_addr[3],
						entry.ipv6Addr.ipv6_addr[4],entry.ipv6Addr.ipv6_addr[5],entry.ipv6Addr.ipv6_addr[6],entry.ipv6Addr.ipv6_addr[7],
						entry.ipv6Addr.ipv6_addr[8],entry.ipv6Addr.ipv6_addr[9],entry.ipv6Addr.ipv6_addr[10],entry.ipv6Addr.ipv6_addr[11],
						entry.ipv6Addr.ipv6_addr[12],entry.ipv6Addr.ipv6_addr[13],entry.ipv6Addr.ipv6_addr[14],entry.ipv6Addr.ipv6_addr[15]);
#endif
	}
#endif


    return len;
}


int32 dump_sw_ipv6_route_table(struct seq_file *s, void *v)
{

	uint32 idx;
	int len=0;
	char validType[4][15] = {"INVALID", "HWSW_SYNC", " SW_ONLY ","HW_ONLY"};

	PROC_PRINTF(">>Software IPv6 Routing Table:\n");
	for(idx=0; idx<MAX_IPV6_ROUTING_SW_TABLE_SIZE; idx++)
	{
		if(rg_db.v6route[idx].valid){

			PROC_PRINTF("[%d][%s] ",idx,validType[rg_db.v6route[idx].valid]);

			switch(rg_db.v6route[idx].rtk_v6route.type)
			{
				case L34_IPV6_ROUTE_TYPE_TRAP:
					PROC_PRINTF(" TRAP");
					break;
				case L34_IPV6_ROUTE_TYPE_DROP:
					PROC_PRINTF(" DROP");
					break;
				case L34_IPV6_ROUTE_TYPE_LOCAL:
					PROC_PRINTF(" LOCAL DNET_IF(%d)",rg_db.v6route[idx].rtk_v6route.nhOrIfidIdx);
					break;
				case L34_IPV6_ROUTE_TYPE_GLOBAL:
					PROC_PRINTF(" GLOBAL NEXTHOP(%d)",rg_db.v6route[idx].rtk_v6route.nhOrIfidIdx);
					break;
				default:
					break;
			}


			PROC_PRINTF(" %s %s prefixLen(%d) \n	   DIP(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
					(rg_db.v6route[idx].rtk_v6route.rt2waninf==TRUE? "RT2WAN": "RT2LAN"),
					(rg_db.v6route[idx].internal?"(EXTERNAL)":"(INTERNAL)"), rg_db.v6route[idx].rtk_v6route.ipv6PrefixLen,
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[0],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[1],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[2],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[3],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[4],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[5],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[6],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[7],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[8],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[9],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[10],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[11],
					rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[12],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[13],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[14],rg_db.v6route[idx].rtk_v6route.ipv6Addr.ipv6_addr[15]);

		}
	}


    return len;
}



int32 dump_ipv6_neighbor_table(struct seq_file *s, void *v)
{
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	int len=0;
#else
	rtk_ipv6Neighbor_entry_t entry;
	uint32 idx;
	int len=0;
	uint8 ipv6Ifid[8];

	PROC_PRINTF("\n\n>>ASIC IPv6 Neighbor Table:\n");
	for(idx=0; idx<MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE; idx++)
	{
		if (rtk_l34_ipv6NeighborTable_get(idx, &entry) != RT_ERR_OK)
			continue;
		if (entry.valid==FALSE)
			continue;
		memcpy(ipv6Ifid, (uint8 *)&entry.ipv6Ifid, 8);
		PROC_PRINTF("  [%d]	valid L2Idx(%d) routingIdx(%d) InterfaceID(%02x%02x:%02x%02x:%02x%02x:%02x%02x) idle=%d static=%d\n",
					   idx, entry.l2Idx,
					   entry.ipv6RouteIdx,
					   ipv6Ifid[0],
					   ipv6Ifid[1],
					   ipv6Ifid[2],
					   ipv6Ifid[3],
					   ipv6Ifid[4],
					   ipv6Ifid[5],
					   ipv6Ifid[6],
					   ipv6Ifid[7],
					   rg_db.v6neighbor[idx].idleSecs,
					   rg_db.v6neighbor[idx].neighborEntry.staticEntry);
	}
#endif

    return len;
}

int32 mibdump_ipv6_neighbor_table(struct seq_file *s, void *v)
{
#ifdef CONFIG_APOLLO_RLE0371
#else
	uint32 idx;
	int len=0;

	PROC_PRINTF(">>SW IPv6 Neighbor Table:\n");
	for(idx=0; idx<MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE; idx++)
	{
		if (rg_db.v6neighbor[idx].neighborEntry.valid==FALSE)
			continue;
		PROC_PRINTF("  [%d]	valid(idle:%d) L2Idx(%d) routingIdx(%d)%sInterfaceID(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)\n",
					   idx,
					   rg_db.v6neighbor[idx].idleSecs,
					   rg_db.v6neighbor[idx].neighborEntry.l2Idx,
					   rg_db.v6neighbor[idx].neighborEntry.matchRouteIdx,
					   rg_db.v6neighbor[idx].neighborEntry.staticEntry?" static ":" ",
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[0],rg_db.v6neighbor[idx].neighborEntry.interfaceId[1],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[2],rg_db.v6neighbor[idx].neighborEntry.interfaceId[3],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[4],rg_db.v6neighbor[idx].neighborEntry.interfaceId[5],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[6],rg_db.v6neighbor[idx].neighborEntry.interfaceId[7],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[8],rg_db.v6neighbor[idx].neighborEntry.interfaceId[9],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[10],rg_db.v6neighbor[idx].neighborEntry.interfaceId[11],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[12],rg_db.v6neighbor[idx].neighborEntry.interfaceId[13],
					   rg_db.v6neighbor[idx].neighborEntry.interfaceId[14],rg_db.v6neighbor[idx].neighborEntry.interfaceId[15]);
	}
#endif

    return len;
}

int32 dump_piso(struct seq_file *s, void *v)
{
	int i,j,ret=0;
	rtk_portmask_t mbr= {{0}},extmbr= {{0}};
	int len=0;
	PROC_PRINTF(">>Port Isolation:\n");
	for(j=0; j<RTK_PORT_ISO_CFG_END; j++)
	{
		PROC_PRINTF("  - mode %d - \n", j);
		for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
		{

		 PROC_PRINTF("\t[Phy port %d:]", i);
		 ret = rtk_port_isolationEntry_get(j,i,&mbr,&extmbr);
		 PROC_PRINTF("\tPhy Port Mask:0x%x",mbr.bits[0]);
		 PROC_PRINTF("\tExtension Port Mask:0x%x\n",extmbr.bits[0]);

		}
		for(i=0; i<RTK_RG_MAX_EXT_PORT; i++)
		{

		 PROC_PRINTF("\t[Ext. port %d:]", i);
		 ret = rtk_port_isolationEntryExt_get(j,i,&mbr,&extmbr);
		 PROC_PRINTF("\tPhy Port Mask:0x%x",mbr.bits[0]);
		 PROC_PRINTF("\tExtension Port Mask:0x%x\n",extmbr.bits[0]);

		}
	}

    return len;
}

int32 dump_wlan_mbssid(struct seq_file *s, void *v)
{
	int i;
	int len=0;
	PROC_PRINTF("IDX\tMAC\t\t\tDevIdx\tctag_if\tVID\tDiffJiffies\n");
	for(i=rg_db.wlanMbssidHeadIdx;i<rg_db.wlanMbssidHeadIdx+MAX_WLAN_MBSSID_SW_TABLE_SIZE;i++)
	{
		int idx=i%MAX_WLAN_MBSSID_SW_TABLE_SIZE;
		if(memcmp(rg_db.wlanMbssid[idx].mac.octet,"\x0\x0\x0\x0\x0\x0",6)==0) continue;
		PROC_PRINTF("%02d\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%d\t%d\t-%lu\n",idx,
			rg_db.wlanMbssid[idx].mac.octet[0],
			rg_db.wlanMbssid[idx].mac.octet[1],
			rg_db.wlanMbssid[idx].mac.octet[2],
			rg_db.wlanMbssid[idx].mac.octet[3],
			rg_db.wlanMbssid[idx].mac.octet[4],
			rg_db.wlanMbssid[idx].mac.octet[5],
			rg_db.wlanMbssid[idx].wlan_dev_idx,
			rg_db.wlanMbssid[idx].vlan_tag_if,
			rg_db.wlanMbssid[idx].vid,
			jiffies-rg_db.wlanMbssid[idx].learn_jiffies	);

	}
	return len;
}


void dump_compare_packet(uint8 *output,uint8 *expect,int out_size,int expect_size)
{
    int off;
    u8 protocol=0;
    int i,j,pppoeif=0;
    uint8 *pkt=NULL;
    char tmpstr[32];
    int size=max(out_size,expect_size);



    for(j=1; j<3; j++)
    {
        //if(j==0) {pkt=input; sprintf(tmpstr,"INPUT");}
        if(j==1)
        {
            pkt=output;
            sprintf(tmpstr,"OUTPUT(size=%d)",out_size);
        }
        if(j==2)
        {
            pkt=expect;
            sprintf(tmpstr,"EXPECT(size=%d)",expect_size);
        }

        for(i=0; i<78; i++)
            rtlglue_printf("=");

        rtlglue_printf("\n");

        rtlglue_printf("%s\n",tmpstr);
        if(size==0) return;

        for(i=0; i<size; i++)
        {
            int diff=0;

            if(j==1)
            {
                if((i>=out_size)||(out_size<=4))
                    break;//goto skip_dump;
            }
            if(j==2)
            {
                if((i>=expect_size)||(expect_size<=4))
                    break;//goto skip_dump;
            }
            if((i%16)==0) rtlglue_printf("%08x ",(unsigned int)(unsigned long int)(&pkt[i]));
            //if((input[i]!=output[i])||(output[i]!=expect[i])) diff=1;
            if(i<min(out_size,expect_size-4))
                if(output[i]!=expect[i]) diff=1;
            rtlglue_printf("%s%02x%s ",(diff==1)?"*"COLOR_H:"",(u8)pkt[i],(diff==1)?COLOR_NM:"");
            if((i%16)==7) rtlglue_printf("  ");
            if((i%16)==15) rtlglue_printf("\n");
        }
        if(i%16!=15)
            rtlglue_printf("\n");

        rtlglue_printf(COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[0],pkt[1],pkt[2],pkt[3],pkt[4],pkt[5]
                       ,pkt[6],pkt[7],pkt[8],pkt[9],pkt[10],pkt[11]);
        off=12;
        if((pkt[off]==0x88)&&(pkt[off+1]==0x99))
        {
            rtlglue_printf("CPU:[" COLOR_Y "Protocol" COLOR_NM "=%d][" COLOR_Y "Res" COLOR_NM "=0x%x][" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "TTL_1" COLOR_NM "=0x%x][" COLOR_Y "L3R" COLOR_NM "=%d][" COLOR_Y "ORG" COLOR_NM "=%d][" COLOR_Y "SPA" COLOR_NM "=%d][" COLOR_Y "EPMSK" COLOR_NM "=0x%x]\n"
                           ,pkt[off+2],pkt[off+3],pkt[off+4]>>5,pkt[off+4]&0x1f
                           ,pkt[off+5]>>7,(pkt[off+5]>>6)&1,pkt[off+5]&7,pkt[off+7]&0x3f);
            off+=8;
        }

        if((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
        {
            rtlglue_printf("SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
            off+=4;
        }

        if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
        {
            rtlglue_printf("CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
            off+=4;
        }

        if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
        {
            rtlglue_printf("PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x]\n",
                           pkt[off+3],(pkt[off+4]<<8)|pkt[off+5]);
            off+=8;
            pppoeif=1;
        }

        if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)))		//IPv6 or IPv6 with PPPoE
        {
            rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%02x%02x%x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4, (pkt[off+2]&0xf)+(pkt[off+3]>>4), (pkt[off+3]&0xf)+(pkt[off+4]>>4), (pkt[off+4]&0xf)+(pkt[off+5]>>4), (pkt[off+5]&0xf), (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
            rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
                           ,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
            rtlglue_printf("     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
                           ,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

            protocol=pkt[off+8];
            if(protocol==0)	//hop-by-hop
            {
                rtlglue_printf("Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
                               ,pkt[off+42], pkt[off+43]);
                rtlglue_printf("          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
                               ,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
                for(i=0; i<pkt[off+43]; i++)
                {
                    rtlglue_printf("         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
                                   ,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
                                   ,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
                }

                protocol=pkt[off+42];
                off+=(50+pkt[off+43]*8);
            }
            else
                off+=42;
        }
        //rtlglue_printf("###############the pkt off before IPV4 is %02x off+1 is %02x, pppoeif is %d\n",pkt[off],pkt[off+1],pppoeif);
        if(((pkt[off]==0x08)&&(pkt[off+1]==0x00)&&(pppoeif==0))||((pkt[off]==0x00)&&(pkt[off+1]==0x21)&&(pppoeif==1)))
        {
            rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=%d(DSCP=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
                           ,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
            rtlglue_printf("	 [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                           ,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
            rtlglue_printf("	 [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
                           ,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

            protocol=pkt[off+11];
            off+=(pkt[off+2]&0xf)*4+2;
        }

        if(protocol==0x6) //TCP
        {
            rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                           ,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
                           ,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
            rtlglue_printf("	[" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                           ,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
                           ,(pkt[off+14]<<8)|pkt[off+15]);
            rtlglue_printf("	[" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
        }
        else if(protocol==0x11) //UDP
        {
            rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
                           ,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));
        }
//skip_dump:
        //      ;
    }

}



int32 mibdump_srvInLan_IP_table(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	PROC_PRINTF(">>MIB for ALG Server In LAN IP Table:\n");
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if(rg_db.algServInLanIpMapping[i].serverAddress>0)
		{
			switch(rg_db.algServInLanIpMapping[i].algType)
			{
				case RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN SIP_TCP");
					break;
				case RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN SIP_UDP");
					break;
				case RTK_RG_ALG_H323_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN H323_TCP");
					break;
				case RTK_RG_ALG_H323_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN H323_UDP");
					break;
				case RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN RTSP_TCP");
					break;
				case RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN RTSP_UDP");
					break;
				case RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN FTP_TCP");
					break;
				case RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT:
					PROC_PRINTF("SrvInLAN FTP_UDP");
					break;
				default:
					break;
			}
			PROC_PRINTF("(%x) serverIP is %08x\n",
				rg_db.algServInLanIpMapping[i].algType,rg_db.algServInLanIpMapping[i].serverAddress);
		}
	}

	return len;
}

int32 mibdump_virtualServer(struct seq_file *s, void *v)
{
	int i,ret;
	rtk_rg_virtualServer_t virtual_server;
	int valid_idx;
	int len=0;

	PROC_PRINTF("Virtual server:\n");
	for(i=0;i<MAX_VIRTUAL_SERVER_SW_TABLE_SIZE;i=valid_idx+1)
	{
		valid_idx=i;

		ret=rtk_rg_apollo_virtualServer_find(&virtual_server,&valid_idx);
		if(ret==RT_ERR_RG_OK)
		{
			PROC_PRINTF("[%d] %s WAN-intf:%d, gwPortStart:%d, localIP:remoteIP:Port=>%08x:%08x:%d, enable_limit_remote_src_port=%d, remote_src_port_start=%d ,remote_src_port_end=%d , mappingRangeCnt=%d, mappingType=[%s]\n",
				valid_idx,
				virtual_server.is_tcp?"TCP":"UDP",
				virtual_server.wan_intf_idx,
				virtual_server.gateway_port_start,
				virtual_server.local_ip,
				virtual_server.remote_ip,
				virtual_server.local_port_start,
				virtual_server.enable_limit_remote_src_port,
				virtual_server.remote_src_port_start,
				virtual_server.remote_src_port_end,
				virtual_server.mappingPortRangeCnt,
				(virtual_server.mappingType==VS_MAPPING_N_TO_N)?"N-to-N":"N-to-1"
				);
		}
		else
			break;
	}

	return len;
}

int32 mibdump_portTrigger(struct seq_file *s, void *v)
{
	int i,displayed=0;
	rtk_rg_portTrigger_list_t *pTriggerEntry;
	int len=0;

	PROC_PRINTF("Port Trigger:\n");
	for(i=0;i<4;i++){
		if(!list_empty(rg_db.systemGlobal.pPortTrigger_matched_head[i])){
			switch(i){
				case 0:
					PROC_PRINTF("\tIncoming UDP=>\n");
					break;
				case 1:
					PROC_PRINTF("\tIncoming TCP=>\n");
					break;
				case 2:
					PROC_PRINTF("\tOutgoing UDP=>\n");
					break;
				case 3:
					PROC_PRINTF("\tOutgoing TCP=>\n");
					break;
				default:
					break;
			}
			list_for_each_entry(pTriggerEntry, rg_db.systemGlobal.pPortTrigger_matched_head[i], trigger_list){
				PROC_PRINTF("\t\tmatch_port:",pTriggerEntry->match_port_start);
				if(pTriggerEntry->match_port_start==pTriggerEntry->match_port_end)
					PROC_PRINTF("%d --> ",pTriggerEntry->match_port_start);
				else
					PROC_PRINTF("%d ~ %d --> ",pTriggerEntry->match_port_start,pTriggerEntry->match_port_end);
				PROC_PRINTF("relate_%s_port:",pTriggerEntry->relate_is_tcp?"TCP":"UDP");
				if(pTriggerEntry->relate_port_start==pTriggerEntry->relate_port_end)
					PROC_PRINTF("%d\n",pTriggerEntry->relate_port_start);
				else
					PROC_PRINTF("%d ~ %d\n",pTriggerEntry->relate_port_start,pTriggerEntry->relate_port_end);
			}
			displayed=1;
		}
	}
	if(displayed){
		PROC_PRINTF("Established relate entry:\n");
		for(i=0;i<2;i++){
			if(!list_empty(rg_db.systemGlobal.pPortTrigger_related_head[i])){
				switch(i){
					case 0:
						PROC_PRINTF("\tUDP=>\n");
						break;
					case 1:
						PROC_PRINTF("\tTCP=>\n");
						break;
					default:
						break;
				}
				list_for_each_entry(pTriggerEntry, rg_db.systemGlobal.pPortTrigger_related_head[i], trigger_list){
					PROC_PRINTF("\t\trelate_port:");
					if(pTriggerEntry->relate_port_start==pTriggerEntry->relate_port_end)
						PROC_PRINTF("%d, ",pTriggerEntry->relate_port_start);
					else
						PROC_PRINTF("%d ~ %d, ",pTriggerEntry->relate_port_start,pTriggerEntry->relate_port_end);
					PROC_PRINTF("local_ip:%d.%d.%d.%d, refCnt:%d\n",
						(pTriggerEntry->local_ip&0xff000000)>>24,
						(pTriggerEntry->local_ip&0xff0000)>>16,
						(pTriggerEntry->local_ip&0xff00)>>8,
						(pTriggerEntry->local_ip&0xff),
						pTriggerEntry->referenceCount);
				}
			}
		}
	}

	return len;
}

int32 mibdump_upnpConnection(struct seq_file *s, void *v)
{
	int i,ret;
	rtk_rg_upnpConnection_t upnp;
	int valid_idx;
	int len=0;

	PROC_PRINTF(">>MIB for UPnP connections:\n");
	for(i=0;i<MAX_UPNP_SW_TABLE_SIZE;i=valid_idx+1)
	{
		valid_idx=i;

		ret=rtk_rg_apollo_upnpConnection_find(&upnp,&valid_idx);
		if(ret==RT_ERR_RG_OK)
		{
			PROC_PRINTF("[%d] %s WAN:%d TYPE:%s TIMEOUT:%d(Idle %d Secs) gwPort:%d localIPandPort=>%08x:%d limitIP=%s,IP=%08x limitPort=%s,Port=%d\n",
				valid_idx,
				upnp.is_tcp?"TCP":"UDP",
				upnp.wan_intf_idx,
				upnp.type==UPNP_TYPE_ONESHOT?"ONE-SHOT":"PERSIST",
				upnp.timeout,
				upnp.idle,
				upnp.gateway_port,
				upnp.local_ip,
				upnp.local_port,
				upnp.limit_remote_ip?"Enable":"DISABLE",
				upnp.remote_ip,
				upnp.limit_remote_port?"Enable":"DISABLE",
				upnp.remote_port);
		}
		else
			break;
	}

	return len;
}

int32 mibdump_staticRoute(struct seq_file *s, void *v)
{
	int i,len=0;
#if defined(CONFIG_RG_RTL9602C_SERIES)
	PROC_PRINTF(">>MIB for static routes: exclude_NAT_refCnt=%d\n",rg_db.systemGlobal.excludeNAT_refCnt);
#else
	PROC_PRINTF(">>MIB for static routes:\n");
#endif
	for(i=0;i<MAX_STATIC_ROUTE_SIZE;i++){
		if(rg_db.staticRoute[i].valid){
			if(rg_db.staticRoute[i].info.ip_version){
				PROC_PRINTF("\n[%d] IPv6_staticRoute\n",i);
				PROC_PRINTF("    IP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[1],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[3],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[5],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[7],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[9],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[11],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[13],
					rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.addr.ipv6_addr[15]);
				PROC_PRINTF("    Mask_Length:%d\n",rg_db.staticRoute[i].info.ipv6.mask_length);
				PROC_PRINTF("    NextHopIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[0],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[1],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[2],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[3],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[4],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[5],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[6],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[7],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[8],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[9],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[10],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[11],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[12],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[13],
					rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[14],rg_db.staticRoute[i].info.ipv6.nexthop.ipv6_addr[15]);
				PROC_PRINTF("    linkLocalIntfIdx:%d\n",rg_db.staticRoute[i].info.ipv6.linklocal_intfidx);
			}else{
				PROC_PRINTF("\n[%d] IPv4_staticRoute\n",i);
				PROC_PRINTF("    IP:%d.%d.%d.%d\n",
					(rg_db.staticRoute[i].info.ipv4.addr>>24)&0xff,
					(rg_db.staticRoute[i].info.ipv4.addr>>16)&0xff,
					(rg_db.staticRoute[i].info.ipv4.addr>>8)&0xff,
					(rg_db.staticRoute[i].info.ipv4.addr)&0xff);
				PROC_PRINTF("    Mask:%d.%d.%d.%d\n",
					(rg_db.staticRoute[i].info.ipv4.mask>>24)&0xff,
					(rg_db.staticRoute[i].info.ipv4.mask>>16)&0xff,
					(rg_db.staticRoute[i].info.ipv4.mask>>8)&0xff,
					(rg_db.staticRoute[i].info.ipv4.mask)&0xff);
				PROC_PRINTF("    NextHopIP:%d.%d.%d.%d\n",
					(rg_db.staticRoute[i].info.ipv4.nexthop>>24)&0xff,
					(rg_db.staticRoute[i].info.ipv4.nexthop>>16)&0xff,
					(rg_db.staticRoute[i].info.ipv4.nexthop>>8)&0xff,
					(rg_db.staticRoute[i].info.ipv4.nexthop)&0xff);
				PROC_PRINTF("    localRouteIntfIdx:%d\n",rg_db.staticRoute[i].info.ipv4.localroute_intfidx);
				PROC_PRINTF("    exclude_NAT:%d\n",rg_db.staticRoute[i].info.ipv4.exclude_subnet);
			}
			PROC_PRINTF("nexthop_mac_autolearn:%d\n", rg_db.staticRoute[i].info.nexthop_mac_auto_learn);
			PROC_PRINTF("nexthop_mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
					rg_db.staticRoute[i].info.nexthop_mac.octet[0],
					rg_db.staticRoute[i].info.nexthop_mac.octet[1],
					rg_db.staticRoute[i].info.nexthop_mac.octet[2],
					rg_db.staticRoute[i].info.nexthop_mac.octet[3],
					rg_db.staticRoute[i].info.nexthop_mac.octet[4],
					rg_db.staticRoute[i].info.nexthop_mac.octet[5]);
			PROC_PRINTF("nexthop_port:%d\n",rg_db.staticRoute[i].info.nexthop_port);
			PROC_PRINTF("ASIC_routing_table_idx:%d\n",rg_db.staticRoute[i].route_idx);
			if(rg_db.staticRoute[i].info.ip_version)
			{
				PROC_PRINTF("ASIC_nexthop_table_idx:%d\n",rg_db.v6route[rg_db.staticRoute[i].route_idx].rtk_v6route.nhOrIfidIdx);
				PROC_PRINTF("ASIC_lut_table_idx:%d\n",rg_db.nexthop[rg_db.v6route[rg_db.staticRoute[i].route_idx].rtk_v6route.nhOrIfidIdx].rtk_nexthop.nhIdx);
				PROC_PRINTF("ASIC_netif_table_idx:%d\n",rg_db.nexthop[rg_db.v6route[rg_db.staticRoute[i].route_idx].rtk_v6route.nhOrIfidIdx].rtk_nexthop.ifIdx);
			}
			else
			{
				PROC_PRINTF("ASIC_nexthop_table_idx:%d\n",rg_db.l3[rg_db.staticRoute[i].route_idx].rtk_l3.nhStart);
				PROC_PRINTF("ASIC_lut_table_idx:%d\n",rg_db.nexthop[rg_db.l3[rg_db.staticRoute[i].route_idx].rtk_l3.nhStart].rtk_nexthop.nhIdx);
				PROC_PRINTF("ASIC_netif_table_idx:%d\n",rg_db.nexthop[rg_db.l3[rg_db.staticRoute[i].route_idx].rtk_l3.nhStart].rtk_nexthop.ifIdx);
				PROC_PRINTF("ASIC_ip_table_idx:%d\n",rg_db.staticRoute[i].nxtip_ipidx);
			}
		}
	}
	return len;
}

#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
int32 mibdump_DMZ(struct seq_file *s, void *v)
{
	int i;
	int len=0;

	PROC_PRINTF(">>MIB for DMZ settings:\n");
	for(i=0;i<MAX_DMZ_TABLE_SIZE;i++)
	{
		if(rg_db.dmzInfo[i].enabled)
		{
			if(rg_db.dmzInfo[i].mac_mapping_enabled)
				{PROC_PRINTF("[%d] DMZ for %02x:%02x:%02x:%02x:%02x:%02x\n",
					i,
					rg_db.dmzInfo[i].mac.octet[0],
					rg_db.dmzInfo[i].mac.octet[1],
					rg_db.dmzInfo[i].mac.octet[2],
					rg_db.dmzInfo[i].mac.octet[3],
					rg_db.dmzInfo[i].mac.octet[4],
					rg_db.dmzInfo[i].mac.octet[5]);}
			else
				{PROC_PRINTF("[%d] DMZ for ipv4[%08x] ",i,rg_db.dmzInfo[i].private_ip);
				 PROC_PRINTF("ipv6[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n",
				 	rg_db.dmzInfo[i].private_ipv6.ipv6_addr[0],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[1],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[2],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[3],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[4],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[5],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[6],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[7],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[8],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[9],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[10],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[11],
					rg_db.dmzInfo[i].private_ipv6.ipv6_addr[12],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[13],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[14],rg_db.dmzInfo[i].private_ipv6.ipv6_addr[15]);}
		}
	}

	return len;
}
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
extern int in6_pton(const char *src, int srclen,
						 u8 *dst,
						 int delim, const char **end);

int32 sw_mcFlow(struct seq_file *s, void *v){

	int i;
	int len=0;

	PROC_PRINTF("SW Flow Table: \n");
	for(i=0;i<DEFAULT_MAX_FLOW_COUNT;i++)
	{

		if(rg_db.mcflowIdxtbl[i].vaild)
		{
			PROC_PRINTF("[%d] path3Idx=%d path4Idx=%d path6Idx=%d lutIdx=%d lutSaIdx=%d pppoePassthroughEntry=%d skbLen=%d \n",i,rg_db.mcflowIdxtbl[i].path3Idx,rg_db.mcflowIdxtbl[i].path4Idx,rg_db.mcflowIdxtbl[i].path6Idx,rg_db.mcflowIdxtbl[i].lutIdx,rg_db.mcflowIdxtbl[i].lutSaIdx,rg_db.mcflowIdxtbl[i].pppoePassthroughEntry,rg_db.mcflowIdxtbl[i].skbLen);
			if(rg_db.mcflowIdxtbl[i].isIpv6)
				PROC_PRINTF("	  McAddr= %pI6\n",rg_db.mcflowIdxtbl[i].multicastAddress);
			else
				PROC_PRINTF("     McAddr= %pI4h\n",rg_db.mcflowIdxtbl[i].multicastAddress);
#if defined(CONFIG_RG_G3_SERIES)
			if(rg_db.mcflowIdxtbl[i].pMcEngineInfo)
			{
				PROC_PRINTF("     l2mcgid=%d  l3mcgid=%d \n",rg_db.mcflowIdxtbl[i].pMcEngineInfo->l2mcgid,rg_db.mcflowIdxtbl[i].pMcEngineInfo->l3mcgid);
			}
#endif
			//dump_packet(rg_db.mcflowIdxtbl[i].flowDataBuf,rg_db.mcflowIdxtbl[i].skbLen,"MC_PATTEN_BUF");
		}
	}
	return len;
}

#if defined(CONFIG_RG_G3_SERIES)
int32 sw_mcHwInfo(struct seq_file *s, void *v){

	int i;
	int len=0;

	PROC_PRINTF("SW HwInfo Table: \n");
	for(i=0;i<DEFAULT_MAX_FLOW_COUNT;i++)
	{

		if(rg_db.mcHwEngineInfoTbl[i].valid)
		{
			if(rg_db.mcHwEngineInfoTbl[i].groupAddress[1] || rg_db.mcHwEngineInfoTbl[i].groupAddress[2] || rg_db.mcHwEngineInfoTbl[i].groupAddress[3])
				PROC_PRINTF("[%d] Group=%pI6",i,&rg_db.mcHwEngineInfoTbl[i].groupAddress[0]);
			else
				PROC_PRINTF("[%d] Group=%pI4h",i,&rg_db.mcHwEngineInfoTbl[i].groupAddress[0]);
			PROC_PRINTF(" l2mcgid=%d l3mcgid=%d p3Pmsk=%x p4Pmsk=%x  mcHwRefCount=%d\n",rg_db.mcHwEngineInfoTbl[i].l2mcgid,rg_db.mcHwEngineInfoTbl[i].l3mcgid,
				rg_db.mcHwEngineInfoTbl[i].p3Pmsk,rg_db.mcHwEngineInfoTbl[i].p4Pmsk,rg_db.mcHwEngineInfoTbl[i].mcHwRefCount);


		}
	}
	return len;
}


static void dump_l3fe_hdr_i(L3FE_HDR_I_t *hdr)
{
	char *name_t2_ctrl[CA_FLOW_TYPE_MAX]={"(UC5_DS)","(UC5_US)","(MC)","(UC2)","","","",""};	//refer rg_ca_flow_profile_keytype_t
	char *name_mdata_h_trap_rsn[RXINFO_REF_RSN_MAX]={"0x0","ACL","FLOWMISS","UNKNOWN_DA","","","","",""};	//refer rxinfo_ref_trap_rsn

	rtlglue_printf("<Table Control>\n");
	rtlglue_printf("  cls: 0x%X mainhash: 0x%X%s lpm: 0x%X hashlite: 0x%X",
		hdr->t1_ctrl, hdr->t2_ctrl, ((hdr->t2_ctrl == 0xF)? "" : name_t2_ctrl[hdr->t2_ctrl]), hdr->t3_ctrl, hdr->t4_ctrl);
	if((hdr->t1_ctrl == 0xF) && (hdr->t2_ctrl == 0xF) && (hdr->t3_ctrl == 0xF) && (hdr->t4_ctrl == 0xF))
		rtlglue_printf(COLOR_H"(may hit special packet)"COLOR_NM);
	rtlglue_printf("\n");
	rtlglue_printf("  orig_packet_len: %d\n", hdr->orig_packet_len);

	// --------------------------- //

	rtlglue_printf("<CPU Header>\n");
	rtlglue_printf("    [CLS: %s]\n", (hdr->fwd_vld & 0x1) ?  COLOR_H "Hit" COLOR_NM : "NO Hit");
	rtlglue_printf("    [Hash(type-0): ");
	switch ((hdr->fwd_vld >> 1) & 0x3) {
	case 0: rtlglue_printf("NO Hit"); break;
	case 1: rtlglue_printf(COLOR_H"Hit Overflow Entry"COLOR_NM); break;
	case 2: rtlglue_printf(COLOR_H"Hit Cache Hash Entry"COLOR_NM); break;
	case 3: rtlglue_printf(COLOR_H"Hit Main Hash Entry"COLOR_NM); break;
	default: rtlglue_printf("UNKNOWN"); break;
	}
	rtlglue_printf(", Hash(type-1): ");
	switch ((hdr->fwd_vld >> 3) & 0x3) {
	case 0: rtlglue_printf("NO Hit"); break;
	case 1: rtlglue_printf(COLOR_H"Hit Overflow Entry"COLOR_NM); break;
	case 2: rtlglue_printf(COLOR_H"Hit Cache Hash Entry"COLOR_NM); break;
	case 3: rtlglue_printf(COLOR_H"Hit Main Hash Entry"COLOR_NM); break;
	default: rtlglue_printf("UNKNOWN"); break;
	}
	rtlglue_printf("]\n");
	rtlglue_printf("    [HashLite(type-0): ");
	switch ((hdr->fwd_vld >> 6) & 0x3) {
	case 0:
		rtlglue_printf("NO Hit"); break;
	case 1:
		rtlglue_printf(COLOR_H"Hit Overflow Entry"COLOR_NM); break;
	case 2:
	case 3:
		rtlglue_printf(COLOR_H"Hit Hash Entry"COLOR_NM); break;
	default:
		rtlglue_printf("UNKNOWN"); break;
	}
	rtlglue_printf(", HashLite(type-1): %s]\n", (hdr->fwd_vld & 0x100) ? COLOR_H"Hit"COLOR_NM : "NO Hit");

	rtlglue_printf("  cls_action: 0x%03X [std_fib: %u, fib_index: %u]\n",
		hdr->cls_action, (hdr->cls_action & 0x400) >> 10, hdr->cls_action & 0x3ff);

	if (hdr->hash_dbl_chk_fail & 0x1) {
		rtlglue_printf("    [Hash Engine double check fail (type-0)]\n");
	}
	if (hdr->hash_dbl_chk_fail & 0x2) {
		rtlglue_printf("    [Hash Engine double check fail (type-1)]\n");
	}
	if (hdr->hash_dbl_chk_fail & 0x4) {
		rtlglue_printf("    [HashLite Engine double check fail]\n");
	}

	rtlglue_printf("  hash_idx: %d  cache_way: %d\n", hdr->hash_idx, hdr->cache_way);
	if (((hdr->fwd_vld >> 1) & 0x3) == 1) {
		rtlglue_printf("    [overflow_hash_idx: %d]\n", hdr->hash_idx & 0x3f);
	}
	if (((hdr->fwd_vld >> 1) & 0x2) == 2) {
		rtlglue_printf("    [hash_idx: %d hash_profile: %d]\n", hdr->hash_idx & 0xffff, (hdr->hash_idx >> 16) & 0xf);
	}
	if ((hdr->fwd_vld >> 5) & 0x1) {
#ifdef CONFIG_ARCH_CORTINA_G3LITE
		rtlglue_printf("    [lpm_idx: %d lpm_sw_idx: %d]\n",
			hdr->hash_idx & 0x3ff,
			((hdr->hash_idx & 0x3e0) >> 1) | (hdr->hash_idx & 0xf));
#else
		rtlglue_printf("    [lpm_idx: %d]\n", hdr->hash_idx & 0x3ff);
#endif
	}
	if (((hdr->fwd_vld >> 6) & 0x3) == 1) {
#ifdef CONFIG_ARCH_CORTINA_G3LITE
		rtlglue_printf("    [hashlite_overflow_idx: 0x%02X]\n", (hdr->hash_idx >> 10) & 0xff);
#else
		rtlglue_printf("    [hashlite_overflow_idx): 0x%03X]\n", (hdr->hash_idx >> 10) & 0x7ff);
#endif
	}
	if (((hdr->fwd_vld >> 6) & 0x2) == 2) {
#ifdef CONFIG_ARCH_CORTINA_G3LITE
		rtlglue_printf("    [hashlite_idx: 0x%02X]\n", (hdr->hash_idx >> 10) & 0xff);
#else
		rtlglue_printf("    [hashlite_idx: 0x%03X]\n", (hdr->hash_idx >> 10) & 0x7ff);
#endif
	}

	// --------------------------- //

	/* Some HDR_A fields filled by L2FE/NI are manipulated by L3FE.PP temporarily:
	 *   HDR_A.ldpid --> HDR_I.lspid
	 *   HDR_A.lspid --> HDR_I.o_lspid
	 *   HDR_A.mcgid --> HDR_I.mcgid(ldpid)
	 * These manipulated fields are restored to proper fields by STG1.
	 *
	 * If L3FE_STG0_CTRL.lpb_idx_mode is 0, PP uses HDR_A.ldpid to index reg L3FE_STG0_LDPID_MAP.
	 * If L3FE_STG0_CTRL.lpb_idx_mode is 1, PP uses HDR_A.lspid to index reg L3FE_STG0_LPB_IDX_TBL.
	 */
	rtlglue_printf("<Source Port ID>\n");
	rtlglue_printf("  pspid: 0x%X lspid: 0x%02X o_lspid: 0x%02X\n", hdr->pspid, hdr->lspid, hdr->o_lspid);

	// --------------------------- //

	rtlglue_printf("<Destination Port ID>\n");
	rtlglue_printf("  dpid_pri: %u mc: %u mcgid/ldpid: 0x%03X deepq: %u no_drop: %u mrr_en: %u\n",
		hdr->dpid_pri, hdr->mc, hdr->mcgid, hdr->deepq, hdr->no_drop, hdr->mrr_en);

	// --------------------------- //

	rtlglue_printf("<QOS>\n");
	rtlglue_printf("  cos: %u  pol_grp_id: 0x%X pol_id: 0x%03X pol_en: %u pol_all_bypass: %u qos_premark: %u\n", hdr->cos,
		hdr->pol_grp_id, hdr->pol_id, hdr->pol_en, hdr->pol_all_bypass, hdr->qos_premark);

	// --------------------------- //

	rtlglue_printf("<MDATA>\n");
	rtlglue_printf("  mdata_h: 0x%08X, mdata_l: 0x%08X\n", hdr->mdata_h, hdr->mdata_l);
#ifdef CONFIG_ARCH_CORTINA_G3LITE
	rtlglue_printf("    [spcl_pkt_enc(6): 0x%02X, L3_ingress_if_id(8): 0x%02X, L3_egress_if_id(8): 0x%02X, lpm_result(12): 0x%03X, mc_fib_id(8): 0x%02X]\n",
		(hdr->mdata_h >> 26) & 0x3f, (hdr->mdata_l >> 24) & 0xff, (hdr->mdata_l >> 16) & 0xff, hdr->mdata_l & 0xfff, hdr->mdata_l & 0xff);
#else
	rtlglue_printf("  [ingress_if_id: 0x%02X, egress_if_id: 0x%02X, sw_id: %d]\n",
		(hdr->mdata_l >> 24) & 0xff, (hdr->mdata_l >> 16) & 0xff, hdr->mdata_l & 0xffff);
#endif
	rtlglue_printf("  [trap_rsn: %s, acl_rsn: 0x%03X]\n", name_mdata_h_trap_rsn[(hdr->mdata_h&RXINFO_REF_TRAP_RSN_BIT)>>RXINFO_REF_TRAP_RSN_SHIFT_BIT], (hdr->mdata_h&RXINFO_REF_ACL_RSN_BIT));

	// --------------------------- //

	if(hdr->spcl_pkt_enc != 0) {
		rtlglue_printf("<Special Packet>\n");
		printk(KERN_INFO "  spcl_pkt_enc(6): 0x%02X, spcl_pkt_hdr_mtch(8): 0x%02X\n", hdr->spcl_pkt_enc, hdr->spcl_pkt_hdr_mtch);
	}

	// --------------------------- //

	rtlglue_printf("<L2>\n");
	rtlglue_printf("  l2_pkt_type: %u ", hdr->l2_pkt_type);
	switch (hdr->l2_pkt_type) {
	case 0: rtlglue_printf("[L2 Unicast]\n"); break;
	case 1: rtlglue_printf("[L2 Multicast]\n"); break;
	case 2: rtlglue_printf("[L2 Broadcast]\n"); break;
	default: rtlglue_printf("[Unknown Unicast/Multicast]\n"); break;
	}
	rtlglue_printf("  mac_da: %02X-%02X-%02X-%02X-%02X-%02X, mac_da_an_sel(4): 0x%X\n",
		hdr->mac_da_5, hdr->mac_da_4, hdr->mac_da_3, hdr->mac_da_2, hdr->mac_da_1, hdr->mac_da_0, hdr->mac_da_an_sel);
	rtlglue_printf("  mac_sa: %02X-%02X-%02X-%02X-%02X-%02X, ethertype(16): 0x%04X\n",
		hdr->mac_sa_5, hdr->mac_sa_4, hdr->mac_sa_3, hdr->mac_sa_2, hdr->mac_sa_1, hdr->mac_sa_0,
		hdr->ethertype);

	// --------------------------- //

	rtlglue_printf("<VLAN>\n");
	rtlglue_printf("  vlan_cnt: %u\n", hdr->vlan_cnt);
	rtlglue_printf("  top_tpid_enc: %u top_vid: 0x%03X top_1p: %u top_dei: %u\n", hdr->top_tpid_enc, hdr->top_vid, hdr->top_802_1p, hdr->top_dei);
	rtlglue_printf("  inner_tpid_enc: %u inner_vid: 0x%03X inner_802_1p: %u inner_dei: %u\n", hdr->inner_tpid_enc, hdr->inner_vid, hdr->inner_802_1p, hdr->inner_dei);

	// --------------------------- //

	rtlglue_printf("<PPP / PPPoE>\n");
	rtlglue_printf("  type: %u [%s] code: 0x%X session_id: 0x%04X protocol_enc(4): 0x%X\n",
		hdr->pppoe_type,
		(hdr->pppoe_type == 0) ? "INVALID" : ((hdr->pppoe_type == 1) ? "PPPoE Discovery" : "PPPoE Session"),
		hdr->pppoe_code_enc, hdr->pppoe_session_id, hdr->ppp_protocol_enc);

	// --------------------------- //

	rtlglue_printf("<L3>\n");
	rtlglue_printf("  ip_vld: %u ip_ver: %u [%s] ip_mtu_enc: %u ip_mtu_en: %u\n",
		hdr->ip_vld, hdr->ip_ver,
		(hdr->ip_ver == 0) ? "IPv4" : "IPv6",
		hdr->ip_mtu_enc, hdr->ip_mtu_en);

	// --------------------------- //

	rtlglue_printf("<IP>\n");
	rtlglue_printf("  ip_protocol: 0x%02X ip_l4_type: %u", hdr->ip_protocol, hdr->ip_l4_type);
	switch (hdr->ip_l4_type) {
	case 1: rtlglue_printf("[TCP]\n"); break;
	case 2: rtlglue_printf("[UDP]\n"); break;
	case 3: rtlglue_printf("[UDP-Lite]\n"); break;
	case 4: rtlglue_printf("[RDPv1]\n"); break;
	case 5: rtlglue_printf("[RDPv2]\n"); break;
	default: rtlglue_printf("[L4 INVALID]\n"); break;
	}

	rtlglue_printf("  ip_sa: %08X-%08X-%08X-%08X, ip_sa_enc: 0x%X\n",
		hdr->ip_sa_3, hdr->ip_sa_2, hdr->ip_sa_1, hdr->ip_sa_0, hdr->ip_sa_enc);
	rtlglue_printf("  ip_da: %08X-%08X-%08X-%08X, ip_da_enc: 0x%X, da_mc: %u, da_sa_equal: %u\n",
		hdr->ip_da_3, hdr->ip_da_2, hdr->ip_da_1, hdr->ip_da_0, hdr->ip_da_enc, hdr->ip_da_mc, hdr->ip_da_sa_equal);
	rtlglue_printf("  ip_dscp: 0x%02X markdown_en: %u marked_down: 0x%02X ip_ecn: %u ip_ecn_en: %u\n",
		hdr->ip_dscp, hdr->ip_dscp_markdown_en, hdr->ip_dscp_marked_down, hdr->ip_ecn, hdr->ip_ecn_en);
	rtlglue_printf("  ipv6_flow_lbl: 0x%05X ip_ttl: 0x%02X ip_options: %u ip_fragment: %u\n",
		hdr->ipv6_flow_lbl, hdr->ip_ttl, hdr->ip_options, hdr->ip_fragment);
	rtlglue_printf("  ipv6_ndp: %u ipv6_hbh: %u ipv6_rh: %u ipv6_doh: %u\n",
		hdr->ipv6_ndp, hdr->ipv6_hbh, hdr->ipv6_rh, hdr->ipv6_doh);

	rtlglue_printf("  icmp_vld: %u ", hdr->icmp_vld);
	switch (hdr->icmp_vld) {
	case 3: rtlglue_printf("[ICMPv6 NDP]"); break;
	case 4: rtlglue_printf("[ICMPv4]"); break;
	case 5: rtlglue_printf("[ICMPv6]"); break;
	case 6: rtlglue_printf("[IGMP]"); break;
	case 7: rtlglue_printf("[MLD]"); break;
	default: rtlglue_printf("[INVALID]"); break;
	}
	rtlglue_printf(" icmp_type: 0x%02X\n", hdr->icmp_type);

	rtlglue_printf("  spi_vld: %u ", hdr->spi_vld);
	switch (hdr->spi_vld) {
	case 1: rtlglue_printf("[IPSec AH]"); break;
	case 2: rtlglue_printf("[IPSec ESP]"); break;
	case 3: rtlglue_printf("[L2TP over IP, control]"); break;
	case 4: rtlglue_printf("[L2TP over IP, data]"); break;
	case 5: rtlglue_printf("[L2TP over UDP, control]"); break;
	case 6: rtlglue_printf("[L2TP over UDP, data]"); break;
	default: rtlglue_printf("[INVALID]"); break;
	}
	rtlglue_printf(" spi/l2tp_session_id: 0x%08X\n", hdr->spi);

	rtlglue_printf("  l3_chksum: 0x%04X l3_chksum_err: %u\n", hdr->l3_chksum, hdr->l3_chksum_err);

	// --------------------------- //

	rtlglue_printf("<L4>\n");
	rtlglue_printf("  l4_sp: 0x%04X l4_dp: 0x%04X port_rng_match_vec: 0x%08X tcp_rdp_ctrl: 0x%03X\n",
		hdr->l4_sp, hdr->l4_dp, hdr->l4_port_rng_match_vec, hdr->tcp_rdp_ctrl);
	rtlglue_printf("  l4_chksum_zero: %u l4_chksum: 0x%08X\n", hdr->l4_chksum_zero, hdr->l4_chksum);
}


static void dump_l2fe_hdr_a(aal_l2_fe_heada_t *hdr)
{
	rtlglue_printf("cpu_flg: %u deep_q: %u\n", hdr->cpu_flag, hdr->deep_q);
	rtlglue_printf("pol_grp_id: 0x%X, pol_id: 0x%03X, pol_en: %u ", hdr->policy_group_id, hdr->policy_id, hdr->policy_ena);
	switch (hdr->policy_ena) {
	case 0: rtlglue_printf("[Disabled]\n"); break;
	case 1: rtlglue_printf("[L2 Enabled]\n"); break;
	case 2: rtlglue_printf("[L3 Enabled]\n"); break;
	case 3: rtlglue_printf("[L2 & L3 Enabled]\n"); break;
	default: rtlglue_printf("[INVALID]\n"); break;
	}

	rtlglue_printf("mark: %u mirror: %u no_drop: %u drop_code: %u\n",
		hdr->marked, hdr->mirror, hdr->no_drop, hdr->drop_code);

	rtlglue_printf("rx_pkt_type: %u ", hdr->rx_packet_type);
	switch (hdr->rx_packet_type) {
	case 0: rtlglue_printf("[Unicast or Disable L2 Learning]\n"); break;
	case 1: rtlglue_printf("[Broadcast]\n"); break;
	case 2: rtlglue_printf("[Multicast]\n"); break;
	case 3: rtlglue_printf("[Unknown Unicast or Enable L2 Learning]\n"); break;
	}

	rtlglue_printf("hdr_type: %u ", hdr->header_type);
	switch (hdr->header_type) {
	case 0: rtlglue_printf("[Generic Unicast HDR]\n"); break;
	case 1: rtlglue_printf("[Multicast HDR, MCGID is valid]\n"); break;
	case 2: rtlglue_printf("[CPU bound packet, MCGID contains L2 learning info]\n"); break;
	case 3: rtlglue_printf("[PTP Packet]\n"); break;
	default: rtlglue_printf("[INVALID]\n"); break;
	}

	rtlglue_printf("mcgid: 0x%02X fe_bypass: %u pkt_size: 0x%04X\n",
		hdr->mc_group_id, hdr->fe_bypass, hdr->packet_size);
	rtlglue_printf("lspid: 0x%02X ldpid: 0x%02X cos: %u\n", hdr->logic_spid, hdr->logic_dpid, hdr->cos);
}

static void dump_l3fe_hdr_a(L3FE_HDR_A_t *hdr)
{
	rtlglue_printf("cpu_flg: %u deep_q: %u\n", hdr->cpu_flg, hdr->deep_q);
	rtlglue_printf("pol_grp_id: 0x%X, pol_id: 0x%03X, pol_en: %u ", hdr->pol_grp_id, hdr->pol_id, hdr->pol_en);
	switch (hdr->pol_en) {
	case 0: rtlglue_printf("[Disabled]\n"); break;
	case 1: rtlglue_printf("[L2 Enabled]\n"); break;
	case 2: rtlglue_printf("[L3 Enabled]\n"); break;
	case 3: rtlglue_printf("[L2 & L3 Enabled]\n"); break;
	default: rtlglue_printf("[INVALID]\n"); break;
	}

	rtlglue_printf("mark: %u mirror: %u no_drop: %u drop_code: %u\n",
		hdr->mark, hdr->mirror, hdr->no_drop, hdr->drop_code);

	rtlglue_printf("rx_pkt_type: %u ", hdr->rx_pkt_type);
	switch (hdr->rx_pkt_type) {
	case 0: rtlglue_printf("[Unicast or Disable L2 Learning]\n"); break;
	case 1: rtlglue_printf("[Broadcast]\n"); break;
	case 2: rtlglue_printf("[Multicast]\n"); break;
	case 3: rtlglue_printf("[Unknown Unicast or Enable L2 Learning]\n"); break;
	}

	rtlglue_printf("hdr_type: %u ", hdr->hdr_type);
	switch (hdr->hdr_type) {
	case 0: rtlglue_printf("[Generic Unicast HDR]\n"); break;
	case 1: rtlglue_printf("[Multicast HDR, MCGID is valid]\n"); break;
	case 2: rtlglue_printf("[CPU bound packet, MCGID contains L2 learning info]\n"); break;
	case 3: rtlglue_printf("[PTP Packet]\n"); break;
	default: rtlglue_printf("[INVALID]\n"); break;
	}

	rtlglue_printf("mcgid: 0x%02X fe_bypass: %u pkt_size: 0x%04X\n",
		hdr->mcgid, hdr->fe_bypass, hdr->pkt_size);
	rtlglue_printf("lspid: 0x%02X ldpid: 0x%02X cos: %u\n", hdr->lspid, hdr->ldpid, hdr->cos);
}

int dump_headera(struct seq_file *s, void *v)
{
	ca_status_t ret = CA_E_OK;
	aal_l2_fe_heada_t l2HdrA_before;
	aal_l2_fe_heada_t l2HdrA_after;
	L3FE_HDR_A_t l3HdrA_before;
	L3FE_HDR_A_t l3HdrA_after;
	int i, count;
	uint32_t *val = NULL;
	L3FE_GLB_DBG_IDX_t dbgVal;


	/* Get L2 HDR_A */
	ASSERT_EQ((ret = aal_l2_fe_pp_heada_get(G3_DEF_DEVID, &l2HdrA_before)), CA_E_OK);

	/* Get L2 HDR_A */
	ASSERT_EQ((ret = aal_l2_fe_pe_heada_get(G3_DEF_DEVID, &l2HdrA_after)), CA_E_OK);

	/* Get L3 HDR_A Before */
	//ASSERT_EQ((ret = aal_l3fe_pp_header_a_get(&l3HdrA)), CA_E_OK);
	val = (uint32_t *)&l3HdrA_before;
	*val = rtk_ne_reg_read(L3FE_PP_HEADER_A_LOW);
	val++;
	*val = rtk_ne_reg_read(L3FE_PP_HEADER_A_HI);

	/* Get L3 HDR_A After */
	//if ((ret = aal_l3fe_glb_dbg_get(14, &hdr_a, sizeof(hdr_a))) != AAL_E_OK)
	val = (uint32_t *)&l3HdrA_after;
	/* total read count is ceil(size/sizeof(int)) */
	count = (sizeof(L3FE_HDR_A_t) + sizeof(*val) - 1) / sizeof(*val);
	for (i = 0; i < count; i++) {
		/* idx[9:5] is debug vector, idx[4:0] chooses data bit range */
		dbgVal.bf.idx = (DBG_HDR_A_PE << 5) | i;
		rtk_ne_reg_write(dbgVal.wrd, L3FE_GLB_DBG_IDX);
		val[i] = rtk_ne_reg_read(L3FE_GLB_DBG_DAT);
	}


	/* Display L2 HDR_A Before */
	rtlglue_printf("---- "COLOR_Y "[L2 HDR_A - PktParser]" COLOR_NM "------------------------------------\n");
	dump_l2fe_hdr_a(&l2HdrA_before);
	rtlglue_printf("----------------------------------------------\n");

	/* Display L2 HDR_A After */
	rtlglue_printf("---- "COLOR_Y "[L2 HDR_A - PktEditor]" COLOR_NM "------------------------------------\n");
	dump_l2fe_hdr_a(&l2HdrA_after);
	rtlglue_printf("----------------------------------------------\n");

	/* Display L3 HDR_A Before */
	rtlglue_printf("---- "COLOR_Y "[L3 HDR_A - PktParser]" COLOR_NM "------------------------------------\n");
	dump_l3fe_hdr_a(&l3HdrA_before);
	rtlglue_printf("----------------------------------------------\n");

	/* Display L3 HDR_A After */
	rtlglue_printf("---- "COLOR_Y "[L3 HDR_A - PktEditor]" COLOR_NM "------------------------------------\n");
	dump_l3fe_hdr_a(&l3HdrA_after);
	rtlglue_printf("----------------------------------------------\n");


	return ret;
}
int dump_headeri(struct seq_file *s, void *v)
{
	ca_status_t ret = CA_E_OK;
	L3FE_HDR_I_t l3HdrI_before;
	L3FE_HDR_I_t l3HdrI_after;
	int i, count;
	uint32_t *val = NULL;
	L3FE_GLB_DBG_IDX_t dbgVal;
	char acl_dbg_info[64];

	//aal_l3fe_glb_cls_stg_monitor_get(l3fe_glb_monitor_vector_e where, void * data, size_t size)
	val =  (uint32_t *)&l3HdrI_before;
	count = (sizeof(L3FE_HDR_I_t) + sizeof(*val) - 1) / sizeof(*val);
	for (i = 0; i < count; i++) {
		/* idx[9:5] is debug vector, idx[4:0] chooses data bit range */
		dbgVal.bf.idx = (DBG_HDR_I_T1_T2 << 5) | i;
		rtk_ne_reg_write(dbgVal.wrd, L3FE_GLB_DBG_IDX);
		val[i] = rtk_ne_reg_read(L3FE_GLB_DBG_DAT);
	}


	val =  (uint32_t *)&l3HdrI_after;
	count = (sizeof(L3FE_HDR_I_t) + sizeof(*val) - 1) / sizeof(*val);
	for (i = 0; i < count; i++) {
		/* idx[9:5] is debug vector, idx[4:0] chooses data bit range */
		dbgVal.bf.idx = (DBG_HDR_I_TE_PE << 5) | i;
		rtk_ne_reg_write(dbgVal.wrd, L3FE_GLB_DBG_IDX);
		val[i] = rtk_ne_reg_read(L3FE_GLB_DBG_DAT);
	}


	/* Display L3 HDR_I Before */
	rtlglue_printf("---- "COLOR_Y "[L3 HDR_I - CLS_HASH]" COLOR_NM "------------------------------------\n");
	dump_l3fe_hdr_i(&l3HdrI_before);
	rtlglue_printf("----------------------------------------------\n");


	/* Display L3 HDR_I After */
	rtlglue_printf("---- "COLOR_Y "[L3 HDR_I - TE_PE]" COLOR_NM "------------------------------------\n");
	dump_l3fe_hdr_i(&l3HdrI_after);
	rtlglue_printf("----------------------------------------------\n");

	if(l3HdrI_after.fwd_vld & 0x1) {
		memset(acl_dbg_info, 0, sizeof(acl_dbg_info));
		if(l3HdrI_after.mdata_h & RXINFO_REF_ACL_RSN_BIT /*mdata_w_2*/)
			_rtk_rg_aclHeaderInfo_get(acl_dbg_info, sizeof(acl_dbg_info), (l3HdrI_after.mdata_h&RXINFO_REF_ACL_RSN_BIT));
		rtlglue_printf("---- "COLOR_Y "[L3 HIT CLS - HW info and SW idx]" COLOR_NM COLOR_G "%s" COLOR_NM "------------------------------------\n", acl_dbg_info);
		ca_used_aal_entry_print_by_fib_idx(G3_DEF_DEVID, l3HdrI_after.cls_action & 0x3ff);
		rtlglue_printf("----------------------------------------------\n");
	}

	return ret;
}

char CounterDesc[L3PE_CNTR_DROP_SRC_ENTRY_MAX][80] = {
	"PP: L2 pkt size illigal (L2_pkt_size < L2_hdr_size + L3_total_len)", // 0
	"PP: L3L4 header offset exceeds 254",
	"PP: IPv4 header checksum validation error",
	"PP: UDP checksum zero",
	"PP: Other detected errors",
	"PP: More than 2 VLAN ID", // 5
	"LPB: MRU check fail",
	"LPB: MyMAC filtering",
	"LPB: default drop",
	"Special packet drop",
	"reserved", // 10
	"DSLite/6RD: Decapsulate ECN check fail",
	"DSLite/6RD: Outer IPDA and IPSA reverse path of forwarding check fail",
	"6RD: outer and inner IPSA consistency check fail",
	"6RD: Decapsulate 6RD IPDA delegated prefix check fail",
	"6RD: Decapsulate 6RD IPMC drop", // 15
	"6RD: outer IPMC with inner IPUC packet",
	"DSLite: IPMC addr prefix check fail",
	"DSLite: IPMC embedded IPv4 addr check fail",
	"DSLite: outer IPMC with inner IPUC packet",
	"DSLite/6RD: IF ID check fail", // 20
	"DSLite: Decapsulate DSLite IPMC drop",
	"Hash Engine double check fail",
	"LPM search miss",
	"HashLite Engine double check fail",
	"T5: ILPB filter drop", // 25
	"T5: MC drop",
	"T5: SA deny",
	"T5: DA deny",
	"T5: software learning deny",
	"T5: unknown VLAN", // 30
	"T5: VLAN membership check fail",
	"T5: STP mode block drop",
	"T5: STP mode learn only and SMAC hit drop",
	"T5: VE deny",
	"T5: VE MC VLAN drop", // 35
	"T5: VE FIB drop",
	"T5: L2E FIB drop",
	"TE: reserved1",
	"TE: reserved2",
	"TE: unmarked WRED drop", // 40
	"TE: marked WRED drop",
	"TE: tail drop",
	"TE: policer drop",
	"TE: reserved7",
	"PE: TCP/UDP/RDPv1/RDPv2 checksum error", // 45
	"PE: L2 CRC error",
	"PE: MTU check fail",
	"STG1: L3 IF table",
	"STG1: FIB LDPID is blackhole",
	"STG1: TTL is 0", // 50
	"STG1: CR other drop",
	"STG2: L3 IF table",
	"STG2: FIB LDPID is blackhole",
	"STG2: TTL is 0",
	"STG2: CR other drop", // 55
	"STG3.U1: L3 IF table",
	"STG3.U1: FIB LDPID is blackhole",
	"STG3.U1: TTL is 0",
	"STG3.U1: CR other drop",
	"STG3.U2: L3 IF table", // 60
	"STG3.U2: FIB LDPID is blackhole",
	"STG3.U2: TTL is 0",
	"STG3.U2: CR other drop",
};
int dump_dropcount(struct seq_file *s, void *v)
{
	ca_status_t ret = CA_E_OK;
	ca_uint16_t num16 = 0;
	int i =0;
	uint64 cntPkt;
	aal_l2_fe_drop_source_t l2fedrop;
	aal_l2_te_pm_egress_t pm_stats;
	aal_l3_te_pm_egress_t l3pm_stats;
	aal_l2_te_pm_policer_t	pm_pol;
	aal_l3_te_pm_policer_t l3pm_pol;


	PROC_PRINTF("usage:\t cat THISPROC before and after your test to show if any hw drop happens.\n");
	PROC_PRINTF("\t echo 0 > THISPROC to clear voq max buffer count.\n");
	PROC_PRINTF("\t echo [OpCode] > THISPROC to show page usage.\n");
	PROC_PRINTF("\t\t OpCode_bit0 (0x1): Port Free Buffer Count and High/Low Threshold\n");
	PROC_PRINTF("\t\t OpCode_bit1 (0x2): Voq NON-ZERO Used Pages and High/Low Threshold\n");
	PROC_PRINTF("\t\t OpCode_bit2 (0x4): Voq ALL      Used Pages and High/Low Threshold\n");
	PROC_PRINTF("\t\t OpCode_bit3 (0x8): L2TM and L3QM page size and numbers\n\n");

	// port mib
	// TODO: ...

	// L2FE drop cnt
	if((ret =aal_l2_fe_drop_source_cnt_get(G3_DEF_DEVID, &l2fedrop))!= AAL_E_OK){
		PROC_PRINTF("ERROR! Fail to get l2fe drop count,  ret=%d)\n", ret);

	}else {
		PROC_PRINTF("===== "COLOR_Y"L2FE drop counter"COLOR_NM" ===== (read-and-clear)\n");
		if(l2fedrop.drop_source_cnt_00 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_00", l2fedrop.drop_source_cnt_00);
		if(l2fedrop.ipv4_cs_chk_drp_01 != 0) PROC_PRINTF("%-40s: %13u\n", "ipv4_cs_chk_drp_01", l2fedrop.ipv4_cs_chk_drp_01);
		if(l2fedrop.dpid_blackhole_drp_02 != 0) PROC_PRINTF("%-40s: %13u\n", "dpid_blackhole_drp_02", l2fedrop.dpid_blackhole_drp_02);
		if(l2fedrop.ilpb_stp_chk_drp_03 != 0) PROC_PRINTF("%-40s: %13u\n", "ilpb_stp_chk_drp_03", l2fedrop.ilpb_stp_chk_drp_03);
		if(l2fedrop.vlan_stp_chk_drp_04 != 0) PROC_PRINTF("%-40s: %13u\n", "vlan_stp_chk_drp_04", l2fedrop.vlan_stp_chk_drp_04);
		if(l2fedrop.ilpb_vlan_type_drp_05 != 0) PROC_PRINTF("%-40s: %13u\n", "ilpb_vlan_type_drp_05", l2fedrop.ilpb_vlan_type_drp_05);
		if(l2fedrop.igr_cls_pmt_stp_drp_06 != 0) PROC_PRINTF("%-40s: %13u\n", "igr_cls_pmt_stp_drp_06", l2fedrop.igr_cls_pmt_stp_drp_06);
		if(l2fedrop.vid4095_drp_07 != 0) PROC_PRINTF("%-40s: %13u\n", "vid4095_drp_07", l2fedrop.vid4095_drp_07);
		if(l2fedrop.unknown_vlan_drp_08 != 0) PROC_PRINTF("%-40s: %13u\n", "unknown_vlan_drp_08", l2fedrop.unknown_vlan_drp_08);
		if(l2fedrop.ve_deny_mc_flag_drp_09 != 0) PROC_PRINTF("%-40s: %13u\n", "ve_deny_mc_flag_drp_09", l2fedrop.ve_deny_mc_flag_drp_09);
		if(l2fedrop.l2e_da_deny_drp_10 != 0) PROC_PRINTF("%-40s: %13u\n", "l2e_da_deny_drp_10", l2fedrop.l2e_da_deny_drp_10);
		if(l2fedrop.non_std_sa_drp_11 != 0) PROC_PRINTF("%-40s: %13u\n", "non_std_sa_drp_11", l2fedrop.non_std_sa_drp_11);
		if(l2fedrop.sw_learn_err_drp_12 != 0) PROC_PRINTF("%-40s: %13u\n", "sw_learn_err_drp_12", l2fedrop.sw_learn_err_drp_12);
		if(l2fedrop.l2e_sa_deny_drp_13 != 0) PROC_PRINTF("%-40s: %13u\n", "l2e_sa_deny_drp_13", l2fedrop.l2e_sa_deny_drp_13);
		if(l2fedrop.sa_mis_or_sa_limit_drp_14 != 0) PROC_PRINTF("%-40s: %13u\n", "sa_mis_or_sa_limit_drp_14", l2fedrop.sa_mis_or_sa_limit_drp_14);
		if(l2fedrop.uuc_umc_bc_drp_15 != 0) PROC_PRINTF("%-40s: %13u\n", "uuc_umc_bc_drp_15", l2fedrop.uuc_umc_bc_drp_15);
		if(l2fedrop.spid_dpid_not_in_rx_vlan_mem_drp_16 != 0) PROC_PRINTF("%-40s: %13u\n", "spid_dpid_not_in_rx_vlan_mem_drp_16", l2fedrop.spid_dpid_not_in_rx_vlan_mem_drp_16);
		if(l2fedrop.dpid_not_in_port_mem_drp_17 != 0) PROC_PRINTF("%-40s: %13u\n", "dpid_not_in_port_mem_drp_17", l2fedrop.dpid_not_in_port_mem_drp_17);
		if(l2fedrop.spid_dpid_not_in_tx_vlan_mem_drp_18 != 0) PROC_PRINTF("%-40s: %13u\n", "spid_dpid_not_in_tx_vlan_mem_drp_18", l2fedrop.spid_dpid_not_in_tx_vlan_mem_drp_18);
		if(l2fedrop.tx_vlan_stp_drp_19 != 0) PROC_PRINTF("%-40s: %13u\n", "tx_vlan_stp_drp_19", l2fedrop.tx_vlan_stp_drp_19);
		if(l2fedrop.egr_cls_deny_drp_20 != 0) PROC_PRINTF("%-40s: %13u\n", "egr_cls_deny_drp_20", l2fedrop.egr_cls_deny_drp_20);
		if(l2fedrop.loopback_drp_21 != 0) PROC_PRINTF("%-40s: %13u\n", "loopback_drp_21", l2fedrop.loopback_drp_21);
		if(l2fedrop.elpb_stp_drp_22 != 0) PROC_PRINTF("%-40s: %13u\n", "elpb_stp_drp_22", l2fedrop.elpb_stp_drp_22);
		if(l2fedrop.drop_source_cnt_23 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_23", l2fedrop.drop_source_cnt_23);
		if(l2fedrop.drop_source_cnt_24 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_24", l2fedrop.drop_source_cnt_24);
		if(l2fedrop.mcfib_vlan_drp_25 != 0) PROC_PRINTF("%-40s: %13u\n", "mcfib_vlan_drp_25", l2fedrop.mcfib_vlan_drp_25);
		if(l2fedrop.drop_source_cnt_26 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_26", l2fedrop.drop_source_cnt_26);
		if(l2fedrop.mcfib_dpid_equal_spid_drp_27 != 0) PROC_PRINTF("%-40s: %13u\n", "mcfib_dpid_equal_spid_drp_27", l2fedrop.mcfib_dpid_equal_spid_drp_27);
		if(l2fedrop.drop_source_cnt_28 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_28", l2fedrop.drop_source_cnt_28);
		if(l2fedrop.drop_source_cnt_29 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_29", l2fedrop.drop_source_cnt_29);
		if(l2fedrop.drop_source_cnt_30 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_30", l2fedrop.drop_source_cnt_30);
		if(l2fedrop.drop_source_cnt_31 != 0) PROC_PRINTF("%-40s: %13u\n", "drop_source_cnt_31", l2fedrop.drop_source_cnt_31);
	}
	PROC_PRINTF("\n");

	// l3FE drop cnt
	PROC_PRINTF("===== "COLOR_Y"L3FE drop counter"COLOR_NM" ===== (read-and-clear)\n");
	for (i = 0; i < L3PE_CNTR_DROP_SRC_ENTRY_MAX; i++) {
		if ((ret = aal_l3pe_cntr_drop_src_pkt_get(G3_DEF_DEVID, i, &cntPkt)) != AAL_E_OK) {
			PROC_PRINTF("ERROR! Fail to get l3fe drop count. (counter index=%lu, ret=%d)\n", i, ret);
			continue;
		}
		if(cntPkt != 0) {
			PROC_PRINTF("%-40s: %13u\n", CounterDesc[i], cntPkt);
		}

	}
	aal_l3pe_cntr_drop_src_pkt_clear(G3_DEF_DEVID);
	PROC_PRINTF("\n");


	// l2te voq
	PROC_PRINTF("===== "COLOR_Y"L2TE voq  counter"COLOR_NM" ===== (read-and-clear)\n");
	PROC_PRINTF("%-4s %-10s  %-13s  %-13s  %-13s  %-13s  %-13s\r\n", "Voq", "Port-Voq", "TotalPkt", "TailDropPkt", "WredMarkPkt", "WredUnMarkPkt", "Max-Buf");
	PROC_PRINTF("-----------------------------------------------------------------------------------\r\n");
	for (i = 0; i < CA_AAL_MAX_VOQ_ID; i++) {
		memset(&pm_stats, 0, sizeof(pm_stats));
		ret = aal_l2_te_pm_egress_voq_get(G3_DEF_DEVID, i, &pm_stats);
		if (CA_E_OK != ret) {
			PROC_PRINTF("ERROR! Fail to get l2 te voq counter. (voq index=%lu, ret=%d)\n", i, ret);
			continue;
		}
		if (0 == pm_stats.total_pkt + pm_stats.td_drop_pkt + pm_stats.wred_mark_pkt + pm_stats.wred_unmark_pkt)
			continue;

		ret = aal_l2_tm_cb_voq_max_buf_get(G3_DEF_DEVID, i, &num16);

		PROC_PRINTF("%-4u %2u-%-7u  %-13u  %-13u  %-13u  %-13u  %-13u\r\n",
			i, i/8, i%8,pm_stats.total_pkt, pm_stats.td_drop_pkt, pm_stats.wred_mark_pkt, pm_stats.wred_unmark_pkt, num16);
	}
	PROC_PRINTF("\n");

	// l3te voq
	PROC_PRINTF("===== "COLOR_Y"L3TE voq  counter"COLOR_NM" ===== (read-and-clear)\n");
	PROC_PRINTF("%-4s %-10s  %-13s  %-13s  %-13s  %-13s  %-13s\r\n", "Voq", "Port-Voq", "TotalPkt", "TailDropPkt", "WredMarkPkt", "WredUnMarkPkt", "Max-Buf");
	PROC_PRINTF("-----------------------------------------------------------------------------------\r\n");
	for (i = 0; i < CA_AAL_MAX_VOQ_ID; i++) {
		memset(&l3pm_stats, 0, sizeof(l3pm_stats));
		ret = aal_l3_te_pm_egress_voq_get(G3_DEF_DEVID, i, &l3pm_stats);
		if (CA_E_OK != ret) {
			PROC_PRINTF("ERROR! Fail to get l3 te voq counter. (voq index=%lu, ret=%d)\n", i, ret);
			continue;
		}
		if (0 == l3pm_stats.total_pkt + l3pm_stats.td_drop_pkt + l3pm_stats.wred_mark_pkt + l3pm_stats.wred_unmark_pkt)
			continue;

		ret = aal_l3_te_cb_voq_max_buf_get(G3_DEF_DEVID, i, &num16);

		PROC_PRINTF("%-4u %s%u-%-4u  %-13u  %-13u  %-13u  %-13u  %-13u\r\n",
			i, i<64?"cpu ":"ni  ", (i<64)?(i/8):(i/8-8), i%8, l3pm_stats.total_pkt, l3pm_stats.td_drop_pkt, l3pm_stats.wred_mark_pkt, l3pm_stats.wred_unmark_pkt, num16);
	}
	PROC_PRINTF("\n");

	// l2te pol
	PROC_PRINTF("===== "COLOR_Y"L2TE pol  counter"COLOR_NM" ===== (read-and-clear)\n");
	PROC_PRINTF("%-4s  %-8s  %-13s  %-13s  %-13s\r\n", "Pol", "TotalPkt", "RedPkt(drop)", "YellowPkt", "GreenPkt");
	PROC_PRINTF("-----------------------------------------------------------------------------------\r\n");
	for (i = 0; i < CA_AAL_MAX_FLOW_ID; i++) {
		memset(&pm_pol, 0, sizeof(pm_pol));
		ret = aal_l2_te_pm_policer_flow_get(G3_DEF_DEVID, i, &pm_pol);
		if (CA_E_OK != ret) {
			PROC_PRINTF("ERROR! Fail to get l3 te pol counter. (pol index=%lu, ret=%d)\n", i, ret);
			continue;
		}
		if ((cntPkt = (pm_pol.red_pkt + pm_pol.yellow_pkt + pm_pol.green_pkt)) == 0)
			continue;

		PROC_PRINTF("%-4u  %-8llu  %-13u  %-13u  %-13u\r\n",
			i, cntPkt, pm_pol.red_pkt, pm_pol.yellow_pkt, pm_pol.green_pkt);
	}
	PROC_PRINTF("\n");

	// l3te pol
	PROC_PRINTF("===== "COLOR_Y"L3TE pol  counter"COLOR_NM" ===== (read-and-clear)\n");
	PROC_PRINTF("%-4s  %-8s  %-13s  %-13s  %-13s\r\n", "Pol", "TotalPkt", "RedPkt(drop)", "YellowPkt", "GreenPkt");
	PROC_PRINTF("-----------------------------------------------------------------------------------\r\n");
	for (i = 0; i < CA_AAL_L3_MAX_FLOW_ID; i++) {
		memset(&l3pm_pol, 0, sizeof(l3pm_pol));
		ret = aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, i, &l3pm_pol);
		if (CA_E_OK != ret) {
			PROC_PRINTF("ERROR! Fail to get l3 te pol counter. (pol index=%lu, ret=%d)\n", i, ret);
			continue;
		}
		if ((cntPkt = (l3pm_pol.red_pkt + l3pm_pol.yellow_pkt + l3pm_pol.green_pkt)) == 0)
			continue;

		PROC_PRINTF("%-4u  %-8llu  %-13u  %-13u  %-13u\r\n",
			i, cntPkt, l3pm_pol.red_pkt, l3pm_pol.yellow_pkt, l3pm_pol.green_pkt);
	}
	PROC_PRINTF("\n");

	return ret;
}
int32 dump_dropcount_opcode(struct file *file, const char *buffer, unsigned long count, void *data)
{
	ca_status_t ret = CA_E_OK;
	int i =0, opcode = 0;
	aal_l2_qm_eq_cfg_t l2qmeq_cfg0, l2qmeq_cfg1;
	aal_l2_tm_cb_free_buf_cnt_t free_buf_cnt_l2;
	aal_l3_te_cb_free_buf_cnt_t free_buf_cnt_l3;
	aal_l2_tm_cb_comb_threshold_t tecbportthresh_l2;
	aal_l2_tm_cb_threshold_t qmschportthresh_l2;
	aal_l3_te_cb_comb_threshold_t tecbportthresh_l3;
	aal_l3_te_cb_threshold_t qmschportthresh_l3;
	ca_uint8_t profileid = 0;
	ca_uint32_t reg_off = 0, value = 0;
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !COPY_FROM_USER(tmpBuf, buffer, len))
	{
		opcode = simple_strtoul(tmpBuf, NULL, 16);
	}

	// TM/QM page size & buffer size
	// port/queue threshold(H/L) & port current(free) buffer

	if(opcode == 0){
		rtlglue_printf(COLOR_Y"clear voq max buffer count ...\r\n"COLOR_NM);

		for (i = 0; i < CA_AAL_MAX_VOQ_ID; i++) {
			ret = aal_l2_tm_cb_voq_max_buf_set(G3_DEF_DEVID, i, 0);
			ret = aal_l3_te_cb_voq_max_buf_set(G3_DEF_DEVID, i, 0);
		}
	}
	if(opcode & (1<<0)){
		rtlglue_printf(COLOR_Y"0x1: Port Free Buffer Count and High/Low Threshold ..."COLOR_NM"\r\n");

		ret = aal_l2_qm_eq_glb_free_cnt_get(G3_DEF_DEVID, &value);
		rtlglue_printf("L2TM run-time shared free buffer cnt: %d\r\n", value);

		rtlglue_printf("%-9s  %-6s  %-6s  %-11s  %-11s | %-10s  %-6s  %-6s  %-10s  %-11s\r\n",
			"L2TE-Port", "Pool0", "Pool1", "b.p. Thrsh", "drop Thrsh", "L3TE-Port", "Pool0", "Pool1", "b.p. Thrsh", "drop Thrsh");

		for (i = 0; i <= CA_AAL_MAX_PORT_ID; i++) {
			if(i < CA_AAL_L3_TE_NI_PORT_OFFSET) {
				// phy port 0~7
				ret = aal_l2_tm_cb_port_free_buf_cnt_get(G3_DEF_DEVID, i, &free_buf_cnt_l2);	//port_free_buf_cnt counts shred buffers
				ret = aal_l3_te_cb_port_free_buf_cnt_get(G3_DEF_DEVID, i+CA_AAL_L3_TE_NI_PORT_OFFSET, &free_buf_cnt_l3);

				ret = aal_l2_tm_cb_port_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_port_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l2);
				
				ret = aal_l2_tm_cb_deep_q_port_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_deep_q_port_threshold_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l2);

				ret = aal_l3_te_cb_port_profile_sel_get(G3_DEF_DEVID, i+CA_AAL_L3_TE_NI_PORT_OFFSET, &profileid);
				ret = aal_l3_te_cb_port_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l3);

				profileid = 0;
				ret = aal_l3_te_cb_dqsch_port_thrsh_profile_sel_get(G3_DEF_DEVID, i, &profileid);				// actually CA didn't implement profile sel, profile index is 0 by default.
				ret = aal_l3_te_cb_dqsch_port_thrsh_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l3);	// L3QM only support hi threshold, there is no low threshold in asic design.


				rtlglue_printf("%-9u  %-6u  %-6u  %-4u / %-4u  %-4u / %-4u | %-4u(ni_%u)  %-6u  %-6u  %-10u  %-4u / %-4u \r\n",
					i, free_buf_cnt_l2.cnt0, free_buf_cnt_l2.cnt1, qmschportthresh_l2.threshold_hi, qmschportthresh_l2.threshold_lo, tecbportthresh_l2.threshold_hi, tecbportthresh_l2.threshold_lo,
					i+CA_AAL_L3_TE_NI_PORT_OFFSET, i, free_buf_cnt_l3.cnt0, free_buf_cnt_l3.cnt1, qmschportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_lo);
			}else {
				// others: l2te 8~15 are physical ports;
				//		l3te 0~7 are cpu ports;
				ret = aal_l2_tm_cb_port_free_buf_cnt_get(G3_DEF_DEVID, i, &free_buf_cnt_l2);
				ret = aal_l3_te_cb_port_free_buf_cnt_get(G3_DEF_DEVID, i-CA_AAL_L3_TE_NI_PORT_OFFSET, &free_buf_cnt_l3);

				ret = aal_l2_tm_cb_port_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_port_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l2);
				
				ret = aal_l2_tm_cb_deep_q_port_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_deep_q_port_threshold_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l2);

				ret = aal_l3_te_cb_port_profile_sel_get(G3_DEF_DEVID, i-CA_AAL_L3_TE_NI_PORT_OFFSET, &profileid);
				ret = aal_l3_te_cb_port_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l3);

				//profileid = 0;
				//ret = aal_l3_te_cb_dqsch_port_thrsh_profile_sel_get(G3_DEF_DEVID, i, &profileid);				// actually CA didn't implement profile sel, profile index is 0 by default.
				//ret = aal_l3_te_cb_dqsch_port_thrsh_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l3);	// L3QM only support hi threshold, there is no low threshold in asic design.

				rtlglue_printf("%-9u  %-6u  %-6u  %-4u / %-4u  %-4u / %-4u | %-3u(cpu_%u)  %-6u  %-6u  %-10s  %-4u / %-4u \r\n",
					i, free_buf_cnt_l2.cnt0, free_buf_cnt_l2.cnt1, qmschportthresh_l2.threshold_hi, qmschportthresh_l2.threshold_lo, tecbportthresh_l2.threshold_hi, tecbportthresh_l2.threshold_lo,
					i-CA_AAL_L3_TE_NI_PORT_OFFSET, i-CA_AAL_L3_TE_NI_PORT_OFFSET, free_buf_cnt_l3.cnt0, free_buf_cnt_l3.cnt1, "N/A", tecbportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_lo);
			}
		}

		rtlglue_printf("-----------------------------------------------------------------------------------\r\n");
	}

	if((opcode & (1<<1)) || (opcode & (1<<2))){
		#define RG_CA_AAL_L3_TE_NI_VOQ_OFFSET ((CA_AAL_MAX_VOQ_ID+1)/2)
		aal_l2_tm_cb_free_buf_cnt_t free_buf_cnt_l2;
		aal_l3_te_cb_free_buf_cnt_t free_buf_cnt_l3;

		if(!(opcode & (1<<2)))
			rtlglue_printf(COLOR_Y"0x2: Voq \"NON-ZERO\" Used Pages and High/Low Threshold ..."COLOR_NM"\r\n");
		else
			rtlglue_printf(COLOR_Y"0x4: Voq \"ALL\"            Used Pages and High/Low Threshold ..."COLOR_NM"\r\n");

		ret = aal_l2_qm_eq_glb_free_cnt_get(G3_DEF_DEVID, &value);
		rtlglue_printf("L2TM run-time shared free buffer cnt: %d\r\n", value);

		rtlglue_printf("%-10s  %-6s  %-6s  %-11s  %-11s | %-10s  %-6s  %-6s  %-10s  %-11s\r\n",
					"L2Port-Voq", "Pool0", "Pool1", "b.p. Thrsh", "drop Thrsh", "L3Port-Voq", "Pool0", "Pool1", "b.p. Thrsh", "drop Thrsh");
		for (i = 0; i <= CA_AAL_MAX_VOQ_ID; i++) {
			if(i < RG_CA_AAL_L3_TE_NI_VOQ_OFFSET) {
				// phy port 0~7
				ret = aal_l2_tm_cb_voq_buf_cnt_get(G3_DEF_DEVID, i, &free_buf_cnt_l2);		// voq_buf_cnt counts shared and private buffers
				ret = aal_l3_te_cb_voq_buf_cnt_get(G3_DEF_DEVID, i+RG_CA_AAL_L3_TE_NI_VOQ_OFFSET, &free_buf_cnt_l3);

				if(!(opcode & (1<<2)) && ((free_buf_cnt_l2.cnt0+free_buf_cnt_l2.cnt1+free_buf_cnt_l3.cnt0+free_buf_cnt_l3.cnt1) == 0))
					continue;

				ret = aal_l2_tm_cb_voq_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l2);
				
				ret = aal_l2_tm_cb_deep_q_voq_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_deep_q_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l2);

				ret = aal_l3_te_cb_voq_profile_sel_get(G3_DEF_DEVID, i+RG_CA_AAL_L3_TE_NI_VOQ_OFFSET, &profileid);
				ret = aal_l3_te_cb_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l3);
				
				profileid = 0;
				ret = aal_l3_te_cb_dqsch_voq_thrsh_profile_sel_get(G3_DEF_DEVID, i, &profileid);				// actually CA didn't implement profile sel, profile index is 0 by default.
				ret = aal_l3_te_cb_dqsch_voq_thrsh_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l3);	// L3QM only support hi threshold, there is no low threshold in asic design.

				rtlglue_printf("%2u-%-7u  %-6u  %-6u  %-4u / %-4u  %-4u / %-4u | ni_%u-%-5u  %-6u  %-6u  %-10u  %-4u / %-4u \r\n",
					i/8, i%8, free_buf_cnt_l2.cnt0, free_buf_cnt_l2.cnt1, qmschportthresh_l2.threshold_hi, qmschportthresh_l2.threshold_lo, tecbportthresh_l2.threshold_hi, tecbportthresh_l2.threshold_lo,
					i/8, i%8, free_buf_cnt_l3.cnt0, free_buf_cnt_l3.cnt1, qmschportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_lo);
			}else {
				// others: l2te 8~15 are physical ports;
				//		l3te 0~7 are cpu ports;
				ret = aal_l2_tm_cb_voq_buf_cnt_get(G3_DEF_DEVID, i, &free_buf_cnt_l2);
				ret = aal_l3_te_cb_voq_buf_cnt_get(G3_DEF_DEVID, i-RG_CA_AAL_L3_TE_NI_VOQ_OFFSET, &free_buf_cnt_l3);

				if(!(opcode & (1<<2)) && ((free_buf_cnt_l2.cnt0+free_buf_cnt_l2.cnt1+free_buf_cnt_l3.cnt0+free_buf_cnt_l3.cnt1) == 0))
					continue;

				ret = aal_l2_tm_cb_voq_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l2);
				
				ret = aal_l2_tm_cb_deep_q_voq_profile_sel_get(G3_DEF_DEVID, i, &profileid);
				ret = aal_l2_tm_cb_deep_q_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l2);

				ret = aal_l3_te_cb_voq_profile_sel_get(G3_DEF_DEVID, i-RG_CA_AAL_L3_TE_NI_VOQ_OFFSET, &profileid);
				ret = aal_l3_te_cb_voq_threshold_profile_get(G3_DEF_DEVID, profileid, &tecbportthresh_l3);
				
				//profileid = 0;
				//ret = aal_l3_te_cb_dqsch_voq_thrsh_profile_sel_get(G3_DEF_DEVID, i, &profileid);				// actually CA didn't implement profile sel, profile index is 0 by default.
				//ret = aal_l3_te_cb_dqsch_voq_thrsh_profile_get(G3_DEF_DEVID, profileid, &qmschportthresh_l3);	// L3QM only support hi threshold, there is no low threshold in asic design.

				rtlglue_printf("%2u-%-7u  %-6u  %-6u  %-4u / %-4u  %-4u / %-4u | cpu_%u-%-4u  %-6u  %-6u  %-10s  %-4u / %-4u \r\n",
					i/8, i%8, free_buf_cnt_l2.cnt0, free_buf_cnt_l2.cnt1, qmschportthresh_l2.threshold_hi, qmschportthresh_l2.threshold_lo, tecbportthresh_l2.threshold_hi, tecbportthresh_l2.threshold_lo,
					(i-RG_CA_AAL_L3_TE_NI_VOQ_OFFSET)/8, i%8, free_buf_cnt_l3.cnt0, free_buf_cnt_l3.cnt1, "N/A", tecbportthresh_l3.threshold_hi, tecbportthresh_l3.threshold_lo);
			}
		}


		rtlglue_printf("-----------------------------------------------------------------------------------\r\n");
	}
	if(opcode & (1<<3)){
		uint32 portidx = 0;
		QM_QM_DEST_PORT0_EQ_CFG_t dest_port_eq_cfg;
		QM_QM_EQ_PROFILE0_t eq_profile;
		QM_QM_CFG1_EQ0_t cfg1_eq0, cfg1_eq1;
		QM_QM_CFG2_EQ0_t cfg2_eq0, cfg2_eq1;

#if defined(CONFIG_ARCH_CORTINA_G3LITE)
		#define L2TM_PAGE_SIZE 32
#elif defined(CONFIG_ARCH_CORTINA_G3HGU)
		#define L2TM_PAGE_SIZE 64
#else
		#define L2TM_PAGE_SIZE 32
#endif

		rtlglue_printf(COLOR_Y"0x8: L2TM and L3QM page size and numbers ..."COLOR_NM"\r\n");

		ret = aal_l2_qm_eq_cfg_get(G3_DEF_DEVID, 0, &l2qmeq_cfg0);
		if(l2qmeq_cfg0.eq_ena == FALSE) {
			rtlglue_printf("L2TM Pool0 is Disabled!\r\n");
		}else {

			rtlglue_printf("L2TM Pool0 total buffer %d (priv num %d + shared num %d), %d Bytes per 1 Buffer\r\n\n",
				l2qmeq_cfg0.buff_num * 1024, l2qmeq_cfg0.prvt_buff_num * 16, l2qmeq_cfg0.buff_num * 1024 - l2qmeq_cfg0.prvt_buff_num * 16, L2TM_PAGE_SIZE);
		}

		ret = aal_l2_qm_eq_cfg_get(G3_DEF_DEVID, 1, &l2qmeq_cfg1);
		if(l2qmeq_cfg1.eq_ena == FALSE) {
			rtlglue_printf("L2TM Pool1 is Disabled!\r\n\n");
		}else {

			rtlglue_printf("L2TM Pool1 total buffer %d (priv cnt %d + shared cnt %d), %d Bytes per 1 Buffer\r\n\n",
				l2qmeq_cfg1.buff_num * 1024, l2qmeq_cfg1.prvt_buff_num * 16, l2qmeq_cfg1.buff_num * 1024 - l2qmeq_cfg1.prvt_buff_num * 16, L2TM_PAGE_SIZE);
		}

		rtlglue_printf("%-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s\r\n",
			"L3TE-Port", "EQ_Profile", "Pool0_ID", "Pool1_ID", "Pool0_Num", "Pool0_Size", "Pool1_Num", "Pool1_Size");

		for (i = 0; i <= CA_AAL_MAX_PORT_ID; i++) {

			if(i < CA_AAL_L3_TE_NI_PORT_OFFSET)
				portidx = i + CA_AAL_L3_TE_NI_PORT_OFFSET;
			else
				portidx = i - CA_AAL_L3_TE_NI_PORT_OFFSET;

			reg_off = QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * portidx);
			dest_port_eq_cfg.wrd = rtk_ne_reg_read(reg_off);

			reg_off = QM_QM_EQ_PROFILE0 + (QM_QM_EQ_PROFILE0_STRIDE * dest_port_eq_cfg.bf.profile_sel);
			eq_profile.wrd = rtk_ne_reg_read(reg_off);

			reg_off = QM_QM_CFG1_EQ0 + (QM_QM_CFG1_EQ0_STRIDE * eq_profile.bf.eqp0);
			cfg1_eq0.wrd = rtk_ne_reg_read(reg_off);

			reg_off = QM_QM_CFG1_EQ0 + (QM_QM_CFG1_EQ0_STRIDE * eq_profile.bf.eqp1);
			cfg1_eq1.wrd = rtk_ne_reg_read(reg_off);

			reg_off = QM_QM_CFG2_EQ0 + (QM_QM_CFG2_EQ0_STRIDE * eq_profile.bf.eqp0);
			cfg2_eq0.wrd = rtk_ne_reg_read(reg_off);

			reg_off = QM_QM_CFG2_EQ0 + (QM_QM_CFG2_EQ0_STRIDE * eq_profile.bf.eqp1);
			cfg2_eq1.wrd = rtk_ne_reg_read(reg_off);

				rtlglue_printf("%-4u%-3s(%u)  %-10u  %-10u  %-10u  %-10u  %-10u  %-10u  %-10u\r\n",
					portidx,  portidx<CA_AAL_L3_TE_NI_PORT_OFFSET?"cpu":"ni", portidx%8, dest_port_eq_cfg.bf.profile_sel, eq_profile.bf.eqp0, eq_profile.bf.eqp1,
					cfg1_eq0.bf.total_buffer_num, 64<<cfg2_eq0.bf.buffer_size, cfg1_eq1.bf.total_buffer_num, 64<<cfg2_eq1.bf.buffer_size);

		}



		rtlglue_printf("-----------------------------------------------------------------------------------\r\n");
	}


	return count;
}
#endif


rtk_rg_asic_hsb_entry_t rawL34Hsb;
rtk_rg_asic_hsa_entry_t rawL34Hsa;

void dump_l34hsb_display(rtk_rg_asic_hsb_entry_t *l34hsbData)
{
	rtlglue_printf("spa: %d extspa: %d ponIdx: %d l2payloadLen: %d diff: %d\n",
		l34hsbData->SPA,
		l34hsbData->SPA_EXT,
		l34hsbData->STM_IDX,
		l34hsbData->L2_LEN, l34hsbData->L2_LEN_DIFF);
	rtlglue_printf("gmacChk: %d ethType: 0x%x\n",
		l34hsbData->GMAC_CHK,
		l34hsbData->ETH_TYPE);
	rtlglue_printf("daIdx: %d saIdx: %d dual: %d dualErr: %d gre: %d l2tp: %d\n",
		l34hsbData->DA_IDX,
		l34hsbData->SA_IDX,
		l34hsbData->DUAL_HDR,
		l34hsbData->DUAL_FAIL,
		l34hsbData->GRE,
		l34hsbData->L2TP);
	rtlglue_printf("stagif: %d svid: %d ",
		l34hsbData->STAG_IF,
		l34hsbData->SVLAN_ID);
	rtlglue_printf("ctagif: %d cvid: %d\n",
		l34hsbData->CTAG_IF,
		l34hsbData->CVLAN_ID);
	rtlglue_printf("pppoeif: %d pppoesid: %d\n",
		l34hsbData->PPPOE_IF,
		l34hsbData->PPPOE_SID);

	rtlglue_printf("ip: %d v6: %d tos: 0x%x ttl: %d option: %d ipmf: %d l3cs: %d\n",
		l34hsbData->IP,
		l34hsbData->IPV4_6,
		l34hsbData->TOS,
		l34hsbData->TTL_ST,
		l34hsbData->IP_OPTION,
		l34hsbData->IPMF,
		l34hsbData->L3_CSOK);
	rtlglue_printf("sipv4: %s ", diag_util_inet_ntoa(l34hsbData->SIP_V4));
	rtlglue_printf("dipv4: %s\n", diag_util_inet_ntoa(l34hsbData->DIP_V4));
	rtlglue_printf("siphash: 0x%x ", l34hsbData->SIP_HSH);
	rtlglue_printf("diphash: 0x%x\n", l34hsbData->DIP_HSH);

	rtlglue_printf("l4: %d tcp: %d l4cs: %d sport: %d dport: %d\n",
		l34hsbData->L4_TYPE,
		l34hsbData->L4_PTC,
		l34hsbData->L4_CSOK,
		l34hsbData->SPORT,
		l34hsbData->DPORT);
	if(l34hsbData->L4_TYPE && l34hsbData->L4_PTC)
		rtlglue_printf("tcp flag ack: %d syn: %d rst: %d fin: %d\n", (l34hsbData->TCP_FLAG>>3)&0x1, (l34hsbData->TCP_FLAG>>2)&0x1, (l34hsbData->TCP_FLAG>>1)&0x1, l34hsbData->TCP_FLAG&0x1);
	if(l34hsbData->L4_TYPE && (0==l34hsbData->L4_PTC))
		rtlglue_printf("udp nocs: %d\n", l34hsbData->UDP_NOCS);
	if(l34hsbData->GRE)
		rtlglue_printf("GRE call id: 0x%x, seq: 0x%x\n", l34hsbData->L2TP_SESSION, l34hsbData->GRE_SEQ_NUM);
	if(l34hsbData->L2TP)
		rtlglue_printf("L2TP session id: 0x%x tunnel id: 0x%x\n", l34hsbData->L2TP_SESSION, l34hsbData->L2TP_ID);

//	if(l34hsbData->DUAL_HDR)
	{
		rtlglue_printf("[outer info] (dual: %d)\n", l34hsbData->DUAL_HDR);
		rtlglue_printf("v6: %d tos: 0x%x mf: %d ttl: %d option: %d l3cs: %d \n",
			l34hsbData->IPV6_OUT,
			l34hsbData->OUT_TOS,
			l34hsbData->OUT_IPMF,
			l34hsbData->OUT_TTL_ST,
			l34hsbData->OUT_IP_OPTION,
			l34hsbData->OUT_L3_CSOK);
		rtlglue_printf("l4: %d tcp: %d sport: %d dport: %d l4cs: %d\n",
			l34hsbData->OUT_L4_TYPE,
			l34hsbData->OUT_L4_PTC,
			l34hsbData->OUT_SPORT,
			l34hsbData->OUT_DPORT,
			l34hsbData->OUT_L4_CSOK);
	}

}

void dump_l34hsa_display(rtk_rg_asic_hsa_entry_t *l34hsaData)
{

	rtlglue_printf("act: %s", (l34hsaData->HSA_ACT == FB_ACTION_FORWARD)?"fwd":((l34hsaData->HSA_ACT == FB_ACTION_TRAP2CPU)?"trap":((l34hsaData->HSA_ACT == FB_ACTION_DROP)?"drop":"UNKNOWN")));

	if(l34hsaData->HSA_ACT != FB_ACTION_FORWARD)
	{
		rtlglue_printf("\t rsn: %d\n", l34hsaData->HSA_RSN);

		rtlglue_printf("prien: %d pri: %d\n", l34hsaData->HSA_PRI_EN, l34hsaData->HSA_PRI);
		rtlglue_printf("fbi: %d hashidx: %d\n", l34hsaData->HSA_HID_VLD, l34hsaData->HSA_HID);

	}else if(l34hsaData->HSA_ACT == FB_ACTION_FORWARD)
	{
		rtlglue_printf("\t routing: %d\n", l34hsaData->S1_P5);

		rtlglue_printf("dmact: %d dmacidx: %d\n", l34hsaData->HSA_DMAC_T, l34hsaData->HSA_DMAC_IDX);
		rtlglue_printf("streamact: %d streamidx: %d\n", l34hsaData->HSA_STREAM_ACT, l34hsaData->HSA_STREAM_IDX);
		rtlglue_printf("lutlookup: %d igrnetif: %d\n", l34hsaData->HSA_UC_LUT_LUP, l34hsaData->HSA_I_IF_IDX);
		rtlglue_printf("dual: %d\n", l34hsaData->HSA_HIT_DUAL);
		rtlglue_printf("flowmibact: %d flowmibidx: %d\n", l34hsaData->HSA_FLOW_COUNTER_ACT, l34hsaData->HSA_FLOW_COUNTER_IDX);

		rtlglue_printf("s1 info\n");
		rtlglue_printf("smact: %d extag: %d\n", l34hsaData->S1_SMAC_T, l34hsaData->S1_EX_TAG_IDX);
		rtlglue_printf("pmask: 0x%x extpmask: 0x%x\n", l34hsaData->S1_PMASK, l34hsaData->S1_EXTP_MASK);
		rtlglue_printf("pppact: %d pppsid: %d egrnetif: %d\n", l34hsaData->S1_PP_ACT, l34hsaData->S1_PP_SID, l34hsaData->S1_O_IF_IDX);
		rtlglue_printf("priact: %d pri: %d\n", l34hsaData->S1_USER_PRI_ACT, l34hsaData->S1_USER_PRI);
		rtlglue_printf("dscpact: %d dscp: %d\n", l34hsaData->S1_DSCP_ACT, l34hsaData->S1_DSCP);
		rtlglue_printf("stagact: %d svidact: %d spriact: %d\n", l34hsaData->S1_STAG_ACT, l34hsaData->S1_SVID_ACT, l34hsaData->S1_SPRI_ACT);
		rtlglue_printf("svidegract: %d svid: %d spri: %d\n", l34hsaData->S1_EGS_SVID_ACT, l34hsaData->S1_SVID, l34hsaData->S1_SPRI);
		rtlglue_printf("ctagact: %d cvidact: %d cpriact: %d\n", l34hsaData->S1_CTAG_ACT, l34hsaData->S1_CVID_ACT, l34hsaData->S1_CPRI_ACT);
		rtlglue_printf("cvidegract: %d cvid: %d cpri: %d\n", l34hsaData->S1_EGS_CVID_ACT, l34hsaData->S1_CVID, l34hsaData->S1_CPRI);
		rtlglue_printf("vid2s: %d vid2c: %d\n", l34hsaData->S1_VID2S_ACT, l34hsaData->S1_VID2C_ACT);

		if(l34hsaData->S1_P5){
			rtlglue_printf("l34 info\n");
			rtlglue_printf("l3cs: 0x%x l4cs: 0x%x\n", l34hsaData->S1_L3_CS, l34hsaData->S1_L4_CS);
			rtlglue_printf("l4act: %d l4dir: %d\n", l34hsaData->S1_L4_ACT, l34hsaData->S1_L4_DIR);
			rtlglue_printf("transip: %s transport:%d\n", diag_util_inet_ntoa(l34hsaData->S1_IP), l34hsaData->S1_PORT);
		}
#if 0 		// show all hsa for verification
		if(l34hsaData->S2_ACT)
#else
		else
#endif
		{
			rtlglue_printf("s2 info (act: %d)\n", l34hsaData->S2_ACT);
			rtlglue_printf("smact: %d\n", l34hsaData->S2_SMAC_T);
			rtlglue_printf("pmask: 0x%x extpmask: 0x%x\n", l34hsaData->S2_PMASK, l34hsaData->S2_EXTP_MASK);
			rtlglue_printf("pppact: %d pppsid: %d egrnetif: %d\n", l34hsaData->S2_PP_ACT, l34hsaData->S2_PP_SID, l34hsaData->S2_O_IF_IDX);
			rtlglue_printf("priact: %d pri: %d\n", l34hsaData->S2_USER_PRI_ACT, l34hsaData->S2_USER_PRI);
			rtlglue_printf("dscpact: %d dscp: %d\n", l34hsaData->S2_DSCP_ACT, l34hsaData->S2_DSCP);
			rtlglue_printf("stagact: %d svidact: %d spriact: %d\n", l34hsaData->S2_STAG_ACT, l34hsaData->S2_SVID_ACT, l34hsaData->S2_SPRI_ACT);
			rtlglue_printf("svidegract: %d svid: %d spri: %d\n", l34hsaData->S2_EGS_SVID_ACT, l34hsaData->S2_SVID, l34hsaData->S2_SPRI);
			rtlglue_printf("ctagact: %d cvidact: %d cpriact: %d\n", l34hsaData->S2_CTAG_ACT, l34hsaData->S2_CVID_ACT, l34hsaData->S2_CPRI_ACT);
			rtlglue_printf("cvidegract: %d cvid: %d cpri: %d\n", l34hsaData->S2_EGS_CVID_ACT, l34hsaData->S2_CVID, l34hsaData->S2_CPRI);
			rtlglue_printf("vid2s: %d vid2c: %d\n", l34hsaData->S2_VID2S_ACT, l34hsaData->S2_VID2C_ACT);
		}
	}
}

int32 dump_l34hs(struct seq_file *s, void *v)
{
	memset((void*)&rawL34Hsb,0,sizeof(rawL34Hsb));
	memset((void*)&rawL34Hsa,0,sizeof(rawL34Hsa));

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#if !defined(CONFIG_RG_G3_SERIES)
	ASSERT_EQ(rtk_rg_asic_hsbData_get(&rawL34Hsb), RT_ERR_RG_OK);
	ASSERT_EQ(rtk_rg_asic_hsaData_get(&rawL34Hsa), RT_ERR_RG_OK);
#endif
#endif

	PROC_PRINTF("---- "COLOR_Y "[L34HSB:]" COLOR_NM "------------------------------------\n");
	dump_l34hsb_display(&rawL34Hsb);
	PROC_PRINTF("----------------------------------------------\n");
	PROC_PRINTF("---- "COLOR_Y "[L34HSA:]" COLOR_NM "------------------------------------\n");
	dump_l34hsa_display(&rawL34Hsa);
	PROC_PRINTF("----------------------------------------------\n");

	return SUCCESS;
}

int32 dump_camtag_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0;
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_asic_camTag_entry_t camTag;
	bzero(&camTag, sizeof(camTag));

	if(fbMode == FB_MODE_4K)
	{
		rtlglue_printf("Not Support in 4K mode.\n");
		return SUCCESS;
	}

	PROC_PRINTF(">>ASIC CamTag Table:\n");
	for(idx = 0; idx < FLOWBASED_TABLESIZE_CAMTAG; idx++)
	{
		rtk_rg_asic_camTagTable_get(idx, &camTag);
		if(camTag.valid)
			PROC_PRINTF("[%d] hashidx:%d\tlck:%d\n", idx, camTag.hsahIdx, camTag.lock);
	}
	PROC_PRINTF("----------------------------------------------\n");
#endif

	return retval;
}

int32 dump_flowtag_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0;
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_asic_flowTag_entry_t flowTag;
	bzero(&flowTag, sizeof(flowTag));

	if(fbMode == FB_MODE_4K)
	{
		PROC_PRINTF("Not Support in 4K mode.\n");
		return SUCCESS;
	}

	PROC_PRINTF(">>ASIC FlowTag Table:\n");
	for(idx = 0; idx < FLOWBASED_TABLESIZE_FLOWTAG; idx++)
	{
		{
			rtk_rg_asic_flowTagTable_get(idx, &flowTag);
			if(flowTag.TTL!=0)
				PROC_PRINTF("[%d] msb:%d\tttl:%d\n", idx, flowTag.hashIdxMsb, flowTag.TTL);
		}
	}
	PROC_PRINTF("----------------------------------------------\n");
#endif
	return retval;
}

int32 dump_flow_p1Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path1_entry_t *p1Data = (rtk_rg_asic_path1_entry_t *)pFlowData;
	if(p1Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P1] valid: %d --\n", flowIdx, p1Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("intf: %d spachk:%d\n", p1Data->in_intf_idx, p1Data->in_spa_check);
	rtlglue_printf("dscpchk: %d tos: %d\n", p1Data->in_tos_check, p1Data->in_tos);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d sidchk: %d\n", p1Data->in_stagif, p1Data->in_ctagif, p1Data->in_cvlan_pri, p1Data->in_pppoeif, p1Data->in_pppoe_sid_check);
	rtlglue_printf("smacidx: %d dmacidx: %d inproto: %d\n", p1Data->in_smac_lut_idx, p1Data->in_dmac_lut_idx, p1Data->in_protocol);
	rtlglue_printf("svlan: %d cvlan: %d spa: %d extspa: %d\n", p1Data->in_svlan_id, p1Data->in_cvlan_id, p1Data->in_spa, p1Data->in_ext_spa);
	rtlglue_printf("pppoesid: %d\n\n", p1Data->in_pppoe_sid);

	rtlglue_printf(">>egress action\n");
	rtlglue_printf("streamact: %d streamidx: %d\n", p1Data->in_out_stream_idx_check_act, p1Data->in_out_stream_idx);
	rtlglue_printf("mtact: %d mtidx: %d exttagidx: %d\n", p1Data->out_share_meter_act, p1Data->out_share_meter_idx, p1Data->out_extra_tag_index);
	rtlglue_printf("intf: %d : dmacidx: %d dmactrans: %d\n", p1Data->out_intf_idx, p1Data->out_dmac_idx, p1Data->out_dmac_trans);
	rtlglue_printf("stagfmt: %d svidfmtact: %d sprifmtact: %d\n", p1Data->out_stag_format_act, p1Data->out_svid_format_act, p1Data->out_spri_format_act);
	rtlglue_printf("egsvidact: %d svid: %d spri: %d\n", p1Data->out_egress_svid_act, p1Data->out_svlan_id, p1Data->out_spri);
	rtlglue_printf("ctagfmt: %d cvidfmtact: %d cprifmtact: %d\n", p1Data->out_ctag_format_act, p1Data->out_cvid_format_act, p1Data->out_cpri_format_act);
	rtlglue_printf("egcvidact: %d cvid: %d cpri: %d\n", p1Data->out_egress_cvid_act, p1Data->out_cvlan_id, p1Data->out_cpri);
	rtlglue_printf("egport2vid:%s\n", (p1Data->out_egress_port_to_vid_act==1)?"sp2c":((p1Data->out_egress_port_to_vid_act==2)?"sp2s":((p1Data->out_egress_port_to_vid_act==3)?"cp2c":"none")));
	rtlglue_printf("usrpriact: %d usrpri: %d\n", p1Data->out_user_pri_act, p1Data->out_user_priority);
	rtlglue_printf("dscpact: %d dscp: %d\n", p1Data->out_dscp_act, p1Data->out_dscp);
	rtlglue_printf("pmask: 0x%x extpmaskidx: %d\n", p1Data->out_portmask, p1Data->out_ext_portmask_idx);
	rtlglue_printf("flowmibact: %d flowmibidx: %d\n", p1Data->out_flow_counter_act, p1Data->out_flow_counter_idx);
	rtlglue_printf("drop: %d multiact: %d uclut: %d smactrans: %d\n\n", p1Data->out_drop, p1Data->out_multiple_act, p1Data->out_uc_lut_lookup, p1Data->out_smac_trans);

	return SUCCESS;
}
int32 dump_flow_p2Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path2_entry_t *p2Data = (rtk_rg_asic_path2_entry_t *)pFlowData;
	if(p2Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P2] valid: %d --\n", flowIdx, p2Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("spachk:%d\n", p2Data->in_spa_check);
	rtlglue_printf("dscpchk: %d tos: %d\n", p2Data->in_tos_check, p2Data->in_tos);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d sidchk: %d\n", p2Data->in_stagif, p2Data->in_ctagif, p2Data->in_cvlan_pri, p2Data->in_pppoeif, p2Data->in_pppoe_sid_check);
	rtlglue_printf("smacidx: %d dmacidx: %d inproto: %d\n", p2Data->in_smac_lut_idx, p2Data->in_dmac_lut_idx, p2Data->in_protocol);
	rtlglue_printf("svlan: %d cvlan: %d spa: %d extspa: %d\n", p2Data->in_svlan_id, p2Data->in_cvlan_id, p2Data->in_spa, p2Data->in_ext_spa);
	rtlglue_printf("pppoesid: %d\n", p2Data->in_pppoe_sid);
	rtlglue_printf("streamchk: %d streamidx: %d\n\n", p2Data->in_stream_idx_check, p2Data->in_stream_idx);

	rtlglue_printf(">>egress action\n");
	rtlglue_printf("intf: %d\n", p2Data->out_intf_idx);
	rtlglue_printf("stagfmt: %d svidfmtact: %d sprifmtact: %d\n", p2Data->out_stag_format_act, p2Data->out_svid_format_act, p2Data->out_spri_format_act);
	rtlglue_printf("egsvidact: %d svid: %d spri: %d\n", p2Data->out_egress_svid_act, p2Data->out_svlan_id, p2Data->out_spri);
	rtlglue_printf("ctagfmt: %d cvidfmtact: %d cprifmtact: %d\n", p2Data->out_ctag_format_act, p2Data->out_cvid_format_act, p2Data->out_cpri_format_act);
	rtlglue_printf("egcvidact: %d cvid: %d cpri: %d\n", p2Data->out_egress_cvid_act, p2Data->out_cvlan_id, p2Data->out_cpri);
	rtlglue_printf("egport2vid:%s\n", (p2Data->out_egress_port_to_vid_act==1)?"sp2c":((p2Data->out_egress_port_to_vid_act==2)?"sp2s":((p2Data->out_egress_port_to_vid_act==3)?"cp2c":"none")));
	rtlglue_printf("usrpriact: %d usrpri: %d\n", p2Data->out_user_pri_act, p2Data->out_user_priority);
	rtlglue_printf("dscpact: %d dscp: %d\n", p2Data->out_dscp_act, p2Data->out_dscp);
	rtlglue_printf("pmask: 0x%x extpmaskidx: %d\n", p2Data->out_portmask, p2Data->out_ext_portmask_idx);
	rtlglue_printf("smactrans: %d\n\n", p2Data->out_smac_trans);

	return SUCCESS;
}
int32 dump_flow_p3Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path3_entry_t *p3Data = (rtk_rg_asic_path3_entry_t *)pFlowData;
	if(p3Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P3] valid: %d --\n", flowIdx, p3Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("intf: %d v6:%d l4proto: %d\n", p3Data->in_intf_idx, p3Data->in_ipv4_or_ipv6, p3Data->in_l4proto);
	rtlglue_printf("dscpchk: %d tos: %d\n", p3Data->in_tos_check, p3Data->in_tos);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d sidchk: %d\n", p3Data->in_stagif, p3Data->in_ctagif, p3Data->in_cvlan_pri, p3Data->in_pppoeif, p3Data->in_pppoe_sid_check);
	if(!p3Data->in_ipv4_or_ipv6){
	rtlglue_printf("srcv4ip: %d.%d.%d.%d dstv4ip: %d.%d.%d.%d\n",
		(p3Data->in_src_ipv4_addr>>24)&0xff, (p3Data->in_src_ipv4_addr>>16)&0xff, (p3Data->in_src_ipv4_addr>>8)&0xff, p3Data->in_src_ipv4_addr&0xff,
		(p3Data->in_dst_ipv4_addr>>24)&0xff, (p3Data->in_dst_ipv4_addr>>16)&0xff, (p3Data->in_dst_ipv4_addr>>8)&0xff, p3Data->in_dst_ipv4_addr&0xff);
	}else{
	rtlglue_printf("srcv6ip: 0x%x dstv6ip: 0x%x\n", p3Data->in_src_ipv6_addr_hash, p3Data->in_dst_ipv6_addr_hash);
	}
	rtlglue_printf("srcport: %d dstport: %d\n\n", p3Data->in_l4_src_port, p3Data->in_l4_dst_port);

	rtlglue_printf(">>egress action\n");
	rtlglue_printf("streamact: %d streamidx: %d\n", p3Data->out_stream_idx_act, p3Data->out_stream_idx);
	rtlglue_printf("mtact: %d mtidx: %d exttagidx: %d\n", p3Data->out_share_meter_act, p3Data->out_share_meter_idx, p3Data->out_extra_tag_index);
	rtlglue_printf("intf: %d : dmacidx: %d dmactrans: %d\n", p3Data->out_intf_idx, p3Data->out_dmac_idx, p3Data->out_dmac_trans);
	rtlglue_printf("stagfmt: %d svidfmtact: %d sprifmtact: %d\n", p3Data->out_stag_format_act, p3Data->out_svid_format_act, p3Data->out_spri_format_act);
	rtlglue_printf("egsvidact: %d svid: %d spri: %d\n", p3Data->out_egress_svid_act, p3Data->out_svlan_id, p3Data->out_spri);
	rtlglue_printf("ctagfmt: %d cvidfmtact: %d cprifmtact: %d\n", p3Data->out_ctag_format_act, p3Data->out_cvid_format_act, p3Data->out_cpri_format_act);
	rtlglue_printf("egcvidact: %d cvid: %d cpri: %d\n", p3Data->out_egress_cvid_act, p3Data->out_cvlan_id, p3Data->out_cpri);
	rtlglue_printf("egport2vid:%s\n", (p3Data->out_egress_port_to_vid_act==1)?"sp2c":((p3Data->out_egress_port_to_vid_act==2)?"sp2s":((p3Data->out_egress_port_to_vid_act==3)?"cp2c":"none")));
	rtlglue_printf("usrpriact: %d usrpri: %d\n", p3Data->out_user_pri_act, p3Data->out_user_priority);
	rtlglue_printf("dscpact: %d dscp: %d\n", p3Data->out_dscp_act, p3Data->out_dscp);
	rtlglue_printf("pmask: 0x%x extpmaskidx: %d\n", p3Data->out_portmask, p3Data->out_ext_portmask_idx);
	rtlglue_printf("flowmibact: %d flowmibidx: %d\n", p3Data->out_flow_counter_act, p3Data->out_flow_counter_idx);
	rtlglue_printf("drop: %d multiact: %d uclut: %d smactrans: %d\n\n", p3Data->out_drop, p3Data->out_multiple_act, p3Data->out_uc_lut_lookup, p3Data->out_smac_trans);

	return SUCCESS;
}
int32 dump_flow_p4Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path4_entry_t *p4Data = (rtk_rg_asic_path4_entry_t *)pFlowData;
	if(p4Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P4] valid: %d --\n", flowIdx, p4Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("intf: %d v6:%d l4proto: %d\n", p4Data->in_intf_idx, p4Data->in_ipv4_or_ipv6, p4Data->in_l4proto);
	rtlglue_printf("dscpchk: %d tos: %d\n", p4Data->in_tos_check, p4Data->in_tos);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d sidchk: %d\n", p4Data->in_stagif, p4Data->in_ctagif, p4Data->in_cvlan_pri, p4Data->in_pppoeif, p4Data->in_pppoe_sid_check);
	if(!p4Data->in_ipv4_or_ipv6){
	rtlglue_printf("srcv4ip: %d.%d.%d.%d dstv4ip: %d.%d.%d.%d\n",
		(p4Data->in_src_ipv4_addr>>24)&0xff, (p4Data->in_src_ipv4_addr>>16)&0xff, (p4Data->in_src_ipv4_addr>>8)&0xff, p4Data->in_src_ipv4_addr&0xff,
		(p4Data->in_dst_ipv4_addr>>24)&0xff, (p4Data->in_dst_ipv4_addr>>16)&0xff, (p4Data->in_dst_ipv4_addr>>8)&0xff, p4Data->in_dst_ipv4_addr&0xff);
	}else{
	rtlglue_printf("srcv6ip: 0x%x dstv6ip: 0x%x\n", p4Data->in_src_ipv6_addr_hash, p4Data->in_dst_ipv6_addr_hash);
	}
	rtlglue_printf("srcport: %d dstport: %d\n\n", p4Data->in_l4_src_port, p4Data->in_l4_dst_port);

	rtlglue_printf(">>egress action\n");
	rtlglue_printf("intf: %d\n", p4Data->out_intf_idx);
	rtlglue_printf("stagfmt: %d svidfmtact: %d sprifmtact: %d\n", p4Data->out_stag_format_act, p4Data->out_svid_format_act, p4Data->out_spri_format_act);
	rtlglue_printf("egsvidact: %d svid: %d spri: %d\n", p4Data->out_egress_svid_act, p4Data->out_svlan_id, p4Data->out_spri);
	rtlglue_printf("ctagfmt: %d cvidfmtact: %d cprifmtact: %d\n", p4Data->out_ctag_format_act, p4Data->out_cvid_format_act, p4Data->out_cpri_format_act);
	rtlglue_printf("egcvidact: %d cvid: %d cpri: %d\n", p4Data->out_egress_cvid_act, p4Data->out_cvlan_id, p4Data->out_cpri);
	rtlglue_printf("egport2vid:%s\n", (p4Data->out_egress_port_to_vid_act==1)?"sp2c":((p4Data->out_egress_port_to_vid_act==2)?"sp2s":((p4Data->out_egress_port_to_vid_act==3)?"cp2c":"none")));
	rtlglue_printf("usrpriact: %d usrpri: %d\n", p4Data->out_user_pri_act, p4Data->out_user_priority);
	rtlglue_printf("dscpact: %d dscp: %d\n", p4Data->out_dscp_act, p4Data->out_dscp);
	rtlglue_printf("pmask: 0x%x extpmaskidx: %d\n", p4Data->out_portmask, p4Data->out_ext_portmask_idx);
	rtlglue_printf("smactrans: %d\n\n", p4Data->out_smac_trans);

	return SUCCESS;
}
int32 dump_flow_p5Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path5_entry_t *p5Data = (rtk_rg_asic_path5_entry_t *)pFlowData;
	if(p5Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P5] valid: %d --\n", flowIdx, p5Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("intf: %d v6:%d l4proto: %d\n", p5Data->in_intf_idx, p5Data->in_ipv4_or_ipv6, p5Data->in_l4proto);
	rtlglue_printf("dscpchk: %d tos: %d\n", p5Data->in_tos_check, p5Data->in_tos);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d\n", p5Data->in_stagif, p5Data->in_ctagif, p5Data->in_cvlan_pri, p5Data->in_pppoeif);
	if(!p5Data->in_ipv4_or_ipv6){

		if(p5Data->out_l4_act && !p5Data->out_l4_direction)
		{
			rtlglue_printf("srcv4ip: %d.%d.%d.%d dstv4ip: %d.%d.%d.%d\n",
				(p5Data->in_src_ipv4_addr>>24)&0xff, (p5Data->in_src_ipv4_addr>>16)&0xff, (p5Data->in_src_ipv4_addr>>8)&0xff, p5Data->in_src_ipv4_addr&0xff,
				(rg_db.netif[p5Data->in_intf_idx].rtk_netif.ipAddr>>24)&0xff, (rg_db.netif[p5Data->in_intf_idx].rtk_netif.ipAddr>>16)&0xff, (rg_db.netif[p5Data->in_intf_idx].rtk_netif.ipAddr>>8)&0xff, rg_db.netif[p5Data->in_intf_idx].rtk_netif.ipAddr&0xff);
		}
		else
		{
			rtlglue_printf("srcv4ip: %d.%d.%d.%d dstv4ip: %d.%d.%d.%d\n",
				(p5Data->in_src_ipv4_addr>>24)&0xff, (p5Data->in_src_ipv4_addr>>16)&0xff, (p5Data->in_src_ipv4_addr>>8)&0xff, p5Data->in_src_ipv4_addr&0xff,
				(p5Data->in_dst_ipv4_addr>>24)&0xff, (p5Data->in_dst_ipv4_addr>>16)&0xff, (p5Data->in_dst_ipv4_addr>>8)&0xff, p5Data->in_dst_ipv4_addr&0xff);
		}


	}else{
		rtlglue_printf("srcv6ip: 0x%x dstv6ip: 0x%x\n", p5Data->in_src_ipv6_addr_hash, p5Data->in_dst_ipv6_addr_hash);
	}
	rtlglue_printf("srcport: %d dstport: %d\n\n", p5Data->in_l4_src_port, p5Data->in_l4_dst_port);

	rtlglue_printf(">>egress action\n");
  	if(!p5Data->in_ipv4_or_ipv6 && p5Data->out_l4_act)
  	{
  		if(p5Data->out_l4_direction)
			rtlglue_printf("out_srcv4ip: %d.%d.%d.%d\n",
				(rg_db.netif[p5Data->out_intf_idx].rtk_netif.ipAddr>>24)&0xff, (rg_db.netif[p5Data->out_intf_idx].rtk_netif.ipAddr>>16)&0xff, (rg_db.netif[p5Data->out_intf_idx].rtk_netif.ipAddr>>8)&0xff, rg_db.netif[p5Data->out_intf_idx].rtk_netif.ipAddr&0xff);
		else
			rtlglue_printf("out_destv4ip: %d.%d.%d.%d\n",
				(p5Data->out_dst_ipv4_addr>>24)&0xff, (p5Data->out_dst_ipv4_addr>>16)&0xff, (p5Data->out_dst_ipv4_addr>>8)&0xff, p5Data->out_dst_ipv4_addr&0xff);
  	}
	rtlglue_printf("streamact: %d streamidx: %d\n", p5Data->out_stream_idx_act, p5Data->out_stream_idx);
	rtlglue_printf("mtact: %d mtidx: %d exttagidx: %d\n", p5Data->out_share_meter_act, p5Data->out_share_meter_idx, p5Data->out_extra_tag_index);
	rtlglue_printf("intf: %d dmacidx: %d\n", p5Data->out_intf_idx, p5Data->out_dmac_idx);
	rtlglue_printf("l4act: %d l4dir: %d(%s) l4port: %d\n", p5Data->out_l4_act, p5Data->out_l4_direction, p5Data->out_l4_direction?"outbound":"inbound", p5Data->out_l4_port);
	rtlglue_printf("stagfmt: %d svidfmtact: %d sprifmtact: %d\n", p5Data->out_stag_format_act, p5Data->out_svid_format_act, p5Data->out_spri_format_act);
	rtlglue_printf("egsvidact: %d svid: %d spri: %d\n", p5Data->out_egress_svid_act, p5Data->out_svlan_id, p5Data->out_spri);
	rtlglue_printf("ctagfmt: %d cvidfmtact: %d cprifmtact: %d\n", p5Data->out_ctag_format_act, p5Data->out_cvid_format_act, p5Data->out_cpri_format_act);
	rtlglue_printf("egcvidact: %d cvid: %d cpri: %d\n", p5Data->out_egress_cvid_act, p5Data->out_cvlan_id, p5Data->out_cpri);
	rtlglue_printf("egport2vid: %s\n", (p5Data->out_egress_port_to_vid_act==1)?"sp2c":((p5Data->out_egress_port_to_vid_act==2)?"sp2s":((p5Data->out_egress_port_to_vid_act==3)?"cp2c":"none")));
	rtlglue_printf("usrpriact: %d usrpri: %d\n", p5Data->out_user_pri_act, p5Data->out_user_priority);
	rtlglue_printf("dscpact: %d dscp: %d\n", p5Data->out_dscp_act, p5Data->out_dscp);
	rtlglue_printf("flowmibact: %d flowmibidx: %d\n", p5Data->out_flow_counter_act, p5Data->out_flow_counter_idx);
	rtlglue_printf("drop: %d\n\n", p5Data->out_drop);

	return SUCCESS;
}
int32 dump_flow_p6Rawdata(int32 flowIdx, void *pFlowData)
{
	rtk_rg_asic_path6_entry_t *p6Data = (rtk_rg_asic_path6_entry_t *)pFlowData;
	if(p6Data==NULL) return FAIL;

	rtlglue_printf(" -- Flow["COLOR_Y"%d"COLOR_NM"] [P6] valid: %d --\n", flowIdx, p6Data->valid);

	rtlglue_printf(">>ingress pattern\n");
	rtlglue_printf("intf: %d proto: %d\n", p6Data->in_intf_idx, p6Data->in_protocol);
	rtlglue_printf("stagif: %d ctagif: %d ctagpri: %d pppoeif: %d\n", p6Data->in_stagif, p6Data->in_ctagif, p6Data->in_cvlan_pri, p6Data->in_pppoeif);
	rtlglue_printf("pptpif: %d l2tpif: %d dslite: %d\n", p6Data->in_pptpif, p6Data->in_l2tpif, p6Data->in_dsliteif);
	if(p6Data->in_pptpif)
	{
		rtlglue_printf("gre callidchk: %d callid: 0x%x\n", p6Data->in_gre_call_id_check, p6Data->in_gre_call_id_check);
	}
	if(p6Data->in_l2tpif)
	{
		rtlglue_printf("l2tp tunnelidchk: %d sessionidchk: %d\n", p6Data->in_l2tp_tunnel_id_check, p6Data->in_l2tp_session_id_check);
		rtlglue_printf("l2tp tunnelid: 0x%x sessionid: 0x%x\n", p6Data->in_l2tp_tunnel_id, p6Data->in_l2tp_session_id);
	}
	rtlglue_printf("smachk: %d dmacchk: %d\n", p6Data->in_src_mac_check, p6Data->in_dst_mac_check);
	rtlglue_printf("pppoesidchk: %d sid: %d\n", p6Data->in_pppoe_sid_check, p6Data->in_pppoe_sid);
	rtlglue_printf("sipchk: %d dipchk: %d\n", p6Data->in_src_ip_check, p6Data->in_dst_ip_check);
	rtlglue_printf("sportchk: %d dportchk: %d\n", p6Data->in_l4_src_port_check, p6Data->in_l4_dst_port_check);
	rtlglue_printf("dscpchk: %d tos: %d\n", p6Data->in_tos_check, p6Data->in_tos);

	rtlglue_printf("smacidx: %d dmacidx: %d\n", p6Data->in_smac_lut_idx, p6Data->in_dmac_lut_idx);
	if(!p6Data->in_dsliteif){
	rtlglue_printf("srcv4ip: %d.%d.%d.%d dstv4ip: %d.%d.%d.%d\n",
		(p6Data->in_src_ipv4_addr>>24)&0xff, (p6Data->in_src_ipv4_addr>>16)&0xff, (p6Data->in_src_ipv4_addr>>8)&0xff, p6Data->in_src_ipv4_addr&0xff,
		(p6Data->in_dst_ipv4_addr>>24)&0xff, (p6Data->in_dst_ipv4_addr>>16)&0xff, (p6Data->in_dst_ipv4_addr>>8)&0xff, p6Data->in_dst_ipv4_addr&0xff);
	}else{
	rtlglue_printf("srcv6ip: 0x%x dstv6ip: 0x%x\n", p6Data->in_src_ipv6_addr_hash, p6Data->in_dst_ipv6_addr_hash);
	}
	rtlglue_printf("srcport: %d dstport: %d\n\n", p6Data->in_l4_src_port, p6Data->in_l4_dst_port);

	rtlglue_printf(">>egress action\n");
	rtlglue_printf("drop: %d\n\n", p6Data->out_drop);

	return SUCCESS;
}

int32 rtk_rg_asic_display_flow_sram_cam_table_idx(uint32 idx)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 maxTableSize=0;
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_asic_path1_entry_t *pP1Data = rtk_rg_malloc(sizeof(rtk_rg_asic_path1_entry_t));
	rtk_rg_asic_camTag_entry_t camTag;

	maxTableSize = (fbMode== FB_MODE_4K)?(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWTCAM):(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWCAM);

	//PROC_PRINTF(">>ASIC Flow (SRAM) Table:\n");
	//rtlglue_printf(">>ASIC Flow (SRAM) Table:\n");
	//for(idx=0; idx<maxTableSize; idx++)
	if (idx<maxTableSize)
	{
		// 4K mode, we check valid bit saved in rgprodb.
		// DDR mode, we check valid bit by reading flow entry saved in sram directly. (we didn't maintain validbit in DDR mode because sram entries are controlled by cache controller.)
		if( (fbMode!= FB_MODE_4K) || ((fbMode== FB_MODE_4K) && (_rtk_rg_flowEntryValidBit_get(idx) == TRUE)))
		{
			if((fbMode== FB_MODE_4K) || ((fbMode!= FB_MODE_4K) && (idx < FLOWBASED_TABLESIZE_FLOWSRAM)))
			{
				// Force to read SRAM flow entries
				retval = rtk_rg_asic_sramFlowEntry_get(idx, (void*)pP1Data);
			}else
			{
				// DRAM mode, read cam entries
				rtk_rg_asic_camTagTable_get(idx-FLOWBASED_TABLESIZE_FLOWSRAM, &camTag);
				if(!camTag.valid)
				{
					rtk_rg_free(pP1Data);
					return FAIL;
				}
				rtk_rg_asic_flowPath1_get(camTag.hsahIdx, pP1Data);
			}

			//if(!pP1Data->valid) continue;
			if(!pP1Data->valid)
			{
				rtk_rg_free(pP1Data);
				return FAIL;
			}
			if(pP1Data->in_path == 0)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p1Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p2Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 1)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p3Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p4Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 2)
			{
					dump_flow_p5Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 3)
			{
					dump_flow_p6Rawdata(idx, (void*)pP1Data);
			}
			if(fbMode!= FB_MODE_4K)
			{
				if(idx < FLOWBASED_TABLESIZE_FLOWSRAM)
				{
					// SRAM cached flow
					rtk_rg_asic_flowTag_entry_t flowTag;
					bzero(&flowTag, sizeof(flowTag));
					rtk_rg_asic_flowTagTable_get(idx, &flowTag);

					rtlglue_printf(">>flowtag - msb:%d\tttl:%d\n\n", flowTag.hashIdxMsb, flowTag.TTL);
				}else
				{
					// CAM cached flow
					rtk_rg_asic_camTag_entry_t camTag;
					rtk_rg_asic_camTagTable_get(idx-FLOWBASED_TABLESIZE_FLOWSRAM, &camTag);

					rtlglue_printf(">>camtag - entryIdx:%d\tlock:%d\n\n", camTag.hsahIdx, camTag.lock);
				}
			}
		}
	}
	//rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pP1Data);
#endif //!defined(CONFIG_RG_G3_SERIES)
	return retval;

}


int32 rtk_rg_asic_dump_flow_table_idx(uint32 idx)
{
	rtlglue_printf(">>ASIC Flow (SRAM) Table:\n");
	return rtk_rg_asic_display_flow_sram_cam_table_idx(idx);
}


int32 rtk_rg_asic_dump_flow_table_all(void)
{
	return dump_flow_table(NULL, NULL);
}


int32 dump_flow_sram_cam_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if defined(CONFIG_RG_G3_SERIES)
	PROC_PRINTF("Not support!\n");
#else
	uint32 idx=0, maxTableSize=0;
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	maxTableSize = (fbMode== FB_MODE_4K)?(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWTCAM):(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWCAM);
	PROC_PRINTF(">>ASIC Flow (SRAM) Table:\n");
	for(idx=0; idx<maxTableSize; idx++)
	{
		retval = rtk_rg_asic_display_flow_sram_cam_table_idx(idx);
	}
	PROC_PRINTF("----------------------------------------------\n");
#endif	// end CONFIG_RG_G3_SERIES
	return retval;
}

int32 dump_flow_sram_cam_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data)
{
#if defined(CONFIG_RG_G3_SERIES)
	rtlglue_printf("Not support!\n");
#else
	unsigned char tmpBuf[256] = {0};
	int len = (count > 255) ? 255 : count;

	uint32 idx=0, maxTableSize=0;
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_table_flow_t *pFlow=NULL;
	rtk_rg_asic_path1_entry_t *pP1Data = rtk_rg_malloc(sizeof(rtk_rg_asic_path1_entry_t));
	rtk_rg_asic_path3_entry_t *pP3Data = (rtk_rg_asic_path3_entry_t *)pP1Data;
	rtk_rg_asic_path5_entry_t *pP5Data = (rtk_rg_asic_path5_entry_t *)pP1Data;
	rtk_rg_asic_path6_entry_t *pP6Data = (rtk_rg_asic_path6_entry_t *)pP1Data;
	uint8 spa_check=0, smac_check=0, dmac_check=0, sip_check=0, dip_check=0, ip_check=0, sipV6_check=0, dipV6_check=0, ipV6_check=0, l4proto_check=0, sport_check=0, dport_check=0;
	rtk_rg_port_idx_t spa=0;
	rtk_mac_t smac, dmac;
	uint32 sip=0, dip=0, ip=0;
	rtk_ipv6_addr_t sipV6, dipV6, ipV6;
	uint32 sipV6_hash=0, dipV6_hash=0, ipV6_srcHash=0, ipV6_dstHash=0;
	uint16 l4proto=0, sport=0, dport=0;

	if (buffer && !COPY_FROM_USER(tmpBuf, buffer, len))
	{
		char *strptr,*split_str;
		tmpBuf[len] = '\0';
		strptr=tmpBuf;

		while(1)
		{
			split_str=strsep(&strptr," ");
			if(strptr==NULL) goto ERROR_PARAMETER;

			if(strcasecmp(split_str,"SPA")==0)
			{
				split_str=strsep(&strptr," ");
				spa=simple_strtol(split_str, NULL, 0);
				spa_check=1;
			}
			else if(strcasecmp(split_str,"DA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str, &dmac);
				dmac_check=1;
			}
			else if(strcasecmp(split_str,"SA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str,&smac);
				smac_check=1;
			}
			else if(strcasecmp(split_str,"SIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, sip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					sip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				sip_check=1;
			}
			else if(strcasecmp(split_str,"DIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, dip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					dip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				dip_check=1;
			}
			else if(strcasecmp(split_str,"IP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, ip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					ip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				ip_check=1;
			}
			else if(strcasecmp(split_str,"L4PROTO")==0)
			{
				split_str=strsep(&strptr," ");
				l4proto=simple_strtol(split_str, NULL, 16);
				l4proto_check=1;
			}

			else if(strcasecmp(split_str,"SPORT")==0)
			{
				split_str=strsep(&strptr," ");
				sport=simple_strtol(split_str, NULL, 0);
				sport_check=1;
			}
			else if(strcasecmp(split_str,"DPORT")==0)
			{
				split_str=strsep(&strptr," ");
				dport=simple_strtol(split_str, NULL, 0);
				dport_check=1;
			}
			else if(strcasecmp(split_str,"SIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str, -1, &(sipV6.ipv6_addr[0]), -1, NULL);
				sipV6_check=1;
				sipV6_hash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(sipV6.ipv6_addr);
			}
			else if(strcasecmp(split_str,"DIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(dipV6.ipv6_addr[0]),-1,NULL);
				dipV6_check=1;
				dipV6_hash = _rtk_rg_sw_flowHashIPv6DstAddr_get(dipV6.ipv6_addr);
			}
			else if(strcasecmp(split_str,"IP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(ipV6.ipv6_addr[0]),-1,NULL);
				ipV6_check=1;
				ipV6_srcHash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(ipV6.ipv6_addr);
				ipV6_dstHash = _rtk_rg_sw_flowHashIPv6DstAddr_get(ipV6.ipv6_addr);
			}
			else
			{
				goto ERROR_PARAMETER;
			}

			if (strptr==NULL) break;
		}
	}

	rtlglue_printf(">>ASIC Flow (SRAM) Table (filter by the following patterns):\n");
	if(spa_check) rtlglue_printf("SPA:%d\n", spa);
	if(dmac_check) rtlglue_printf("DA:%02x:%02x:%02x:%02x:%02x:%02x\n", dmac.octet[0], dmac.octet[1], dmac.octet[2], dmac.octet[3], dmac.octet[4], dmac.octet[5]);
	if(smac_check) rtlglue_printf("SA:%02x:%02x:%02x:%02x:%02x:%02x\n", smac.octet[0], smac.octet[1], smac.octet[2], smac.octet[3], smac.octet[4], smac.octet[5]);
	if(sip_check) rtlglue_printf("SIP:%d.%d.%d.%d\n", (sip>>24)&0xff, (sip>>16)&0xff, (sip>>8)&0xff, (sip)&0xff);
	if(dip_check) rtlglue_printf("DIP:%d.%d.%d.%d\n", (dip>>24)&0xff, (dip>>16)&0xff, (dip>>8)&0xff, (dip)&0xff);
	if(ip_check) rtlglue_printf("IP:%d.%d.%d.%d\n", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, (ip)&0xff);
	if(l4proto_check) rtlglue_printf("L4PROTO:0x%04x\n", l4proto);
	if(sport_check) rtlglue_printf("SPORT:%d\n", sport);
	if(dport_check) rtlglue_printf("SPORT:%d\n", dport);
	if(sipV6_check) rtlglue_printf("SIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x)\n",
						sipV6.ipv6_addr[0], sipV6.ipv6_addr[1], sipV6.ipv6_addr[2], sipV6.ipv6_addr[3],
						sipV6.ipv6_addr[4], sipV6.ipv6_addr[5], sipV6.ipv6_addr[6], sipV6.ipv6_addr[7],
						sipV6.ipv6_addr[8], sipV6.ipv6_addr[9], sipV6.ipv6_addr[10], sipV6.ipv6_addr[11],
						sipV6.ipv6_addr[12], sipV6.ipv6_addr[13], sipV6.ipv6_addr[14], sipV6.ipv6_addr[15], sipV6_hash);
	if(dipV6_check) rtlglue_printf("DIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (dstHash:0x%x)\n",
						dipV6.ipv6_addr[0], dipV6.ipv6_addr[1], dipV6.ipv6_addr[2], dipV6.ipv6_addr[3],
						dipV6.ipv6_addr[4], dipV6.ipv6_addr[5], dipV6.ipv6_addr[6], dipV6.ipv6_addr[7],
						dipV6.ipv6_addr[8], dipV6.ipv6_addr[9], dipV6.ipv6_addr[10], dipV6.ipv6_addr[11],
						dipV6.ipv6_addr[12], dipV6.ipv6_addr[13], dipV6.ipv6_addr[14], dipV6.ipv6_addr[15], dipV6_hash);
	if(ipV6_check) rtlglue_printf("IP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x, dstHash:0x%x)\n",
						ipV6.ipv6_addr[0], ipV6.ipv6_addr[1], ipV6.ipv6_addr[2], ipV6.ipv6_addr[3],
						ipV6.ipv6_addr[4], ipV6.ipv6_addr[5], ipV6.ipv6_addr[6], ipV6.ipv6_addr[7],
						ipV6.ipv6_addr[8], ipV6.ipv6_addr[9], ipV6.ipv6_addr[10], ipV6.ipv6_addr[11],
						ipV6.ipv6_addr[12], ipV6.ipv6_addr[13], ipV6.ipv6_addr[14], ipV6.ipv6_addr[15], ipV6_srcHash, ipV6_dstHash);

	rtlglue_printf("\n");
	maxTableSize = (fbMode== FB_MODE_4K)?(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWTCAM):(FLOWBASED_TABLESIZE_FLOWSRAM+FLOWBASED_TABLESIZE_FLOWCAM);
	for(idx=0; idx<maxTableSize; idx++)
	{
		// 4K mode, we check valid bit saved in rgprodb.
		// DDR mode, we check valid bit by reading flow entry saved in sram directly. (we didn't maintain validbit in DDR mode because sram entries are controlled by cache controller.)
		if( (fbMode!= FB_MODE_4K) || ((fbMode== FB_MODE_4K) && (_rtk_rg_flowEntryValidBit_get(idx) == TRUE)))
		{
			pFlow = &rg_db.flow[idx];
			//check spa
			if(spa_check && pFlow->ingressPort!=spa) continue;
			//check dmac
			if(dmac_check)
			{
				if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
					&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac, &dmac, sizeof(rtk_mac_t)))
					continue;
				if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
					&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2McEntry.mac, &dmac, sizeof(rtk_mac_t)))
					continue;
			}
			//check smac
			if(smac_check)
			{
				if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
					&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2UcEntry.mac, &smac, sizeof(rtk_mac_t)))
					continue;
				if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
					&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2McEntry.mac, &smac, sizeof(rtk_mac_t)))
					continue;
			}
			// Force to read SRAM flow entries
			ASSERT_EQ(rtk_rg_asic_sramFlowEntry_get(idx, (void*)pP1Data), RT_ERR_RG_OK);
			//if(!pP1Data->valid) continue;
			if(!pP1Data->valid)
			{
				WARNING("Hw flow[%d] is invalid", idx);
				rtk_rg_free(pP1Data);
				return count;
			}
			switch(pP1Data->in_path)
			{
				case FB_PATH_12:
					//skip ip/port/l4protocol checking
					if(sip_check || dip_check || ip_check || sipV6_check || dipV6_check || ipV6_check || l4proto_check || sport_check || dport_check)
						continue;
					break;
				case FB_PATH_34:
					if(pP3Data->in_ipv4_or_ipv6==0)	//ipv4
					{
						if(sipV6_check || dipV6_check || ipV6_check)
							continue;
						if(sip_check && pP3Data->in_src_ipv4_addr!=sip)
							continue;
						if(dip_check && pP3Data->in_dst_ipv4_addr!=dip)
							continue;
						if(ip_check && pP3Data->in_src_ipv4_addr!=ip && pP3Data->in_dst_ipv4_addr!=ip)
							continue;
					}
					else	//ipv6
					{
						if(sip_check || dip_check || ip_check)
							continue;
						if(sipV6_check && pP3Data->in_src_ipv6_addr_hash!=sipV6_hash)
							continue;
						if(dipV6_check && pP3Data->in_dst_ipv6_addr_hash!=dipV6_hash)
							continue;
						if(ipV6_check && pP3Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP3Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
							continue;
					}
					if(l4proto_check && ((pP3Data->in_l4proto==1 && l4proto!=0x6) || (pP3Data->in_l4proto==0 && l4proto!=0x11)))
						continue;
					if(sport_check && pP3Data->in_l4_src_port!=sport)
						continue;
					if(dport_check && pP3Data->in_l4_dst_port!=dport)
						continue;
					break;
				case FB_PATH_5:
					if(pP5Data->in_ipv4_or_ipv6==0)	//ipv4
					{
						uint32 checkDip = (pP5Data->out_l4_act==1 && pP5Data->out_l4_direction==0)?(rg_db.netif[pP5Data->in_intf_idx].rtk_netif.ipAddr):(pP5Data->in_dst_ipv4_addr);

						if(sipV6_check || dipV6_check || ipV6_check)
							continue;
						if(sip_check && pP5Data->in_src_ipv4_addr!=sip)
							continue;
						if(dip_check && checkDip!=dip)
							continue;
						if(ip_check && pP5Data->in_src_ipv4_addr!=ip && checkDip!=ip)
							continue;
					}
					else	//ipv6
					{
						if(sip_check || dip_check || ip_check)
							continue;
						if(sipV6_check && pP5Data->in_src_ipv6_addr_hash!=sipV6_hash)
							continue;
						if(dipV6_check && pP5Data->in_dst_ipv6_addr_hash!=dipV6_hash)
							continue;
						if(ipV6_check && pP5Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP5Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
							continue;
					}
					if(l4proto_check && ((pP5Data->in_l4proto==1 && l4proto!=0x6) || (pP5Data->in_l4proto==0 && l4proto!=0x11)))
						continue;
					if(sport_check && pP5Data->in_l4_src_port!=sport)
						continue;
					if(dport_check && pP5Data->in_l4_dst_port!=dport)
						continue;
					break;
				case FB_PATH_6:
					if(pP6Data->in_dsliteif==0)	//ipv4
					{
						if(sipV6_check || dipV6_check || ipV6_check)
							continue;
						if(sip_check && pP6Data->in_src_ipv4_addr!=sip)
							continue;
						if(dip_check && pP6Data->in_dst_ipv4_addr!=dip)
							continue;
						if(ip_check && pP6Data->in_src_ipv4_addr!=ip && pP6Data->in_dst_ipv4_addr!=ip)
							continue;
					}
					else	//ipv6
					{
						if(sip_check || dip_check || ip_check)
							continue;
						if(sipV6_check && pP6Data->in_src_ipv6_addr_hash!=sipV6_hash)
							continue;
						if(dipV6_check && pP6Data->in_dst_ipv6_addr_hash!=dipV6_hash)
							continue;
						if(ipV6_check && pP6Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP6Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
							continue;
					}
					//skip port/l4protocol checking
					if(l4proto_check || sport_check || dport_check)
						continue;
					break;
				default:
					continue;
			}

			if(pP1Data->in_path == 0)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p1Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p2Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 1)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p3Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p4Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 2)
			{
					dump_flow_p5Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == 3)
			{
					dump_flow_p6Rawdata(idx, (void*)pP1Data);
			}
			if(fbMode!= FB_MODE_4K)
			{
				rtk_rg_asic_flowTag_entry_t flowTag;
				bzero(&flowTag, sizeof(flowTag));
				rtk_rg_asic_flowTagTable_get(idx, &flowTag);

				rtlglue_printf(">>flowtag - msb:%d\tttl:%d\n\n", flowTag.hashIdxMsb, flowTag.TTL);
			}
		}
	}
	rtlglue_printf("----------------------------------------------\n");
	return count;

ERROR_PARAMETER:
	rtlglue_printf("Accepted parameters: SPA, DA, SA, SIP, DIP, IP, L4PROTO(tcp: 0x6, udp: 0x11), SPORT, DPORT, SIP6, DIP6\n");
	rtlglue_printf("Example: (Dump flows according to their smac and dip)\n");
	rtlglue_printf("  echo \"SPA 0 SA 00:00:00:0a:0b:0c DIP 10.10.10.1\" > /proc/dump/sw_flow\n");
#endif	// end CONFIG_RG_G3_SERIES
	return count;
}

int32 dump_rg_flowHiPri_table(struct seq_file *s, void *v)
{
	uint32 idx;
	rtk_rg_err_code_t retval=0;

	PROC_PRINTF(">>RG Flow flowHighPriTbl:\n");
	for(idx=0 ; idx<MAX_FLOW_HIGHPRI_TABLE_SIZE ; idx++)
	{
		if(rg_db.flowHighPriTbl[idx].valid==0)
			continue;

		PROC_PRINTF("[%d]  ",idx);
		if(rg_db.flowHighPriTbl[idx].highPriPatten.isIpv6)
		{
			if(rg_db.flowHighPriTbl[idx].highPriPatten.careDipEn)
				PROC_PRINTF("DIP %pI6  ",&rg_db.flowHighPriTbl[idx].highPriPatten.hp_dip[0]);
			if(rg_db.flowHighPriTbl[idx].highPriPatten.careSipEn)
				PROC_PRINTF("SIP %pI6  ",&rg_db.flowHighPriTbl[idx].highPriPatten.hp_sip[0]);
		}
		else
		{
			if(rg_db.flowHighPriTbl[idx].highPriPatten.careDipEn)
				PROC_PRINTF("DIP %pI4h  ",&rg_db.flowHighPriTbl[idx].highPriPatten.hp_dip[0]);
			if(rg_db.flowHighPriTbl[idx].highPriPatten.careSipEn)
				PROC_PRINTF("SIP %pI4h  ",&rg_db.flowHighPriTbl[idx].highPriPatten.hp_sip[0]);
		}
		if(rg_db.flowHighPriTbl[idx].highPriPatten.careSportEn)
			PROC_PRINTF("SPort %d  ",rg_db.flowHighPriTbl[idx].highPriPatten.hp_sport);

		if(rg_db.flowHighPriTbl[idx].highPriPatten.careDportEn)
			PROC_PRINTF("DPort %d  ",rg_db.flowHighPriTbl[idx].highPriPatten.hp_dport);

		if(rg_db.flowHighPriTbl[idx].highPriPatten.careL4ProtoEn)
			PROC_PRINTF("L4PROTO %s",rg_db.flowHighPriTbl[idx].highPriPatten.hp_tcpudp==0?"UDP":"TCP");
		
		if(rg_db.flowHighPriTbl[idx].highPriPatten.assignFlowPriEn)
			PROC_PRINTF("FlowPri %d  ",rg_db.flowHighPriTbl[idx].highPriPatten.hp_flowPri);

		PROC_PRINTF("\n");

	}

	return retval;
}


int32 dump_rg_flow_table(struct seq_file *s, void *v)
{
	uint32 idx=0;
	rtk_rg_err_code_t retval=0;
	rtk_rg_asic_path1_entry_t *pP1Data = NULL;

	//PROC_PRINTF(">>sizeof(rtk_rg_table_flow_t)= %d\n", sizeof(rtk_rg_table_flow_t));

	PROC_PRINTF(">>RG Flow Table:\n");
	for(idx=0; idx<MAX_FLOW_SW_TABLE_SIZE; idx++)
	{
		if(rg_db.flow[idx].sw_valid)
		{
			pP1Data = &RG_PFLOW(idx)->path1;
			if(pP1Data->in_path == FB_PATH_12)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p1Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p2Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == FB_PATH_34)
			{
				if(pP1Data->in_multiple_act == 0)
					dump_flow_p3Rawdata(idx, (void*)pP1Data);
				else
					dump_flow_p4Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == FB_PATH_5)
			{
					dump_flow_p5Rawdata(idx, (void*)pP1Data);
			}else if (pP1Data->in_path == FB_PATH_6)
			{
					dump_flow_p6Rawdata(idx, (void*)pP1Data);
			}
			rtlglue_printf("idleSecs: %u, static: %u, canBeReplaced: %u, isHairpinNat: %u, isLocalInNapt: %u, isTrapToPs: %u, isMulticast: %u, isUsedByFrag: %u, isSwOnly: %u\n", rg_db.flow[idx].idleSecs, rg_db.flow[idx].staticEntry, rg_db.flow[idx].canBeReplaced, rg_db.flow[idx].isHairpinNat, rg_db.flow[idx].isLocalInNapt, rg_db.flow[idx].isTrapToPs, rg_db.flow[idx].isMulticast, rg_db.flow[idx].isUsedByFrag, rg_db.flow[idx].isSwOnly);
			rtlglue_printf("smacL2Idx: %u, dmacL2Idx: %u, naptOrTcpUdpGroupIdx: %d, arpOrNeighborIdx_src: %d, arpOrNeighborIdx_dst: %d, ingressPort: %u, egressTagAccType: %u highPri2HwEntry:%u \n", rg_db.flow[idx].smacL2Idx, rg_db.flow[idx].dmacL2Idx, rg_db.flow[idx].naptOrTcpUdpGroupIdx, rg_db.flow[idx].arpOrNeighborIdx_src, rg_db.flow[idx].arpOrNeighborIdx_dst, rg_db.flow[idx].ingressPort, rg_db.flow[idx].egressTagAccType,rg_db.flow[idx].highPri2HwEntry);
			rtlglue_printf("srcWlanDevIdx: %d, byDmac2cvid: %u, isToSTCmodify: %d, tos_tc: %d, isMeterAct: %d, swMeterOffsetIdx: %d, isFlowMibAct: %d, swFlowMibOffsetIdx: %d\n", rg_db.flow[idx].srcWlanDevIdx, rg_db.flow[idx].byDmac2cvid, rg_db.flow[idx].isToSTCmodify, rg_db.flow[idx].tos_tc, rg_db.flow[idx].isMeterAct, rg_db.flow[idx].swMeterOffsetIdx, rg_db.flow[idx].isFlowMibAct, rg_db.flow[idx].swFlowMibOffsetIdx);
			rtlglue_printf("hasTinyAckStreamId: %d, tinyAckStreamId: %u\n", rg_db.flow[idx].hasTinyAckStreamId, rg_db.flow[idx].tinyAckStreamId);
			if(idx>=MAX_FLOW_HW_TABLE_SIZE)
			{
				rtlglue_printf("naptFilterRateLimitIdx: %d, naptFilterPktCntIdx: %d, naptFilterByteCnttIdx: %d, naptFilterCopyCnt:%d \n",
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].naptFilterInfo.naptFilterRateLimitIdx,
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].naptFilterInfo.naptFilterPktCntIdx,
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].naptFilterInfo.naptFilterByteCnttIdx,
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].naptFilterInfo.naptFilterCopyCnt);
				rtlglue_printf("igr_acl_idx: %d, igr_mirror_count: %d \n",
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].igrACLFilterInfo.acl_index,
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].igrACLFilterInfo.mirrorCnt);
				rtlglue_printf("egr_acl_idx: %d, egr_mirror_count: %d \r\n\n",
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].egrACLFilterInfo.acl_index,
					rg_db.flowList[idx-MAX_FLOW_HW_TABLE_SIZE].egrACLFilterInfo.mirrorCnt);
			}
#if defined(CONFIG_RG_G3_SERIES)
			rtlglue_printf("mainHash_hwFlowIdx: %d\n", rg_db.flow[idx].mainHash_hwFlowIdx);
			if(rg_db.flow[idx].mainHash_hwFlowIdx >= 0)
			{
				if((rg_db.mainHashValidSet[rg_db.flow[idx].mainHash_hwFlowIdx>>5] & (0x1<<(rg_db.flow[idx].mainHash_hwFlowIdx&0x1f)))==0x0)
					rtlglue_printf("\033[1;33;41m[WARNING] Valid bitset of main hash[%d] is not enabled \033[0m\n", rg_db.flow[idx].mainHash_hwFlowIdx);
				if(rg_db.swFlowIdx_mainHashMapping[rg_db.flow[idx].mainHash_hwFlowIdx].swFlowIdx != idx)
					rtlglue_printf("\033[1;33;41m[WARNING] Corresponding swFlowIdx[%d] of main hash[%d] is not synchronized \033[0m\n", rg_db.swFlowIdx_mainHashMapping[rg_db.flow[idx].mainHash_hwFlowIdx].swFlowIdx, rg_db.flow[idx].mainHash_hwFlowIdx);
			}
			rtlglue_printf("Host policing state: %s, flow_meter_mib_conf_dependence: %s\n", (rg_db.flow[idx].hitHostPolingState==RTK_RG_G3_FLOW_HOSTPOLICING_HIT_NONE)?"no hit!":((rg_db.flow[idx].hitHostPolingState==RTK_RG_G3_FLOW_HOSTPOLICING_HIT_SA)?"SMAC hit!":"DMAC hit!"), rg_db.systemGlobal.flow_meter_mib_conf_dependence?"Enable":"Disable");
#endif
		}
	}
	rtlglue_printf("----------------------------------------------\n");

	return retval;
}

int32 dump_rg_flow_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[256] = {0};
	int len = (count > 255) ? 255 : count;

	uint32 idx=0;
	rtk_rg_table_flow_t *pFlow=NULL;
	rtk_rg_asic_path1_entry_t *pP1Data=NULL;
	uint8 spa_check=0, smac_check=0, dmac_check=0, sip_check=0, dip_check=0, ip_check=0, sipV6_check=0, dipV6_check=0, ipV6_check=0, l4proto_check=0, sport_check=0, dport_check=0;
	rtk_rg_port_idx_t spa=0;
	rtk_mac_t smac, dmac;
	uint32 sip=0, dip=0, ip=0;
	rtk_ipv6_addr_t sipV6, dipV6, ipV6;
	uint32 sipV6_hash=0, dipV6_hash=0, ipV6_srcHash=0, ipV6_dstHash=0;
	uint16 l4proto=0, sport=0, dport=0;

	if (buffer && !COPY_FROM_USER(tmpBuf, buffer, len))
	{
		char *strptr,*split_str;
		tmpBuf[len] = '\0';
		strptr=tmpBuf;

		while(1)
		{
			split_str=strsep(&strptr," ");
			if(strptr==NULL) goto ERROR_PARAMETER;

			if(strcasecmp(split_str,"SPA")==0)
			{
				split_str=strsep(&strptr," ");
				spa=simple_strtol(split_str, NULL, 0);
				spa_check=1;
			}
			else if(strcasecmp(split_str,"DA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str, &dmac);
				dmac_check=1;
			}
			else if(strcasecmp(split_str,"SA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str,&smac);
				smac_check=1;
			}
			else if(strcasecmp(split_str,"SIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, sip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					sip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				sip_check=1;
			}
			else if(strcasecmp(split_str,"DIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, dip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					dip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				dip_check=1;
			}
			else if(strcasecmp(split_str,"IP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, ip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					ip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				ip_check=1;
			}
			else if(strcasecmp(split_str,"L4PROTO")==0)
			{
				split_str=strsep(&strptr," ");
				l4proto=simple_strtol(split_str, NULL, 16);
				l4proto_check=1;
			}

			else if(strcasecmp(split_str,"SPORT")==0)
			{
				split_str=strsep(&strptr," ");
				sport=simple_strtol(split_str, NULL, 0);
				sport_check=1;
			}
			else if(strcasecmp(split_str,"DPORT")==0)
			{
				split_str=strsep(&strptr," ");
				dport=simple_strtol(split_str, NULL, 0);
				dport_check=1;
			}
			else if(strcasecmp(split_str,"SIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str, -1, &(sipV6.ipv6_addr[0]), -1, NULL);
				sipV6_check=1;
				sipV6_hash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(sipV6.ipv6_addr);
			}
			else if(strcasecmp(split_str,"DIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(dipV6.ipv6_addr[0]),-1,NULL);
				dipV6_check=1;
				dipV6_hash = _rtk_rg_sw_flowHashIPv6DstAddr_get(dipV6.ipv6_addr);

			}
			else if(strcasecmp(split_str,"IP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(ipV6.ipv6_addr[0]),-1,NULL);
				ipV6_check=1;
				ipV6_srcHash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(ipV6.ipv6_addr);
				ipV6_dstHash = _rtk_rg_sw_flowHashIPv6DstAddr_get(ipV6.ipv6_addr);
			}
			else
			{
				goto ERROR_PARAMETER;
			}

			if (strptr==NULL) break;
		}
	}

	rtlglue_printf(">>RG Flow Table (filter by the following patterns):\n");
	if(spa_check) rtlglue_printf("SPA:%d\n", spa);
	if(dmac_check) rtlglue_printf("DA:%02x:%02x:%02x:%02x:%02x:%02x\n", dmac.octet[0], dmac.octet[1], dmac.octet[2], dmac.octet[3], dmac.octet[4], dmac.octet[5]);
	if(smac_check) rtlglue_printf("SA:%02x:%02x:%02x:%02x:%02x:%02x\n", smac.octet[0], smac.octet[1], smac.octet[2], smac.octet[3], smac.octet[4], smac.octet[5]);
	if(sip_check) rtlglue_printf("SIP:%d.%d.%d.%d\n", (sip>>24)&0xff, (sip>>16)&0xff, (sip>>8)&0xff, (sip)&0xff);
	if(dip_check) rtlglue_printf("DIP:%d.%d.%d.%d\n", (dip>>24)&0xff, (dip>>16)&0xff, (dip>>8)&0xff, (dip)&0xff);
	if(ip_check) rtlglue_printf("IP:%d.%d.%d.%d\n", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, (ip)&0xff);
	if(l4proto_check) rtlglue_printf("L4PROTO:0x%04x\n", l4proto);
	if(sport_check) rtlglue_printf("SPORT:%d\n", sport);
	if(dport_check) rtlglue_printf("DPORT:%d\n", dport);
	if(sipV6_check) rtlglue_printf("SIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x)\n",
						sipV6.ipv6_addr[0], sipV6.ipv6_addr[1], sipV6.ipv6_addr[2], sipV6.ipv6_addr[3],
						sipV6.ipv6_addr[4], sipV6.ipv6_addr[5], sipV6.ipv6_addr[6], sipV6.ipv6_addr[7],
						sipV6.ipv6_addr[8], sipV6.ipv6_addr[9], sipV6.ipv6_addr[10], sipV6.ipv6_addr[11],
						sipV6.ipv6_addr[12], sipV6.ipv6_addr[13], sipV6.ipv6_addr[14], sipV6.ipv6_addr[15], sipV6_hash);
	if(dipV6_check) rtlglue_printf("DIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (dstHash:0x%x)\n",
						dipV6.ipv6_addr[0], dipV6.ipv6_addr[1], dipV6.ipv6_addr[2], dipV6.ipv6_addr[3],
						dipV6.ipv6_addr[4], dipV6.ipv6_addr[5], dipV6.ipv6_addr[6], dipV6.ipv6_addr[7],
						dipV6.ipv6_addr[8], dipV6.ipv6_addr[9], dipV6.ipv6_addr[10], dipV6.ipv6_addr[11],
						dipV6.ipv6_addr[12], dipV6.ipv6_addr[13], dipV6.ipv6_addr[14], dipV6.ipv6_addr[15], dipV6_hash);
	if(ipV6_check) rtlglue_printf("IP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x, dstHash:0x%x)\n",
						ipV6.ipv6_addr[0], ipV6.ipv6_addr[1], ipV6.ipv6_addr[2], ipV6.ipv6_addr[3],
						ipV6.ipv6_addr[4], ipV6.ipv6_addr[5], ipV6.ipv6_addr[6], ipV6.ipv6_addr[7],
						ipV6.ipv6_addr[8], ipV6.ipv6_addr[9], ipV6.ipv6_addr[10], ipV6.ipv6_addr[11],
						ipV6.ipv6_addr[12], ipV6.ipv6_addr[13], ipV6.ipv6_addr[14], ipV6.ipv6_addr[15], ipV6_srcHash, ipV6_dstHash);

	rtlglue_printf("\n");
	for(idx=0; idx<MAX_FLOW_SW_TABLE_SIZE; idx++)
	{
		if(rg_db.flow[idx].sw_valid==0) continue;

		pFlow = &rg_db.flow[idx];
		pP1Data = &RG_PFLOW(idx)->path1;
		//check spa
		if(spa_check && pFlow->ingressPort!=spa) continue;
		//check dmac
		if(dmac_check)
		{
			if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
				&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac, &dmac, sizeof(rtk_mac_t)))
				continue;
			if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
				&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2McEntry.mac, &dmac, sizeof(rtk_mac_t)))
				continue;
		}
		//check smac
		if(smac_check)
		{
			if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
				&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2UcEntry.mac, &smac, sizeof(rtk_mac_t)))
				continue;
			if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
				&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2McEntry.mac, &smac, sizeof(rtk_mac_t)))
				continue;
		}
		switch(pP1Data->in_path)
		{
			case FB_PATH_12:
				//skip ip/port/l4protocol checking
				if(sip_check || dip_check || ip_check || sipV6_check || dipV6_check || ipV6_check || l4proto_check || sport_check || dport_check)
					continue;
				break;
			case FB_PATH_34:
				if(RG_PFLOW(idx)->path3.in_ipv4_or_ipv6==0)	//ipv4
				{
					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && RG_PFLOW(idx)->path3.in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && RG_PFLOW(idx)->path3.in_dst_ipv4_addr!=dip)
						continue;
					if(ip_check && RG_PFLOW(idx)->path3.in_src_ipv4_addr!=ip && RG_PFLOW(idx)->path3.in_dst_ipv4_addr!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && RG_PFLOW(idx)->path3.in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && RG_PFLOW(idx)->path3.in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && RG_PFLOW(idx)->path3.in_src_ipv6_addr_hash!=ipV6_srcHash && RG_PFLOW(idx)->path3.in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				if(l4proto_check && ((RG_PFLOW(idx)->path3.in_l4proto==1 && l4proto!=0x6) || (RG_PFLOW(idx)->path3.in_l4proto==0 && l4proto!=0x11)))
					continue;
				if(sport_check && RG_PFLOW(idx)->path3.in_l4_src_port!=sport)
					continue;
				if(dport_check && RG_PFLOW(idx)->path3.in_l4_dst_port!=dport)
					continue;
				break;
			case FB_PATH_5:
				if(RG_PFLOW(idx)->path5.in_ipv4_or_ipv6==0)	//ipv4
				{
					uint32 checkDip = (RG_PFLOW(idx)->path5.out_l4_act==1 && RG_PFLOW(idx)->path5.out_l4_direction==0)?(rg_db.netif[RG_PFLOW(idx)->path5.in_intf_idx].rtk_netif.ipAddr):(RG_PFLOW(idx)->path5.in_dst_ipv4_addr);

					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && RG_PFLOW(idx)->path5.in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && checkDip!=dip)
						continue;
					if(ip_check && RG_PFLOW(idx)->path5.in_src_ipv4_addr!=ip && checkDip!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && RG_PFLOW(idx)->path5.in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && RG_PFLOW(idx)->path5.in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && RG_PFLOW(idx)->path5.in_src_ipv6_addr_hash!=ipV6_srcHash && RG_PFLOW(idx)->path5.in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				if(l4proto_check && ((RG_PFLOW(idx)->path5.in_l4proto==1 && l4proto!=0x6) || (RG_PFLOW(idx)->path5.in_l4proto==0 && l4proto!=0x11)))
					continue;
				if(sport_check && RG_PFLOW(idx)->path5.in_l4_src_port!=sport)
					continue;
				if(dport_check && RG_PFLOW(idx)->path5.in_l4_dst_port!=dport)
					continue;
				break;
			case FB_PATH_6:
				if(RG_PFLOW(idx)->path6.in_dsliteif==0)	//ipv4
				{
					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && RG_PFLOW(idx)->path6.in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && RG_PFLOW(idx)->path6.in_dst_ipv4_addr!=dip)
						continue;
					if(ip_check && RG_PFLOW(idx)->path6.in_src_ipv4_addr!=ip && RG_PFLOW(idx)->path6.in_dst_ipv4_addr!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && RG_PFLOW(idx)->path6.in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && RG_PFLOW(idx)->path6.in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && RG_PFLOW(idx)->path6.in_src_ipv6_addr_hash!=ipV6_srcHash && RG_PFLOW(idx)->path6.in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				//skip port/l4protocol checking
				if(l4proto_check || sport_check || dport_check)
					continue;
				break;
			default:
				continue;
		}

		if(pP1Data->in_path == FB_PATH_12)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p1Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p2Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_34)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p3Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p4Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_5)
		{
				dump_flow_p5Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_6)
		{
				dump_flow_p6Rawdata(idx, (void*)pP1Data);
		}
		rtlglue_printf("idleSecs: %u, static: %u, canBeReplaced: %u, isHairpinNat: %u, isLocalInNapt: %u, isTrapToPs: %u, isMulticast: %u, isUsedByFrag: %u, isSwOnly: %u\n", rg_db.flow[idx].idleSecs, rg_db.flow[idx].staticEntry, rg_db.flow[idx].canBeReplaced, rg_db.flow[idx].isHairpinNat, rg_db.flow[idx].isLocalInNapt, rg_db.flow[idx].isTrapToPs, rg_db.flow[idx].isMulticast, rg_db.flow[idx].isUsedByFrag, rg_db.flow[idx].isSwOnly);
		rtlglue_printf("smacL2Idx: %u, dmacL2Idx: %u, naptOrTcpUdpGroupIdx: %d, arpOrNeighborIdx_src: %d, arpOrNeighborIdx_dst: %d, ingressPort: %u, egressTagAccType: %u\n", rg_db.flow[idx].smacL2Idx, rg_db.flow[idx].dmacL2Idx, rg_db.flow[idx].naptOrTcpUdpGroupIdx, rg_db.flow[idx].arpOrNeighborIdx_src, rg_db.flow[idx].arpOrNeighborIdx_dst, rg_db.flow[idx].ingressPort, rg_db.flow[idx].egressTagAccType);
		rtlglue_printf("srcWlanDevIdx: %d, byDmac2cvid: %u, isToSTCmodify: %d, tos_tc: %d, isMeterAct: %d, swMeterOffsetIdx: %d, isFlowMibAct: %d, swFlowMibOffsetIdx: %d \r\n\n", rg_db.flow[idx].srcWlanDevIdx, rg_db.flow[idx].byDmac2cvid, rg_db.flow[idx].isToSTCmodify, rg_db.flow[idx].tos_tc, rg_db.flow[idx].isMeterAct, rg_db.flow[idx].swMeterOffsetIdx, rg_db.flow[idx].isFlowMibAct, rg_db.flow[idx].swFlowMibOffsetIdx);
#if defined(CONFIG_RG_G3_SERIES)
		rtlglue_printf("mainHash_hwFlowIdx: %d\n", rg_db.flow[idx].mainHash_hwFlowIdx);
		if(rg_db.flow[idx].mainHash_hwFlowIdx >= 0)
		{
			if((rg_db.mainHashValidSet[rg_db.flow[idx].mainHash_hwFlowIdx>>5] & (0x1<<(rg_db.flow[idx].mainHash_hwFlowIdx&0x1f)))==0x0)
				rtlglue_printf("\033[1;33;41m[WARNING] Valid bitset of main hash[%d] is not enabled \033[0m\n", rg_db.flow[idx].mainHash_hwFlowIdx);
			if(rg_db.swFlowIdx_mainHashMapping[rg_db.flow[idx].mainHash_hwFlowIdx].swFlowIdx != idx)
				rtlglue_printf("\033[1;33;41m[WARNING] Corresponding swFlowIdx[%d] of main hash[%d] is not synchronized \033[0m\n", rg_db.swFlowIdx_mainHashMapping[rg_db.flow[idx].mainHash_hwFlowIdx].swFlowIdx, rg_db.flow[idx].mainHash_hwFlowIdx);
		}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		rtlglue_printf("Host policing state: n.a. (use L2 CLSs)\n");
#else
		rtlglue_printf("Host policing state: %s (use L3 hash)\n", (rg_db.flow[idx].hitHostPolingState==RTK_RG_G3_FLOW_HOSTPOLICING_HIT_NONE)?"no hit!":((rg_db.flow[idx].hitHostPolingState==RTK_RG_G3_FLOW_HOSTPOLICING_HIT_SA)?"SMAC hit!":"DMAC hit!"));
#endif
#endif
	}
	rtlglue_printf("----------------------------------------------\n");
	return count;

ERROR_PARAMETER:
	rtlglue_printf("Accepted parameters: SPA, DA, SA, SIP, DIP, IP, L4PROTO(tcp: 0x6, udp: 0x11), SPORT, DPORT, SIP6, DIP6\n");
	rtlglue_printf("Example: (Dump flows according to their smac and dip)\n");
	rtlglue_printf("  echo \"SPA 0 SA 00:00:00:0a:0b:0c DIP 10.10.10.1\" > /proc/dump/sw_flow\n");
	return count;
}


int32 dump_rg_flow_list(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	int i;
	rtk_rg_flow_linkList_t *pFlowEntry;
	ptrdiff_t flowEntryIdx;
	uint32 freeListCount;

	PROC_PRINTF(">>flow list:\n");
	for(i=0; i<(MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT); i++)
	{
		if(!_rtk_rg_list_head_empty(&rg_db.flowListHead[i]))
		{
			PROC_PRINTF(" Flow_Hash[%3d] : \n", i);
			_rtk_rg_list_for_each_entry(pFlowEntry, &(rg_db.flowList[0]), &rg_db.flowListHead[i], flow_idxList)
			{
				flowEntryIdx = _rtk_rg_list_entry_idx(pFlowEntry, &(rg_db.flowList[0])) + MAX_FLOW_HW_TABLE_SIZE; //use address-based method to get flowList entry index. (flowList[i] mapping to flow [i+MAX_FLOW_HW_TABLE_SIZE])
				if(_rtk_rg_list_next_entry(pFlowEntry, &(rg_db.flowList[0]), flow_idxList) != rg_db.flowListHead[i].first)
					PROC_PRINTF("	 Flow idx[%d] ->\n", flowEntryIdx);
				else
					PROC_PRINTF("	 Flow idx[%d]\n", flowEntryIdx);
			}
			PROC_PRINTF("\n");
		}
	}
#if 1
	freeListCount = 0;
	PROC_PRINTF(">>flow free list:\n");
	_rtk_rg_list_for_each_entry(pFlowEntry, &(rg_db.flowList[0]), &rg_db.flowFreeListHead, flow_idxList)
	{
		flowEntryIdx = _rtk_rg_list_entry_idx(pFlowEntry, &(rg_db.flowList[0])) + MAX_FLOW_HW_TABLE_SIZE; //use address-based method to get flowList entry index. (flowList[i] mapping to flow [i+MAX_FLOW_HW_TABLE_SIZE])
#if 0
		if(_rtk_rg_list_next_entry(pFlowEntry, &(rg_db.flowList[0]), flow_idxList) != rg_db.flowFreeListHead.first)
			PROC_PRINTF("	 Flow idx[%d] ->\n", flowEntryIdx);
		else
			PROC_PRINTF("	 Flow idx[%d]\n", flowEntryIdx);
#endif
		freeListCount++;
		if(freeListCount > MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE)
			WARNING("The flow list count in flow free list is more than %d, please check!",MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE);
	}
	PROC_PRINTF("	 total number of flow list in flow free list: %d\n",freeListCount);
#endif
	PROC_PRINTF("\n");

	return retval;
}

int32 dump_rg_tcp_udp_connection_of_L2L3(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	uint32 i=0, freeNum=0, usedNum=0;
	rtk_rg_flow_tcpUdpTracking_group_linkList_t *pGroupEntry, *pNextGroupEntry;
	rtk_rg_flow_tcpUdpTracking_linkList_t *pFlowEntry, *pNextFlowEntry;
	ptrdiff_t flowEntryIdx;
	char *state[8]={"INVALID","SYN_RECV","SYN_ACK_RECV","TCP_CONNECTED","RST_RECV","FIRST_FIN","FIN_SEND_AND_RECV","LAST_ACK"};

	if(!_rtk_rg_list_head_empty(&rg_db.flowTcpUdpTrackingFreeGroupHead))
	{
		_rtk_rg_list_for_each_entry_safe(pGroupEntry, pNextGroupEntry, &rg_db.flowTcpUdpTrackingGroupList[0], &rg_db.flowTcpUdpTrackingFreeGroupHead, group_idxList)
		{
			freeNum++;
		}
	}
	PROC_PRINTF(">>Software L2/L3 tcp/udp connection (Free:%d): \n", freeNum);
	for(i=0; i<MAX_FLOW_TCP_UDP_TRACKING_TABLE_SIZE; i++)
	{
		if(!_rtk_rg_list_head_empty(&rg_db.flowTcpUdpTrackingGroupHead[i]))
		{
			PROC_PRINTF("------------------- Hash index[%d] -------------------\n", i);
			_rtk_rg_list_for_each_entry_safe(pGroupEntry, pNextGroupEntry, &rg_db.flowTcpUdpTrackingGroupList[0], &rg_db.flowTcpUdpTrackingGroupHead[i], group_idxList)
			{
				usedNum++;

				PROC_PRINTF("[%05d]-[%s]-[idle: %d s]\n", pGroupEntry->group_idx, (pGroupEntry->isL2OrL3)?"L3":"L2", pGroupEntry->idleSecs);

				if(pGroupEntry->isV4OrV6==0)
				{
					PROC_PRINTF(" 	 [%s][%s%s] sip=%d.%d.%d.%d dip=%d.%d.%d.%d sport=%d dport=%d\n",
						(pGroupEntry->isV4OrV6)?"IPv6":"IPv4",
						(pGroupEntry->isTcp)?"TCP ":"UDP",
						(pGroupEntry->isTcp)?state[pGroupEntry->tcpState]:"",
						(pGroupEntry->sip>>24)&0xff, (pGroupEntry->sip>>16)&0xff, (pGroupEntry->sip>>8)&0xff, (pGroupEntry->sip)&0xff,
						(pGroupEntry->dip>>24)&0xff, (pGroupEntry->dip>>16)&0xff, (pGroupEntry->dip>>8)&0xff, (pGroupEntry->dip)&0xff,
						pGroupEntry->sport, pGroupEntry->dport);
				}
				else
				{
					PROC_PRINTF(" 	 [%s][%s%s] sip hash=0x%x dip hash=0x%x sport=%d dport=%d\n",
						(pGroupEntry->isV4OrV6)?"IPv6":"IPv4",
						(pGroupEntry->isTcp)?"TCP ":"UDP",
						(pGroupEntry->isTcp)?state[pGroupEntry->tcpState]:"",
						pGroupEntry->sip, pGroupEntry->dip,
						pGroupEntry->sport, pGroupEntry->dport);
				}
				PROC_PRINTF(" 	 src_cpri_en: %d, src_cpri: %d, dst_cpri_en: %d, dst_cpri: %d\n", pGroupEntry->src_cpri_en, pGroupEntry->src_cpri, pGroupEntry->dst_cpri_en, pGroupEntry->dst_cpri);
				if(!_rtk_rg_list_head_empty(&pGroupEntry->flowTcpUdpTrackingHead))
				{
					_rtk_rg_list_for_each_entry_safe(pFlowEntry, pNextFlowEntry, &(rg_db.flowTcpUdpTrackingList[0]), &pGroupEntry->flowTcpUdpTrackingHead, flow_idxList)
					{
						flowEntryIdx = _rtk_rg_list_entry_idx(pFlowEntry, &(rg_db.flowTcpUdpTrackingList[0])); //use address-based method to get flowTcpUdpTrackingList entry index. (flowTcpUdpTrackingList[i] mapping to flow[i])
						if(rg_db.flow[flowEntryIdx].sw_valid==0)
							PROC_PRINTF("\033[1;33;41m[WARNING] Flow[%d] of L2/L3 tcp connection[%d] is invalid. \033[0m\n", flowEntryIdx, pGroupEntry->group_idx);
						if((RG_PFLOW(flowEntryIdx)->path1.in_path==FB_PATH_34 && RG_PFLOW(flowEntryIdx)->path3.in_multiple_act==0)
							|| (RG_PFLOW(flowEntryIdx)->path1.in_path==FB_PATH_5 && RG_PFLOW(flowEntryIdx)->path5.out_l4_act==0))
							PROC_PRINTF(" 	 Flow [Idx: %u] from %s\n", flowEntryIdx, (RG_PFLOW(flowEntryIdx)->path5.in_src_ipv4_addr==pGroupEntry->sip)?"Client":"Server");
						else
						{
							PROC_PRINTF("\033[1;33;41m[WARNING] Flow[%d] of L2/L3 tcp connection[%d] is not bridge/route. \033[0m\n", flowEntryIdx, pGroupEntry->group_idx);
							if(rg_db.flow[flowEntryIdx].sw_valid)
							{
								rtk_rg_asic_path1_entry_t *pP1Data = &RG_PFLOW(flowEntryIdx)->path1;
								uint32 idx = flowEntryIdx;
								if(pP1Data->in_path == FB_PATH_12)
								{
									if(pP1Data->in_multiple_act == 0)
										dump_flow_p1Rawdata(idx, (void*)pP1Data);
									else
										dump_flow_p2Rawdata(idx, (void*)pP1Data);
								}else if (pP1Data->in_path == FB_PATH_34)
								{
									if(pP1Data->in_multiple_act == 0)
										dump_flow_p3Rawdata(idx, (void*)pP1Data);
									else
										dump_flow_p4Rawdata(idx, (void*)pP1Data);
								}else if (pP1Data->in_path == FB_PATH_5)
								{
										dump_flow_p5Rawdata(idx, (void*)pP1Data);
								}else if (pP1Data->in_path == FB_PATH_6)
								{
										dump_flow_p6Rawdata(idx, (void*)pP1Data);
								}
								PROC_PRINTF("idleSecs: %u, static: %u, canBeReplaced: %u, isHairpinNat: %u, isLocalInNapt: %u, isTrapToPs: %u, isMulticast: %u, isUsedByFrag: %u, isSwOnly: %u\n", rg_db.flow[idx].idleSecs, rg_db.flow[idx].staticEntry, rg_db.flow[idx].canBeReplaced, rg_db.flow[idx].isHairpinNat, rg_db.flow[idx].isLocalInNapt, rg_db.flow[idx].isTrapToPs, rg_db.flow[idx].isMulticast, rg_db.flow[idx].isUsedByFrag, rg_db.flow[idx].isSwOnly);
								PROC_PRINTF("smacL2Idx: %u, dmacL2Idx: %u, naptOrTcpUdpGroupIdx: %d, arpOrNeighborIdx_src: %d, arpOrNeighborIdx_dst: %d, ingressPort: %u, egressTagAccType: %u\r\n\n", rg_db.flow[idx].smacL2Idx, rg_db.flow[idx].dmacL2Idx, rg_db.flow[idx].naptOrTcpUdpGroupIdx, rg_db.flow[idx].arpOrNeighborIdx_src, rg_db.flow[idx].arpOrNeighborIdx_dst, rg_db.flow[idx].ingressPort, rg_db.flow[idx].egressTagAccType);
								PROC_PRINTF("srcWlanDevIdx: %d, byDmac2cvid: %u\r\n\n", rg_db.flow[idx].srcWlanDevIdx, rg_db.flow[idx].byDmac2cvid);
							}
							PROC_PRINTF("\n");
						}
					}
				}
				else
					PROC_PRINTF(" 	 No any flow exist\n");
			}
		}
	}

	if((freeNum+usedNum)!=MAX_FLOW_TCP_UDP_TRACKING_GROUP_SIZE)
		PROC_PRINTF("\033[1;33;41m[WARNING] Leak of L2/L3 tcp connection, Used=%d Free=%d \033[0m\n", usedNum, freeNum);

	return retval;
}

int32 dump_rg_sw_frag_of_L2L3(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	uint32 i, fragNum=0, fragFreeNum=0, fragQueueNum=0, fragQueueFreeNum=0, tmpQueueNum=0;
	rtk_rg_L2L3_fragment_t *pFragList;
	rtk_rg_L2L3_fragment_queue_t *pFragQueueList;

	PROC_PRINTF(">>Sw L2/L3 fragment list:\n");
	for(i=0; i<MAX_L2L3_FRAGMENT_HASH_BUCKET; i++)
	{
		if(!list_empty(&rg_db.L2L3FragHashListHead[i]))
		{
			PROC_PRINTF("------------------- Fragment Hash index[%3d] -------------------\n", i);
			list_for_each_entry(pFragList, &rg_db.L2L3FragHashListHead[i], frag_list)
			{
				fragNum++;

				PROC_PRINTF("------- Fragment index[%3d] -------\n", pFragList->frag_idx);
				if(pFragList->isV4OrV6==0)
				{
					PROC_PRINTF(" 	 [%s] sip=%d.%d.%d.%d dip=%d.%d.%d.%d ipId=%d ipProto=0x%x sport=%d dport=%d\n",
						(pFragList->isV4OrV6)?"IPv6":"IPv4",
						(pFragList->sip>>24)&0xff, (pFragList->sip>>16)&0xff, (pFragList->sip>>8)&0xff, (pFragList->sip)&0xff,
						(pFragList->dip>>24)&0xff, (pFragList->dip>>16)&0xff, (pFragList->dip>>8)&0xff, (pFragList->dip)&0xff,
						pFragList->ipId, pFragList->ipProto,
						pFragList->sport, pFragList->dport);
				}
				else
				{
					PROC_PRINTF(" 	 [%s] sip hash=0x%x dip hash=0x%x ipId=%d ipProto=0x%x sport=%d dport=%d\n",
						(pFragList->isV4OrV6)?"IPv6":"IPv4",
						pFragList->sip, pFragList->dip,
						pFragList->ipId, pFragList->ipProto,
						pFragList->sport, pFragList->dport);
				}
				PROC_PRINTF(" 	 [%s] fragAction=%d beginIdleTime=%lu receivedLength=%d totalLength=%d flowHitIdx=%d\n",
							(pFragList->fragAction==RG_FWDENGINE_RET_QUEUE_FRAG)? "Queue packet" : ((pFragList->fragAction==RG_FWDENGINE_RET_DROP)? "Drop" :((pFragList->fragAction==RG_FWDENGINE_RET_TO_PS)? "To PS" : "Forward")),
							pFragList->fragAction, pFragList->beginIdleTime, pFragList->receivedLength, pFragList->totalLength, pFragList->flowHitIdx);
				tmpQueueNum=0;
				if(!list_empty(&pFragList->fragQueueHead))
				{
					list_for_each_entry(pFragQueueList, &pFragList->fragQueueHead, fragQueue_list)
					{
						fragQueueNum++;
						tmpQueueNum++;
					}
				}
				if(pFragList->queueCount==tmpQueueNum)
					PROC_PRINTF(" 	 queueCount=%d", pFragList->queueCount);
				else
					PROC_PRINTF("\033[1;33;41m[WARNING] queueCount(%d) does not synchronize with number(%d) of packets in queue.\033[0m\n", pFragList->queueCount, tmpQueueNum);
			}
			PROC_PRINTF("\n");
		}
	}

	if(!list_empty(&rg_db.L2L3FragFreeListHead))
	{
		list_for_each_entry(pFragList, &rg_db.L2L3FragFreeListHead, frag_list)
		{
			fragFreeNum++;
		}
	}
	if(!list_empty(&rg_db.L2L3FragQueueFreeListHead))
	{
		list_for_each_entry(pFragQueueList, &rg_db.L2L3FragQueueFreeListHead, fragQueue_list)
		{
			fragQueueFreeNum++;
		}
	}
	if((fragNum+fragFreeNum)==MAX_L2L3_FRAGMENT_TABLE_SIZE)
		PROC_PRINTF("[Fragment List] Usage=%d, Free=%d\n", fragNum, fragFreeNum);
	else
		PROC_PRINTF("\033[1;33;41m[WARNING] [Fragment List] Usage(%d)+Free(%d) != Total(%d)\033[0m\n", fragNum, fragFreeNum, MAX_L2L3_FRAGMENT_TABLE_SIZE);
	if((fragQueueNum+fragQueueFreeNum)==MAX_L2L3_FRAGMENT_QUEUE_SIZE)
		PROC_PRINTF("[Fragment Queue] Usage=%d, Free=%d\n", fragQueueNum, fragQueueFreeNum);
	else
		PROC_PRINTF("\033[1;33;41m[WARNING] [Fragment Queue] Usage(%d)+Free(%d) != Total(%d)\033[0m\n", fragQueueNum, fragQueueFreeNum, MAX_L2L3_FRAGMENT_QUEUE_SIZE);

	return retval;
}

int32 flush_rg_sw_frag_of_L2L3(struct file *file, const char *buffer, unsigned long count, void *data)
{
	_rtk_rg_L2L3_fragmentListFlush();

	return count;
}

#if defined(CONFIG_RG_FLOW_4K_MODE)
int32 dump_rg_flow_tcamList(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	int i;
	rtk_rg_flowTcam_linkList_t *pFlowTcamEntry;

	PROC_PRINTF(">>flowTcam list:\n");
	for(i=0; i<(MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT); i++)
	{
		if(!list_empty(&rg_db.flowTcamListHead[i]))
		{
			PROC_PRINTF(" Flow_Hash[%3d] : \n", i);
			list_for_each_entry(pFlowTcamEntry, &rg_db.flowTcamListHead[i], flowTcam_list)
			{
				if(pFlowTcamEntry->flowTcam_list.next!=&rg_db.flowTcamListHead[i])
					PROC_PRINTF("	 FlowTcam idx[%d] ->\n", pFlowTcamEntry->idx);
				else
					PROC_PRINTF("	 FlowTcam idx[%d]\n", pFlowTcamEntry->idx);
			}
			PROC_PRINTF("\n");
		}
	}
	PROC_PRINTF(">>flowTcam free list:\n");
	list_for_each_entry(pFlowTcamEntry, &rg_db.flowTcamFreeListHead, flowTcam_list)
	{
		if(pFlowTcamEntry->flowTcam_list.next!=&rg_db.flowTcamFreeListHead)
			PROC_PRINTF("	 FlowTcam idx[%d] ->\n", pFlowTcamEntry->idx);
		else
			PROC_PRINTF("	 FlowTcam idx[%d]\n", pFlowTcamEntry->idx);
	}
	PROC_PRINTF("\n");

	return retval;
}
#endif

int32 dump_flowdram_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0;
	rtk_rg_asic_path1_entry_t *pP1Data = NULL;

	PROC_PRINTF(">>Main Memory Flow Table:\n");
	for(idx=0; idx<MAX_FLOW_HW_TABLE_SIZE; idx++)
	{
		// Read DDR flow entries
		//retval = rtk_rg_asic_flowPath1_get(idx, pP1Data);
		pP1Data=&RG_PFLOW(idx)->path1;

		if(!pP1Data->valid) continue;

		if(pP1Data->in_path == 0)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p1Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p2Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 1)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p3Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p4Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 2)
		{
				dump_flow_p5Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 3)
		{
				dump_flow_p6Rawdata(idx, (void*)pP1Data);
		}
	}
	rtlglue_printf("----------------------------------------------\n");
#else
	uint32 idx=0, entryNum= _rtk_rg_flowEntryNum_get();
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_asic_path1_entry_t *pP1Data = NULL;

	if(fbMode == FB_MODE_4K)
	{
		PROC_PRINTF("Not Support in 4K mode.\n");
		return SUCCESS;
	}

	pP1Data = rtk_rg_malloc(sizeof(rtk_rg_asic_path1_entry_t));

	PROC_PRINTF(">>Main Memory Flow Table:\n");
	for(idx=0; idx<entryNum; idx++)
	{
		// Read DDR flow entries
		retval = rtk_rg_asic_flowPath1_get(idx, pP1Data);

		if(!pP1Data->valid) continue;

		if(pP1Data->in_path == 0)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p1Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p2Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 1)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p3Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p4Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 2)
		{
				dump_flow_p5Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == 3)
		{
				dump_flow_p6Rawdata(idx, (void*)pP1Data);
		}
	}
	rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pP1Data);
#endif
	return retval;
}

int32 dump_flowdram_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data)
{
#if !defined(CONFIG_RG_G3_SERIES)
	unsigned char tmpBuf[256] = {0};
	int len = (count > 255) ? 255 : count;

	uint32 idx=0, entryNum= _rtk_rg_flowEntryNum_get();
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();
	rtk_rg_table_flow_t *pFlow=NULL;
	rtk_rg_asic_path1_entry_t *pP1Data = rtk_rg_malloc(sizeof(rtk_rg_asic_path1_entry_t));
	rtk_rg_asic_path3_entry_t *pP3Data = (rtk_rg_asic_path3_entry_t *)pP1Data;
	rtk_rg_asic_path5_entry_t *pP5Data = (rtk_rg_asic_path5_entry_t *)pP1Data;
	rtk_rg_asic_path6_entry_t *pP6Data = (rtk_rg_asic_path6_entry_t *)pP1Data;
	uint8 spa_check=0, smac_check=0, dmac_check=0, sip_check=0, dip_check=0, ip_check=0, sipV6_check=0, dipV6_check=0, ipV6_check=0, l4proto_check=0, sport_check=0, dport_check=0;
	rtk_rg_port_idx_t spa=0;
	rtk_mac_t smac, dmac;
	uint32 sip=0, dip=0, ip=0;
	rtk_ipv6_addr_t sipV6, dipV6, ipV6;
	uint32 sipV6_hash=0, dipV6_hash=0, ipV6_srcHash=0, ipV6_dstHash=0;
	uint16 l4proto=0, sport=0, dport=0;

	if(fbMode == FB_MODE_4K)
	{
		rtlglue_printf("Not Support in 4K mode.\n");
		return SUCCESS;
	}

	if (buffer && !COPY_FROM_USER(tmpBuf, buffer, len))
	{
		char *strptr,*split_str;
		tmpBuf[len] = '\0';
		strptr=tmpBuf;

		while(1)
		{
			split_str=strsep(&strptr," ");
			if(strptr==NULL) goto ERROR_PARAMETER;

			if(strcasecmp(split_str,"SPA")==0)
			{
				split_str=strsep(&strptr," ");
				spa=simple_strtol(split_str, NULL, 0);
				spa_check=1;
			}
			else if(strcasecmp(split_str,"DA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str, &dmac);
				dmac_check=1;
			}
			else if(strcasecmp(split_str,"SA")==0)
			{
				split_str=strsep(&strptr," ");
				_rtk_rg_str2mac(split_str,&smac);
				smac_check=1;
			}
			else if(strcasecmp(split_str,"SIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, sip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					sip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				sip_check=1;
			}
			else if(strcasecmp(split_str,"DIP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, dip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					dip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				dip_check=1;
			}
			else if(strcasecmp(split_str,"IP")==0)
			{
				char *ip_token, *split_ip_token, j;

				split_str=strsep(&strptr," ");
				ip_token=split_str;
				for(j=0, ip=0; j<4; j++)
				{
					if(ip_token==NULL) goto ERROR_PARAMETER;
					split_ip_token=strsep(&ip_token,".");
					ip|=(simple_strtol(split_ip_token, NULL, 0)<<((3-j)<<3));
				}
				ip_check=1;
			}
			else if(strcasecmp(split_str,"L4PROTO")==0)
			{
				split_str=strsep(&strptr," ");
				l4proto=simple_strtol(split_str, NULL, 16);
				l4proto_check=1;
			}

			else if(strcasecmp(split_str,"SPORT")==0)
			{
				split_str=strsep(&strptr," ");
				sport=simple_strtol(split_str, NULL, 0);
				sport_check=1;
			}
			else if(strcasecmp(split_str,"DPORT")==0)
			{
				split_str=strsep(&strptr," ");
				dport=simple_strtol(split_str, NULL, 0);
				dport_check=1;
			}
			else if(strcasecmp(split_str,"SIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str, -1, &(sipV6.ipv6_addr[0]), -1, NULL);
				sipV6_check=1;
				sipV6_hash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(sipV6.ipv6_addr);
			}
			else if(strcasecmp(split_str,"DIP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(dipV6.ipv6_addr[0]),-1,NULL);
				dipV6_check=1;
				dipV6_hash = _rtk_rg_sw_flowHashIPv6DstAddr_get(dipV6.ipv6_addr);
			}
			else if(strcasecmp(split_str,"IP6")==0)
			{
				split_str=strsep(&strptr," ");
				in6_pton(split_str,-1,&(ipV6.ipv6_addr[0]),-1,NULL);
				ipV6_check=1;
				ipV6_srcHash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(ipV6.ipv6_addr);
				ipV6_dstHash = _rtk_rg_sw_flowHashIPv6DstAddr_get(ipV6.ipv6_addr);
			}
			else
			{
				goto ERROR_PARAMETER;
			}

			if (strptr==NULL) break;
		}
	}

	rtlglue_printf(">>Main Memory Flow Table (filter by the following patterns):\n");
	if(spa_check) rtlglue_printf("SPA:%d\n", spa);
	if(dmac_check) rtlglue_printf("DA:%02x:%02x:%02x:%02x:%02x:%02x\n", dmac.octet[0], dmac.octet[1], dmac.octet[2], dmac.octet[3], dmac.octet[4], dmac.octet[5]);
	if(smac_check) rtlglue_printf("SA:%02x:%02x:%02x:%02x:%02x:%02x\n", smac.octet[0], smac.octet[1], smac.octet[2], smac.octet[3], smac.octet[4], smac.octet[5]);
	if(sip_check) rtlglue_printf("SIP:%d.%d.%d.%d\n", (sip>>24)&0xff, (sip>>16)&0xff, (sip>>8)&0xff, (sip)&0xff);
	if(dip_check) rtlglue_printf("DIP:%d.%d.%d.%d\n", (dip>>24)&0xff, (dip>>16)&0xff, (dip>>8)&0xff, (dip)&0xff);
	if(ip_check) rtlglue_printf("IP:%d.%d.%d.%d\n", (ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, (ip)&0xff);
	if(l4proto_check) rtlglue_printf("L4PROTO:0x%04x\n", l4proto);
	if(sport_check) rtlglue_printf("SPORT:%d\n", sport);
	if(dport_check) rtlglue_printf("DPORT:%d\n", dport);
	if(sipV6_check) rtlglue_printf("SIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x)\n",
						sipV6.ipv6_addr[0], sipV6.ipv6_addr[1], sipV6.ipv6_addr[2], sipV6.ipv6_addr[3],
						sipV6.ipv6_addr[4], sipV6.ipv6_addr[5], sipV6.ipv6_addr[6], sipV6.ipv6_addr[7],
						sipV6.ipv6_addr[8], sipV6.ipv6_addr[9], sipV6.ipv6_addr[10], sipV6.ipv6_addr[11],
						sipV6.ipv6_addr[12], sipV6.ipv6_addr[13], sipV6.ipv6_addr[14], sipV6.ipv6_addr[15], sipV6_hash);
	if(dipV6_check) rtlglue_printf("DIP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (dstHash:0x%x)\n",
						dipV6.ipv6_addr[0], dipV6.ipv6_addr[1], dipV6.ipv6_addr[2], dipV6.ipv6_addr[3],
						dipV6.ipv6_addr[4], dipV6.ipv6_addr[5], dipV6.ipv6_addr[6], dipV6.ipv6_addr[7],
						dipV6.ipv6_addr[8], dipV6.ipv6_addr[9], dipV6.ipv6_addr[10], dipV6.ipv6_addr[11],
						dipV6.ipv6_addr[12], dipV6.ipv6_addr[13], dipV6.ipv6_addr[14], dipV6.ipv6_addr[15], dipV6_hash);
	if(ipV6_check) rtlglue_printf("IP6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x (srcHash:0x%x, dstHash:0x%x)\n",
						ipV6.ipv6_addr[0], ipV6.ipv6_addr[1], ipV6.ipv6_addr[2], ipV6.ipv6_addr[3],
						ipV6.ipv6_addr[4], ipV6.ipv6_addr[5], ipV6.ipv6_addr[6], ipV6.ipv6_addr[7],
						ipV6.ipv6_addr[8], ipV6.ipv6_addr[9], ipV6.ipv6_addr[10], ipV6.ipv6_addr[11],
						ipV6.ipv6_addr[12], ipV6.ipv6_addr[13], ipV6.ipv6_addr[14], ipV6.ipv6_addr[15], ipV6_srcHash, ipV6_dstHash);

	rtlglue_printf("\n");
	for(idx=0; idx<entryNum; idx++)
	{
		// Read DDR flow entries
		assert_ok(rtk_rg_asic_flowPath1_get(idx, pP1Data));

		if(!pP1Data->valid) continue;

		pFlow = &rg_db.flow[idx];

		//check spa
		if(spa_check && pFlow->ingressPort!=spa) continue;
		//check dmac
		if(dmac_check)
		{
			if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
				&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac, &dmac, sizeof(rtk_mac_t)))
				continue;
			if(rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
				&& memcmp(&rg_db.lut[pFlow->dmacL2Idx].rtk_lut.entry.l2McEntry.mac, &dmac, sizeof(rtk_mac_t)))
				continue;
		}
		//check smac
		if(smac_check)
		{
			if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2UC
				&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2UcEntry.mac, &smac, sizeof(rtk_mac_t)))
				continue;
			if(rg_db.lut[pFlow->smacL2Idx].rtk_lut.entryType==RTK_LUT_L2MC
				&& memcmp(&rg_db.lut[pFlow->smacL2Idx].rtk_lut.entry.l2McEntry.mac, &smac, sizeof(rtk_mac_t)))
				continue;
		}
		switch(pP1Data->in_path)
		{
			case FB_PATH_12:
				//skip ip/port/l4protocol checking
				if(sip_check || dip_check || ip_check || sipV6_check || dipV6_check || ipV6_check || l4proto_check || sport_check || dport_check)
					continue;
				break;
			case FB_PATH_34:
				if(pP3Data->in_ipv4_or_ipv6==0) //ipv4
				{
					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && pP3Data->in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && pP3Data->in_dst_ipv4_addr!=dip)
						continue;
					if(ip_check && pP3Data->in_src_ipv4_addr!=ip && pP3Data->in_dst_ipv4_addr!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && pP3Data->in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && pP3Data->in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && pP3Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP3Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				if(l4proto_check && ((pP3Data->in_l4proto==1 && l4proto!=0x6) || (pP3Data->in_l4proto==0 && l4proto!=0x11)))
					continue;
				if(sport_check && pP3Data->in_l4_src_port!=sport)
					continue;
				if(dport_check && pP3Data->in_l4_dst_port!=dport)
					continue;
				break;
			case FB_PATH_5:
				if(pP5Data->in_ipv4_or_ipv6==0) //ipv4
				{
					uint32 checkDip = (pP5Data->out_l4_act==1 && pP5Data->out_l4_direction==0)?(rg_db.netif[pP5Data->in_intf_idx].rtk_netif.ipAddr):(pP5Data->in_dst_ipv4_addr);

					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && pP5Data->in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && checkDip!=dip)
						continue;
					if(ip_check && pP5Data->in_src_ipv4_addr!=ip && checkDip!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && pP5Data->in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && pP5Data->in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && pP5Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP5Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				if(l4proto_check && ((pP5Data->in_l4proto==1 && l4proto!=0x6) || (pP5Data->in_l4proto==0 && l4proto!=0x11)))
					continue;
				if(sport_check && pP5Data->in_l4_src_port!=sport)
					continue;
				if(dport_check && pP5Data->in_l4_dst_port!=dport)
					continue;
				break;
			case FB_PATH_6:
				if(pP6Data->in_dsliteif==0) //ipv4
				{
					if(sipV6_check || dipV6_check || ipV6_check)
						continue;
					if(sip_check && pP6Data->in_src_ipv4_addr!=sip)
						continue;
					if(dip_check && pP6Data->in_dst_ipv4_addr!=dip)
						continue;
					if(ip_check && pP6Data->in_src_ipv4_addr!=ip && pP6Data->in_dst_ipv4_addr!=ip)
						continue;
				}
				else	//ipv6
				{
					if(sip_check || dip_check || ip_check)
						continue;
					if(sipV6_check && pP6Data->in_src_ipv6_addr_hash!=sipV6_hash)
						continue;
					if(dipV6_check && pP6Data->in_dst_ipv6_addr_hash!=dipV6_hash)
						continue;
					if(ipV6_check && pP6Data->in_src_ipv6_addr_hash!=ipV6_srcHash && pP6Data->in_dst_ipv6_addr_hash!=ipV6_dstHash)
						continue;
				}
				//skip port/l4protocol checking
				if(l4proto_check || sport_check || dport_check)
					continue;
				break;
			default:
				continue;
		}

		if(pP1Data->in_path == FB_PATH_12)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p1Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p2Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_34)
		{
			if(pP1Data->in_multiple_act == 0)
				dump_flow_p3Rawdata(idx, (void*)pP1Data);
			else
				dump_flow_p4Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_5)
		{
				dump_flow_p5Rawdata(idx, (void*)pP1Data);
		}else if (pP1Data->in_path == FB_PATH_6)
		{
				dump_flow_p6Rawdata(idx, (void*)pP1Data);
		}

	}
	rtlglue_printf("----------------------------------------------\n");
	return count;

ERROR_PARAMETER:
	rtlglue_printf("Accepted parameters: SPA, DA, SA, SIP, DIP, IP, L4PROTO(tcp: 0x6, udp: 0x11), SPORT, DPORT, SIP6, DIP6\n");
	rtlglue_printf("Example: (Dump flows according to their smac and dip)\n");
	rtlglue_printf("  echo \"SPA 0 SA 00:00:00:0a:0b:0c DIP 10.10.10.1\" > /proc/dump/sw_flow\n");
#endif //!defined(CONFIG_RG_G3_SERIES)
	return count;
}

int32 dump_flow_table(struct seq_file *s, void *v)
{
#if defined(CONFIG_RG_G3_SERIES)
	flow_table_dump();
	return 0;
#else	// not CONFIG_RG_G3_SERIES

#if defined(CONFIG_RG_FLOW_4K_MODE)
	return dump_flow_sram_cam_table(s, v);
#else	// not CONFIG_RG_FLOW_4K_MODE
	return dump_flowdram_table(s, v);
#endif	// end CONFIG_RG_FLOW_4K_MODE

#endif	// end CONFIG_RG_G3_SERIES
}

int32 dump_flow_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data)
{
#if defined(CONFIG_RG_G3_SERIES)
	rtlglue_printf("Not support!\n");
	return count;
#else	// not CONFIG_RG_G3_SERIES

#if defined(CONFIG_RG_FLOW_4K_MODE)
	return dump_flow_sram_cam_table_by_filter(file, buffer, count, data);
#else	// not CONFIG_RG_FLOW_4K_MODE
	return dump_flowdram_table_by_filter(file, buffer, count, data);
#endif	// end CONFIG_RG_FLOW_4K_MODE

#endif	// end CONFIG_RG_G3_SERIES
}

int32 dump_flowtrf_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0, entryNum= _rtk_rg_flowEntryNum_get();
	rtk_rg_asic_fbMode_t fbMode = _rtk_rg_fbMode_get();

	uint32 *pflowTrfvalids = rtk_rg_malloc(sizeof(uint32) * 1024);
	uint32 *pflowTrfbits = rtk_rg_malloc(sizeof(uint32) * 1024);

	memset(pflowTrfvalids, 1, (sizeof(uint32) * 1024));

	rtlglue_printf("\n>>ASIC flow traffic table - FB mode: %d (%d entries)\n", fbMode, entryNum);
	ASSERT_EQ(rtk_rg_asic_flowTraffic_get(&pflowTrfvalids[0], &pflowTrfbits[0]), SUCCESS);

	entryNum >>= 5;
	for(idx = 0; idx < entryNum; idx++)
	{
		if(pflowTrfbits[idx]!=0x0)
			PROC_PRINTF("FlowEntry [%d~%d]: 0x%08x\r\n", idx<<5, ((idx+1)<<5)-1, pflowTrfbits[idx]);
	}


	rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pflowTrfvalids);
	rtk_rg_free(pflowTrfbits);
#endif //!defined(CONFIG_RG_G3_SERIES)
	return retval;
}


int32 dump_flow_mib(struct seq_file *s, void *v)
{
	uint32 idx=0;
	rtk_rg_err_code_t retval=0;
	rtk_rg_table_flowmib_t *pFlowMIB = rtk_rg_malloc(sizeof(rtk_rg_table_flowmib_t));

	PROC_PRINTF(">>ASIC Flow MIB Table: (Show Non Zero Entries)\n");
	PROC_PRINTF("     [Ingress]        \t[Egress]\r\n");
	PROC_PRINTF("     pktCnt  byteCnt\tpktCnt  byteCnt\r\n");

	for(idx=0; idx<FLOWBASED_TABLESIZE_FLOWMIB; idx++)
	{
		ASSERT_EQ(RTK_RG_ASIC_FLOWMIB_GET(idx, pFlowMIB), SUCCESS);

		// skip empty entries
		if(!(pFlowMIB->in_packet_cnt||pFlowMIB->in_byte_cnt||pFlowMIB->out_packet_cnt||pFlowMIB->out_byte_cnt))
			continue;
		PROC_PRINTF("[%2d] %-6u  %-7llu\t%-6u  %-7llu\r\n", idx, pFlowMIB->in_packet_cnt, pFlowMIB->in_byte_cnt, pFlowMIB->out_packet_cnt, pFlowMIB->out_byte_cnt);
	}
	PROC_PRINTF("Pure SW flow mib\n");
	for(idx=PURE_SW_FLOWMIB_IDX_OFFSET ; idx<PURE_SW_FLOWMIB_TABLE_SIZE+PURE_SW_FLOWMIB_IDX_OFFSET; idx++)
	{
		ASSERT_EQ(RTK_RG_ASIC_FLOWMIB_GET(idx, pFlowMIB), SUCCESS);

		// skip empty entries
		if(!(pFlowMIB->in_packet_cnt||pFlowMIB->in_byte_cnt||pFlowMIB->out_packet_cnt||pFlowMIB->out_byte_cnt))
			continue;
		PROC_PRINTF("[%2d] %-6u  %-7llu\t%-6u  %-7llu\r\n", idx, pFlowMIB->in_packet_cnt, pFlowMIB->in_byte_cnt, pFlowMIB->out_packet_cnt, pFlowMIB->out_byte_cnt);
	}
	PROC_PRINTF("----------------------------------------------\n");

	rtk_rg_free(pFlowMIB);

	return retval;
}


int32 dump_macind_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0;
	rtk_rg_asic_indirectMac_entry_t *pMacInd = NULL;
	pMacInd = rtk_rg_malloc(sizeof(rtk_rg_asic_indirectMac_entry_t));

	PROC_PRINTF(">>ASIC Mac Indirect Access Table:\n");
	for(idx = 0; idx < FLOWBASED_TABLESIZE_MACIND; idx++)
	{
		rtk_rg_asic_indirectMacTable_get(idx, pMacInd);
		if((pMacInd->l2_idx != 0x0 && pMacInd->l2_idx != 0xfff) || rg_db.indMacTbl[idx].indMacEnt.l2_idx != 0x0)		// note: 0 is valid entry, should not be filtered.
			rtlglue_printf("[%d] lut index: %d (sw: %d, refcount: %d)\n", idx, pMacInd->l2_idx, rg_db.indMacTbl[idx].indMacEnt.l2_idx, rg_db.indMacTbl[idx].indMacRefCount);
	}
	rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pMacInd);
#endif
	return retval;
}

int32 dump_ethtype_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	uint32 idx=0;
	rtk_rg_asic_etherType_entry_t *pEthType = NULL;
	pEthType = rtk_rg_malloc(sizeof(rtk_rg_asic_etherType_entry_t));

	PROC_PRINTF(">>ASIC EtherType Table:\n");
	for(idx = 0; idx < FLOWBASED_TABLESIZE_ETHERTYPE; idx++)
	{
#if !defined(CONFIG_RG_G3_SERIES)
		rtk_rg_asic_etherTypeTable_get(idx, pEthType);
#endif //!defined(CONFIG_RG_G3_SERIES)
		if(pEthType->ethertype != 0x0)
			rtlglue_printf("[%d] ethertype: 0x%x\n", idx, pEthType->ethertype);
	}
	rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pEthType);

	return retval;
}

int32 dump_extpmask_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	uint32 idx=0;
	rtk_rg_asic_extPortMask_entry_t *pExtPMask = NULL;
	pExtPMask = rtk_rg_malloc(sizeof(rtk_rg_asic_extPortMask_entry_t));

	PROC_PRINTF(">>ASIC ExtPortMask Table:\n");
	for(idx = 0; idx < FLOWBASED_TABLESIZE_EXTPORT; idx++)
	{
#if defined(CONFIG_RG_G3_SERIES)
		// no hw table
		pExtPMask->extpmask = rg_db.extPortTbl[idx].extPortEnt.extpmask;
#else
		rtk_rg_asic_extPortMaskTable_get(idx, pExtPMask);
#endif
		if(pExtPMask->extpmask != 0x0)
			rtlglue_printf("[%d] extpmask: 0x%x swRefCnt=%d \n", idx, pExtPMask->extpmask,rg_db.extPortTbl[idx].extPortRefCount);
	}
	rtlglue_printf("----------------------------------------------\n");

	rtk_rg_free(pExtPMask);

	return retval;
}
#if defined(CONFIG_RG_RTL9607C_SERIES)
int32 dump_wanaccesslimit_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 idx=0, i=0;
	rtk_enable_t state;
	rtk_rg_asic_wanAccessLimit_entry_t *pWalEnt = NULL;
	pWalEnt = rtk_rg_malloc(sizeof(rtk_rg_asic_wanAccessLimit_entry_t));

	PROC_PRINTF(">>ASIC WAN Access Limit Table:\n");
	PROC_PRINTF("--SMAC Index List:--\n");
	for(idx = 0; idx < MAX_WANACCESSLIMIT_TABLE_SIZE; idx++)
	{
		rtk_rg_asic_wanAccessLimitTable_get(idx, pWalEnt);
		if(pWalEnt->valid)
			rtlglue_printf("[%d] SA_IDX: %d mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				idx, pWalEnt->sa_idx
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[0]
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[1]
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[2]
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[3]
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[4]
					   , rg_db.lut[pWalEnt->sa_idx].rtk_lut.entry.l2UcEntry.mac.octet[5]);
	}

	PROC_PRINTF("\nLimited Ports(SW): 0x%X\n\n", rg_db.systemGlobal.wanAccessLimit_pmask.portmask);

	PROC_PRINTF("--Enabled Ports:--\n");
	for(idx = RTK_RG_PORT0; idx <= RTK_RG_PORT_MASTERCPU_CORE1; idx++)
	{
		rtk_rg_asic_wanAccessLimitPort_get(idx, &state);
		if(state)
			rtlglue_printf("%5d", idx);
	}
	PROC_PRINTF("\n--Enabled EXTPorts:--\n");
	for(idx = RTK_RG_EXT_PORT0; idx <= RTK_RG_EXT_PORT5; idx++)
	{
		rtk_rg_asic_wanAccessLimitPort_get(idx, &state);
		if(state)
			rtlglue_printf("  9-%d", idx-11);
	}
	for(idx = RTK_RG_MAC10_EXT_PORT0; idx <= RTK_RG_MAC10_EXT_PORT5; idx++)
	{
		rtk_rg_asic_wanAccessLimitPort_get(idx, &state);
		if(state)
			rtlglue_printf(" 10-%d", idx-17);
	}
	for(idx = RTK_RG_MAC7_EXT_PORT0; idx <= RTK_RG_MAC7_EXT_PORT5; idx++)
	{
		rtk_rg_asic_wanAccessLimitPort_get(idx, &state);
		if(state)
			rtlglue_printf("  7-%d", idx-23);
	}

	rtlglue_printf("\n----------------------------------------------\n");


	PROC_PRINTF(">>WAN Access Limit learning statistics:\n");
	for(i=0;i<RTK_RG_PORT_MAX;i++)
	{
		if(RG_INVALID_PORT(i)) continue;
		PROC_PRINTF("  port[%d] ",i);
		if(rg_db.systemGlobal.accessWanLimitPortBased[i]>=0)
		{
			PROC_PRINTF("limit is %04d, count is %04d, limitReachAction is %s\n",rg_db.systemGlobal.accessWanLimitPortBased[i],
				atomic_read(&rg_db.systemGlobal.accessWanLimitPortBasedCount[i]),
				rg_db.systemGlobal.accessWanLimitPortBasedAction[i]==SA_LEARN_EXCEED_ACTION_DROP?"DROP":
				rg_db.systemGlobal.accessWanLimitPortBasedAction[i]==SA_LEARN_EXCEED_ACTION_PERMIT?"PERMIT":"PERMIT_L2");
		}
		else
		{
			PROC_PRINTF("Unlimited, count is %04d\n",atomic_read(&rg_db.systemGlobal.accessWanLimitPortBasedCount[i]));
		}

	}


	rtk_rg_free(pWalEnt);
#endif
	return retval;
}
#endif

int32 dump_extratag_table(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
#if !defined(CONFIG_RG_G3_SERIES)
	uint32 list=0, idx=0, notEmpty=0, len = 0;
	char contentBuf[512];
	rtk_rg_asic_extraTagAction_t *pExtraTag = NULL;
	pExtraTag = rtk_rg_malloc(sizeof(rtk_rg_asic_extraTagAction_t));

	PROC_PRINTF(">>ASIC ExtraTag Table:\n");
	for(list = FLOWBASED_EXTRATAG_LISTMIN; list <= FLOWBASED_EXTRATAG_LISTMAX; list++)
	{
		notEmpty = 0;
		PROC_PRINTF(" -- Action List[%d] --\n", list);
		for(idx = 0; idx < FLOWBASED_EXTRATAG_ACTIONS; idx++)
		{
			rtk_rg_asic_extraTagAction_get(list, idx, pExtraTag);
			if(pExtraTag->type1.act_bit==FB_EXTG_ACTBIT_NOACTION)
				break;
			else{
				switch(pExtraTag->type1.act_bit)
				{
					case FB_EXTG_ACTBIT_1:
						PROC_PRINTF("\tacttype[%d]: inserttag bufoff:%d len:%d\n", pExtraTag->type1.act_bit, pExtraTag->type1.src_addr_offset, pExtraTag->type1.length);
						notEmpty = 1;
						break;
					case FB_EXTG_ACTBIT_2:
						PROC_PRINTF("\tacttype[%d]: ethtran ethertype:0x%04x\n", pExtraTag->type2.act_bit, pExtraTag->type2.ethertype);
						break;
					case FB_EXTG_ACTBIT_3:
						PROC_PRINTF("\tacttype[%d]: updatelen pktoff:%d width:%d value:%d op:%d\n", pExtraTag->type3.act_bit, pExtraTag->type3.pkt_buff_offset, pExtraTag->type3.length, pExtraTag->type3.value, pExtraTag->type3.operation);
						break;
					case FB_EXTG_ACTBIT_4:
						if(pExtraTag->type4.data_src_type==1)
							PROC_PRINTF("\tacttype[%d]: ipv4id pktoff:%d, intfidx:%d\n", pExtraTag->type4.act_bit, pExtraTag->type4.pkt_buff_offset, pExtraTag->type4.seq_ack_reg_idx);
						else
							PROC_PRINTF("\tacttype[%d]: greseqack pktoff:%d, intfidx:%d, reduceseq:%d, reduceack:%d\n", pExtraTag->type4.act_bit, pExtraTag->type4.pkt_buff_offset, pExtraTag->type4.seq_ack_reg_idx, pExtraTag->type4.reduce_seq, pExtraTag->type4.reduce_ack);
						break;
					case FB_EXTG_ACTBIT_5:
						PROC_PRINTF("\tacttype[%d]: l3chksum pktoff:%d\n", pExtraTag->type5.act_bit, pExtraTag->type5.pkt_buff_offset);
						break;
					case FB_EXTG_ACTBIT_6:
						PROC_PRINTF("\tacttype[%d]: l4chksum pktoff:%d\n", pExtraTag->type6.act_bit, pExtraTag->type6.pkt_buff_offset);
						break;
				}
			}
		}
		if(notEmpty)
		{
			rtk_rg_asic_extraTagInsertHdrLen_get(list, &len);
			PROC_PRINTF("\t>>IncresedLen = %d\n", len);
		}

	}
	bzero(&contentBuf, sizeof(contentBuf));
	rtk_rg_asic_extraTagContentBuffer_get(0, 512, (char *)&contentBuf);
	memDump(&contentBuf, sizeof(contentBuf), "[Extra Tag Content Buffer]");


	rtlglue_printf("\n----------------------------------------------\n");

	rtk_rg_free(pExtraTag);
#endif //!defined(CONFIG_RG_G3_SERIES)
	return retval;
}

#endif //CONFIG_RG_FLOW_BASED_PLATFORM

int32 dump_rg_mac_filter_whitelist(struct seq_file *s, void *v){
	rtk_rg_err_code_t retval=0;
	unsigned int i;

	PROC_PRINTF(">>MAC filter white list:\n");
	for(i=0 ; i<g_whiteList_idx ; i++) {
		PROC_PRINTF("[%d] MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", i
                       , g_whiteList[i].octet[0]
                       , g_whiteList[i].octet[1]
                       , g_whiteList[i].octet[2]
                       , g_whiteList[i].octet[3]
                       , g_whiteList[i].octet[4]
                       , g_whiteList[i].octet[5]);
	}

	return retval;
}

int32  dump_icmp_connection(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval=0;
	rtk_rg_table_icmp_linkList_t *pEntry;
	int32 i=0;

	PROC_PRINTF("ICMP connection:\n");
	//lookup all link-list
	for(pEntry=rg_db.pICMPCtrlFlowHead->pPrev;pEntry!=rg_db.pICMPCtrlFlowHead;pEntry=pEntry->pPrev)
	{
		if(pEntry->icmpFlow.valid==1 )
		{
			PROC_PRINTF("[%d][WaitReply] internalIP=%pI4h remoteIP=%pI4h ICMPCode[0x%x] ICMPID[0x%x] \n\r ICMPSeqNum[0x%x] ICMPType[0x%x] inboundIPID[0x%x] IPID[0x%x] psConnection[%d] jiffies[%u]\n",
				i,&pEntry->icmpFlow.internalIP,&pEntry->icmpFlow.remoteIP,pEntry->icmpFlow.ICMPCode,pEntry->icmpFlow.ICMPID,
				pEntry->icmpFlow.ICMPSeqNum,pEntry->icmpFlow.ICMPType,pEntry->icmpFlow.inboundIPID,pEntry->icmpFlow.IPID ,pEntry->icmpFlow.psConnection,pEntry->icmpFlow.jiffies);
			i++;
		}
	}

	return retval;
}



#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int32 dump_rg_funcbasedMeter(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval = RT_ERR_RG_OK;
	rtk_rg_funcbasedMeterConf_t mConf;
	int i, j;

	PROC_PRINTF("funcbasedMeter Mode: %s \n", (rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_NOT_INIT)?"RTK_RG_METERMODE_NOT_INIT":((rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)?"RTK_RG_METERMODE_HW_INDEX":"RTK_RG_METERMODE_SW_INDEX"));
	for(i = 0 ; i < RTK_RG_METER_MAX ; i++)
	{
		mConf.type = i;
		if(mConf.type == RTK_RG_METER_ACL)
			PROC_PRINTF("---------- funcbasedMeter type: %s ----------\n", "ACL");
		else if(mConf.type == RTK_RG_METER_HOSTPOL)
			PROC_PRINTF("---------- funcbasedMeter type: %s ----------\n", "host policing");
		else if(mConf.type == RTK_RG_METER_STORMCTL)
			PROC_PRINTF("---------- funcbasedMeter type: %s ----------\n", "storm control");
		else if(mConf.type == RTK_RG_METER_PROC)
			PROC_PRINTF("---------- funcbasedMeter type: %s ----------\n", "proc");
		for(j = 0 ; j < MAX_FUNCBASEDMETER_SIZE ; j++)
		{
			mConf.idx = j;
			if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)
			{
				retval = RT_ERR_RG_OK;
				mConf.state = rg_db.systemGlobal.funbasedMeter[mConf.type][mConf.idx].state;
				if(mConf.state == RTK_RG_ENABLED)
					WARNING("Check! funbasedMeter_mode is RTK_RG_METERMODE_HW_INDEX, but funbasedMeter[%d][%d] is enabled!!", mConf.type, mConf.idx);
			}
			else
				retval = rtk_rg_apolloPro_funcbasedMeter_get(&mConf);
			if(retval== RT_ERR_RG_OK)
			{
    			PROC_PRINTF("funcbasedMeter[%2d]: %s", mConf.idx, mConf.state?"Enable":"Disable");
				if(mConf.state)
				{
					PROC_PRINTF("(rate=%d, ifgInclude=%s,", mConf.rate, mConf.ifgInclude?"Include":"Exclude");
					PROC_PRINTF(" hwIndex=%d)", rg_db.systemGlobal.funbasedMeter[mConf.type][mConf.idx].hwIdx);
				}
			}
			else
			{
				PROC_PRINTF("Get funcbasedMeter[%d] failed. return value = %x", mConf.idx, retval);
			}
			PROC_PRINTF("\n");
		}
		PROC_PRINTF("\n");
	}
	return retval;
}

int dump_rg_funcbasedMeter_by_type( struct file *filp, const char *buff,unsigned long len, void *data )
{
	int i, retval, type = _rtk_rg_pasring_proc_string_to_integer(buff,len);
	rtk_rg_funcbasedMeterConf_t mConf;

	rtlglue_printf("funcbasedMeter Mode: %s \n", (rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_NOT_INIT)?"RTK_RG_METERMODE_NOT_INIT":((rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)?"RTK_RG_METERMODE_HW_INDEX":"RTK_RG_METERMODE_SW_INDEX"));
	if(type == RTK_RG_METER_ACL)
		rtlglue_printf("---------- funcbasedMeter type: %s ----------\n", "ACL");
	else if(type == RTK_RG_METER_HOSTPOL)
		rtlglue_printf("---------- funcbasedMeter type: %s ----------\n", "host policing");
	else if(type == RTK_RG_METER_STORMCTL)
		rtlglue_printf("---------- funcbasedMeter type: %s ----------\n", "storm control");
	if(type >= RTK_RG_METER_MAX)
	{
		rtlglue_printf("Invalid type! The type value should be less than %d.\n", RTK_RG_METER_MAX);
		return len;
	}

	mConf.type = type;
	for(i = 0 ; i < MAX_FUNCBASEDMETER_SIZE ; i++)
	{
		mConf.idx = i;
		retval = rtk_rg_apolloPro_funcbasedMeter_get(&mConf);
		if(retval== RT_ERR_RG_OK)
		{
    		rtlglue_printf("funcbasedMeter[%2d]: %s", mConf.idx, mConf.state?"Enable":"Disable");
			if(mConf.state)
			{
				rtlglue_printf("(rate=%d,\t ifgInclude=%s,\t", mConf.rate, mConf.ifgInclude?"Include":"Exclude");
				rtlglue_printf(" hwIndex=%d)", rg_db.systemGlobal.funbasedMeter[mConf.type][mConf.idx].hwIdx);
			}
		}
		else
		{
			rtlglue_printf("Get funcbasedMeter[%d] failed. return value = %x", mConf.idx, retval);
		}
		rtlglue_printf("\n");
	}
	rtlglue_printf("\n");
	return len;
}
int32 dump_rg_hostPolicing(struct seq_file *s, void *v)
{
	rtk_rg_err_code_t retval = RT_ERR_RG_OK;
	rtk_mac_t zeroMac;
	int i;

	memset(&zeroMac, 0, sizeof(rtk_mac_t));
	PROC_PRINTF("List non-empty entries only.\n");

	for(i = 0 ; i < HOST_POLICING_TABLE_SIZE ; i++)
	{
		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
			continue;
		PROC_PRINTF("--------------------------------------------------\n");
		PROC_PRINTF("hostPolicing[%d]: %02x:%02x:%02x:%02x:%02x:%02x\n", i, rg_db.hostPoliceList[i].info.macAddr.octet[0], rg_db.hostPoliceList[i].info.macAddr.octet[1], rg_db.hostPoliceList[i].info.macAddr.octet[2], rg_db.hostPoliceList[i].info.macAddr.octet[3], rg_db.hostPoliceList[i].info.macAddr.octet[4], rg_db.hostPoliceList[i].info.macAddr.octet[5]);
		PROC_PRINTF("ingressLimitCtrl: %s\n", rg_db.hostPoliceList[i].info.ingressLimitCtrl?"ENABLE":"DISABLE");
		if(rg_db.hostPoliceList[i].info.ingressLimitCtrl)
			PROC_PRINTF("     - limitMeterIdx: %d\n", rg_db.hostPoliceList[i].info.limitMeterIdx);
		PROC_PRINTF("egressLimitCtrl: %s\n", rg_db.hostPoliceList[i].info.egressLimitCtrl?"ENABLE":"DISABLE");
		if(rg_db.hostPoliceList[i].info.egressLimitCtrl)
			PROC_PRINTF("     - limitMeterIdx: %d\n", rg_db.hostPoliceList[i].info.limitMeterIdx);
		PROC_PRINTF("mibCountCtrl: %s\n", rg_db.hostPoliceList[i].info.mibCountCtrl?"ENABLE":"DISABLE");
#if defined(CONFIG_RG_G3_SERIES)
		PROC_PRINTF("loggingRx_policerIdx: %3d", rg_db.hostPoliceList[i].loggingRx_policerIdx);
		if((G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX <= rg_db.hostPoliceList[i].loggingRx_policerIdx) && (rg_db.hostPoliceList[i].loggingRx_policerIdx < (G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX + G3_FLOW_POLICER_HOSTPOLICING_SIZE)))
			PROC_PRINTF("     (flow policer group: HOSTPOLMTR, index in group: %d)\n", rg_db.hostPoliceList[i].loggingRx_policerIdx - G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX);
		else if((G3_FLOW_POLICER_IDXSHIFT_HPLOGRX <= rg_db.hostPoliceList[i].loggingRx_policerIdx) && (rg_db.hostPoliceList[i].loggingRx_policerIdx < G3_FLOW_POLICER_IDXSHIFT_HPLOGRX + G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE))
			PROC_PRINTF("     (flow policer group: HPLOGRX, index in group: %d)\n", rg_db.hostPoliceList[i].loggingRx_policerIdx - G3_FLOW_POLICER_IDXSHIFT_HPLOGRX);
		else
			PROC_PRINTF("\n");
		PROC_PRINTF("loggingTx_policerIdx: %3d", rg_db.hostPoliceList[i].loggingTx_policerIdx);
		if((G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX <= rg_db.hostPoliceList[i].loggingTx_policerIdx) && (rg_db.hostPoliceList[i].loggingTx_policerIdx < (G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX + G3_FLOW_POLICER_HOSTPOLICING_SIZE)))
			PROC_PRINTF("     (flow policer group: HOSTPOLMTR, index in group: %d)\n", rg_db.hostPoliceList[i].loggingTx_policerIdx - G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX);
		else if((G3_FLOW_POLICER_IDXSHIFT_HPLOGTX <= rg_db.hostPoliceList[i].loggingTx_policerIdx) && (rg_db.hostPoliceList[i].loggingTx_policerIdx < G3_FLOW_POLICER_IDXSHIFT_HPLOGTX + G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE))
			PROC_PRINTF("     (flow policer group: HPLOGTX, index in group: %d)\n", rg_db.hostPoliceList[i].loggingTx_policerIdx - G3_FLOW_POLICER_IDXSHIFT_HPLOGTX);
		else
			PROC_PRINTF("\n");
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		PROC_PRINTF("ingressSA_l2ClsIdx: %3d, egressDA_l2ClsIdx: %3d\n", rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress, rg_db.hostPoliceList[i].l2ClsIndex.idx_egress);
#endif
#endif
	}
	PROC_PRINTF("--------------------------------------------------\n");
	return retval;
}

#endif

void common_dump(int level,const char *funcs,int line, char *comment,...)
{
	int show=1;
	char string[16]={0};
	int color, bgcolor;
	va_list a_list;
	va_start(a_list,comment);

	if(rg_kernel.filter_level&level)
	{
		//show=_rtk_rg_trace_filter_compare(rg_db.pktHdr->skb,rg_db.pktHdr);
		show=rg_kernel.tracefilterShow;
	}
	if(show==1)
	{
		int mt_trace_i;

		vsprintf( rg_mt_watch_tmp, comment,a_list);
		for(mt_trace_i=1;mt_trace_i<512;mt_trace_i++)
		{
			if(rg_mt_watch_tmp[mt_trace_i]==0)
			{
				if(rg_mt_watch_tmp[mt_trace_i-1]=='\n') rg_mt_watch_tmp[mt_trace_i-1]=' ';
				else break;
			}
		}

		switch(level)
		{
			case RTK_RG_DEBUG_LEVEL_DEBUG:
				sprintf(string,"DEBUG"); color=33; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_FIXME:
				sprintf(string,"FIXME"); color=34; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_CALLBACK:
				sprintf(string,"CALLBACK"); color=34; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_TRACE:
				sprintf(string,"TRACE"); color=35; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_ACL:
				sprintf(string,"ACL"); color=36; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_WARN:
				sprintf(string,"WARNING"); color=33; bgcolor=41;
				break;
			case RTK_RG_DEBUG_LEVEL_TRACE_DUMP:
				sprintf(string,"TRACE_DUMP");color=35; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_WMUX:
				sprintf(string,"WMUX");color=32; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_MACLEARN:
				sprintf(string,"MACLEARN");color=31; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_TABLE:
				sprintf(string,"TABLE");color=31; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_ALG:
				sprintf(string,"ALG");color=31; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_IGMP:
				sprintf(string,"IGMP");color=31; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_ACL_RRESERVED:
				sprintf(string,"ACL_RSV");color=36; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_RG_API:
				sprintf(string,"RG_API");color=36; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_TIMER:
				sprintf(string,"TIMER");color=31; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_ACL_CONTROL_PATH:
				sprintf(string,"ACL_CTRL");color=36; bgcolor=40;
				break;
			case RTK_RG_DEBUG_LEVEL_TABLE_FULL:
				sprintf(string,"TABLE_FULL");color=33; bgcolor=41;
				break;
			case RTK_RG_DEBUG_LEVEL_WIFI_FAST_FORWARD:
				sprintf(string,"WIFI_FAST_FWD");color=31; bgcolor=40;
				break;
			default:
				sprintf(string,"XXX");color=37; bgcolor=40;
				break;

		}
		rtlglue_printf("\033[1;%d;%dm[%s] %s\033[1;30;40m @ %s(%d)\033[0m\n",color,bgcolor,string,rg_mt_watch_tmp,funcs,line);
	}

	va_end (a_list);
}


rtk_rg_proc_t debugProc[]=
{
#if defined(CONFIG_RG_RTL9602C_SERIES)
	{
		.name="sipfilter" ,
		.get = dump_ipmc_sipFilter ,
		.set = NULL ,
	},
	{
		.name="ipmv6_l3mcr" ,
		.get = dump_ipmv6_l3mcr_table ,
		.set = NULL ,
	},
	{
		.name="dslite" ,
		.get = dump_dslite_table ,
		.set = NULL ,
	},
	{
		.name="dslite_status" ,
		.get = dump_dslite_status ,
		.set = NULL ,
	},
	{
		.name="dsliteMc" ,
		.get = dump_dsliteMc_table ,
		.set = NULL ,
	},
    {
        .name="host_policing" ,
        .get =  _rtk_rg_proc_host_policing_get,
        .set =  NULL,
    },
#endif
	{
		.name="napt" ,
		.get = dump_napt ,
		.set = NULL ,
	},
	{
		.name="sw_napt" ,
		.get = dump_sw_napt ,
		.set = NULL ,
	},
	{
		.name="l4" ,
		.get = dump_sw_napt ,
		.set = NULL ,
	},
	{
		.name="sw_netstat" ,
		.get = dump_netstat ,
		.set = NULL ,
	},
	{
		.name="netif" ,
		.get = dump_netif ,
		.set = NULL ,
	},
	{
		.name="sw_netif" ,
		.get = dump_sw_netif ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_WAN_MSS_CACHE)
	{
		.name="sw_msscache" ,
		.get = mibdump_mssCache ,
		.set = NULL ,
	},
#endif
	{
		.name="pppoe" ,
		.get = dump_pppoe ,
		.set = NULL ,
	},
	{
		.name="sw_pppoe" ,
		.get = dump_sw_pppoe ,
		.set = NULL ,
	},
	{
		.name="nexthop" ,
		.get = dump_nexthop ,
		.set = NULL ,
	},
	{
		.name="sw_nexthop" ,
		.get = dump_sw_nexthop ,
		.set = NULL ,
	},
	{
		.name="arp" ,
		.get = dump_arp ,
		.set = NULL ,
	},
	{
		.name="sw_arp" ,
		.get = dump_sw_arp ,
		.set = NULL ,
	},
	{
		.name="ip" ,
		.get = dump_ip ,
		.set = NULL ,
	},
	{
		.name="sw_ip" ,
		.get = dump_sw_ip ,
		.set = NULL ,
	},
	{
		.name="v6Extip" ,
		.get = dump_ipv6_extIp ,
		.set = NULL ,
	},
	{
		.name="l3" ,
		.get = dump_l3 ,
		.set = NULL ,
	},
	{
		.name="sw_l3" ,
		.get = dump_sw_l3 ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="sw_mcFlow" ,
		.get = sw_mcFlow ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_G3_SERIES)
	{
		.name="sw_mcHwInfo" ,
		.get = sw_mcHwInfo ,
		.set = NULL ,
	},
#endif
#endif
	{
		.name="l4hs" ,
		.get = dump_l4hs ,
		.set = tracefilterRULE0_dump_hs_timer ,
	},
	{
		.name="hs" ,
		.get = dump_hs ,
		.set = tracefilterRULE0_dump_hs_timer ,
	},
	{
		.name="sw_lut_group" ,
		.get = dump_lut_group_table ,
		.set = NULL ,
	},
	{
		.name="lut" ,
		.get = dump_lut_table ,
		.set = NULL ,
	},
	{
		.name="bcamlut" ,
		.get = dump_lut_bcam_table ,
		.set = NULL ,
	},
	{
		.name="l2" ,
		.get = dump_lut_table ,
		.set = NULL ,
	},
	{
		.name="ipmcgrp" ,
		.get = dump_ipmc_group ,
		.set = NULL ,
	},
	{
		.name="ipmc" ,
		.get = dump_ipmc_routing ,
		.set = NULL ,
	},
	{
		.name="sw_mac_ip" ,
		.get = mac_ip_table ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	{
		.name="vlan" ,
		.get = dump_vlan_table ,
		.set = NULL ,
	},
#endif
	{
		.name="vlanmbr" ,
		.get = dump_vlan_mbr ,
		.set = NULL ,
	},
	{
		.name="svlan" ,
		.get = dump_svlan_mbr ,
		.set = NULL ,
	},
	{
		.name="gatewayServicePort" ,
		.get = dump_gatewayServicePort ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==1)
	{
		.name="ipset" ,
		.get = dump_rg_ipset ,
		.set = dump_rg_ipset_by_index ,
	},
#endif
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
	{
		.name="ipsets" ,
		.get = dump_rg_ipsets ,
		.set = dump_rg_ipsets_by_index ,
	},
#endif
	{
		.name="acl" ,
		.get = dump_acl ,
		.set = dump_acl_by_index ,
	},
#if defined(CONFIG_RG_G3_SERIES)
	{
		.name="acl_ca" ,
		.get = dump_acl_ca ,
		.set = dump_acl_ca_by_index ,
	},
#endif
	{
		.name="acl_rg" ,
		.get = dump_rg_acl ,
		.set = NULL ,
	},
	{
		.name="sw_acl" ,
		.get = dump_rg_acl ,
		.set = NULL ,
	},
	{
		.name="napt_filterAndQos_rg" ,
		.get = dump_rg_naptPriority ,
		.set = NULL ,
	},
	{
		.name="sw_napt_filterAndQos" ,
		.get = dump_rg_naptPriority ,
		.set = NULL ,
	},
	{
		.name="acl_template" ,
		.get = dump_acl_template ,
		.set = NULL ,
	},
	{
		.name="acl_vidrange" ,
		.get = dump_acl_vidRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_iprange" ,
		.get = dump_acl_ipRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_portrange" ,
		.get = dump_acl_portRangeTable ,
		.set = NULL ,
	},
	{
		.name="acl_pktlenrange" ,
		.get = dump_acl_pktlenRangeTable ,
		.set = NULL ,
	},
	{
		.name="cf" ,
		.get = dump_cf ,
		.set = NULL ,
	},
	{
		.name="cf_rg" ,
		.get = dump_rg_cf ,
		.set = NULL ,
	},
	{
		.name="sw_cf" ,
		.get = dump_rg_cf ,
		.set = NULL ,
	},
	{
		.name="acl_reserved_info" ,
		.get = dump_rg_acl_reserved_info,
		.set = NULL ,
	},
#if 0
	{
		.name="reg" ,
		.get = dump_reg ,
		.set = NULL ,
	},
#endif
	{
		.name="bind" ,
		.get = dump_l34_bind_table ,
		.set = NULL ,
	},
	{
		.name="wantype" ,
		.get = dump_l34_wantype_table ,
		.set = NULL ,
	},
	{
		.name="sw_wantype" ,
		.get = dump_l34_sw_wantype_table ,
		.set = NULL ,
	},
	{
		.name="v6route" ,
		.get = dump_ipv6_route_table ,
		.set = NULL ,
	},
	{
		.name="sw_v6route" ,
		.get = dump_sw_ipv6_route_table ,
		.set = NULL ,
	},
	{
		.name="neighbor" ,
		.get = dump_ipv6_neighbor_table ,
		.set = NULL ,
	},
	{
		.name="portIso" ,
		.get = dump_piso ,
		.set = NULL ,
	},
	{
		.name="sw_portIso" ,
		.get = mibdump_port_isolation ,
		.set = NULL ,
	},
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="sw_shortcut" ,
		.get = dump_sw_shortcut ,
		.set = NULL ,
	},
#endif
	{
		.name="sw_mbssid" ,
		.get = dump_wlan_mbssid ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpAll" ,
		.get = mibdump_redirect_httpAll ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpCount" ,
		.get = mibdump_redirect_httpCount ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpURL" ,
		.get = mibdump_redirect_httpURL ,
		.set = NULL ,
	},
	{
		.name="sw_redirectHttpWhiteList" ,
		.get = mibdump_redirect_httpWhiteList ,
		.set = NULL ,
	},
	{
		.name="diag_shell" ,
		.get = diag_shell_dump ,
		.set = NULL ,
	},
	{
		.name="sw_layer2LearningStatistics" ,
		.get = mibdump_softwareLearningStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_urlFilter" ,
		.get = dump_rg_urlFilter ,
		.set = NULL ,
	},
	{
		.name="sw_urlAssignPri" ,
		.get = dump_rg_urlPri ,
		.set = NULL ,
	},
	{
		.name="sw_accessWanStatistics" ,
		.get = mibdump_accessWanStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_groupMacStatistics" ,
		.get = mibdump_groupMacStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_vlanGroupMacLimitStatistics" ,
		.get = mibdump_vlanGroupMacLimitStatistics ,
		.set = NULL ,
	},
	{
		.name="sw_categories" ,
		.get = mibdump_softwareLearningCategories ,
		.set = NULL,
	},
	{
		.name="sw_lut" ,
		.get = mibdump_lut_table ,
		.set = NULL ,
	},
	{
		.name="sw_bcamlut" ,
		.get = mibdump_lut_bcam_table ,
		.set = NULL ,
	},
	{
		.name="sw_neighbor" ,
		.get = mibdump_ipv6_neighbor_table ,
		.set = NULL ,
	},
	{
		.name="sw_frag" ,
		.get = mibdump_frag ,
		.set = NULL ,
	},
	{
		.name="sw_srvInLan" ,
		.get = mibdump_srvInLan_IP_table ,
		.set = NULL ,
	},
	{
		.name="sw_virtualServers" ,
		.get = mibdump_virtualServer ,
		.set = NULL ,
	},
	{
		.name="sw_portTrigger" ,
		.get = mibdump_portTrigger ,
		.set = NULL ,
	},
	{
		.name="sw_upnpConnections" ,
		.get = mibdump_upnpConnection ,
		.set = NULL ,
	},
	{
		.name="sw_bind" ,
		.get = mibdump_l34_bind_table ,
		.set = NULL ,
	},

#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT

	{
		.name="sw_dmzSettings" ,
		.get = mibdump_DMZ ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_APOLLO_ROMEDRIVER

	{
		.name="sw_algDatabase" ,
		.get = _rtk_rg_alg_displayAllDataBase ,
		.set = NULL ,
	},
	{
		.name="sw_algDynamicPort" ,

		.get = mibdump_alg_dynamic_port ,
		.set = NULL ,
	},
	{
		.name="sw_algPreservePort" ,

		.get = mibdump_alg_preserve_port ,
		.set = NULL ,
	},
	{
		.name="sw_algSrvInLanIPAddress" ,
		.get = mibdump_alg_srvInLan_ip ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_RG_WMUX_SUPPORT

	{
		.name="wmux" ,
		.get = _rtk_rg_wmux_info ,
		.set = NULL ,
	},
#endif

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT

	{
		.name="sw_ipv6statefulInfo" ,
		.get = mibdump_ipv6_stateful_connections ,
		.set = NULL ,
	},
	{
		.name="sw_ipv6fragInfo" ,
		.get = mibdump_v6Frag ,
		.set = NULL ,
	},
#endif
	{
		.name="sw_vlan" ,
		.get = dump_rg_vlan_table ,
		.set = NULL ,
	},
	{
		.name="gponDsBcFilterAndRemarking" ,
		.get = dump_gpon_ds_bc_filterAndRemarkingRules ,
		.set = NULL ,
	},

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	{
		.name="sw_vmacSkb" ,
		.get = mibdump_vmac_skb ,
		.set = mibclean_vmac_skb ,
	},
#endif
	{
		.name="staticRoute" ,
		.get = mibdump_staticRoute ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="l34hs" ,
		.get = dump_l34hs,
		.set = NULL,
	},
#if defined(CONFIG_RG_G3_SERIES)
	{
		.name="headera",
		.get = dump_headera,
		.set = NULL ,
	},
	{
		.name="headeri",
		.get = dump_headeri,
		.set = NULL ,
	},
	{
		.name="dropcount",
		.get = dump_dropcount,
		.set = dump_dropcount_opcode ,
	},

#else
#if !defined(CONFIG_RG_FLOW_4K_MODE)
	{
		.name="flowSramCam" ,
		.get = dump_flow_sram_cam_table ,
		.set = NULL ,
	},
#endif	// end !defined(CONFIG_RG_FLOW_4K_MODE)
	{
		.name="flowTag" ,
		.get = dump_flowtag_table ,
		.set = NULL ,
	},
	{
		.name="flowTrf" ,
		.get = dump_flowtrf_table ,
		.set = NULL ,
	},
	{
		.name="camtag" ,
		.get = dump_camtag_table ,
		.set = NULL ,
	},
#endif	// end defined(CONFIG_RG_G3_SERIES)
	{
		.name="flow" ,
		.get = dump_flow_table ,
		.set = dump_flow_table_by_filter ,
	},
	{
		.name="sw_flow" ,
		.get = dump_rg_flow_table ,
		.set = dump_rg_flow_table_by_filter ,
	},
	{
		.name="sw_flow_list" ,
		.get = dump_rg_flow_list ,
		.set = NULL ,
	},
	{
		.name="flowmib" ,
		.get = dump_flow_mib,
		.set = NULL ,
	},
	{
		.name="macind" ,
		.get = dump_macind_table ,
		.set = NULL ,
	},
	{
		.name="ethtype" ,
		.get = dump_ethtype_table ,
		.set = NULL ,
	},
	{
		.name="extpmask" ,
		.get = dump_extpmask_table ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_RTL9607C_SERIES)
	{
		.name="wanaccesslimit" ,
		.get = dump_wanaccesslimit_table ,
		.set = NULL ,
	},
#endif
	{
		.name="extratag" ,
		.get = dump_extratag_table ,
		.set = NULL ,
	},
	{
		.name="sw_tcp_udp_connection_of_L2L3" ,
		.get = dump_rg_tcp_udp_connection_of_L2L3 ,
		.set = NULL ,
	},
	{
		.name="sw_frag_of_L2L3" ,
		.get = dump_rg_sw_frag_of_L2L3 ,
		.set = flush_rg_sw_frag_of_L2L3 ,
	},
#if defined(CONFIG_RG_FLOW_4K_MODE)
	{
		.name="sw_flow_tcamList" ,
		.get = dump_rg_flow_tcamList ,
		.set = NULL ,
	},
#endif
	{
		.name="sw_flowHighPriTbl" ,
		.get = dump_rg_flowHiPri_table ,
		.set = NULL ,
	},

#endif // CONFIG_RG_FLOW_BASED_PLATFORM
	{
		.name="mac_whitelist" ,
		.get = dump_rg_mac_filter_whitelist ,
		.set = NULL ,
	},
	{
		.name="isakmpDB" ,
		.get = dump_isakmpDB ,
		.set = NULL ,
	},
	{
		.name="sw_icmp_connection" ,
		.get = dump_icmp_connection ,
		.set = NULL ,
	},
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	{
		.name="funcbasedMeter" ,
		.get = dump_rg_funcbasedMeter ,
		.set = dump_rg_funcbasedMeter_by_type ,
	},
	{
		.name="hostPocicing" ,
		.get = dump_rg_hostPolicing,
		.set = NULL,
	},
#endif

#if 0 // debug only,  for field index width of shortcut
	{
		.name="table_size" ,
		.get = dump_table_size ,
		.set = NULL ,
	},
#endif
	{
		.name="meminfo" ,
		.get = dump_meminfo ,
		.set = NULL ,
	},
	{
		.name="shareMeter" ,
		.get = dump_rg_shareMeter,
		.set = NULL ,
	},
	{
		.name="sw_shaper" ,
		.get = dump_swShaper_table,
		.set = NULL ,
	},
};


static int common_debug_single_open(struct inode *inode, struct file *file)
{
    int i,j,r=-1,match_idx=-1,smp_id=0;
#ifdef CONFIG_SMP
	smp_id=smp_processor_id();
#ifdef RG_BY_TASKLET
#else
	smp_id=rg_db.systemGlobal.smpRgCpuFromCpu[smp_id];
#endif
#endif
	rg_inbound_queue_lock(smp_id,&rg_kernel.rg_inbound_queue_lock);

    for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u\n",debugProc[i].inode_id,(unsigned int)inode->i_ino);
		for(j=0;j<RTK_RG_MAX_PROC_GROUP;j++)
		{
	        if(debugProc[i].inode_id[j]==(unsigned int)inode->i_ino)
	        {
				r=rtk_rg_single_open(file, debugProc[i].get, NULL);
				match_idx=i;
				break;
	        }
		}
		if(match_idx!=-1) break;
    }

	rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);

    return r;
}


static ssize_t common_debug_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    int i,r=-1,j,match_idx=-1,smp_id=0;
	char* cpybuf=rtk_rg_malloc(2048);

#ifdef CONFIG_SMP
	smp_id=smp_processor_id();
#ifdef RG_BY_TASKLET
#else
	smp_id=rg_db.systemGlobal.smpRgCpuFromCpu[smp_id];
#endif
#endif
	if(userbuf && cpybuf)
	{
		int ret = copy_from_user(cpybuf,userbuf,count);
		if(ret)
		{
			printk("copy_from_user warning ret=%d",ret);
			if(cpybuf) rtk_rg_free(cpybuf);
			return ret;
		}
		if(count >=2048 )
		{
			printk("size over Buf 2048");
			if(cpybuf) rtk_rg_free(cpybuf);
			return r;
		}
	}
	else
	{
		printk("userbuf/cpybuf NULL");
		if(cpybuf) rtk_rg_free(cpybuf);
		return r;
	}

	rg_inbound_queue_lock(smp_id,&rg_kernel.rg_inbound_queue_lock);

    for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_write inode_id=%u i_ino=%u\n",debugProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);
		for(j=0;j<RTK_RG_MAX_PROC_GROUP;j++)
		{
#if defined(CONFIG_RG_G3_SERIES)
			if(debugProc[i].inode_id[j]==(unsigned int)file->f_inode->i_ino)
#else
	        if(debugProc[i].inode_id[j]==(unsigned int)file->f_dentry->d_inode->i_ino)
#endif
	        {
				if(debugProc[i].unlockBefortWrite==1)
					rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);

				r=debugProc[i].set(file,cpybuf,count,off);
				match_idx=i;

				if(debugProc[i].unlockBefortWrite!=1)
					rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);

				break;
	        }
		}
		if(match_idx!=-1) break;
    }

	if(match_idx==-1)
		rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);

	if(cpybuf) rtk_rg_free(cpybuf);
    return r;
}



void rg_proc_init(void)
{

	struct proc_dir_entry *p;
	struct proc_dir_entry *proc_root=NULL;
	int32 i;

	proc_root = proc_mkdir("dump", NULL);


	for( i=0; i< (sizeof(debugProc)/sizeof(rtk_rg_proc_t)) ;i++)
	{
		if(debugProc[i].get==NULL)
			debugProc[i].proc_fops.open=NULL_proc_single_open;
		else
			debugProc[i].proc_fops.open=common_debug_single_open;

		if(debugProc[i].set==NULL)
			debugProc[i].proc_fops.write=NULL;
		else
			debugProc[i].proc_fops.write=common_debug_single_write;

		debugProc[i].proc_fops.read=rtk_rg_seq_read;
		debugProc[i].proc_fops.llseek=seq_lseek;
		debugProc[i].proc_fops.release=single_release;


		p = proc_create_data(debugProc[i].name, S_IRUGO, proc_root, &(debugProc[i].proc_fops),NULL);
		if(!p){
			printk("create proc dump/%s failed!\n",debugProc[i].name);
			continue;
		}
		debugProc[i].inode_id[0] = p->low_ino;
	}


	return ;

}
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

uint32 _rtk_rg_flow_hash(rtk_rg_pktHdr_t *pPktHdr)
{

	int i;
	int32 flowHashIdx=FAIL;
	uint8 _isGatewayPacket = 0;
	uint32 igrSVID, igrCVID, lutDaIdx_gmacChk;
	rtk_rg_table_flowEntry_t flowPathEntry;
	int32 smacL2Idx=FAIL, dmacL2Idx=FAIL;
		//check is bridge or routing
	for(i=0; i<MAX_NETIF_SW_TABLE_SIZE; i++)
	{
		if(rg_db.netif[i].rtk_netif.valid==0) continue;
		if(memcmp(pPktHdr->pDmac, rg_db.netif[i].rtk_netif.gateway_mac.octet, ETHER_ADDR_LEN)==0)
		{
			_isGatewayPacket=1;
			break;
		}
	}


	//get flow hash index
	if(pPktHdr->pRxDesc->rx_reason<RG_CPU_REASON_NORMAL_FWD && pPktHdr->pRxDesc->rx_fbi==1)
	{
		flowHashIdx = pPktHdr->pRxDesc->rx_fb_hash;
		flowHashIdx <<= MAX_FLOW_WAYS_SHIFT;
	}
	else
	{
		lutDaIdx_gmacChk = 0; //only path34 care
		igrSVID = (pPktHdr->tagif&SVLAN_TAGIF)?pPktHdr->stagVid:0;
		igrCVID = (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagVid:0;
		memset(&flowPathEntry, 0 , sizeof(rtk_rg_table_flowEntry_t));

		if(_isGatewayPacket)
			flowPathEntry.path1.in_path = FB_PATH_5;
		else
			flowPathEntry.path1.in_path = ((pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))&&(pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF))) ? FB_PATH_34 : FB_PATH_12;

		if(flowPathEntry.path1.in_path==FB_PATH_5)
		{
			flowPathEntry.path5.in_src_ipv4_addr	= (pPktHdr->tagif&IPV4_TAGIF)?pPktHdr->ipv4Sip:pPktHdr->ipv6Sip_hash;
			flowPathEntry.path5.in_dst_ipv4_addr	= (pPktHdr->tagif&IPV4_TAGIF)?pPktHdr->ipv4Dip:pPktHdr->ipv6Dip_hash;
			flowPathEntry.path5.in_l4_src_port		= pPktHdr->sport;
			flowPathEntry.path5.in_l4_dst_port		= pPktHdr->dport;
			flowPathEntry.path5.in_l4proto			= (pPktHdr->tagif&TCP_TAGIF)?1:0;
#if defined(CONFIG_RG_RTL9607C_SERIES)
			if(!(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
#endif
			{
				flowPathEntry.path5.in_cvlan_pri	= (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagPri:0;
				flowPathEntry.path5.in_tos			= pPktHdr->tos;
			}

		}
		else if(flowPathEntry.path1.in_path==FB_PATH_34)
		{
			flowPathEntry.path3.in_src_ipv4_addr	= (pPktHdr->tagif&IPV4_TAGIF)?pPktHdr->ipv4Sip:pPktHdr->ipv6Sip_hash;
			flowPathEntry.path3.in_dst_ipv4_addr	= (pPktHdr->tagif&IPV4_TAGIF)?pPktHdr->ipv4Dip:pPktHdr->ipv6Dip_hash;
			flowPathEntry.path3.in_l4_src_port		= pPktHdr->sport;
			flowPathEntry.path3.in_l4_dst_port		= pPktHdr->dport;
			flowPathEntry.path3.in_l4proto			= (pPktHdr->tagif&TCP_TAGIF)?1:0;
			flowPathEntry.path3.in_ipv4_or_ipv6 	= (pPktHdr->tagif&IPV6_TAGIF)?1:0;
#if defined(CONFIG_RG_RTL9607C_SERIES)
			if(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0)
			{

				if((((pPktHdr->tagif&IPV4_TAGIF)&&((pPktHdr->ipv4Dip&0xf0000000)==0xe0000000)) || ((pPktHdr->tagif&IPV6_TAGIF)&&((pPktHdr->pIpv6Dip[0])==0xff)))
				&& !((pPktHdr->tagif&PPPOE_TAGIF) && (pPktHdr->isGatewayPacket==0)))
				{

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
					if(pPktHdr->tagif & IPV4_TAGIF)
						ASSERT_EQ(rtl_igmp_lookupGroupLutIdx(pPktHdr->pIpv4Dip,0,&lutDaIdx_gmacChk),SUCCESS);
					else
						ASSERT_EQ(rtl_igmp_lookupGroupLutIdx((uint32*)pPktHdr->pIpv6Dip,1,&lutDaIdx_gmacChk),SUCCESS);
#elif defined(CONFIG_RG_G3_SERIES)

#endif
				}
				else
				{
					lutDaIdx_gmacChk						= _rtk_rg_macLookup(pPktHdr->pDmac, rg_db.systemGlobal.initParam.fwdVLAN_CPU, FALSE);
				}

				if(lutDaIdx_gmacChk>=MAX_LUT_HW_TABLE_SIZE)
				{
					TRACE("path3 but lut entry of dmac is not exist lutDaIdx_gmacChk=%d gotoEnd",lutDaIdx_gmacChk);
					goto FLOWIDX_END;
				}
			}else
#endif
			{
				lutDaIdx_gmacChk					= (_isGatewayPacket)?1:0;
				flowPathEntry.path3.in_cvlan_pri	= (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagPri:0;
				flowPathEntry.path3.in_tos			= pPktHdr->tos;
			}
		}
		else	// path12
		{
			// support shortcut of path1(only for SVL ingress cvlan)
			if(!((pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))&&(pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF))))
			{
				smacL2Idx = _rtk_rg_macLookup(pPktHdr->pSmac, rg_db.systemGlobal.initParam.fwdVLAN_CPU, FALSE);
				dmacL2Idx = _rtk_rg_macLookup(pPktHdr->pDmac, rg_db.systemGlobal.initParam.fwdVLAN_CPU, FALSE);
				if( (dmacL2Idx<0) ||
					(dmacL2Idx>=MAX_LUT_HW_TABLE_SIZE) ||
					(smacL2Idx<0) ||
					(smacL2Idx>=MAX_LUT_HW_TABLE_SIZE))
				{
					TRACE("[Path1 flow] smacL2Idx or dmacL2Idx is not found, goto gotoEnd");
					goto FLOWIDX_END;
				}
			}
			flowPathEntry.path1.in_smac_lut_idx = smacL2Idx;
			flowPathEntry.path1.in_dmac_lut_idx = dmacL2Idx;
#if defined(CONFIG_RG_RTL9607C_SERIES)
			if(!(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
#endif
			{
				flowPathEntry.path1.in_cvlan_pri	= (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagPri:0;
				flowPathEntry.path1.in_tos			= pPktHdr->tos;
			}
		}
		flowHashIdx = _rtk_rg_flow_hashIndex(flowPathEntry, igrSVID, igrCVID, lutDaIdx_gmacChk /*used by path3 & path4*/);
	}

FLOWIDX_END:
	DEBUG("flowHashIdx=%d\n", flowHashIdx);
	return flowHashIdx;
}
#endif

int _rtk_rg_trace_filter_compare(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	int32 i,j;
	int32 show=0;
	int32 da_hit=0,sa_hit=0;
#if 0
	rtk_rg_pktHdr_t *pPktHdr;
	pPktHdr=(rtk_rg_pktHdr_t *)(*(u32 *)(skb->data+skb->len));
	if((((u32)pPktHdr)&0xf0000000)!=0x80000000)
	{
		if((u32)pPktHdr!=0) //skip ARP,NB which send by fwdEngine
		{
			printk("FIXME: PKTHDR isn't put at end of skb.\n");
		}
		return 1;
	}
#endif

	if(skb==NULL || pPktHdr==NULL)
		return 1;

	for(i=0;i<TRACFILTER_MAX;i++)
	{
		da_hit=0;
		sa_hit=0;

		if(!(rg_kernel.traceFilterRuleMask & (1<<i)))
			continue;

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SPA)
		{
			if(rg_kernel.trace_filter[i].spa!=pPktHdr->ingressPort) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DA)
		{
			for(j=0;j<6;j++)
			{
				if((rg_kernel.trace_filter[i].dmac.octet[j]&rg_kernel.trace_filter[i].dmac_mask.octet[j])!=
					(skb->data[j]&rg_kernel.trace_filter[i].dmac_mask.octet[j]))
						da_hit=1;
			}
			if(da_hit)
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SA)
		{
			for(j=0;j<6;j++)
			{
				if((rg_kernel.trace_filter[i].smac.octet[j]&rg_kernel.trace_filter[i].smac_mask.octet[j])!=
					(skb->data[j+6]&rg_kernel.trace_filter[i].smac_mask.octet[j]))
						sa_hit=1;
			}
			if(sa_hit)
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_ETH)
		{
			if(rg_kernel.trace_filter[i].ethertype!=pPktHdr->etherType) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SIP)
		{
			if((pPktHdr->tagif&IPV4_TAGIF) || (pPktHdr->tagif&ARP_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF))
			{
				if(rg_kernel.trace_filter[i].sip!=pPktHdr->ipv4Sip)
					continue;
			}else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DIP)
		{
			if((pPktHdr->tagif&IPV4_TAGIF) || (pPktHdr->tagif&ARP_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF))
			{
				if(rg_kernel.trace_filter[i].dip!=pPktHdr->ipv4Dip)
					continue;
			}else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_IP)
		{
			if((pPktHdr->tagif&IPV4_TAGIF) || (pPktHdr->tagif&ARP_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF))
			{
				if((rg_kernel.trace_filter[i].ip!=pPktHdr->ipv4Dip)&&(rg_kernel.trace_filter[i].ip!=pPktHdr->ipv4Sip))
					continue;
			}else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_L4PROTO)
		{
			if((pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))==0)
				continue;
			else
				if(rg_kernel.trace_filter[i].l4proto!=pPktHdr->ipProtocol) continue;
		}
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_FLOW_CHANGE)
		{
			uint32 _flowHashIdx = _rtk_rg_flow_hash(pPktHdr);
			if(rg_kernel.trace_filter[i].flowIdxCh!=_flowHashIdx) continue;
		}
#endif
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SPORT)
		{
			if(rg_kernel.trace_filter[i].sport!=pPktHdr->sport) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DPORT)
		{
			if(rg_kernel.trace_filter[i].dport!=pPktHdr->dport) continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_REASON)
		{
			if(rg_kernel.trace_filter[i].reason!=pPktHdr->pRxDesc->rx_reason) continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_CVLAN)
		{
			if(pPktHdr->tagif&CVLAN_TAGIF)
			{
				if((pPktHdr->ctagVid!=rg_kernel.trace_filter[i].cvlanid))
					continue;
			}else
				continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SVLAN)
		{
			if(pPktHdr->tagif&SVLAN_TAGIF)
			{
				if(pPktHdr->stagVid!=rg_kernel.trace_filter[i].svlanid)
					continue;
			}else
				continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_PPPOESESSIONID)
		{
			if(pPktHdr->tagif&PPPOE_TAGIF)
			{
				if(pPktHdr->sessionId!=rg_kernel.trace_filter[i].sessionid)
					continue;
			}else
				continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_V6DIP)
		{
			if(((pPktHdr->tagif&IPV6_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF)) && pPktHdr->pIpv6Dip!=NULL)
			{
				if( !(memcmp(pPktHdr->pIpv6Dip,&(rg_kernel.trace_filter[i].dipv6[0]),16)==0) )
					continue;
			}
			else
				continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_V6SIP)
		{
			if(((pPktHdr->tagif&IPV6_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF)) && pPktHdr->pIpv6Sip!=NULL)
			{
				if( !(memcmp(pPktHdr->pIpv6Sip,&(rg_kernel.trace_filter[i].sipv6[0]),16)==0) )
					continue;
			}
			else
				continue;
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SIP_RANGE)
		{
			if((pPktHdr->tagif&IPV4_TAGIF) || (pPktHdr->tagif&ARP_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF))
			{
				if(rg_kernel.trace_filter[i].sip_lowbound > pPktHdr->ipv4Sip ||
					rg_kernel.trace_filter[i].sip_highbound < pPktHdr->ipv4Sip)
					continue;
			}else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_DIP_RANGE)
		{
			if((pPktHdr->tagif&IPV4_TAGIF) || (pPktHdr->tagif&ARP_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF))
			{
				if(rg_kernel.trace_filter[i].dip_lowbound > pPktHdr->ipv4Dip ||
					rg_kernel.trace_filter[i].dip_highbound < pPktHdr->ipv4Dip)
					continue;
			}else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_V6SIP_RANGE)
		{
			if(((pPktHdr->tagif&IPV6_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF)) && pPktHdr->pIpv6Sip!=NULL)
			{
				if (memcmp(pPktHdr->pIpv6Sip, &rg_kernel.trace_filter[i].sipv6_lowbound[0], 16) < 0 ||
					memcmp(pPktHdr->pIpv6Sip, &rg_kernel.trace_filter[i].sipv6_highbound[0], 16) > 0)
						continue;
			}
			else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_V6DIP_RANGE)
		{
			if(((pPktHdr->tagif&IPV6_TAGIF) || (pPktHdr->tagif&DSLITE_INNER_TAGIF)) && pPktHdr->pIpv6Dip!=NULL)
			{
				if (memcmp(pPktHdr->pIpv6Dip, &rg_kernel.trace_filter[i].dipv6_lowbound[0], 16) < 0 ||
					memcmp(pPktHdr->pIpv6Dip, &rg_kernel.trace_filter[i].dipv6_highbound[0], 16) > 0)
						continue;
			}
			else
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_LEN_RANGE)
		{
			if(rg_kernel.trace_filter[i].len_lowbound > skb->len ||
				rg_kernel.trace_filter[i].len_highbound < skb->len) 
				continue;
		}
		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_TCP_FLAGS_MSK)
		{
			if((pPktHdr->tagif&(TCP_TAGIF))==0)
				continue;
			else if(rg_kernel.trace_filter[i].tcp_flags_enable){
				uint16 value=0;
				value|=(pPktHdr->tcpFlags_high<<8);
				value|=(*(uint8 *)&pPktHdr->tcpFlags);
				value&=rg_kernel.trace_filter[i].tcp_flags_enable;
				if(rg_kernel.trace_filter[i].tcp_flags_value != value) continue;
			}
		}

		if(rg_kernel.trace_filter_bitmask[i]&RTK_RG_DEBUG_TRACE_FILTER_SHOWNUMBEROFTIMES)
		{
			if(rg_kernel.trace_filter[i].showNumberOfTimesCounter>= rg_kernel.trace_filter[i].showNumberOfTimes)
				continue;
			else
				rg_kernel.trace_filter[i].showNumberOfTimesCounter++;
		}


		show=1;
		break;
	}

	return show;
}




void dump_packet(u8 *pkt,u32 size,char *memo)
{
	int off;
	u8 protocol=0;
	int i;
	int pppoeif=0;
	u8 splitLine[80]={0};
	memset(splitLine,'=',79);
	rtlglue_printf("%s\n",splitLine);

	if(size==0)
	{
		rtlglue_printf("%s\npacket_length=0\n",memo);
		return;
	}

	rtlglue_printf("%s\n",memo);
	rtlglue_print_hex_dump("", DUMP_PREFIX_ADDRESS, 16, 1, pkt, size, 1);
	rtlglue_printf("\n" COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[0],pkt[1],pkt[2],pkt[3],pkt[4],pkt[5]
		,pkt[6],pkt[7],pkt[8],pkt[9],pkt[10],pkt[11]);
	off=12;
	if((pkt[off]==0x88)&&(pkt[off+1]==0x99))
	{

		if(((pkt[off+8]==0x88)&&(pkt[off+9]==0xa8))||((pkt[off+8]==0x81)&&(pkt[off+9]==0x00))||((pkt[off+8]==0x88)&&((pkt[off+9]==0x63)||(pkt[off+9]==0x64)))||
		((pkt[off+8]==0x86)&&(pkt[off+9]==0xdd))||((pkt[off]==0x08)&&(pkt[off+1]==0x00)))
		{
			//TO CPU
			rtlglue_printf("CPU:[" COLOR_Y "Protocol" COLOR_NM "=%d][" COLOR_Y "Res" COLOR_NM "=0x%x][" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "TTL_1" COLOR_NM "=0x%x][" COLOR_Y "L3R" COLOR_NM "=%d][" COLOR_Y "ORG" COLOR_NM "=%d][" COLOR_Y "SPA" COLOR_NM "=%d][" COLOR_Y "EPMSK" COLOR_NM "=0x%x]\n"
				,pkt[off+2],pkt[off+3],pkt[off+4]>>5,pkt[off+4]&0x1f
				,pkt[off+5]>>7,(pkt[off+5]>>6)&1,pkt[off+5]&7,pkt[off+7]&0x3f);
			off+=8;
		}
		else
		{
			//FROM CPU
			rtlglue_printf("CPU:[" COLOR_Y "Proto" COLOR_NM "=%d][" COLOR_Y "L3CS" COLOR_NM "=%d][" COLOR_Y "L4CS" COLOR_NM "=%d][" COLOR_Y "TxPortMask" COLOR_NM "=0x%x][" COLOR_Y "EFID_EN" COLOR_NM "=%d][" COLOR_Y "EFID" COLOR_NM "=%d][" COLOR_Y "Priority" COLOR_NM "=%d]\n"
				,pkt[off+2],(pkt[off+3]>>7)&1,(pkt[off+3]>>6)&1,pkt[off+3]&0x3f,pkt[off+4]>>5,(pkt[off+4]>>3)&3,pkt[off+4]&7);
			rtlglue_printf("    [" COLOR_Y "Keep" COLOR_NM "=%d][" COLOR_Y "VSEL" COLOR_NM "=%d][" COLOR_Y "DisLrn" COLOR_NM "=%d][" COLOR_Y "PSEL" COLOR_NM "=%d][" COLOR_Y "Rsv1" COLOR_NM "=%d][" COLOR_Y "Rsv0" COLOR_NM "=%d][" COLOR_Y "L34Keep" COLOR_NM "=%d][" COLOR_Y "QSEL" COLOR_NM "=%d]\n"
				,pkt[off+5]>>7,(pkt[off+5]>>6)&1,(pkt[off+5]>>5)&1,(pkt[off+5]>>4)&1,(pkt[off+5]>>3)&1,(pkt[off+5]>>2)&1,(pkt[off+5]>>1)&1,pkt[off+5]&1);
			rtlglue_printf("    [" COLOR_Y "ExtSPA" COLOR_NM "=%d][" COLOR_Y "PPPoEAct" COLOR_NM "=%d][" COLOR_Y "PPPoEIdx" COLOR_NM "=%d][" COLOR_Y "L2BR" COLOR_NM "=%d][" COLOR_Y "QID" COLOR_NM "=%d]\n"
				,(pkt[off+6]>>5)&7,(pkt[off+6]>>3)&3,pkt[off+6]&7,(pkt[off+7]>>7)&1,pkt[off+6]&0x7f);
			off+=12;
		}
	}

{
#ifdef CONFIG_RG_RTL9602C_SERIES
	uint32 vs_tpid=0;
	uint32 vs_tpid2=0;
	rtk_enable_t vs_tpid2_en=0;
	i = (pkt[off]<<8) | (pkt[off+1]);
	assert_ok(rtk_svlan_tpidEntry_get(0, &vs_tpid));
	assert_ok(rtk_svlan_tpidEntry_get(1, &vs_tpid2));
	assert_ok(rtk_svlan_tpidEnable_get(1, &vs_tpid2_en));
	if ((i==vs_tpid) || ((vs_tpid2_en)&&(i==vs_tpid2)))
#else
	if ((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
#endif
	{
		rtlglue_printf("SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
		off+=4;
	}
}
	if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
	{
		rtlglue_printf("CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
		off+=4;
	}

	if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
	{
		rtlglue_printf("PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x][" COLOR_Y "Len" COLOR_NM "=%d]\n",
			pkt[off+3],((u32)pkt[off+4]<<8)|pkt[off+5],((u32)pkt[off+6]<<8)|pkt[off+7]);
		off+=8;
		pppoeif=1;
	}

	if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)))		//IPv6 or IPv6 with PPPoE
	{
		rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%02x%02x%x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
			,pkt[off+2]>>4, (pkt[off+2]&0xf)+(pkt[off+3]>>4), (pkt[off+3]&0xf)+(pkt[off+4]>>4), (pkt[off+4]&0xf)+(pkt[off+5]>>4), (pkt[off+5]&0xf), (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
		rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
			,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
			,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
		rtlglue_printf("     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
			,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
			,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

		protocol=pkt[off+8];
		if(protocol==0)	//hop-by-hop
		{
			rtlglue_printf("Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
				,pkt[off+42], pkt[off+43]);
			rtlglue_printf("          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
				,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
			for(i=0; i<pkt[off+43]; i++)
			{
				rtlglue_printf("         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
					,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
					,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
			}

			protocol=pkt[off+42];
			off+=(50+pkt[off+43]*8);
		}
		else
			off+=42;
	}

	if(((pkt[off]==0x08)&&(pkt[off+1]==0x00))||((pkt[off]==0x00)&&(pkt[off+1]==0x21)))
	{
		rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=%d(DSCP=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
			,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
			,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
		rtlglue_printf("     [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
			,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
		rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
			,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

		protocol=pkt[off+11];
		off+=(pkt[off+2]&0xf)*4+2;
	}

	if(protocol==0x6) //TCP
	{
		rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
			,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
			,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
		rtlglue_printf("    [" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
			,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
			,(pkt[off+14]<<8)|pkt[off+15]);
		rtlglue_printf("    [" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
	}
	else if(protocol==0x11) //UDP
	{
		rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
			,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));

	}
}

void _rtk_rg_error_id_mapping(int id, char *mappingName)
{
	switch(id)
	{
		case RT_ERR_RG_ADD_ARP_MAC_FAILED: strcpy(mappingName,"RT_ERR_RG_ADD_ARP_MAC_FAILED"); break;
		case RT_ERR_RG_ARP_NOT_FOUND: strcpy(mappingName,"RT_ERR_RG_ARP_NOT_FOUND"); break;
		case RT_ERR_RG_CHIP_NOT_SUPPORT: strcpy(mappingName,"RT_ERR_RG_CHIP_NOT_SUPPORT"); break;
		case RT_ERR_RG_CPU_TAG_DIFF_BRIDGE_WAN: strcpy(mappingName,"RT_ERR_RG_CPU_TAG_DIFF_BRIDGE_WAN"); break;
		case RT_ERR_RG_CREATE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_ERR_RG_CREATE_GATEWAY_LUT_FAIL"); break;
		case RT_ERR_RG_DELETE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_ERR_RG_DELETE_GATEWAY_LUT_FAIL"); break;
		case RT_ERR_RG_INTF_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_INTF_SET_FAIL"); break;
		case RT_ERR_RG_INVALID_PARAM: strcpy(mappingName,"RT_ERR_RG_INVALID_PARAM"); break;
		case RT_ERR_RG_NXP_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_NXP_SET_FAIL"); break;
		case RT_ERR_RG_PPPOE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_PPPOE_SET_FAIL"); break;
		case RT_ERR_RG_PORT_BIND_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_PORT_BIND_SET_FAIL"); break;
		case RT_ERR_RG_ROUTE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_ROUTE_SET_FAIL"); break;
		case RT_ERR_RG_WANTYPE_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_WANTYPE_SET_FAIL"); break;
		case RT_ERR_RG_VLAN_SET_FAIL: strcpy(mappingName,"RT_ERR_RG_VLAN_SET_FAIL"); break;
		case RT_ERR_RG_VLAN_PRI_CONFLICT_WIFI: strcpy(mappingName,"RT_ERR_RG_VLAN_PRI_CONFLICT_WIFI"); break;
		default: strcpy(mappingName,"Please refer to rtk_rg_struct.h!"); break;
	}
}




extern char StringErrName[];






rtk_rg_err_code_t return_err_mapping(int err_id, char *err_name,int line)
{

	if(err_id!=RT_ERR_RG_OK)
	{
		if(memcmp(err_name,"RT_ERR_RG_",10)==0)
		{
			strcpy(StringErrName,err_name);
		}
		else
		{
			_rtk_rg_error_id_mapping(err_id,StringErrName);
		}
		WARNING("Return Error (0x%x:%s) at line:%d",err_id,StringErrName,line);
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN)
		{
			_rtk_rg_dump_stack();
		}
	}
	return err_id;
}

#define MAX_PROC_PRINT_SIZE	1024
char proc_print_buf[MAX_PROC_PRINT_SIZE];
char *proc_printf(struct seq_file *s, char *fmt, ...)
{
    int n;
    int size = MAX_PROC_PRINT_SIZE;     /* Guess we need no more than 512 bytes */
    va_list ap;

    while (1) {
        va_start(ap, fmt);
        n = vsnprintf(proc_print_buf, size, fmt, ap);

		if((s==NULL)||(rg_db.systemGlobal.proc_to_pipe==0))
			{printk("%s",proc_print_buf);}
		else
			{seq_puts(s,proc_print_buf);}

        va_end(ap);

		if (n < 0)
		    return NULL;

		if (n < size)
		    return proc_print_buf;

		size = n + 1;

    }
	return NULL;
}

//#define PROC_PRINTF( comment ,arg...)	do { if((s==NULL)||(rg_db.systemGlobal.proc_to_pipe==0)) {printk(comment,##arg);} else {seq_printf(s,comment,##arg);} }while(0)
#endif
void _rtk_rg_rtlglue_printf(char* fmt, ...)
{
	int len;
	va_list aList;
	char tty_display_char_temp[512];
	struct tty_struct *display_tty = NULL;
	struct task_struct *task_temp = NULL;

	va_start(aList, fmt);
	vsprintf(tty_display_char_temp, fmt, aList);
	va_end (aList);

	if(rg_db.systemGlobal.debug_message_display_to_tty == RTK_RG_ENABLED)
	{
		/*The used /bin/sh process doesn't exist anymore or the tty structure of the used /bin/sh process is NULL*/
		task_temp = pid_task(find_vpid(rg_db.systemGlobal.tty_display_cur_sh_process_pid), PIDTYPE_PID);
		if((task_temp != NULL) && (strncmp(task_temp->comm, "sh", 2) == 0))
		{
			if((rg_db.systemGlobal.tty_display_cur_sh_process != NULL) && (rg_db.systemGlobal.tty_display_cur_sh_process->signal != NULL))
				display_tty = rg_db.systemGlobal.tty_display_cur_sh_process->signal->tty;

			if(display_tty != NULL)
			{
				//remove the '\n' character  at the end of the string
				len = strlen(tty_display_char_temp);
				if(len >= 512)
				{
					tty_display_char_temp[511]= '\0';
				}
				else
				{
					if(tty_display_char_temp[len-1] == '\n')
						tty_display_char_temp[len-1]=' ';
				}

				//wite to tty
				(display_tty->driver->ops->write)(
				display_tty,	  // The tty itself
				tty_display_char_temp,				 // String
				strlen(tty_display_char_temp)); 	 // Length

				(display_tty->driver->ops->write)(display_tty, "\015\012", 2);
				return;
			}
		}

		printk("\033[1;33;40m[DEBUG] Unable to print debug message to current tty, turn off proc/fc/trace/debug_message_display_to_current_tty automatically!\033[1;30;40m @ %s(%d)\033[0m\n",__FUNCTION__,__LINE__); //Can not use DEBUG here (because it call _rtk_rg_fc_rtlglue_printf)
		rg_db.systemGlobal.tty_display_cur_sh_process = NULL;
		rg_db.systemGlobal.debug_message_display_to_tty = RTK_RG_DISABLED;
	}

	//use printk to print debug message
	printk(tty_display_char_temp);

	return;
}


