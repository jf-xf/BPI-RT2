#ifndef __CA_IPC_PE_H__
#define __CA_IPC_PE_H__

#include <ca_types.h>

typedef enum {
	CA_IPC_SESSION_VPN 				= 1,
	CA_IPC_SESSION_WFO 				= 2,
	CA_IPC_SESSION_PKT_SHAPER 		= 3,
	CA_IPC_SESSION_SW_RATE_LIMITER	= 4,
	CA_IPC_SESSION_MACSEC 			= 5,
    CA_IPC_SESSION_WMC_FLOODING 	= 6,
	CA_IPC_SESSION_FDB				= 7,
    CA_IPC_SESSION_SYSTEM			= 8,
    CA_IPC_SESSION_HTTP_TEST		= 9,
    CA_IPC_SESSION_CRYPTO_ENGINE 	= 10,
	CA_IPC_SESSION_RTL_WFO_5G  		= 11,
    CA_IPC_SESSION_RTL_WFO_6G 		= 12,
	CA_IPC_SESSION_RTL_WFO_LOOP 	= 13,
	CA_IPC_SESSION_DMALSO 			= 14,
	CA_IPC_SESSION_MAX,
} ca_ipc_session_id_t;

typedef enum {
	CA_IPC_CPU_ARM = 0,
	CA_IPC_CPU_PE0 = 1,
	CA_IPC_CPU_PE1 = 2,
	CA_IPC_CPU_PE2 = 3,
	CA_IPC_CPU_MAX,
} ca_ipc_cpu_id_t;

typedef enum {
	CA_IPC_PRIO_LOW = 0,
	CA_IPC_PRIO_HIGH = 1,
} ca_ipc_priority_t;

typedef struct {
	ca_ipc_session_id_t	session_id;
	ca_ipc_cpu_id_t		cpu_id;
} __attribute__ ((__packed__)) ca_ipc_addr_t;


typedef int (*ca_ipc_msg_proc_t) (ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id, const void *msg_data, ca_uint16_t *msg_size);


typedef struct {
	ca_uint16_t msg_no;
	ca_uint16_t reserved; /* 'proc' is callback function pointer, it must be 32-bit aligned. */
	ca_ipc_msg_proc_t proc;
} __attribute__ ((__packed__)) ca_ipc_msg_handle_t;

typedef struct {
	ca_ipc_session_id_t session_id; /* the session_id */
	ca_ipc_cpu_id_t dst_cpu_id; /* the destination cpu_id */
	ca_ipc_priority_t priority; /* the priority of message*/
	/* the message number within the session.
	 * This is customized message number.
	 * The destination CPU will be based on this number
	 * to look for the message handler.
	 */
	ca_uint16_t msg_no;

	const void * msg_data; /* the message data to transmit. */
	ca_uint16_t msg_size; /* the length of message data. */
} ca_ipc_pkt_t; //ipc_send_data_t


ca_status_t ca_ipc_msg_handle_register(ca_ipc_session_id_t session_id, const ca_ipc_msg_handle_t *msg_handle_array, ca_uint32_t msg_handle_count);
ca_status_t ca_ipc_msg_handle_unregister(ca_ipc_session_id_t session_id);
ca_status_t ca_ipc_msg_async_send(ca_ipc_pkt_t *p_ipc_pkt);
ca_status_t ca_ipc_msg_sync_send(ca_ipc_pkt_t *p_ipc_pkt, void *result_data, ca_uint16_t *result_size);

#endif /* __CA_IPC_PE_H__ */
