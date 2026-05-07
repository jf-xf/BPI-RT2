#ifndef __CA_IPC_PRI_H__
#define __CA_IPC_PRI_H__

#include <ca_types.h>
#define CA_IPC_VERSION 	0x0100

/* IPC return code */
#define	CA_IPC_OK		CA_E_OK
#define	CA_IPC_ETIMEOUT		CA_E_TIMEOUT
#define	CA_IPC_EINVAL		CA_E_INIT
#define	CA_IPC_EEXIST		CA_E_EXISTS
#define	CA_IPC_ENOMEM		CA_E_MEMORY
#define	CA_IPC_ENOCLIENT	CA_E_NOT_FOUND
#define	CA_IPC_EQFULL		CA_E_FULL
#define	CA_IPC_EINTERNAL	CA_E_INTERNAL

/* Message Priority */
#define CA_IPC_LPRIO		0x0
#define CA_IPC_HPRIO		0x1

/* CPU_ID */
#define CPU_ARM			0x0
#define CPU_RCPU0		0x1
#define CPU_RCPU1		0x2
#if defined(CONFIG_ARCH_REALTEK_9607F)
#define CPU_RCPU2		0x3
#endif

/* Client ID */
#define CA_CLNT_ID_WFO_ARM	0x0
#define CA_CLNT_ID_WFO_PE0	0x1
#define CA_CLNT_ID_WFO_PE1	0x2
#if defined(CONFIG_ARCH_REALTEK_9607F)
#define CA_CLNT_ID_WFO_PE2	0x3
#endif
#define CA_CLNT_ID_TUNNEL_PE0	0x3
#define CA_CLNT_ID_TUNNEL_PE1	0x4


//struct ipc_addr {
//	ca_uint8	client_id;
//	ca_uint8	cpu_id;
//} __attribute__((__packed__));

struct ipc_context;
//typedef int (*ipc_msg_proc)( struct ipc_addr peer, ca_uint16 msg_no,
//		const void *msg_data, ca_uint16 msg_size,
//		struct ipc_context* context);


struct ca_ipc_msg {
	ca_uint8 msg_no;
	unsigned long proc;
};

//int ca_ipc_msg_handle_register( ca_uint8 client_id, const struct ca_ipc_msg *msg_procs,
//	       	ca_uint16 msg_count, ca_uint16 invoke_count, void *private_data,
//	       	struct ipc_context **context);

//int ca_ipc_send( struct ipc_context *context, ca_uint8 cpu_id,
//		ca_uint8 client_id, ca_uint8 priority, ca_uint16 msg_no,
//		const void *msg_data, ca_uint16 msg_size);


#define _CA_IPC_SYNC__
#ifdef _CA_IPC_SYNC__

//typedef int (*ipc_invoke_proc)( struct ipc_addr peer, ca_uint16 msg_no,
//		const void *msg_data, ca_uint16 msg_size,
//		void *result_data, ca_uint16 *result_data_size,
//		struct ipc_context* context );

//int ca_ipc_invoke( struct ipc_context *context, ca_uint8 cpu_id,
//		ca_uint8 client_id, ca_uint8 priority, ca_uint16 msg_no,
//		const void *msg_data, ca_uint16 msg_size,
//		void *result_data, ca_uint16 *result_size );
#endif

//int ca_ipc_msg_handle_unregister(struct ipc_context *context);
void ca_ipc_print_status(ca_uint8 cpu_id);
void ca_ipc_reset_list_info(ca_uint8 cpu_id);

#endif /* __CA_IPC_PRI_H__ */
