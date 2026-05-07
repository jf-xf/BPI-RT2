
#ifndef _HOST_SPEEDUP_H
#define _HOST_SPEEDUP_H

#include <linux/skbuff.h>

#define IP_SPEEDUP_DEBUG
//#define IP_SPEEDUP_OUTORDER_DBG

enum SPEEDUP_CHECK_RST {
	SPEEDUP_RET_UNMATCH,
	SPEEDUP_RET_MATCH_NO_ACK,
	SPEEDUP_RET_MATCH_QUEUED,
	SPEEDUP_RET_MATCH_ACK
};

#define SPEEDUP_ADJUST_CHKSUM_ID(id_mod, id_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (id_mod != id_org){ \
			accumulate += id_org; \
			accumulate -= id_mod; \
		} \
		accumulate += chksum; \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = (__u16) ~accumulate; \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = (__u16) accumulate; \
		} \
	}while(0)	/* Checksum adjustment */

#define SPEEDUP_ADJUST_CHKSUM_SEQ(seq_mod, seq_org, chksum) \
			do { \
				s32 accumulate = 0; \
				if (seq_mod != seq_org){ \
					accumulate = ((seq_org) & 0xffff); \
					accumulate += (( (seq_org) >> 16 ) & 0xffff); \
					accumulate -= ((seq_mod) & 0xffff); \
					accumulate -= (( (seq_mod) >> 16 ) & 0xffff); \
				} \
				accumulate += chksum; \
				if (accumulate < 0) { \
					accumulate = -accumulate; \
					accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
					accumulate += accumulate >> 16; \
					chksum =(__u16) ~accumulate; \
				} else { \
					accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
					accumulate += accumulate >> 16; \
					chksum = (__u16) accumulate; \
				} \
			}while(0)	/* Checksum adjustment */


typedef struct speedup_pktHdr_s
{
	struct sk_buff	*skb;

	unsigned int	dataLen;

	/* header pointer */
	unsigned char	*pL2Hdr;
	unsigned char	*pPPPoEHdr;
	unsigned char	*pL3Hdr;
	unsigned char	*pL4Hdr;

	/* header offset */
	unsigned int	pppoeHdrOffset;
	unsigned int	l3HdrOffset;
	unsigned int	l4HdrOffset;

	/* ipv4 address/ID info */
	unsigned char	saddr[IP6_ADDR_LEN];
	unsigned char	daddr[IP6_ADDR_LEN];
	unsigned int	id;
	/* source and dest port info in tcphdr */
	unsigned short	sport;
	unsigned short	dport;
	/* detailed info of tcphdr */
	unsigned int	seqNo;
	unsigned int	ackNo;

	union {
		struct {
			unsigned int	isPPPoE:1;
			unsigned int	isIPv4:1;
			unsigned int	isIPv6:1;
			unsigned int	isTCP:1;
			unsigned int	isDSACK:1;
			unsigned int	rsvd:27;
		} bits;
		
		unsigned int all;
	} flags;
#define f_ispppoe	flags.bits.isPPPoE
#define f_isipv4	flags.bits.isIPv4
#define f_isipv6	flags.bits.isIPv6
#define f_istcp		flags.bits.isTCP
#define f_isdsack	flags.bits.isDSACK

	/* any other info should save here */
	void *pStreamInfo;	//point to _speedup_stream_track
}speedup_pktHdr_t;

#ifdef IP_SPEEDUP_DEBUG
extern unsigned int g_vec_nr[CONFIG_NR_CPUS];
extern unsigned int get_softirq_delay_stats(int vec_nr);
#endif
extern unsigned short FAST_ACK_WIN_SIZE;
extern rwlock_t speedup_lock;
/*
direction:
1, Up stream
2, Down stream
*/
extern void speedtest_packet_parser(struct sk_buff *skb, speedup_pktHdr_t *pPktHdr);
extern int shouldBypassSpeedUP(void *pHostTrack);
extern int isUploadSpeedTestStream(void *pHostTrack);
extern int isHostCheck(struct sk_buff *skb, int hook, int direction);
extern int isHostSpeedUpEnable(void);
extern int isDownloadHostSpeedUpEnable(void);
extern void * shouldSpeedUp(unsigned int dir, u8* dstip, u8* srcip, unsigned short dport, unsigned short sport,int ipver);
extern int speedtest_stream_shortcut_tx_learn(void *pHostTrack, speedup_pktHdr_t *pPktHdr);
extern void speedtest_stream_shortcut_rx_learn(void *pHostTrack, speedup_pktHdr_t *pPktHdr, u32 dataLen);
extern int speedtest_stream_shortcut_check(void *pHostTrack, speedup_pktHdr_t *pPktHdr, u32 dataLen);
extern int speedtest_upload_shortcut_enter(void *pHostTrack, speedup_pktHdr_t *pPktHdr);
extern int send_delay_ack_timer(void);
extern int delay_ack_xmit(speedup_pktHdr_t *pPktHdr);
extern void send_upload_data_now(speedup_pktHdr_t *pPktHdr);
extern void speedtest_sample_enter(int pktLen);
extern void speedtest_payload_statistic(unsigned int payloadLen);
extern void fast_ack_calibration_enter(int pktLen);
extern int HostSpeedUP_Stat_get(u8* dstip, u8* srcip, u16 dport, u16 sport,int ipver);
extern unsigned int get_speedtest_time(void);
extern unsigned int get_time_duration(unsigned int startTime);
extern int getUploadAckStat(speedup_pktHdr_t *pPktHdr);
extern void start_xmit_upload_data(void);

#endif /*_HOST_SPEEDUP_H*/

