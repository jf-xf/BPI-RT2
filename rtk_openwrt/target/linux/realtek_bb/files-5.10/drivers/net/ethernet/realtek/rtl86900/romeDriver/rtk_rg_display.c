
/*
 * Include Files
 */
#ifdef __KERNEL__
#include <osal/lib.h>
#include <common/error.h>
#include <common/rt_type.h>
//#include <rtk_rg_liteRomeDriver.h>
//#include <rtk_rg_apolloPro_asicDriver.h>
#include <rtk_rg_internal.h>
#else
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <math.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <rtdrv/rtdrv_rg_netfilter.h>
#include <rtdrv/rtdrv_netfilter.h>
#define rtlglue_printf printf
#endif

#include <rtk/l2.h>

#if defined(CONFIG_RG_G3_SERIES)
#include "aal_l3_cls.h"
#endif

//#include <rtk/port.h>

/*
#ifdef CONFIG_APOLLO_RLE0371
#include <hal/chipdef/apollo/apollo_def.h>
#else
#include <hal/chipdef/apollomp/apollomp_def.h>
#endif
*/

//#include <rtk_rg_debug.h>

char *name_of_rg_netifPPPoEAct[]={ //mappint to rtk_rg_acl_cvlan_tagif_decision_t
	"Keep",
	"Add",
	"Modify",
	"Remove",
};

int32 dump_lut_display(uint32 idx, rtk_l2_addr_table_t *data)
{
#if defined(CONFIG_RG_RTL9600_SERIES)
		rtlglue_printf("--------------- LUT TABLE (%d)----------------\n",idx);
#else	//support lut traffic bit
		if(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)
			rtlglue_printf("--------------- LUT TABLE (%d)----------------[Idle %d Secs], [arpRefCount: %d]\n",idx, rg_db.lut[idx].idleSecs, rg_db.lut[idx].arp_refCount);
		else
			rtlglue_printf("--------------- LUT TABLE (%d)----------------[Idle %d Secs]\n",idx, rg_db.lut[idx].idleSecs);
#endif

        rtlglue_printf("LUT idx=%d  Group idx=%d\n",idx, rg_db.lut[idx].lutGroupIdx);
        if(data->entryType==RTK_LUT_L2UC)
        {
 #if defined(CONFIG_RG_RTL9600_SERIES)
            rtlglue_printf("[P1] mac=%02x:%02x:%02x:%02x:%02x:%02x cvid=%d l3lookup=%d ivl=%d\n"
                           ,data->entry.l2UcEntry.mac.octet[0]
                           ,data->entry.l2UcEntry.mac.octet[1]
                           ,data->entry.l2UcEntry.mac.octet[2]
                           ,data->entry.l2UcEntry.mac.octet[3]
                           ,data->entry.l2UcEntry.mac.octet[4]
                           ,data->entry.l2UcEntry.mac.octet[5]
                           ,data->entry.l2UcEntry.vid
                           ,0
                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL)?1:0);
#else	// support ctag_if
			rtlglue_printf("[P1] mac=%02x:%02x:%02x:%02x:%02x:%02x cvid=%d ctagif=%d l3lookup=%d ivl=%d\n"
	                           ,data->entry.l2UcEntry.mac.octet[0]
	                           ,data->entry.l2UcEntry.mac.octet[1]
	                           ,data->entry.l2UcEntry.mac.octet[2]
	                           ,data->entry.l2UcEntry.mac.octet[3]
	                           ,data->entry.l2UcEntry.mac.octet[4]
	                           ,data->entry.l2UcEntry.mac.octet[5]
	                           ,data->entry.l2UcEntry.vid
	                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0
	                           ,0
	                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL)?1:0);
#endif

#if defined(CONFIG_RG_RTL9600_SERIES)
			rtlglue_printf("efid=%d sapri_en=%d fwdpri_en=%d lutpri=%d\n",
							data->entry.l2UcEntry.efid,
							(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_LOOKUP_PRI)?1:0,
							(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI)?1:0,
							data->entry.l2UcEntry.priority);
#endif
            rtlglue_printf("fid=%d spa=%d age=%d auth1x=%d sablock=%d\n"
                           ,data->entry.l2UcEntry.fid
                           ,data->entry.l2UcEntry.port
                           ,data->entry.l2UcEntry.age
                           ,data->entry.l2UcEntry.auth
                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0);

            rtlglue_printf("dablock=%d ext_spa=%d arp_used=%d static=%d\n"
                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0
                           ,data->entry.l2UcEntry.ext_port
                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)?1:0
                           ,(data->entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0);

        }
        else if(data->entryType==RTK_LUT_L2MC)
        {
            rtlglue_printf("[P2] mac=%02x:%02x:%02x:%02x:%02x:%02x ivl=%d vid=%d fid=%d l3lookup=%d\n"
                           ,data->entry.l2McEntry.mac.octet[0]
                           ,data->entry.l2McEntry.mac.octet[1]
                           ,data->entry.l2McEntry.mac.octet[2]
                           ,data->entry.l2McEntry.mac.octet[3]
                           ,data->entry.l2McEntry.mac.octet[4]
                           ,data->entry.l2McEntry.mac.octet[5]
                           ,(data->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_IVL)?1:0
                           ,data->entry.l2McEntry.vid
                           ,data->entry.l2McEntry.fid
                           ,0);

            rtlglue_printf("mbr=0x%x extmbr=0x%x static=%d\n"
                           ,data->entry.l2McEntry.portmask.bits[0]
                           ,data->entry.l2McEntry.ext_portmask.bits[0],(data->entry.l2McEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0);

#if defined(CONFIG_RG_RTL9600_SERIES)
            rtlglue_printf("lutpri=%d fwdpri_en=%d\n"
                           ,data->entry.l2McEntry.priority
                           ,(data->entry.l2McEntry.flags&RTK_L2_MCAST_FLAG_FWD_PRI)?1:0);
#endif

        }
        else if(data->entryType==RTK_LUT_L3MC)
        {

#if defined(CONFIG_RG_RTL9602C_SERIES)

			{
				rtlglue_printf("[P3] gip=%d.%d.%d.%d\n",((data->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data->entry.ipmcEntry.dip>>16)&0xff,(data->entry.ipmcEntry.dip>>8)&0xff,(data->entry.ipmcEntry.dip)&0xff);
				rtlglue_printf("sip=%d.%d.%d.%d %s%d\n",(data->entry.ipmcEntry.sip>>24)&0xff,(data->entry.ipmcEntry.sip>>16)&0xff,(data->entry.ipmcEntry.sip>>8)&0xff,(data->entry.ipmcEntry.sip)&0xff,
					(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?"IVL VID=":"SVL FID=",
					(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IVL)?data->entry.ipmcEntry.vid:data->entry.ipmcEntry.fid);
				rtlglue_printf("mbr=0x%x extmbr=0x%x\n",data->entry.ipmcEntry.portmask.bits[0],data->entry.ipmcEntry.ext_portmask.bits[0]);
				rtlglue_printf("l3_trans_index = %x sip_index=%d \n",data->entry.ipmcEntry.l3_trans_index,data->entry.ipmcEntry.sip_index);
				rtlglue_printf("sip_filter_en=%d static=%d ipv6=%d  wan_sa=%d\n",(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0,
					(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0,
					(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_IPV6)?1:0,
					(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0 );

			}



#elif defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
            if(!(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY))
            {
                rtlglue_printf("[P3] gip=%d.%d.%d.%d\n",((data->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data->entry.ipmcEntry.dip>>16)&0xff,(data->entry.ipmcEntry.dip>>8)&0xff,(data->entry.ipmcEntry.dip)&0xff);
                rtlglue_printf("sip=%d.%d.%d.%d cvid=%d\n",(data->entry.ipmcEntry.sip>>24)&0xff,(data->entry.ipmcEntry.sip>>16)&0xff,(data->entry.ipmcEntry.sip>>8)&0xff,(data->entry.ipmcEntry.sip)&0xff,data->entry.ipmcEntry.vid);
                rtlglue_printf("mbr=0x%x extmbr=0x%x\n",data->entry.ipmcEntry.portmask.bits[0],data->entry.ipmcEntry.ext_portmask.bits[0]);
                rtlglue_printf("lutpri=%d fwdpri_en=%d\n"
                               ,data->entry.ipmcEntry.priority
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0);
            }
            else
            {
                rtlglue_printf("[P4] gip=%d.%d.%d.%d\n",((data->entry.ipmcEntry.dip>>24)&0xff)|0xe0,(data->entry.ipmcEntry.dip>>16)&0xff,(data->entry.ipmcEntry.dip>>8)&0xff,(data->entry.ipmcEntry.dip)&0xff);
                rtlglue_printf("mbr=0x%x extmbr=0x%x l3trans=0x%x\n",data->entry.ipmcEntry.portmask.bits[0],data->entry.ipmcEntry.ext_portmask.bits[0],data->entry.ipmcEntry.l3_trans_index);
                rtlglue_printf("lutpri=%d fwdpri_en=%d dip_only=%d ext_fr=%d wan_sa=%d static=%d\n"
                               ,data->entry.ipmcEntry.priority
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FWD_PRI)?1:0
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_DIP_ONLY)?1:0
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_FORCE_EXT_ROUTE)?1:0
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0
                               ,(data->entry.ipmcEntry.flags&RTK_L2_IPMCAST_FLAG_STATIC)?1:0
                              );
            }
#endif

        }
#if defined(CONFIG_RG_RTL9602C_SERIES)
		else if (data->entryType==RTK_LUT_L3V6MC)
		{


				rtlglue_printf("[P4] v6ip= ---%02x(dip117_112)%02x(dip111_104)-----%02x(dip43_40)%02x:%02x%02x:%02x%02x \n",
					data->entry.ipmcEntry.dip6.ipv6_addr[1]&0x3f,data->entry.ipmcEntry.dip6.ipv6_addr[2],data->entry.ipmcEntry.dip6.ipv6_addr[10]&0xf,
					data->entry.ipmcEntry.dip6.ipv6_addr[11],data->entry.ipmcEntry.dip6.ipv6_addr[12],data->entry.ipmcEntry.dip6.ipv6_addr[13],
					data->entry.ipmcEntry.dip6.ipv6_addr[14],data->entry.ipmcEntry.dip6.ipv6_addr[15]);
				rtlglue_printf("mbr=0x%x  ext_mbr=0x%x	l3mcr_idx=%d \n", data->entry.ipmcEntry.portmask.bits[0],data->entry.ipmcEntry.ext_portmask.bits[0],data->entry.ipmcEntry.l3_mcr_index);
				rtlglue_printf("notsalearn=%d  l3lookup=%d  ip6=%d  wan_sa=%d\n",
					(data->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_STATIC)?1:0,1,
					(data->entry.ipmcEntry.flags &RTK_L2_IPMCAST_FLAG_IPV6)?1:0  ,
					(data->entry.ipmcEntry.flags & RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN)?1:0);

		}
#endif
	//sw field
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support host policy
	if(rg_db.lut[idx].host_idx_valid)
		rtlglue_printf("Host idx(for logging)=%d  ", rg_db.lut[idx].host_idx);
	else
		rtlglue_printf("Host idx(for logging)=invalid  ");
	rtlglue_printf("host_dmac_rateLimit(for policing)=%d\n", rg_db.lut[idx].host_dmac_rateLimit);
	rtlglue_printf("mcStaticRefCnt=%d\n", rg_db.lut[idx].mcStaticRefCnt);
#endif
	rtlglue_printf("wlan_device_idx=%d\n", rg_db.lut[idx].wlan_device_idx);
#if defined(CONFIG_RG_G3_SERIES)
	rtlglue_printf("hashLiteIdx=%d\n", rg_db.lut[idx].hashLiteIdx);
#endif
	rtlglue_printf("countingInLearningLimit=%d\n", rg_db.lut[idx].countingInLearningLimit);
	if(rg_db.lut[idx].redirect_http_req){
		if(rg_db.lut[idx].redirect_http_req==1)rtlglue_printf("TRAP %sHTTP[Def]\n",rg_db.systemGlobal.forcePortal_url_list[0].manually_clear_req?"":"FIRST ");	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
		else if(rg_db.lut[idx].redirect_http_req==0xFF)rtlglue_printf("DROP HTTP\n");
		else rtlglue_printf("TRAP FIRST HTTP[%d]\n",rg_db.lut[idx].redirect_http_req-2);	//type value 0 stands for disable, 1 stands for default URL, 2 stands for type0, 3 stands for type 1,...
	}

	return SUCCESS;
}


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int32 display_netif(uint32 idx, rtk_rg_asic_netif_entry_t *intf, rtk_rg_asic_netifMib_entry_t *intfMib)
{
	uint8	*mac;

	if (intf->valid)
	{
		mac = (uint8 *)&intf->gateway_mac_addr.octet[0];
		rtlglue_printf("  [%d]- %02x:%02x:%02x:%02x:%02x:%02x    IP: %d.%d.%d.%d\n",
			idx, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
			(intf->gateway_ipv4_addr>>24)&0xff, (intf->gateway_ipv4_addr>>16)&0xff, (intf->gateway_ipv4_addr>>8)&0xff, intf->gateway_ipv4_addr&0xff);
		rtlglue_printf("      MTU check: %d, MTU %d Bytes\n", intf->intf_mtu_check, intf->intf_mtu);
		rtlglue_printf("      IgrAct: %d, EgrAct: %d, denyv4: %d, denyv6: %d\n", intf->ingress_action, intf->egress_action, intf->deny_ipv4, intf->deny_ipv6);
		rtlglue_printf("      Allow IgrPMask: 0x%x, IgrExtPmask: 0x%x\n", intf->allow_ingress_portmask.bits[0], intf->allow_ingress_ext_portmask.bits[0]);
		rtlglue_printf("      Out PPPoE Act: %d(%s), sid: 0x%x\n", intf->out_pppoe_act, name_of_rg_netifPPPoEAct[intf->out_pppoe_act], intf->out_pppoe_sid);

		rtlglue_printf("      Igr pkt count : uc(%u), mc(%u), bc(%u)\n", intfMib->in_intf_uc_packet_cnt, intfMib->in_intf_mc_packet_cnt, intfMib->in_intf_bc_packet_cnt);
		rtlglue_printf("      Igr byte count: uc(%llu), mc(%llu), bc(%llu)\n", intfMib->in_intf_uc_byte_cnt, intfMib->in_intf_mc_byte_cnt, intfMib->in_intf_bc_byte_cnt);

		rtlglue_printf("      Egr pkt count : uc(%u), mc(%u), bc(%u)\n", intfMib->out_intf_uc_packet_cnt, intfMib->out_intf_mc_packet_cnt, intfMib->out_intf_bc_packet_cnt);
		rtlglue_printf("      Egr byte count: uc(%llu), mc(%llu), bc(%llu)\n", intfMib->out_intf_uc_byte_cnt, intfMib->out_intf_mc_byte_cnt, intfMib->out_intf_bc_byte_cnt);

		rtlglue_printf("\n\n");
	}
	return SUCCESS;
}
#elif defined(CONFIG_RG_G3_SERIES)
int32 display_netif(uint32 idx, struct ca_int_l3_intf_s *genericIntf, ca_l3_intf_stats_t *intfMib)
{
	rtlglue_printf("[%u] ", genericIntf->intf.intf_id);
	if((genericIntf->intf.intf_id - NETIF_START_IDX) < (MAX_LAN_INTERFACE_SIZE * MAX_GENERIC_INTERFACE_PER_LAN))
	{
		//LAN
		if(((genericIntf->intf.intf_id - NETIF_START_IDX) % MAX_GENERIC_INTERFACE_PER_LAN) + NETIF_START_IDX < GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT)
			rtlglue_printf(" [LAN]\n", genericIntf->intf.intf_id);
		else
			rtlglue_printf(" [WLAN]\n", genericIntf->intf.intf_id);
	}
	else
	{
		//WAN
		rtlglue_printf(" [WAN]\n", genericIntf->intf.intf_id);
	}

	rtlglue_printf("      port_id = 0x%x\n", genericIntf->intf.port_id);
	rtlglue_printf("      mac_addr = %pM\n", genericIntf->intf.mac_addr);
	rtlglue_printf("      tpid = 0x%04x\n", genericIntf->intf.outer_tpid);
	rtlglue_printf("      vid = %u\n", genericIntf->intf.outer_vid);
	rtlglue_printf("      mtu = %u\n", genericIntf->intf.mtu);


	//Only rounting WAN will set IP
	if (genericIntf->intf.ip_addr.afi == CA_IPV4)
		rtlglue_printf("      ip_addr = %pI4h / %u\n", &genericIntf->intf.ip_addr.ip_addr.ipv4_addr, genericIntf->intf.ip_addr.addr_len);
	else
		rtlglue_printf("      ip_addr = %pI6h / %u\n",	genericIntf->intf.ip_addr.ip_addr.ipv6_addr, genericIntf->intf.ip_addr.addr_len);

	rtlglue_printf("      nat_enable = %u\n",  genericIntf->intf.nat_enable);
	rtlglue_printf("      tunnel_id = %u\n", genericIntf->intf.tunnel_id);

	rtlglue_printf("      L3_CLS info (dev / type / fibmode / tblIdx / keyIdx) :\n");
	if(genericIntf->cls_idx_uc5tuple == CA_UINT16_INVALID)
		rtlglue_printf("         >> 5_tuple N/A\n");
	else
		rtlglue_printf("         >> 5_tuple %d/%d/%d/%d/%d\n",
			CLS_DEV_ID(genericIntf->cls_idx_uc5tuple), CLS_KEY_TYPE(genericIntf->cls_idx_uc5tuple), CLS_FIB_MODE(genericIntf->cls_idx_uc5tuple), CLS_TBL_ID(genericIntf->cls_idx_uc5tuple), CLS_KEY_ID(genericIntf->cls_idx_uc5tuple));

	if(genericIntf->cls_idx_uc2tuple == CA_UINT16_INVALID)
		rtlglue_printf("         >> 2_tuple N/A\n");
	else
		rtlglue_printf("         >> 2_tuple %d/%d/%d/%d/%d\n",
			CLS_DEV_ID(genericIntf->cls_idx_uc2tuple), CLS_KEY_TYPE(genericIntf->cls_idx_uc2tuple), CLS_FIB_MODE(genericIntf->cls_idx_uc2tuple), CLS_TBL_ID(genericIntf->cls_idx_uc2tuple), CLS_KEY_ID(genericIntf->cls_idx_uc2tuple));

	if(genericIntf->cls_idx_mc == CA_UINT16_INVALID)
		rtlglue_printf("         >> mc_tuple N/A\n");
	else
		rtlglue_printf("         >> mc_tuple %d/%d/%d/%d/%d\n",
			CLS_DEV_ID(genericIntf->cls_idx_mc), CLS_KEY_TYPE(genericIntf->cls_idx_mc), CLS_FIB_MODE(genericIntf->cls_idx_mc), CLS_TBL_ID(genericIntf->cls_idx_mc), CLS_KEY_ID(genericIntf->cls_idx_mc));

	rtlglue_printf("      Igr pkt count: %llu, drop count: %llu, csum err: %llu\n", intfMib->rx_pkts, intfMib->rx_droppkts, intfMib->rx_csum_err);
	rtlglue_printf("      Igr byte count: %llu  drop byte count: %llu\n", intfMib->rx_bytes + (4 * intfMib->rx_pkts)/*CRC*/, intfMib->rx_dropbytes + (4 * intfMib->rx_droppkts)/*CRC*/);
	rtlglue_printf("      Egr pkt count: %llu, drop count: %llu\n", intfMib->tx_pkts, intfMib->tx_droppkts);
	rtlglue_printf("      Egr byte count: %llu  drop byte count: %llu\n", intfMib->tx_bytes + (4 * intfMib->tx_pkts)/*CRC*/, intfMib->tx_dropbytes + (4 * intfMib->tx_droppkts)/*CRC*/);

	return SUCCESS;
}
#endif



