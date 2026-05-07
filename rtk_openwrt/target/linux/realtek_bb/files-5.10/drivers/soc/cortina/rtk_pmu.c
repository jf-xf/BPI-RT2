/*
 * Copyright (c) Cortina-Access Limited 2015.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* refer to /linux-4.4.x/arch/arm/kernel/perf_event_v7.c */

#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <soc/cortina/cortina-soc.h>
#include <soc/cortina/rtk_pmu.h>

#define	ARMV7_PMNC_MASK		0x3f	 /* Mask for writable bits */
#define	ARMV7_IDX_CYCLE_COUNTER	0
#define	ARMV7_IDX_COUNTER0	1
#define	ARMV7_MAX_COUNTERS	32
#define	ARMV7_COUNTER_MASK	(ARMV7_MAX_COUNTERS - 1)

/*
 * Perf Event to low level counters mapping
 */
#define	ARMV7_IDX_TO_COUNTER(x)	\
	(((x) - ARMV7_IDX_COUNTER0) & ARMV7_COUNTER_MASK)

/*
 * Per-CPU PMNC: config reg
 */
#define ARMV7_PMNC_E		(1 << 0) /* Enable all counters */
#define ARMV7_PMNC_P		(1 << 1) /* Reset all counters */
#define ARMV7_PMNC_C		(1 << 2) /* Cycle counter reset */
#define ARMV7_PMNC_D		(1 << 3) /* CCNT counts every 64th cpu cycle */
#define ARMV7_PMNC_X		(1 << 4) /* Export to ETM */
#define ARMV7_PMNC_DP		(1 << 5) /* Disable CCNT if non-invasive debug*/
#define	ARMV7_PMNC_N_SHIFT	11	 /* Number of counters supported */
#define	ARMV7_PMNC_N_MASK	0x1f
#define	ARMV7_PMNC_MASK		0x3f	 /* Mask for writable bits */

/*
 * FLAG: counters overflow flag status reg
 */
#define	ARMV7_FLAG_MASK		0xffffffff	/* Mask for writable bits */
#define	ARMV7_OVERFLOWED_MASK	ARMV7_FLAG_MASK

/*
 * PMXEVTYPER: Event selection reg
 */
#define	ARMV7_EVTYPE_MASK	0xc80000ff	/* Mask for writable bits */
#define	ARMV7_EVTYPE_EVENT	0xff		/* Mask for EVENT bits */

raw_spinlock_t		pmu_lock;

#define NESTED_CHECK	1
#ifdef NESTED_CHECK
unsigned int		pmnc_state;
#endif

static inline uint32_t armv7_pmnc_read(void)
{
        uint32_t val;
        __asm__ volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(val));
        return val;
}

static inline void armv7_pmnc_write(uint32_t val)
{
        val &= ARMV7_PMNC_MASK;
        isb();
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(val));
}

static inline int armv7_pmnc_select_counter(int idx)
{
        uint32_t counter = ARMV7_IDX_TO_COUNTER(idx);
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" : : "r" (counter));
        isb();

        return idx;
}

static inline uint32_t armv7pmu_read_counter(int idx)
{
        uint32_t value = 0;

        if (idx == ARMV7_IDX_CYCLE_COUNTER)
                __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (value));
        else if (armv7_pmnc_select_counter(idx) == idx)
                __asm__ volatile("mrc p15, 0, %0, c9, c13, 2" : "=r" (value));

        return value;
}

static inline void armv7pmu_write_counter(int idx, uint32_t value)
{
        if (idx == ARMV7_IDX_CYCLE_COUNTER)
                __asm__ volatile("mcr p15, 0, %0, c9, c13, 0" : : "r" (value));
        else if (armv7_pmnc_select_counter(idx) == idx)
                __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" : : "r" (value));
}

static inline void armv7_pmnc_write_evtsel(int idx, uint32_t val)
{
        if (armv7_pmnc_select_counter(idx) == idx)
        {
                val &= ARMV7_EVTYPE_MASK;
                __asm__ volatile("mcr p15, 0, %0, c9, c13, 1" : : "r" (val));
        }
}

static inline int armv7_pmnc_enable_counter(int idx)
{
        uint32_t counter = ARMV7_IDX_TO_COUNTER(idx);
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 1" : : "r" (BIT(counter)));
        return idx;
}

static inline int armv7_pmnc_disable_counter(int idx)
{
        uint32_t counter = ARMV7_IDX_TO_COUNTER(idx);
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 2" : : "r" (BIT(counter)));
        return idx;
}

static inline int armv7_pmnc_enable_intens(int idx)
{
        uint32_t counter = ARMV7_IDX_TO_COUNTER(idx);
        __asm__ volatile("mcr p15, 0, %0, c9, c14, 1" : : "r" (BIT(counter)));
        return idx;
}

static inline int armv7_pmnc_disable_intens(int idx)
{
        uint32_t counter = ARMV7_IDX_TO_COUNTER(idx);
        __asm__ volatile("mcr p15, 0, %0, c9, c14, 2" : : "r" (BIT(counter)));
        isb();
        /* Clear the overflow flag in case an interrupt is pending. */
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" : : "r" (BIT(counter)));
        isb();

        return idx;
}

static inline uint32_t armv7_pmnc_getreset_flags(void)
{
        uint32_t val;

        /* Read */
        __asm__ volatile("mrc p15, 0, %0, c9, c12, 3" : "=r" (val));

        /* Write to clear flags */
        val &= ARMV7_FLAG_MASK;
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" : : "r" (val));

        return val;
}

static void armv7pmu_start(void)
{
        unsigned long flags;

        raw_spin_lock_irqsave(&pmu_lock, flags);
        /* Enable all counters */
        armv7_pmnc_write(armv7_pmnc_read() | ARMV7_PMNC_E);
#ifdef NESTED_CHECK
        pmnc_state = 1;
#endif
        raw_spin_unlock_irqrestore(&pmu_lock, flags);
}

static void armv7pmu_stop(void)
{
        unsigned long flags;

        raw_spin_lock_irqsave(&pmu_lock, flags);
        /* Disable all counters */
        armv7_pmnc_write(armv7_pmnc_read() & ~ARMV7_PMNC_E);
#ifdef NESTED_CHECK
        pmnc_state = 0;
#endif
        raw_spin_unlock_irqrestore(&pmu_lock, flags);
}

static uint32_t armv7_read_num_pmnc_events(void)
{
        uint32_t nb_cnt;

        /* Read the nb of CNTx counters supported from PMNC */
        nb_cnt = (armv7_pmnc_read() >> ARMV7_PMNC_N_SHIFT) & ARMV7_PMNC_N_MASK;
        /* Add the CPU cycles counter and return */
        return nb_cnt + 1;
}

static void armv7pmu_reset(void)
{
        uint32_t idx, nb_cnt = 6;

        nb_cnt = armv7_read_num_pmnc_events();
        /* The counter and interrupt enable registers are unknown at reset. */
        for (idx = ARMV7_IDX_CYCLE_COUNTER; idx < nb_cnt; ++idx)
        {
                armv7_pmnc_disable_counter(idx);
                armv7_pmnc_disable_intens(idx);
        }

        /* Initialize & Reset PMNC: C and P bits */
        armv7_pmnc_write(ARMV7_PMNC_P | ARMV7_PMNC_C);
}

perf_stat_t perf_event_stat[PERF_ARRAY_MAX];
__u64 avrg_count[PERF_ARRAY_MAX];
const unsigned int event_type[PERF_EVENT_MAX] =
	{ ARMV7_PERFCTR_CLOCK_CYCLES, ARMV7_PERFCTR_L1_ICACHE_REFILL, ARMV7_PERFCTR_L1_DCACHE_REFILL,
	  ARMV7_PERFCTR_L1_DCACHE_WB, ARMV7_PERFCTR_L2_CACHE_REFILL, ARMV7_PERFCTR_BUS_CYCLES };

//#define FORCE_MEASURE_CLOCK_CYCLE_ONLY		1

static void perf_event_init(void)
{
	memset( &perf_event_stat, 0, sizeof(perf_stat_t)*PERF_ARRAY_MAX );
	memset( &avrg_count, 0, sizeof(__u64)*PERF_ARRAY_MAX);
}

void start_perf_counter(unsigned int event_mask)
{
#ifndef FORCE_MEASURE_CLOCK_CYCLE_ONLY
	int i;
#endif

#ifdef NESTED_CHECK
	if (pmnc_state == 1) {
		printk("[Error] rtk pmu: please review your test code. start_perf_counter() can't be nested.\n");
	}
#endif
	armv7pmu_reset();
	armv7pmu_start();

#ifdef FORCE_MEASURE_CLOCK_CYCLE_ONLY
	armv7_pmnc_write_evtsel(0, ARMV7_PERFCTR_CLOCK_CYCLES);
	armv7_pmnc_enable_counter(0);
#else
 	for(i=0;i<PERF_EVENT_MAX;i++) {
		if (event_mask & (1<<i))
			armv7_pmnc_write_evtsel(i, event_type[i]);
	}

 	for(i=0;i<PERF_EVENT_MAX;i++) {
		if (event_mask & (1<<i))
			armv7_pmnc_enable_counter(i);
	}
#endif
}

int stop_perf_counter(unsigned int sw_index, unsigned int event_mask)
{
	if (sw_index < PERF_ARRAY_MAX) {
#ifdef FORCE_MEASURE_CLOCK_CYCLE_ONLY
		armv7_pmnc_disable_counter(0);
		perf_event_stat[sw_index].executed_num++;
		perf_event_stat[sw_index].acc_count[0] += armv7pmu_read_counter(0);
		perf_event_stat[sw_index].measure_times[0]++;
#else
		int i;
	
		for(i=0;i<PERF_EVENT_MAX;i++) {
			if (event_mask & (1<<i))
				armv7_pmnc_disable_counter(i);
		}
		perf_event_stat[sw_index].executed_num++;
		
		for(i=0;i<PERF_EVENT_MAX;i++)
		{
			if (event_mask & (1<<i)) {
				perf_event_stat[sw_index].acc_count[i] += armv7pmu_read_counter(i);
				perf_event_stat[sw_index].measure_times[i]++;
			}
		}
#endif
	}
	armv7pmu_stop();
	return 0;
}

static int pmu_show_help(struct seq_file *m, void *v)
{
	seq_printf(m, "Usage:\n");
	seq_printf(m, "  echo dump > pmu\n");
	seq_printf(m, "  echo clear > pmu\n");

	return 0;
}

static int pmu_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pmu_show_help, (void *) file);
}

static ssize_t pmu_proc_write(struct file * file, const char __user * userbuf, size_t count, loff_t * off)
{
	char 	tmpbuf[64];
	char	*strptr, *cmd_addr;
	int 	i, j;

	if(count > 64)
		goto errout;

	if (userbuf && !copy_from_user(tmpbuf, userbuf, count)) {
		tmpbuf[count] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		
		if (cmd_addr==NULL)
			goto errout;

		if (!memcmp(cmd_addr, "clear", 5)) {
			perf_event_init();
			printk("rtk pmu: clear perf data.\n");
		} 
		else if (!memcmp(cmd_addr, "dump", 4)) {
			printk("rtk pmu: dump perf data:\n");
			for( i = 0; i < PERF_ARRAY_MAX; i++ )
			{
				if (perf_event_stat[i].executed_num==0)
					continue;

				for( j = 0; j < PERF_EVENT_MAX; j++ ) {
				
					if(perf_event_stat[i].measure_times[j] == 0)
						continue;
					
					avrg_count[i] = div64_u64(perf_event_stat[i].acc_count[j],perf_event_stat[i].measure_times[j]);
				
					printk("index[%d]: acc_count[%d]=%llu, measure_times=%llu, average=%llu\n",
						i, j, perf_event_stat[i].acc_count[j],perf_event_stat[i].measure_times[j], avrg_count[i]);
				}
			}
		}
		else
			goto errout;
	} 
	else {
errout:
		printk("error input!\n");
	}
	return count;
}

static const struct file_operations fops_pmu = {
	.owner		= THIS_MODULE,
	.open		= pmu_proc_open,
	.read		= seq_read,
	.write		= pmu_proc_write,
	.release	= seq_release,
};

int rtk_pmu_proc_init(void)
{
	if (proc_create("pmu", 0, ca_proc_dir, &fops_pmu) == NULL) {
		pr_err("Fail to create proc entry pmu.\n");
		return -ENOMEM;
	}
	return 0;
}

void rtk_pmu_proc_exit(void)
{
	remove_proc_entry("pmu", ca_proc_dir);
}

static int __init rtk_pmu_init(void)
{
	rtk_pmu_proc_init();
	raw_spin_lock_init(&pmu_lock);
#ifdef NESTED_CHECK
	pmnc_state = 0;
#endif
	perf_event_init();
	return 0;
}

static void __exit rtk_pmu_exit(void)
{
	rtk_pmu_proc_exit();
}

EXPORT_SYMBOL(start_perf_counter);
EXPORT_SYMBOL(stop_perf_counter);
module_init(rtk_pmu_init);
module_exit(rtk_pmu_exit);

MODULE_DESCRIPTION ("Realtek PMU Module");

