#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/kd.h>
#include <linux/slab.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>
#include <linux/tiocl.h>
#include <linux/kbd_kern.h>
#include <linux/consolemap.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/pm.h>
#include <linux/font.h>
#include <linux/bitops.h>
#include <linux/notifier.h>
#include <linux/device.h>
#include <linux/completion.h>
#include <asm/io.h>
#include <linux/delay.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include <ca_types.h>
#include "ca_ipc_pri.h"
#include <ca_ipc_pe.h>
#include <registers.h>
#include <ca_hwsem.h>

MODULE_LICENSE("GPL");

#if defined(CONFIG_ARCH_RTL8198F)
/* 98F only has 1 A17 and 1 Taroko */
#define IPC_CPU_NUMBER 2
#define IPC_LIST_SIZE 6144
#define IPC_ITEM_SIZE 128
#define IPC_INT_BASE_OFFSET	0xc
#define IPC_INT_SOURCE_EN	0x3f
#elif defined(CONFIG_ARCH_REALTEK_9607F)
#define IPC_CPU_NUMBER 4
#define IPC_LIST_SIZE 6144
#define IPC_ITEM_SIZE 128
#define IPC_INT_BASE_OFFSET	0x10
#define IPC_INT_SOURCE_EN	0x7f
#else
#define IPC_CPU_NUMBER 3
#define IPC_LIST_SIZE 6144
#define IPC_ITEM_SIZE 128
#define IPC_INT_BASE_OFFSET	0xc
#define IPC_INT_SOURCE_EN	0x3f
#endif
#define IPC_DEFAULT_TIMEOUT (HZ)

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

#define CA_IPC_MEM_RESOURCE_MBX_REGS    0
#define CA_IPC_MEM_RESOURCE_MBX_REGS    0
#define CA_IPC_MEM_RESOURCE_SHAREMEM    1

//#define CA_IPC_DEBUG

#ifdef CA_IPC_DEBUG
#define CA_DEBUG( fmt, args...) printk("CA_IPC:"fmt"\n", ##args)
#else
#define CA_DEBUG( fmt, args...)
#endif
#define CA_ERROR( fmt, args...) printk("CA_IPC:"fmt"\n", ##args)


//#define CA_IPC_TEST
#define CA_IPC_LOCK(l, f)   spin_lock_irqsave(l, f)
#define CA_IPC_UNLOCK(l, f) spin_unlock_irqrestore(l, f)

struct fifo_info {
	ca_uint16 first;
	ca_uint16 last;
} __attribute__ ((__packed__));

#define DUMMY_SIZE (IPC_ITEM_SIZE - 2*sizeof(struct fifo_info) - sizeof(ca_uint16) - sizeof(ca_uint32))

struct list_info {
	struct fifo_info low_fifo;	// low priority
	struct fifo_info high_fifo;	// high priority

	ca_uint16 free_offset;	// free list
	ca_uint8 dummy[DUMMY_SIZE]; // aligned to IPC_ITEM_SIZE ;
	ca_uint32 version;
} __attribute__ ((__packed__));

struct msg_header {
	ca_ipc_addr_t src_addr;
	ca_ipc_addr_t dst_addr;

	ca_uint16 priority:1;
	ca_uint16 ipc_flag:2;
	ca_uint16 msg_no:13;
	ca_uint16 payload_size;
	ca_uint16 trans_id;
	ca_uint16 next_offset;
} __attribute__ ((__packed__));

#define PAYLOAD_SIZE (IPC_ITEM_SIZE - sizeof(struct msg_header))
#define MAX_ITEM_NO ((IPC_LIST_SIZE - sizeof(struct list_info)) / IPC_ITEM_SIZE)

struct list_item
{
    struct msg_header msg_header;
    ca_uint8 payload[PAYLOAD_SIZE];
} __attribute__ ((__packed__));

struct list
{
    struct list_info list_info;
    struct list_item list_item[MAX_ITEM_NO];
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
};

struct IPC_Module {
	ca_ipc_addr_t addr;
    struct list *root_list;
    struct list *this_list;
	void __iomem * mbox_addr;
	unsigned int mbx_irq;
	struct work_struct work;
	struct fifo_info wait_queue;
	spinlock_t lock;
	struct list_head session_list;
};

static struct ipc_context *findSession(struct IPC_Module *context,
				      ca_uint8 session_id)
{
	struct list_head *ptr;
	struct ipc_context *session;

	list_for_each(ptr, &context->session_list) {
		session = list_entry(ptr, struct ipc_context, list);
		if (session->addr.session_id == session_id) {
			return session;
		}
	}

	return NULL;
}

static struct IPC_Module *module_context;

void dumpmem(int * ptr, int size)
{
	int i;

	for(i = 0; i < size; i+=4)
	{
		if(i%16 == 0)
		{
			printk("\n0x%lx: ", (unsigned long)ptr);
		}
		printk("0x%x ",*ptr );
		ptr++;
	}
	printk("\n");
}
/***************************************************
 * Target CPU Always is Core0
 ***************************************************/
static void _ipc_raise_int(ca_uint8 source_cpu, ca_uint8 target_cpu)
{
	void __iomem * reg_addr;
	ca_uint32 reg_val;

	reg_addr = module_context->mbox_addr;

	switch (source_cpu)
	{
        case  CPU_ARM:
                reg_addr = reg_addr ;
                break;
        case  CPU_RCPU0:
                reg_addr = reg_addr + IPC_INT_BASE_OFFSET*4;
                break;
        case  CPU_RCPU1:
                reg_addr = reg_addr + IPC_INT_BASE_OFFSET*5;
                break;
        case  CPU_RCPU2:
                reg_addr = reg_addr + IPC_INT_BASE_OFFSET*6;
                break;
        default:
                CA_ERROR("No such source CPU ID:%d", source_cpu);
	}

	switch (target_cpu)
	{
		case  CPU_ARM:
			reg_val = 0x00000001 ;
			break;
		case  CPU_RCPU0:
			reg_val = 0x00000010 ;
			break;
		case  CPU_RCPU1:
			reg_val = 0x00000020 ;
			break;
		case  CPU_RCPU2:
			reg_val = 0x00000040 ;
			break;
		default:
			CA_ERROR("No such targeted CPU ID:%d", target_cpu);
	}

	writel(reg_val, reg_addr);
}

static void _ipc_clear_int(void)
{
	void __iomem * reg_status_addr;
	ca_uint32 reg_status_val;
	ca_uint8 cpu_id = module_context->addr.cpu_id;

	/* Obtain the interrupts status register */
	reg_status_addr = module_context->mbox_addr ;//+ 0x4 + (module_context->addr.cpu_id * IPC_INT_BASE_OFFSET);

    switch (cpu_id)
    {
        case  CPU_ARM:
                reg_status_addr = reg_status_addr+0x4;
                break;
        case  CPU_RCPU0:
                reg_status_addr = reg_status_addr+0x4 + IPC_INT_BASE_OFFSET*4;
                break;
        case  CPU_RCPU1:
                reg_status_addr = reg_status_addr+0x4 + IPC_INT_BASE_OFFSET*5;
                break;
        case  CPU_RCPU2:
                reg_status_addr = reg_status_addr+0x4 + IPC_INT_BASE_OFFSET*6;
                break;
        default:
            CA_ERROR("%s :No such CPU ID:%d", __FUNCTION__,cpu_id);
    }

	reg_status_val = readl(reg_status_addr);

	/* Issued from ARM core0 */
	if(reg_status_val & 0x00000001)
	{
		writel(0x00000001, reg_status_addr);
	}
	/* Issued from Taroko DP0 */
	else if (reg_status_val & 0x00000010)
	{
		writel(0x00000010, reg_status_addr);
	}
	/* Issued from Taroko DP1 */
	else if (reg_status_val & 0x00000020)
	{
		writel(0x00000020, reg_status_addr);
	}
	/* Issued from Taroko DP2 */
	else if (reg_status_val & 0x00000040)
	{
		writel(0x00000040, reg_status_addr);
	}
	else
	{
		CA_ERROR("No such interrupt status:0x%x", reg_status_val);
	}
}

static void _ipc_enable_int(ca_uint8 cpu_id)
{
	void __iomem * reg_en_addr;
	reg_en_addr = module_context->mbox_addr;
    switch (cpu_id)
    {
        case  CPU_ARM:
                reg_en_addr = reg_en_addr+0x8;
                break;
        case  CPU_RCPU0:
                reg_en_addr = reg_en_addr+0x8 + IPC_INT_BASE_OFFSET*4;
                break;
        case  CPU_RCPU1:
                reg_en_addr = reg_en_addr+0x8 + IPC_INT_BASE_OFFSET*5;
                break;
        case  CPU_RCPU2:
                reg_en_addr = reg_en_addr+0x8 + IPC_INT_BASE_OFFSET*6;
                break;
        default:
            CA_ERROR("%s :No such CPU ID:%d", __FUNCTION__,cpu_id);
    }

	/* Obtain the interrupts enable register */
	//reg_en_addr = module_context->mbox_addr + 0x8 + (module_context->addr.cpu_id * IPC_INT_BASE_OFFSET);

	CA_DEBUG("_ipc_enable_int 0x%x 0x%x \n",reg_en_addr,IPC_INT_SOURCE_EN);
	writel(IPC_INT_SOURCE_EN, reg_en_addr);
}

static void initial_list_info(ca_uint8 cpu_id)
{
	ca_uint16 offset;
	unsigned int i, max_item_no;
	struct list_info *list;
	struct msg_header *ptr;
	unsigned long flags;

	max_item_no = (IPC_LIST_SIZE - sizeof(struct list_info)) /
	    IPC_ITEM_SIZE;

	offset = sizeof(struct list_info);

	for (i = 0; i < max_item_no; ++i) {
		ptr = (struct msg_header *)((unsigned long)(&module_context->root_list[cpu_id]) + offset);
		offset += IPC_ITEM_SIZE;
		ptr->next_offset = offset;
	}
	ptr->next_offset = 0;

	list = &(module_context->root_list[cpu_id].list_info);

	CA_IPC_LOCK(&module_context->lock, flags);

	list->free_offset = sizeof(struct list_info);
	list->low_fifo.first = list->low_fifo.last =
	    list->high_fifo.first = list->high_fifo.last = 0;
	list->version = CA_IPC_VERSION;

	CA_IPC_UNLOCK(&module_context->lock, flags);

	return;
}

void ca_ipc_print_status(ca_uint8 cpu_id)
{
	ca_uint16 offset;
	unsigned int i, max_item_no;
	struct list_info *list;
	struct msg_header *ptr;
	unsigned long flags;

	/* Print list info */
	list = &(module_context->root_list[cpu_id].list_info);
	CA_IPC_LOCK(&module_context->lock, flags);

	printk("========= Dump IPC status of cpu %d =========\n", cpu_id);
	printk("free offset = %d\n", list->free_offset);
	printk("hi fifo from %d to %d\n", list->high_fifo.first, list->high_fifo.last);
	printk("low fifo from %d to %d\n", list->low_fifo.first, list->low_fifo.last);

	CA_IPC_UNLOCK(&module_context->lock, flags);

	/* Print next offset */
	max_item_no = (IPC_LIST_SIZE - sizeof(struct list_info)) /
	    IPC_ITEM_SIZE;

	offset = sizeof(struct list_info);

	printk("---- print each msg ----\n");
	for (i = 0; i < max_item_no; ++i) {
		ptr = (struct msg_header *)((unsigned long)list + offset);
		printk("%02d: offset = %d, next offset = %d\n", i, offset, ptr->next_offset);
		offset += IPC_ITEM_SIZE;
	}

	return;
}

void ca_ipc_reset_list_info(ca_uint8 cpu_id)
{
	initial_list_info(cpu_id);

	return;
}

ca_status_t _ipc_msg_handle_register(ca_uint8 session_id, const struct ca_ipc_msg *msg_procs,
		    ca_uint16 msg_count, ca_uint16 invoke_count,
		    void *private_data)
{
	struct ipc_context *session;
	ca_uint16 total_procs_no;

	total_procs_no = msg_count + invoke_count;
	if (0 == session_id || (0 != total_procs_no && NULL == msg_procs) ||
	    (0 == total_procs_no && NULL != msg_procs)) {
		CA_ERROR("register parameters error:session[%d]", session_id);
		return CA_IPC_EINVAL;
	}

	session = findSession(module_context, session_id);
	if (NULL != session) {
		CA_ERROR("has the session:id[%d]", session_id);
		return CA_IPC_EEXIST;
	}

	session = kmalloc(sizeof(struct ipc_context), GFP_KERNEL);
	if (NULL == session) {
		CA_ERROR("malloc failed:session[%d]", session_id);
		return CA_IPC_ENOMEM;
	}

	if (0 == total_procs_no) {
		session->msg_procs = NULL;
	} else {
		session->msg_procs =
		    (struct ca_ipc_msg *)kmalloc(sizeof(struct ca_ipc_msg) *
						 total_procs_no, GFP_KERNEL);

		if (NULL == session->msg_procs) {
			kfree(session->msg_procs);
			kfree(session);
			CA_ERROR("malloc failed:session[%d]", session_id);
			return CA_IPC_ENOMEM;
		}

		memcpy(session->msg_procs, msg_procs,
		       sizeof(struct ca_ipc_msg) * total_procs_no);
	}

	session->msg_number = msg_count;
	session->invoke_number = invoke_count;
	session->addr.cpu_id = module_context->addr.cpu_id;
	session->addr.session_id = session_id;
	session->trans_id = 0;
	session->private_data = private_data;
	session->wait_trans_id = 0;
	session->ack_item = NULL;

	spin_lock_init(&session->lock);
	init_completion(&session->complete);
	INIT_LIST_HEAD(&session->list);

	list_add(&session->list, &(module_context->session_list));
	//*context = session;

	CA_ERROR("register succesfull:session_id[%d] session %p msg_count %d \n",
		session_id, session, msg_count);
	return CA_IPC_OK;
}
#define MSG_HANDLE_COUNT_MAX	64
ca_status_t ca_ipc_msg_handle_register(ca_ipc_session_id_t session_id,
	const ca_ipc_msg_handle_t * msg_handle_array,ca_uint32_t msg_handle_count)
{
    ca_status_t result;
    //struct ca_ipc_msg msg_handle[128];
	struct ca_ipc_msg msg_handle[MSG_HANDLE_COUNT_MAX];
	ca_uint16 i;

	for (i = 0; (i< msg_handle_count) && (i<MSG_HANDLE_COUNT_MAX); i++) {
    	msg_handle[i].msg_no =(ca_uint8) msg_handle_array[i].msg_no;
	    msg_handle[i].proc = (unsigned long)msg_handle_array[i].proc;
	}
	result = _ipc_msg_handle_register((ca_uint8)session_id,msg_handle,msg_handle_count,0,NULL);

	return result;
}

EXPORT_SYMBOL(ca_ipc_msg_handle_register);


ca_status_t _ipc_msg_handle_unregister(struct ipc_context *context)
{
	struct list_head *ptr;
	struct ipc_context *session;
	ca_uint8 session_id;

	if (NULL == context) {
		CA_ERROR("%s parameters invalid", __func__);
	}

	session_id = context->addr.session_id;

	list_for_each(ptr, &module_context->session_list) {
		session = list_entry(ptr, struct ipc_context, list);
		if (session == context) {
			list_del(ptr);
			kfree(session->msg_procs);
			kfree(session);

			CA_DEBUG("session deregister:session[%d]", session_id);
			return CA_IPC_OK;
		}
	}

	CA_ERROR("No such session:session[%d]", session_id);
	return CA_IPC_ENOCLIENT;
}

ca_status_t ca_ipc_msg_handle_unregister(ca_ipc_session_id_t session_id)
{
    struct ipc_context* session;
    ca_status_t result;

    session = findSession(module_context, session_id);
	if (NULL == session) {
		CA_ERROR("IPC has no the session:id[%d]", session_id);
		return CA_IPC_EEXIST;
	}

    result =_ipc_msg_handle_unregister(session);

    return result;

}

EXPORT_SYMBOL(ca_ipc_msg_handle_unregister);

static ca_status_t check_client_ready(ca_uint8 cpu_id)
{
	struct list_info *list;
	ca_status_t result = CA_IPC_OK;
	list = &(module_context->root_list[cpu_id].list_info);
	//printk("IPC:  check_client_ready list is %lx\n",list);
	//printk("IPC:  check_client_ready &&version is %lx\n",&(module_context->this_list->list_info.version));
	if (list->version == 0xFFFFFFFF)
	{
		printk("CA_IPC error : plaese check co-processer alredy be booted\n");
		result = CA_IPC_ENOCLIENT;
	}
	return result;
}

static unsigned long get_list_item(ca_uint8 cpu_id)
{
	ca_uint16 offset;
	struct list_info *list;
	struct msg_header *tmp;
	unsigned long flags;

	offset = 0;
	list = &(module_context->root_list[cpu_id].list_info);

	CA_IPC_LOCK(&module_context->lock, flags);

	/* Protect DS using MB0, to manipulate free_list */
	ca_sem_lock(cpu_id, 0);

	if (list->free_offset != 0) {
		offset = list->free_offset;
		tmp = (struct msg_header *)((unsigned long)list + offset);
		list->free_offset = tmp->next_offset;
	}
	ca_sem_unlock(cpu_id, 0);

	CA_IPC_UNLOCK(&module_context->lock, flags);

	return offset;
}

static void free_list_item(ca_uint8 cpu_id, struct msg_header *item)
{
	unsigned long offset;
	struct list_info *list;
	unsigned long flags;

	list = &(module_context->root_list[cpu_id].list_info);
	offset = (unsigned long)item - (unsigned long)list;

	if (offset < sizeof(struct list_info) || IPC_LIST_SIZE < offset) {
		CA_ERROR("Something is wrong. Check it");
		return;
	}

	CA_IPC_LOCK(&module_context->lock, flags);

	/* Protect DS using MB0, to manipulate free_list */
	ca_sem_lock(cpu_id, 0);

	item->next_offset = list->free_offset;
	list->free_offset = offset;

	ca_sem_unlock(cpu_id, 0);

	CA_IPC_UNLOCK(&module_context->lock, flags);
}

static int fill_header(struct ipc_context *context, ca_uint8 cpu_id,
		       ca_uint8 session_id, ca_uint8 ipc_flag, ca_uint8 priority,
		       ca_uint16 msg_no, ca_uint16 msg_size,
		       struct msg_header *header)
{
	if (NULL == context || IPC_CPU_NUMBER <= cpu_id ||
	    IPC_ITEM_SIZE < (msg_size + sizeof(struct msg_header))) {

		CA_ERROR("Invalid Parameter");
		return CA_IPC_EINVAL;
	}

	header->src_addr = context->addr;
	header->dst_addr.cpu_id = cpu_id;
	header->dst_addr.session_id = session_id;
	header->ipc_flag = ipc_flag;
	header->priority = priority;
	header->msg_no = msg_no;
	header->payload_size = msg_size;

	context->trans_id += 1;
	header->trans_id = context->trans_id;

	return CA_IPC_OK;
}

static void write_dest_list(ca_uint8 cpu_id, ca_uint8 priority,
			    ca_uint16 offset)
{
	struct fifo_info *fifo;
	struct list_info *dest_list;
	struct msg_header *tmp;
	unsigned long flags;
	ca_uint8 source_cpu;
	ca_uint8 target_cpu;

	if (offset < sizeof(struct list_info) || IPC_LIST_SIZE < offset) {
		CA_ERROR("Invalid [%u]. Something wrong. Check it.", offset);
		return;
	}

	dest_list = &(module_context->root_list[cpu_id].list_info);
	tmp = (struct msg_header *)((unsigned long)dest_list + offset);
	tmp->next_offset = 0;
	source_cpu = tmp->src_addr.cpu_id;
	target_cpu = tmp->dst_addr.cpu_id;

	if (CA_IPC_HPRIO == priority) {
		fifo = &dest_list->high_fifo;
	} else {
		fifo = &dest_list->low_fifo;
	}

	/* Protect DS using MB0, to manipulate high_fifo & low_fifo */
	CA_IPC_LOCK(&module_context->lock, flags);
	ca_sem_lock(cpu_id, 0);

	if (0 == fifo->last) {
		fifo->first = fifo->last = offset;
	} else {
		tmp = (struct msg_header *)((unsigned long)(&module_context->root_list[cpu_id]) + fifo->last);
		fifo->last = tmp->next_offset = offset;
	}

	//Send Event to Destination CPU
    _ipc_raise_int(source_cpu, target_cpu);

	ca_sem_unlock(cpu_id, 0);
	CA_IPC_UNLOCK(&module_context->lock, flags);
}

static int ipc_send_msg(struct ipc_context *context, ca_uint8 cpu_id,
			ca_uint8 session_id, ca_uint8 ipc_flag,
			ca_uint8 priority, ca_uint16 msg_no,
			const void *msg_data, ca_uint16 msg_size)
{
	unsigned rc;
	ca_uint16 offset;
	unsigned long send_timeout;
	struct msg_header *header;

	if(check_client_ready(cpu_id) != CA_IPC_OK)
	{
		return CA_IPC_ENOCLIENT;
	}

	send_timeout = jiffies + IPC_DEFAULT_TIMEOUT;

	do {
		offset = get_list_item(cpu_id);
		if (0 != offset) {
			break;
		}
		//schedule();
	} while (time_before(jiffies, send_timeout));

	if (time_after_eq(jiffies, send_timeout)) {
		return CA_IPC_EQFULL;
	}

	header = (struct msg_header *)((unsigned long)(&module_context->root_list[cpu_id]) + offset);
	rc = fill_header(context, cpu_id, session_id, ipc_flag, priority, msg_no, msg_size, header);

	if (rc != CA_IPC_OK) {
		free_list_item(cpu_id, header);
		return rc;
	}

	if (CA_IPC_SYNC_MSG == ipc_flag) {
		context->wait_trans_id = header->trans_id;
	}

	memcpy_toio(header + 1, msg_data, msg_size);

	write_dest_list(cpu_id, priority, offset);
	CA_DEBUG("Send Message to [%d:%d] OK", cpu_id, session_id);

	return CA_IPC_OK;
}

ca_status_t ca_ipc_msg_async_send(ca_ipc_pkt_t* p_ipc_pkt)
{
    struct ipc_context *sender;
    sender = findSession(module_context, p_ipc_pkt->session_id);

    if(p_ipc_pkt->msg_size>PAYLOAD_SIZE)
    {
        CA_ERROR("ca_ipc_msg_async_send : message size[%d] more than PAYLOAD_SIZE[%ld]\n",p_ipc_pkt->msg_size,PAYLOAD_SIZE);
		return CA_IPC_EINVAL;
    }

    if (NULL == sender) {
		CA_ERROR("ca_ipc_msg_async_send : No Such session_id %d\n",p_ipc_pkt->session_id);
		return CA_IPC_ENOCLIENT;
	}

    return ipc_send_msg(sender, p_ipc_pkt->dst_cpu_id , p_ipc_pkt->session_id , CA_IPC_ASYN_MSG,
			p_ipc_pkt->priority , p_ipc_pkt->msg_no , p_ipc_pkt->msg_data , p_ipc_pkt->msg_size);
}
EXPORT_SYMBOL(ca_ipc_msg_async_send);

ca_status_t ca_ipc_msg_sync_send(ca_ipc_pkt_t* p_ipc_pkt,void * result_data,ca_uint16_t * result_size)
{
    struct ipc_context *sender;
	struct msg_header *ack_header;
	ca_status_t rc;
    sender = findSession(module_context, p_ipc_pkt->session_id);

    if(p_ipc_pkt->msg_size>PAYLOAD_SIZE)
    {
        CA_ERROR("ca_ipc_msg_sync_send : message size[%d] more than PAYLOAD_SIZE[%ld]\n",p_ipc_pkt->msg_size,PAYLOAD_SIZE);
		return CA_IPC_EINVAL;
    }

    if (NULL == sender) {
		CA_ERROR("ca_ipc_msg_sync_send : No Such session_id %d\n",p_ipc_pkt->session_id);
		return CA_IPC_ENOCLIENT;
	}

	init_completion(&sender->complete);

    rc =  ipc_send_msg(sender, p_ipc_pkt->dst_cpu_id , p_ipc_pkt->session_id , CA_IPC_SYNC_MSG,
			p_ipc_pkt->priority , p_ipc_pkt->msg_no , p_ipc_pkt->msg_data , p_ipc_pkt->msg_size);

    if (CA_IPC_OK != rc) {
		return rc;
	}
	CA_DEBUG("%s session %p\n",__func__, sender);

	rc = wait_for_completion_timeout(&sender->complete,
					       IPC_DEFAULT_TIMEOUT);
	sender->wait_trans_id = 0;

	if (0 == rc) {
		CA_ERROR("ca_ipc_msg_sync_send timeout\n");
		return CA_IPC_EINTERNAL;
	}

	rc = CA_IPC_EINTERNAL;

	if (sender->ack_item != NULL) {
		if (CA_IPC_ACK_MSG == sender->ack_item->ipc_flag) {
			ack_header = sender->ack_item;

			if (0 < ack_header->payload_size) {
				if (ack_header->payload_size <= *result_size) {
					*result_size = ack_header->payload_size;
					memcpy_fromio(result_data, ack_header + 1,
							*result_size);
					rc = CA_IPC_OK;
				} else {
					CA_ERROR("Buffer is too small");
					rc = CA_IPC_ENOMEM;
				}
			} else {
				*result_size = 0;
				rc = CA_IPC_OK;
			}
		}
	}else
	{
		CA_ERROR("ca_ipc_msg_sync_send error %d\n",__LINE__);
	}

	free_list_item(sender->addr.cpu_id, sender->ack_item);
	sender->ack_item = NULL;

	return rc;
}
EXPORT_SYMBOL(ca_ipc_msg_sync_send);

#ifdef _CA_IPC_SYNC__

int ca_ipc_invoke(struct ipc_context *context, ca_uint8 cpu_id,
		  ca_uint8 session_id, ca_uint8 priority, ca_uint16 msg_no,
		  const void *msg_data, ca_uint16 msg_size,
		  void *result_data, ca_uint16 * result_size)
{
	int rc;
	unsigned long flags;
	struct msg_header *ack_header;

	CA_IPC_LOCK(&context->lock, flags);

	rc = ipc_send_msg(context, cpu_id, session_id, CA_IPC_SYNC_MSG,
			  priority, msg_no, msg_data, msg_size);

	if (CA_IPC_OK != rc) {
		CA_IPC_UNLOCK(&context->lock, flags);
		return rc;
	}

	init_completion(&context->complete);
	rc = wait_for_completion_interruptible_timeout(&context->complete,
						       IPC_DEFAULT_TIMEOUT);

	context->wait_trans_id = 0;

	if (0 > rc) {
		CA_IPC_UNLOCK(&context->lock, flags);
		CA_ERROR("Internal Error %d", rc);
		return CA_IPC_EINTERNAL;
	}

	if (0 == rc) {
		CA_IPC_UNLOCK(&context->lock, flags);
		CA_ERROR("Invoke timeout: session[%d:%d] call session[%d:%d]",
			 context->addr.cpu_id, context->addr.session_id,
			 cpu_id, session_id);

		return CA_IPC_ETIMEOUT;
	}

	rc = CA_IPC_EINTERNAL;
	if (CA_IPC_ACK_MSG == context->ack_item->ipc_flag) {
		ack_header = context->ack_item;

		if (0 < ack_header->payload_size) {
			if (ack_header->payload_size <= *result_size) {
				*result_size = ack_header->payload_size;
				memcpy_fromio(result_data, ack_header + 1,
				       *result_size);
				rc = CA_IPC_OK;
			} else {
				CA_ERROR("Buffer is too small");
				rc = CA_IPC_ENOMEM;
			}
		} else {
			*result_size = 0;
			rc = CA_IPC_OK;
		}
	}

	free_list_item(context->addr.cpu_id, context->ack_item);
	CA_IPC_UNLOCK(&context->lock, flags);

	return rc;
}

#endif

static unsigned long find_callback(struct ca_ipc_msg *messages,
				   ca_uint16 msg_number,
				   ca_uint16 search_target)
{
	unsigned i;

	for (i = 0; i < msg_number; ++i) {
		if (messages[i].msg_no == search_target) {
			return messages[i].proc;
		}
	}
	return 0;
}

static int do_asyn_message(struct ipc_context *session,
			   struct msg_header *header)
{
	unsigned long addr;
	ca_ipc_msg_proc_t callback;

	addr = find_callback(session->msg_procs, session->msg_number,
			     header->msg_no);
	if (0 == addr) {
		CA_ERROR("do_asyn_message : No intestesd message:"
			 "sender[%d:%d] msg_no[%d] receiver[%d:%d]",
			 header->src_addr.cpu_id,
			 header->src_addr.session_id, header->msg_no,
			 session->addr.cpu_id, session->addr.session_id);
	} else {
		callback = (ca_ipc_msg_proc_t) addr;
		callback(header->src_addr,header->msg_no, header->trans_id ,header + 1,
				&(header->payload_size));
	}
	free_list_item(session->addr.cpu_id, header);

	return 0;
}

#ifdef _CA_IPC_SYNC__

static int do_sync_message(struct ipc_context *session,
			   struct msg_header *header)
{
	unsigned long addr,offset;
	ca_ipc_msg_proc_t callback;
	struct msg_header *cb_buffer;
	CA_DEBUG("IPC debug ARM: do_syn_message\n");
	addr = find_callback(session->msg_procs, session->msg_number,
			     header->msg_no);
	if (0 == addr) {
		CA_ERROR("do_sync_message : No intestesd message:"
			 "sender[%d:%d] msg_no[%d] receiver[%d:%d]",
			 header->src_addr.cpu_id,
			 header->src_addr.session_id, header->msg_no,
			 session->addr.cpu_id, session->addr.session_id);
	} else {
		callback = (ca_ipc_msg_proc_t) addr;
		callback(header->src_addr,header->msg_no, header->trans_id ,header + 1,
				&(header->payload_size));
	}

	if(header->payload_size > PAYLOAD_SIZE)
	{
		printk("IPC Error message size big than PAYLOAD_SIZE :"
			 "sender[%d:%d] msg_no[%d] receiver[%d:%d]\n",
			 header->src_addr.cpu_id,
			 header->src_addr.session_id, header->msg_no,
			 session->addr.cpu_id, session->addr.session_id);
		goto cleanup;
	}

	offset = get_list_item(header->src_addr.cpu_id);
	if (0 == offset) {
		goto cleanup;
	}

	cb_buffer = (struct msg_header *)((unsigned long)(&module_context->root_list[header->src_addr.cpu_id]) + offset);
	cb_buffer->payload_size = header->payload_size;//IPC_ITEM_SIZE - sizeof(struct msg_header);

	memcpy(cb_buffer+1,header + 1,cb_buffer->payload_size);

	fill_header(session, header->src_addr.cpu_id,
	    header->src_addr.session_id, CA_IPC_ACK_MSG,
	    header->priority, header->trans_id,
	    cb_buffer->payload_size, cb_buffer);

	write_dest_list(header->src_addr.cpu_id, header->priority, offset);

cleanup:
	free_list_item(session->addr.cpu_id, header);

	return 0;
}

static int do_ack_message(struct ipc_context *session, struct msg_header *header)
{
	#if 0
	int rc;
	unsigned long offset;
	#endif
	if (NULL == session || NULL == header) {
		CA_ERROR("session %p ack message %p", session, header);
		return 0;
	}

	#if 0

	//CA_IPC_LOCK(&session->lock, flags);
	rc = (session->wait_trans_id != header->msg_no);
	//CA_IPC_UNLOCK(&session->lock, flags);

	if (rc) {
		CA_ERROR("do_ack_message :No intestesd message:"
			 "sender[%d:%d] msg_no[%d] receiver[%d:%d]",
			 header->src_addr.cpu_id,
			 header->src_addr.session_id, header->msg_no,
			 session->addr.cpu_id, session->addr.session_id);
		offset = (unsigned long)header - (unsigned long)module_context->this_list;
		free_list_item(session->addr.cpu_id, header);
		return 0;
	}
	#endif
	//CA_IPC_LOCK(&session->lock, flags);
	session->wait_trans_id = 0;
	//CA_IPC_UNLOCK(&session->lock, flags);

	session->ack_item = header;
	complete(&session->complete);

	CA_DEBUG("Received ack message");
	return 1;
}

#endif

static void do_message(struct work_struct *work)
{
	unsigned long flags, offset;
	struct ipc_context *session;
	struct msg_header *item;

	struct list_info *list;

	list = &(module_context->this_list->list_info);

	do {
		item = NULL;

		// pop from wait list
		CA_IPC_LOCK(&module_context->lock, flags);

		offset = module_context->wait_queue.first;

		if (0 != offset) {
			item = (struct msg_header *)((unsigned long)(module_context->this_list) + offset);
			module_context->wait_queue.first = item->next_offset;
			if (0 == module_context->wait_queue.first) {
				module_context->wait_queue.last = 0;
			}

			item->next_offset = 0;
		}

		CA_IPC_UNLOCK(&module_context->lock, flags);

		if (NULL == item) {
			break;
		}

		session = findSession(module_context, item->dst_addr.session_id);
		CA_DEBUG("do_message session id = %d\n",item->dst_addr.session_id);
		if (NULL == session) {
			free_list_item(module_context->addr.cpu_id, item);
			break;
		}
		CA_DEBUG("Recev msg_type[%d] session %p", item->ipc_flag, session);
		CA_DEBUG(" message:"
					 "sender[%d:%d] msg_no[%d] receiver[%d:%d] len %d",
					 item->src_addr.cpu_id,
					 item->src_addr.session_id, item->msg_no,
					 session->addr.cpu_id, session->addr.session_id,
					 item->payload_size);

		switch (item->ipc_flag) {
		case CA_IPC_ASYN_MSG:
			do_asyn_message(session, item);
			break;
#ifdef _CA_IPC_SYNC__
		case CA_IPC_SYNC_MSG:
			do_sync_message(session, item);
			break;
		case CA_IPC_ACK_MSG:
			do_ack_message(session, item);
			break;
#endif
		default:
			free_list_item(module_context->addr.cpu_id, item);
			break;
		};
	} while (1);
}

static irqreturn_t list_proc(int irq, void *device)
{
	struct list_info *list;
	struct fifo_info *fifo;
	unsigned long first_offset;
	unsigned long last_offset;
	struct msg_header *tmp;
	unsigned long flags;

	/* Clear interrupt status */
	CA_DEBUG("IPC list_proc\n");
	_ipc_clear_int();

	list = &(module_context->this_list->list_info);
	first_offset = 0;

	do {
		fifo = NULL;
		CA_IPC_LOCK(&module_context->lock, flags);

		ca_sem_lock(CA_SEM_IPC_ARM, 0);

		if (0 != list->high_fifo.first) {
			fifo = &list->high_fifo;
		} else if (0 != list->low_fifo.first) {
			fifo = &list->low_fifo;
		} else {
			fifo = NULL;
		}

		ca_sem_unlock(CA_SEM_IPC_ARM, 0);

		if (NULL != fifo) {
			/* Protect DS using MB0, to manipulate high_fifo & low_fifo */
			ca_sem_lock(CA_SEM_IPC_ARM, 0);

			first_offset = fifo->first;

			// pop all ready list from read queue,
			tmp = (struct msg_header *)((unsigned long)(module_context->this_list) + first_offset);
			last_offset = fifo->last;
			fifo->first = 0;
			fifo->last = 0;

			ca_sem_unlock(CA_SEM_IPC_ARM, 0);

			fifo = &module_context->wait_queue;
			// push the ready list into wait queue
			if (0 == fifo->last) {
				fifo->first = first_offset;
				fifo->last = last_offset;
			} else {
				tmp = (struct msg_header *)((unsigned long)(module_context->this_list) + fifo->last);
				tmp->next_offset = first_offset;
				fifo->last = last_offset;
			}
		}

		CA_IPC_UNLOCK(&module_context->lock, flags);

		schedule_work(&module_context->work);
	} while (NULL != fifo);

	return IRQ_HANDLED;
}

//static short CPU_ID = CPU_ARM;

#ifdef CONFIG_OF
/* Match table for of_platform binding */
static const struct of_device_id cortina_ipc_of_match[] = {
        { .compatible = "cortina,ipc", },
        {},
};
MODULE_DEVICE_TABLE(of, cortina_ipc_of_match);
#endif

#ifdef CA_IPC_TEST
int rx_callback_async(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	const char hellostr[]="Hello from ARM";
	const char hellostr2[]="2.Hello from ARM  ";
	ca_uint16_t result_size = 0x10;
    ca_ipc_pkt_t send_date;
    ca_ipc_cpu_id_t target_cpu = CA_IPC_CPU_PE0;

	/* Print out the message from Taroko */
	#if 1
	printk( "async callback receives msg_no[%d] message[%s] \n", msg_no,(const char *)msg_data);
    send_date.dst_cpu_id = target_cpu;
    send_date.session_id = 5;
    send_date.msg_no = 1;
    send_date.msg_size = 0x10;
    send_date.msg_data = hellostr;
    send_date.priority = CA_IPC_PRIO_HIGH;

	//ca_ipc_send(context, CPU_RCPU0, 5, CA_IPC_HPRIO, 1, hellostr, sizeof(hellostr));
    ca_ipc_msg_async_send(&send_date);

	//msleep(2000);
	#endif

	return 543;
}

int rx_callback_sync(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	char byestr[]={"Bye from ARM , nice to meet you"};
	int size_str = sizeof(byestr);
	printk( "sync callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	memcpy(msg_data,byestr,size_str);
	*msg_size = size_str;
	return 0;
}

ca_ipc_msg_handle_t invoke_procs_async[]=
{
	{ .msg_no=6, .proc=rx_callback_async }
};

ca_ipc_msg_handle_t invoke_procs_sync[]=
{
	{ .msg_no=8, .proc=rx_callback_sync }
};

//static struct ipc_context *context;

static int init_receiver(void)
{
	int rc;
	//ca_uint16_t result_size = 0x10;

	rc= ca_ipc_msg_handle_register(5, invoke_procs_async, 1);
	if( CA_IPC_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}

	rc= ca_ipc_msg_handle_register(3, invoke_procs_sync, 1);

	if( CA_IPC_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}
	return 0;
}
#endif

#define NAME_SIZE 50

typedef enum {
    CA_SYSTEM_APP_NAME = 1,
    CA_SYSTEM_VERSION = 2,
    CA_SYSTEM_READY_NOTIFY=3,
} ca_system_ipc_command_t;

typedef struct {
	char name[NAME_SIZE];
	char version[NAME_SIZE];
	ca_uint32_t bootready;
} ca_dsp_status_t;

ca_dsp_status_t pedsp0_status={"empty\0" , "empty\0" , 0};
ca_dsp_status_t pedsp1_status={"empty\0" , "empty\0" , 0};
#if defined(CONFIG_ARCH_REALTEK_9607F)
ca_dsp_status_t pedsp2_status={"empty\0" , "empty\0" , 0};
#endif
int pe_notify_appname_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	CA_DEBUG("%s %d\n",__FUNCTION__,__LINE__);
	//printk("cpu_id = 0x%x trans_id 0x%x  , msg_data = %s msg_size =%d\n",peer.cpu_id,trans_id,msg_data,*msg_size);

	if (*msg_size > NAME_SIZE)
	{
		printk("IPC error : pe_notify_appname_cb message size biger than buffer\n");
		return 1;
	}

	if(peer.cpu_id == CA_IPC_CPU_PE0)
	{
		memcpy(pedsp0_status.name,msg_data,*msg_size);
		pedsp0_status.name[*msg_size] = '\0';
	}
	else if (peer.cpu_id == CA_IPC_CPU_PE1)
	{
		memcpy(pedsp1_status.name,msg_data,*msg_size);
		pedsp1_status.name[*msg_size] = '\0';
	}
#if defined(CONFIG_ARCH_REALTEK_9607F)
	else if (peer.cpu_id == CA_IPC_CPU_PE2)
	{
		memcpy(pedsp2_status.name,msg_data,*msg_size);
		pedsp1_status.name[*msg_size] = '\0';
	}
#endif	
	return 0;
}

int pe_notify_version_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	CA_DEBUG("%s %d\n",__FUNCTION__,__LINE__);
	//printk("cpu_id = 0x%x trans_id 0x%x  , msg_data = %s msg_size =%d\n",peer.cpu_id,trans_id,msg_data,*msg_size);

	if (*msg_size > NAME_SIZE)
	{
		printk("IPC error : pe_notify_version_cb message size biger than buffer\n");
		return 1;
	}

	if(peer.cpu_id == CA_IPC_CPU_PE0)
	{
		memcpy(pedsp0_status.version,msg_data,*msg_size);
		pedsp0_status.version[*msg_size] = '\0';
	}
	else if (peer.cpu_id == CA_IPC_CPU_PE1)
	{
		memcpy(pedsp1_status.version,msg_data,*msg_size);
		pedsp1_status.version[*msg_size] = '\0';
	}
#if defined(CONFIG_ARCH_REALTEK_9607F)
	else if (peer.cpu_id == CA_IPC_CPU_PE2)
	{
		memcpy(pedsp2_status.version,msg_data,*msg_size);
		pedsp2_status.version[*msg_size] = '\0';
	}
#endif
	return 0;
}

int pe_notify_ready_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	CA_DEBUG("%s %d\n",__FUNCTION__,__LINE__);
	//printk("cpu_id = 0x%x trans_id 0x%x\n",peer.cpu_id,trans_id);

	if(peer.cpu_id == CA_IPC_CPU_PE0)
		pedsp0_status.bootready = 1;
	else if (peer.cpu_id == CA_IPC_CPU_PE1)
		pedsp1_status.bootready = 1;
#if defined(CONFIG_ARCH_REALTEK_9607F)
	else if (peer.cpu_id == CA_IPC_CPU_PE2)
		pedsp2_status.bootready = 1;
#endif
	return 0;
}
ca_ipc_msg_handle_t pe_notify[]=
{
	{ .msg_no=CA_SYSTEM_APP_NAME, .proc=pe_notify_appname_cb },
	{ .msg_no=CA_SYSTEM_VERSION, .proc=pe_notify_version_cb },
	{ .msg_no=CA_SYSTEM_READY_NOTIFY, .proc=pe_notify_ready_cb }
};

static int init_pe_notify(void)
{
	int rc;
	
	rc= ca_ipc_msg_handle_register(CA_IPC_SESSION_SYSTEM, pe_notify, 3);
	if( CA_IPC_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}
	return rc;
}

static int ca_ipc_probe(struct platform_device *pdev)
{
	int rc;
	int ret;
	struct resource mem_resource;
	const struct of_device_id *match;
	struct device_node *np;
    unsigned long shm_size;
	unsigned int Cpuidlen=0;
	const __be32 *thisCPU;

	module_context = kmalloc(sizeof(struct IPC_Module), GFP_KERNEL);
	module_context->addr.cpu_id = CPU_ARM;

	/* assign DT node pointer */
	np = pdev->dev.of_node;

	/* search DT for a match */
	match = of_match_device(cortina_ipc_of_match, &pdev->dev);
	if (!match){
		return -EINVAL;
	}

	thisCPU = of_get_property(pdev->dev.of_node, "pe-ipc-cpuid", &Cpuidlen);

	if(be32_to_cpup(thisCPU) == CPU_ARM){
		printk("IPC: host cpu is ARM , thisCPU=0x%x \n",be32_to_cpup(thisCPU));
		module_context->addr.cpu_id = CPU_ARM;
	}else if(be32_to_cpup(thisCPU) == CPU_RCPU0){
		printk("IPC: host cpu is PE0 , thisCPU=0x%x \n",be32_to_cpup(thisCPU));
		module_context->addr.cpu_id = CPU_RCPU0;
	}else if(be32_to_cpup(thisCPU) == CPU_RCPU1){
		printk("IPC: host cpu is PE1 , thisCPU=0x%x \n",be32_to_cpup(thisCPU));
		module_context->addr.cpu_id = CPU_RCPU1;
    }
#if defined(CONFIG_ARCH_REALTEK_9607F)
	else if(be32_to_cpup(thisCPU) == CPU_RCPU2){
		printk("IPC: host cpu is PE2 , thisCPU=0x%x \n",be32_to_cpup(thisCPU));
		module_context->addr.cpu_id = CPU_RCPU2;
    }
#endif
	/* get "mbx register" from DT and convert to platform mem address resource */
	ret = of_address_to_resource(np, CA_IPC_MEM_RESOURCE_MBX_REGS, &mem_resource);
	if (ret) {
		printk("%s: of_address_to_resource(CA_IPC_MEM_RESOURCE_MBX_REGS) return %d\n", __func__, ret);
		return ret;
	}

	module_context->mbox_addr =
		devm_ioremap(&pdev->dev, mem_resource.start, resource_size(&mem_resource));

	if (!module_context->mbox_addr) {
		return -ENOMEM;
	}

	/* get "shared memory" from DT and convert to platform mem address resource */
	np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!np) {
        printk("No %s specified\n", "memory-region");
        return ret;
	}

	ret = of_address_to_resource(np, 0, &mem_resource);
	if (ret) {
		printk("%s: of_address_to_resource(CA_IPC_MEM_RESOURCE_SHAREMEM) return %d\n", __func__, ret);
		return ret;
	}

	shm_size = resource_size(&mem_resource);
	module_context->root_list = devm_ioremap(&pdev->dev, mem_resource.start, shm_size);
	printk("IPC: list physical start from: 0x%lx\n", (unsigned long)mem_resource.start);
	printk("IPC: list virtual start from: 0x%lx\n", (unsigned long)module_context->root_list);
	printk("IPC: list size: 0x%lx\n", shm_size);

	if (!module_context->root_list) {
		return -ENOMEM;
	}
	memset_io((void *)&(module_context->root_list[0]), 0xFF, shm_size); //let't shm set to un-init states

	module_context->this_list = &(module_context->root_list[module_context->addr.cpu_id]);

	printk("IPC: Debug:...Clearing memory.... from 0x%lx\n", (unsigned long)(module_context->this_list));
	memset_io((void *)module_context->this_list, 0x0, IPC_LIST_SIZE);

	spin_lock_init(&module_context->lock);
	INIT_WORK(&module_context->work, do_message);

	CA_DEBUG("cpu_id[%d] memory address 0x%lx \n", module_context->addr.cpu_id, (unsigned long)(module_context->this_list));
	CA_DEBUG("Remap address = %lx\n",(unsigned long)(module_context->mbox_addr));

	module_context->addr.session_id = 0;
	module_context->wait_queue.first = module_context->wait_queue.last = 0;

	initial_list_info(module_context->addr.cpu_id);
	INIT_LIST_HEAD(&module_context->session_list);

	printk("IPC: version is %x\n",module_context->this_list->list_info.version);
	np = pdev->dev.of_node;
	/* get "interrupts" property from DT */
	module_context->mbx_irq = irq_of_parse_and_map(np, 0);
	printk("IPC: irq = %d\n", module_context->mbx_irq);

	rc = request_irq(module_context->mbx_irq, list_proc, 0, "ca_ipc", NULL);
	if (0 < rc) {
		printk(KERN_WARNING "request irq failed[%d]", rc);
		kfree(module_context);
		return rc;
	}

	/* Enable interrupt */
	_ipc_enable_int(module_context->addr.cpu_id);

#ifdef CA_IPC_TEST
	init_receiver();
#endif
	init_pe_notify();

	return 0;
}

static int ca_ipc_remove(struct platform_device *op)
{
	if (0 == module_context->addr.cpu_id) {
		iounmap(module_context->root_list);
		iounmap(module_context->mbox_addr);
		kfree((void *)module_context->root_list);
	}

	free_irq(module_context->mbx_irq, NULL);
	kfree(module_context);

	return 0;
}

static struct platform_driver ca_ipc_driver = {
	.probe = ca_ipc_probe,
	.remove = ca_ipc_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ca_ipc",
		.of_match_table = of_match_ptr(cortina_ipc_of_match),
	},
};

static int __init ca_ipc_init(void)
{
	return platform_driver_register(&ca_ipc_driver);
}

static void __exit ca_ipc_exit(void)
{
	platform_driver_unregister(&ca_ipc_driver);
}

module_init(ca_ipc_init);
module_exit(ca_ipc_exit);


#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int pedsp0_proc_name_r(struct seq_file *m, void *v)
{
	//printk("pedsp0_proc_name_r %s\n", pedsp0_status.name);
	seq_printf(m,"%s\n",pedsp0_status.name);
	//seq_puts(m,pedsp0_status.name);	
	//printk("pedsp0_proc_name_r\n");
	return 0;
}

static int pedsp0_proc_open_name(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp0_proc_name_r, NULL);
}


static const struct file_operations pedsp0_proc_name_fops = {
	.open		= pedsp0_proc_open_name,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp0_proc_version_r(struct seq_file *s, void *v)
{
	//printk("pedsp0_proc_version_r %s\n", pedsp0_status.version);
	seq_printf(s,"%s\n",pedsp0_status.version);
	//seq_puts(s,pedsp0_status.version);
	return 0;
}

static int pedsp0_proc_open_version(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp0_proc_version_r, NULL);
}

static const struct file_operations pedsp0_proc_version_fops = {
	.open		= pedsp0_proc_open_version,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp0_proc_status_r(struct seq_file *s, void *v)
{
	//printk("pedsp0_proc_status_r %s\n", pedsp0_status.version);

	if (pedsp0_status.bootready == 1 )
		seq_printf(s,"BootComplete\n");
	else
		seq_printf(s,"resetting\n");
	//seq_printf(s,"%s\n",pedsp0_status.version);
	//seq_puts(s,pedsp0_status.version);
	return 0;
}

static int pedsp0_proc_open_status(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp0_proc_status_r, NULL);
}


static const struct file_operations pedsp0_proc_status_fops = {
	.open		= pedsp0_proc_open_status,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp1_proc_name_r(struct seq_file *m, void *v)
{
	//printk("pedsp1_proc_name_r %s\n", pedsp1_status.name);
	seq_printf(m,"%s\n",pedsp1_status.name);
	//seq_puts(m,pedsp1_status.name);	
	//printk("pedsp1_proc_name_r\n");
	return 0;
}

static int pedsp1_proc_open_name(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp1_proc_name_r, NULL);
}


static const struct file_operations pedsp1_proc_name_fops = {
	.open		= pedsp1_proc_open_name,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp1_proc_version_r(struct seq_file *s, void *v)
{
	//printk("pedsp1_proc_version_r %s\n", pedsp1_status.version);
	seq_printf(s,"%s\n",pedsp1_status.version);
	//seq_puts(s,pedsp1_status.version);
	return 0;
}

static int pedsp1_proc_open_version(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp1_proc_version_r, NULL);
}

static const struct file_operations pedsp1_proc_version_fops = {
	.open		= pedsp1_proc_open_version,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp1_proc_status_r(struct seq_file *s, void *v)
{
	//printk("pedsp1_proc_status_r %s\n", pedsp1_status.version);

	if (pedsp1_status.bootready == 1 )
		seq_printf(s,"BootComplete\n");
	else
		seq_printf(s,"resetting\n");
	//seq_printf(s,"%s\n",pedsp1_status.version);
	//seq_puts(s,pedsp1_status.version);
	return 0;
}

static int pedsp1_proc_open_status(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp1_proc_status_r, NULL);
}


static const struct file_operations pedsp1_proc_status_fops = {
	.open		= pedsp1_proc_open_status,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#if defined(CONFIG_ARCH_REALTEK_9607F)
static int pedsp2_proc_name_r(struct seq_file *m, void *v)
{
	seq_printf(m,"%s\n",pedsp2_status.name);
	return 0;
}

static int pedsp2_proc_open_name(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp2_proc_name_r, NULL);
}


static const struct file_operations pedsp2_proc_name_fops = {
	.open		= pedsp2_proc_open_name,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp2_proc_version_r(struct seq_file *s, void *v)
{
	//printk("pedsp2_proc_version_r %s\n", pedsp2_status.version);
	seq_printf(s,"%s\n",pedsp2_status.version);
	//seq_puts(s,pedsp2_status.version);
	return 0;
}

static int pedsp2_proc_open_version(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp2_proc_version_r, NULL);
}

static const struct file_operations pedsp2_proc_version_fops = {
	.open		= pedsp2_proc_open_version,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pedsp2_proc_status_r(struct seq_file *s, void *v)
{
	//printk("pedsp2_proc_status_r %s\n", pedsp2_status.version);

	if (pedsp2_status.bootready == 1 )
		seq_printf(s,"BootComplete\n");
	else
		seq_printf(s,"resetting\n");
	//seq_printf(s,"%s\n",pedsp2_status.version);
	//seq_puts(s,pedsp2_status.version);
	return 0;
}

static int pedsp2_proc_open_status(struct inode *inode, struct file *file)
{
	return single_open(file, pedsp2_proc_status_r, NULL);
}


static const struct file_operations pedsp2_proc_status_fops = {
	.open		= pedsp2_proc_open_status,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif
static int __init proc_pestatus_init(void)
{
	struct proc_dir_entry *proc_pedsp_dev_dir ;
	struct proc_dir_entry *entry;

	proc_pedsp_dev_dir = proc_mkdir("pedsp0Info", NULL);
	if (proc_pedsp_dev_dir == NULL) {
		printk("create proc pedsp0Info failed!\n");
		return 1;
	}

	proc_pedsp_dev_dir = proc_mkdir("pedsp1Info", NULL);
	if (proc_pedsp_dev_dir == NULL) {
		printk("create proc pedsp1Info failed!\n");
		return 1;
	}
#if defined(CONFIG_ARCH_REALTEK_9607F)
	proc_pedsp_dev_dir = proc_mkdir("pedsp2Info", NULL);
	if (proc_pedsp_dev_dir == NULL) {
		printk("create proc pedsp2Info failed!\n");
		return 1;
	}
#endif
	if((entry = proc_create("pedsp0Info" "/" "name", 0, NULL, &pedsp0_proc_name_fops)) == NULL) {
		printk("create proc pedsp0/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp0Info" "/" "version", 0, NULL, &pedsp0_proc_version_fops)) == NULL) {
		printk("create proc pedsp0/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp0Info" "/" "status", 0, NULL, &pedsp0_proc_status_fops)) == NULL) {
		printk("create proc pedsp0/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp1Info" "/" "name", 0, NULL, &pedsp1_proc_name_fops)) == NULL) {
		printk("create proc pedsp1/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp1Info" "/" "version", 0, NULL, &pedsp1_proc_version_fops)) == NULL) {
		printk("create proc pedsp1/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp1Info" "/" "status", 0, NULL, &pedsp1_proc_status_fops)) == NULL) {
		printk("create proc pedsp1/name failed!\n");
		return 1;
	}
#if defined(CONFIG_ARCH_REALTEK_9607F)
	if((entry = proc_create("pedsp2Info" "/" "name", 0, NULL, &pedsp2_proc_name_fops)) == NULL) {
		printk("create proc pedsp2/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp2Info" "/" "version", 0, NULL, &pedsp2_proc_version_fops)) == NULL) {
		printk("create proc pedsp2/name failed!\n");
		return 1;
	}

	if((entry = proc_create("pedsp2Info" "/" "status", 0, NULL, &pedsp2_proc_status_fops)) == NULL) {
		printk("create proc pedsp2/name failed!\n");
		return 1;
	}
#endif
	return 0;
}
fs_initcall(proc_pestatus_init);