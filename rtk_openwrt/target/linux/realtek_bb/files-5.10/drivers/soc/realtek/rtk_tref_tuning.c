#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/arm-smccc.h>
#include <linux/thermal.h>
#include <soc/cortina/cortina-smc.h>
#include <soc/cortina/cortina-soc.h>
#define DEBUG 0
static unsigned int kick_time = 5;
static struct task_struct *pTask;


#if defined(CONFIG_CA8277_THERMAL) || defined(CONFIG_CA8271_THERMAL)
struct thermal_zone_device *thermal_dev = NULL;
/*
  thermal_zone_get_temp: 
  return 50000  -> 50 Celsius
*/
static int elnath_tref_action(void){
	int ret;
	struct arm_smccc_res res;
	int temperature =0;
	int divide=0;
	ret = thermal_zone_get_temp(thermal_dev, &temperature);

	if (ret)
		return ret;

	if(temperature >= 85000)
		divide =4;
	else if(temperature >= 75000)
		divide =2;
	else
		divide =1;

	arm_smccc_smc(RTK_SVC_DDR_TREF_TUNING, 1, temperature, divide, 0, 0, 0, 0, &res);
	ret = (int) res.a0;
#if DEBUG
	printk("[%s %d]res.a0= %d\n", __func__, __LINE__,  ret);
#endif
	return ret;
}

#else
static int elnath_tref_action(void){
	return -1;
}
#endif

/* 8277C and 8277B */
static int kick_rtk_job(void){
	struct arm_smccc_res res;
	unsigned thermal_init = 0;
	int ret = 0;

#if defined(CONFIG_CA8277_THERMAL) || defined(CONFIG_CA8271_THERMAL)
		thermal_init = 1;
#else
		thermal_init = 0;
#endif

	arm_smccc_smc(RTK_SVC_DDR_TREF_TUNING, thermal_init, 0, 0, 0, 0, 0, 0, &res);
	ret = (int) res.a0;
#if DEBUG
	printk("[%s %d]res.a0= %d\n", __func__, __LINE__,  ret);
#endif
	return ret;
}

static int job_routine_thread(void *data)
{
	int ret = 0;
    while(!kthread_should_stop())
    {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(kick_time * HZ);
		if(IS_ELNATH){
			ret = elnath_tref_action();
		}else{//taurus or venus
			ret = kick_rtk_job();
		}
		if(ret == -1){
				printk("rtk_tref_tuning: BL31 not implement DDR_TREF_TUNING\n");
				do_exit(0);
		}
    }

	return 0;
}

static void kthread_job(void){
	if( pTask == NULL){
		pTask = kthread_create(job_routine_thread, NULL, "rtk_tref_tuning");
		if(IS_ERR(pTask))
		{
			printk("[Kthread : rtk_tref] init failed %ld!\n", PTR_ERR(pTask));
		}else{
			wake_up_process(pTask);
			printk("[Kthread : rtk_tref] init complete!\n");
		}
	}
}

int __init rtk_tref_init(void)
{	
	if(IS_ELNATH){
#if defined(CONFIG_CA8277_THERMAL) || defined(CONFIG_CA8271_THERMAL)
		thermal_dev = thermal_zone_get_zone_by_name("cpu-thermal");
		if (IS_ERR(thermal_dev)) {
			printk("%s: Unable to get thermal zone for tuning\n", __func__);
			return PTR_ERR(thermal_dev);
		}
#else
		return -1;
#endif
	}
	kthread_job();

	return 0;
}

void __exit rtk_tref_exit(void)
{
	kthread_stop(pTask);
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek DDR tref tuning module");
MODULE_AUTHOR("Alan <alan.chao@realtek.com>");
device_initcall_sync(rtk_tref_init);
module_exit(rtk_tref_exit);