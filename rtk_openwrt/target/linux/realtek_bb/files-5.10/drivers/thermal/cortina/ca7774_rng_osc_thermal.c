/*
 * Cortina-Access CA7774 Rin Oscillator Thermal Sensor
 *
 * Copyright (C) 2018 Cortina Access, Inc.
 *		http://www.ca7774_rng_osc-access.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/cpufreq.h>
#include <linux/time.h>
#include <linux/cputime.h>
#include <linux/kernel_stat.h>
#include <dt-bindings/clock/ca7774-clock.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <soc/cortina/cortina-clk-ca7774-soc.h>

#if 0
#define DEBUG_CA7774_RNG_OSC_THERMAL  1
#endif

#define GET_CPU_LOAD 1

#ifdef DEBUG_CA7774_RNG_OSC_THERMAL
    #define ca7774_rng_osc_dbg(...)     printk("ca7774_rng_osc_thermal: " __VA_ARGS__ )
#else
    #define ca7774_rng_osc_dbg(...)     ({})
#endif /* DEBUG_CA7774_RNG_OSC_THERMAL */

#define GLOBAL_TIMEOUT_VALUE_US   10
#define G3_SAMPLES 10
#define PE0_SAMPLES 10
#define PE1_SAMPLES 10

#define DSS_CNTL   0x0
#define DSS_STATUS 0x4

/* we dont want to support udelay in the kernel */
#define USLEEP(x) mdelay(1)

/* parts per xunit operations */
#define XUNIT ((int64_t) 1000000)
#define XUNIT_DIVIDER	(1000000/XUNIT)
#define MILLI_CELCIUS_UNIT 1000
#define XUNIT_MCELCIUS_DIVIDER (XUNIT/MILLI_CELCIUS_UNIT)

#define USE_PREPROCESSOR_FOR_MATH 1

#ifdef USE_PREPROCESSOR_FOR_MATH 
#define PPM_TO_XUNIT(x) ((int64_t) (x/XUNIT_DIVIDER))
#define SINGLE_TO_PPM(x) ((int64_t) (x*1000000))
#define SINGLE_TO_XUNIT(x) ((int64_t) (x*XUNIT))
#define PERCENT_TO_XUNIT(x) ((int64_t) ((x*XUNIT)/100) )
#define PPM_TO_SINGLE(x) ((int) (x/1000000))
#define XUNIT_TO_SINGLE(x) ((int64_t) (x/XUNIT))
#define XUNIT_TO_MILLI_CELCIUS(x) ((int) (x/XUNIT_MCELCIUS_DIVIDER))
#define MULT_PPM(x,y) ((x*y)/1000000)
#define MULT_XUNIT(x,y) ((x*y)/XUNIT)
#define DIV_PPM(num,div) (num/div)
#define DIV_XUNIT(num,div) (num/div)
#else
#define PPM_TO_XUNIT(x) ppm_to_xunit(x)
#define SINGLE_TO_XUNIT(x) single_to_xunit(x)
#define XUNIT_TO_SINGLE(x) xunit_to_single(x)
#define PPM_TO_SINGLE(x) ppm_to_single(x)
#define MULT_XUNIT(x,y) mult_xunit(x,y)
#define DIV_XUNIT(num,div) div_xunit(num,div)


static int64_t single_to_xunit(int single)
{
	return((int64_t)single*XUNIT);
}

static int64_t ppm_to_xunit(int64_t ppm)
{
	return(ppm/XUNIT_DIVIDER);
}

static int xunit_to_single(int64_t xunit)
{
	return((int) (xunit/XUNIT));
}

static int ppm_to_single(int64_t ppm)
{
	return((int) (ppm/1000000));
}
static int64_t mult_xunit(int64_t x, int64_t y)
{
	return((x*y)/XUNIT);
}

static int64_t div_xunit(int64_t num, int64_t div)
{
	return(num/div);

}
#endif /* USE_PREPROCESSOR_FOR_MATH */

struct cpu_stats  
{
	u64		   		prev_wall_time;
	u64				prev_idle_time;
};


/* Cortina Thermal Sensor Dev Structure */
struct ca7774_rng_osc_thermal_priv {
	struct thermal_zone_device      *tz_dev;
	void __iomem 			*g3_dss_base;
	void __iomem 			*pe0_dss_base;
	void __iomem 			*pe1_dss_base;
	struct clk 			**clks;
	struct cpu_stats		cpu_stats[4];
	u64				cpu_load;
	u64 				cpu_load_wait_cputime;
	u64				accumulated_wall_time;
	u64				accumulated_idle_time;
	
};

unsigned int ca7774_rng_osc_read32(void __iomem *reg, u32 offset)
{
	unsigned int value;
	value = readl(reg + offset);
#if 0
	ca7774_rng_osc_dbg("%s reg @ 0x%llx read  ----> 0x%08x\n", __func__, (u64) reg + offset, value);
#endif
	return value;
}
void ca7774_rng_osc_write32(void __iomem *reg, u32 offset, u32 data)
{
#if 0
	ca7774_rng_osc_dbg("%s reg @ 0x%llx write <---- 0x%08x\n", __func__, (u64) reg + offset, data);
#endif
	writel(data, reg + offset);
}




/* temp code from cputool */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Func:        sample_average_top_ring_oscillator and wire
// Purpose:     Read the ring oscillator that we care about on a particular wire and sample and average its value
//
// Parameters:  
//  unsigned int ro_index:      For the top ring oscillator it starts at 1 and ends at ring oscillator 6 
//  unsigned int wire_index:    Wire index - can be 0 or 1, perhaps the IP has these at different lengths/inverter counts?
//  unsigned int sample_count:  Number of sample to take - sample are done back to back waiting only for completion:w
//  
// Returns:
//  unsigned int :              Average count of cycles in the Ring Oscillator count period.
//                              Generally as the temperature increases (when >0C) the ring oscillator get slower and the
//                              count remains larger and cannot count as far down.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int sample_average_top_ring_oscillator(struct ca7774_rng_osc_thermal_priv *priv, unsigned int ro_index, unsigned int wire_index, unsigned int sample_count) {
    unsigned int sample_index;
    unsigned int averaged_sample=0;
    unsigned int val_set_not_on=0;
    unsigned int val_set_on=0;
    unsigned int check_done=0;
    unsigned int timeout=0;
    uint64_t value;
    //unsigned int *ptr;
    void __iomem *sensor;
    unsigned int dss_count=0xfffff;         // dss_count is the pre-load value for the Ring Oscillator counter
                                            // This ring oscillator is unique in its usage of this, others always
                                            // pre-load an internal value


    sensor = priv->g3_dss_base;

    for (sample_index=0;sample_index<sample_count;sample_index++) {
        
            val_set_not_on=ro_index*2+16*wire_index+dss_count*32+1;
            val_set_on=val_set_not_on+(1<<25);

        //ca7774_rng_osc_dbg("Value at GLOBAL_DSS_0_CNTL:%x\n",*(gpvalue+GLOBAL_DSS_0_CNTL));

            // Put DSS-RO in reset
        ca7774_rng_osc_write32(sensor, DSS_CNTL,0);

            // Take the DSS out of reset
        ca7774_rng_osc_write32(sensor, DSS_CNTL,1);


        // Pre-load and select ring oscillator index and wire 
        ca7774_rng_osc_write32(sensor, DSS_CNTL,val_set_not_on);

        // Turn on the selected ring oscillator index/wire
        ca7774_rng_osc_write32(sensor, DSS_CNTL,val_set_on);

        check_done=ca7774_rng_osc_read32(sensor, DSS_STATUS);
        //ca7774_rng_osc_dbg("Check Value:%x\n",check_done);
        check_done=check_done & 1;
        timeout=0;
        while ((check_done==0) && (timeout<GLOBAL_TIMEOUT_VALUE_US)) {
            check_done=ca7774_rng_osc_read32(sensor, DSS_STATUS) & 1;
            msleep(1);
            timeout++;
            //ca7774_rng_osc_dbg("Timeout... %d\n",timeout);

        }
        value=(ca7774_rng_osc_read32(sensor, DSS_STATUS) & 0x1fffff)>>1;
        //ca7774_rng_osc_dbg("%x\n",value);
        averaged_sample+=value;
    }

    averaged_sample=(int)(averaged_sample/sample_count);

        // Put DSS-RO in reset
    ca7774_rng_osc_write32(sensor, DSS_CNTL,0);

    return averaged_sample;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Func:        sample_average_pe_ring_oscillator and wire
// Purpose:     Read the ring oscillator that we care about on a particular wire and sample and average its value
//
// Parameters:  
//  unsigned int ro_index:      For the top ring oscillator it starts at 1 and ends at ring oscillator 6 
//  unsigned int wire_index:    Wire index - can be 0 or 1, perhaps the IP has these at different lengths/inverter counts?
//  unsigned int pe_number:     Which PE Ring oscillator do we want to choose - 0 or 1
//  
// Returns:
//  unsigned int :              Average count of cycles in the Ring Oscillator count period.
//                              Generally as the temperature increases (when >0C) the ring oscillator get slower and the
//                              count remains larger and cannot count as far down.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int sample_average_pe_ring_oscillator(struct ca7774_rng_osc_thermal_priv *priv, unsigned int ro_index, unsigned int wire_index,unsigned int sample_count, int pe_number) {
    unsigned int sample_index;
    unsigned int averaged_sample=0;
    unsigned int val_set_not_on=0;
    unsigned int val_set_on=0;
    unsigned int check_done=0;
    unsigned int timeout=0;
    unsigned int value;
    void __iomem *sensor;

    
    if(pe_number == 1)
	sensor = priv->pe1_dss_base;
    else
	sensor = priv->pe0_dss_base;

    for (sample_index=0;sample_index<sample_count;sample_index++) {
        
            val_set_not_on=ro_index*2+16*wire_index+1;
            val_set_on=val_set_not_on+(1<<25);
	

            // Put PE-RO in reset
        ca7774_rng_osc_write32(sensor, DSS_CNTL,0);

            // Take the DSS out of reset
        ca7774_rng_osc_write32(sensor, DSS_CNTL,1);


        // Pre-load and select ring oscillator index and wire 
        ca7774_rng_osc_write32(sensor, DSS_CNTL,val_set_not_on);

        // Turn on the selected ring oscillator index/wire
        ca7774_rng_osc_write32(sensor, DSS_CNTL,val_set_on);

        check_done=ca7774_rng_osc_read32(sensor, DSS_STATUS);
        check_done=check_done & 1;
        timeout=0;
        while ((check_done==0) && (timeout<GLOBAL_TIMEOUT_VALUE_US)) {
            check_done=ca7774_rng_osc_read32(sensor, DSS_STATUS) & 1;
            USLEEP(10);
            timeout++;
            //ca7774_rng_osc_dbg("Timeout... %d\n",timeout);

        }
        value=(ca7774_rng_osc_read32(sensor, DSS_STATUS) & 0x1fffff)>>1;
        //ca7774_rng_osc_dbg("%x\n",value);
        averaged_sample+=value;
    }
    averaged_sample=(int)(averaged_sample/sample_count);

        // Put DSS-RO in reset
    ca7774_rng_osc_write32(sensor, DSS_CNTL, 0);

    return averaged_sample;
}



/* args select 0=greater fit, 1 log fit, debug 1=printk, eaxi_enabled 0=g3 */
int64_t read_g3_die_temperature(struct ca7774_rng_osc_thermal_priv *priv, unsigned int select, unsigned int debug, unsigned int eaxi_enabled)
{
    
    unsigned int count,sample_count,pe0_count,pe1_count, arm_cpu_speed;
    int64_t linear_fit_value,linear_fit_pe0, linear_fit_pe1, averaged_dss, cpu_load, adjusted_load_averaged_dss, greater_load;

#ifdef DSS_LOG_FIT /* no log fit in kernel */
    int64_t log_fit_value;
#endif /* no log fit in kernel */
    /* ppt stands for parts per thousand */
    int64_t count_xunit;
    int64_t pe0_count_xunit;
    int64_t pe1_count_xunit;
    int64_t cci_speed;
    struct clk *cpu_clk; 

    cpu_clk = priv->clks[CA7774_CLK_CPU];


    sample_count=G3_SAMPLES;
    count=(int64_t) sample_average_top_ring_oscillator(priv, 5,1,G3_SAMPLES);
    pe0_count=(int64_t) sample_average_pe_ring_oscillator(priv, 5,1,PE0_SAMPLES,0);
    pe1_count=(int64_t) sample_average_pe_ring_oscillator(priv, 5,1,PE1_SAMPLES,1);
    
    if (debug) {
        ca7774_rng_osc_dbg("%s TOP DSS Count averaged over %d sample of R1-0:%d - 0x%8.8x\n", __func__, sample_count,count,count);
        ca7774_rng_osc_dbg("PE0 DSS Count averaged over %d sample of R1-0:%d - 0x%8.8x\n",sample_count,pe0_count,pe0_count);
        ca7774_rng_osc_dbg("PE1 DSS Count averaged over %d sample of R1-0:%d - 0x%8.8x\n",sample_count,pe1_count,pe1_count);
    }
#if 0
    arm_cpu_speed=get_arm_speed(debug);
#else
    arm_cpu_speed = clk_get_rate(cpu_clk)/1000000;
    if (debug)
    ca7774_rng_osc_dbg("%s Current cpu freq reported at %d Mhz\n", __func__, arm_cpu_speed);
#endif /* 0 */
    
    //Linear fit
    //=(P26-10887)/8.107
    //linear_fit_value=((double)count-10887.0)/8.107;
    //linear_fit_value=((double)count-10887.0)/8.107;
    //linear_fit_value=(double)count*0.0592-613.1;
    // TOP R5-W1

#if 0 /* 400Mhz has now 1/2 clock too */
    if (arm_cpu_speed==400)
        cci_speed=arm_cpu_speed;
    else
#else
        cci_speed=arm_cpu_speed/2;
#endif /* 400Mhz has now 1/2 clock too */
         
    /* to keep floating point precision in the division
     *  we multiple by xunit, to keep up the 1/xunit of precision
     *  when we perform division.
     */
    count_xunit=(int64_t) MULT_XUNIT((SINGLE_TO_XUNIT(count)), (DIV_XUNIT((SINGLE_TO_XUNIT(600)),cci_speed)));
    if (debug)
    ca7774_rng_osc_dbg("%s count_xunit %lld \n", __func__, count_xunit);

    linear_fit_value=MULT_XUNIT(count_xunit,(PPM_TO_XUNIT(66000))) - PPM_TO_XUNIT(551260000);

#ifdef DSS_LOG_FIT /* no log fit in kernel */
    //Logarithmic fit
    //=EXP((P27-9387.4)/487.22)
    // y = 0.0022e0.0011x
    // Skewed exponent so high end is more hockeystick
    log_fit_value=MULT_XUNIT((PPM_TO_XUNIT(2200)),(exp(DIV_XUNIT(count_xunit,(PPM_TO_XUNIT(1110))))));
#endif /* no log fit in kernel */
         
    // PE0 graph linear function 
    // y = 0.0378x - 511.28
    //linear_fit_pe0=(double)pe0_count*0.0378-511.28;
    pe0_count_xunit=SINGLE_TO_XUNIT(pe0_count); //*(600.0/cci_speed_xunit);
    linear_fit_pe0=MULT_XUNIT(pe0_count_xunit,(PPM_TO_XUNIT(43500))) - PPM_TO_XUNIT(478490000);
         
    // PE1 graph linear function 
    // y = 0.0379x - 511.39
    //linear_fit_pe1=(double)pe1_count*0.0379-511.39;
    pe1_count_xunit=SINGLE_TO_XUNIT(pe1_count); //*(600.0/cci_speed_xunit);
    linear_fit_pe1=MULT_XUNIT(pe1_count_xunit,(PPM_TO_XUNIT(43500))) - PPM_TO_XUNIT(478070000);
                        
    averaged_dss=((2*linear_fit_value)+linear_fit_pe0)/3;

#ifdef GET_CPU_LOAD  /* fake load to 1 */
    if (debug) ca7774_rng_osc_dbg("Calculating CPU load across 4x cores for 5 seconds...\n");

    /* cpu_load is already in percent, so we much convert */
    cpu_load=PERCENT_TO_XUNIT(priv->cpu_load);
#else
    /* fake the load */
    cpu_load=PERCENT_TO_XUNIT(10); /* 10 % */
#endif /* GET_CPU_LOAD */


    adjusted_load_averaged_dss=averaged_dss+MULT_XUNIT(SINGLE_TO_XUNIT(15),(SINGLE_TO_XUNIT(1)-cpu_load));

#ifdef DSS_LOG_FIT /* no log fit in kernel */
    if (adjusted_load_averaged_dss>log_fit_value)
        greater_load=adjusted_load_averaged_dss;
    else         
        greater_load=log_fit_value;
#else
        greater_load=adjusted_load_averaged_dss;
#endif /* DSS_LOG_FIT - no log fit in kernel */
                             
        ca7774_rng_osc_dbg("Current CPU Load Across all cores: %d percent\n", (int) XUNIT_TO_SINGLE(cpu_load * 100));

    if (debug) {
#ifdef DSS_LOG_FIT /* no log fit in kernel */
        ca7774_rng_osc_dbg("TOP DSS Exponent fit temperature based on calibration data is:\t\t %lld parts per %lld\n",log_fit_value, XUNIT);
#endif /*  DSS_LOG_FIT */
        ca7774_rng_osc_dbg("TOP DSS Linear fit temperature based on calibration data is:\t\t %lld parts per %lld\n",linear_fit_value, XUNIT);
        ca7774_rng_osc_dbg("PE0 DSS Linear fit temperature based on calibration data is:\t\t %lld parts per %lld\n",linear_fit_pe0, XUNIT);
        ca7774_rng_osc_dbg("PE1 DSS Linear fit temperature based on calibration data is:\t\t %lld parts per %lld\n",linear_fit_pe1, XUNIT);
        ca7774_rng_osc_dbg("Average DSS over all fit temperature is:\t\t\t\t %lld parts per %lld\n",averaged_dss, XUNIT);
        ca7774_rng_osc_dbg("Load adjusted and average DSS over all fit temperature is:\t\t %lld parts per %lld\n\n",adjusted_load_averaged_dss, XUNIT);
        ca7774_rng_osc_dbg("Conservative curve at load or highest fit of temperature :\t\t %lld parts per %lld\n",greater_load, XUNIT);
    }
    if (eaxi_enabled)
#if 0
        ca7774_rng_osc_dbg("Read Saturn temperature is                               \t\t %d\n",read_saturn_temp_hgu_only());
#else
        ca7774_rng_osc_dbg("Read of Saturn temperature not supported in this driver\n");

#endif

#ifdef DSS_LOG_FIT /* no log fit in kernel */
    if (select==0)
        return greater_load;
    else
        return log_fit_value;
#else
        return greater_load;
#endif /* DSS_LOG_FIT */
}

/* end of temp code from cputool */


static int ca7774_rng_osc_get_temp(void *data, int *temp)
{
	struct ca7774_rng_osc_thermal_priv *priv = data;
	uint64_t temp_xunit;
	unsigned int j;
        u64 cur_wall_time, cur_idle_time;
        u64 delta_wall_time, delta_idle_time;
	int cpu_percent_load;
	int all_cpu_load_avg;
	

	/* args select 0=greater fit, 1 log fit, debug 1=printk, eaxi_enabled 0=g3 */
	/* returned value is in parts per million */
	temp_xunit = read_g3_die_temperature(priv, 0, 0, 0);

	/* thermal API expects temperture in millicelius */
	*temp = (int) XUNIT_TO_MILLI_CELCIUS(temp_xunit);
	if(0)
        ca7774_rng_osc_dbg("G3 Temperature is %d millicelcius\n", *temp);

	j = 0;
	for_each_online_cpu(j) 
	{

 	  cur_idle_time = get_cpu_idle_time(j, &cur_wall_time, 0);
	  delta_wall_time = cur_wall_time - priv->cpu_stats[j].prev_wall_time;
	  delta_idle_time = cur_idle_time - priv->cpu_stats[j].prev_idle_time;
          cpu_percent_load =  (int) (((delta_wall_time - delta_idle_time) * 100)/delta_wall_time);
	  if(cpu_percent_load < 0)
 	   cpu_percent_load = 0;
	   
#if 0
          ca7774_rng_osc_dbg("CPU%d Current CPU Wall Time %lld Idle Time %lld\n", j, cur_idle_time, cur_wall_time);
          ca7774_rng_osc_dbg("CPU%d Prev CPU Wall Time %lld Idle Time %lld\n", j, 
		priv->cpu_stats[j].prev_idle_time, priv->cpu_stats[j].prev_wall_time);

          ca7774_rng_osc_dbg("CPU%d Delta CPU Wall Time %lld Idle Time %lld\n\n", j, delta_wall_time, delta_idle_time);
          ca7774_rng_osc_dbg("CPU%d instant load %d\n", j, cpu_percent_load);
#endif
	  priv->accumulated_wall_time += delta_wall_time;
	  priv->accumulated_idle_time += delta_idle_time;

	  priv->cpu_stats[j].prev_idle_time = cur_idle_time;
	  priv->cpu_stats[j].prev_wall_time = cur_wall_time;

        }
	  if(priv->accumulated_wall_time/4 >=  priv->cpu_load_wait_cputime)
	    {
	  	all_cpu_load_avg =
                  (int) (((priv->accumulated_wall_time - priv->accumulated_idle_time) * 100)/priv->accumulated_wall_time);
		/* reset the accumulated counters */
          	ca7774_rng_osc_dbg("accumulated wall time: %lld\n", priv->accumulated_wall_time); 
          	ca7774_rng_osc_dbg("ALL CPU load average %d after %lld cpu wall clocks\n", 
		  all_cpu_load_avg, priv->cpu_load_wait_cputime);

		/* set the cpu load average over the period cpu_load_wait_cputime wall clocks */
		priv->cpu_load = all_cpu_load_avg;

		priv->accumulated_wall_time = 0;
		priv->accumulated_idle_time = 0;
	    }


	return 0;
}

static struct thermal_zone_of_device_ops ops = {
	.get_temp = ca7774_rng_osc_get_temp,
};

static const struct of_device_id ca7774_rng_osc_thermal_id_table[] = {
	{.compatible = "cortina,ca7774_rng_osc_thermal_sensor"},
	{}
};

static int ca7774_rng_osc_thermal_probe(struct platform_device *pdev)
{
	struct thermal_zone_device *thermal = NULL;
	struct ca7774_rng_osc_thermal_priv *priv;
	struct resource res;
	int rc;
	unsigned int i;
	struct device_node *np = pdev->dev.of_node;
	int cpu_load_avg_sec;


	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

        dev_set_drvdata(&pdev->dev, priv);

	/* read in parameters from DT */
        for (i = 0; i <= 2; i++) {
                void __iomem *map_res;
                rc = of_address_to_resource(np, i, &res);
                if (rc != 0) {
                        if (i == 0) {
                                pr_err("no DSS base specified for %s\n", 
                                        np->full_name);
                                return -ENOSYS;
                        }
                        break;
                }
                map_res = of_iomap(np, i);
                if (map_res == NULL) {
                        pr_err("Unable to map resource %d for %s\n",
                                i, np->full_name);
                        return -ENOSYS;
                }

        	ca7774_rng_osc_dbg("%s %s phy base: 0x%llx\n", __func__, res.name, res.start);

                if (strcmp(res.name, "g3-dss") == 0)
                        priv->g3_dss_base = map_res;
                else if (strcmp(res.name, "pe0-dss") == 0) 
                        priv->pe0_dss_base = map_res;
                else if (strcmp(res.name, "pe1-dss") == 0) 
                        priv->pe1_dss_base = map_res;
        }

        ca7774_rng_osc_dbg("%s g3 dss va base: 0x%llx\n", __func__, (u64) priv->g3_dss_base);
        ca7774_rng_osc_dbg("%s pe0 dss va base: 0x%llx\n", __func__, (u64) priv->pe0_dss_base);
        ca7774_rng_osc_dbg("%s pe1 dss va base: 0x%llx\n", __func__, (u64) priv->pe1_dss_base);

	/* get a hold of the soc clocks */
	priv->clks = cortina_clk_get_ca7774_soc_clocks();

	/* average load of 5 sec */
	cpu_load_avg_sec = 5;

	/* what is this? why is wall clock about 1,500,000?? per sec */
	priv->cpu_load_wait_cputime = 1500000 * cpu_load_avg_sec;


        priv->tz_dev = thermal_zone_of_sensor_register(&pdev->dev, 0, (void *) priv, &ops);

	if (IS_ERR(priv->tz_dev)) {
		dev_err(&pdev->dev, "Failed to register thermal zone sensor\n");
		return PTR_ERR(thermal);
	}


	return 0;
}

static int ca7774_rng_osc_thermal_exit(struct platform_device *pdev)
{
	struct ca7774_rng_osc_thermal_priv *priv = dev_get_drvdata(&pdev->dev);

	thermal_zone_of_sensor_unregister(&pdev->dev, priv->tz_dev);

	return 0;
}

MODULE_DEVICE_TABLE(of, ca7774_rng_osc_thermal_id_table);

static struct platform_driver ca7774_rng_osc_thermal_driver = {
	.probe = ca7774_rng_osc_thermal_probe,
	.remove = ca7774_rng_osc_thermal_exit,
	.driver = {
		   .name = "ca7774_rng_osc_thermal",
		   .of_match_table = ca7774_rng_osc_thermal_id_table,
		   },
};

module_platform_driver(ca7774_rng_osc_thermal_driver);

MODULE_DESCRIPTION("Cortina-Access CA7774 (G3) Ring Oscillator thermal driver");
MODULE_LICENSE("GPL");
