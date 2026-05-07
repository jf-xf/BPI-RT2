#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_tftp.h>

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_tftp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	int ret;
	int newPort;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_upnpConnection_t upnpConn;
	pPktHdr=(rtk_rg_pktHdr_t *)pPktInfo;

	if(after && direct==NAPT_DIRECTION_OUTBOUND){
		newPort=_rtk_rg_extPort_get(FALSE, pPktHdr->ipv4Sip, pPktHdr->sport, pPktHdr->sport);	//choose another port for UPNP
		if(newPort==FAIL) return RG_FWDENGINE_ALG_RET_SUCCESS;

		//20161223LUKE: We should update checksum with sport here.
		//20210202LUKE: for UDP has checksum zero means no-checksum, therefore recalculation is unnecessary.
		if(*pPktHdr->pL4Checksum>0)
			*pPktHdr->pL4Checksum=htons(_rtk_rg_fwdengine_L4checksumUpdateForMss(ntohs(*pPktHdr->pL4Checksum),ntohs(*pPktHdr->pSport),newPort));
		//Modify packet's sport to newPort
		*pPktHdr->pSport=htons(newPort);
		
		//Add a one-shot UPnP flow for incoming connection
		bzero(&upnpConn,sizeof(rtk_rg_upnpConnection_t));
		upnpConn.is_tcp=0;
		upnpConn.wan_intf_idx=pPktHdr->netifIdx;
		upnpConn.gateway_port=newPort;
		upnpConn.local_ip=pPktHdr->ipv4Sip;
		upnpConn.local_port=pPktHdr->sport;
		upnpConn.limit_remote_ip=1;
		upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
		upnpConn.remote_ip=pPktHdr->ipv4Dip;
		upnpConn.remote_port=0;
		upnpConn.type=UPNP_TYPE_ONESHOT;
		upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_TFTP_UDP];	//auto time out if the server do not connect to this WAN
		assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}
