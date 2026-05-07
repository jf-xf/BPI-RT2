#ifndef _RTK_IGMP_HOOK_H
#define _RTK_IGMP_HOOK_H

#include <rtk_igmp_struct.h>
#include <rtk_igmp_debug.h>


void *rtk_igmp_malloc(int NBYTES) ;
void rtk_igmp_free(void *APTR) ;

extern rtk_igmp_Sysdb_t  igmpSysdb;;


#if defined(CONFIG_DEBUG_SPINLOCK)
#define IGMP_ASSERT_NO_HARDIRQ()	\
	do{ \
		if(in_irq()){													\
		IGMP_WARNING("call spin_lock when in_irq() is TRUE");	\
		dump_stack();}												\
	}while(0)

#define IGMP_ASSERT_IN_INTERRUPT() \
	do{ \
		if(!in_interrupt()){	\
		IGMP_WARNING("expect hardirq or softirq here, preempt_count = %d", preempt_count());	\
		dump_stack();} \
	}while(0)

#define IGMP_ASSERT_PREEMPT_DISABLE() \
	do{ \
		if(preempt_count()<=0) {	\
		IGMP_WARNING("expect preempt should be disabled");	\
		dump_stack();} \
	}while(0)
#else
#define IGMP_ASSERT_NO_HARDIRQ()
#define IGMP_ASSERT_IN_INTERRUPT()
#define IGMP_ASSERT_PREEMPT_DISABLE()
#endif


#define igmp_spin_lock(lock)			\
	do {						\
		IGMP_ASSERT_NO_HARDIRQ();\
		IGMP_ASSERT_IN_INTERRUPT();\
		spin_lock(&lock); \
	} while(0)

#define igmp_spin_unlock(lock)			\
	do {						\
		spin_unlock(&lock); \
	} while(0)


#define igmp_spin_lock_bh(lock)	\
	do {					\
		IGMP_ASSERT_NO_HARDIRQ();\
		spin_lock_bh(&lock);\
	} while(0)

#define igmp_spin_unlock_bh(lock)	\
	do {					\
		spin_unlock_bh(&lock);	\
	} while(0)


#define igmp_spin_lock_bh_irq_protect(lock)			\
	do {	/*in normal case: caller is bottom half and use irqsave*/	\
		if(irqs_disabled() && preempt_count()>0){	\
			fc_spin_lock(lock);}			\
		else{							\
			fc_spin_lock_bh(lock);}		\
	} while(0)
#define igmp_spin_unlock_bh_irq_protect(lock)			\
	do {						\
		if(irqs_disabled()){			\
			fc_spin_unlock(lock);}		\
		else{							\
			fc_spin_unlock_bh(lock);}	\
	} while(0)




#endif


