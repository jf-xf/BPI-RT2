#ifndef _DT_BINDINGS_SOC_TZ_H
/* Reserved memory region for BL31 and BL32 (TruestZone)
 * Case 1: No BL32(OPTEE OS) supoort
 * tz_smc_buf      : 0x0FFD_F000 ~ 0x0FFD FFFF , 0x1000. ( 4KB)
 * tz_pool  : 0x0FFE0000 ~ 0x0FFFFFFF ,128KB

 * Case2: OPTEE_OS Support
 * tz_smc_buf          : 0x0EEF_F000 ~ 0x0EEF_FFFF , 0x1000    ( 4KB)
 * tz_TZ_OPTEE_SHMEM   : 0x0EF0_0000 _ 0x0EFF_FFFF , 0x100000  ( 1MB)
 * tz_pool             : 0x0F00_0000 ~ 0x0FFF_FFFF , 0x1000000 ( 16MB)
 */

/********************************************************
 * tz_smc_buf 
 * For smc call use. Share mem between bl31 and kernel. 
 * Like OTP read/write
 */

#define TZ_SMC_BUF_PADDR_WITHOUT_TEE	0x0EEFF000  //20221228 , discard 0x0FFDF000. Use the smae addr as TEE env.
#define TZ_SMC_BUF_SIZE			0x1000

#define TZ_POOL_PADDR_WITHOUT_TEE	0x0FFC0000
/* 128KB -> 256 KB, 20221228 For BL31 dec*/
#define TZ_POOL_SIZE_WITHOUT_TEE	0x40000
/*******************************************************
 * tz_pool
 * For BL31, BL32
 */
#ifdef CONFIG_OPTEE
#define TZ_POOL_PADDR_WITH_TEE		0x0F000000
/* 16MB */
#define TZ_POOL_SIZE_WITH_TEE		0x1000000

/*****************************************************
 * BL32 Share mem
****************************************************/

#define TZ_OPTEE_SHMEM_PADDR            0x10000000
#define TZ_OPTEE_SHMEM_SIZE             0x100000

#define TZ_SMC_BUF_PADDR_WITH_TEE	0x0EEFF000

#endif

#endif //_DT_BINDINGS_SOC_TZ_H

