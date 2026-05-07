#include "igmpproxy.h"
#include <sys/time.h>
#ifdef _SUPPORT_IGMPV3_


//#define IGMPV3LOG	printf
#define IGMPV3LOG(x,...)	if (Log2Stderr>x) printf(__VA_ARGS__)
#define IGMPV3_MAX_SRCNUM	64

__u32 gsrctmp[IGMPV3_MAX_SRCNUM];

#ifdef MULTICAST_LANWAN_BIND
extern struct IfDesc *bind_up[MAXWAN];
extern int bind_up_num;
#endif

int igmpv3_query( struct mcft_entry *entry, int srcnum, __u32 *srcfilter );
void  setIGMPVersionToV2(void);
extern int IGMPCurrentVersion;
extern int fast_leave;
struct src_entry *add_to_srclist(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry *p;
	struct in_addr tmp_addr;
	
	if(mcp==NULL) return NULL;

	tmp_addr.s_addr = mcp->grp_addr;
	IGMPV3LOG(LOG_DEBUG, "%s> group=%s", __FUNCTION__, inet_ntoa( tmp_addr ) );
	tmp_addr.s_addr = src;
	IGMPV3LOG(LOG_DEBUG, ", src=%s\n", inet_ntoa( tmp_addr ) );
	
	p = mcp->srclist;
	while (p) {
		if (p->srcaddr == src)
			return p;
		p = p->next;
	}
	
	p = malloc(sizeof(struct src_entry));
	if (!p) {
		return NULL;
	}
	memset( p, 0, sizeof(struct src_entry) );
	p->srcaddr = src;
	p->timer.lefttime = 0;
	p->timer.retry_left = 0;
	//p->mrt_state = 0;
	p->next = mcp->srclist;
	mcp->srclist = p;
	return p;
}

int del_from_srclist(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry **q, *p;
	struct in_addr tmp_addr;

	if(mcp==NULL) return  -1;

	tmp_addr.s_addr=mcp->grp_addr;
	IGMPV3LOG(LOG_DEBUG, "%s> group=%s", __FUNCTION__, inet_ntoa( tmp_addr ) );
	tmp_addr.s_addr=src;
	IGMPV3LOG(LOG_DEBUG, ", src=%s\n", inet_ntoa( tmp_addr ) );
	
	q = &mcp->srclist;
	p = *q;
	while (p) {
		if(p->srcaddr == src) {
			*q = p->next;
			free(p);
			return 0;
		}
		q = &p->next;
		p = p->next;
	}
	
	return 0;
}

struct src_entry * get_specific_src(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry **q, *p;
	struct in_addr tmp_addr;

	if(mcp==NULL) return NULL;

	tmp_addr.s_addr=mcp->grp_addr;
	IGMPV3LOG(LOG_DEBUG, "%s> group=%s", __FUNCTION__, inet_ntoa( tmp_addr ) );
	tmp_addr.s_addr=src;
	IGMPV3LOG(LOG_DEBUG, ", src=%s\n", inet_ntoa( tmp_addr ) );
	
	q = &mcp->srclist;
	p = *q;
	while (p) {
		if(p->srcaddr == src)
		{
			return p;
		}
		q = &p->next;
		p = p->next;
	}
	
	return NULL;
}

int get_srclist_num( struct mcft_entry *mcp )
{
	int ret=0;
	struct src_entry *p;
	struct in_addr tmp_addr;

	if(mcp==NULL) return ret;
	
	p = mcp->srclist;
	while(p)
	{
		ret++;
		p=p->next;
	}

	tmp_addr.s_addr=mcp->grp_addr;
	IGMPV3LOG(LOG_DEBUG, "%s> group:%s has %d source(s)\n", __FUNCTION__, inet_ntoa( tmp_addr ), ret );

	return ret;
}

int RTK_RG_multicastFlow_add(unsigned int group, int *index);

int RTK_RG_multicastFlow_delete(int index);

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_add_group( struct mcft_entry *mymcp)
{
	__u32 group = mymcp->grp_addr;
	struct ip_mreq mreq;
	struct IfDesc *up_dp ;
	struct in_addr tmp_addr;
	int ret=-1;
        int idx ;

	tmp_addr.s_addr=group;
	IGMPV3LOG(LOG_DEBUG, "%s> join the group=%s\n", __FUNCTION__, inet_ntoa( tmp_addr ) );

	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	
	for(idx=0;idx<igmp_up_if_num;idx++){
		up_dp = getIfByName(igmp_up_if_name[idx]);	
		if(up_dp==NULL)
			continue;
		if (up_dp->InAdr.s_addr == 0)
			continue;
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,mymcp))
			continue;
#endif
		IGMPV3LOG(LOG_DEBUG, "%s> IP_ADD_MEMBERSHIP to %s\n", __FUNCTION__, igmp_up_if_name[idx] );

		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret) {
			char FmtBu0[ 32 ], FmtBu1[ 32 ];
			printf("%s(%d): setsockopt IP_ADD_MEMBERSHIP %s error on %s(%s)\n", __FUNCTION__, __LINE__,
				inet_ntop(AF_INET, &mreq.imr_multiaddr.s_addr, FmtBu0, sizeof(FmtBu0)), igmp_up_if_name[idx],
				inet_ntop(AF_INET, &mreq.imr_interface.s_addr, FmtBu1, sizeof(FmtBu1)));
			perror("Add Membership");
		}
	}

	if(!isIGMPSnoopingEnabled())
	{
		struct mcft_entry *mymcp = NULL;
		mymcp = get_mcft(group);
		if(mymcp)
		{
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
			rtk_fc_multicast_flow_add(group);
#endif
		}
	}
	return ret;
}

int igmp_del_group( struct mcft_entry *mymcp )
{
	__u32 group = mymcp->grp_addr;
	struct ip_mreq mreq;
	struct IfDesc *up_dp ;
	struct in_addr tmp_addr;
	int idx ;
        int ret=0 ;

	tmp_addr.s_addr=group;
	IGMPV3LOG(LOG_DEBUG, "%s> drop the group=%s\n", __FUNCTION__, inet_ntoa( tmp_addr ) );

	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	
	for(idx=0;idx<igmp_up_if_num;idx++){
		up_dp = getIfByName(igmp_up_if_name[idx]);	
		if(up_dp==NULL)
			continue;
		if (up_dp->InAdr.s_addr == 0)
			continue;
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,mymcp))
			continue;
#endif

		IGMPV3LOG(LOG_DEBUG, "%s> IP_DROP_MEMBERSHIP to %s\n", __FUNCTION__, igmp_up_if_name[idx] );
	
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(MRouterFD, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret) {
			char FmtBu0[ 32 ], FmtBu1[ 32 ];
			printf("%s(%d): setsockopt IP_DROP_MEMBERSHIP %s error on %s(%s)\n", __FUNCTION__, __LINE__,
				inet_ntop(AF_INET, &mreq.imr_multiaddr.s_addr, FmtBu0, sizeof(FmtBu0)), igmp_up_if_name[idx],
				inet_ntop(AF_INET, &mreq.imr_interface.s_addr, FmtBu1, sizeof(FmtBu1)));
			perror("Delete Membership");
		}
	}	

	{
		struct mcft_entry *mymcp = NULL;
		mymcp = get_mcft(group);
		if(mymcp){
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
			rtk_fc_multicast_flow_delete(group);
#endif
		}
	}

	return ret;
}
#else
int igmp_add_group( struct mcft_entry *mymcp)
{
	__u32 group = mymcp->grp_addr;
	struct ip_mreq mreq;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	struct in_addr tmp_addr;
	int ret;

	tmp_addr.s_addr=group;
	IGMPV3LOG(LOG_DEBUG, "%s> join the group=%s\n", __FUNCTION__, inet_ntoa( tmp_addr ) );

	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(MRouterFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret)
		;//printf("setsockopt IP_ADD_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));

	if(!isIGMPSnoopingEnabled())
	{
		struct mcft_entry *mymcp = NULL;
		mymcp = get_mcft(group);
		if(mymcp)
		{
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
			rtk_fc_multicast_flow_add(group);
#endif
		}
	}
	return ret;		
}

int igmp_del_group( struct mcft_entry *mymcp  )
{
	__u32 group = mymcp->grp_addr;
	struct ip_mreq mreq;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	struct in_addr tmp_addr;
	int ret;

	tmp_addr.s_addr=group;
	IGMPV3LOG(LOG_DEBUG, "%s> drop the group=%s\n", __FUNCTION__, inet_ntoa( tmp_addr ) );

	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(MRouterFD, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret)
		printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	
	{
		struct mcft_entry *mymcp = NULL;
		mymcp = get_mcft(group);
		if(mymcp && mymcp->rg_flow_index != -1)
		{
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
			rtk_fc_multicast_flow_delete(group);
#endif
		}
	}

	return ret;
}
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_add_mr(struct mcft_entry *mymcp, __u32 src, int enable )
{
	__u32 group = mymcp->grp_addr;
#ifdef MULTICAST_LANWAN_BIND
	struct IfDesc *up_dp ;
#endif
	struct MRouteDesc	mrd;
        int idx ;
	/* add multicast routing entry */
	mrd.OriginAdr.s_addr = src;
	mrd.SubsAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;

	/** for specific case, it can't add MRoute in all WAN interface, because it will always match last add entry in kernel.
	      in this case, we add MRoute when receive IGMPMSG_NOCACHE msg. **/
	if(src == htonl(INADDR_ANY)){
		IGMPV3LOG(LOG_DEBUG, "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
		IGMPV3LOG(LOG_DEBUG, ", src:%s, enable:%d\n", inet_ntoa(mrd.OriginAdr), enable );
		for(idx=0;idx<igmp_up_if_num;idx++)
		{   
#ifdef MULTICAST_LANWAN_BIND
			up_dp = getIfByName(igmp_up_if_name[idx]);	
			if(up_dp==NULL)
				continue;
			if (up_dp->InAdr.s_addr == 0)
				continue;
			if(!rtk_is_bind_interface(up_dp,mymcp))
				continue;	
#endif
			IGMPV3LOG(LOG_DEBUG, "%s> addMRoute to %s\n", __FUNCTION__, igmp_up_if_name[idx] );
			mrd.InVif = igmp_up_if_idx[idx];
			
			memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
			mrd.TtlVc[(unsigned char)igmp_down_if_idx] = enable;	//forwarding intf
			mrd.TtlVc[(unsigned char)igmp_up_if_idx[idx]] = enable;	//incomming intf
			addMRoute(&mrd);
		}
	}
	return (1);
}

int igmp_del_mr( struct mcft_entry *mymcp,__u32 src )
{
	__u32 group = mymcp->grp_addr;
#ifdef MULTICAST_LANWAN_BIND
	struct IfDesc *up_dp ;
#endif
	struct MRouteDesc	mrd;
	int ret=0;
        int idx;
		
	/* delete multicast routing entry */
	mrd.OriginAdr.s_addr = src;
	mrd.McAdr.s_addr = group;

	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));

	IGMPV3LOG(LOG_DEBUG, "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG(LOG_DEBUG, ", src:%s\n", inet_ntoa(mrd.OriginAdr) );
	for(idx=0;idx<igmp_up_if_num;idx++){
#ifdef MULTICAST_LANWAN_BIND
		up_dp = getIfByName(igmp_up_if_name[idx]);	
		if(up_dp==NULL)
				continue;
		if(!rtk_is_bind_interface(up_dp,mymcp))
			continue;	
#endif		
		IGMPV3LOG(LOG_DEBUG, "%s> delMRoute to %s\n", __FUNCTION__, igmp_up_if_name[idx] );
           mrd.InVif = igmp_up_if_idx[idx];
	   delMRoute(&mrd);
	}
	return ret;
}
#else
int igmp_add_mr(struct mcft_entry *mymcp, __u32 src, int enable )
{
	__u32 group = mymcp->grp_addr;
	struct MRouteDesc	mrd;

	/* add multicast routing entry */
	mrd.OriginAdr.s_addr = src;
	mrd.SubsAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;

	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	mrd.TtlVc[(uint8)igmp_down_if_idx] = enable;	

	IGMPV3LOG(LOG_DEBUG, "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG(LOG_DEBUG, ", src:%s, enable:%d\n", inet_ntoa(mrd.OriginAdr), enable );

	return (addMRoute(&mrd));
}

int igmp_del_mr( struct mcft_entry *mymcp, __u32 src )
{
	__u32 group = mymcp->grp_addr;
	struct MRouteDesc	mrd;
	int ret=0;

	/* delete multicast routing entry */
	mrd.OriginAdr.s_addr = src;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));

	IGMPV3LOG(LOG_DEBUG, "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG(LOG_DEBUG, ", src:%s\n", inet_ntoa(mrd.OriginAdr) );

	delMRoute(&mrd);	
	return ret;
}
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_set_srcfilter( struct mcft_entry *p )
{
	struct in_addr tmp_addr;
	struct ip_msfilter *imsfp;
	int	size,i;
	struct IfDesc *up_dp ;
	struct src_entry *s;
	int idx;
         if(p==NULL)	return -1;

	  
    //        up_dp = getIfByName(igmp_up_if_name[idx]);
			
	   /*use the "send_buf buffer*/
	   imsfp = (struct ip_msfilter *)send_buf;
	   imsfp->imsf_multiaddr=p->grp_addr;
//	   imsfp->imsf_interface=up_dp->InAdr.s_addr;
	   imsfp->imsf_fmode=p->filter_mode;
	   imsfp->imsf_numsrc=0;

	   tmp_addr.s_addr=imsfp->imsf_multiaddr;
	   IGMPV3LOG(LOG_DEBUG, "%s> maddr:%s", __FUNCTION__, inet_ntoa(tmp_addr) );
	   tmp_addr.s_addr=imsfp->imsf_interface;
	   IGMPV3LOG(LOG_DEBUG, ", if:%s, fmode:%d\n", inet_ntoa(tmp_addr), imsfp->imsf_fmode  );

	   i=0;
	   s=p->srclist;
	     while(s)
	   {
	   	tmp_addr.s_addr=s->srcaddr;
		IGMPV3LOG(LOG_DEBUG, "%s>try to match=> fmode:%d, timer:%d, slist:%s\n", __FUNCTION__, p->filter_mode, s->timer.lefttime, inet_ntoa(tmp_addr) );
		if( ((p->filter_mode==MCAST_INCLUDE) && (s->timer.lefttime>0)) ||
		    ((p->filter_mode==MCAST_EXCLUDE) && (s->timer.lefttime==0)) )
		{
			imsfp->imsf_slist[i] = s->srcaddr;
			tmp_addr.s_addr=imsfp->imsf_slist[i];
			IGMPV3LOG(LOG_DEBUG, "%s> slist:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
			i++;
		}
		s=s->next;
	   }
	   imsfp->imsf_numsrc=i;
	   size = IP_MSFILTER_SIZE( i );
	   IGMPV3LOG(LOG_DEBUG, "%s> numsrc:%d, size:%d\n", __FUNCTION__, imsfp->imsf_numsrc, size );

	   
	    for(idx=0;idx<igmp_up_if_num;idx++)
          {
		up_dp = getIfByName(igmp_up_if_name[idx]);
		if(up_dp==NULL)
			continue;
		if (up_dp->InAdr.s_addr == 0)
			continue;
#ifdef MULTICAST_LANWAN_BIND
		if(!rtk_is_bind_interface(up_dp,p))
			continue;
#endif
		IGMPV3LOG(LOG_DEBUG, "%s> IP_MSFILTER to %s\n", __FUNCTION__, igmp_up_if_name[idx] );
		
		imsfp->imsf_interface=up_dp->InAdr.s_addr;
		if (setsockopt(MRouterFD, IPPROTO_IP, IP_MSFILTER, imsfp,size) < 0 )
		{
			perror("setsockopt IP_MSFILTER");
		}
	  }  
	return 0;
}
#else
int igmp_set_srcfilter( struct mcft_entry *p )
{
	struct in_addr tmp_addr;
	struct ip_msfilter *imsfp;
	int	size,i;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	struct src_entry *s;

	if(p==NULL)	return -1;

	/*use the "send_buf buffer*/
	imsfp = (struct ip_msfilter *)send_buf;
	imsfp->imsf_multiaddr=p->grp_addr;
	imsfp->imsf_interface=up_dp->InAdr.s_addr;
	imsfp->imsf_fmode=p->filter_mode;
	imsfp->imsf_numsrc=0;

	tmp_addr.s_addr=imsfp->imsf_multiaddr;
	IGMPV3LOG(LOG_DEBUG, "%s> maddr:%s", __FUNCTION__, inet_ntoa(tmp_addr) );
	tmp_addr.s_addr=imsfp->imsf_interface;
	IGMPV3LOG(LOG_DEBUG, ", if:%s, fmode:%d\n", inet_ntoa(tmp_addr), imsfp->imsf_fmode  );

	i=0;
	s=p->srclist;
	while(s)
	{
		tmp_addr.s_addr=s->srcaddr;
		IGMPV3LOG(LOG_DEBUG, "%s>try to match=> fmode:%d, timer:%d, slist:%s\n", __FUNCTION__, p->filter_mode, s->timer.lefttime, inet_ntoa(tmp_addr) );
		if( ((p->filter_mode==MCAST_INCLUDE) && (s->timer.lefttime>0)) ||
		    ((p->filter_mode==MCAST_EXCLUDE) && (s->timer.lefttime==0)) )
		{
			imsfp->imsf_slist[i] = s->srcaddr;
			tmp_addr.s_addr=s->srcaddr;
			IGMPV3LOG(LOG_DEBUG, "%s> slist:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
			i++;
		}
		s=s->next;
	}
	imsfp->imsf_numsrc=i;
	size = IP_MSFILTER_SIZE( i );
	IGMPV3LOG(LOG_DEBUG, "%s> numsrc:%d, size:%d\n", __FUNCTION__, imsfp->imsf_numsrc, size );

	if (setsockopt(MRouterFD, IPPROTO_IP, IP_MSFILTER, imsfp,size) < 0 )
	{
		//perror("setsockopt IP_MSFILTER"); 
        	return -1;
	}

	return 0;
}
#endif
int check_src_set( __u32 src, struct src_entry *srclist )
{
	struct src_entry *p;
	for( p=srclist; p!=NULL; p=p->next )
	{
		if ( src == p->srcaddr )
			return 1;
	}
	return 0;
}

int check_src( __u32 src, __u32 *sources, int numsrc )
{
	int i;
	for (i=0;i< numsrc; i++)
	{
		if( src == sources[i] )
			return 1;
	}
	return 0;
}

void handle_igmpv3_isex( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	// Mason Yu Test
	//printf("handle_igmpv3_isex\n");
		
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				// Mason Yu Test
				printf("handle_igmpv3_isex: MCAST_INCLUDE\n");
				//IN(A), IS_EX(B) => EX(A*B, B-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B-A)=0
						if( check_src_set( s->srcaddr, old_set )==0 )
						{
							s->timer.lefttime = 0;
							s->timer.retry_left = 0;
							igmp_add_mr( mymcp, s->srcaddr, 0 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					struct src_entry *s_next=s->next;
					
					//Delete (A-B)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}					
					s = s_next;
				}

				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = last_member_query_count;
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				// Mason Yu Test
				//printf("handle_igmpv3_isex: MCAST_EXCLUDE\n");
#ifdef KEEP_GROUP_MEMBER
				add_user(mymcp, src);
#endif

				//EX(X,Y), IS_EX(A) => EX(A-Y, Y*A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A-X-Y)=Group Timer
						if( check_src_set( s->srcaddr, old_set )==0 )
						{
							s->timer.lefttime = MEMBER_QUERY_INTERVAL;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp, s->srcaddr, 1 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					struct src_entry *s_next=s->next;
					
					//Delete (X-A), Delete(Y-A)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}
					s = s_next;
				}
								
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = last_member_query_count;

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
				// hack here to immediately add mr for ASM case
				if (mymcp->srclist==NULL && mymcp->mrt_state == 0) {
					igmp_add_mr( mymcp, 0, 1 );
					mymcp->mrt_state = 1;
				}
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_isin( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				//printf("handle_igmpv3_isin: MCAST_INCLUDE\n");				
				//IN(A), IN(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						if( check_src_set( s->srcaddr, old_set )==0 )
							igmp_add_mr( mymcp, s->srcaddr, 1 );
					}
				}

				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s;
				
				//printf("handle_igmpv3_isin: MCAST_EXCLUDE\n");
				//EX(X,Y), IS_IN(A) => EX(X+A, Y-A)
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						igmp_add_mr( mymcp, s->srcaddr, 1 );
					}
				}
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

void handle_igmpv3_toin( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;	
	
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group) && srcnum) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				// Mason Yu Test
				//printf("handle_igmpv3_toin: MCAST_INCLUDE\n");

				//IN(A), TO_IN(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						if( check_src_set( s->srcaddr, old_set )==0 )
							igmp_add_mr( mymcp, s->srcaddr, 1 );
					}
				}
				
				//send Q(G,A-B)
				i=0;
				s = old_set;
				while(s)
				{
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						gsrctmp[i]=s->srcaddr;
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}					
					s = s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );				
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				// Mason Yu Test
				//printf("handle_igmpv3_toin: MCAST_EXCLUDE and srcnum=%d\n", srcnum);
				if ( srcnum != 0 ) {

					//EX(X,Y), TO_IN(A) => EX(X+A, Y-A)
					old_set = mymcp->srclist;
					for(i=0;i<srcnum;i++)
					{
						s = add_to_srclist( mymcp, grec_src[i] );
						if(s)
						{	// (A)= GMI
							s->timer.lefttime = MEMBER_QUERY_INTERVAL;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp, s->srcaddr, 1 );
						}
					}	
                                	
					//send Q(G,X-A)
					i=0;
					s = old_set;
					while(s)
					{
						if( s->timer.lefttime>0 )
						{
							if( check_src( s->srcaddr, grec_src, srcnum )==0 )
							{
								gsrctmp[i]=s->srcaddr;
								i++;
								if(i==IGMPV3_MAX_SRCNUM) break;
							}
						}					
						s = s->next;
					}
					if(i>0) igmpv3_query( mymcp, i, gsrctmp );

					//send Q(G)
					if( mymcp->igmp_ver==IGMP_VER_3 )
						igmpv3_query( mymcp, 0, NULL );
					else
						igmp_query(ALL_SYSTEMS, mymcp->grp_addr, last_member_query_interval);
					//set the new state
					mymcp->filter_mode = MCAST_EXCLUDE;
					igmp_set_srcfilter( mymcp );
				}
				else { // srcnum == 0
#ifdef KEEP_GROUP_MEMBER
					int count;
					if(mymcp->user_count != 0)
					{
						if( mymcp->igmp_ver==IGMP_VER_3 )
							igmpv3_query( mymcp, 0, NULL );
						else
							igmp_query(ALL_SYSTEMS, mymcp->grp_addr, last_member_query_interval);
						if(mymcp->user_count == 1)
						{
							extern void igmp_specific_timer_expired(void *arg);
							timeout(igmp_specific_timer_expired , mymcp, last_member_query_interval/IGMP_TIMER_SCALE, &mymcp->timer.ch);
						}
					}
					count = del_user(mymcp, src);			
					
					if (count == 0) {// no member, drop it!
						if(fast_leave){
							igmp_del_group(mymcp);
							igmp_del_mr( mymcp, 0 );
							del_mcft(mymcp);
						}
					}
					else {
						//set the new state
						mymcp->filter_mode = MCAST_EXCLUDE;
						igmp_set_srcfilter( mymcp );
					}
#endif
				}
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_toex( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_toex: MCAST_INCLUDE\n");
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif
				//IN(A), TO_EX(B) => EX(A*B, B-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B-A)=0
						if( check_src_set( s->srcaddr, old_set )==0 )
						{
							s->timer.lefttime = 0;
							s->timer.retry_left = 0;
							igmp_add_mr( mymcp, s->srcaddr, 0 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					struct src_entry *s_next=s->next;
					
					//Delete (A-B)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}					
					s = s_next;
				}

				//send Q(G,A*B)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					if( s->timer.lefttime > 0 )
					{
						gsrctmp[i]=s->srcaddr;
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}
					s=s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = last_member_query_count;
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
				// hack here to immediately add mr for ASM case
				if (mymcp->srclist==NULL && mymcp->mrt_state == 0) {
					igmp_add_mr( mymcp, 0, 1 );
					mymcp->mrt_state = 1;
				}
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_toex: MCAST_EXCLUDE\n");
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif	
				//EX(X,Y), TO_EX(A) => EX(A-Y, Y*A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A-X-Y)=Group Timer
						if( check_src_set( s->srcaddr, old_set )==0 )
						{
							s->timer.lefttime = mymcp->timer.lefttime;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp, s->srcaddr, 1 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					struct src_entry *s_next=s->next;
					
					//Delete (X-A), Delete(Y-A)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}
					s = s_next;
				}

				//send Q(G,A-Y)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					if( s->timer.lefttime > 0 )
					{
						gsrctmp[i]=s->srcaddr;
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}
					s=s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = last_member_query_count;

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_allow( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_allow: MCAST_INCLUDE\n");				
				//IN(A), ALLOW(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						if( check_src_set( s->srcaddr, old_set )==0 )
							igmp_add_mr( mymcp, s->srcaddr, 1 );
					}
				}

				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s;
				
				//printf("handle_igmpv3_allow: MCAST_EXCLUDE\n");
				//EX(X,Y), ALLOW(A) => EX(X+A, Y-A)
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						igmp_add_mr( mymcp, s->srcaddr, 1 );
					}
				}
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

void handle_igmpv3_block( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST( ntohl(group) ))
		return;
	/* check if it's protocol reserved group */
	if(( ntohl(group)&0xFFFFFF00 )==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( mymcp );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s;
				
				// Mason Yu Test
				//printf("handle_igmpv3_block: MCAST_INCLUDE\n");
				
				//IN(A), BLOCK(B) => IN(A)
				//send Q(G,A*B)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					if( check_src( s->srcaddr, grec_src, srcnum )==1 )
					{
						gsrctmp[i]=s->srcaddr;
#ifdef KEEP_GROUP_MEMBER

						int count;
						count = del_user(mymcp, src);
						if (count == 0) {// no member, drop it!
							igmp_del_group(mymcp);
							igmp_del_mr( mymcp, s->srcaddr );
							del_mcft(mymcp);
						}
#endif	
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}
					s=s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
			
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_block: MCAST_EXCLUDE\n");

				//EX(X,Y), BLOCK(A) => EX( X+(A-Y), Y )
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A-X-Y)=Group Timer
						if( check_src_set( s->srcaddr, old_set )==0 )
						{
							s->timer.lefttime = mymcp->timer.lefttime;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp, s->srcaddr, 1 );
						}
					}
				}
				
				//send Q(G,A-Y)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					if( s->timer.lefttime > 0 )
					{
						if( check_src( s->srcaddr, grec_src, srcnum )==1 )
						{
							gsrctmp[i]=s->srcaddr;
#ifdef KEEP_GROUP_MEMBER

							int count;
							count = del_user(mymcp, src);
							if (count == 0) {// no member, drop it!
								igmp_del_group(mymcp);
								igmp_del_mr( mymcp, s->srcaddr );
								del_mcft(mymcp);
							}
#endif
							i++;
							if(i==IGMPV3_MAX_SRCNUM) break;
						}
					}
					s=s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

int igmpv3_query( struct mcft_entry *entry, int srcnum, __u32 *srcfilter )
{
    struct in_addr tmp_addr;
    struct igmpv3_query	*igmpv3;
    struct sockaddr_in	sdst;
    struct IfDesc 	*dp = getIfByName(igmp_down_if_name);
    __u32	grp=0;
    int		i,totalsize=0;

    if(entry) grp=entry->grp_addr;
    k_set_if(dp->InAdr.s_addr);
    	
    igmpv3            = (struct igmpv3_query *)send_buf;
    igmpv3->type      = 0x11;
    igmpv3->code      = last_member_query_interval;
    igmpv3->csum      = 0;
    igmpv3->group     = grp;
    igmpv3->resv      = 0;
    igmpv3->suppress  = 1;
    igmpv3->qrv       = robust_count;
    igmpv3->qqic      = MEMBER_QUERY_INTERVAL;
    igmpv3->nsrcs     = srcnum;

    tmp_addr.s_addr=grp;
    IGMPV3LOG(LOG_INFO, "%s> send to group:%s, src:", __FUNCTION__, inet_ntoa( tmp_addr ) );
    for(i=0;i<srcnum;i++)
    {
    	igmpv3->srcs[i] = srcfilter[i];
    	tmp_addr.s_addr=igmpv3->srcs[i];
    	IGMPV3LOG(LOG_INFO, "(%s)", inet_ntoa( tmp_addr ) );
    }
    totalsize	      = sizeof(struct igmpv3_query)+igmpv3->nsrcs*sizeof(__u32);
    igmpv3->csum      = in_cksum((u_short *)igmpv3, totalsize );
    IGMPV3LOG(LOG_INFO, "\n" );


    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    if(grp)
    	sdst.sin_addr.s_addr = grp; 
    else 
    	sdst.sin_addr.s_addr = ALL_SYSTEMS;

    if (sendto(MRouterFD, igmpv3, totalsize, 0, (struct sockaddr *)&sdst, sizeof(sdst)) < 0)
    {
    	tmp_addr.s_addr=dp->InAdr.s_addr;
	printf("igmpv3_query> sendto error, from %s ", inet_ntoa(tmp_addr));
	tmp_addr.s_addr=sdst.sin_addr.s_addr;
	printf("to %s\n", inet_ntoa(tmp_addr));
    }

    return 0;
}



void handle_group_timer(void)
{
	struct mcft_entry *p,*next;
	struct in_addr tmp_addr;
	p = mcpq;
	next = NULL;
	while( p!=NULL )
	{
		next = p->next;
		
		if( p->timer.lefttime )
		{
			p->timer.lefttime--;
			if( (p->timer.lefttime==0) && (p->timer.retry_left!=0) )
			{
				p->timer.retry_left--;
				if( p->timer.retry_left )
				{
					tmp_addr.s_addr=p->grp_addr;
					IGMPV3LOG(LOG_INFO, "%s> GROUP TIMEOUT, send Query to group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );

					p->timer.lefttime = last_member_query_interval;
				    if( IGMPCurrentVersion==3 )
						igmpv3_query( p, 0, NULL );
					else
						igmp_query(ALL_SYSTEMS, p->grp_addr, last_member_query_interval);
				}
				
			}
			
			switch( p->filter_mode )
			{
			case MCAST_INCLUDE:
				break;
			case MCAST_EXCLUDE:
				if( p->timer.lefttime==0 )
				{
					struct src_entry *s, *s_next;
					tmp_addr.s_addr=p->grp_addr;
					IGMPV3LOG(LOG_DEBUG,"%s> group:%s is timeout(EXCLUDE mode)\n", __FUNCTION__, inet_ntoa(tmp_addr) );
					
					s=p->srclist;
					while(s)
					{
						s_next=s->next;
						if( s->timer.lefttime==0 )
						{
							//remove this source
							igmp_del_mr( p, s->srcaddr );
							del_from_srclist( p, s->srcaddr );
						}
						s=s_next;
					}
					
					if( p->srclist )
					{
						tmp_addr.s_addr=p->grp_addr;
						IGMPV3LOG(LOG_DEBUG,"%s> group:%s changes to INCLUDE mode\n", __FUNCTION__, inet_ntoa(tmp_addr) );

						p->filter_mode=MCAST_INCLUDE;
						igmp_set_srcfilter( p );
					}else{
						tmp_addr.s_addr=p->grp_addr;
						IGMPV3LOG(LOG_DEBUG,"%s> remove group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );

						//delete this group record
						if(p->mrt_state)
						{
							igmp_del_mr( p, 0);
							p->mrt_state=0;
						}
						igmp_del_group( p );
						del_mcft( p );
					}
				}
				break;
			default:
				break;
			}
			
		}

		p = next;
	}
}

void handle_src_timer(void)
{
	struct mcft_entry *p,*next;
	struct in_addr tmp_addr;
	p = mcpq;
	next = NULL;
	while( p!=NULL )
	{
		struct src_entry *s, *src_next;
		int	change_sf=0;
		
		next = p->next;
		s = p->srclist;
		src_next = NULL;
		while( s )
		{
			src_next = s->next;
			
			if( s->timer.lefttime )
			{
				s->timer.lefttime--;
				if( (s->timer.lefttime==0) && (s->timer.retry_left!=0) )
				{
					s->timer.retry_left--;
					if( s->timer.retry_left )
					{
						tmp_addr.s_addr=p->grp_addr;
						IGMPV3LOG(LOG_DEBUG,"%s> SRC TIMEOUT, send Query to group:%s", __FUNCTION__, inet_ntoa(tmp_addr) );
						tmp_addr.s_addr=s->srcaddr;
						IGMPV3LOG(LOG_DEBUG,", src:%s\n", inet_ntoa(tmp_addr) );
						
						s->timer.lefttime = last_member_query_interval;
						igmpv3_query( p, 1, &s->srcaddr );
					}
				}
				
					
				switch( p->filter_mode )
				{
				case MCAST_INCLUDE:
					if( s->timer.lefttime )
					{
						//forward this src
						if(p->mrt_state)
						{
							tmp_addr.s_addr=p->grp_addr;
							IGMPV3LOG(LOG_DEBUG,"%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
							igmp_del_mr( p, 0);
							p->mrt_state=0;
						}
					}else{
						tmp_addr.s_addr=s->srcaddr;
						IGMPV3LOG(LOG_DEBUG,"%s> remove src:%s", __FUNCTION__, inet_ntoa(tmp_addr) );
						tmp_addr.s_addr=p->grp_addr;
						IGMPV3LOG(LOG_DEBUG," from group:%s\n", inet_ntoa(tmp_addr) );
						
						//==0, stop this src
						igmp_del_mr( p, s->srcaddr );
						del_from_srclist( p, s->srcaddr );
						//NO MORE SOURCE, DELETE GROUP RECORD
						change_sf=1;
					}
					break;
				case MCAST_EXCLUDE:
					if( s->timer.lefttime )
					{
						//forward this src
						if(p->mrt_state)
						{
							tmp_addr.s_addr=p->grp_addr;
							IGMPV3LOG(LOG_DEBUG,"%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
							igmp_del_mr( p, 0 );
							p->mrt_state=0;
						}
					}else{
						tmp_addr.s_addr=s->srcaddr;
						IGMPV3LOG(LOG_DEBUG,"%s> stop forwarding src:%s", __FUNCTION__, inet_ntoa(tmp_addr) );
						tmp_addr.s_addr=p->grp_addr;
						IGMPV3LOG(LOG_DEBUG," for group:%s\n", inet_ntoa(tmp_addr) );

						//==0, stop this src, do not remove record
						igmp_add_mr( p, s->srcaddr, 0 );
						change_sf=1;
					}
					break;
				default:
					break;
				}
			}				
			s = src_next;
		}
		
		//set the new state
		if(change_sf)	igmp_set_srcfilter( p );
		
		//EX( {}, X )
		if( (p->filter_mode==MCAST_EXCLUDE) && (p->srclist!=NULL) )
		{
			int allsrcinex=1;
			s = p->srclist;
			while(s)
			{
				if(s->timer.lefttime>0)
				{
					allsrcinex=0;
					break;
				}
				s=s->next;
			}
			if(allsrcinex==1)
			{
				if( p->mrt_state==0 )
				{
					tmp_addr.s_addr=p->grp_addr;
					IGMPV3LOG(LOG_DEBUG,"%s> forward all sources for group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );

					//igmp_add_mr( p, 0, 1 );
					//p->mrt_state = 1;
				}
			}
		}
		
		//for empty condition
		if( p->srclist==NULL )
		{
			switch( p->filter_mode )
			{
			case MCAST_INCLUDE:
				//not foreward all source
				//delete this group record
				if(p->mrt_state)
				{
					tmp_addr.s_addr=p->grp_addr;
					IGMPV3LOG(LOG_DEBUG,"%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
					igmp_del_mr( p, 0 );
					p->mrt_state=0;
				}

				tmp_addr.s_addr=p->grp_addr;
				IGMPV3LOG(LOG_DEBUG,"%s> remove group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
				igmp_del_group( p );
				del_mcft( p );
				break;
			case MCAST_EXCLUDE:
				//forward all source
				if( p->mrt_state==0 )
				{
					tmp_addr.s_addr=p->grp_addr;
					IGMPV3LOG(LOG_DEBUG,"%s> forward all sources for group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );

					igmp_add_mr( p, 0, 1 );
					p->mrt_state = 1;
				}
				break;
			default:
				break;
			}			
		}
		
		p = next;
	}
}



static struct timeval start_time={0,0};
static int init_stat_time=0;
void igmpv3_timer(void)
{
	struct timeval cur_time;
	
	if( init_stat_time==0 )
	{
		gettimeofday( &start_time, NULL );
		init_stat_time = 1;
		return;
	}


	
	gettimeofday( &cur_time, NULL );
	if( (cur_time.tv_sec > (start_time.tv_sec+1)) ||
	    ((cur_time.tv_sec == (start_time.tv_sec+1)) && (cur_time.tv_usec >=start_time.tv_usec)) )
	{
		//suppose 1 second passed
		//printf( "." );fflush(NULL);
				
		handle_group_timer();
		handle_src_timer();		
		
		start_time.tv_sec = cur_time.tv_sec;
		start_time.tv_usec = cur_time.tv_usec;
	}

	return;
}


int igmpv3_accept(int recvlen, struct IfDesc *dp, int flag)
{
	register __u32 src, dst, group;
	struct iphdr *ip;
	struct igmphdr *igmp;
	int iphdrlen;
	struct mcft_entry *mymcp;
	struct igmpmsg *msg;
	struct in_addr tmp_addr;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	struct MRouteDesc	mrd;
	int idx;
	struct src_entry *q=NULL;
#endif
#ifdef MULTICAST_LANWAN_BIND
	struct IfDesc *tmp_bind_up = NULL;
	int i;
#endif
	
	if (recvlen < sizeof(struct iphdr)) 
	{
		igmpproxy_log(LOG_WARNING, 0, "received packet too short (%u bytes) for IP header", recvlen);
		return 0;
	}
	
	ip  = (struct iphdr *)recv_buf;
	src = ip->saddr;
	dst = ip->daddr;
	
	if(!IN_MULTICAST( ntohl(dst) ))	/* It isn't a multicast */
		return -1; 
	if(chk_local(src)) 	/* It's our report looped back */
		return -1;
#ifdef CONFIG_IGMPPROXY_SENDER_IP_ZERO
	if(src == 0)
		return -1;
#endif
	if(dst == ALL_PRINTER)	/* It's MS-Windows UPNP all printers notify */
		return -1;
		
	//pkt_debug(recv_buf);
	
	iphdrlen  = ip->ihl << 2;
	//ipdatalen = ip->tot_len;
	
	igmp    = (struct igmphdr *)(recv_buf + iphdrlen);
	group   = igmp->group;
	
	/* determine message type */
	tmp_addr.s_addr=ip->saddr;
	IGMPV3LOG(LOG_INFO, "\n%s> receive IGMP type [%x] from %s to ", __FUNCTION__, igmp->type, inet_ntoa(tmp_addr));
	tmp_addr.s_addr=ip->daddr;
	IGMPV3LOG(LOG_INFO, "%s\n", inet_ntoa(tmp_addr));
	switch (igmp->type) {
		case IGMP_HOST_MEMBERSHIP_QUERY:
			/* Linux Kernel will process local member query, it wont reach here */
			break;
	
		case IGMP_HOST_MEMBERSHIP_REPORT:
		case IGMPV2_HOST_MEMBERSHIP_REPORT:
			{
				if(flag == FROM_WAN)
					break;

				if(group == ALL_PRINTER){	/* ignore join SSDP group for Dial-On Demand */
					break;
				}

				/* Receive V2 Report here! */
				setIGMPVersionToV2();
				tmp_addr.s_addr=group;
				IGMPV3LOG(LOG_INFO, "%s> REPORT(V1/V2), group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
				if(!chk_mcft(group)) 
				{
					mymcp = add_mcft(group, src);
					if(!mymcp) return -1;		
					mymcp->igmp_ver = IGMP_VER_2;
					if (igmp_add_group( mymcp )) {
						del_mcft(mymcp);
						return -1;
					}

					//Group Timer=GMI
					mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
					mymcp->timer.retry_left = last_member_query_count;
					
					//set the new state
					mymcp->filter_mode = MCAST_EXCLUDE;
					igmp_set_srcfilter( mymcp );
			}
					
				mymcp = get_mcft(group);
				if(mymcp) mymcp->igmp_ver = IGMP_VER_2;
				
				//Report => IS_EX( {} )	
				handle_igmpv3_isex( group,src, 0, NULL );
			}
			break;
 		case IGMP_HOST_V3_MEMBERSHIP_REPORT:
		     {
			struct igmpv3_report *igmpv3;
			struct igmpv3_grec *igmpv3grec;
			__u16 rec_id;
			
			if(flag == FROM_WAN)
				break;
			
			IGMPV3LOG(LOG_INFO, "%s> REPORT(V3)\n", __FUNCTION__ );
			igmpv3 = (struct igmpv3_report *)igmp;
			//printf( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT\n" );
			//printf( "igmpv3->type:0x%x\n", igmpv3->type );
			//printf( "igmpv3->ngrec:0x%x\n", ntohs(igmpv3->ngrec) );
		
			rec_id=0;
			igmpv3grec =  &igmpv3->grec[0];
			while( rec_id < ntohs(igmpv3->ngrec) )
			{
				int srcnum;
				//printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
				//printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
				//printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
				//printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );
			
				group = igmpv3grec->grec_mca;
				srcnum = ntohs(igmpv3grec->grec_nsrcs);

				if(group == ALL_PRINTER){	/* ignore join SSDP group for Dial-On Demand */
					rec_id++;
					igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof( __u32 ) );
					continue;
				}

				switch( igmpv3grec->grec_type )
				{
				case IGMPV3_MODE_IS_INCLUDE:
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> IS_IN, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_isin( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_MODE_IS_EXCLUDE:
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> IS_EX, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_isex( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_CHANGE_TO_INCLUDE: 
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> TO_IN, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_toin( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_CHANGE_TO_EXCLUDE: 
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> TO_EX, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_toex( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_ALLOW_NEW_SOURCES:
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> ALLOW, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_allow( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_BLOCK_OLD_SOURCES:
					tmp_addr.s_addr=group;
					IGMPV3LOG(LOG_INFO, "%s> BLOCK, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(tmp_addr), srcnum );
					handle_igmpv3_block( group,src, srcnum, igmpv3grec->grec_src );
					break;
				default:
					IGMPV3LOG(LOG_INFO, "%s> Unknown Group Record Types [%x]\n", __FUNCTION__, igmpv3grec->grec_type );
					break;
				}
			
				rec_id++;
				//printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
				igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof( __u32 ) );
				//printf( "count result: 0x%x\n", igmpv3grec );
			}
			break;
		     }
		case IGMP_HOST_LEAVE_MESSAGE :
			if(flag == FROM_WAN)
				break;
			tmp_addr.s_addr=group;
			IGMPV3LOG(LOG_INFO, "%s> LEAVE(V2), group:%s\n", __FUNCTION__, inet_ntoa(tmp_addr) );
			if(chk_mcft(group))
			{
				//Leave => TO_IN( {} )
				handle_igmpv3_toin( group,src, 0, NULL );
			}
			break;
		case IGMPMSG_NOCACHE: // ipmr_cache_report (no route for incomming group)
#ifndef CONFIG_IGMPPROXY_MULTIWAN
			IGMPV3LOG(LOG_INFO, "%s> dot define multi wan\n", __FUNCTION__);
			//do nothing here.
#else	// ifndef CONFIG_IGMPPROXY_MULTIWAN
			msg = (struct igmpmsg*)recv_buf;
			IGMPV3LOG(LOG_INFO, "%s> [IGMPMSG_NOCACHE] srcaddr: %x, daddr: %x\n", __FUNCTION__, msg->im_src.s_addr, msg->im_dst.s_addr);

			for(idx=0;idx<igmp_up_if_num;idx++){
				if(msg->im_vif == igmp_up_if_idx[idx]){
#ifdef MULTICAST_LANWAN_BIND
					tmp_bind_up = getIfByVif((const int)msg->im_vif);
					bind_up_num = 1;
					bind_up[0] = tmp_bind_up;
#endif
					break;
				}
			}
			if(idx < igmp_up_if_num)
			{
				mymcp=get_mcft(msg->im_dst.s_addr);
				if(mymcp){
#ifdef MULTICAST_LANWAN_BIND
					for(i=0;i<mymcp->bind_up_num;i++)
					{
						if(tmp_bind_up->vif_idx == mymcp->bind_up[i]->vif_idx)
						{
#endif
							for( q=mymcp->srclist; q!=NULL; q=q->next )
							{
								if ( msg->im_src.s_addr == q->srcaddr )
								break;
							}
							/*  RFC3810 Section 7.3. https://tools.ietf.org/html/rfc3810#section-7.3
					 	 	*  INCLUDE mode:
					 	 	*   - if src timer > 0, then Suggest to forward traffic from source.
					 	 	*   - if src timer = 0, then Suggest to stop forward traffic from source and remove source record.
					 	 	*  EXCLUDE mode:
					 	 	*   - if src timer > 0, then Suggest to forward traffic from source.
					 	 	*   - if src timer = 0, then Suggest to stop forward traffic from source.
					 	 	*   - if no source element: Suggest to forward traffic from all sources
					 	 	*/
							// it for specific src addr
							if(q != NULL){
								/* add multicast routing entry for specific src addr case */
								if(q->timer.lefttime > 0)
								{
									mrd.OriginAdr.s_addr = msg->im_src.s_addr;
									mrd.SubsAdr.s_addr = 0;
									mrd.McAdr.s_addr = msg->im_dst.s_addr;

									IGMPV3LOG(LOG_DEBUG, "[%s:%d] found src, %s's multicast traffic(src:%s, grp:%s) route to %s\n", __FUNCTION__, __LINE__, 
										igmp_up_if_name[idx], inet_ntoa(mrd.OriginAdr), inet_ntoa(mrd.McAdr), igmp_down_if_name);
									mrd.InVif = igmp_up_if_idx[idx];

									memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
									mrd.TtlVc[(uint8)igmp_down_if_idx] = 1;	//forwarding intf
									mrd.TtlVc[(uint8)igmp_up_if_idx[idx]] = 1;	//incomming intf
									addMRoute(&mrd);
								}
							}
							else
							{
								if(mymcp->filter_mode == MCAST_EXCLUDE)
								{
									q = add_to_srclist( mymcp, msg->im_src.s_addr);
									if(q)
									{
										q->timer.lefttime = MEMBER_QUERY_INTERVAL;
										q->timer.retry_left = MEMBER_QUERY_COUNT;
									}

									mrd.OriginAdr.s_addr = msg->im_src.s_addr;
									mrd.SubsAdr.s_addr = 0;
									mrd.McAdr.s_addr = msg->im_dst.s_addr;

									IGMPV3LOG(LOG_DEBUG, "[%s:%d] not found src, %s's multicast traffic(src:%s, grp:%s) route to %s\n", __FUNCTION__, __LINE__, 
										igmp_up_if_name[idx], inet_ntoa(mrd.OriginAdr), inet_ntoa(mrd.McAdr), igmp_down_if_name);
									mrd.InVif = igmp_up_if_idx[idx];

									memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
									mrd.TtlVc[(uint8)igmp_down_if_idx] = 1;	//forwarding intf
									mrd.TtlVc[(uint8)igmp_up_if_idx[idx]] = 1;	//incomming intf
									addMRoute(&mrd);
								}
							}
#ifdef MULTICAST_LANWAN_BIND
							break;
						}
					}
#endif
				}
				else{
					IGMPV3LOG(LOG_INFO, "%s> not find mcp\n", __FUNCTION__);
					//do nothing here.
				}
			}
#endif
			break;
		default:
			IGMPV3LOG(LOG_INFO, "%s> receive IGMP Unknown type [%x]\n", __FUNCTION__, igmp->type );
			break;
    }
    return 0;
}


#endif /*_SUPPORT_IGMPV3_*/
