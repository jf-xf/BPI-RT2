#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
#elif defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
#ifndef _RG_PROFILE_H
#define _RG_PROFILE_H

#include <asm/processor.h>
#include <asm/uaccess.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/seq_file.h>


//#include "rtk_rg_struct.h"
//#include "rtk_rg_internal.h"
#include "rtk_rg_define.h"
char *proc_printf(struct seq_file *s, char *fmt, ...);
#define PROC_PRINTF( comment ,arg...)	do { proc_printf(s,comment,##arg); }while(0)





#define asm_get_perf_ctrl3()        __asm_mfc0($13, 0)
#define asm_set_perf_ctrl3(v)       __asm_mtc0(v, $13, 0)

#ifndef TO_STR
    #define _TO_STR(_s) #_s
    #define TO_STR(s) _TO_STR(s)
#endif


#define PERF_DISABLE            (0x0)
#define PERF_KERNEL_MODE        (0x1)
#define PERF_SUPERVISOR_MODE    (0x2)
#define PERF_USER_MODE          (0x4)
#define PERF_ALL_MODE           (PERF_KERNEL_MODE|PERF_SUPERVISOR_MODE|PERF_USER_MODE)


#define __asm_mfc0(mfc_reg, mfc_sel) ({   \
                        unsigned int __ret; \
                        __asm__ __volatile__ ( \
                                              "mfc0 %0," TO_STR(mfc_reg) "," TO_STR(mfc_sel) ";\n\t" \
                                              : "=r" (__ret)); \
                        __ret;})

#define __asm_mtc0(value, mtc_reg, mtc_sel) ({    \
                        unsigned int __value=(value); \
                        __asm__ __volatile__ ( \
                                              "mtc0 %0, " TO_STR(mtc_reg) "," TO_STR(mtc_sel) ";\n\t" \
                                              : : "r" (__value)); })

#define __asm_mtc0_clear(mtc_reg, mtc_sel) ({    \
                        __asm__ __volatile__ ( \
                                              "mtc0 $0, " TO_STR(mtc_reg) "," TO_STR(mtc_sel) ";\n\t" \
                                              : : ); })


#define asm_get_perf_ctrl0()        __asm_mfc0($25, 0)
#define asm_get_perf_data0()        __asm_mfc0($25, 1)
#define asm_get_perf_ctrl1()        __asm_mfc0($25, 2)
#define asm_get_perf_data1()        __asm_mfc0($25, 3)

#define asm_set_perf_ctrl0(v)       __asm_mtc0(v, $25, 0)
#define asm_set_perf_data0(v)       __asm_mtc0(v, $25, 1)
#define asm_set_perf_ctrl1(v)       __asm_mtc0(v, $25, 2)
#define asm_set_perf_data1(v)       __asm_mtc0(v, $25, 3)

#define asm_clear_perf_data0()      __asm_mtc0_clear($25, 1)
#define asm_clear_perf_data1()      __asm_mtc0_clear($25, 3)


#define _asm_get_cp0_count() ({ \
    u32 __ret=0;                \
    __asm__ __volatile__ (      \
    "   mfc0    %0, $9  "       \
    : "=r"(__ret)               \
    );                          \
    __ret;                      \
})

#define get_all_event_counters(con0, con1) \
    con0=asm_get_perf_data0();             \
    con1=asm_get_perf_data1();             \


#define clear_all_cp3_event_counter()    \
    asm_clear_perf_data0();    \
    asm_clear_perf_data1();  \

typedef union {
    struct {
        unsigned int m:1; //1
        unsigned int mbz_0:1; //0
        unsigned int tcid:8; //0
        unsigned int mt_en:2; //0
        unsigned int vpeid:4; //0
        unsigned int pctd:1; //0
        unsigned int mbz_1:3; //0
        unsigned int event:7; //0
        unsigned int ie:1; //0
        unsigned int usk:3; //0
        unsigned int exl:1; //0
    } f;
    unsigned int v;
} perf_ctrl_t;

typedef enum mips_cnt0_perf_cnt_type_e
{
	MIPS_CNT0_CYCLES=0,
	MIPS_CNT0_DATA_CACHE_MISSES=11,
	MIPS_CNT0_STORE_MISSES=13,
	MIPS_CNT0_L2_CACHE_MISSES=22,
	MIPS_CNT0_UNCACHE_LOADS=33,
	MIPS_CNT0_INST_MISS_STALL_CYCLES=37,
	MIPS_CNT0_DATA_MISS_STALL_CYCLES=39,
	MIPS_CNT0_UNCACHED_STALL_CYCLES=40,
} mips_cnt0_perf_cnt_type_t;

typedef enum mips_cnt1_perf_cnt_type_e
{
	MIPS_CNT1_CYCLES=0,
	MIPS_CNT1_INST_CAHCE_MISSES=9,
	MIPS_CNT1_DATA_CACHE_MISSES=11,
	MIPS_CNT1_LOAD_MISSES=13,
	MIPS_CNT1_L2_CACHE_ACCESSES=21,
	MIPS_CNT1_UNCACHE_STORES=33,
	MIPS_CNT1_DATA_MISS_STALL_CYCLES=37,
	MIPS_CNT1_L2_MISS_CYCLES=39,
} mips_cnt1_perf_cnt_type_t;

typedef enum mips_perf_functions_e
{
	MIPS_PERF_NIC_RX=0,
	MIPS_PERF_RG_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_RG_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_RG_FROM_NIC_RX,
	MIPS_PERF_RG_FROM_WIFI_RX,
	MIPS_PERF_RG_FROM_PS,
	MIPS_PERF_RG_TIMER_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_RG_FROM_TIMER,
	MIPS_PERF_GMAC0_TX_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC0_TX_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC0_TX,
	MIPS_PERF_GMAC1_TX_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC1_TX_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC1_TX,
	MIPS_PERF_GMAC2_TX_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC2_TX_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC2_TX,
	MIPS_PERF_WLAN0_TX_ENQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WLAN0_TX_DEQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WLAN0_TX,
	MIPS_PERF_WLAN1_TX_ENQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WLAN1_TX_DEQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WLAN1_TX,
//workqueue
	MIPS_PERF_GMAC9_TX_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC9_TX_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC9_TX,
	MIPS_PERF_GMAC10_TX_ENQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC10_TX_DEQUEUE_WAIT_UNLOCK,
	MIPS_PERF_GMAC10_TX,
	MIPS_PERF_WIFI_AC_TX_ENQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WIFI_AC_TX_DEQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WIFI_AC_TX,
	MIPS_PERF_WIFI_N_TX_ENQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WIFI_N_TX_DEQUEUE_WAIT_UNLCOK,
	MIPS_PERF_WIFI_N_TX,
	MIPS_PERF_MAX
}mips_perf_functions_t;

typedef struct mips_perf_table_s
{
	uint32 count[4];
	uint64 event_count0[4];
	uint64 event_count1[4];
	uint32 tmp_event_count0[4];
	uint32 tmp_event_count1[4];
} mips_perf_table_t;


void mips_perf_measure_entrance(int index);
void mips_perf_measure_exit(int index);
int mips_perf_measure_dump(struct inode *inode, struct file *file);
int mips_perf_measure_reset(struct inode *inode, struct file *file);
extern int _rtk_rg_pasring_proc_string_to_integer(const char *buff,unsigned long len);

#endif

#else //other platfrom
#error
#endif
