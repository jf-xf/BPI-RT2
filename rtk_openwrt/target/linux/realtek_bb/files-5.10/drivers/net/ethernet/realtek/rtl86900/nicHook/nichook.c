
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/list.h>

#include "ca_ni.h"
#include "ca_ext.h"
//typedef int (*p2NicRxHook_t)(int portmask,int priority,p2rfunc_t rx) ;
//typedef int (*p2NicRxUnHook_t)(int portmask,int priority,p2rfunc_t rx) ;

p2NicRxHook_t nic_rx_hook=NULL;
p2NicRxUnHook_t nic_rx_un_hook=NULL;

int ni_hook_debug=0;

/* for rx handler mantian*/
static struct list_head ndyHookHead;

/*__IRAM*/
static int insert_nic_ndyhook_list(void)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;
	int ret;
	//printk(KERN_INFO "[%s-%d]\n", __func__,__LINE__);	
	list_for_each_safe(next,tmp,&ndyHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);
		if(cur == NULL){
			printk(KERN_INFO "[%s-%d] cur == NULL\n", __func__,__LINE__);
			return 0;
		}
		//printk(KERN_INFO "[%s-%d] psk=%x, pri=%d, rx=%p\n", __func__,__LINE__,cur->portmask,cur->priority,cur->do_rx);	
		nic_rx_hook(cur->portmask,cur->priority,cur->do_rx);
	}
	//printk(KERN_INFO "[%s-%d]\n", __func__,__LINE__);	
	return 0;
}


//Using for module ca-ni calling
void nic_hook_rx_register(p2NicRxHook_t nicRxHook)

{
	printk("[%s-%d] nicRxHook=%p\n",__func__,__LINE__,nicRxHook);
   nic_rx_hook = nicRxHook;
   insert_nic_ndyhook_list();

}

 

//Using for module ca-ni calling
void nic_unhook_rx_register(p2NicRxUnHook_t nicRxUnHook)

{
	printk("[%s-%d] nicRxUnHook=%p\n",__func__,__LINE__,nicRxUnHook);
    nic_rx_un_hook = nicRxUnHook;

}


static int insert_ndyhook_entry(drv_nic_hook_entry_t *entry)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;
	int notEmpty=0;
//printk(KERN_INFO "[%s-%d]\n", __func__,__LINE__);
	list_for_each_safe(next,tmp,&ndyHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

		if(cur == NULL){
			return -1;
		}
//printk("priority: %d, portmask %d, rx: %p\n",cur->priority,cur->portmask,cur->do_rx);

	    if(cur->priority <= entry->priority){
			cur->list.prev->next = &entry->list;
			entry->list.next = &cur->list;
			entry->list.prev = cur->list.prev;
			cur->list.prev = &entry->list;
			printk("insert here, priority: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);
			return 0;
		}
		notEmpty=1;
	}
	if(notEmpty)
	{
	printk("not first entry: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);	
			list_add_tail(&entry->list,&ndyHookHead);
	}
	else
	{/*first entry*/
	printk("first entry: %d, portmask %d, rx: %p\n",entry->priority,entry->portmask,entry->do_rx);	
			list_add(&entry->list,&ndyHookHead);
	}

	return 0;
} 

static void nic_ndyhook_clear(void)
{

	struct list_head *next = NULL, *tmp=NULL;
	drv_nic_hook_entry_t *cur = NULL;
	printk(KERN_INFO "[%s-%d]\n", __func__,__LINE__);
	list_for_each_safe(next,tmp,&ndyHookHead){

		cur = list_entry(next,drv_nic_hook_entry_t,list);

	    if(cur!=NULL){
			list_del(&cur->list);
			kfree(cur);
		}
	}
	return;
}


//using for build-in or other module calling
//return value
// 0: nic driver is ready.
// 1: nic driver is not ready yet.
int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx)
{
	drv_nic_hook_entry_t *entry;

	if(nic_rx_hook==NULL){
		//nic driver module is not ready yet. 
		//rg driver may initalize before nic,
		//record func ptr addr into temply buffer
		//when nic driver is ready, we insert it into register rxhook.
		entry = (drv_nic_hook_entry_t*)kmalloc(sizeof(drv_nic_hook_entry_t),GFP_KERNEL);
		if(!entry){
			printk(KERN_INFO "[%s-%d] not have enough memory\n", __func__,__LINE__);
			return -1;
		}		
		printk(KERN_INFO "[%s-%d] portmask=%x, pri:%d, rx=%p\n", __func__,__LINE__, portmask,priority,rx);
		/*assigned value to entry*/
		entry->do_rx = rx;
		entry->portmask = portmask;
		entry->priority = priority;
		/*insert entry depend on priority*/
		insert_ndyhook_entry(entry);		
		return NIC_DRIVER_IS_NOT_READY;
	}

	return nic_rx_hook(portmask,priority,rx);

}

 

//using for build-in or other module calling
int drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx)
{
	if(nic_rx_un_hook==NULL){
		struct list_head *next = NULL, *tmp=NULL;
		drv_nic_hook_entry_t *cur = NULL;
		printk(KERN_INFO "[%s-%d]\n", __func__,__LINE__);

		list_for_each_safe(next,tmp,&ndyHookHead){

			cur = list_entry(next,drv_nic_hook_entry_t,list);

			if(cur == NULL){
				return -1;
			}

	    	if(cur->do_rx == rx && cur->portmask == portmask && cur->priority==priority){
				list_del(&cur->list);
				kfree(cur);
				return 0;
			}
		}		
		return NIC_DRIVER_IS_NOT_READY;
	}

	return nic_rx_un_hook(portmask,priority,rx);

}

int nic_rxhook_test(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv)
{
	//int i;
	//ca_eth_private_t *cep = netdev_priv(napi->dev);
	//cep->port_cfg.rx_ldpid;
	printk("[%s-%d] \n",__func__,__LINE__);
	//printk("[%s-%d] rx_ldpid=%d, tx_ldpit=%d\n",__func__,__LINE__,cep->port_cfg.rx_ldpid,cep->port_cfg.tx_ldpid);
	//printk("[%s-%d] status=%x, num_phy_port=%d\n",__func__,__LINE__,cep->status,cep->port_cfg.num_phy_port);
#if 0
	for ( i = 0; i < ca_ni_num_intf; i++) 
	{
	
	}
#endif
	return RE8670_RX_CONTINUE;
} 

int __init nic_hook_init(void)
{
	int ret=0,i;
	//test only.
	printk("[%s-%d]\n",__func__,__LINE__);
	INIT_LIST_HEAD(&ndyHookHead);
	//ret = drv_nic_register_rxhook(0x7f,RENIC_RXPRI_RG,nic_rxhook_test);
	//printk("[%s-%d] ret=%d\n",__func__,__LINE__,ret);

	return 0;

}

 

void __exit nic_hook_exit(void)
{
	printk("[%s-%d]\n",__func__,__LINE__);
	nic_ndyhook_clear();
}

 

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek NIC Hook module");
MODULE_AUTHOR("Realtek");

module_init(nic_hook_init);
module_exit(nic_hook_exit);

EXPORT_SYMBOL(drv_nic_register_rxhook);
EXPORT_SYMBOL(drv_nic_unregister_rxhook);
EXPORT_SYMBOL(nic_hook_rx_register);
EXPORT_SYMBOL(nic_unhook_rx_register);
