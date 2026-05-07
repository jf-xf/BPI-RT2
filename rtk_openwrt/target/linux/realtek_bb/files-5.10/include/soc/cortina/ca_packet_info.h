#ifndef __CA_PACKET_INFO_H__
#define __CA_PACKET_INFO_H__

#include <linux/types.h>

/*************************************************************************
 * PTP packet struct
 *************************************************************************/

#define PTP_SYNC			0x0
#define PTP_DELAY_REQ			0x1
#define PTP_PDELAY_REQ			0x2
#define PTP_PDELAY_RESP			0x3
#define PTP_FOLLOW_UP			0x8
#define PTP_DELAY_RESP			0x9
#define PTP_PDELAY_RESP_FOLLOW_UP	0xA
#define PTP_ANNOUNCE			0xB
#define PTP_SIGNALING			0xC
#define PTP_MANAGEMENT			0xD

enum ptp_control_field {
	PTP_CTL_SYNC,
	PTP_CTL_DELAY_REQ,
	PTP_CTL_FOLLOW_UP,
	PTP_CTL_DELAY_RESP,
	PTP_CTL_MANAGEMENT,
	PTP_CTL_OTHER,
};

/* PTP version 2; IEEE 1588-2008 */
struct ca_ptp_header {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint8_t			msg_type : 4;
	uint8_t			transport_specific : 4;
	uint8_t			version_ptp : 4;
	uint8_t			reserved0 : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	uint8_t			transport_specific : 4;
	uint8_t			msg_type : 4;
	uint8_t			reserved0 : 4;
	uint8_t			version_ptp : 4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	uint16_t		msg_len;
	uint8_t			domain_num;
	uint8_t			reserved1;
	uint16_t		flag;
	int64_t			correction_field;
	uint32_t		reserved2;
	uint8_t			clock_id[8];
	uint16_t		source_port_id;
	uint16_t		seq_id;
	uint8_t			control_field;
	int8_t			log_msg_interval;
} __packed;

/*************************************************************************
 * MACSec frame struct
 *************************************************************************/

/* copied from struct macsec_eth_header but no struct ethhdr */
struct macsec_header {
	/* SecTAG */
	u8  tci_an;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u8  short_length:6,
		  unused:2;
#elif defined(__BIG_ENDIAN_BITFIELD)
	u8        unused:2,
	    short_length:6;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	__be32 packet_number;
	u8 secure_channel_id[8]; /* optional */
} __packed;

#define MACSEC_TCI_VERSION 0x80
#define MACSEC_TCI_ES      0x40 /* end station */
#define MACSEC_TCI_SC      0x20 /* SCI present */
#define MACSEC_TCI_SCB     0x10 /* epon */
#define MACSEC_TCI_E       0x08 /* encryption */
#define MACSEC_TCI_C       0x04 /* changed text */
#define MACSEC_AN_MASK     0x03 /* association number */
#define MACSEC_TCI_CONFID  (MACSEC_TCI_E | MACSEC_TCI_C)

/*************************************************************************
 * Packet Info
 *************************************************************************/

/* For keeping small size of struct, it only contains necessary parsed info required.
 * This struct is not for dump.
 */
typedef struct {
	uint16_t l2_header_offset; /* Offset of L2 header (MAC_DA) from skb->head. */
	uint16_t l3_header_offset; /* Offset of L3 header (IPv4, IPv6, ARP) from skb->head. */
	uint16_t l4_header_offset; /* Offset of L4 header (TCP, UDP) from skb->head. Only valid when L3 is valid. */
	uint16_t l4_payload_offset; /* Offset of L4 payload. Only valid when L4 is valid. */

	/* L2 info
	 * MAC, VLAN tag, PPP header
	 */

	bool first_tag;
	__be16 first_tpid;
	__be16 first_vid; /* include PCP, DEI */

	bool second_tag;
	__be16 second_tpid;
	__be16 second_vid; /* include PCP, DEI */

	bool third_tag;
	__be16 third_tpid;
	__be16 third_vid; /* include PCP, DEI */

	__be16 ethertype;

	/* L3 info
	 * IPv4 header, IPv6 header, ARP header
	 */

	bool is_arp;
	bool is_ipv4;
	bool is_ipv6;

	bool is_inner_ipv4;
	bool is_inner_ipv6;

	bool is_frag;
	bool extlen_zero; /* IPv6 extension header length is zero */
	uint8_t ip_prot; /* IP_Protocol in IPv4 header, or last NextHeader in IPv6 header; valid when is_ipv4 or is_ipv6 is true. */

	/* L4 info
	 * TCP header, UDP header
	 */

	uint16_t l4_sport; /* valid when ip_prot is TCP, UDP, UDP-Lite */
	uint16_t l4_dport; /* valid when ip_prot is TCP, UDP, UDP-Lite */

} ca_packet_info_t;

#endif /* __CA_PACKET_INFO_H__ */
