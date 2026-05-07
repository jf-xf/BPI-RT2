#ifndef PTOOL_H
#define PTOOL_H

#ifdef CONFIG_RTK_PTOOL_CPU_PERF

#if defined(CONFIG_CA8277B_SERIES) || defined(CONFIG_RTL8277C_SERIES)
#include <linux/math64.h>
#define MAX_HASH_IDX_BIT 10
#define MAX_HASH_IDX (1<<MAX_HASH_IDX_BIT)
#define MAX_HASH_WAY_BIT 4	/* 16 way */
#define FUNC_LEN	64

int profile_start(int);
int profile_stop(void);
int profile(const char *func,u64 pc);
void profile_end(int idx);
int profile_dump(void);

/*
[1] Realtek ARMv8 CPU performance monitor.

	user guide:
		1. add "#include <linux/ptool.h>" in front of the trace file.
		2. add "PROFILE_START" to the function header and add "PROFILE_END" at the end of function.
		3. Start monitoring with "echo 1 > /proc/ptool/profile_enable".
		4. Stop monitoring with "echo 0 > /proc/ptool/profile_enable".
		5. Dump statistics by "cat /proc/ptool/profile_dump".		
*/

#define PROFILE_START int profile_idx; \
	do {\
		asm __volatile__("	MSR pmcr_el0,xzr \n":::);\
		{\
			u64 pc;\
			asm __volatile__("	ADR %0, . \n":"=r"(pc)::); \
			profile_idx=profile(__FUNCTION__,pc);\
			asm __volatile__(	"	MOV w0,#0x1 \n"\
			"	MSR pmcr_el0,x0 \n":::"x0");\
		}\
	} while(0);
	
#define PROFILE_END   do {\
	asm __volatile__("	MSR pmcr_el0,xzr \n":::);\
	profile_end(profile_idx);\
	asm __volatile__(	"	MOV w0,#0x1 \n"\
	"	MSR pmcr_el0,x0 \n":::"x0");\
	} while(0);

#define PROFILE_START_VAR(caller_profile_idx, caller_string); \
		do {\
			asm __volatile__("	MSR pmcr_el0,xzr \n":::);\
			{\
				u64 pc;\
				asm __volatile__("	ADR %0, . \n":"=r"(pc)::); \
				caller_profile_idx=profile(caller_string,pc);\
				asm __volatile__(	"	MOV w0,#0x1 \n"\
				"	MSR pmcr_el0,x0 \n":::"x0");\
			}\
		} while(0);
		
#define PROFILE_END_VAR(caller_profile_idx)   do {\
		asm __volatile__("	MSR pmcr_el0,xzr \n":::);\
		profile_end(caller_profile_idx);\
		asm __volatile__(	"	MOV w0,#0x1 \n"\
		"	MSR pmcr_el0,x0 \n":::"x0");\
		} while(0);

#elif defined(CONFIG_RTL9607C_SERIES)

#include <linux/math64.h>
#define MAX_HASH_IDX_BIT 12
#define MAX_HASH_IDX (1<<MAX_HASH_IDX_BIT)
#define MAX_HASH_WAY_BIT 4	/* 16 way */
#define FUNC_LEN	42

int profile_start(void);
int profile_stop(void);
int profile(const char *func);
void profile_end(int idx);
int profile_dump(void);

/*
[1] Realtek CPU performance monitor.

	user guide:
		1. add "#include <linux/ptool.h>" in front of the trace file.
		2. add "PROFILE_START" to the function header and add "PROFILE_END" at the end of function.
		3. Start monitoring with "echo 1 > /proc/ptool/profile_enable".
		4. Stop monitoring with "echo 0 > /proc/ptool/profile_enable".
		5. Dump statistics by "cat /proc/ptool/profile_dump".
	
[2] Logical analysis by Realtek GPIO.

	user guide:
		1. add "#include <linux/ptool.h>" in front of the trace file.
		2. add "LA_GPIO_HIGH(LA_GPIO_XXX)" to the function header and add "LA_GPIO_LOW(LA_GPIO_XXX)" at the end of function.
		3. config  which GPIO pin is used by each LA_GPIO_XXX (only pin1~pin63 is supportted)
		     please see "cat /proc/ptool/la_gpio_set" for more tips.		
*/

#define PROFILE_START int profile_idx=profile(__FUNCTION__);
#define PROFILE_END profile_end(profile_idx);
#define PROFILE_START_VAR(caller_profile_idx, caller_string) do{} while(0);
#define PROFILE_END_VAR(caller_profile_idx, caller_string) do{} while(0);

#else

#define PROFILE_START do{} while(1);
#define PROFILE_END do{} while(1);
#define PROFILE_START_VAR(caller_profile_idx, caller_string) do{} while(0);
#define PROFILE_END_VAR(caller_profile_idx, caller_string) do{} while(0);

#endif	//chip

#endif	//CONFIG_RTK_PTOOL_CPU_PERF

#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_PTOOL_LA_BY_GPIO)

typedef enum rtk_la_gpio_signal_e
{
	LA_GPIO_SOFTIRQ,
	LA_GPIO_HW_ISR,
	LA_GPIO_SPINLOCK,
	LA_GPIO_NIC_RX,
	LA_GPIO_NIC_TX,	
	LA_GPIO_WIFI_RX_DSR,	
	LA_GPIO_WIFI_TX,	
	LA_GPIO_AUX0,
	LA_GPIO_AUX1,
	LA_GPIO_AUX2,
	LA_GPIO_AUX3,
	LA_GPIO_AUX4,
	LA_GPIO_AUX5,
	LA_GPIO_AUX6,
	LA_GPIO_AUX7,
	LA_GPIO_MAX,
} rtk_la_gpio_signal_t;

extern int la_gpio_signal_cpu_mask[LA_GPIO_MAX];
extern int la_gpio_signal_gpio_id[LA_GPIO_MAX];
void la_gpio_high(int cpu_mask, int gpio_id);
void la_gpio_low(int cpu_mask, int gpio_id);

#define LA_GPIO_HIGH(signal) do { if(la_gpio_signal_gpio_id[signal]) la_gpio_high(la_gpio_signal_cpu_mask[signal], la_gpio_signal_gpio_id[signal]); } while(0);
#define LA_GPIO_LOW(signal) do { if(la_gpio_signal_gpio_id[signal]) la_gpio_low(la_gpio_signal_cpu_mask[signal], la_gpio_signal_gpio_id[signal]); } while(0);

#else	
#define LA_GPIO_HIGH(x) do{} while(0);
#define LA_GPIO_LOW(x)  do{} while(0);	
#endif		//CONFIG_RTL9607C_SERIES && CONFIG_RTK_PTOOL_LA_BY_GPIO

#endif

