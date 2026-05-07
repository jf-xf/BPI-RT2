#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_fcc.h>
#include <rtk_rg_alg_tool.h>

static int _rtk_rg_fcc_process(int direct, unsigned char * pData, unsigned int dataLen,rtk_rg_alg_connection_t * pConn)
{
	int ret,off=0;
	unsigned short *pFCCClientPort,*pFCCSignalPort,*pFCCMediaPort;
	unsigned int *pFCCIPAddr;
	unsigned char FCC_FMT,FCI_TYPE;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_upnpConnection_t upnpConn;

	pPktHdr = (rtk_rg_pktHdr_t *)pConn->pPktHdr;
	skb = (struct sk_buff *)pConn->skb;

	//dump_packet(pData,dataLen,"fcc");

	//check if it match RTCP format:V=2,P=0,PT=205,SSRC of packet sender=0
	ALG("pData %x %x %x %x %x %x",*pData,*(pData+1),*(pData+4),*(pData+5),*(pData+6),*(pData+7));
	if(((pData[off]&0x80)==0) || (pData[off+1]!=0xcd) || pData[off+4] || pData[off+5] || pData[off+6] || pData[off+7]){
		return FAIL;
	}

	FCC_FMT=pData[off]&0x1f;
	ALG("FCC_FMT is %x",FCC_FMT);
	off+=12;	//pointer to FCI

	if(direct==NAPT_DIRECTION_OUTBOUND){
		//lookup for fcc request packet
		if(FCC_FMT==ALG_FCC_FMT_REQUEST && (pData[off]==0) && (pData[off+1]==0) && (pData[off+2]==0) && (pData[off+3]==0)){
			pFCCClientPort=(u16*)&pData[off+4];
			ALG("[FCC request] original port is %d, new port is %d",ntohs(*pFCCClientPort),ntohs(*pPktHdr->pSport));
			pConn->app.fcc.mediaReceivePort=ntohs(*pFCCClientPort);
			*pFCCClientPort=*pPktHdr->pSport;
			
		}
	}else{
		//lookup for fcc response packet, result=0
		if(FCC_FMT==ALG_FCC_FMT_RESPONSE && pData[off]==0){
			FCI_TYPE=pData[off+1];
			if(FCI_TYPE==ALG_FCC_TYPE_NORMAL){
				pFCCMediaPort=(u16*)&pData[off+4];
				pFCCIPAddr=(u32*)&pData[off+8];
				ALG("[FCC response] media data will be sent from %pI4:%d, create upnp connection here.",pFCCIPAddr,ntohs(*pFCCMediaPort));
				//Add a one-shot UPnP flow for incoming connection to L4 WAN
				upnpConn.is_tcp=pConn->tuple.isTcp;
				upnpConn.wan_intf_idx=pPktHdr->srcNetifIdx;
				upnpConn.gateway_port=pPktHdr->dport;	//original dport
				upnpConn.local_ip=pConn->tuple.internalIp.ip;
				upnpConn.local_port=pConn->app.fcc.mediaReceivePort;
				upnpConn.limit_remote_ip=1;
				upnpConn.limit_remote_port=1;		//can't restrict client using which port to connect
				upnpConn.remote_ip=ntohl(*pFCCIPAddr);
				upnpConn.remote_port=ntohs(*pFCCMediaPort);
				upnpConn.type=UPNP_TYPE_ONESHOT;
				upnpConn.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_FCC_UDP];	//auto time out if the server do not connect to this WAN
				assert_ok((pf.rtk_rg_upnpConnection_add)(&upnpConn,&ret));
			}else if(FCI_TYPE==ALG_FCC_TYPE_REDIRECT){
				pFCCSignalPort=(u16*)&pData[off+2];
				pFCCIPAddr=(u32*)&pData[off+8];
				ALG("[FCC response] redirect to %pI4:%d, add ALG dynamic port here.",pFCCIPAddr,ntohs(*pFCCSignalPort));
				assert_ok(_rtk_rg_algDynamicPort_set(rtk_rg_algRegFunc_fcc,0,0, ntohs(*pFCCSignalPort),0,rg_db.systemGlobal.udp_short_timeout));
			}
		}		
	}
	return 1;
}

int _rtk_rg_fcc_handler(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo, unsigned char * pConnInfo)
{
	int ret = SUCCESS;
#ifdef __KERNEL__
	unsigned char * pData, *pAppData;
	unsigned int appLen=0,dataOff=0;
	rtk_rg_pktHdr_t *pPktHdr;
	struct sk_buff *skb;
	rtk_rg_alg_connection_t * pConn;

	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	pConn = (rtk_rg_alg_connection_t *)pConnInfo;
	skb = (struct sk_buff *)pSkb;

	pData = skb->data;
	dataOff = pPktHdr->l4Offset + 8; /*udp header length is 8 bytes*/

	appLen = skb->len - dataOff;
	pAppData = pData + dataOff;

	//fcc command length is at least 12 bytes:
	if (appLen < 12)
		return FAIL;

	//do nothing before napt modification
	if(after == 0)
		return SUCCESS;

	if(direct==NAPT_DIRECTION_OUTBOUND)
	{
		if(pConn->tuple.isIp6==0)
		{
			pConn->tuple.extIp.ip = ntohl(*pPktHdr->pIpv4Sip);
			pConn->tuple.extPort = ntohs(*pPktHdr->pSport);
		}
	}

	pConn->skb=pSkb;
	pConn->pPktHdr=pPktInfo;

	ret = _rtk_rg_fcc_process(direct, pAppData, appLen, pConn);
	if(ret < 1)
		return FAIL;
#endif
	return ret;
}

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_fcc(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo)
{
//Attention: caller function needs to make sure it needs to do napt modification
//ipv6 address and port doesn't need to do napt modification now
#ifdef __KERNEL__
	int ret;
	rtk_rg_pktHdr_t *pPktHdr;
	rtk_rg_alg_connection_t * pConn;
	rtk_rg_alg_tuple_t tuple;
	pPktHdr = (rtk_rg_pktHdr_t *)pPktInfo;
	memset(&tuple, 0, sizeof(rtk_rg_alg_tuple_t));

	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);

			pConn = _rtk_rg_alg_connection_find(&tuple);
			if(pConn == NULL)
				pConn = _rtk_rg_alg_connection_add(&tuple);
		}
	}else{
		//Post function
		_rtk_rg_alg_init_tuple(direct, after, pPktHdr, &tuple);

		pConn = _rtk_rg_alg_connection_find(&tuple);
		if(pConn == NULL)
			return RG_FWDENGINE_ALG_RET_FAIL;

		ret = _rtk_rg_fcc_handler(direct, after, pSkb, pPktInfo, (unsigned char *)pConn);
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}
