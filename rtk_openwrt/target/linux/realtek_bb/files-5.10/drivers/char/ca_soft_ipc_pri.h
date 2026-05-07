#ifndef __CA_IPC_PRI_H__
#define __CA_IPC_PRI_H__

#include <ca_types.h>
#include <ca_ipc_pe.h>
#define CA_IPC_VERSION 	0x0200

/* IPC return code */
#define	CA_IPC_OK		CA_E_OK
#define	CA_IPC_ETIMEOUT		CA_E_TIMEOUT
#define	CA_IPC_EINVAL		CA_E_INIT
#define	CA_IPC_EEXIST		CA_E_EXISTS
#define	CA_IPC_ENOMEM		CA_E_MEMORY
#define	CA_IPC_ENOCLIENT	CA_E_NOT_FOUND
#define	CA_IPC_EQFULL		CA_E_FULL
#define	CA_IPC_EINTERNAL	CA_E_INTERNAL

/* Mailbox action_type */
#define MB_TYPE_NONE		0x0  //none
#define MB_TYPE_RQ			0x1  //request
#define MB_TYPE_READY		0x2  //ready
#define MB_TYPE_AS			0x3  //async send
#define MB_TYPE_SS			0x4  //sync send
#define MB_TYPE_ACK			0x5  //Ack
#define MB_TYPE_DONE		0x6
#define MB_TYPE_ERROR		0xFF  //error

/* message staus */
#define MESSAGE_STATUS_FREE     0x0
#define MESSAGE_STATUS_PREPARED  0x1
#define MESSAGE_STATUS_WAIT     0x2
#define MESSAGE_STATUS_BUSY     0x3

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

#if defined(CONFIG_ARCH_REALTEK_9607F)
#define IPC_CPU_NUMBER 4
#else
#define IPC_CPU_NUMBER 3
#endif
#define IPC_LIST_SIZE 0x5400
#define IPC_ITEM_SIZE 0x200

#define IPC_DEFAULT_TIMEOUT (5*HZ)

#ifndef  IPC_CPU_NUMBER
#error "CA IPC CPU number is not defined"
#endif

#ifndef  IPC_LIST_SIZE
#error "CA IPC list size is not defined"
#endif

#ifndef  IPC_ITEM_SIZE
#error "CA IPC item size in list is not defined"
#endif

#define CA_IPC_USED_MSG	0x00
#define	CA_IPC_ASYN_MSG	0x01
#define CA_IPC_SYNC_MSG	0x02
#define CA_IPC_ACK_MSG	0x03

#if IPC_LIST_SIZE > 665536
#error "Modify data type in fifl_info, msg_header, and others related"
#endif

#if defined(CONFIG_ARCH_REALTEK_9607F)
#define PER_IRQ_SOFT0_OFFSET 0x28
#define PER_IRQ_SOFT1_OFFSET 0x24
#define PER_IRQ_SOFT2_OFFSET 0x4b4
#else
#define PER_IRQ_SOFT0_OFFSET 0x18
#define PER_IRQ_SOFT1_OFFSET 0x14
#endif

#define CA_IPC_TASKLET
//#define CA_IPC_DEBUG

#ifdef CA_IPC_DEBUG
#define CA_DEBUG( fmt, args...) printk("CA_IPC:"fmt"\n", ##args)
#elif defined(CONFIG_DYNAMIC_DEBUG)
#define CA_DEBUG( fmt, args...) pr_debug("CA_IPC:"fmt"\n", ##args)
#else
#define CA_DEBUG( fmt, args...) \
	({								\
	if (0)							\
		printk("CA_IPC:"fmt"\n", ##args);	\
	})
#endif
#define CA_ERROR( fmt, args...) printk("CA_IPC:"fmt"\n", ##args)



#define DUMMY_SIZE (IPC_ITEM_SIZE  - (3*sizeof(ca_uint32)) )

struct list_ctrl {

    ca_uint32 done_offset;
    ca_uint32 current_send_offset;
    ca_uint32 version;
    ca_uint8 dummy[DUMMY_SIZE];    // aligned to IPC_ITEM_SIZE
} __attribute__ ((__packed__));





struct msg_header {
	ca_ipc_addr_t src_addr;
	ca_ipc_addr_t dst_addr;

	ca_uint16 priority:1;
	ca_uint16 ipc_flag:2;
	ca_uint16 msg_no:13;        //16 bit
	ca_uint16 payload_size;    //16 bit
	ca_uint16 trans_id;        //16 bit
	ca_uint16 next_offset;     //16 bit
} __attribute__ ((__packed__));

#define PAYLOAD_SIZE (IPC_ITEM_SIZE - sizeof(struct msg_header))
#define MAX_ITEM_NO (((IPC_LIST_SIZE - sizeof(struct list_ctrl)) / IPC_ITEM_SIZE)-1)
#define LAST_ITEM_NO	(MAX_ITEM_NO-1)

struct list_item
{
    struct msg_header msg_header;
    ca_uint8 payload[PAYLOAD_SIZE];
} __attribute__ ((__packed__));

struct list
{
	struct list_ctrl list_ctrl;
	struct list_item list_item[MAX_ITEM_NO];
	struct list_item sync_ack;
} __attribute__ ((__packed__)) list;

/* Information used by session */
struct ipc_context {
	struct list_head list;
	ca_ipc_addr_t addr;
	ca_uint16 trans_id;
	void *private_data;
	struct ca_ipc_msg *msg_procs;
	ca_uint16 msg_number;
	ca_uint16 invoke_number;
	ca_uint16 wait_trans_id;
	spinlock_t lock;
	struct completion complete;
	struct msg_header *ack_item;
	ca_uint16 ack_offset;
};

struct ipc_module {
	ca_ipc_addr_t addr;
    	struct list *root_list;
	struct list *arm_pe0;
	struct list *arm_pe1;
    struct list *pe0_arm;
	struct list *pe1_arm;
	//void __iomem * mbox_addr;
	void __iomem * irq_reg_to_pe;
	void __iomem * irq_reg_from_pe;
	unsigned int mbx_irq;
#if defined(CA_IPC_TASKLET)
	struct tasklet_struct tasklet_pe0;
	struct tasklet_struct tasklet_pe1;
#else
	struct work_struct work_pe0;
	struct work_struct work_pe1;
#endif
	ca_uint16 wait_queue0_count;
	ca_uint16 wait_queue1_count;
	spinlock_t lock;
	struct list_head session_list;
	//struct msg_header *ack_item;
	//ca_uint16 pe0_current_offset;
	//ca_uint16 pe1_current_offset;
#if defined(CONFIG_ARCH_REALTEK_9607F)
	struct list *arm_pe2;
	struct list *pe2_arm;
#if defined(CA_IPC_TASKLET)
	struct tasklet_struct tasklet_pe2;
#else
	struct work_struct work_pe2;
#endif
#endif
};


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
