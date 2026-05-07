#ifndef RTK_RG_DEFINE_H
#define RTK_RG_DEFINE_H

#include <linux/version.h>

#include <hal/chipdef/chip.h>	//for chip id and sub type

#ifdef __KERNEL__
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
#else
#include <linux/config.h>
#endif
#endif

#include  "asicDriver/rtk_rg_asic_tblSize.h"
#include "rtk_rg_cpuReason.h"
#include <rtk_rg_wlan_internal.h>


#if defined(CONFIG_HWNAT_RG_MODULE)
#define __IRAM_FWDENG
#define __IRAM_FWDENG_L2
#define __IRAM_FWDENG_L34
#define __IRAM_FWDENG_SLOWPATH
#define __SRAM_FWDENG_SLOWPATH
#define __SRAM_FWDENG_DATA
#define __SRAM_FWDENG_PREDATA
#else
#define __IRAM_FWDENG			__attribute__ ((section(".iram-fwd")))
#define __IRAM_FWDENG_L2		__attribute__ ((section(".iram-l2-fwd")))
#define __IRAM_FWDENG_L34		__attribute__ ((section(".iram-l34-fwd")))
#define __IRAM_FWDENG_SLOWPATH	__attribute__ ((section(".iram-l34-fwd")))
#define __SRAM_FWDENG_SLOWPATH	__attribute__ ((section(".sram_text")))
#define __SRAM_FWDENG_DATA		__attribute__ ((section(".sram_data")))
#define __SRAM_FWDENG_PREDATA	__attribute__ ((section(".sram_predata")))
#endif

#define RG_UNCACHE_DW(x)  (*((volatile u32*)(((u32)x)|0xa0000000)))
#define RG_UNCACHE_ADDR(x)  (((u32)x)|0xa0000000)

#if defined(CONFIG_RG_G3_SERIES)||defined(CONFIG_APOLLO_MODEL)
#define POINTER_CAST	unsigned long
#else
#define POINTER_CAST	unsigned int
#endif

/* for SMP */
#ifdef CONFIG_SMP
#define TIMER_AGG
//#define OUTBOUND_BY_TASKLET
//#define RG_BY_TASKLET
//#define RG_SC_SKIP_LOCK

/* tasklet */
#define MAX_RG_RX_QUEUE_SIZE 		4096
#define MAX_RG_RX_HI_QUEUE_SIZE 	1024
#define MAX_RG_GMAC0_TX_QUEUE_SIZE 	4096
#define MAX_RG_GMAC1_TX_QUEUE_SIZE 	1024
#if defined(CONFIG_GMAC2_USABLE)
#define MAX_RG_GMAC2_TX_QUEUE_SIZE 	1024
#endif
#if defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8812FE)
#define MAX_RG_WLAN0_TX_QUEUE_SIZE 	8192
#define MAX_RG_WLAN1_TX_QUEUE_SIZE 	8192
#else
#define MAX_RG_WLAN0_TX_QUEUE_SIZE  2048
#define MAX_RG_WLAN1_TX_QUEUE_SIZE  2048
#endif
//workqueue
#define MAX_RG_INBOUND_QUEUE_SIZE 1024 //must power of 2
#define MAX_RG_GMAC9_OUTBOUND_QUEUE_SIZE 1024 //must power of 2
#define MAX_RG_GMAC10_OUTBOUND_QUEUE_SIZE 1024 //must power of 2
#if defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8812FE)
#define MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE 8192 //must power of 2
#define MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE 8192 //must power of 2
#else
#define MAX_RG_WIFI_11AC_OUTBOUND_QUEUE_SIZE 2048 //must power of 2
#define MAX_RG_WIFI_11N_OUTBOUND_QUEUE_SIZE 2048 //must power of 2
#endif
#define MAX_RG_TIMER_QUEUE_SIZE 1024 //must power of 2

#define rg_inbound_queue_lock(smp_id,lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_inbound_queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_RG_UNLOCK][smp_id];	\
		atomic_inc(&rg_kernel.rg_inbound_queue_entrance);	\
		spin_lock_bh(lock);	\
	} while(0)

#define rg_inbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
		atomic_dec(&rg_kernel.rg_inbound_queue_entrance);	\
	} while(0)

#define rg_wq_rx_queue_lock(smp_id,lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_wq_rx_queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_WQRX_UNLOCK][smp_id];	\
		atomic_inc(&rg_kernel.rg_wq_rx_queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_wq_rx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.rg_wq_rx_queue_entrance);	\
	} while(0)

#define rg_gmac9_outbound_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_gmac9_outbound_queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_GMAC9_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.rg_gmac9_outbound_queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_gmac9_outbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
		atomic_dec(&rg_kernel.rg_gmac9_outbound_queue_entrance);	\
	} while(0)

#define rg_gmac10_outbound_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_gmac10_outbound_queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_GMAC10_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.rg_gmac10_outbound_queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_gmac10_outbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
		atomic_dec(&rg_kernel.rg_gmac10_outbound_queue_entrance);	\
	} while(0)

#define rg_wifi_11ac_outbound_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_wifi_11ac_outbound_queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_WIFI_11AC_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.rg_wifi_11ac_outbound_queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_wifi_11ac_outbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
		atomic_dec(&rg_kernel.rg_wifi_11ac_outbound_queue_entrance);	\
	} while(0)

#define rg_wifi_11n_outbound_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rg_wifi_11n_outbound_queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_WIFI_11N_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.rg_wifi_11n_outbound_queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_wifi_11n_outbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
		atomic_dec(&rg_kernel.rg_wifi_11n_outbound_queue_entrance);	\
	} while(0)

#define rg_rx_queue_lock(smp_id,lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rx_ctrl.queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_IPI_WAIT_RX_UNLOCK][smp_id];	\
		atomic_inc(&rg_kernel.rx_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_rx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.rx_ctrl.queue_entrance);	\
	} while(0)

#define rg_rx_hi_queue_lock(smp_id,lock)			\
	do {						\
		if(atomic_read(&rg_kernel.rx_hi_ctrl.queue_entrance)>=1)	\
			++rg_db.systemGlobal.smp_statistic[RG_SMP_IPI_WAIT_RX_HI_UNLOCK][smp_id];	\
		atomic_inc(&rg_kernel.rx_hi_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_rx_hi_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.rx_hi_ctrl.queue_entrance);	\
	} while(0)

#define rg_gmac0_tx_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.gmac0_tx_ctrl.queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_GMAC0_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.gmac0_tx_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_gmac0_tx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.gmac0_tx_ctrl.queue_entrance);	\
	} while(0)

#define rg_gmac1_tx_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.gmac1_tx_ctrl.queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_GMAC1_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.gmac1_tx_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_gmac1_tx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.gmac1_tx_ctrl.queue_entrance);	\
	} while(0)
#if defined(CONFIG_GMAC2_USABLE)
#define rg_gmac2_tx_queue_lock(smp_id, lock)			\
		do {						\
			if(atomic_read(&rg_kernel.gmac2_tx_ctrl.queue_entrance)>=1) \
				++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_GMAC2_UNLOCK][smp_id]; \
			atomic_inc(&rg_kernel.gmac2_tx_ctrl.queue_entrance);	\
			spin_lock_bh(lock); \
		} while(0)
	
#define rg_gmac2_tx_queue_unlock(lock)			\
		do {						\
			spin_unlock_bh(lock); \
			atomic_dec(&rg_kernel.gmac2_tx_ctrl.queue_entrance);	\
		} while(0)
#endif
#define rg_wlan0_tx_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.wlan0_tx_ctrl.queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_WLAN0_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.wlan0_tx_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_wlan0_tx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.wlan0_tx_ctrl.queue_entrance);	\
	} while(0)

#define rg_wlan1_tx_queue_lock(smp_id, lock)			\
	do {						\
		if(atomic_read(&rg_kernel.wlan1_tx_ctrl.queue_entrance)>=1) \
			++rg_db.systemGlobal.smp_statistic[RG_SMP_WAIT_WLAN1_UNLOCK][smp_id]; \
		atomic_inc(&rg_kernel.wlan1_tx_ctrl.queue_entrance);	\
		spin_lock_bh(lock); \
	} while(0)

#define rg_wlan1_tx_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock); \
		atomic_dec(&rg_kernel.wlan1_tx_ctrl.queue_entrance);	\
	} while(0)

#define RGDB_LOCK spin_lock_bh(&rg_kernel.rg_inbound_queue_lock);
#define RGDB_UNLOCK spin_unlock_bh(&rg_kernel.rg_inbound_queue_lock);
#else
//for uniprocessor, the flags is useless for spin_lock_bh
#define rg_inbound_queue_lock(smp_id,lock)			\
	do {						\
		smp_id=0;		\
		spin_lock_bh(lock);	\
	} while(0)

#define rg_inbound_queue_unlock(lock)			\
	do {						\
		spin_unlock_bh(lock);	\
	} while(0)

#define RGDB_LOCK
#define RGDB_UNLOCK
#endif

/*
_rtk_rg_list_entry_idx(): get the element index of the original array
	listElement_ptr: pointer which points to the address of the element
	listElementArray_ptr: pointer which points to the start address of the original array
Note that, the type of listElement_ptr and listElementArray_ptr should be the pointer of element
*/
#define _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr) \
	(abs((listElement_ptr) - (listElementArray_ptr)))


/*
_rtk_rg_list_first_entry(): get the first list element by from list head
	listHead_ptr: pointer which points to the list head variable
*/
#define _rtk_rg_list_first_entry(listHead_ptr, type) \
	((type *)((listHead_ptr)->first))

/*
_rtk_rg_list_next_entry(): get the next linked list element by nextIdx
	listElement_ptr: pointer which points to the address of the current element
	listElementArray_ptr: pointer which points to the start address of the original array
Note that, the type of listElement_ptr and listElementArray_ptr should be the pointer of element
*/
#define _rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member) \
	(listElementArray_ptr + ((listElement_ptr)->member).nextIdx)

/*
_rtk_rg_list_prev_entry(): get the previous linked list element by prevIdx
	listElement_ptr: pointer which points to the address of the current element
	listElementArray_ptr: pointer which points to the start address of the original array
Note that, the type of listElement_ptr and listElementArray_ptr should be the pointer of element
*/
#define _rtk_rg_list_prev_entry(listElement_ptr, listElementArray_ptr, member) \
	(listElementArray_ptr + ((listElement_ptr)->member).prevIdx)

/*
_rtk_rg_init_list(): initial linked list element
	listElement_ptr: pointer which points to the address of the current element
	listElementArray_ptr: pointer which points to the start address of the original array
Note that, the type of listElement_ptr and listElementArray_ptr should be the pointer of element
*/
#define _rtk_rg_init_list(listElement_ptr, listElementArray_ptr, member) \
	do { \
		((listElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
		((listElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
	} while(0)

/*
_rtk_rg_init_list_head(): initial linked list head
*/
#define _rtk_rg_init_list_head(listHead_ptr) \
	do { \
		(listHead_ptr)->first = (listHead_ptr); \
	}while (0)

/*
_rtk_rg_list_head_empty(): check if linked list head is empty
*/
#define _rtk_rg_list_head_empty(listHead_ptr) ((listHead_ptr)->first == listHead_ptr)

/*
_rtk_rg_list_del_init_from_listHead(): delete linked list element from linked list head (For the deleted element is the first element of the linked list)
*/
#define _rtk_rg_list_del_init_from_listHead(listElement_ptr, listElementArray_ptr, listHead_ptr, member) \
	do { \
		if(listElement_ptr == (listHead_ptr)->first) \
		{ \
			if(_rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member) == listElement_ptr) \
				(listHead_ptr)->first = listHead_ptr; \
			else \
				(listHead_ptr)->first = _rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member); \
		} \
	}while (0)


/*
_rtk_rg_list_del_init_from_list(): delete linked list element from the linked list
Note. Please call _rtk_rg_list_del_init_from_listHead() before calling _rtk_rg_list_del_init_from_list()
*/
#define _rtk_rg_list_del_init_from_list(listElement_ptr, listElementArray_ptr, member) \
	do { \
		(_rtk_rg_list_prev_entry(listElement_ptr, listElementArray_ptr, member))->member.nextIdx = ((listElement_ptr)->member).nextIdx; \
		(_rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member))->member.prevIdx = ((listElement_ptr)->member).prevIdx; \
		((listElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
		((listElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
	}while (0)

/*
_rtk_rg_list_add_to_tail(): add linked list element to the tail of linked list
*/
#define _rtk_rg_list_add_to_tail(listElement_ptr, listElementArray_ptr, listHead_ptr ,member) \
	do { \
		if(_rtk_rg_list_head_empty(listHead_ptr))\
		{ \
			(listHead_ptr)->first = (listElement_ptr); \
		} \
		else \
		{ \
			typeof(listElement_ptr) firstElement_ptr = ((typeof(listElement_ptr))((listHead_ptr)->first)); \
			typeof(listElement_ptr) lastElement_ptr = (_rtk_rg_list_prev_entry((typeof(listElement_ptr))((listHead_ptr)->first), listElementArray_ptr, member)); \
			((lastElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
			((firstElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
			((listElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(lastElement_ptr, listElementArray_ptr); \
			((listElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(firstElement_ptr, listElementArray_ptr); \
		} \
	}while (0)

/*
_rtk_rg_list_add(): add linked list element to the front of linked list
*/


#define _rtk_rg_list_add(listElement_ptr, listElementArray_ptr, listHead_ptr ,member) \
	do { \
		if(_rtk_rg_list_head_empty(listHead_ptr))\
		{ \
			(listHead_ptr)->first = (listElement_ptr); \
		} \
		else \
		{ \
			typeof(listElement_ptr) firstElement_ptr = ((typeof(listElement_ptr))((listHead_ptr)->first)); \
			typeof(listElement_ptr) lastElement_ptr = (_rtk_rg_list_prev_entry((typeof(listElement_ptr))((listHead_ptr)->first), listElementArray_ptr, member)); \
			((lastElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
			((firstElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(listElement_ptr, listElementArray_ptr); \
			((listElement_ptr)->member).prevIdx = _rtk_rg_list_entry_idx(lastElement_ptr, listElementArray_ptr); \
			((listElement_ptr)->member).nextIdx = _rtk_rg_list_entry_idx(firstElement_ptr, listElementArray_ptr); \
			(listHead_ptr)->first = (listElement_ptr); \
		} \
	}while (0)


/*
_rtk_rg_list_for_each_entry(): iterator to the linked list
	listElement_ptr: element iterator
	listElementArray_ptr: pointer which points to the start address of the original array
	listHead_ptr: pointer which points to the list head variable
	member: variable name of linked list structure
	flag_started: variable for identifying if back to the first elememt.
*/
#define _rtk_rg_list_for_each_entry(listElement_ptr, listElementArray_ptr, listHead_ptr, member) \
	for (listElement_ptr = (_rtk_rg_list_head_empty(listHead_ptr))?NULL:((typeof(listElement_ptr))((listHead_ptr)->first)); \
	     listElement_ptr != NULL; \
	     listElement_ptr = ((((listElement_ptr)->member).nextIdx==_rtk_rg_list_entry_idx(((typeof(listElement_ptr))((listHead_ptr)->first)), listElementArray_ptr))?NULL:_rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member)))

/*
_rtk_rg_list_for_each_entry_safe(): iterator to the linked list
	listElement_ptr: element iterator
	listNextElement_ptr: element iterator (the next element of listElement_ptr)
	listElementArray_ptr: pointer which points to the start address of the original array
	listHead_ptr: pointer which points to the list head variable
	member: variable name of linked list structure
	flag_started: variable for identifying if back to the first elememt.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 3)
#define _rtk_rg_smp_store_mb(x, y) \
				({	\
					smp_store_mb(x,y);	\
				})
#else
#define _rtk_rg_smp_store_mb(x, y) \
				({	\
					set_mb(x,y);	\
				})
#endif

#define _rtk_rg_list_for_each_entry_safe(listElement_ptr, listNextElement_ptr, listElementArray_ptr, listHead_ptr, member) \
	for (listElement_ptr = (_rtk_rg_list_head_empty(listHead_ptr))?NULL:((typeof(listElement_ptr))((listHead_ptr)->first)), listNextElement_ptr = (listElement_ptr == NULL)?NULL:((((listElement_ptr)->member).nextIdx==_rtk_rg_list_entry_idx(((typeof(listElement_ptr))((listHead_ptr)->first)), listElementArray_ptr))?NULL:_rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member)); \
		 listElement_ptr != NULL; \
		 _rtk_rg_smp_store_mb(listElement_ptr,listNextElement_ptr),  listNextElement_ptr = (listElement_ptr == NULL)?NULL:((((listElement_ptr)->member).nextIdx==_rtk_rg_list_entry_idx(((typeof(listElement_ptr))((listHead_ptr)->first)), listElementArray_ptr))?NULL:_rtk_rg_list_next_entry(listElement_ptr, listElementArray_ptr, member)))



#define CONFIG_RG_CALLBACK_LAN_DEVICE_NAME "eth0"
#define CONFIG_RG_CALLBACK_LOOPBACK_DEVICE_NAME "lo"

#if defined(CONFIG_RG_G3_SERIES)
#define NETIF_START_IDX 1
#else
#define NETIF_START_IDX 0
#endif


#if defined(CONFIG_RG_G3_SERIES)
#define G3_DEF_DEVID 0
#define G3_FLOW_AGE_STATE_IDLE			1
#define G3_FLOW_AGE_STATE_ALIVE			6

#define MAX_LAN_INTERFACE_SIZE			3
#define MAX_WAN_INTERFACE_SIZE			(MAX_NETIF_SW_TABLE_SIZE-MAX_LAN_INTERFACE_SIZE)
#define	MAX_GENERIC_INTERFACE_PER_LAN	11
#define	MAX_GENERIC_INTERFACE_SIZE		64
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT		(5 + NETIF_START_IDX)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP0		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+1)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP1		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+2)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP2		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+3)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP3		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+4)

#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP4		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP5		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP6		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN0_WDS_VXD	(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_ROOT		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_VAP0		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_VAP1		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_VAP2		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_VAP3		(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)
#define	GENERIC_INTERFACE_INDEX_OF_WLAN1_WDS_VXD	(GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT+5)


#endif


/* ysleu:For OpenWRT RG, if WLAN client mode is enabled, the specific CONFIG_MASTER_WLAN0_ENABLE
         must be enabled too but OpenWRT wireless driver(linux 3.18) does not define it.*/
#if defined(CONFIG_OPENWRT_RG) && defined(CONFIG_RTL_REPEATER_MODE_SUPPORT)
#define CONFIG_MASTER_WLAN0_ENABLE
#endif


#define RESERVED_FREE_RAM_SIZE (5*1024*1024)	//reserved free ram size( 5M bytes )
#define MAX_LAYER4_PORT_NUM 65535
#define MAX_L3_LEN 65535
#define MIN_TCP_HDR_LEN 20


#define DEFAULT_L2_LANIF_IDX 0
#define DEFAULT_L2_WANIF_IDX 0
#define DEFAULT_MC_WANIF_IDX 14


/* for gpon FPGA testing */
//#define CONFIG_APOLLO_GPON_FPGATEST

/*Switch for NAPT and routing shortcut function */
#define CONFIG_ROME_NAPT_SHORTCUT

/*Switch for NAPT table refresh by LRU */
#define CONFIG_ROME_NAPT_LRU

/*Switch for NEIGHBOR table refresh by LRU */
#define CONFIG_ROME_NEIGHBOR_LRU

#define DEFAULT_BRIDGE_WAN_MTU 16383

/* System Module */

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define DEFAULT_CPU_VLAN 			4096
#define DEFAULT_CPU_SVLAN 			2
#define DEFAULT_PROTO_BLOCK_VLAN 	4098		//used for IP_version only_mode, as PVID to block traffic to other port
#define DEFAULT_BIND_INTERNET		4099
#define DEFAULT_BIND_OTHER			4100		//this will use 4100~4100+DEFAULT_BIND_LAN_OFFSET+DEFAULT_BIND_WLAN_OFFSET
#else
#define DEFAULT_CPU_VLAN 			1
#if !defined(CONFIG_RG_RTL9600_SERIES)
#define DEFAULT_CPU_SVLAN 			2
#endif
#define DEFAULT_PROTO_BLOCK_VLAN 	4000		//used for IP_version only_mode, as PVID to block traffic to other port
#define DEFAULT_BIND_INTERNET		4005
#define DEFAULT_BIND_OTHER			4012		//this will use 4012~4012+DEFAULT_BIND_LAN_OFFSET+DEFAULT_BIND_WLAN_OFFSET
#endif

#ifdef CONFIG_RG_WAN_PORT_ISOLATE
#if defined(CONFIG_RG_RTL9600_SERIES)
#define DEFAULT_WAN_VLAN 			2
#else
#define DEFAULT_WAN_VLAN 			3
#endif
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES)
#define DEFAULT_DS_RATE_LIMIT_BY_SHAPER_VLAN   4001
#define DEFAULT_DS_LOOKBACK_VLAN	4002
#endif

#ifdef CONFIG_MASTER_WLAN0_ENABLE
#ifdef CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_VAP_SUPPORT
#define DEFAULT_BIND_LAN_OFFSET		18			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4, wds5, wds6, wds7 ,wds8, wds9, wds10, wds11, wds12
#else
#define DEFAULT_BIND_LAN_OFFSET		14			//0,1,2,3 ext-0, ext-1, root, wds1, wds2, wds3 ,wds4, wds5, wds6, wds7, wds8
#endif
#else
#ifdef CONFIG_RTL_VAP_SUPPORT
#define DEFAULT_BIND_LAN_OFFSET		10			//0,1,2,3 ext-0, ext-1, root, vap1, vap2, vap3, vap4
#else
#define DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif
#endif
#else
#define DEFAULT_BIND_LAN_OFFSET		5			//0,1,2,3 ext-0, ext-1
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

#define HOST_POLICING_TABLE_SIZE	32
#define V4_DEFAULT_ROUTE_IDX	15
#define V6_DEFAULT_ROUTE_IDX	7

#elif defined(CONFIG_RG_RTL9602C_SERIES)

#define HOST_POLICING_TABLE_SIZE	8
#define V4_DEFAULT_ROUTE_IDX	15
#define V6_DEFAULT_ROUTE_IDX	7

#elif defined(CONFIG_RG_RTL9600_SERIES)

#define V4_DEFAULT_ROUTE_IDX	7
#define V6_DEFAULT_ROUTE_IDX	3

#endif




#ifdef CONFIG_DUALBAND_CONCURRENT
#define SLAVE_WIFI_ROUTE_IDX	6
#define SLAVE_SSID_TAG_ETH		0x889a
#define SLAVE_SSID_TAG_LEN		4
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define DEFAULT_L3MCR_IDX		0
#define LAN_FID					0		//20170524LUKE: change to zero for invalid CVLAN may cause hashed fail problem.
#define WAN_FID					0		//20170524LUKE: change to zero for invalid CVLAN may cause hashed fail problem.
#define Layer2HOUSE_KEEP_NUM	264		//how many entries should be check at once, MUST be the common divisor of LUT table

#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define DEFAULT_L3MCR_IDX		0
#define LAN_FID					1
#define WAN_FID					1
#define Layer2HOUSE_KEEP_NUM	136		//how many entries should be check at once, MUST be the common divisor of LUT table

#elif defined(CONFIG_RG_RTL9600_SERIES)
#define LAN_FID					2
#define WAN_FID					2
#define Layer2HOUSE_KEEP_NUM	264		//how many entries should be check at once, MUST be the common divisor of LUT table

#endif

#define NIC_RX_PRIORITY			16		//used to nic register for RX


#define WanAccessCategoryNum	8
#define MAX_WanAccessARPCount	1		//means we only send "MAX_WanAccessARPCount" times to check host on-line or not, each count will ask for THREE times.

#define WanAccessLimitForbid 	0
#define WanAccessLimitCheck  	1
#define WanAccessLimitPermit 	2

//#define RTK_RG_EGRESS_QOS_TEST_PATCH	0
//#define RTK_RG_INGRESS_QOS_TEST_PATCH	0

#define RG_MIN_MRU	60

//#define PPPOE_DISCOVERY_GROUPID	0
//#define PPPOE_SESSION_GROUPID	1
/* Port and Protocol VLAN Group */
#define MAX_PORT_PROTO_GROUP_SIZE	4
#define RG_IPV4_GROUPID				0
#define RG_ARP_GROUPID				1
#define RG_IPV6_GROUPID				2
#define RG_IPV4_ETHERTYPE			0x0800
#define RG_ARP_ETHERTYPE			0x0806
#define RG_IPV6_ETHERTYPE			0x86dd
#define RG_PPPOED_ETHERTYPE         0x8863
#define RG_PPPOES_ETHERTYPE         0x8864
#define RG_DOT1X_EAPOL_ETHERTYPE    0x888E
#define RG_IPV4_PPP_PROTOCOL        0x0021
#define RG_IPV6_PPP_PROTOCOL        0x0057


#define RG_IP_PROTO_HOPOPT          0x00
#define RG_IP_PROTO_ICMP            0x01
#define RG_IP_PROTO_IGMP            0x02
#define RG_IP_PROTO_IPinIP          0x04
#define RG_IP_PROTO_TCP             0x06
#define RG_IP_PROTO_IGP             0x09
#define RG_IP_PROTO_UDP             0x11
#define RG_IP_PROTO_IPv6_Route      0x2b
#define RG_IP_PROTO_IPv6_Frag       0x2c
#define RG_IP_PROTO_IPv6            0x29
#define RG_IP_PROTO_IDRP            0x2d
#define RG_IP_PROTO_RSVP            0x2e
#define RG_IP_PROTO_GRE             0x2f
#define RG_IP_PROTO_ESP             0x32
#define RG_IP_PROTO_AH              0x33
#define RG_IP_PROTO_ICMPv6          0x3a
#define RG_IP_PROTO_IPv6_NoNxt      0x3b
#define RG_IP_PROTO_IPV6_OPTS       0x3c
#define RG_IP_PROTO_EIGRP           0x58
#define RG_IP_PROTO_OSPF            0x59
#define RG_IP_PROTO_PIM             0x67
#define RG_IP_PROTO_IPComp          0x6c
#define RG_IP_PROTO_L2TP            0x73
#define RG_IP_PROTO_SCTP            0x84
#define RG_IP_PROTO_Mob_Header      0x87
#define RG_IP_PROTO_HIP             0x8b
#define RG_IP_PROTO_Shim6           0x8c
#define RG_IP_PROTO_RESERVED        0xff

#define RTK_RG_DEFAULT_FRAGMENT_LIST_TIMEOUT	60

#if 0
#define APOLLO_SWITCH_CHIP		0x6266
#define APOLLO_SWITCH_CHIP_REV_B	0x3
#define APOLLO_SWITCH_CHIP_REV_C	0x4
//#define APOLLO_SWITCH_CHIP_REV_C	0x0 //only for debug (don't patch old ic function)
#define APOLLO_SWITCH_CHIP_REV_D	0x5
#endif


#define PATCH_6266_MTU_PPPOE		8
#define PATCH_6266_MTU_BINDING		2

#define RTK_RG_SKB_PREALLOCATE	1

#if defined(CONFIG_DUALBAND_CONCURRENT)
#define SKB_BUF_SIZE  1800
#else
#define SKB_BUF_SIZE  1600
#endif

#define JUMBO_SKB_BUF_SIZE	(13312+2)		//IXIA max packet size.
#define MASS_PACKET_SIZE	1514		//CAUTION!! this size DO NOT contain CRC length


/* RTK RG Timeout Defination */
#ifdef __KERNEL__
#define TICKTIME_PERIOD		(CONFIG_HZ)	//How many jiffies per second
#endif
#define RTK_RG_DEFAULT_HOUSE_KEEP_SECOND	2	//secs
#define RTK_RG_DEFAULT_ARP_REQUEST_INTERVAL_SECOND 	1 // 1 secs
#define RTK_RG_DEFAULT_AUTO_TEST_FAIL_ARP_INTERVAL_SECOND 4 //unit: (1/10)sec

#define RTK_RG_DEFAULT_ICMP_TRACKING_TIMEOUT_INTERVAL_SECOND 	300 // 300 secs

#define RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL 30  //secs
#define RTK_RG_DEFAULT_IGMP_SYS_TIMER_INTERVAL 10 //secs
#define RTK_RG_DEFAULT_IGMP_SYS_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT 0 //0 means unlimited size
#define RTK_RG_DEFAULT_IGMP_PORT_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT 0 //0 means unlimited size
#define RTK_RG_DEFAULT_MLD_PORT_MAX_SIMULTANEOUS_GROUP_SIZE_UNLIMIT 0 //0 means unlimited size

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define RTK_RG_DEFAULT_WIFI_FLOW_TRL_DETECT_INTERVAL 1  //unit: (1/10)sec
#endif



#define RTK_RG_SWRATELIMIT_JIFFIES	(CONFIG_HZ/2)   //unit: jiffies (nember of jiffies elasped between sw rate limit counter update)
#define	RTK_RG_SW_SHAPER_TIMER_TICK	(CONFIG_HZ/50)

#if defined(CONFIG_RG_RTL9600_SERIES)||defined(CONFIG_RG_RTL9602C_SERIES)||defined(CONFIG_RG_RTL9607C_SERIES)||defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define SW_DOS_FLOOR_THRESHOLD		20
#endif
#define RTK_RG_DEFAULT_DPI_ACCELERATE_SECONDS	0	//secs
#if defined(CONFIG_ROME_NAPT_SHORTCUT)
#define RTK_RG_DEFAULT_V4_SHORTCUT_TIMEOUT 	3600 // 1 hour
#define V4_SHORTCUT_KEEP_NUM (MAX_NAPT_SHORTCUT_SIZE>>4) // how many entries should be check at once, MUST be the common divisor of v4 shortcut table size
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define RG_V4SC_VALID(idx) (rg_db.v4ShortCutValidSet[idx>>5] & (0x1<<(idx&0x1f)))
#endif
#endif	// end CONFIG_ROME_NAPT_SHORTCUT

#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
#define RTK_RG_DEFAULT_V6_SHORTCUT_TIMEOUT 	3600 // 1 hour
#define V6_SHORTCUT_KEEP_NUM (MAX_NAPT_V6_SHORTCUT_SIZE) // how many entries should be check at once, MUST be the common divisor of v6 shortcut table size
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define RG_V6SC_VALID(idx) (rg_db.v6ShortCutValidSet[idx>>5] & (0x1<<(idx&0x1f)))
#endif
#endif	// end CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT

#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
#define RTK_RG_DEFAULT_FLOW_TIMEOUT 	60 // 60 secs
#define FLOW_HOUSE_KEEP_TICK_PERIOD 	2
#if defined(CONFIG_RG_G3_SERIES)
#define FLOW_KEEP_NUM 		8192 // how many entries should be check at once
#define SW_FLOW_KEEP_NUM 	8192 // how many entries should be check at once
#else
#if defined(CONFIG_RG_FLOW_4K_MODE)
#define FLOW_KEEP_NUM 	MAX_FLOW_HW_TABLE_SIZE // how many entries should be check at once, MUST be the common divisor of MAX_FLOW_HW_TABLE_SIZE
#else
#define FLOW_KEEP_NUM 	4096 // how many entries should be check at once, MUST be the common divisor of MAX_FLOW_HW_TABLE_SIZE
#endif
#define SW_FLOW_KEEP_NUM 	4096 // how many entries should be check at once
#endif
#endif

#define MAX_TCP_SHORT_TIMEOUT_HOUSEKEEP_RING_SIZE	2048

#define RTK_RG_DEFAULT_TCP_LONG_TIMEOUT 	3600 // 1 hour
#define RTK_RG_DEFAULT_TCP_SHORT_TIMEOUT	20	// 20 secs
#define RTK_RG_DEFAULT_UDP_LONG_TIMEOUT 	600 // 10 mins
#define RTK_RG_DEFAULT_UDP_SHORT_TIMEOUT	20	// 20 secs
#define RTK_RG_DEFAULT_ARP_TIMEOUT 	300 // 300 secs
#define RTK_RG_DEFAULT_NEIGHBOR_TIMEOUT 	300 // 300 secs
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
#define RTK_RG_DEFAULT_L2_TIMEOUT 	300 // 300 secs
#endif


/* RTK RG API */
#if defined(CONFIG_RG_G3_SERIES)
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_CPU_PORTMASK ((1<<RTK_RG_PORT_CPU) | (1<<RTK_RG_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS))
#define RTK_RG_ALL_MAC_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_CPU) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#if defined(CONFIG_ARCH_CORTINA_G3HGU)
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)	//loop back mode reserve port 5 resource to port 6, port 6 reserve to loop back function
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT0) | (1<<RTK_RG_PORT1) | (1<<RTK_RG_PORT2) | (1<<RTK_RG_PORT3) | (1<<RTK_RG_PORT4) | (1<<RTK_RG_PORT_PON) | (1<<RTK_RG_PORT_CPU) | (1<<RTK_RG_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS) | (1<<RTK_RG_EXT_PORT0) | (1<<RTK_RG_EXT_PORT1) | (1<<RTK_RG_EXT_PORT2) | (1<<RTK_RG_EXT_PORT3))
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT4) | (1<<RTK_RG_MAC_PORT_PON) | (1<<RTK_RG_MAC_PORT_CPU) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT4) | (1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT4))
#else
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT0) | (1<<RTK_RG_PORT1) | (1<<RTK_RG_PORT2) | (1<<RTK_RG_PORT3) | (1<<RTK_RG_PORT4) | (1<<RTK_RG_PORT5) | (1<<RTK_RG_PORT6) | (1<<RTK_RG_PORT_PON) | (1<<RTK_RG_PORT_CPU) | (1<<RTK_RG_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS) | (1<<RTK_RG_EXT_PORT0) | (1<<RTK_RG_EXT_PORT1) | (1<<RTK_RG_EXT_PORT2) | (1<<RTK_RG_EXT_PORT3))
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT4) | (1<<RTK_RG_MAC_PORT5) | (1<<RTK_RG_MAC_PORT6) | (1<<RTK_RG_MAC_PORT_PON) | (1<<RTK_RG_MAC_PORT_CPU) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT4) | (1<<RTK_RG_MAC_PORT5) | (1<<RTK_RG_MAC_PORT6) | (1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT4)|(1<<RTK_RG_MAC_PORT5)|(1<<RTK_RG_MAC_PORT6))
#endif
#else
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT0) | (1<<RTK_RG_PORT1) | (1<<RTK_RG_PORT2) | (1<<RTK_RG_PORT3) | (1<<RTK_RG_PORT_PON) | (1<<RTK_RG_PORT_CPU) | (1<<RTK_RG_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS) | (1<<RTK_RG_EXT_PORT0) | (1<<RTK_RG_EXT_PORT1) | (1<<RTK_RG_EXT_PORT2) | (1<<RTK_RG_EXT_PORT3))
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT_PON) | (1<<RTK_RG_MAC_PORT_CPU) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_MAC_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0) | (1<<RTK_RG_MAC_PORT1) | (1<<RTK_RG_MAC_PORT2) | (1<<RTK_RG_MAC_PORT3) | (1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3))
#endif
#define RTK_RG_ALL_MASTER_CPU_PORTMASK (1<<RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3))
#define RTK_RG_ALL_MASTER_EXT_PORTMASK (1<<RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (1<<RTK_RG_EXT_PORT1)
#define RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK ((1<<RTK_RG_PORT_CPU_WLAN0_ROOT) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP0) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP1) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP2) | (1<<RTK_RG_PORT_CPU_WLAN0_VAP3) | (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS))
#define RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)
#define RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK (RTK_RG_ALL_MASTER_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK (RTK_RG_ALL_SLAVE_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VIRUAL_PORTMASK ((1<<(RTK_RG_PORT_MAX-RTK_RG_EXT_PORT0))-1)
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK ((1<<(RTK_RG_EXT_PORT0-RTK_RG_EXT_BASED_PORT))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_EXT_BASED_PORT)))

#elif defined(CONFIG_RG_RTL9607C_SERIES)
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT_MAX) - 1)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT_MAX) - 1)
#if defined(CONFIG_DUALBAND_CONCURRENT) || defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE) || defined(CONFIG_GMAC2_USABLE)
#define RTK_RG_ALL_CPU_PORTMASK ((1<<RTK_RG_PORT_SLAVECPU) | (1<<RTK_RG_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_PORT_MASTERCPU_CORE1))
#define RTK_RG_ALL_MAC_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_SLAVECPU) | (1<<RTK_RG_MAC_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_MAC_PORT_MASTERCPU_CORE1))
#else
#define RTK_RG_ALL_CPU_PORTMASK ((1<<RTK_RG_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_PORT_MASTERCPU_CORE1))
#define RTK_RG_ALL_MAC_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_MAC_PORT_MASTERCPU_CORE1))
#endif
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU (RTK_RG_ALL_MAC_PORTMASK & ~RTK_RG_ALL_MAC_CPU_PORTMASK)
#define RTK_RG_ALL_MASTER_CPU_PORTMASK ((1<<RTK_RG_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_PORT_MASTERCPU_CORE1))
#define RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_MASTERCPU_CORE0) | (1<<RTK_RG_MAC_PORT_MASTERCPU_CORE1))
#define RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_SLAVECPU)
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5)|(1<<RTK_RG_MAC10_EXT_PORT0)|(1<<RTK_RG_MAC10_EXT_PORT1)|(1<<RTK_RG_MAC10_EXT_PORT2)|(1<<RTK_RG_MAC10_EXT_PORT3)|(1<<RTK_RG_MAC10_EXT_PORT4)|(1<<RTK_RG_MAC10_EXT_PORT5)|(1<<RTK_RG_MAC7_EXT_PORT0)|(1<<RTK_RG_MAC7_EXT_PORT1)|(1<<RTK_RG_MAC7_EXT_PORT2)|(1<<RTK_RG_MAC7_EXT_PORT3)|(1<<RTK_RG_MAC7_EXT_PORT4)|(1<<RTK_RG_MAC7_EXT_PORT5))
#define RTK_RG_ALL_VIRUAL_PORTMASK ((1<<(RTK_RG_PORT_MAX-RTK_RG_EXT_PORT0))-1)
#if 1//defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#define RTK_RG_ALL_MASTER_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5)|(1<<RTK_RG_MAC10_EXT_PORT0)|(1<<RTK_RG_MAC10_EXT_PORT1)|(1<<RTK_RG_MAC10_EXT_PORT2)|(1<<RTK_RG_MAC10_EXT_PORT3)|(1<<RTK_RG_MAC10_EXT_PORT4)|(1<<RTK_RG_MAC10_EXT_PORT5))
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK ((1<<RTK_RG_MAC7_EXT_PORT0)|(1<<RTK_RG_MAC7_EXT_PORT1)|(1<<RTK_RG_MAC7_EXT_PORT2)|(1<<RTK_RG_MAC7_EXT_PORT3)|(1<<RTK_RG_MAC7_EXT_PORT4)|(1<<RTK_RG_MAC7_EXT_PORT5))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK RTK_RG_ALL_VIRUAL_PORTMASK // T.B.D.
#else
#define RTK_RG_ALL_MASTER_EXT_PORTMASK (1<<RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (1<<RTK_RG_EXT_PORT1)
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK ((1<<(RTK_RG_EXT_PORT0-RTK_RG_EXT_BASED_PORT))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_EXT_BASED_PORT)))
#endif
#define RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK RTK_RG_ALL_MASTER_EXT_PORTMASK
#define RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK RTK_RG_ALL_SLAVE_EXT_PORTMASK
#define RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK (RTK_RG_ALL_MASTER_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK (RTK_RG_ALL_SLAVE_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT4))

#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT_MAX) - 1)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT_MAX) - 1)
#define RTK_RG_ALL_CPU_PORTMASK ((1<<RTK_RG_PORT_CPU))
#define RTK_RG_ALL_MAC_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_MASTER_CPU_PORTMASK ((1<<RTK_RG_PORT_CPU))
#define RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK ((1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5))
#define RTK_RG_ALL_VIRUAL_PORTMASK ((1<<(RTK_RG_PORT_MAX-RTK_RG_EXT_PORT0))-1)
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
#define RTK_RG_ALL_MASTER_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5))
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (0)
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK RTK_RG_ALL_VIRUAL_PORTMASK // T.B.D.
#else
#define RTK_RG_ALL_MASTER_EXT_PORTMASK (1<<RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (1<<RTK_RG_EXT_PORT1)
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK ((1<<(RTK_RG_EXT_PORT0-RTK_RG_EXT_BASED_PORT))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_EXT_BASED_PORT)))
#endif
#define RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK RTK_RG_ALL_MASTER_EXT_PORTMASK
#define RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK RTK_RG_ALL_SLAVE_EXT_PORTMASK
#define RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK (RTK_RG_ALL_MASTER_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK (RTK_RG_ALL_SLAVE_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3))

#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_PORT_CPU)
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT_MAX) - 1)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_CPU_PORTMASK (1<<RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT_PON))
#define RTK_RG_ALL_MASTER_CPU_PORTMASK (1<<RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK	(1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4)|(1<<RTK_RG_EXT_PORT5))
#define RTK_RG_ALL_MASTER_EXT_PORTMASK (1<<RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (1<<RTK_RG_EXT_PORT1)
#define RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK RTK_RG_ALL_MASTER_EXT_PORTMASK
#define RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK RTK_RG_ALL_SLAVE_EXT_PORTMASK
#define RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK (RTK_RG_ALL_MASTER_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK (RTK_RG_ALL_SLAVE_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT2-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT3-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT4-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT5-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1))

#elif defined(CONFIG_RG_RTL9600_SERIES)
#define RTK_RG_MAX_EXT_PORT (RTK_RG_PORT_MAX - RTK_RG_PORT_CPU)
#define RTK_RG_ALL_PORTMASK ((1<<RTK_RG_PORT_MAX) - 1)
#define RTK_RG_ALL_MAC_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII)|(1<<RTK_RG_MAC_PORT_CPU))
#define RTK_RG_ALL_CPU_PORTMASK (1<<RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3)|(1<<RTK_RG_MAC_PORT_PON)|(1<<RTK_RG_MAC_PORT_RGMII))
#define RTK_RG_ALL_MASTER_CPU_PORTMASK (1<<RTK_RG_PORT_CPU)
#define RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK	(1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK (1<<RTK_RG_MAC_PORT_CPU)
#define RTK_RG_ALL_EXT_PORTMASK ((1<<RTK_RG_EXT_PORT0)|(1<<RTK_RG_EXT_PORT1)|(1<<RTK_RG_EXT_PORT2)|(1<<RTK_RG_EXT_PORT3)|(1<<RTK_RG_EXT_PORT4))
#define RTK_RG_ALL_MASTER_EXT_PORTMASK (1<<RTK_RG_EXT_PORT0)
#define RTK_RG_ALL_SLAVE_EXT_PORTMASK (1<<RTK_RG_EXT_PORT1)
#define RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK RTK_RG_ALL_MASTER_EXT_PORTMASK
#define RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK RTK_RG_ALL_SLAVE_EXT_PORTMASK
#define RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK (RTK_RG_ALL_MASTER_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK (RTK_RG_ALL_SLAVE_EXT_PORTMASK>>RTK_RG_EXT_BASED_PORT)
#define RTK_RG_ALL_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT2-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT3-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT4-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_USED_VIRUAL_PORTMASK (1|(1<<(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))|(1<<(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)))
#define RTK_RG_ALL_LAN_PORTMASK ((1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT1)|(1<<RTK_RG_MAC_PORT2)|(1<<RTK_RG_MAC_PORT3))

#else
#error
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define RTK_RG_IS_WAN_PORT(port) ((port==RTK_RG_PORT_PON) || (((0x1<<port) & rg_db.systemGlobal.wanPortMask.portmask) != 0x0))
#else
#define RTK_RG_IS_WAN_PORT(port) (port==RTK_RG_PORT_PON)
#endif

#define RG_INVALID_PORT(port) (((0x1<<port) & RTK_RG_ALL_PORTMASK)==0x0)
#define RG_INVALID_PORTMASK(portMsk) ((portMsk & RTK_RG_ALL_PORTMASK)!=portMsk)
#if defined(CONFIG_APOLLOPRO_FPGA)
#define RG_INVALID_MAC_PORT(port) (((0x1<<port) & RTK_RG_ALL_MAC_PORTMASK)==0x0)
#define RG_INVALID_MAC_PORTMASK(portMsk) ((portMsk & RTK_RG_ALL_MAC_PORTMASK)!=portMsk)
#else
#define RG_INVALID_MAC_PORT(port) (((0x1<<port) & RTK_RG_ALL_MAC_PORTMASK & rg_db.systemGlobal.phyPortStatus)==0x0)
#define RG_INVALID_MAC_PORTMASK(portMsk) ((portMsk & RTK_RG_ALL_MAC_PORTMASK & rg_db.systemGlobal.phyPortStatus)!=portMsk)
#endif

#define RG_INVALID_UINT32_ENTRY_BIT_CONTINUE(check_array, index) { \
		if(check_array[(index>>5)] == 0x0){ index += 31; continue;} \
		else if((check_array[(index>>5)]&(1<<(index&0x1f))) == 0x0){ continue;} \
	}


//#define MAX_ASIC_NXTHOP_TBL_SIZE 32
//#define MAX_ASIC_NETIF_TBL_SIZE 8

#define RTK_RG_DEFAULT_HTTP_MONITOR_PORT 80
#define RTK_RG_DEFAULT_HTTPS_MONITOR_PORT 443

#define RTK_RG_DEFAULT_LOCAL_IN_NAPT_ADD_TO_SHORTCUT 1

#define RTK_RG_DEFAULT_WHITELIST_STATE 0

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define RTK_RG_DEFAULT_CF1_OVERRIDE 0
#define RTK_RG_DEFAULT_REPLACE_HW_FLOW 1
#endif

//ipset feature - CONFIG_RG_IPSET_VERSION==1
#define MAX_IPSET_IDX_WIDTH 		1	//default is 1
#define MAX_IPSET_IDX_SIZE 			(1<<MAX_IPSET_IDX_WIDTH)
#define MAX_IPSET_SW_ENTRY_SIZE 	64	//default is 64
#define MAX_IPSET_HASH_WIDTH		8	//default is 8
#define MAX_IPSET_HASH_SIZE			(1<<MAX_IPSET_HASH_WIDTH)
#define MAX_IPSET_HASH_IPV6_WIDTH	48	//default is 48
//ipset feature - CONFIG_RG_IPSET_VERSION==2
#define MAX_IPSETS_GROUP_SIZE 		4		//default is 4
#define MAX_IPSETS_IDX_SIZE 		20		//default is 20, should not larger than 65535 due to only support one way rule idx link list
#define MAX_IPSETS_SW_ENTRY_SIZE 	1000	//default is 1000, should not larger than 65535 due to only support one way rule idx link list
#define MAX_IPSETS_HASH_WIDTH		8		//default is 8
#define MAX_IPSETS_HASH_SIZE			(1<<MAX_IPSETS_HASH_WIDTH)
#define IPSETS_GROUP_PRI_DEF 		255		//priority 8 bit
#define IPSETS_TARGET_LEVEL_DEF 	15		//level 4 bit

//20141219LUKE: used to transform egress WAN intf to ingress pattern
#define CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM 1

//NAPT filter for priority (pure software)
#define MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT 8
#define MAX_NAPT_FILER_SW_ENTRY_SIZE (0x1<<MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT)

//GPON Broadcast to Lan filter and REmarking
#define MAX_GPON_DS_BC_FILTER_SW_ENTRY_SIZE 128

//vlan group mac limit
#define MAX_VLAN_GROUP_MAC_LIMIT_NUMBER 32

/*StormControl*/
#define MAX_STORMCONTROL_ENTRY_SIZE 32

/* Queue weights structure */
#define RTK_RG_MAX_NUM_OF_QUEUE 8
#define RTK_RG_MAX_NUM_OF_GPON_QUEUE 128

/* Cpu Tag */
#define RTK_RG_CPU_TAG_LEN 8

/* SVLAN Module*/
/* SVlan */
#define SVLANMBRTBL_SIZE 		64		// 64 member config
#define SVLANMC2STBL_SIZE 		8		// 8 MC2S, Multicast to SVID
#define SVLANC2STBL_SIZE 		128		// 128 C2S, CVID/PVID to SVID
#define SVLANSP2CTBL_SIZE 		128		// 128 SP2C, SVID to CVID/PVID

/* ALG Module */
#define MAX_ALG_FUNCTIONS		32
//#define FTP_PORT_STR			0x504f5254		//LUKE20130816: move to separate ALG file
//#define FTP_PASV_RESP_STR		0x32323720		//LUKE20130816: move to separate ALG file
#define L2TP_CTRL_MSG_STR		0xc802
//#define MAX_FTP_CTRL_FLOW_SIZE	32			//LUKE20130816: move to separate ALG file
#define ALG_SRV_IN_LAN_IDX		8
#define MAX_L2TP_CTRL_FLOW_SIZE	32

#define ALG_RTSP_UPNP_TIMEOUT		300
#define ALG_H323_UPNP_TIMEOUT		300
#define ALG_FTP_PASV_UPNP_TIMEOUT	30		//secs
#define ALG_FTP_ACTIVE_UPNP_TIMEOUT	30		//secs
#define ALG_SIP_UPNP_TIMEOUT		30
#define ALG_BATTLENET_UPNP_TIMEOUT	30000
#define ALG_TFTP_UPNP_TIMEOUT		30
#define ALG_FCC_UPNP_TIMEOUT		30
#define ALG_SIP_PRESERVE_TIMEOUT	30


#define PPTP_CTRL_MSG_TYPE		1		// 1 for Control Message
#define PPTP_MAGIC				0x1a2b3c4d
#define MAX_PPTP_SESSION_SIZE	16
#define MAX_ALG_SERV_IN_LAN_NUM		8
#define MAX_ALG_DYNAMIC_PORT_NUM	256
#define MAX_ALG_PRESERVE_EXTPORT_NUM	256

#define CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT	1
#define RTK_RG_MAX_SC_CLIENT	1

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define DEFAULT_MAX_FLOW_COUNT	512
#define DEFAULT_MAX_DATA_PATTEN	128

#endif


/* HW & SW Combine tables */
#define MAX_VLAN_HW_TABLE_SIZE	4096
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define MAX_VLAN_SW_TABLE_SIZE	4352		//reserved 256 for RG reserved and upper application.
#else
#define MAX_VLAN_SW_TABLE_SIZE	4096
#endif
#define MAX_LUT_SW_LIST_SIZE	512		//used for access WAN limit

//wifi tx qos mapping
#define RTK_WIFI_TX_QUEUE_VALUE_DISABLED	-1
#define RTK_WIFI_TX_QUEUE_VALUE_BK	1
#define RTK_WIFI_TX_QUEUE_VALUE_BE	3
#define RTK_WIFI_TX_QUEUE_VALUE_VI	5
#define RTK_WIFI_TX_QUEUE_VALUE_VO	7

//--------------------------------------------------------------
#if defined(CONFIG_RG_G3_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE_SHIFT	5
#define LUT_HW_TABLE_SIZE_SHIFT			12
#define MAX_LUT_HASH_WAY_SHIFT			3

#define MAX_NETIF_SW_TABLE_SIZE_SHIFT	4
#define MAX_NETIF_HW_TABLE_SIZE_SHIFT	4

#define MAX_EXTIP_SW_TABLE_SIZE_SHIFT	4
#define MAX_EXTIP_HW_TABLE_SIZE_SHIFT 	0

#define MAX_L3_SW_TABLE_SIZE_SHIFT		4
#define MAX_L3_HW_TABLE_SIZE_SHIFT		0

#define MAX_ARP_SW_TABLE_SIZE_SHIFT		(LUT_HW_TABLE_SIZE_SHIFT-1) //11
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		0

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_LUT_BCAM_TABLE_SIZE_FPGA	4
#endif
//--------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9607C_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE_SHIFT	6
#define LUT_HW_TABLE_SIZE_SHIFT			11
#define MAX_LUT_HASH_WAY_SHIFT			2

#define MAX_NETIF_SW_TABLE_SIZE_SHIFT	4
#define MAX_NETIF_HW_TABLE_SIZE_SHIFT	4

#define MAX_EXTIP_SW_TABLE_SIZE_SHIFT	4
#define MAX_EXTIP_HW_TABLE_SIZE_SHIFT 	0

#define MAX_L3_SW_TABLE_SIZE_SHIFT		4
#define MAX_L3_HW_TABLE_SIZE_SHIFT		0

#define MAX_ARP_SW_TABLE_SIZE_SHIFT		(LUT_HW_TABLE_SIZE_SHIFT-1) //10
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		0

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_LUT_BCAM_TABLE_SIZE_FPGA	4
#endif
//--------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE_SHIFT	3
#define LUT_HW_TABLE_SIZE_SHIFT			11
#define MAX_LUT_HASH_WAY_SHIFT			2

#define MAX_NETIF_SW_TABLE_SIZE_SHIFT	4
#define MAX_NETIF_HW_TABLE_SIZE_SHIFT	4

#define MAX_EXTIP_SW_TABLE_SIZE_SHIFT	4
#define MAX_EXTIP_HW_TABLE_SIZE_SHIFT 	0

#define MAX_L3_SW_TABLE_SIZE_SHIFT		4
#define MAX_L3_HW_TABLE_SIZE_SHIFT		0

#define MAX_ARP_SW_TABLE_SIZE_SHIFT		(LUT_HW_TABLE_SIZE_SHIFT-1) //10
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		0

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_LUT_BCAM_TABLE_SIZE_FPGA	4
#endif
//--------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE_SHIFT	6
#define LUT_HW_TABLE_SIZE_SHIFT			10
#define MAX_LUT_HASH_WAY_SHIFT			2

#define MAX_NETIF_SW_TABLE_SIZE_SHIFT	4
#define MAX_NETIF_HW_TABLE_SIZE_SHIFT	4

#define MAX_EXTIP_SW_TABLE_SIZE_SHIFT	4
#define MAX_EXTIP_HW_TABLE_SIZE_SHIFT 	3

#define MAX_L3_SW_TABLE_SIZE_SHIFT		4
#define MAX_L3_HW_TABLE_SIZE_SHIFT		4

#define MAX_ARP_SW_TABLE_SIZE_SHIFT		(LUT_HW_TABLE_SIZE_SHIFT-1) //9

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_LUT_BCAM_TABLE_SIZE_FPGA	8
#define MAX_ARP_HW_TABLE_SIZE_FPGA		16
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		5	//FPGA only 32 entries, but real IC is 128 entries.
#else
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		7
#endif
#define MAX_DSLITE_SW_TABLE_SIZE		4
#define MAX_DSLITE_HW_TABLE_SIZE		4
#define MAX_IPMV6_L3MCR_TABL_SIZE 		8 //each entry per port point to multicast routing translate table (for rtk_l34_ip6mcRoutingTransIdx_set/get)

//--------------------------------------------------------------
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_LUT_BCAM_TABLE_SIZE_SHIFT	6
#define LUT_HW_TABLE_SIZE_SHIFT			11
#define MAX_LUT_HASH_WAY_SHIFT			2

#define MAX_NETIF_SW_TABLE_SIZE_SHIFT	4
#define MAX_NETIF_HW_TABLE_SIZE_SHIFT	3

#define MAX_EXTIP_SW_TABLE_SIZE_SHIFT	4
#define MAX_EXTIP_HW_TABLE_SIZE_SHIFT 	3

#define MAX_L3_SW_TABLE_SIZE_SHIFT		4
#define MAX_L3_HW_TABLE_SIZE_SHIFT		3

#define MAX_ARP_SW_TABLE_SIZE_SHIFT		(LUT_HW_TABLE_SIZE_SHIFT-1) //10

#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
#define MAX_ARP_HW_TABLE_SIZE_FPGA		16
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		5 //FPGA only 32 entries, but real IC is 128 entries.
#else
#define MAX_ARP_HW_TABLE_SIZE_SHIFT		9
#endif
//--------------------------------------------------------------

#endif

#if MAX_ARP_HW_TABLE_SIZE_SHIFT>=MAX_ARP_SW_TABLE_SIZE_SHIFT
#error "ARP SW/HW table size configure error!"
#endif

// lut table
#define MAX_LUT_BCAM_TABLE_SIZE		(1 << MAX_LUT_BCAM_TABLE_SIZE_SHIFT)
#define LUT_HW_TABLE_SIZE			(1 << LUT_HW_TABLE_SIZE_SHIFT)
#define MAX_LUT_HW_TABLE_SIZE		(LUT_HW_TABLE_SIZE+MAX_LUT_BCAM_TABLE_SIZE)
#define MAX_LUT_HASH_WAY_SIZE		(1 << MAX_LUT_HASH_WAY_SHIFT)
#define MAX_LUT_SW_TABLE_SIZE		(MAX_LUT_HW_TABLE_SIZE+MAX_LUT_SW_LIST_SIZE)

#define MAX_LUT_SW_TABLE_HEAD		(LUT_HW_TABLE_SIZE>>MAX_LUT_HASH_WAY_SHIFT)		//software lut list head size

#define MAX_LUT_HW_HASH_HEAD		(MAX_LUT_HW_TABLE_SIZE>>MAX_LUT_HASH_WAY_SHIFT)	//hw lut and bcam hash list head size

#define MAX_FORCIBLY_BINDING_MAC_SIZE	4
#define MAX_FORCIBLY_BINDING_KEY_LEN	64

// netif table
#define MAX_NETIF_SW_TABLE_SIZE		(1 << MAX_NETIF_SW_TABLE_SIZE_SHIFT)	//please check MAX_NETIF_SW_TABLE_SIZE= (SC_BFW_INTFIDX -1)
#define MAX_NETIF_HW_TABLE_SIZE		(1 << MAX_NETIF_HW_TABLE_SIZE_SHIFT)
#if defined(CONFIG_RG_WAN_MSS_CACHE)
#define MAX_NETIF_MSSCACHE_HASHBITS	7		//128
#define MAX_NETIF_MSSCACHE_HASH		(1<<MAX_NETIF_MSSCACHE_HASHBITS)
#define	MAX_NETIF_MSSCACHE_POOL		512		//must be multiplier of MAX_NETIF_MSSCACHE_HASH
#define MAX_NETIF_MSSCACHE_WAYS		MAX_NETIF_MSSCACHE_POOL/MAX_NETIF_MSSCACHE_HASH
#endif
#define DEFAULT_MSSCACHE_VALUE		1460
#define DEFAULT_V6MSSCACHE_VALUE	1440

// external ip table
#define MAX_EXTIP_SW_TABLE_SIZE 	(1 << MAX_EXTIP_SW_TABLE_SIZE_SHIFT)		//please check MAX_EXTIP_SW_TABLE_SIZE= 1<<(SC_BFW_EIPIDX-1)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_EXTIP_HW_TABLE_SIZE 	0
#else
#define MAX_EXTIP_HW_TABLE_SIZE 	(1 << MAX_EXTIP_HW_TABLE_SIZE_SHIFT)
#endif

// arp table
#define MAX_ARP_SW_TABLE_SIZE		(1 << MAX_ARP_SW_TABLE_SIZE_SHIFT)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_ARP_HW_TABLE_SIZE		0
#else
#define MAX_ARP_HW_TABLE_SIZE		(1 << MAX_ARP_HW_TABLE_SIZE_SHIFT)
#endif

// l3 table
#define MAX_L3_SW_TABLE_SIZE		(1 << MAX_L3_SW_TABLE_SIZE_SHIFT)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_L3_HW_TABLE_SIZE		0
#else
#define MAX_L3_HW_TABLE_SIZE		(1 << MAX_L3_HW_TABLE_SIZE_SHIFT)
#endif

#define MAX_PPPOE_SW_TABLE_SIZE		16
#define MAX_NEXTHOP_SW_TABLE_SIZE	32
#define MAX_DSLITEMC_SW_TABLE_SIZE	4
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_PPPOE_HW_TABLE_SIZE		0
#define MAX_NEXTHOP_HW_TABLE_SIZE	0
#define MAX_DSLITEMC_HW_TABLE_SIZE	0
#else
#define MAX_PPPOE_HW_TABLE_SIZE		8
#define MAX_NEXTHOP_HW_TABLE_SIZE	16
#define MAX_DSLITEMC_HW_TABLE_SIZE	4
#endif

#define MAX_STATIC_ROUTE_SIZE		MAX_L3_SW_TABLE_SIZE

#define MAX_DSLITE_MTU_SIZE			1460		//1500 pre-minus IPV6 headers
#define MAX_PPPOEDSLITE_MTU_SIZE	1452		//1492 pre-minus IPV6 headers

#if defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#if defined(CONFIG_RG_SW_NAPT_SIZE_SHIFT) && (CONFIG_RG_SW_NAPT_SIZE_SHIFT!=0)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   CONFIG_RG_SW_NAPT_SIZE_SHIFT
#else	// does not set CONFIG_RG_SW_NAPT_SIZE
#ifdef CONFIG_RG_SW_NAPT_SIZE_L
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   16	//65536
#else
#define MAX_NAPT_IN_SW_TABLE_SHIFT   15
#endif
#elif defined(CONFIG_RG_SW_NAPT_SIZE_M)
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   14	//16384
#else
#define MAX_NAPT_IN_SW_TABLE_SHIFT   14
#endif
#else	//not CONFIG_RG_SW_NAPT_SIZE_L && not CONFIG_RG_SW_NAPT_SIZE_M
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   13	//8192
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   13
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_NAPT_IN_SW_TABLE_SHIFT   12
#endif
#endif
#endif	// end CONFIG_RG_SW_NAPT_SIZE
#define MAX_NAPT_IN_SW_TABLE_SIZE	(1 << MAX_NAPT_IN_SW_TABLE_SHIFT)


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SHIFT	0
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SHIFT	12
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SHIFT	10
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_IN_HW_TABLE_SIZE	0
#define MAX_NAPT_IN_HASH_SIZE		1024
#else
#define MAX_NAPT_IN_HW_TABLE_SIZE	(1 << MAX_NAPT_IN_HW_TABLE_SHIFT)
#define MAX_NAPT_IN_HASH_SIZE		(MAX_NAPT_IN_HW_TABLE_SIZE>>2)
#endif

#define MAX_NAPT_OUT_SW_TABLE_SHIFT   MAX_NAPT_IN_SW_TABLE_SHIFT
#define MAX_NAPT_OUT_SW_TABLE_SIZE	(1 << MAX_NAPT_OUT_SW_TABLE_SHIFT)


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SHIFT	0
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SHIFT	12
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SHIFT	11
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_NAPT_OUT_HW_TABLE_SIZE	0
#define MAX_NAPT_OUT_HASH_SIZE		1024
#else
#define MAX_NAPT_OUT_HW_TABLE_SIZE	(1 << MAX_NAPT_OUT_HW_TABLE_SHIFT)
#define MAX_NAPT_OUT_HASH_SIZE 		(MAX_NAPT_OUT_HW_TABLE_SIZE>>2)
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#if defined(CONFIG_RG_G3_SERIES)
#define MAX_SHAREMETER_TABLE_SIZE 32 //l2 meter size (Every 32 policers used for a function)
#else
#define MAX_SHAREMETER_TABLE_SIZE 48 //l2 meter size
#endif
#define MAX_FLOWBASEDMETER_TABLE_SIZE FLOWBASED_TABLESIZE_SHAREMTR
#define MAX_FUNCBASEDMETER_SIZE 32 //sw maintained meter
#if defined(CONFIG_RG_G3_SERIES)
#define G3_FLOW_POLICER_FLOWMTR_SIZE		MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for flow meter
#define G3_FLOW_POLICER_FLOWMIB_SIZE		MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for flow mib
#define G3_FLOW_POLICER_HOSTPOLICING_SIZE	MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for host policing
#define G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE	MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for host policing rx (pure logging)
#define G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE	MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for host policing tx (pure logging)
#define G3_FLOW_POLICER_STORMCONTROL_SIZE	MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for storm control
#define G3_FLOW_POLICER_PROC_SIZE			MAX_FUNCBASEDMETER_SIZE		//the number of flow policers used for proc rate limit
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMTR	0
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMIB	(G3_FLOW_POLICER_IDXSHIFT_FLOWMTR + G3_FLOW_POLICER_FLOWMTR_SIZE)
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
/*SMAC and DMAC host policing use the different policer group, SMAC use the same index range with flow mib (SMAC host policing: L2; flow mib: L3)*/
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX	(G3_FLOW_POLICER_IDXSHIFT_FLOWMTR + G3_FLOW_POLICER_FLOWMTR_SIZE) //SMAC
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX	(G3_FLOW_POLICER_IDXSHIFT_FLOWMIB + G3_FLOW_POLICER_FLOWMIB_SIZE) //DMAC
#else
/*SMAC and DMAC host policing use the same policer group*/
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX	(G3_FLOW_POLICER_IDXSHIFT_FLOWMIB + G3_FLOW_POLICER_FLOWMIB_SIZE) //SMAC
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX	(G3_FLOW_POLICER_IDXSHIFT_FLOWMIB + G3_FLOW_POLICER_FLOWMIB_SIZE) //DMAC
#endif
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGRX	(G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX + G3_FLOW_POLICER_HOSTPOLICING_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGTX	(G3_FLOW_POLICER_IDXSHIFT_HPLOGRX + G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_STORMCTL	(G3_FLOW_POLICER_IDXSHIFT_HPLOGTX + G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_PROC		(G3_FLOW_POLICER_IDXSHIFT_STORMCTL + G3_FLOW_POLICER_STORMCONTROL_SIZE)
#endif
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_SHAREMETER_TABLE_SIZE 16 //l2 meter size
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_SHAREMETER_TABLE_SIZE 32 //l2 meter size
#endif

#if defined(CONFIG_RG_SW_NAPT_SIZE_L) || defined(CONFIG_RG_SW_NAPT_SIZE_M)
#if MAX_NAPT_OUT_SW_TABLE_SIZE > 65536
#error "Out of FwdEngine NAPT flow size"
#endif
#else
#if MAX_NAPT_OUT_SW_TABLE_SIZE > 32767
#error "Out of FwdEngine NAPT flow size"
#endif
#endif

#if MAX_NAPT_OUT_HW_TABLE_SIZE > MAX_NAPT_OUT_SW_TABLE_SIZE
#error "NAPT SW/HW table size configure error!"
#endif

#endif

#if defined(CONFIG_RG_G3_SERIES)
#define MAX_HW_SHAREMETER_TABLE_SIZE (256-32) //the last 32 policers are used by rtk DoS security
#else
#define MAX_HW_SHAREMETER_TABLE_SIZE MAX_SHAREMETER_TABLE_SIZE
#endif
#define PURE_SW_SHAREMETER_TABLE_SIZE_SHIFT 5
#define PURE_SW_SHAREMETER_TABLE_SIZE (1<<PURE_SW_SHAREMETER_TABLE_SIZE_SHIFT)
#define PURE_SW_METER_IDX_OFFSET 48

#define MAX_SW_SHAPER_TABLE_SIZE		PURE_SW_SHAREMETER_TABLE_SIZE
#define MAX_SW_SHAPER_QUEUE_SIZE 		64		//bucket size for busting

#define MAX_FLOW_HIGHPRI_TABLE_SIZE		256
#define MAX_FLOW_URLPRI_IDX_TABLE_SIZE		1024

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)

#if defined(CONFIG_RG_FLOW_4K_MODE)
#define MAX_FLOW_TABLE_SIZE			FLOWBASED_TABLESIZE_FLOWSRAM
#define MAX_FLOW_TCAM_TABLE_SIZE		FLOWBASED_TABLESIZE_FLOWTCAM
#elif defined(CONFIG_RG_FLOW_8K_MODE)
#define MAX_FLOW_TABLE_SIZE			FLOWTABLE_DDR_8K
#elif defined(CONFIG_RG_FLOW_16K_MODE)
#define MAX_FLOW_TABLE_SIZE			FLOWTABLE_DDR_16K
#elif defined(CONFIG_RG_FLOW_32K_MODE)
#define MAX_FLOW_TABLE_SIZE			FLOWTABLE_DDR_32K
#elif defined(CONFIG_APOLLOPRO_FPGA)	// for model test case verification
#define MAX_FLOW_TABLE_SIZE			FLOWBASED_TABLESIZE_FLOWSRAM
#endif

#if defined(CONFIG_RG_FLOW_4K_MODE)
#define MAX_FLOW_WAYS_SHIFT			2
#else
#define MAX_FLOW_WAYS_SHIFT			0
#endif
#define MAX_FLOW_WAYS				(1<<MAX_FLOW_WAYS_SHIFT)

#if defined(CONFIG_RG_FLOW_4K_MODE)
#define MAX_FLOW_HW_TABLE_SIZE (MAX_FLOW_TABLE_SIZE+MAX_FLOW_TCAM_TABLE_SIZE)
#else
#define MAX_FLOW_HW_TABLE_SIZE MAX_FLOW_TABLE_SIZE
#endif

#if defined(CONFIG_RG_FLOW_SW_TABLE_SIZE) && (CONFIG_RG_FLOW_SW_TABLE_SIZE!=0)
#define MAX_FLOW_SW_TABLE_SIZE CONFIG_RG_FLOW_SW_TABLE_SIZE
#else
#define MAX_FLOW_SW_TABLE_SIZE (1<<(MAX_NAPT_OUT_SW_TABLE_SHIFT+1))
#endif

#if (MAX_FLOW_SW_TABLE_SIZE>>5)==0
#define MAX_FLOW_SW_VALID_BITS_ARRAY_SIZE (MAX_FLOW_SW_TABLE_SIZE>>5)
#else
#define MAX_FLOW_SW_VALID_BITS_ARRAY_SIZE ((MAX_FLOW_SW_TABLE_SIZE>>5)+1)
#endif


#define MAX_EXTPORT_TABLE_SIZE		FLOWBASED_TABLESIZE_EXTPORT
#define MAX_INDMAC_TABLE_SIZE			FLOWBASED_TABLESIZE_MACIND
#define MAX_ETHTYPE_TABLE_SIZE		FLOWBASED_TABLESIZE_ETHERTYPE
#define MAX_WANACCESSLIMIT_TABLE_SIZE FLOWBASED_TABLESIZE_WAL
#define MAX_FLOWMIB_TABLE_SIZE 		FLOWBASED_TABLESIZE_FLOWMIB
#define MAX_EXTRATAG_TABLE_SIZE 		FLOWBASED_EXTRATAG_LISTMAX

#define EXTPTBL_RSVED_ENTRY			1	// reserve entry[0] to indicate none extension port setting

#define PURE_SW_FLOWMIB_TABLE_SIZE 32
#define PURE_SW_FLOWMIB_IDX_OFFSET MAX_FLOWMIB_TABLE_SIZE
#endif //CONFIG_RG_RTL9607C_SERIES or CONFIG_RG_G3_SERIES


#define MAX_FLOW_TCP_UDP_TRACKING_TABLE_SHIFT	12
#define MAX_FLOW_TCP_UDP_TRACKING_TABLE_SIZE	(1<<MAX_FLOW_TCP_UDP_TRACKING_TABLE_SHIFT)
#define MAX_FLOW_TCP_UDP_TRACKING_GROUP_SHIFT	14 //MAX_NAPT_OUT_SW_TABLE_SHIFT
#define MAX_FLOW_TCP_UDP_TRACKING_GROUP_SIZE	(1<<MAX_FLOW_TCP_UDP_TRACKING_GROUP_SHIFT)

#endif //CONFIG_RG_FLOW_BASED_PLATFORM

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_BIND_SW_TABLE_SIZE	32
#define MAX_BIND_HW_TABLE_SIZE	0
#define MAX_WANTYPE_SW_TABLE_SIZE	16
#define MAX_WANTYPE_HW_TABLE_SIZE	0
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_BIND_SW_TABLE_SIZE	32
#define MAX_BIND_HW_TABLE_SIZE	32
#define MAX_WANTYPE_SW_TABLE_SIZE	16
#define MAX_WANTYPE_HW_TABLE_SIZE	16
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_BIND_SW_TABLE_SIZE	32
#define MAX_BIND_HW_TABLE_SIZE	32
#define MAX_WANTYPE_SW_TABLE_SIZE	16
#define MAX_WANTYPE_HW_TABLE_SIZE	8
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_IPMCGRP_HW_TABLE_SIZE 64
#define MAX_IPMCGRP_NON_HW_TABLE_SIZE 0 //not support
#define MAX_IPMCFILTER_HW_TABLE_SIZE 0
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_IPMCGRP_HW_TABLE_SIZE 0	  //not support
#define MAX_IPMCGRP_NON_HW_TABLE_SIZE 64
#define MAX_IPMCFILTER_HW_TABLE_SIZE 16
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_IPMCGRP_HW_TABLE_SIZE 64
#define MAX_IPMCGRP_NON_HW_TABLE_SIZE 0
#define MAX_IPMCFILTER_HW_TABLE_SIZE 0 //not support
#endif
#define MAX_IPMCGRP_SW_TABLE_SIZE (MAX_IPMCGRP_HW_TABLE_SIZE+MAX_IPMCGRP_NON_HW_TABLE_SIZE)



/* debug flags */

// for bit field width checking of shortcut table
//#define SHORTCUT_BITFILED_DEBUG

// for napt table entry width
//#define NAPT_TABLE_SIZE_DEBUG




// rtk_rg_table_naptIn_t

#define MAX_NAPT_SW_ENTRY_WAYS_SHIFT    2
#define MAX_NAPT_IN_SW_ENTRY_WIDTH      (MAX_NAPT_IN_SW_TABLE_SHIFT-MAX_NAPT_SW_ENTRY_WAYS_SHIFT)
#define MAX_NAPT_IN_IDLESEC_WIDTH       12
#define MAX_NAPT_IN_REFCOUNT_WIDTH      1
#define MAX_NAPT_IN_CONETYPE_WIDTH      2
#define MAX_NAPT_IN_NAPTOUTIDX_WIDTH    MAX_NAPT_OUT_SW_TABLE_SHIFT
#define MAX_NAPT_IN_CANBEREPLACE_WIDTH  1
#define MAX_NAPT_IN_PRIVALID_WIDTH      1
#define MAX_NAPT_IN_PRIVALUE_WIDTH      3
#define MAX_NAPT_IN_CANTADDTOHW_WIDTH   1

// rtk_rg_table_naptOut_t
#define MAX_NAPT_OUT_SW_ENTRY_WIDTH     (MAX_NAPT_OUT_SW_TABLE_SHIFT-MAX_NAPT_SW_ENTRY_WAYS_SHIFT)
#define MAX_NAPT_OUT_STATE_WIDTH        4
#define MAX_NAPT_OUT_PORT_WIDTH         16
#define MAX_NAPT_OUT_IDLESEC_WIDTH      12
#define MAX_NAPT_OUT_CANTADDTOHW_WIDTH  2
#define MAX_NAPT_OUT_PRIVALID_WIDTH     1
#define MAX_NAPT_OUT_PRIVALUE_WIDTH     3
#define MAX_NAPT_OUT_CANBEREPLACE_WIDTH 1
#define MAX_NAPT_OUT_RECORD_WIDTH 		1
#define MAX_NAPT_OUT_LOOKUPHIT_WIDTH 	2
#define MAX_NAPT_OUT_FORCE_EXTPORT_WIDTH 1
#define MAX_NAPT_OUT_CPRI_EN_WIDTH 		1
#define MAX_NAPT_OUT_CPRI_WIDTH 		3


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define MAX_FLOW_ARP_IDX_WIDTH			(MAX_ARP_SW_TABLE_SIZE_SHIFT+1)
#define MAX_FLOW_IDLESEC_WIDTH			12
#define MAX_FLOW_NAPT_IDX_WIDTH			(MAX_NAPT_OUT_SW_TABLE_SHIFT+1)
#define MAX_FLOW_LUT_IDX_WIDTH			(LUT_HW_TABLE_SIZE_SHIFT+1) //+1 for bcam
#define MAX_FLOW_SPA_WIDTH				5
#define MAX_FLOW_WLAN_DEVICE_IDX_WIDTH	6	//max value of MAX_WLAN_DEVICE_NUM is 34, it need 6+1 bits, but we used at most 32.
#define MAX_FLOW_SW_NAPT_FILTER_IDX_WIDTH	(MAX_NAPT_FILER_SW_ENTRY_SIZE_SHIFT+1)
#define MAX_FLOW_SW_SHAREMETER_IDX_WIDTH	PURE_SW_SHAREMETER_TABLE_SIZE_SHIFT
#if defined(CONFIG_RG_G3_SERIES)
#define MAX_FLOW_HW_MAIN_HASH_WIDTH		16	// 65536
#define MAX_FLOW_HW_MAIN_HASH_SIZE		(1<<MAX_FLOW_HW_MAIN_HASH_WIDTH)

#endif
#endif



#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 0
#elif defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 8
#elif defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_IPV6_ROUTING_HW_TABLE_SIZE 4
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
#define MAX_IPV6_ROUTING_SW_TABLE_SIZE 16
#else
#define MAX_IPV6_ROUTING_SW_TABLE_SIZE 8
#endif
#else

#define MAX_IPV6_ROUTING_SW_TABLE_SIZE (MAX_IPV6_ROUTING_HW_TABLE_SIZE<<1)

#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE 0
#define MAX_IPV6_NEIGHBOR_HASH_WAY_SHIFT	5
#else
#define MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE 128
#define MAX_IPV6_NEIGHBOR_HASH_WAY_SHIFT	3
#endif
#define MAX_IPV6_NEIGHBOR_HASH_WAY_SIZE		0x1<<MAX_IPV6_NEIGHBOR_HASH_WAY_SHIFT
#define MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE 	MAX_IPV6_NEIGHBOR_HASH_WAY_SIZE<<4		//hash algorithm choose 0~15, (1<<IPV6_NEIGHBOR_HASH_WAY_SHIFT)-way for single index

#define MAX_WLAN_MBSSID_SW_TABLE_SIZE 64

/* Software tables */
#define MAX_VIRTUAL_SERVER_SW_TABLE_SIZE	32
#define MAX_UPNP_SW_TABLE_SIZE			128
#define MAX_DMZ_TABLE_SIZE					MAX_NETIF_SW_TABLE_SIZE
#define MAX_GATEWAYSERVICEPORT_TABLE_SIZE	32

#define IPV4_FRAGMENT_OUT_HASH_MASK_SHIFT	(0x1<<9)
#define IPV4_FRAGMENT_OUT_HASH_MASK			(IPV4_FRAGMENT_OUT_HASH_MASK_SHIFT-1)
#define IPV4_FRAGMENT_IN_HASH_MASK_SHIFT	(0x1<<8)
#define IPV4_FRAGMENT_IN_HASH_MASK			(IPV4_FRAGMENT_IN_HASH_MASK_SHIFT-1)
#define MAX_FRAG_IN_FREE_TABLE_SIZE			128
#define MAX_FRAG_OUT_FREE_TABLE_SIZE		128
#define MAX_IPV4_FRAGMENT_QUEUE_SIZE 		128		//TCP+UDP+ICMP

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define MAX_L2L3_FRAGMENT_HASH_BUCKET_SHIFT 8
#define MAX_L2L3_FRAGMENT_HASH_BUCKET 		(1<<MAX_L2L3_FRAGMENT_HASH_BUCKET_SHIFT)
#define MAX_L2L3_FRAGMENT_HASH_BUCKET_MASK 	(MAX_L2L3_FRAGMENT_HASH_BUCKET-1)
#define MAX_L2L3_FRAGMENT_TABLE_SIZE 		256
#define MAX_L2L3_FRAGMENT_QUEUE_SIZE 		256
#endif

#define MAX_NAPT_SHORTCUT_SIZE_SHIFT		(MAX_NAPT_OUT_SW_TABLE_SHIFT+1)
#define MAX_NAPT_SHORTCUT_SIZE 				(1<<MAX_NAPT_SHORTCUT_SIZE_SHIFT) 	//MAX_NAPT_OUT_SW_TABLE_SIZE*2
#define MAX_NAPT_SHORTCUT_WAYS_SHIFT 		2  		//must power of 2
#define MAX_NAPT_SHORTCUT_WAYS 				(1<<MAX_NAPT_SHORTCUT_WAYS_SHIFT) // 2==>4
#define FWD_SHORTCUT_BIT_NUM				(MAX_NAPT_SHORTCUT_SIZE_SHIFT-MAX_NAPT_SHORTCUT_WAYS_SHIFT)		//bit num of MAX_NAPT_SHORTCUT_SIZE

#define MAX_NAPT_V6_SHORTCUT_SIZE_SHIFT		12 //4096
#define MAX_NAPT_V6_SHORTCUT_SIZE			(1<<MAX_NAPT_V6_SHORTCUT_SIZE_SHIFT) //4096
#define MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT 	3  		//must power of 2
#define MAX_NAPT_V6_SHORTCUT_WAYS 			(1<<MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT) // 3==>8
#define FWD_V6_SHORTCUT_BIT_NUM				(MAX_NAPT_V6_SHORTCUT_SIZE_SHIFT-MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT)		//bit num of MAX_NAPT_V6_SHORTCUT_SIZE

#define MAX_IGMP_BLACK_LIST_SIZE			128
#define MAX_IGMP_WHITE_LIST_SIZE			128
#define MAX_ICMPCTRLFLOW_SIZE				32
#define MAX_ARP_SW_TABLE_HEAD				256
#if defined(CONFIG_RG_RTL9602C_SERIES)
#define MAX_ARP_HW_TABLE_HEAD				256
#endif
#define ARP_SW_TABLE_THRESHOLD				180
#if defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_ARP_FOR_LAN_INTF				256		//preserve 256 entries in hw table for LAN intfs
#define MAX_ARP_FOR_WAN_INTF				256 	//preserve 256 entries in hw table for WAN intfs
#elif defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_ARP_FOR_LAN_INTF				128		//preserve 128 entries in hw table for LAN intfs (shared with LAN & WAN)
#define MAX_ARP_FOR_WAN_INTF				128 	//preserve 128 entries in hw table for WAN intfs (shared with LAN & WAN)
#endif

#define MAX_FRAGMENT_QUEUE_THRESHOLD		50		//if there are so many packets need to queue, the first packet may not come in anymore, so kick they all
#define DEF_SOFTWARE_LEARNING_LIMIT			-1		//default software lut learning limit
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
#define v6StatefulHOUSE_KEEP_NUM			512		//how many entries should be check at once, MUST be the common divisor of list number
#define v6StatefulHOUSE_THRESHOLD			1024	//when the active connection is over this threshold, the timeout check function will be STATIC period(at most v6StatefulHOUSE_KEEP_NUM each time)
#define MAX_IPV6_STATEFUL_TABLE_SIZE		2048
//#define MAX_IPV6_STATEFUL_TABLE_SIZE		40		//test mode for LRU
#define MAX_IPV6_STATEFUL_HASH_HEAD_SIZE	256
#define MAX_IPV6_FRAGMENT_QUEUE_SIZE 		128
#endif
#define MAX_CONGESTION_CTRL_RING_SIZE		1200
#define MAX_CONGESTION_CTRL_PORTS			(RTK_RG_PORT_PON+1)
#define URL_ENTRY_DEFAULT_PRI 				4
#define MAX_URL_HiPri_ENTRY_SIZE 			128
#define MAX_URL_FILTER_ENTRY_SIZE 			512
#define MAX_URL_FILTER_BUF_LENGTH 			1024
#define MAX_URL_FILTER_STR_LENGTH			128
#define MAX_URL_FILTER_PATH_LENGTH			256

#define URL_FILTER_MAC_HASH_SHIFT_BIT		8
#define URL_FILTER_MAC_HASH_SIZE			(1<<URL_FILTER_MAC_HASH_SHIFT_BIT)

#define MAX_MAC_FILTER_ENTRY_SIZE 			30
#define MAX_IPMC_IP_LIST 					8
#define MAX_FORCE_PORTAL_URL_NUM			8
#define MAX_REDIRECT_PUSH_WEB_SIZE			768
#define MAX_REDIRECT_URL_NUM				8
#define MAX_REDIRECT_WHITE_LIST_NUM			8
#define MAX_AVOID_PORTAL_URL_NUM			8
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
#define MAX_VMAC_SKB_QUEUE_SIZE				32
#endif
#define MAX_LANNET_PARSE_COUNT				3
#define MAX_LANNET_DEV_NAME_LENGTH			32
#define MAX_LANNET_BRAND_NAME_LENGTH		16
#define MAX_LANNET_MODEL_NAME_LENGTH		16
#define MAX_LANNET_OS_NAME_LENGTH			16
#define MAX_LANNET_SUB_BRAND_SIZE			10
#define MAX_LANNET_SUB_MODEL_SIZE			10
#define MAX_LANNET_SUB_OS_SIZE				10


#ifdef TIMER_AGG
#define MAX_RG_TIMER_SIZE					16
#endif

#if defined(CONFIG_RG_RTL9600_SERIES)
#define MAX_WIFI_LIMIT_RATE					0xffff8	//0x1ffff << 3
#elif defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#define MAX_WIFI_LIMIT_RATE					0x3ffff8	//0x7ffff << 3
#endif

#if defined(CONFIG_RG_G3_SERIES)
#define MAX_SWITCH_METER_RATE 32768523 //the same with rtk switch setting (Note. If rate >= 32767499, disable l3 policer and rate will be 32767499)
#else
#define MAX_SWITCH_METER_RATE (0x7FFFF << 3) //the same with rtk switch setting
#endif

//nptv6 acceleration mechanism
#define MAX_NPTV6_ACC_UPSTREAM_BIT_RATE 4194296
#define MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE (480280*(102+20)*8/1000) // 468753
#define MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE (253600*(102+20)*8/1000) //247513
#define MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE (107000*(102+20)*8/1000) //104432
#define MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE (670100*(102+20)*8/1000) // 654017

#define MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM 4
#define MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM 4
#define MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM 2
#define MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM 2
#define MAX_NPTV6_ACC_UPSTREAM_SIZE 32
#define MAX_NPTV6_ACC_DOWNSTREAM_SIZE 32
#define MAX_NPTV6_ACC_UPSTREAM0_SIZE 16
#define MAX_NPTV6_ACC_DOWNSTREAM0_SIZE 16

#define RG_NPTV6_ACC_MAC_CPU_PORTMASK RTK_RG_ALL_MAC_CPU_PORTMASK
#define RG_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK (0x1<<RTK_RG_MAC_EXT_PORT1)
#define RG_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK (0x1<<RTK_RG_MAC_EXT_PORT2)
#define RG_NPTV6_ACC_MAC_CPU_EXT_PORTMASK (RG_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK | RG_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK)
//vxlan acceleration mechanism
#define RG_VXLAN_ACC_CPU_PORTMASK RTK_RG_ALL_MAC_CPU_PORTMASK
#define RG_VXLAN_ACC_UPSTREAM_EXTPMAK (0x1<<RTK_RG_MAC_EXT_PORT3)
#define RG_VXLAN_ACC_DOWNSTREAM_EXTPMAK (0x1<<RTK_RG_MAC_EXT_PORT4)
#define RG_VXLAN_ACC_CPU_EXT_PORTMASK (RG_VXLAN_ACC_UPSTREAM_EXTPMAK|RG_VXLAN_ACC_DOWNSTREAM_EXTPMAK)
#define RG_VXLAN_ACC_EPHEMERAL_SPORT	49152
#define RG_VXLAN_ACC_EXTRAPMSK			0xe
#define RG_VXLAN_ACC_EXTRAPMSK_RATE		217784
#define RG_VXLAN_ACC_EXTRAPMSK_METER	24

//for Timer definitions
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#define TC_BASE					0xB8003200
#define APOLLOPRO_TC6DATA		(TC_BASE + 0x70)
#define APOLLOPRO_TC6CNT		(TC_BASE + 0x74)
#define APOLLOPRO_TC6CTL		(TC_BASE + 0x78)
#define APOLLOPRO_TC6INT		(TC_BASE + 0x7C)
#define APOLLOPRO_TCEN			(1 << 28)
#define APOLLOPRO_TCMODE_TIMER	(1 << 24)
#define APOLLOPRO_TCIE			(1 << 20)
#define APOLLOPRO_TCIP			(1 << 16)
#define RG_TC_DF				1000
#define GIC_IRQ_BASE			16
#define MIPS_GIC_IRQ_BASE		GIC_IRQ_BASE
#define GIC_EXT_TC6         	6
#define BSP_TC6_IRQ         	( MIPS_GIC_IRQ_BASE +  GIC_EXT_TC6           )
#else
#define BSP_TC_BASE			0xB8003200
#define BSP_TC2DATA			(BSP_TC_BASE + 0x20)
#define BSP_TC2CNT			(BSP_TC_BASE + 0x24)
#define BSP_TC2CTL			(BSP_TC_BASE + 0x28)
#define BSP_TC2INT			(BSP_TC_BASE + 0x2C)
#define BSP_TCEN			(1 << 28)
#define BSP_TCMODE_TIMER	(1 << 24)
#define BSP_TCIE			(1 << 20)
#define BSP_TCIP			(1 << 16)
#define RG_TC_DF			200
#define BSP_TC2_IRQ			45
#endif

/* IGMP/MLD snooping */
#define RTK_RG_MULTICAST_MODE_MACFID 0


/* PROC GROUP */
#define RTK_RG_MAX_PROC_GROUP 4

#define RTK_RG_PROC_CHECK_SYN_IMPACT(ingressMacPort, packetLength) \
	((rg_db.systemGlobal.trapSpecificLengthSyn.portMask & (0x1<<ingressMacPort)) && (rg_db.systemGlobal.trapSpecificLengthSyn.priority != -1) && \
		((rg_db.systemGlobal.trapSpecificLengthSyn.pktLenEnd == 0) || \
			((rg_db.systemGlobal.trapSpecificLengthSyn.pktLenStart <= packetLength) && (rg_db.systemGlobal.trapSpecificLengthSyn.pktLenEnd >= packetLength))) )

#define RTK_RG_PROC_CHECK_ACK_IMPACT_FLOW(ingressMacPort) \
	((rg_db.systemGlobal.trapSpecificLengthAck.portMask & (0x1<<ingressMacPort)) && (rg_db.systemGlobal.trapSpecificLengthAck.priority != -1) && (rg_db.systemGlobal.trapSpecificLengthAck.isTrap == 0))

#define RTK_RG_PROC_CHECK_ACK_IMPACT(ingressMacPort, packetLength) \
	((rg_db.systemGlobal.trapSpecificLengthAck.portMask & (0x1<<ingressMacPort)) && (rg_db.systemGlobal.trapSpecificLengthAck.priority != -1) && \
		((rg_db.systemGlobal.trapSpecificLengthAck.pktLenEnd == 0 ) || \
			((rg_db.systemGlobal.trapSpecificLengthAck.pktLenStart <= packetLength) && (rg_db.systemGlobal.trapSpecificLengthAck.pktLenEnd >= packetLength))) )

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
//for flow_flush_disable_by_feature
#define RTK_RG_FFD_BIT_ACL				0x2
#define RTK_RG_FFD_BIT_NAPT_FILTER		0x4
#else
//for shortcut_flush_disable_by_feature
#define RTK_RG_FSD_BIT_ACL				0x2
#define RTK_RG_FSD_BIT_NAPT_FILTER		0x4
#endif

#define RTK_RG_MAX_DOS_RATE_LIMIT_TABLE_SIZE		MAX_NETIF_SW_TABLE_SIZE
#define RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(index) 	for(index = 0; index < RTK_RG_MAX_DOS_RATE_LIMIT_TABLE_SIZE; index++)  if((rg_db.systemGlobal.dosRateLimit[index].shareMeterIdx>=0) && (rg_db.systemGlobal.dosRateLimit[index].shareMeterIdx<MAX_SHAREMETER_TABLE_SIZE))

#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
//802.1x Access Control Protocol
#define RTK_RG_MAX_DOT1X_MAC_ENTRY_SIZE		CONFIG_RG_8021X_MAC_TABLE_SIZE
#endif

/* USERSPACE HELPER */
#define CB_MAX_FILE_SIZE 1024
#define CB_MAX_FILE_LINE 10
#define CB_CMD_BUFF_SIZE 256

/* common */
#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

#define IDX_UNHIT_DROP	-1
#define IDX_UNHIT_PASS	0

extern char rg_mt_watch_tmp[512];


void assert_ok_sub_func(int assert_ret, char *assert_str,const char *func,int line);
void assert_sub_func(int assert_ret, char *assert_str,const char *func,int line);

#if 0
#ifndef assert_ok
#define assert_ok(x)\
{ int assert_ret;\
	assert_ret=(x);\
	if(assert_ret!=0) { \
		if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) {\
        	rtlglue_printf(assert_ok_str,#x,assert_ret,__FUNCTION__,__LINE__);\
		}\
	}\
}
#endif


#ifndef assert
#define assert(x)\
if(!(x)) {\
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) {\
		rtlglue_printf(assert_str, #x,__FUNCTION__,__LINE__);\
	}\
}
#endif

#else
#ifndef assert_ok
#define assert_ok(x) do { assert_ok_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#ifndef assert
#define assert(x) do { assert_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#endif


/* debug */

#if defined(CONFIG_RG_DEBUG)


#ifndef DUMP_PACKET
#define DUMP_PACKET(pkt,size,memo) dump_packet(pkt,size,memo)
#endif


#ifndef DEBUG
#define DEBUG( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_DEBUG) common_dump(RTK_RG_DEBUG_LEVEL_DEBUG,__FUNCTION__,__LINE__,comment,##arg); }while(0)
#endif

#ifndef FIXME
#define FIXME( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_FIXME) common_dump(RTK_RG_DEBUG_LEVEL_FIXME,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef CBACK
#define CBACK( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_CALLBACK) common_dump(RTK_RG_DEBUG_LEVEL_CALLBACK,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TRACE
#define TRACE( comment ,arg...)	do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE) common_dump(RTK_RG_DEBUG_LEVEL_TRACE,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL
#define ACL( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ACL) common_dump(RTK_RG_DEBUG_LEVEL_ACL,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL_RSV
#define ACL_RSV( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ACL_RRESERVED) common_dump(RTK_RG_DEBUG_LEVEL_ACL_RRESERVED,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ACL_CTRL
#define ACL_CTRL( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ACL_CONTROL_PATH) common_dump(RTK_RG_DEBUG_LEVEL_ACL_CONTROL_PATH,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef WARNING
#define WARNING( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WARN) common_dump(RTK_RG_DEBUG_LEVEL_WARN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef WMUX
#define WMUX( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WMUX) common_dump(RTK_RG_DEBUG_LEVEL_WMUX,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef MACLN
#define MACLN( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_MACLEARN) common_dump(RTK_RG_DEBUG_LEVEL_MACLEARN,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TABLE
#define TABLE( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TABLE) common_dump(RTK_RG_DEBUG_LEVEL_TABLE,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TABLE_FULL
#define TABLE_FULL( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TABLE_FULL) common_dump(RTK_RG_DEBUG_LEVEL_TABLE_FULL,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef WIFI_FAST_FORWARD
#define WIFI_FAST_FORWARD( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WIFI_FAST_FORWARD) common_dump(RTK_RG_DEBUG_LEVEL_WIFI_FAST_FORWARD,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef ALG
#define ALG( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_ALG) common_dump(RTK_RG_DEBUG_LEVEL_ALG,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef IGMP
#define IGMP( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_IGMP) common_dump(RTK_RG_DEBUG_LEVEL_IGMP,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef RGAPI
#define RGAPI( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_RG_API) common_dump(RTK_RG_DEBUG_LEVEL_RG_API,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef TIMER
#define TIMER( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TIMER) common_dump(RTK_RG_DEBUG_LEVEL_TIMER,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif

#ifndef HTTP
#define HTTP( comment ,arg...) do { if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_HTTP) common_dump(RTK_RG_DEBUG_LEVEL_HTTP,__FUNCTION__,__LINE__,comment,##arg);}while(0)
#endif



#else //!defined(CONFIG_RG_DEBUG)

#ifndef assert
#define assert(x)
#endif


#ifndef COMMON_DUMP
#define COMMON_DUMP( bitmask, string, color,bgcolor,comment ,arg...)
#endif

#ifndef DEBUG
#define DEBUG( comment ,arg...)
#endif

#ifndef FIXME
#define FIXME( comment ,arg...)
#endif

#ifndef CBACK
#define CBACK( comment ,arg...)
#endif

#ifndef TRACE
#define TRACE( comment ,arg...)
#endif

#ifndef ACL
#define ACL( comment ,arg...)
#endif

#ifndef ACL_RSV
#define ACL_RSV( comment ,arg...)
#endif

#ifndef ACL_CTRL
#define ACL_CTRL( comment ,arg...)
#endif

#ifndef WARNING
#define WARNING( comment ,arg...)
#endif

#ifndef WMUX
#define WMUX( comment ,arg...)
#endif

#ifndef MACLN
#define MACLN( comment ,arg...)
#endif

#ifndef TABLE
#define TABLE( comment ,arg...)
#endif

#ifndef TABLE_FULL
#define TABLE_FULL( comment ,arg...)
#endif

#ifndef WIFI_FAST_FORWARD
#define WIFI_FAST_FORWARD( comment ,arg...)
#endif

#ifndef ALG
#define ALG( comment ,arg...)
#endif

#ifndef IGMP
#define IGMP( comment ,arg...)
#endif

#ifndef RGAPI
#define RGAPI( comment ,arg...)
#endif

#ifndef TIMER
#define TIMER( comment ,arg...)
#endif

#ifndef HTTP
#define HTTP( comment ,arg...)
#endif


#ifndef DUMP_PACKET
#define DUMP_PACKET(pkt,size,memo)
#endif


#endif

#if 0
#define RETURN_ERR(err_name) \
	do{\
		if(err_name!=RT_ERR_RG_OK) \
		{\
			extern char StringErrName[];\
			if(memcmp(#err_name,"RTK_ERR_RG_",11)==0)\
				strcpy(StringErrName,#err_name);\
			else\
			{\
				_rtk_rg_error_id_mapping(err_name,StringErrName);\
			}\
			WARNING("Return Error (0x%x:%s)",err_name,StringErrName);\
		}\
		return err_name;\
	}while(0)
#else

#define RETURN_ERR(err_name)  do { return return_err_mapping(err_name,#err_name,__LINE__); } while(0)
#define RETURN_WITH_LOG(ret_value,log_level,comment,arg...) \
	do{\
		if(rg_kernel.debug_level&log_level)\
			common_dump(log_level,__FUNCTION__,__LINE__,comment,##arg);\
		return (ret_value);\
	}while(0)
#endif

#define ACL_PTOOL_VERIFY_START PROFILE_FUNC_START
#define ACL_PTOOL_VERIFY_END PROFILE_FUNC_END

#if defined(CONFIG_RG_RTL9607C_SERIES) && defined(CONFIG_RTK_PTOOL) && defined(__KERNEL__)
#include <linux/ptool.h>

#define PROFILE_FUNC_START PROFILE_START
#define PROFILE_FUNC_END PROFILE_END
#else

#undef PROFILE
#define PROFILE int profile_idx; do { profile_idx=profile(__FUNCTION__); }while(0)
#define PROFILE_END do { profile_end(profile_idx); }while(0)
#define PROFILE_FUNC_START do {} while(0)
#define PROFILE_FUNC_END do {} while(0)

#define PROFILE_COUNT_START \
	do{\
		if(rg_kernel.cp3_execute_count>0){\
			if(!rg_kernel.cp3_execute_count_state)\
				rg_kernel.cp3_execute_count_state=profile(__FUNCTION__);\
			rg_kernel.cp3_execute_count--;\
		}\
	}while(0)
#define PROFILE_COUNT_END \
	do{\
		if(rg_kernel.cp3_execute_count_state && rg_kernel.cp3_execute_count==0){\
			profile_end(rg_kernel.cp3_execute_count_state);\
			rg_kernel.cp3_execute_count_state=0;\
		}\
	}while(0)
#endif

// L2 Switch: RTK APIs --> RG APIs
//from stp.h
#include<rtk/stp.h>
#define rtk_rg_stp_init(arg,args...) rtk_stp_init(arg,##args)
#define rtk_rg_stp_mstpState_get(arg,args...) rtk_stp_mstpState_get(arg,##args)
#define rtk_rg_stp_mstpState_set(arg,args...) rtk_stp_mstpState_set(arg,##args)


//from module/gpon/gpon.h
#include<module/gpon/gpon.h>
#define rtk_rg_gpon_activate(arg,args...) rtk_gpon_activate(arg,##args)
#define rtk_rg_gpon_aesKeySwitch_get(arg,args...) rtk_gpon_aesKeySwitch_get(arg,##args)
#define rtk_rg_gpon_aesKeySwitch_set(arg,args...) rtk_gpon_aesKeySwitch_set(arg,##args)
#define rtk_rg_gpon_aesKeyWord_set(arg,args...) rtk_gpon_aesKeyWord_set(arg,##args)
#define rtk_rg_gpon_alarmStatus_get(arg,args...) rtk_gpon_alarmStatus_get(arg,##args)
#define rtk_rg_gpon_autoDisTx_set(arg,args...) rtk_gpon_autoDisTx_set(arg,##args)
#define rtk_rg_gpon_berInterval_get(arg,args...) rtk_gpon_berInterval_get(arg,##args)
#define rtk_rg_gpon_burstOverhead_set(arg,args...) rtk_gpon_burstOverhead_set(arg,##args)
#define rtk_rg_gpon_cdrLosStatus_get(arg,args...) rtk_gpon_cdrLosStatus_get(arg,##args)
#define rtk_rg_gpon_dataPath_reset(arg,args...) rtk_gpon_dataPath_reset(arg,##args)
#define rtk_rg_gpon_dbruPeriod_get(arg,args...) rtk_gpon_dbruPeriod_get(arg,##args)
#define rtk_rg_gpon_dbruPeriod_set(arg,args...) rtk_gpon_dbruPeriod_set(arg,##args)
#define rtk_rg_gpon_dbruState_get(arg,args...) rtk_gpon_dbruState_get(arg,##args)
#define rtk_rg_gpon_dbruState_set(arg,args...) rtk_gpon_dbruState_set(arg,##args)
#define rtk_rg_gpon_deActivate(arg,args...) rtk_gpon_deActivate(arg,##args)
#define rtk_rg_gpon_drainOutDefaultQueue_set(arg,args...) rtk_gpon_drainOutDefaultQueue_set(arg,##args)
#define rtk_rg_gpon_dsBwmapCrcCheckState_get(arg,args...) rtk_gpon_dsBwmapCrcCheckState_get(arg,##args)
#define rtk_rg_gpon_dsBwmapCrcCheckState_set(arg,args...) rtk_gpon_dsBwmapCrcCheckState_set(arg,##args)
#define rtk_rg_gpon_dsBwmapFilterOnuIdState_get(arg,args...) rtk_gpon_dsBwmapFilterOnuIdState_get(arg,##args)
#define rtk_rg_gpon_dsBwmapFilterOnuIdState_set(arg,args...) rtk_gpon_dsBwmapFilterOnuIdState_set(arg,##args)
#define rtk_rg_gpon_dsEthPti_get(arg,args...) rtk_gpon_dsEthPti_get(arg,##args)
#define rtk_rg_gpon_dsEthPti_set(arg,args...) rtk_gpon_dsEthPti_set(arg,##args)
#define rtk_rg_gpon_dsFecBypass_get(arg,args...) rtk_gpon_dsFecBypass_get(arg,##args)
#define rtk_rg_gpon_dsFecBypass_set(arg,args...) rtk_gpon_dsFecBypass_set(arg,##args)
#define rtk_rg_gpon_dsFecThrd_get(arg,args...) rtk_gpon_dsFecThrd_get(arg,##args)
#define rtk_rg_gpon_dsFecThrd_set(arg,args...) rtk_gpon_dsFecThrd_set(arg,##args)
#define rtk_rg_gpon_usFecSts_get(arg,args...) rtk_gpon_usFecSts_get(arg,##args)
#define rtk_rg_gpon_dsFecSts_get(arg,args...) rtk_gpon_dsFecSts_get(arg,##args)
#define rtk_rg_gpon_dsFlow_get(arg,args...) rtk_gpon_dsFlow_get(arg,##args)
#define rtk_rg_gpon_dsFlow_set(arg,args...) rtk_gpon_dsFlow_set(arg,##args)
#define rtk_rg_gpon_dsGemPortBcPassState_get(arg,args...) rtk_gpon_dsGemPortBcPassState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortBcPassState_set(arg,args...) rtk_gpon_dsGemPortBcPassState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortByteCnt_get(arg,args...) rtk_gpon_dsGemPortByteCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortEthFwdCnt_get(arg,args...) rtk_gpon_dsGemPortEthFwdCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortEthRxCnt_get(arg,args...) rtk_gpon_dsGemPortEthRxCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFcsCheckState_get(arg,args...) rtk_gpon_dsGemPortFcsCheckState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFcsCheckState_set(arg,args...) rtk_gpon_dsGemPortFcsCheckState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortFrameTimeOut_get(arg,args...) rtk_gpon_dsGemPortFrameTimeOut_get(arg,##args)
#define rtk_rg_gpon_dsGemPortFrameTimeOut_set(arg,args...) rtk_gpon_dsGemPortFrameTimeOut_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_del(arg,args...) rtk_gpon_dsGemPortMacEntry_del(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_get(arg,args...) rtk_gpon_dsGemPortMacEntry_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacEntry_set(arg,args...) rtk_gpon_dsGemPortMacEntry_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacFilterMode_get(arg,args...) rtk_gpon_dsGemPortMacFilterMode_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacFilterMode_set(arg,args...) rtk_gpon_dsGemPortMacFilterMode_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMacForceMode_get(arg,args...) rtk_gpon_dsGemPortMacForceMode_get(arg,##args)
#define rtk_rg_gpon_dsGemPortMacForceMode_set(arg,args...) rtk_gpon_dsGemPortMacForceMode_set(arg,##args)
#define rtk_rg_gpon_dsGemPortMiscCnt_get(arg,args...) rtk_gpon_dsGemPortMiscCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPortNonMcPassState_get(arg,args...) rtk_gpon_dsGemPortNonMcPassState_get(arg,##args)
#define rtk_rg_gpon_dsGemPortNonMcPassState_set(arg,args...) rtk_gpon_dsGemPortNonMcPassState_set(arg,##args)
#define rtk_rg_gpon_dsGemPortPktCnt_get(arg,args...) rtk_gpon_dsGemPortPktCnt_get(arg,##args)
#define rtk_rg_gpon_dsGemPort_del(arg,args...) rtk_gpon_dsGemPort_del(arg,##args)
#define rtk_rg_gpon_dsGemPort_get(arg,args...) rtk_gpon_dsGemPort_get(arg,##args)
#define rtk_rg_gpon_dsGemPort_set(arg,args...) rtk_gpon_dsGemPort_set(arg,##args)
#define rtk_rg_gpon_dsGtcMiscCnt_get(arg,args...) rtk_gpon_dsGtcMiscCnt_get(arg,##args)
#define rtk_rg_gpon_dsOmciCnt_get(arg,args...) rtk_gpon_dsOmciCnt_get(arg,##args)
#define rtk_rg_gpon_dsOmciPti_get(arg,args...) rtk_gpon_dsOmciPti_get(arg,##args)
#define rtk_rg_gpon_dsOmciPti_set(arg,args...) rtk_gpon_dsOmciPti_set(arg,##args)
#define rtk_rg_gpon_dsPlendStrictMode_get(arg,args...) rtk_gpon_dsPlendStrictMode_get(arg,##args)
#define rtk_rg_gpon_dsPlendStrictMode_set(arg,args...) rtk_gpon_dsPlendStrictMode_set(arg,##args)
#define rtk_rg_gpon_dsPloamBcAcceptState_get(arg,args...) rtk_gpon_dsPloamBcAcceptState_get(arg,##args)
#define rtk_rg_gpon_dsPloamBcAcceptState_set(arg,args...) rtk_gpon_dsPloamBcAcceptState_set(arg,##args)
#define rtk_rg_gpon_dsPloamDropCrcState_get(arg,args...) rtk_gpon_dsPloamDropCrcState_get(arg,##args)
#define rtk_rg_gpon_dsPloamDropCrcState_set(arg,args...) rtk_gpon_dsPloamDropCrcState_set(arg,##args)
#define rtk_rg_gpon_dsPloamNomsg_get(arg,args...) rtk_gpon_dsPloamNomsg_get(arg,##args)
#define rtk_rg_gpon_dsPloamNomsg_set(arg,args...) rtk_gpon_dsPloamNomsg_set(arg,##args)
#define rtk_rg_gpon_dsPloamOnuIdFilterState_get(arg,args...) rtk_gpon_dsPloamOnuIdFilterState_get(arg,##args)
#define rtk_rg_gpon_dsPloamOnuIdFilterState_set(arg,args...) rtk_gpon_dsPloamOnuIdFilterState_set(arg,##args)
#define rtk_rg_gpon_dsPloam_get(arg,args...) rtk_gpon_dsPloam_get(arg,##args)
#define rtk_rg_gpon_dsScrambleState_get(arg,args...) rtk_gpon_dsScrambleState_get(arg,##args)
#define rtk_rg_gpon_dsScrambleState_set(arg,args...) rtk_gpon_dsScrambleState_set(arg,##args)
#define rtk_rg_gpon_eqd_set(arg,args...) rtk_gpon_eqd_set(arg,##args)
#define rtk_rg_gpon_extraSnTxTimes_get(arg,args...) rtk_gpon_extraSnTxTimes_get(arg,##args)
#define rtk_rg_gpon_extraSnTxTimes_set(arg,args...) rtk_gpon_extraSnTxTimes_set(arg,##args)
#define rtk_rg_gpon_flowCounter_get(arg,args...) rtk_gpon_flowCounter_get(arg,##args)
#define rtk_rg_gpon_forceLaser_get(arg,args...) rtk_gpon_forceLaser_get(arg,##args)
#define rtk_rg_gpon_forceLaser_set(arg,args...) rtk_gpon_forceLaser_set(arg,##args)
#define rtk_rg_gpon_gemUsDataByteCnt_get(arg,args...) rtk_gpon_gemUsDataByteCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsEthCnt_get(arg,args...) rtk_gpon_gemUsEthCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsForceIdleState_get(arg,args...) rtk_gpon_gemUsForceIdleState_get(arg,##args)
#define rtk_rg_gpon_gemUsForceIdleState_set(arg,args...) rtk_gpon_gemUsForceIdleState_set(arg,##args)
#define rtk_rg_gpon_gemUsGemCnt_get(arg,args...) rtk_gpon_gemUsGemCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsIdleByteCnt_get(arg,args...) rtk_gpon_gemUsIdleByteCnt_get(arg,##args)
#define rtk_rg_gpon_gemUsIntrDlt_check(arg,args...) rtk_gpon_gemUsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gemUsIntrMask_get(arg,args...) rtk_gpon_gemUsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gemUsIntrMask_set(arg,args...) rtk_gpon_gemUsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gemUsIntr_get(arg,args...) rtk_gpon_gemUsIntr_get(arg,##args)
#define rtk_rg_gpon_gemUsPortCfg_get(arg,args...) rtk_gpon_gemUsPortCfg_get(arg,##args)
#define rtk_rg_gpon_gemUsPortCfg_set(arg,args...) rtk_gpon_gemUsPortCfg_set(arg,##args)
#define rtk_rg_gpon_gemUsPtiVector_get(arg,args...) rtk_gpon_gemUsPtiVector_get(arg,##args)
#define rtk_rg_gpon_gemUsPtiVector_set(arg,args...) rtk_gpon_gemUsPtiVector_set(arg,##args)
#define rtk_rg_gpon_globalCounter_get(arg,args...) rtk_gpon_globalCounter_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrDlt_check(arg,args...) rtk_gpon_gtcDsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gtcDsIntrDlt_get(arg,args...) rtk_gpon_gtcDsIntrDlt_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrMask_get(arg,args...) rtk_gpon_gtcDsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gtcDsIntrMask_set(arg,args...) rtk_gpon_gtcDsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gtcDsIntr_get(arg,args...) rtk_gpon_gtcDsIntr_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrDlt_check(arg,args...) rtk_gpon_gtcUsIntrDlt_check(arg,##args)
#define rtk_rg_gpon_gtcUsIntrDlt_get(arg,args...) rtk_gpon_gtcUsIntrDlt_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrMask_get(arg,args...) rtk_gpon_gtcUsIntrMask_get(arg,##args)
#define rtk_rg_gpon_gtcUsIntrMask_set(arg,args...) rtk_gpon_gtcUsIntrMask_set(arg,##args)
#define rtk_rg_gpon_gtcUsIntr_get(arg,args...) rtk_gpon_gtcUsIntr_get(arg,##args)
#define rtk_rg_gpon_indNrmPloamState_get(arg,args...) rtk_gpon_indNrmPloamState_get(arg,##args)
#define rtk_rg_gpon_indNrmPloamState_set(arg,args...) rtk_gpon_indNrmPloamState_set(arg,##args)
#define rtk_rg_gpon_init(arg,args...) rtk_gpon_init(arg,##args)
#define rtk_rg_gpon_ipv4McAddrPtn_get(arg,args...) rtk_gpon_ipv4McAddrPtn_get(arg,##args)
#define rtk_rg_gpon_ipv4McAddrPtn_set(arg,args...) rtk_gpon_ipv4McAddrPtn_set(arg,##args)
#define rtk_rg_gpon_ipv6McAddrPtn_get(arg,args...) rtk_gpon_ipv6McAddrPtn_get(arg,##args)
#define rtk_rg_gpon_ipv6McAddrPtn_set(arg,args...) rtk_gpon_ipv6McAddrPtn_set(arg,##args)
#define rtk_rg_gpon_irq_get(arg,args...) rtk_gpon_irq_get(arg,##args)
#define rtk_rg_gpon_laserTime_get(arg,args...) rtk_gpon_laserTime_get(arg,##args)
#define rtk_rg_gpon_laserTime_set(arg,args...) rtk_gpon_laserTime_set(arg,##args)
#define rtk_rg_gpon_losCfg_get(arg,args...) rtk_gpon_losCfg_get(arg,##args)
#define rtk_rg_gpon_losCfg_set(arg,args...) rtk_gpon_losCfg_set(arg,##args)
#define rtk_rg_gpon_macVersion_get(arg,args...) rtk_gpon_macVersion_get(arg,##args)
#define rtk_rg_gpon_onuId_get(arg,args...) rtk_gpon_onuId_get(arg,##args)
#define rtk_rg_gpon_onuId_set(arg,args...) rtk_gpon_onuId_set(arg,##args)
#define rtk_rg_gpon_onuState_get(arg,args...) rtk_gpon_onuState_get(arg,##args)
#define rtk_rg_gpon_onuState_set(arg,args...) rtk_gpon_onuState_set(arg,##args)
#define rtk_rg_gpon_optLosStatus_get(arg,args...) rtk_gpon_optLosStatus_get(arg,##args)
#define rtk_rg_gpon_password_get(arg,args...) rtk_gpon_password_get(arg,##args)
#define rtk_rg_gpon_password_set(arg,args...) rtk_gpon_password_set(arg,##args)
#define rtk_rg_gpon_ploamState_get(arg,args...) rtk_gpon_ploamState_get(arg,##args)
#define rtk_rg_gpon_ploamState_set(arg,args...) rtk_gpon_ploamState_set(arg,##args)
#define rtk_rg_gpon_ponStatus_get(arg,args...) rtk_gpon_ponStatus_get(arg,##args)
#define rtk_rg_gpon_portMacForceMode_set(arg,args...) rtk_gpon_portMacForceMode_set(arg,##args)
#define rtk_rg_gpon_port_get(arg,args...) rtk_gpon_port_get(arg,##args)
#define rtk_rg_gpon_rdi_get(arg,args...) rtk_gpon_rdi_get(arg,##args)
#define rtk_rg_gpon_rdi_set(arg,args...) rtk_gpon_rdi_set(arg,##args)
#define rtk_rg_gpon_resetDoneState_get(arg,args...) rtk_gpon_resetDoneState_get(arg,##args)
#define rtk_rg_gpon_resetState_set(arg,args...) rtk_gpon_resetState_set(arg,##args)
#define rtk_rg_gpon_rogueOnt_set(arg,args...) rtk_gpon_rogueOnt_set(arg,##args)
#define rtk_rg_gpon_scheInfo_get(arg,args...) rtk_gpon_scheInfo_get(arg,##args)
#define rtk_rg_gpon_serialNumber_get(arg,args...) rtk_gpon_serialNumber_get(arg,##args)
#define rtk_rg_gpon_serialNumber_set(arg,args...) rtk_gpon_serialNumber_set(arg,##args)
#define rtk_rg_gpon_tcontCounter_get(arg,args...) rtk_gpon_tcontCounter_get(arg,##args)
#define rtk_rg_gpon_tcont_create(arg,args...) rtk_gpon_tcont_create(arg,##args)
#define rtk_rg_gpon_tcont_destroy_logical(arg,args...) rtk_gpon_tcont_destroy_logical(arg,##args)
#define rtk_rg_gpon_tcont_destroy(arg,args...) rtk_gpon_tcont_destroy(arg,##args)
#define rtk_rg_gpon_tcont_get(arg,args...) rtk_gpon_tcont_get(arg,##args)
#define rtk_rg_gpon_tcont_get_physical(arg,args...) rtk_gpon_tcont_get_physical(arg,##args)
#define rtk_rg_gpon_test_get(arg,args...) rtk_gpon_test_get(arg,##args)
#define rtk_rg_gpon_test_set(arg,args...) rtk_gpon_test_set(arg,##args)
#define rtk_rg_gpon_topIntrMask_get(arg,args...) rtk_gpon_topIntrMask_get(arg,##args)
#define rtk_rg_gpon_topIntrMask_set(arg,args...) rtk_gpon_topIntrMask_set(arg,##args)
#define rtk_rg_gpon_topIntr_disableAll(arg,args...) rtk_gpon_topIntr_disableAll(arg,##args)
#define rtk_rg_gpon_topIntr_get(arg,args...) rtk_gpon_topIntr_get(arg,##args)
#define rtk_rg_gpon_usAutoPloam_set(arg,args...) rtk_gpon_usAutoPloam_set(arg,##args)
#define rtk_rg_gpon_usBurstPolarity_get(arg,args...) rtk_gpon_usBurstPolarity_get(arg,##args)
#define rtk_rg_gpon_usBurstPolarity_set(arg,args...) rtk_gpon_usBurstPolarity_set(arg,##args)
#define rtk_rg_gpon_usFlow_get(arg,args...) rtk_gpon_usFlow_get(arg,##args)
#define rtk_rg_gpon_usFlow_set(arg,args...) rtk_gpon_usFlow_set(arg,##args)
#define rtk_rg_gpon_usGtcMiscCnt_get(arg,args...) rtk_gpon_usGtcMiscCnt_get(arg,##args)
#define rtk_rg_gpon_usOmciCnt_get(arg,args...) rtk_gpon_usOmciCnt_get(arg,##args)
#define rtk_rg_gpon_usPloamBuf_flush(arg,args...) rtk_gpon_usPloamBuf_flush(arg,##args)
#define rtk_rg_gpon_usPloamCrcGenState_get(arg,args...) rtk_gpon_usPloamCrcGenState_get(arg,##args)
#define rtk_rg_gpon_usPloamCrcGenState_set(arg,args...) rtk_gpon_usPloamCrcGenState_set(arg,##args)
#define rtk_rg_gpon_usPloamOnuIdFilterState_set(arg,args...) rtk_gpon_usPloamOnuIdFilterState_set(arg,##args)
#define rtk_rg_gpon_usPloamOnuIdFilter_get(arg,args...) rtk_gpon_usPloamOnuIdFilter_get(arg,##args)
#define rtk_rg_gpon_usPloam_set(arg,args...) rtk_gpon_usPloam_set(arg,##args)
#define rtk_rg_gpon_usScrambleState_get(arg,args...) rtk_gpon_usScrambleState_get(arg,##args)
#define rtk_rg_gpon_usScrambleState_set(arg,args...) rtk_gpon_usScrambleState_set(arg,##args)
#define rtk_rg_gpon_usSmalSstartProcState_get(arg,args...) rtk_gpon_usSmalSstartProcState_get(arg,##args)
#define rtk_rg_gpon_usSmalSstartProcState_set(arg,args...) rtk_gpon_usSmalSstartProcState_set(arg,##args)
#define rtk_rg_gpon_usSuppressLaserState_get(arg,args...) rtk_gpon_usSuppressLaserState_get(arg,##args)
#define rtk_rg_gpon_usSuppressLaserState_set(arg,args...) rtk_gpon_usSuppressLaserState_set(arg,##args)
#define rtk_rg_gpon_usTcont_del(arg,args...) rtk_gpon_usTcont_del(arg,##args)
#define rtk_rg_gpon_usTcont_get(arg,args...) rtk_gpon_usTcont_get(arg,##args)
#define rtk_rg_gpon_usTcont_set(arg,args...) rtk_gpon_usTcont_set(arg,##args)
#define rtk_rg_gpon_evtHdlStateChange_reg(arg,args...) rtk_gponapp_evtHdlStateChange_reg(arg,##args)
#define rtk_rg_gpon_evtHdlPloam_reg(arg,args...) rtk_gponapp_evtHdlPloam_reg(arg,##args)
#define rtk_rg_gpon_evtHdlPloam_dreg(arg,args...) rtk_gponapp_evtHdlPloam_dreg(arg,##args)
#define rtk_rg_gpon_evtHdlOmci_reg(arg,args...) rtk_gponapp_evtHdlOmci_reg(arg,##args)
#define rtk_rg_gpon_callbackExtMsgSetHandle_reg(arg,args...) rtk_gponapp_callbackExtMsgSetHandle_reg(arg,##args)
#define rtk_rg_gpon_callbackExtMsgGetHandle_reg(arg,args...) rtk_gponapp_callbackExtMsgGetHandle_reg(arg,##args)
#define rtk_rg_gpon_parameter_set(arg,args...) rtk_gpon_parameter_set(arg,##args)
#define rtk_rg_gpon_parameter_get(arg,args...) rtk_gpon_parameter_get(arg,##args)
#define rtk_rg_gpon_omci_tx(arg,args...) rtk_gpon_omci_tx(arg,##args)
#define rtk_rg_gpon_initial(arg,args...) rtk_gpon_initial(arg,##args)
#define rtk_rg_gpon_deinitial(arg,args...) rtk_gpon_deinitial(arg,##args)
#define rtk_rg_gpon_dbruBlockSize_get(arg,args...) rtk_gpon_dbruBlockSize_get(arg,##args)
#define rtk_rg_gpon_dbruBlockSize_set(arg,args...) rtk_gpon_dbruBlockSize_set(arg,##args)


//from epon.h
#include<rtk/epon.h>
#define rtk_rg_epon_churningKey_get(arg,args...) rtk_epon_churningKey_get(arg,##args)
#define rtk_rg_epon_churningKey_set(arg,args...) rtk_epon_churningKey_set(arg,##args)
#define rtk_rg_epon_churningStatus_get(arg,args...) rtk_epon_churningStatus_get(arg,##args)
#define rtk_rg_epon_dsFecState_get(arg,args...) rtk_epon_dsFecState_get(arg,##args)
#define rtk_rg_epon_dsFecState_set(arg,args...) rtk_epon_dsFecState_set(arg,##args)
#define rtk_rg_epon_fecState_get(arg,args...) rtk_epon_fecState_get(arg,##args)
#define rtk_rg_epon_fecState_set(arg,args...) rtk_epon_fecState_set(arg,##args)
#define rtk_rg_epon_forceLaserState_get(arg,args...) rtk_epon_forceLaserState_get(arg,##args)
#define rtk_rg_epon_forceLaserState_set(arg,args...) rtk_epon_forceLaserState_set(arg,##args)
#define rtk_rg_epon_init(arg,args...) rtk_epon_init(arg,##args)
#define rtk_rg_epon_intrMask_get(arg,args...) rtk_epon_intrMask_get(arg,##args)
#define rtk_rg_epon_intrMask_set(arg,args...) rtk_epon_intrMask_set(arg,##args)
#define rtk_rg_epon_intr_disableAll(arg,args...) rtk_epon_intr_disableAll(arg,##args)
#define rtk_rg_epon_intr_get(arg,args...) rtk_epon_intr_get(arg,##args)
#define rtk_rg_epon_laserTime_get(arg,args...) rtk_epon_laserTime_get(arg,##args)
#define rtk_rg_epon_laserTime_set(arg,args...) rtk_epon_laserTime_set(arg,##args)
#define rtk_rg_epon_llidEntryNum_get(arg,args...) rtk_epon_llidEntryNum_get(arg,##args)
#define rtk_rg_epon_llid_entry_get(arg,args...) rtk_epon_llid_entry_get(arg,##args)
#define rtk_rg_epon_llid_entry_set(arg,args...) rtk_epon_llid_entry_set(arg,##args)
#define rtk_rg_epon_losState_get(arg,args...) rtk_epon_losState_get(arg,##args)
#define rtk_rg_epon_mibCounter_get(arg,args...) rtk_epon_mibCounter_get(arg,##args)
#define rtk_rg_epon_mibGlobal_reset(arg,args...) rtk_epon_mibGlobal_reset(arg,##args)
#define rtk_rg_epon_mibLlidIdx_reset(arg,args...) rtk_epon_mibLlidIdx_reset(arg,##args)
#define rtk_rg_epon_mpcpTimeoutVal_get(arg,args...) rtk_epon_mpcpTimeoutVal_get(arg,##args)
#define rtk_rg_epon_mpcpTimeoutVal_set(arg,args...) rtk_epon_mpcpTimeoutVal_set(arg,##args)
#define rtk_rg_epon_opticalPolarity_get(arg,args...) rtk_epon_opticalPolarity_get(arg,##args)
#define rtk_rg_epon_opticalPolarity_set(arg,args...) rtk_epon_opticalPolarity_set(arg,##args)
#define rtk_rg_epon_registerReq_get(arg,args...) rtk_epon_registerReq_get(arg,##args)
#define rtk_rg_epon_registerReq_set(arg,args...) rtk_epon_registerReq_set(arg,##args)
#define rtk_rg_epon_syncTime_get(arg,args...) rtk_epon_syncTime_get(arg,##args)
#define rtk_rg_epon_thresholdReport_get(arg,args...) rtk_epon_thresholdReport_get(arg,##args)
#define rtk_rg_epon_thresholdReport_set(arg,args...) rtk_epon_thresholdReport_set(arg,##args)
#define rtk_rg_epon_usFecState_get(arg,args...) rtk_epon_usFecState_get(arg,##args)
#define rtk_rg_epon_usFecState_set(arg,args...) rtk_epon_usFecState_set(arg,##args)


//from i2c.h
#include<rtk/i2c.h>
#define rtk_rg_i2c_clock_get(arg,args...) rtk_i2c_clock_get(arg,##args)
#define rtk_rg_i2c_clock_set(arg,args...) rtk_i2c_clock_set(arg,##args)
#define rtk_rg_i2c_eepMirror_get(arg,args...) rtk_i2c_eepMirror_get(arg,##args)
#define rtk_rg_i2c_eepMirror_read(arg,args...) rtk_i2c_eepMirror_read(arg,##args)
#define rtk_rg_i2c_eepMirror_set(arg,args...) rtk_i2c_eepMirror_set(arg,##args)
#define rtk_rg_i2c_eepMirror_write(arg,args...) rtk_i2c_eepMirror_write(arg,##args)
#define rtk_rg_i2c_enable_get(arg,args...) rtk_i2c_enable_get(arg,##args)
#define rtk_rg_i2c_enable_set(arg,args...) rtk_i2c_enable_set(arg,##args)
#define rtk_rg_i2c_init(arg,args...) rtk_i2c_init(arg,##args)
#define rtk_rg_i2c_read(arg,args...) rtk_i2c_read(arg,##args)
#define rtk_rg_i2c_width_get(arg,args...) rtk_i2c_width_get(arg,##args)
#define rtk_rg_i2c_width_set(arg,args...) rtk_i2c_width_set(arg,##args)
#define rtk_rg_i2c_write(arg,args...) rtk_i2c_write(arg,##args)

//from intr.h
#include<rtk/intr.h>
#define rtk_rg_intr_gphyStatus_clear(arg,args...) rtk_intr_gphyStatus_clear(arg,##args)
#define rtk_rg_intr_gphyStatus_get(arg,args...) rtk_intr_gphyStatus_get(arg,##args)
#define rtk_rg_intr_imr_get(arg,args...) rtk_intr_imr_get(arg,##args)
#define rtk_rg_intr_imr_restore(arg,args...) rtk_intr_imr_restore(arg,##args)
#define rtk_rg_intr_imr_set(arg,args...) rtk_intr_imr_set(arg,##args)
#define rtk_rg_intr_ims_clear(arg,args...) rtk_intr_ims_clear(arg,##args)
#define rtk_rg_intr_ims_get(arg,args...) rtk_intr_ims_get(arg,##args)
#define rtk_rg_intr_init(arg,args...) rtk_intr_init(arg,##args)
#define rtk_rg_intr_linkdownStatus_clear(arg,args...) rtk_intr_linkdownStatus_clear(arg,##args)
#define rtk_rg_intr_linkdownStatus_get(arg,args...) rtk_intr_linkdownStatus_get(arg,##args)
#define rtk_rg_intr_linkupStatus_clear(arg,args...) rtk_intr_linkupStatus_clear(arg,##args)
#define rtk_rg_intr_linkupStatus_get(arg,args...) rtk_intr_linkupStatus_get(arg,##args)
#define rtk_rg_intr_polarity_get(arg,args...) rtk_intr_polarity_get(arg,##args)
#define rtk_rg_intr_polarity_set(arg,args...) rtk_intr_polarity_set(arg,##args)
#define rtk_rg_intr_speedChangeStatus_clear(arg,args...) rtk_intr_speedChangeStatus_clear(arg,##args)
#define rtk_rg_intr_speedChangeStatus_get(arg,args...) rtk_intr_speedChangeStatus_get(arg,##args)


//from ponmac.h
#include<rtk/ponmac.h>
#define rtk_rg_ponmac_flow2Queue_get(arg,args...) rtk_ponmac_flow2Queue_get(arg,##args)
#define rtk_rg_ponmac_flow2Queue_set(arg,args...) rtk_ponmac_flow2Queue_set(arg,##args)
#define rtk_rg_ponmac_init(arg,args...) rtk_ponmac_init(arg,##args)
#define rtk_rg_ponmac_losState_get(arg,args...) rtk_ponmac_losState_get(arg,##args)
#define rtk_rg_ponmac_mode_get(arg,args...) rtk_ponmac_mode_get(arg,##args)
#define rtk_rg_ponmac_mode_set(arg,args...) rtk_ponmac_mode_set(arg,##args)
#define rtk_rg_ponmac_opticalPolarity_get(arg,args...) rtk_ponmac_opticalPolarity_get(arg,##args)
#define rtk_rg_ponmac_opticalPolarity_set(arg,args...) rtk_ponmac_opticalPolarity_set(arg,##args)
#define rtk_rg_ponmac_queueDrainOut_set(arg,args...) rtk_ponmac_queueDrainOut_set(arg,##args)
#define rtk_rg_ponmac_queue_add(arg,args...) rtk_ponmac_queue_add(arg,##args)
#define rtk_rg_ponmac_queue_del(arg,args...) rtk_ponmac_queue_del(arg,##args)
#define rtk_rg_ponmac_queue_get(arg,args...) rtk_ponmac_queue_get(arg,##args)
#define rtk_rg_ponmac_transceiver_get(arg,args...) rtk_ponmac_transceiver_get(arg,##args)
#define rtk_rg_ponmac_bwThreshold_set(arg,args...) rtk_ponmac_bwThreshold_set(arg,##args)
#define rtk_rg_ponmac_bwThreshold_get(arg,args...) rtk_ponmac_bwThreshold_get(arg,##args)

//from port.h
#include<rtk/port.h>
#define rtk_rg_port_adminEnable_get(arg,args...) rtk_port_adminEnable_get(arg,##args)
#define rtk_rg_port_adminEnable_set(arg,args...) rtk_port_adminEnable_set(arg,##args)
#define rtk_rg_port_cpuPortId_get(arg,args...) rtk_port_cpuPortId_get(arg,##args)
#define rtk_rg_port_enhancedFid_get(arg,args...) rtk_port_enhancedFid_get(arg,##args)
#define rtk_rg_port_enhancedFid_set(arg,args...) rtk_port_enhancedFid_set(arg,##args)
#define rtk_rg_port_flowctrl_get(arg,args...) rtk_port_flowctrl_get(arg,##args)
#define rtk_rg_port_gigaLiteEnable_get(arg,args...) rtk_port_gigaLiteEnable_get(arg,##args)
#define rtk_rg_port_gigaLiteEnable_set(arg,args...) rtk_port_gigaLiteEnable_set(arg,##args)
#define rtk_rg_port_greenEnable_get(arg,args...) rtk_port_greenEnable_get(arg,##args)
#define rtk_rg_port_greenEnable_set(arg,args...) rtk_port_greenEnable_set(arg,##args)
#define rtk_rg_port_init(arg,args...) rtk_port_init(arg,##args)

#define rtk_rg_port_isolationEntryExt_get(arg,args...) rtk_port_isolationEntryExt_get(arg,##args)
#define rtk_rg_port_isolationEntryExt_set(arg,args...) rtk_port_isolationEntryExt_set(arg,##args)

#define rtk_rg_port_link_get(arg,args...) rtk_port_link_get(arg,##args)
#define rtk_rg_port_macExtMode_get(arg,args...) rtk_port_macExtMode_get(arg,##args)
#define rtk_rg_port_macExtMode_set(arg,args...) rtk_port_macExtMode_set(arg,##args)
#define rtk_rg_port_macExtRgmiiDelay_get(arg,args...) rtk_port_macExtRgmiiDelay_get(arg,##args)
#define rtk_rg_port_macExtRgmiiDelay_set(arg,args...) rtk_port_macExtRgmiiDelay_set(arg,##args)
#define rtk_rg_port_macForceAbilityState_get(arg,args...) rtk_port_macForceAbilityState_get(arg,##args)
#define rtk_rg_port_macForceAbilityState_set(arg,args...) rtk_port_macForceAbilityState_set(arg,##args)
#define rtk_rg_port_macForceAbility_get(arg,args...) rtk_port_macForceAbility_get(arg,##args)
#define rtk_rg_port_macForceAbility_set(arg,args...) rtk_port_macForceAbility_set(arg,##args)
#define rtk_rg_port_macLocalLoopbackEnable_get(arg,args...) rtk_port_macLocalLoopbackEnable_get(arg,##args)
#define rtk_rg_port_macLocalLoopbackEnable_set(arg,args...) rtk_port_macLocalLoopbackEnable_set(arg,##args)
#define rtk_rg_port_macRemoteLoopbackEnable_get(arg,args...) rtk_port_macRemoteLoopbackEnable_get(arg,##args)
#define rtk_rg_port_macRemoteLoopbackEnable_set(arg,args...) rtk_port_macRemoteLoopbackEnable_set(arg,##args)
#define rtk_rg_port_phyAutoNegoAbility_get(arg,args...) rtk_port_phyAutoNegoAbility_get(arg,##args)
#define rtk_rg_port_phyAutoNegoAbility_set(arg,args...) rtk_port_phyAutoNegoAbility_set(arg,##args)
#define rtk_rg_port_phyAutoNegoEnable_get(arg,args...) rtk_port_phyAutoNegoEnable_get(arg,##args)
#define rtk_rg_port_phyAutoNegoEnable_set(arg,args...) rtk_port_phyAutoNegoEnable_set(arg,##args)
#define rtk_rg_port_phyCrossOverMode_get(arg,args...) rtk_port_phyCrossOverMode_get(arg,##args)
#define rtk_rg_port_phyCrossOverMode_set(arg,args...) rtk_port_phyCrossOverMode_set(arg,##args)
#define rtk_rg_port_phyForceModeAbility_get(arg,args...) rtk_port_phyForceModeAbility_get(arg,##args)
#define rtk_rg_port_phyForceModeAbility_set(arg,args...) rtk_port_phyForceModeAbility_set(arg,##args)
#define rtk_rg_port_phyMasterSlave_get(arg,args...) rtk_port_phyMasterSlave_get(arg,##args)
#define rtk_rg_port_phyMasterSlave_set(arg,args...) rtk_port_phyMasterSlave_set(arg,##args)
#define rtk_rg_port_phyReg_get(arg,args...) rtk_port_phyReg_get(arg,##args)
#define rtk_rg_port_phyReg_set(arg,args...) rtk_port_phyReg_set(arg,##args)
#define rtk_rg_port_phyTestMode_get(arg,args...) rtk_port_phyTestMode_get(arg,##args)
#define rtk_rg_port_phyTestMode_set(arg,args...) rtk_port_phyTestMode_set(arg,##args)
#define rtk_rg_port_rtctResult_get(arg,args...) rtk_port_rtctResult_get(arg,##args)
#define rtk_rg_port_rtct_start(arg,args...) rtk_port_rtct_start(arg,##args)
#define rtk_rg_port_specialCongestStatus_clear(arg,args...) rtk_port_specialCongestStatus_clear(arg,##args)
#define rtk_rg_port_specialCongestStatus_get(arg,args...) rtk_port_specialCongestStatus_get(arg,##args)
#define rtk_rg_port_specialCongest_get(arg,args...) rtk_port_specialCongest_get(arg,##args)
#define rtk_rg_port_specialCongest_set(arg,args...) rtk_port_specialCongest_set(arg,##args)
#define rtk_rg_port_speedDuplex_get(arg,args...) rtk_port_speedDuplex_get(arg,##args)

//from rate.h
#include<rtk/rate.h>
#define rtk_rg_rate_egrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_egrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_egrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_egrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlEnable_get(arg,args...) rtk_rate_egrQueueBwCtrlEnable_get(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlEnable_set(arg,args...) rtk_rate_egrQueueBwCtrlEnable_set(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlMeterIdx_get(arg,args...) rtk_rate_egrQueueBwCtrlMeterIdx_get(arg,##args)
#define rtk_rg_rate_egrQueueBwCtrlMeterIdx_set(arg,args...) rtk_rate_egrQueueBwCtrlMeterIdx_set(arg,##args)
#define rtk_rg_rate_init(arg,args...) rtk_rate_init(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlRate_get(arg,args...) rtk_rate_portEgrBandwidthCtrlRate_get(arg,##args)
#define rtk_rg_rate_portEgrBandwidthCtrlRate_set(arg,args...) rtk_rate_portEgrBandwidthCtrlRate_set(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlIncludeIfg_get(arg,args...) rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlIncludeIfg_set(arg,args...) rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlRate_get(arg,args...) rtk_rate_portIgrBandwidthCtrlRate_get(arg,##args)
#define rtk_rg_rate_portIgrBandwidthCtrlRate_set(arg,args...) rtk_rate_portIgrBandwidthCtrlRate_set(arg,##args)
#define rtk_rg_rate_shareMeterBucket_get(arg,args...) rtk_rate_shareMeterBucket_get(arg,##args)
#define rtk_rg_rate_shareMeterBucket_set(arg,args...) rtk_rate_shareMeterBucket_set(arg,##args)
#define rtk_rg_rate_shareMeterExceed_clear(arg,args...) rtk_rate_shareMeterExceed_clear(arg,##args)
#define rtk_rg_rate_shareMeterExceed_get(arg,args...) rtk_rate_shareMeterExceed_get(arg,##args)
#define rtk_rg_rate_shareMeterMode_get(arg,args...) rtk_rate_shareMeterMode_get(arg,##args)
#define rtk_rg_rate_shareMeterMode_set(arg,args...) rtk_rate_shareMeterMode_set(arg,##args)
#define rtk_rg_rate_shareMeter_get(arg,args...) rtk_rate_shareMeter_get(arg,##args)
#define rtk_rg_rate_shareMeter_set(arg,args...) rtk_rate_shareMeter_set(arg,##args)
#define rtk_rg_rate_stormBypass_get(arg,args...) rtk_rate_stormBypass_get(arg,##args)
#define rtk_rg_rate_stormBypass_set(arg,args...) rtk_rate_stormBypass_set(arg,##args)
#define rtk_rg_rate_stormControlEnable_get(arg,args...) rtk_rate_stormControlEnable_get(arg,##args)
#define rtk_rg_rate_stormControlEnable_set(arg,args...) rtk_rate_stormControlEnable_set(arg,##args)
#define rtk_rg_rate_stormControlMeterIdx_get(arg,args...) rtk_rate_stormControlMeterIdx_get(arg,##args)
#define rtk_rg_rate_stormControlMeterIdx_set(arg,args...) rtk_rate_stormControlMeterIdx_set(arg,##args)
#define rtk_rg_rate_stormControlPortEnable_get(arg,args...) rtk_rate_stormControlPortEnable_get(arg,##args)
#define rtk_rg_rate_stormControlPortEnable_set(arg,args...) rtk_rate_stormControlPortEnable_set(arg,##args)
#define rtk_rg_rate_hostMacAddr_get(arg,args...) rtk_rate_hostMacAddr_get(arg,##args)
#define rtk_rg_rate_hostMacAddr_set(arg,args...) rtk_rate_hostMacAddr_set(arg,##args)
#define rtk_rg_rate_hostBwCtrlMeterIdx_get(arg,args...) rtk_rate_hostBwCtrlMeterIdx_get(arg,##args)
#define rtk_rg_rate_hostBwCtrlMeterIdx_set(arg,args...) rtk_rate_hostBwCtrlMeterIdx_set(arg,##args)
#define rtk_rg_rate_hostEgrBwCtrlState_get(arg,args...) rtk_rate_hostEgrBwCtrlState_get(arg,##args)
#define rtk_rg_rate_hostEgrBwCtrlState_set(arg,args...) rtk_rate_hostEgrBwCtrlState_set(arg,##args)
#define rtk_rg_rate_hostIgrBwCtrlState_get(arg,args...) rtk_rate_hostIgrBwCtrlState_get(arg,##args)
#define rtk_rg_rate_hostIgrBwCtrlState_set(arg,args...) rtk_rate_hostIgrBwCtrlState_set(arg,##args)

//from rldp.h
#include<rtk/rldp.h>
#define rtk_rg_rldp_config_get(arg,args...) rtk_rldp_config_get(arg,##args)
#define rtk_rg_rldp_config_set(arg,args...) rtk_rldp_config_set(arg,##args)
#define rtk_rg_rldp_init(arg,args...) rtk_rldp_init(arg,##args)
#define rtk_rg_rldp_portConfig_get(arg,args...) rtk_rldp_portConfig_get(arg,##args)
#define rtk_rg_rldp_portConfig_set(arg,args...) rtk_rldp_portConfig_set(arg,##args)
#define rtk_rg_rldp_portStatus_clear(arg,args...) rtk_rldp_portStatus_clear(arg,##args)
#define rtk_rg_rldp_portStatus_get(arg,args...) rtk_rldp_portStatus_get(arg,##args)
#define rtk_rg_rldp_status_get(arg,args...) rtk_rldp_status_get(arg,##args)
#define rtk_rg_rlpp_init(arg,args...) rtk_rlpp_init(arg,##args)
#define rtk_rg_rlpp_trapType_get(arg,args...) rtk_rlpp_trapType_get(arg,##args)
#define rtk_rg_rlpp_trapType_set(arg,args...) rtk_rlpp_trapType_set(arg,##args)


//from stat.h
#include<rtk/stat.h>
#define rtk_rg_stat_global_get(arg,args...) rtk_stat_global_get(arg,##args)
#define rtk_rg_stat_global_getAll(arg,args...) rtk_stat_global_getAll(arg,##args)
#define rtk_rg_stat_global_reset(arg,args...) rtk_stat_global_reset(arg,##args)
#define rtk_rg_stat_init(arg,args...) rtk_stat_init(arg,##args)
#define rtk_rg_stat_logCtrl_get(arg,args...) rtk_stat_logCtrl_get(arg,##args)
#define rtk_rg_stat_logCtrl_set(arg,args...) rtk_stat_logCtrl_set(arg,##args)
#define rtk_rg_stat_log_get(arg,args...) rtk_stat_log_get(arg,##args)
#define rtk_rg_stat_log_reset(arg,args...) rtk_stat_log_reset(arg,##args)
#define rtk_rg_stat_mibCntMode_get(arg,args...) rtk_stat_mibCntMode_get(arg,##args)
#define rtk_rg_stat_mibCntMode_set(arg,args...) rtk_stat_mibCntMode_set(arg,##args)
#define rtk_rg_stat_mibCntTagLen_get(arg,args...) rtk_stat_mibCntTagLen_get(arg,##args)
#define rtk_rg_stat_mibCntTagLen_set(arg,args...) rtk_stat_mibCntTagLen_set(arg,##args)
#define rtk_rg_stat_mibLatchTimer_get(arg,args...) rtk_stat_mibLatchTimer_get(arg,##args)
#define rtk_rg_stat_mibLatchTimer_set(arg,args...) rtk_stat_mibLatchTimer_set(arg,##args)
#define rtk_rg_stat_mibSyncMode_get(arg,args...) rtk_stat_mibSyncMode_get(arg,##args)
#define rtk_rg_stat_mibSyncMode_set(arg,args...) rtk_stat_mibSyncMode_set(arg,##args)
#define rtk_rg_stat_pktInfo_get(arg,args...) rtk_stat_pktInfo_get(arg,##args)
#define rtk_rg_stat_port_get(arg,args...) rtk_stat_port_get(arg,##args)
#define rtk_rg_stat_port_getAll(arg,args...) rtk_stat_port_getAll(arg,##args)
#define rtk_rg_stat_port_reset(arg,args...) rtk_stat_port_reset(arg,##args)
#define rtk_rg_stat_rstCntValue_get(arg,args...) rtk_stat_rstCntValue_get(arg,##args)
#define rtk_rg_stat_rstCntValue_set(arg,args...) rtk_stat_rstCntValue_set(arg,##args)
#define rtk_rg_stat_hostCnt_reset(arg,args...) rtk_stat_hostCnt_reset(arg,##args)
#define rtk_rg_stat_hostCnt_get(arg,args...) rtk_stat_hostCnt_get(arg,##args)
#define rtk_rg_stat_hostState_get(arg,args...) rtk_stat_hostState_get(arg,##args)
#define rtk_rg_stat_hostState_set(arg,args...) rtk_stat_hostState_set(arg,##args)

//from switch.h
#include<rtk/switch.h>
#define rtk_rg_switch_allExtPortMask_set(arg,args...) rtk_switch_allExtPortMask_set(arg,##args)
#define rtk_rg_switch_allPortMask_set(arg,args...) rtk_switch_allPortMask_set(arg,##args)
#define rtk_rg_switch_chip_reset(arg,args...) rtk_switch_chip_reset(arg,##args)
#define rtk_rg_switch_deviceInfo_get(arg,args...) rtk_switch_deviceInfo_get(arg,##args)
#define rtk_rg_switch_init(arg,args...) rtk_switch_init(arg,##args)
#define rtk_rg_switch_logicalPort_get(arg,args...) rtk_switch_logicalPort_get(arg,##args)
#define rtk_rg_switch_maxPktLenLinkSpeed_get(arg,args...) rtk_switch_maxPktLenLinkSpeed_get(arg,##args)
#define rtk_rg_switch_maxPktLenLinkSpeed_set(arg,args...) rtk_switch_maxPktLenLinkSpeed_set(arg,##args)
#define rtk_rg_switch_mgmtMacAddr_get(arg,args...) rtk_switch_mgmtMacAddr_get(arg,##args)
#define rtk_rg_switch_mgmtMacAddr_set(arg,args...) rtk_switch_mgmtMacAddr_set(arg,##args)
#define rtk_rg_switch_nextPortInMask_get(arg,args...) rtk_switch_nextPortInMask_get(arg,##args)
#define rtk_rg_switch_phyPortId_get(arg,args...) rtk_switch_phyPortId_get(arg,##args)
#define rtk_rg_switch_port2PortMask_clear(arg,args...) rtk_switch_port2PortMask_clear(arg,##args)
#define rtk_rg_switch_port2PortMask_set(arg,args...) rtk_switch_port2PortMask_set(arg,##args)
#define rtk_rg_switch_portIdInMask_check(arg,args...) rtk_switch_portIdInMask_check(arg,##args)
#define rtk_rg_switch_portMask_Clear(arg,args...) rtk_switch_portMask_Clear(arg,##args)
#define rtk_rg_switch_version_get(arg,args...) rtk_switch_version_get(arg,##args)
#define rtk_rg_switch_maxPktLenByPort_get(arg,args...) rtk_switch_maxPktLenByPort_get(arg,##args)
#define rtk_rg_switch_maxPktLenByPort_set(arg,args...) rtk_switch_maxPktLenByPort_set(arg,##args)
#define rtk_rg_switch_version_get(arg,args...) rtk_switch_version_get(arg,##args)


//from trap.h
#include<rtk/trap.h>
#define rtk_rg_trap_igmpCtrlPkt2CpuEnable_get(arg,args...) rtk_trap_igmpCtrlPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_igmpCtrlPkt2CpuEnable_set(arg,args...) rtk_trap_igmpCtrlPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_init(arg,args...) rtk_trap_init(arg,##args)
#define rtk_rg_trap_ipMcastPkt2CpuEnable_get(arg,args...) rtk_trap_ipMcastPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_ipMcastPkt2CpuEnable_set(arg,args...) rtk_trap_ipMcastPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_l2McastPkt2CpuEnable_get(arg,args...) rtk_trap_l2McastPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_l2McastPkt2CpuEnable_set(arg,args...) rtk_trap_l2McastPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_mldCtrlPkt2CpuEnable_get(arg,args...) rtk_trap_mldCtrlPkt2CpuEnable_get(arg,##args)
#define rtk_rg_trap_mldCtrlPkt2CpuEnable_set(arg,args...) rtk_trap_mldCtrlPkt2CpuEnable_set(arg,##args)
#define rtk_rg_trap_oamPduAction_get(arg,args...) rtk_trap_oamPduAction_get(arg,##args)
#define rtk_rg_trap_oamPduAction_set(arg,args...) rtk_trap_oamPduAction_set(arg,##args)
#define rtk_rg_trap_oamPduPri_get(arg,args...) rtk_trap_oamPduPri_get(arg,##args)
#define rtk_rg_trap_oamPduPri_set(arg,args...) rtk_trap_oamPduPri_set(arg,##args)
#define rtk_rg_trap_portIgmpMldCtrlPktAction_get(arg,args...) rtk_trap_portIgmpMldCtrlPktAction_get(arg,##args)
#define rtk_rg_trap_portIgmpMldCtrlPktAction_set(arg,args...) rtk_trap_portIgmpMldCtrlPktAction_set(arg,##args)
#define rtk_rg_trap_reasonTrapToCpuPriority_get(arg,args...) rtk_trap_reasonTrapToCpuPriority_get(arg,##args)
#define rtk_rg_trap_reasonTrapToCpuPriority_set(arg,args...) rtk_trap_reasonTrapToCpuPriority_set(arg,##args)
#define rtk_rg_trap_rmaAction_get(arg,args...) rtk_trap_rmaAction_get(arg,##args)
#define rtk_rg_trap_rmaAction_set(arg,args...) rtk_trap_rmaAction_set(arg,##args)
#define rtk_rg_trap_rmaPri_get(arg,args...) rtk_trap_rmaPri_get(arg,##args)
#define rtk_rg_trap_rmaPri_set(arg,args...) rtk_trap_rmaPri_set(arg,##args)

#include "rtk/classify.h"
#define rtk_rg_classify_unmatchAction_set(arg,args...) rtk_classify_unmatchAction_set(arg,##args)

#include "rtk/vlan.h"
#define rtk_rg_vlan_reservedVidAction_set(arg,args...) rtk_vlan_reservedVidAction_set(arg,##args)
#define rtk_rg_vlan_vlanFunctionEnable_set(arg,args...) rtk_vlan_vlanFunctionEnable_set(arg,##args)

#include "rtk/svlan.h"
#define rtk_rg_svlan_svlanFunctionEnable_set(arg,args...) rtk_svlan_svlanFunctionEnable_set(arg,##args)

#include "rtk/led.h"
#define rtk_rg_led_pon_port_set(arg,args...) rtk_led_pon_port_set(arg,##args)

#include "rtk/pon_led.h"
#define rtk_rg_pon_led_status_set(arg,args...) rtk_pon_led_status_set(arg,##args)

#include "rtk/qos.h"
#define rtk_rg_qos_schedulingType_set(arg,args...) rtk_qos_schedulingType_set(arg,##args)

#include "rtk/l2.h"
#define rtk_rg_l2_portAgingEnable_set(arg,args...) rtk_l2_portAgingEnable_set(arg,##args)
#define rtk_rg_l2_aging_set(arg,args...) rtk_l2_aging_set(arg,##args)
#define rtk_rg_l2_portAgingEnable_get(arg,args...) rtk_l2_portAgingEnable_get(arg,##args)
#define rtk_rg_l2_aging_get(arg,args...) rtk_l2_aging_get(arg,##args)

#define rtk_rg_l2_ipmcMode_get(arg,args...) rtk_l2_ipmcMode_get(arg,##args)
#define rtk_rg_l2_ipmcMode_set(arg,args...) rtk_l2_ipmcMode_set(arg,##args)
#define rtk_rg_l2_ipmcGroupLookupMissHash_set(arg,args...) rtk_l2_ipmcGroupLookupMissHash_set(arg,##args)

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#define MAX_TRAP_HASH_RESULT 16

#define RTK_RG_LOG_FLOW_STRING_SIZE	32

#define RTK_RG_UNMATCH_FLOW_TRAP_PRI_HTTP 4
#define RTK_RG_UNMATCH_FLOW_TRAP_PRI 3
#define RTK_RG_UNMATCH_FLOW_TRAP_PRI_DEFAULT 0

#endif


#endif

