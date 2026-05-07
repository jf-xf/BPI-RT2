#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/param.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/if_pppox.h>

#include "rtk_fc_ipsec.h"
#include "rtk_fc_helper.h"
#include "rtk_fc_driver.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_assistant.h"

#include <linux/crypto.h>

#include <crypto/hash.h>
#include <linux/scatterlist.h>
#include <net/xfrm.h>
#include <crypto/aead.h>
#include <crypto/internal/geniv.h>
#if defined(CONFIG_RTK_PTOOL_CPU_PERF)
#include <linux/ptool.h>
#endif

//#define IPSEC_DEBUG 0
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
      #undef __devexit_p
      #undef __devexit
      #undef __devinit
      #undef __devinitconst
      #define __devexit_p(x) x
      #define __devexit
      #define __devinit
      #define __devinitconst
#endif
//#define IPSEC_IPI_DEBUG 1



rtk_fc_cryptoEngine_dest_desc_t aes_cbc_decrypt_dest_desc 			= {0};
rtk_fc_cryptoEngine_source_desc_t aes_cbc_decrypt_cmmd_setting_src	= {0};
rtk_fc_cryptoEngine_command_t 	aes_cbc_decrypt_cmmd_setting		= {0};
rtk_fc_cryptoEngine_source_desc_t des_cbc_decrypt_cmmd_setting_src	= {0};
rtk_fc_cryptoEngine_command_t 	des_cbc_decrypt_cmmd_setting		= {0};
rtk_fc_cryptoEngine_source_desc_t aes_cbc_decrypt_iv_src			= {0};
rtk_fc_cryptoEngine_source_desc_t aes_cbc_decrypt_data_src			= {0};
rtk_fc_cryptoEngine_source_desc_t aes_cbc_decrypt_data_padding_src	= {0};
rtk_fc_cryptoEngine_source_desc_t des_cbc_decrypt_data_src			= {0};
rtk_fc_cryptoEngine_source_desc_t des_cbc_decrypt_data_padding_src	= {0};


rtk_fc_cryptoEngine_dest_desc_t aes_cbc_encrypt_dest_desc 			= {0};
rtk_fc_cryptoEngine_dest_desc_t aes_cbc_encrypt_dest_desc_icv 		= {0};
rtk_fc_cryptoEngine_dest_desc_t aes_cbc_encrypt_dest_desc_icv_pad	= {0};

rtk_fc_cryptoEngine_source_desc_t aes_cbc_encrypt_cmmd_setting_src	= {0};
rtk_fc_cryptoEngine_command_t 	aes_cbc_encrypt_cmmd_setting		= {0};
rtk_fc_cryptoEngine_command_t 	aes_cbc_encrypt_cmmd_setting_hmacmd5= {0};
rtk_fc_cryptoEngine_source_desc_t  aes_cbc_encrypt_data_src			= {0};


rtk_fc_cryptoEngine_source_desc_t des_cbc_encrypt_cmmd_setting_src	= {0};
rtk_fc_cryptoEngine_command_t 	des_cbc_encrypt_cmmd_setting		= {0};
rtk_fc_cryptoEngine_command_t 	des_cbc_encrypt_cmmd_setting_hmacmd5= {0};
rtk_fc_cryptoEngine_source_desc_t  des_cbc_encrypt_data_src			= {0};


rtk_fc_cryptoEngine_source_desc_t aes_gcm_encrypt_cmmd_setting_src     = {0};
rtk_fc_cryptoEngine_command_t  aes_gcm_encrypt_cmmd_setting            = {0};
rtk_fc_cryptoEngine_source_desc_t aes_gcm_decrypt_cmmd_setting_src     = {0};
rtk_fc_cryptoEngine_command_t  aes_gcm_decrypt_cmmd_setting            = {0};
rtk_fc_cryptoEngine_source_desc_t aes_gcm_encrypt_iv_src               = {0};
//rtk_fc_cryptoEngine_source_desc_t aes_gcm_encrypt_aead_src           = {0};


struct device *rtk_fc_crypto_dev = NULL;
uint32_t iv_dma[RTK_FC_HW_MAX_JOBS];
uint32_t cmdsetting_dma[RTK_FC_HW_MAX_JOBS];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)

u8 *aead_ptr[RTK_FC_HW_MAX_JOBS] = {0};
u8 *iv_ptr[RTK_FC_HW_MAX_JOBS] = {0};
rtk_fc_cryptoEngine_command_t *cmdsetting_ptr[RTK_FC_HW_MAX_JOBS];
#else
u8 aead_ptr[RTK_FC_HW_MAX_JOBS][12] = {0};
u8 iv_ptr[RTK_FC_HW_MAX_JOBS][16] = {0};
rtk_fc_cryptoEngine_command_t cmdsetting_ptr[RTK_FC_HW_MAX_JOBS] = {0};
#endif

rtk_fc_crypto_key_usage_t key_usage[RTK_FC_SPEC_XFRM_INFO_MAX_NUM];
rtk_fc_crypto_key_usage_t key_pad_usage[RTK_FC_SPEC_XFRM_INFO_MAX_NUM];

void _rtk_fc_ipsec_dump_key_usage(void)
{
	int i;
	printk("==================KEY=======================\n");
	printk("Valid: ");
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
	
		if(key_usage[i].valid==0)
		{
			printk("[%d] ",i);
		}
	}
	printk("\n");
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
	
		if(key_usage[i].valid==1)
		{
			printk("[%d](size:%d): \n",i,key_usage[i].key_size);
			rtk_fc_hexdump(key_usage[i].key, key_usage[i].key_size);
			printk("\n");
		}
	}
	printk("==================HASH KEY===================\n");
	printk("Valid: ");
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
	
		if(key_pad_usage[i].valid==0)
		{
			printk("[%d] ",i);
		}
	}
	printk("\n");

}


static void _rtk_fc_ipsec_descriptor_quick_init(void)
{
	rtk_fc_cryptoEngine_command_t *cmd_ptr = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	int i;
#endif	
	
	/*aes cbc mix mode decrypt dest-descriptor*/	
	aes_cbc_decrypt_dest_desc.ctrl.type2.wrd 		= 0;
	aes_cbc_decrypt_dest_desc.ctrl.type2.bits.ws	= 1;
	aes_cbc_decrypt_dest_desc.ctrl.type2.bits.enc 	= 1;
	aes_cbc_decrypt_dest_desc.ctrl.type2.bits.fs	= 1;
	aes_cbc_decrypt_dest_desc.ctrl.type2.bits.ls	= 1;

	/*aes cbc mix mode decrypt command setting src-descriptor*/
	cmd_ptr = &aes_cbc_decrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.he = 1;
	cmd_ptr->wrd0.bits.icvtl	= 0x14;
	cmd_ptr->wrd0.bits.cm = 0x1;
	cmd_ptr->wrd0.bits.ces = 0;
	cmd_ptr->wrd0.bits.ce = 0;
	cmd_ptr->wrd1.bits.cabs = 1;


	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.ap = 0x00;


	/*des cbc mix mode decrypt command setting src-descriptor*/
	cmd_ptr = &des_cbc_decrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.he = 1;
	cmd_ptr->wrd0.bits.icvtl	= 0x14;
	cmd_ptr->wrd0.bits.cm = 0x1;
	cmd_ptr->wrd0.bits.ces = 1;  //RTK_CES_DES
	cmd_ptr->wrd0.bits.ce = 0;
	cmd_ptr->wrd1.bits.cabs = 1;


	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.ap = 0x00;


	/*aes cbc mix mode decrypt IV src-descriptor*/
	aes_cbc_decrypt_iv_src.ctrl.type1.wrd = 0;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.rs	= 1;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.fs	= 1;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.ls	= 0;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.sk 	= 1;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.pk 	= 1;

	/*aes cbc mix mode decrypt data src-descriptor*/

	aes_cbc_decrypt_data_src.ctrl.type2.wrd 	= 0;
	aes_cbc_decrypt_data_src.ctrl.type2.bits.rs	= 1;
		
	aes_cbc_decrypt_data_padding_src.ctrl.type2.wrd 	= 0;
	aes_cbc_decrypt_data_padding_src.ctrl.type2.bits.rs	= 1;
	aes_cbc_decrypt_data_padding_src.ctrl.type2.bits.ls	= 1;
	aes_cbc_decrypt_data_padding_src.sdbp = fc_db.ipsec_addr.ipsec_crypt_padding_dma;
			
	
	/*des cbc mix mode decrypt data src-descriptor*/

	des_cbc_decrypt_data_src.ctrl.type2.wrd 	= 0;
	des_cbc_decrypt_data_src.ctrl.type2.bits.rs	= 1;
		
	des_cbc_decrypt_data_padding_src.ctrl.type2.wrd 	= 0;
	des_cbc_decrypt_data_padding_src.ctrl.type2.bits.rs	= 1;
	des_cbc_decrypt_data_padding_src.ctrl.type2.bits.ls	= 1;
	des_cbc_decrypt_data_padding_src.sdbp = fc_db.ipsec_addr.ipsec_crypt_padding_dma;

	
	/*aes cbc mix mode encrypt data dest-descriptor*/
	aes_cbc_encrypt_dest_desc.ctrl.type2.wrd 		= 0;
	aes_cbc_encrypt_dest_desc.ctrl.type2.bits.ws  	= 1;
    aes_cbc_encrypt_dest_desc.ctrl.type2.bits.enc 	= 1;
    aes_cbc_encrypt_dest_desc.ctrl.type2.bits.fs  	= 1;
    aes_cbc_encrypt_dest_desc.ctrl.type2.bits.ls  	= 0;

	/*aes cbc mix mode encrypt ICV dest-descriptor*/
	
	aes_cbc_encrypt_dest_desc_icv.ctrl.type1.wrd 		= 0;
	aes_cbc_encrypt_dest_desc_icv.ctrl.type1.bits.ws 	= 1;
	aes_cbc_encrypt_dest_desc_icv.ctrl.type1.bits.enc 	= 0;
		
	
	aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.wrd 		= 0;
	aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.bits.ws 	= 1;
	aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.bits.enc 	= 0;
	aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.bits.ls 	= 1;
	aes_cbc_encrypt_dest_desc_icv_pad.ddbp 					= fc_db.ipsec_addr.ipsec_icv_padding_dma;

	/*aes cbc mix mode encrypt cmd setting(hmac sha-1) src-descriptor*/	
		
	cmd_ptr = &aes_cbc_encrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.em = 3;
    cmd_ptr->wrd0.bits.he = 1;
    cmd_ptr->wrd0.bits.hm = 1;
    cmd_ptr->wrd1.bits.habs = 1;
    cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd0.bits.cm = 0x1; //cbc 0x1
	cmd_ptr->wrd0.bits.ces = 0; 
	cmd_ptr->wrd0.bits.ce = 1;
	cmd_ptr->wrd1.bits.cabs = 1;
	cmd_ptr->wrd2.bits.eptl = 0;
	   
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;
    aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.ap = 0x00;

	 /*aes cbc mix mode encrypt data src-descriptor*/ 
		 

	aes_cbc_encrypt_data_src.ctrl.type2.wrd = 0;
    aes_cbc_encrypt_data_src.ctrl.type2.bits.rs = 1;
    aes_cbc_encrypt_data_src.ctrl.type2.bits.ls = 1;


	 /*des cbc mix mode encrypt data src-descriptor*/ 
		 

	des_cbc_encrypt_data_src.ctrl.type2.wrd = 0;
    des_cbc_encrypt_data_src.ctrl.type2.bits.rs = 1;
    des_cbc_encrypt_data_src.ctrl.type2.bits.ls = 1;




	/*aes cbc mix mode encrypt cmd setting(hmac md5) src-descriptor*/	
	cmd_ptr = &aes_cbc_encrypt_cmmd_setting_hmacmd5;
	memset((uint8*)cmd_ptr, 0, 12);
	
	cmd_ptr->wrd0.bits.em = 3;
	cmd_ptr->wrd0.bits.he = 1;
	cmd_ptr->wrd0.bits.hm = 1;
	cmd_ptr->wrd1.bits.habs = 1;
	cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd0.bits.cm = 0x1; //cbc 0x1
	cmd_ptr->wrd0.bits.ces = 0;  //RTK_CES_AES
	cmd_ptr->wrd0.bits.ce = 1;
	cmd_ptr->wrd1.bits.cabs = 1;
	cmd_ptr->wrd2.bits.eptl = 0;

	cmd_ptr->wrd0.bits.hm 	= 0;//RTK_HM_MD5;
	cmd_ptr->wrd1.bits.hibs = 1;
	cmd_ptr->wrd1.bits.hobs = 1;
	cmd_ptr->wrd1.bits.hkbs = 1;


	/*des cbc mix mode encrypt cmd setting(hmac sha-1) src-descriptor*/ 
	//setting->ces = RTK_CES_DES;
	//setting->cm = RTK_CM_DES_CBC;
	//if(enc == CRYPTO_MODE_3DES_CBC) setting->tride = 1;	
	cmd_ptr = &des_cbc_encrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.em = 3;
	cmd_ptr->wrd0.bits.he = 1;
	cmd_ptr->wrd0.bits.hm = 1;
	cmd_ptr->wrd1.bits.habs = 1;
	cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd0.bits.cm = 0x1; //cbc 0x1
	cmd_ptr->wrd0.bits.ces = 1;  //RTK_CES_DES
	cmd_ptr->wrd0.bits.ce = 1;
	cmd_ptr->wrd1.bits.cabs = 1;
	cmd_ptr->wrd2.bits.eptl = 0;

	/*des cbc mix mode encrypt cmd setting(hmac md5) src-descriptor*/	
	cmd_ptr = &des_cbc_encrypt_cmmd_setting_hmacmd5;
	memset((uint8*)cmd_ptr, 0, 12);
	
	cmd_ptr->wrd0.bits.em = 3;
	cmd_ptr->wrd0.bits.he = 1;
	cmd_ptr->wrd0.bits.hm = 1;
	cmd_ptr->wrd1.bits.habs = 1;
	cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd0.bits.cm = 0x1; //cbc 0x1
	cmd_ptr->wrd0.bits.ces = 1;  //RTK_CES_DES
	cmd_ptr->wrd0.bits.ce = 1;
	cmd_ptr->wrd1.bits.cabs = 1;
	cmd_ptr->wrd2.bits.eptl = 0;

	cmd_ptr->wrd0.bits.hm 	= 0;//RTK_HM_MD5;
	cmd_ptr->wrd1.bits.hibs = 1;
	cmd_ptr->wrd1.bits.hobs = 1;
	cmd_ptr->wrd1.bits.hkbs = 1;

    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;
    des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.ap = 0x00;


	/*aes gcm mode encrypt cmd setting src-descriptor*/	
		
	cmd_ptr = &aes_gcm_encrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.em = 0;
    cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd0.bits.cm = 8; //gcm
	cmd_ptr->wrd0.bits.ces = 0; 
	cmd_ptr->wrd0.bits.ce = 1;
	cmd_ptr->wrd1.bits.cabs = 1;
    cmd_ptr->wrd0.bits.aks = 2;
	   
    aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
	aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
    aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
    aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
    aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
    aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;

	
	/*aes gcm mix mode decrypt cmd setting src-descriptor*/	
		
	cmd_ptr = &aes_gcm_decrypt_cmmd_setting;
	memset((uint8*)cmd_ptr, 0, 12);
	cmd_ptr->wrd0.bits.em = 0;
    cmd_ptr->wrd0.bits.icvtl = 0x40;
	cmd_ptr->wrd1.bits.cabs = 1;
	cmd_ptr->wrd0.bits.ces = 0; 
	cmd_ptr->wrd0.bits.cm = 8; //gcm
	cmd_ptr->wrd0.bits.ce = 0;
    cmd_ptr->wrd0.bits.aks = 2;
	   
    aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.wrd = 0;
	aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.rs = 1;
    aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.fs = 1;
    aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.cl = 3;
    aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.sk = 1;
    aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.pk = 1;

	/*AES GCM encrypt iv */
	
	aes_gcm_encrypt_iv_src.ctrl.type1.wrd = 0;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.rs=1;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.fs=1;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.ls=0;
	//data_desc.ctrl.type1.bits.il=ivlen/4;
	
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.sk = 1;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.pk = 1;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)

	for(i = 0 ; i < RTK_FC_HW_MAX_JOBS ; i++)
	{
		iv_ptr[i] = RTK_FC_HELPER_FC_KMALLOC(16, GFP_ATOMIC);
		cmdsetting_ptr[i] = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_cryptoEngine_command_t), GFP_ATOMIC);
		aead_ptr[i] = RTK_FC_HELPER_FC_KMALLOC(12, GFP_ATOMIC);
	}
#endif	
	
	return;
}



typedef struct rtk_fc_ipsec_ipi_debug_s{
	rtk_fc_pktHdr_t pktHdr;
	uint16 ivsize;
	uint32 cryptlen;
	int ipsec_shortCut_info_table_index;
	u8 iv[16];
	u8 skb_data[2048];
	uint32 skb_data_len;
	u8 key[64];
	uint32 key_sz;
	uint32 cipher_offset;
	u8 *dest_addr;
	u8* source_addr;
	u32 spi;
	rtk_fc_cryptoEngine_dest_desc_t  	cipher_dest_desc;
	rtk_fc_cryptoEngine_dest_desc_t  	icv_dest_desc;

	rtk_fc_cryptoEngine_source_desc_t 	cmd_src_desc;
	rtk_fc_cryptoEngine_command_t 		cmd;
	rtk_fc_cryptoEngine_source_desc_t 	iv_src_desc;
	rtk_fc_cryptoEngine_source_desc_t 	data_src_desc;
}rtk_fc_ipsec_ipi_debug_t;

rtk_fc_ipsec_ipi_debug_t ipsec_ipi_debug[MAX_FC_IPSEC_QUEUE_SIZE] = {0};

int print_num = 0;

rtk_fc_ipsec_ipi_ctrl_t rtk_fc_ipsec_ipi = {0};
rtk_fc_ipsec_ring_ctrl_t rtk_fc_ipsec_ipi_ring = {0};
uint32 last_count = 0;
uint32 no_packet_round_count = 0;
uint32 ipsec_exec_timeout= 0;
u64 total_enq_cnt = 0;
u64 total_deq_cnt = 0;
uint32 slow_cnt = 0;
uint32 decrypt_cnt = 0;
u8 ipsec_debug = 0;
u64 deq_err_cnt = 0;
u8 hw_crypto_init = 0;

#ifndef devm_ioremap_nocache
#define devm_ioremap_nocache devm_ioremap
#endif
#define RTK_FC_IPSEC_STATE_DYING_SOFT 1
#define RTK_FC_IPSEC_STATE_DYING_HARD 2

#define RTK_FC_HASH_PADDING_PUT_UINT32_LE(n,b,i)                       \
{                                                               \
    (b)[(i)    ] = (unsigned char) ( ( (n)       ) & 0xFF );   \
    (b)[(i) + 1] = (unsigned char) ( ( (n) >>  8 ) & 0xFF );   \
    (b)[(i) + 2] = (unsigned char) ( ( (n) >> 16 ) & 0xFF );   \
    (b)[(i) + 3] = (unsigned char) ( ( (n) >> 24 ) & 0xFF );   \
}

#define RTK_FC_HASH_PADDING_PUT_UINT32_BE(n,b,i)               \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}

#define RTK_FC_IPSEC_HASH_PADDING_PUT_UINT32_LE(n,b,i)                       \
{                                                               \
    (b)[(i)    ] = (unsigned char) ( ( (n)       ) & 0xFF );   \
    (b)[(i) + 1] = (unsigned char) ( ( (n) >>  8 ) & 0xFF );   \
    (b)[(i) + 2] = (unsigned char) ( ( (n) >> 16 ) & 0xFF );   \
    (b)[(i) + 3] = (unsigned char) ( ( (n) >> 24 ) & 0xFF );   \
}

#define RTK_FC_IPSEC_HASH_PADDING_PUT_UINT32_BE(n,b,i)               \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}

static const u8 mix_mode_hash_padding[64] __attribute__((aligned(32))) =
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

char *name_of_policy_dir[]={
						"XFRM_POLICY_IN",
						"XFRM_POLICY_OUT",
						"XFRM_POLICY_FWD",
						"XFRM_POLICY_MASK",
						"XFRM_POLICY_MAX"
					};



static inline unsigned int rtk_fc_crypto_aead_authsize(struct crypto_aead *tfm)
{
	return tfm->authsize;
}
static inline unsigned int rtk_fc_crypto_aead_alg_ivsize(struct aead_alg *alg)
{
	//printk("rtk_fc_crypto_aead_alg_ivsize: alg pointer:%p\n",alg);
	return alg->ivsize;
}

static inline struct crypto_tfm *rtk_fc_crypto_aead_tfm(struct crypto_aead *tfm)
{
	return &tfm->base;
}

static inline struct aead_alg *rtk_fc_crypto_aead_alg(struct crypto_aead *tfm)
{
	return container_of(rtk_fc_crypto_aead_tfm(tfm)->__crt_alg,
			    struct aead_alg, base);
}

static inline unsigned int rtk_fc_crypto_aead_ivsize(struct crypto_aead *tfm)
{
	return rtk_fc_crypto_aead_alg_ivsize(rtk_fc_crypto_aead_alg(tfm));
}

static inline void *rtk_fc_crypto_tfm_ctx(struct crypto_tfm *tfm)
{
	return tfm->__crt_ctx;
}

static inline void *rtk_fc_crypto_aead_ctx(struct crypto_aead *tfm)
{
	return rtk_fc_crypto_tfm_ctx(&tfm->base);
}
static inline unsigned int rtk_fc_crypto_tfm_alg_blocksize(struct crypto_tfm *tfm)
{
	return tfm->__crt_alg->cra_blocksize;
}

static inline unsigned int rtk_fc_crypto_aead_blocksize(struct crypto_aead *tfm)
{
	return rtk_fc_crypto_tfm_alg_blocksize(rtk_fc_crypto_aead_tfm(tfm));
}

void rtk_fc_hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}
void rtk_fc_reg_to_dev32_big(void *addr, const unsigned char *src, unsigned long nword)
{
   unsigned long v;

   while (nword--) {
     v = 0;
     v = (v << 8) | ((unsigned long)*src++);
     v = (v << 8) | ((unsigned long)*src++);
     v = (v << 8) | ((unsigned long)*src++);
     v = (v << 8) | ((unsigned long)*src++);
     rtk_fc_reg_io_write32(addr, v);
     addr += 4;
   }
}

void rtk_fc_reg_to_dev32_little(void *addr, const unsigned char *src, unsigned long nword)
{
   unsigned long v;

   while (nword--) {
     v = 0;
     v = (v >> 8) | ((unsigned long)*src++ << 24UL);
     v = (v >> 8) | ((unsigned long)*src++ << 24UL);
     v = (v >> 8) | ((unsigned long)*src++ << 24UL);
     v = (v >> 8) | ((unsigned long)*src++ << 24UL);
     rtk_fc_reg_io_write32(addr, v);
	 
     addr += 4;
   }
   
}

void rtk_fc_reg_to_dev32_s(void *addr, const unsigned char *src, unsigned long nword)
{
#ifdef CONFIG_CPU_BIG_ENDIAN
	rtk_fc_reg_to_dev32_little(addr, src, nword);
#else
	rtk_fc_reg_to_dev32_big(addr, src, nword);
#endif
}

static inline void rtk_fc_crypto_xor_byte(u8 *a, const u8 *b, unsigned int size)
{
	for (; size; size--)
	{
		//printk("size = %d\n",size);
		*a++ ^= *b++;
	}
}

void rtk_fc_crypto_xor(u8 *dst, const u8 *src, unsigned int size)
{
	u32 *a = (u32 *)dst; // IV = 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
	u32 *b = (u32 *)src; // salt = 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88

	for (; size >= 4; size -= 4)
	{
		*a++ ^= *b++;
		//printk("(1.5)*a = 0x%x *b = 0x%x\n",*(a-1), *(b-1));
		//printk("(2)*a = 0x%x *b = 0x%x\n",*a, *b);
	}
	//printk("size = %d\n",size);
	//printk("a[0] = 0x%x , a[1] = 0x%x\n", a[0], a[1]);
	rtk_fc_crypto_xor_byte((u8 *)a, (u8 *)b, size);
}

int rtk_fc_ipsec_write_hw_keypad(int index, u8 *key, int key_sz)
{
	int ret = SUCCESS;
	rtk_fc_cryptoEngine_keypad_lock_t data_desc;
	
	//printk("rtk_fc_ipsec_write_hw_keypad:addr: %p\n",rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKP(index));

	rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKP(index), key, 16);


	memset(&data_desc, 0, sizeof(rtk_fc_cryptoEngine_keypad_lock_t));
	data_desc.len = 64/8; //8bytes units

	//printk("rtk_fc_ipsec_write_hw_keypad:addr: %p\n",rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(index));

	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_KEYPAD_LOCK(index), *((uint32*) &data_desc));
	//rtk_fc_hexdump((unsigned char *)key, key_sz);

	return ret;
}

int rtk_fc_ipsec_write_hw_key(int index, u8 *key, int key_sz)
{
	int ret = SUCCESS;
	//printk("rtk_fc_ipsec_write_hw_key:addr: %p\n",rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(index));
	rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(index), key, 8);
	//rtk_fc_hexdump((unsigned char *)key, key_sz);

	return ret;
}


int _rtk_fc_crypto_get_hashkey_index(u8 *key, int key_size, int *key_index)
{
	int i , find = 0 ;

	for(i = 0; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
		if(!key_pad_usage[i].valid)
		{
			find = 1;
			*key_index = i;
			break;
		}
	}
	if(find == 1)
		return SUCCESS;
	else{
		*key_index = -1;
		return FAILED;
	}

	

}
EXPORT_SYMBOL(_rtk_fc_crypto_get_hashkey_index);

int _rtk_fc_crypto_get_key_index(int *key_index)
{
	int i , find = 0 ;

	for(i = 0; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
		if(!key_usage[i].valid)
		{
			find = 1;
			*key_index = i;
			break;
		}
	}
	if(find == 1)
		return SUCCESS;
	else{
		*key_index = -1;
		return FAILED;
	}

	

}

int _rtk_fc_crypto_get_keyhash_index(int *key_index)
{
	int i , find = 0 ;

	for(i = 0; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i ++)
	{
		if(!key_pad_usage[i].valid)
		{
			find = 1;
			*key_index = i;
			break;
		}
	}
	if(find == 1)
		return SUCCESS;
	else{
		*key_index = -1;
		return FAILED;
	}

}

int _rtk_fc_crytpo_key_del_by_index(int index)
{

	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	if(index < 0  || index >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM)
	{
		WARNING("Out of range.\n");
		return FAILED;
	}
	
	
		
	key_usage[index].valid = 0;
	key_usage[index].key_size = 0;
	

	//rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(rt_crypto_key_set_cfg->key_index), rt_crypto_key_set_cfg->key, 8);
	printk("DEL KEY(%d) DONE!!\n",index);

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return SUCCESS;
}
EXPORT_SYMBOL(_rtk_fc_crytpo_key_del_by_index);


int _rtk_fc_crytpo_keyhash_del_by_index(int index)
{

	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	if(index < 0  || index >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM)
	{
		WARNING("Out of range.\n");
		return FAILED;
	}
	
	
		
	key_pad_usage[index].valid = 0;
	key_pad_usage[index].key_size = 0;
	

	//rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(rt_crypto_key_set_cfg->key_index), rt_crypto_key_set_cfg->key, 8);
	printk("DEL KEYHASH(%d) DONE!!\n",index);

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return SUCCESS;
}
EXPORT_SYMBOL(_rtk_fc_crytpo_keyhash_del_by_index);


int _rtk_fc_crypto_key_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg)
{

	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	if(rt_crypto_key_set_cfg == NULL)
	{
		WARNING("Null pointer.\n");
		return FAILED;
	}
	if(rt_crypto_key_set_cfg->key_index < 0  || rt_crypto_key_set_cfg->key_index >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM)
	{
		WARNING("Out of range.\n");
		return FAILED;
	}
	
	
		
	key_usage[rt_crypto_key_set_cfg->key_index].valid = 1;
	key_usage[rt_crypto_key_set_cfg->key_index].key_size = rt_crypto_key_set_cfg->key_sz;
	memcpy(&key_usage[rt_crypto_key_set_cfg->key_index].key,rt_crypto_key_set_cfg->key, rt_crypto_key_set_cfg->key_sz);
	

	rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(rt_crypto_key_set_cfg->key_index), rt_crypto_key_set_cfg->key, 8);
	printk("SET KEY(%d) DONE!!\n",rt_crypto_key_set_cfg->key_index);

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return SUCCESS;
}
EXPORT_SYMBOL(_rtk_fc_crypto_key_add_by_index);

int _rtk_fc_crypto_keyhash_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg)
{

	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	if(rt_crypto_key_set_cfg == NULL)
	{
		WARNING("Null pointer.\n");
		return FAILED;
	}
	if(rt_crypto_key_set_cfg->key_index < 0  || rt_crypto_key_set_cfg->key_index >= RTK_FC_SPEC_XFRM_INFO_MAX_NUM)
	{
		WARNING("Out of range.\n");
		return FAILED;
	}
	
	
		
	key_pad_usage[rt_crypto_key_set_cfg->key_index].valid = 1;
	key_pad_usage[rt_crypto_key_set_cfg->key_index].key_size = rt_crypto_key_set_cfg->key_sz;
	memcpy(&key_pad_usage[rt_crypto_key_set_cfg->key_index].key,rt_crypto_key_set_cfg->key, rt_crypto_key_set_cfg->key_sz);
	
	rtk_fc_ipsec_write_hw_keypad(rt_crypto_key_set_cfg->key_index, rt_crypto_key_set_cfg->key, rt_crypto_key_set_cfg->key_sz);

	//rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(rt_crypto_key_set_cfg->key_index), rt_crypto_key_set_cfg->key, 8);
	printk("SET KEY HASH(%d) DONE!!\n",rt_crypto_key_set_cfg->key_index);

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return SUCCESS;
}
EXPORT_SYMBOL(_rtk_fc_crypto_keyhash_add_by_index);

int _rtk_fc_crypto_key_get_index(uint32 *index)
{
	int ret = SUCCESS;
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	ret  = _rtk_fc_crypto_get_key_index(index);
	if(ret != SUCCESS)
	{
		printk("Key full\n");
		ret = FAILED;
	}
	else
	{
		if(*index!=-1){
			key_usage[*index].valid = 1;
			printk("Get key index:%d success!\n", *index);
		}else
			return FAILED;

		

	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return ret;
}
EXPORT_SYMBOL(_rtk_fc_crypto_key_get_index);

int _rtk_fc_crypto_keyhash_get_index(uint32 *index)
{
	int ret = SUCCESS;
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	ret  = _rtk_fc_crypto_get_keyhash_index(index);
	if(ret != SUCCESS)
	{
		printk("Key full\n");
		ret = FAILED;
	}
	else
	{
		if(*index!=-1){
			key_pad_usage[*index].valid = 1;
			printk("Get keyhash index:%d success!\n", *index);
		}else
			return FAILED;

		

	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return ret;
}
EXPORT_SYMBOL(_rtk_fc_crypto_keyhash_get_index);

int _rtk_fc_crypto_key_set(rt_crypto_key_set_t *rt_crypto_key_set_cfg, int *key_index)
{
	int ret = SUCCESS, index = -1;
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(!hw_crypto_init)
	{
		WARNING("hw crypto is not inited.\n");
		return FAILED;
	}
	rtk_fc_hexdump(rt_crypto_key_set_cfg->key, rt_crypto_key_set_cfg->key_sz);
	
	ret  = _rtk_fc_crypto_get_key_index(&index);
	
	
	if(ret != SUCCESS)
	{
		printk("Key full\n");
		ret = FAILED;
	}
	else
	{
		if(index!=-1){
			
			rt_crypto_key_set_cfg->key_index = index;
			key_usage[index].valid = 1;
			key_usage[index].key_size = rt_crypto_key_set_cfg->key_sz;
			memcpy(&key_usage[index].key,rt_crypto_key_set_cfg->key, rt_crypto_key_set_cfg->key_sz);
		}else
			return FAILED;

		rtk_fc_reg_to_dev32_s(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SKDR(index), rt_crypto_key_set_cfg->key, 8);
		printk("SET KEY DONE!!\n");

	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return ret;
}
EXPORT_SYMBOL(_rtk_fc_crypto_key_set);

void rtk_fc_mix_mode_hmacMD5_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index)
{
    uint64 hash_total_len;
    uint32 low, high, last, padn, hash_padlen;
    uint8 hash_msglen[8] = {0x0};

    /*
     * Calculate hash padding values and size
     */
    memset((void *)fc_db.rtk_fc_ipsec_md5_hash_padding_ary[index].hash_padding , 0x0, sizeof(fc_db.rtk_fc_ipsec_md5_hash_padding_ary[index].hash_padding));
    if (isHMAC) {
        hash_total_len = (msglen + aadlen) + 64;
        //DBG_CRYPTO_INFO("hmac add 64bytes key pad\r\n");
    } else {
        hash_total_len = (msglen + aadlen);
    }

    /* Note:
     * AND 0x3F -> Know the last data size which need to be calculated.
     * Use the last data size to calculate how many bytes of padding.
     * Because there're reserved 8 bytes to represent total msglen in bits, so (64 - 8) = 56 or (64*2 - 8) = 120
     */
    last = ((msglen + aadlen) & 0x3F);
    padn = (last < 56) ? (56 - last) : (120 - last);
    memcpy((void *)fc_db.rtk_fc_ipsec_md5_hash_padding_ary[index].hash_padding, (void *)mix_mode_hash_padding, padn);

    /* Total hash msglen use 8bytes to represent!!!seperate low 4 bytes, high 4 bytes!!
     * Note: 
     * Because when store value to low, hash_total_len[31:0] need to left shift 3 bits(means 1byte to 8bits)
     * so if there are values in b'31:29, the 32bits low value can't store over 32 bits, so leave b'31:29 of 
     * hash_total_len[31:0] to high value.
     */
    high = ((hash_total_len & (0xFFFFFFFF)) >> 29)
         | (((hash_total_len >> 32) & (0xFFFFFFFF)) <<  3);
    low  = ((hash_total_len & (0xFFFFFFFF)) <<  3);

    //RTK_FC_HASH_PADDING_PUT_UINT32_BE(high, hash_msglen, 0);
   	//RTK_FC_HASH_PADDING_PUT_UINT32_BE(low , hash_msglen, 4);
	RTK_FC_HASH_PADDING_PUT_UINT32_LE(low,  hash_msglen, 0);
	RTK_FC_HASH_PADDING_PUT_UINT32_LE(high, hash_msglen, 4);

    memcpy((void *)(fc_db.rtk_fc_ipsec_md5_hash_padding_ary[index].hash_padding + padn), (void *)hash_msglen, 8);
    hash_padlen = (padn + 8);
	fc_db.rtk_fc_ipsec_md5_hash_padding_ary[index].hash_padlen = hash_padlen;

    return;
}

void rtk_fc_des_mix_mode_hmacMD5_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index)
{
    uint64 hash_total_len;
    uint32 low, high, last, padn, hash_padlen;
    uint8 hash_msglen[8] = {0x0};

    /*
     * Calculate hash padding values and size
     */
    memset((void *)fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[index].hash_padding , 0x0, sizeof(fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[index].hash_padding));
    if (isHMAC) {
        hash_total_len = (msglen + aadlen) + 64;
        //DBG_CRYPTO_INFO("hmac add 64bytes key pad\r\n");
    } else {
        hash_total_len = (msglen + aadlen);
    }

    /* Note:
     * AND 0x3F -> Know the last data size which need to be calculated.
     * Use the last data size to calculate how many bytes of padding.
     * Because there're reserved 8 bytes to represent total msglen in bits, so (64 - 8) = 56 or (64*2 - 8) = 120
     */
    last = ((msglen + aadlen) & 0x3F);
    padn = (last < 56) ? (56 - last) : (120 - last);
    memcpy((void *)fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[index].hash_padding, (void *)mix_mode_hash_padding, padn);

    /* Total hash msglen use 8bytes to represent!!!seperate low 4 bytes, high 4 bytes!!
     * Note: 
     * Because when store value to low, hash_total_len[31:0] need to left shift 3 bits(means 1byte to 8bits)
     * so if there are values in b'31:29, the 32bits low value can't store over 32 bits, so leave b'31:29 of 
     * hash_total_len[31:0] to high value.
     */
    high = ((hash_total_len & (0xFFFFFFFF)) >> 29)
         | (((hash_total_len >> 32) & (0xFFFFFFFF)) <<  3);
    low  = ((hash_total_len & (0xFFFFFFFF)) <<  3);

    //RTK_FC_HASH_PADDING_PUT_UINT32_BE(high, hash_msglen, 0);
   	//RTK_FC_HASH_PADDING_PUT_UINT32_BE(low , hash_msglen, 4);
	RTK_FC_HASH_PADDING_PUT_UINT32_LE(low,  hash_msglen, 0);
	RTK_FC_HASH_PADDING_PUT_UINT32_LE(high, hash_msglen, 4);

    memcpy((void *)(fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[index].hash_padding + padn), (void *)hash_msglen, 8);
    hash_padlen = (padn + 8);
	fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[index].hash_padlen = hash_padlen;

    return;
}

void rtk_fc_des_mix_mode_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index)
{
    uint64 hash_total_len;
    uint32 low, high, last, padn, hash_padlen;
    uint8 hash_msglen[8] = {0x0};

    /*
     * Calculate hash padding values and size
     */
    memset((void *)fc_db.rtk_fc_ipsec_des_hash_padding_ary[index].hash_padding , 0x0, sizeof(fc_db.rtk_fc_ipsec_des_hash_padding_ary[index].hash_padding));
    if (isHMAC) {
        hash_total_len = (msglen + aadlen) + 64;
        //DBG_CRYPTO_INFO("hmac add 64bytes key pad\r\n");
    } else {
        hash_total_len = (msglen + aadlen);
    }

    /* Note:
     * AND 0x3F -> Know the last data size which need to be calculated.
     * Use the last data size to calculate how many bytes of padding.
     * Because there're reserved 8 bytes to represent total msglen in bits, so (64 - 8) = 56 or (64*2 - 8) = 120
     */
    last = ((msglen + aadlen) & 0x3F);
    padn = (last < 56) ? (56 - last) : (120 - last);
    memcpy((void *)fc_db.rtk_fc_ipsec_des_hash_padding_ary[index].hash_padding, (void *)mix_mode_hash_padding, padn);

    /* Total hash msglen use 8bytes to represent!!!seperate low 4 bytes, high 4 bytes!!
     * Note: 
     * Because when store value to low, hash_total_len[31:0] need to left shift 3 bits(means 1byte to 8bits)
     * so if there are values in b'31:29, the 32bits low value can't store over 32 bits, so leave b'31:29 of 
     * hash_total_len[31:0] to high value.
     */
    high = ((hash_total_len & (0xFFFFFFFF)) >> 29)
         | (((hash_total_len >> 32) & (0xFFFFFFFF)) <<  3);
    low  = ((hash_total_len & (0xFFFFFFFF)) <<  3);

    RTK_FC_HASH_PADDING_PUT_UINT32_BE(high, hash_msglen, 0);
    RTK_FC_HASH_PADDING_PUT_UINT32_BE(low , hash_msglen, 4);

    memcpy((void *)(fc_db.rtk_fc_ipsec_des_hash_padding_ary[index].hash_padding + padn), (void *)hash_msglen, 8);
    hash_padlen = (padn + 8);
	fc_db.rtk_fc_ipsec_des_hash_padding_ary[index].hash_padlen = hash_padlen;

    return;
}

void rtk_fc_mix_mode_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index)
{
    uint64 hash_total_len;
    uint32 low, high, last, padn, hash_padlen;
    uint8 hash_msglen[8] = {0x0};

    /*
     * Calculate hash padding values and size
     */
    memset((void *)fc_db.rtk_fc_ipsec_hash_padding_ary[index].hash_padding , 0x0, sizeof(fc_db.rtk_fc_ipsec_hash_padding_ary[index].hash_padding));
    if (isHMAC) {
        hash_total_len = (msglen + aadlen) + 64;
        //DBG_CRYPTO_INFO("hmac add 64bytes key pad\r\n");
    } else {
        hash_total_len = (msglen + aadlen);
    }

    /* Note:
     * AND 0x3F -> Know the last data size which need to be calculated.
     * Use the last data size to calculate how many bytes of padding.
     * Because there're reserved 8 bytes to represent total msglen in bits, so (64 - 8) = 56 or (64*2 - 8) = 120
     */
    last = ((msglen + aadlen) & 0x3F);
    padn = (last < 56) ? (56 - last) : (120 - last);
    memcpy((void *)fc_db.rtk_fc_ipsec_hash_padding_ary[index].hash_padding, (void *)mix_mode_hash_padding, padn);

    /* Total hash msglen use 8bytes to represent!!!seperate low 4 bytes, high 4 bytes!!
     * Note: 
     * Because when store value to low, hash_total_len[31:0] need to left shift 3 bits(means 1byte to 8bits)
     * so if there are values in b'31:29, the 32bits low value can't store over 32 bits, so leave b'31:29 of 
     * hash_total_len[31:0] to high value.
     */
    high = ((hash_total_len & (0xFFFFFFFF)) >> 29)
         | (((hash_total_len >> 32) & (0xFFFFFFFF)) <<  3);
    low  = ((hash_total_len & (0xFFFFFFFF)) <<  3);

    RTK_FC_HASH_PADDING_PUT_UINT32_BE(high, hash_msglen, 0);
    RTK_FC_HASH_PADDING_PUT_UINT32_BE(low , hash_msglen, 4);

    memcpy((void *)(fc_db.rtk_fc_ipsec_hash_padding_ary[index].hash_padding + padn), (void *)hash_msglen, 8);
    hash_padlen = (padn + 8);
	fc_db.rtk_fc_ipsec_hash_padding_ary[index].hash_padlen = hash_padlen;

    return;
}

int _rtk_fc_ipsec_table_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx)
{
	int offset = 0;
	rtk_fc_ipsec_flowindex_list_t *flow_index_list;
	rtk_fc_ipsec_flowindex_list_t *pTmp_flow_index_list, *pTmp_next_flow_index_list;

	IPSEC("_rtk_fc_ipsec_table_setting");
	if(pG3IgrExtraInfo==NULL || pPktHdr==NULL)
	{
		IPSEC("NULL input");
		return FAILED;
	}


	
	if(pG3IgrExtraInfo->ipsec_hook_table_index!=-1)
	{
		
		if(pG3IgrExtraInfo->ipsec_direction == 1) //Encrypt
		{
			if(pPktHdr->iph==NULL)
			{
				IPSEC("ip header NULL!");
				return FAILED;
			}
			if(pPktHdr->esph ==NULL)
			{
				IPSEC("esp header NULL!");
				return FAILED;
			}
			if(pPktHdr->iph)
			{
				memcpy(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].contentBuffer+offset, pPktHdr->iph, sizeof(struct iphdr));
				offset += sizeof(struct iphdr);
				fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].ipid = ntohs(pPktHdr->iph->id)+1;

			}
			if(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].is_NATT)
			{
				if(!pPktHdr->outer_udph)
				{
					IPSEC("NAT-T But no outer udp!");
					return FAILED;
				}
				else
				{
					memcpy(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].contentBuffer+offset, pPktHdr->outer_udph, sizeof(struct udphdr));
					offset += sizeof(struct udphdr);
				}
			}

			if(!pPktHdr->esph)
			{
				IPSEC("ESP But no esp!");
				return FAILED;
			}
			else
			{
				memcpy(fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].contentBuffer+offset, pPktHdr->esph, sizeof(struct ip_esp_hdr));
				offset += sizeof(struct ip_esp_hdr);
			}

			
			if(pG3IgrExtraInfo->ipsec_seq_num > fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].seq_no )
			{
				//printk("----[B4]%s(%d):pG3IgrExtraInfo->ipsec_seq_num = %d, fc_ipsec_info.seq_no = %d \n",__func__,__LINE__, pG3IgrExtraInfo->ipsec_seq_num, fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].seq_no);
				fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].seq_no = pG3IgrExtraInfo->ipsec_seq_num+1;
				
				pPktHdr->skip_replace_seqno = 1;
				//printk("----[After]%s(%d):Update fc_ipsec_info.seq_no = %d \n",__func__,__LINE__, fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].seq_no);
			}
			fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].remote_lut_idx = pG3IgrExtraInfo->remote_tunnel_lut_idx;
			
			IPSEC("Update fc_ipsec_info[%d] seq_no = %d\n", pG3IgrExtraInfo->ipsec_hook_table_index, fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].seq_no);

				
		}
		fc_db.fc_ipsec_info[pG3IgrExtraInfo->ipsec_hook_table_index].devGwMacIdx   = pPktHdr->ipsec_sc_info.devGwMacIdx;

		{
			
			
			int find = 0;
			if(!list_empty(&fc_db.ipsec_swFlow_list[pG3IgrExtraInfo->ipsec_hook_table_index]))
			{
				list_for_each_entry_safe(pTmp_flow_index_list, pTmp_next_flow_index_list, &fc_db.ipsec_swFlow_list[pG3IgrExtraInfo->ipsec_hook_table_index], flow_list)	//just return the first entry right behind of head
				{
					if(pTmp_flow_index_list->flow_index == flowIdx)
					{
						find = 1;
						TABLE("Find flow index:%d in flow list (table:%d)",flowIdx, pG3IgrExtraInfo->ipsec_hook_table_index);
					}
				}
			}
			if(find == 0)
			{
				flow_index_list = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_ipsec_flowindex_list_t), GFP_ATOMIC);
				
				INIT_LIST_HEAD(&flow_index_list->flow_list);
				
				flow_index_list->flow_index = flowIdx;
				list_add_tail(&flow_index_list->flow_list, &fc_db.ipsec_swFlow_list[pG3IgrExtraInfo->ipsec_hook_table_index]);
				TABLE("Add flow index:%d to flow list (table:%d)",flowIdx, pG3IgrExtraInfo->ipsec_hook_table_index);
			}
		}
		return SUCCESS;
	}
	
	
	
	WARNING("IPSEC table setting failed!!!\n");
	return FAILED;
}

int _rtk_fc_esp_setting(int index, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, bool *swOnly, int direction)
{
	int ret = SUCCESS;

	pPktHdr->ipsec_hook_table_index = index;
	*swOnly = TRUE; // Should be not true when nic acc
	pG3IgrExtraInfo->dualHdrType 			= RTK_FC_DUALTYPE_IPSEC;
	pG3IgrExtraInfo->ipsec_hook_table_index = index;
	
	pG3IgrExtraInfo->ipsec_direction 		= fc_db.fc_ipsec_info[index].direction;
	pG3IgrExtraInfo->ipsec_spi 				= ntohl(fc_db.fc_ipsec_info[index].spi);
	IPSEC("tbl index =%d, SPI = %x", index,fc_db.fc_ipsec_info[index].spi);
	pG3IgrExtraInfo->isIpsec_acc			= 1;
	if(direction==1)
	{
		if(pPktHdr->esph == NULL)
		{
			IPSEC("IPSEC encrypt but no ESP header!");
			return FAILED;
		}
		pG3IgrExtraInfo->ipsec_seq_num			= ntohl(pPktHdr->esph->seq_no);
		pG3IgrExtraInfo->remote_tunnel_lut_idx	= pPktHdr->dmacL2Idx;
	}
	else
	{
		pG3IgrExtraInfo->ipsec_seq_num			= pFcIngressData->ipsec_info.ingress_seq_num;
		pG3IgrExtraInfo->remote_tunnel_lut_idx	= pPktHdr->smacL2Idx;
	}
	
	if(fc_db.fc_ipsec_info[index].key_is_set==0)
	{
		if(_rtk_fc_ipsec_get_supported_mode(fc_db.fc_ipsec_info[index].ealgo, fc_db.fc_ipsec_info[index].aalgo) )
		{
			int key_index= -1, hash_key_index = -1;
			
			
			if(fc_db.fc_ipsec_info[index].key_hash_sz>0){
				printk("Set hash key: algo: %d\n", fc_db.fc_ipsec_info[index].ealgo);
				
				ret  = _rtk_fc_crypto_get_hashkey_index(fc_db.fc_ipsec_info[index].key_hash,fc_db.fc_ipsec_info[index].key_hash_sz, &hash_key_index);
				if(ret == SUCCESS)
				{
					printk("hash key index = %d\n",hash_key_index);
					rtk_fc_ipsec_write_hw_keypad(hash_key_index, fc_db.fc_ipsec_info[index].key_hash, fc_db.fc_ipsec_info[index].key_hash_sz);
					
					fc_db.fc_ipsec_info[index].hash_key_index = hash_key_index;
					
					key_pad_usage[hash_key_index].valid = 1;
					key_pad_usage[hash_key_index].key_size = fc_db.fc_ipsec_info[index].key_hash_sz;
					memcpy(&key_pad_usage[hash_key_index].key, fc_db.fc_ipsec_info[index].key_hash, fc_db.fc_ipsec_info[index].key_hash_sz);
				}
			}
			
			if(fc_db.fc_ipsec_info[index].key_sz>0){
				printk("Set cipher key: algo: %d\n", fc_db.fc_ipsec_info[index].ealgo);	
				ret  = _rtk_fc_crypto_get_key_index(&key_index);
				if(ret == SUCCESS)
				{
					printk("key index = %d\n",key_index);
					//rtk_fc_hexdump(fc_db.fc_ipsec_info[index].key, fc_db.fc_ipsec_info[index].key_sz);
					rtk_fc_ipsec_write_hw_key(key_index, fc_db.fc_ipsec_info[index].key, fc_db.fc_ipsec_info[index].key_sz);
					
					fc_db.fc_ipsec_info[index].key_index = key_index;
					
					key_usage[key_index].valid = 1;
					key_usage[key_index].key_size = fc_db.fc_ipsec_info[index].key_sz;
					memcpy(&key_usage[key_index].key, fc_db.fc_ipsec_info[index].key, fc_db.fc_ipsec_info[index].key_sz);
				}
				else
				{
					printk("Get key index failed!\n");
				}
			}
			
			fc_db.fc_ipsec_info[index].key_is_set = 1;
		}
		

	}
	
	pG3IgrExtraInfo->cipher_mode = fc_db.fc_ipsec_info[index].ealgo;
	
	return ret;
}
int _rtk_fc_ipsec_recreate_seqNum(int cipher_mode, int iv_size, int auth_size, rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, int ipsec_shortCut_info_table_index, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int seq_no)
{
	int crypt_len = 0;
	int offset = 0;
	int ret = SUCCESS;
	int enq_failed = 0;
	//int seqno = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no;//;
	int isNAT_T = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].is_NATT;
	int isESN = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].is_ESN;
	u8 tmp[16];
	int tmp_iv_size = iv_size;
	int ring_index = -1;
	int aead_size;
	//printk("RTSKB_LEN(rtskb) = %u offset = %d iv_size = %d auth_size = %d, sizeof(struct ip_esp_hdr) = %lu\n",RTSKB_LEN(rtskb), offset, iv_size, auth_size, sizeof(struct ip_esp_hdr));
	/*
		Set ring info first

	*/
	

	offset += 14;
	if(pPktHdr->ppph)
		offset+=8;
	if(pPktHdr->cvh)
		offset+=4;
	if(pPktHdr->svh)
		offset+=4;

	
	if(cipher_mode == RTK_FC_EALG_AESCBC) // aes cbc
	{
		aead_size   = sizeof(struct ip_esp_hdr);
		crypt_len = RTSKB_LEN(rtskb) - offset - 20 - sizeof(struct ip_esp_hdr) - iv_size - auth_size/*icv*/;
		if(isNAT_T)
			crypt_len -=8; // udp header
#if 0 			
		ret = _rtk_fc_aes_cbc_slow_path_decrypt(rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index, iv_size);
		if(ret != SUCCESS)
		{
			WARNING("recreate seqnum-decrypt failed!");
			goto ERROR_RETURN;
		}
#endif		
		IPSEC("[IPSEC][SLOW]_rtk_fc_ipsec_recreate_seqNum: change seq_no from %d to %d\n", ntohl(pPktHdr->esph->seq_no), seq_no);
		/*
			Fill out the descriptors
		*/
		pPktHdr->esph->seq_no = htonl(seq_no);
		fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no+=1;
		/*
			recreate a new IV
		*/
		
		{
			// Start insert iph, esph
			{
				do {
					u64 a;
	
					memcpy(&a, fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].salt_in + tmp_iv_size - 8, 8);
	
					a |= 1;
					a *= seq_no;
	
					memcpy(&tmp[0] + tmp_iv_size - 8, &a, 8);
				} while ((tmp_iv_size -= 8));
	
				memcpy((u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), &tmp[0], 16);
				
			}
		}
		
		if(isESN)
		{
			
			/*
			Set ipi work info
			*/
			memcpy(RTSKB_DATA(rtskb)+RTSKB_LEN(rtskb), &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no_hi, 4); // put the seq_no_hi after ESP trailer
			DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb)+4, "[AES CBC ESN]after put seq_hi to tail");
			
			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_ENCRYT, 
												cipher_mode, iv_size, aead_size, auth_size, crypt_len+iv_size,
												(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr),
												(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+crypt_len+iv_size, 
												pNicTxPriv,1,0, pPktHdr);

			if(ret != SUCCESS)
			{
				enq_failed = 1;
				ret = RTK_FC_RET_DROP_IPSEC_SLOW_IPI_FULL;
				goto ERROR_RETURN;
			}
			
			ret = _rtk_fc_aes_cbc_esn_slow_path_encrypt_ipi(rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index, iv_size);
			if(ret != SUCCESS)
			{
				WARNING("recreate seqnum-encrypt failed!auth_size = %d, crypt_len = %d ipsec_shortCut_info_table_index = %d",auth_size, crypt_len,ipsec_shortCut_info_table_index);
				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");
				goto ERROR_RETURN;
			}
		}
		else
		{
			
			/*
			Set ipi work info
			*/

			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_ENCRYT, 
							cipher_mode, iv_size, aead_size, auth_size, crypt_len+iv_size,
							(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr),
							(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+crypt_len+iv_size, 
							pNicTxPriv,0,0, pPktHdr);
			if(ret != SUCCESS)
			{
				enq_failed = 1;
				ret = RTK_FC_RET_DROP_IPSEC_SLOW_IPI_FULL;
				goto ERROR_RETURN;
			}
			ret = _rtk_fc_aes_cbc_slow_path_encrypt_ipi(ring_index, rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index, iv_size);
			if(ret != SUCCESS)
			{
				WARNING("recreate seqnum-encrypt failed!auth_size = %d, crypt_len = %d ipsec_shortCut_info_table_index = %d",auth_size, crypt_len,ipsec_shortCut_info_table_index);
				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");
				goto ERROR_RETURN;
			}

		}
		
	}
	else if(cipher_mode == RTK_FC_EALG_DESCBC || cipher_mode == RTK_FC_EALG_3DESCBC) // aes cbc
	{
		aead_size   = sizeof(struct ip_esp_hdr);
		crypt_len = RTSKB_LEN(rtskb) - offset - 20 - sizeof(struct ip_esp_hdr) - iv_size - auth_size/*icv*/;
		if(isNAT_T)
			crypt_len -=8; // udp header
	
		IPSEC("[IPSEC][SLOW]_rtk_fc_ipsec_recreate_seqNum: change seq_no from %d to %d\n", ntohl(pPktHdr->esph->seq_no), seq_no);
		/*
			Fill out the descriptors
		*/
		pPktHdr->esph->seq_no = htonl(seq_no);
		fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no+=1;
		/*
			recreate a new IV
		*/
		
		{
			// Start insert iph, esph
			{
				do {
					u64 a;
	
					memcpy(&a, fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].salt_in + tmp_iv_size - 8, 8);
	
					a |= 1;
					a *= seq_no;
	
					memcpy(&tmp[0] + tmp_iv_size - 8, &a, 8);
				} while ((tmp_iv_size -= 8));
	
				memcpy((u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), &tmp[0], 8);
				
			}
		}
		
		if(0/*isESN*/)
		{
			
			/*
			Set ipi work info
			*/
			memcpy(RTSKB_DATA(rtskb)+RTSKB_LEN(rtskb), &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no_hi, 4); // put the seq_no_hi after ESP trailer
			DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb)+4, "[DES CBC ESN]after put seq_hi to tail");
			
			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_ENCRYT, 
												cipher_mode, iv_size, aead_size, auth_size, crypt_len+iv_size,
												(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr),
												(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+crypt_len+iv_size, 
												pNicTxPriv,1,0, pPktHdr);

			if(ret != SUCCESS)
			{
				enq_failed = 1;
				ret = RTK_FC_RET_DROP_IPSEC_SLOW_IPI_FULL;
				goto ERROR_RETURN;
			}
			
			ret = _rtk_fc_aes_cbc_esn_slow_path_encrypt_ipi(rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index, iv_size);
			if(ret != SUCCESS)
			{
				WARNING("recreate seqnum-encrypt failed!auth_size = %d, crypt_len = %d ipsec_shortCut_info_table_index = %d",auth_size, crypt_len,ipsec_shortCut_info_table_index);
				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");
				goto ERROR_RETURN;
			}
		}
		else
		{
			
			/*
			Set ipi work info
			*/

			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_ENCRYT, 
							cipher_mode, iv_size, aead_size, auth_size, crypt_len+iv_size,
							(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr),
							(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+crypt_len+iv_size, 
							pNicTxPriv,0,0, pPktHdr);
			if(ret != SUCCESS)
			{
				enq_failed = 1;
				ret = RTK_FC_RET_DROP_IPSEC_SLOW_IPI_FULL;
				goto ERROR_RETURN;
			}
			ret = _rtk_fc_des_cbc_slow_path_encrypt_ipi(ring_index, cipher_mode, rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index, iv_size);
			if(ret != SUCCESS)
			{
				WARNING("recreate seqnum-encrypt failed!auth_size = %d, crypt_len = %d ipsec_shortCut_info_table_index = %d",auth_size, crypt_len,ipsec_shortCut_info_table_index);
				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");
				goto ERROR_RETURN;
			}

		}
		
	}
	else if(cipher_mode == RTK_FC_EALG_AES_GCM_ICV12) // gcm
	{	
		crypt_len = RTSKB_LEN(rtskb) - offset - 20 - sizeof(struct ip_esp_hdr) - 8/*gcm iv size = 8*/ - auth_size/*icv*/;
		if(isNAT_T)
			crypt_len -=8; // udp header
#if 0 			
		ret = _rtk_fc_gcm_slow_path_decrypt(rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index);
		if(ret != SUCCESS)
		{
			WARNING("recreate seqnum-decrypt failed!");
			goto ERROR_RETURN;
		}
#endif
		IPSEC("[IPSEC][SLOW]change seq_no from %d to %d\n", ntohl(pPktHdr->esph->seq_no), seq_no);
		/*
			Set ipi work info first
		*/
		if(isESN)
			aead_size   = 12;
		else
			aead_size   = sizeof(struct ip_esp_hdr);
		ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_ENCRYT, 
											  cipher_mode, iv_size, aead_size, auth_size, crypt_len,
											  (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+8, 
											  (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+8+crypt_len, 
											  pNicTxPriv, 0, 0, pPktHdr);
		if(ret != SUCCESS)
		{
			enq_failed = 1;
			ret = RTK_FC_RET_DROP_IPSEC_SLOW_IPI_FULL;
			goto ERROR_RETURN;
		}



		/*
			Fill out the descriptors
		*/
		pPktHdr->esph->seq_no = htonl(seq_no);
		fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no+=1;
		
		/*
			recreate a new IV
		*/
		{
			u8  iv[8];
			
			memcpy(&iv[0]+4, (u8 *)&seq_no, sizeof(uint32));
			
			//nonce-> key last 4 bytes
			//iv -> increment from 1, and do crypto xor
			rtk_fc_crypto_xor(&iv[0],  fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].salt_in, iv_size);
		
			memcpy((u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), &iv[0], iv_size);
			wmb();
	
			
		}		
		ret = _rtk_fc_gcm_slow_path_encrypt_ipi(ring_index, rtskb, pPktHdr, auth_size, crypt_len, ipsec_shortCut_info_table_index);
		if(ret != SUCCESS)
		{
			WARNING("recreate seqnum-encrypt failed!auth_size = %d, crypt_len = %d ipsec_shortCut_info_table_index = %d",auth_size, crypt_len,ipsec_shortCut_info_table_index);
			dump_packet(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");
			goto ERROR_RETURN;
		}
		
	}
	//DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After change esp seq_no");

	

	
	return ret;
ERROR_RETURN:
	
	if(unlikely(fc_db.fwdStatistic))
	{
		atomic_inc(&fc_db.statistic.perPortCnt_ipsec_change_seq_num_drop[pPktHdr->ingressPort.macPortIdx]);
	}
	IPSEC("recreate seqnum failed! enq failed = %d",enq_failed);
	return ret;
}

__IRAM_FC_SHORTCUT
static int rtk_fc_ipsec_decrypt_des_set_cmdSetting(uint32 cryptlen, int cipher_mode, uint32 ivlen, uint32 key_index, uint32 hash_key_index, int key_size , rtk_fc_cryptoEngine_command_t *cmd_setting, rtk_fc_pktHdr_t *pPktHdr, uint32_t *cmdsetting_dmaPtr)
{
	uint32_t addr;

	cmd_setting->wrd0 = des_cbc_decrypt_cmmd_setting.wrd0;
	cmd_setting->wrd1 = des_cbc_decrypt_cmmd_setting.wrd1;
	cmd_setting->wrd2 = des_cbc_decrypt_cmmd_setting.wrd2;

	cmd_setting->wrd2.bits.etl = (cryptlen + 7)/8;
    cmd_setting->wrd1.bits.elds = cryptlen % 8;

	if(cipher_mode==RTK_FC_EALG_3DESCBC)
		cmd_setting->wrd0.bits.tride =1 ;
	
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
	des_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.pl = hash_key_index;

	addr = dma_map_single(&(ni_info_data.pdev->dev), cmd_setting, sizeof(rtk_fc_cryptoEngine_command_t), DMA_TO_DEVICE);
	*cmdsetting_dmaPtr = addr;

	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, des_cbc_decrypt_cmmd_setting_src.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);


	return SUCCESS;
}

__IRAM_FC_SHORTCUT
static int rtk_fc_ipsec_decrypt_set_cmdSetting(uint32 cryptlen, uint32 ivlen, uint32 key_index, int key_size , rtk_fc_cryptoEngine_command_t *cmd_setting, rtk_fc_pktHdr_t *pPktHdr, uint32_t *cmdsetting_dmaPtr)
{
	uint32_t addr;

	cmd_setting->wrd0 = aes_cbc_decrypt_cmmd_setting.wrd0;
	cmd_setting->wrd1 = aes_cbc_decrypt_cmmd_setting.wrd1;
	cmd_setting->wrd2 = aes_cbc_decrypt_cmmd_setting.wrd2;

	cmd_setting->wrd2.bits.etl = (cryptlen + 15)/16;
    cmd_setting->wrd1.bits.elds = cryptlen % 16;
	if(key_size == 32)
	   cmd_setting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
   	else if(key_size == 16)
	   cmd_setting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
   	else if(key_size == 24)
	   cmd_setting->wrd0.bits.aks = 1; //AES_KEYSIZE_192

	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
	aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.bits.pl = key_index;

	addr = dma_map_single(&(ni_info_data.pdev->dev), cmd_setting, sizeof(rtk_fc_cryptoEngine_command_t), DMA_TO_DEVICE);
	*cmdsetting_dmaPtr = addr;

	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_decrypt_cmmd_setting_src.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);


	return SUCCESS;
}
__IRAM_FC_SHORTCUT
static void  _rtk_fc_ipsec_shortCut_set_iv_src_desc(int ivlen, int key_index, int hash_key_index, u8* addrs, rtk_fc_pktHdr_t *pPktHdr, uint32_t *iv_dma)
{
	uint32_t addr;

	aes_cbc_decrypt_iv_src.ctrl.type1.bits.il	= ivlen/4;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.kl 	= key_index;
	aes_cbc_decrypt_iv_src.ctrl.type1.bits.pl 	= hash_key_index;
	addr = dma_map_single(&(ni_info_data.pdev->dev), addrs, ivlen, DMA_TO_DEVICE);//fc_db.ipsec_addr.iv_dma;
	*iv_dma = addr;
	wmb();
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_decrypt_iv_src.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		
}
__IRAM_FC_SHORTCUT
static void _rtk_fc_ipsec_shortCut_set_src_desc(int cryptlen, int cryp_pad_len, u8 *pData, rtk_fc_pktHdr_t *pPktHdr)
{
	uint32_t addr;

	aes_cbc_decrypt_data_src.ctrl.type2.bits.enl = cryptlen;
	if(cryp_pad_len == 0)
		aes_cbc_decrypt_data_src.ctrl.type2.bits.ls=1;

	addr = dma_map_single(&(ni_info_data.pdev->dev), pData, cryptlen, DMA_TO_DEVICE);
	
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_decrypt_data_src.ctrl.type2.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);

	if(cryp_pad_len)
	{
		aes_cbc_decrypt_data_padding_src.ctrl.type2.bits.enl = cryp_pad_len;
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_decrypt_data_padding_src.ctrl.type2.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, fc_db.ipsec_addr.ipsec_crypt_padding_dma);

	}
}

__IRAM_FC_SHORTCUT
static void _rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(int icv_len, int hash_padding_len, u8 *icv_addr, rtk_fc_pktHdr_t *pPktHdr)
{
	uint32_t addr;
	
	aes_cbc_encrypt_dest_desc_icv.ctrl.type1.bits.adl = icv_len;
	if(hash_padding_len ==0)
	{
		aes_cbc_encrypt_dest_desc_icv.ctrl.type1.bits.ls = 1;
	}
	addr =	dma_map_single(&(ni_info_data.pdev->dev), icv_addr, icv_len, DMA_TO_DEVICE);;//fc_db.ipsec_addr.plain_text_dma;;//fc_db.ipsec_addr.icv_dma;
	
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, aes_cbc_encrypt_dest_desc_icv.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, addr);
	wmb();

	if(hash_padding_len)
	{
		aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.bits.adl = hash_padding_len;
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, aes_cbc_encrypt_dest_desc_icv_pad.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, aes_cbc_encrypt_dest_desc_icv_pad.ddbp);

	}
}

int _rtk_fc_aes_cbc_esn_slow_path_encrypt_ipi(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize)
{
	int ret = SUCCESS;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	uint32 key_index;
	int aead_size = 0;
	int crypt_len, key_size, cryp_pad_len;
	u8 iv_tmp[16] = {0};
	u8 key_tmp[32] = {0};
	
	key_index	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index ;
	
	aead_size	= 8;
	
	
		
	crypt_len 	= cryptlen+ivsize;
	key_size 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	IPSEC("crypt_len = %d, key_size = %d\n",crypt_len,key_size);
	/*
		ESN Need to encrypt first
	*/
	_rtk_fc_ipsec_aescbc_esn_set_cipher_dest_desc(crypt_len, pCipher_data);
	_rtk_fc_ipsec_aescbc_esn_set_cipheronly_cmdsetting(crypt_len, key_index);
	
	
	
	memcpy(&key_tmp[0], &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key[0], key_size);
	_rtk_fc_ipsec_aescbc_esn_cipher_key(key_size, &key_tmp[0]);
	

	memcpy(&iv_tmp[0], ((u8 *)(pPktHdr->esph)+aead_size), ivsize);
	_rtk_fc_ipsec_aescbc_esn_cipher_iv(ivsize, &iv_tmp[0]);
	
	
	
	cryp_pad_len = (16 - (crypt_len % 16)) % 16;
	_rtk_fc_ipsec_set_src_desc(crypt_len, cryp_pad_len, (u8 *)(pPktHdr->esph)+aead_size);
		

	return ret;
}
int _rtk_fc_des_cbc_slow_path_encrypt_ipi(int ring_index, int cipher_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize)
{
	int ret = SUCCESS;
	u8 *src_aad_plaintext_hash_padding_buffer=NULL;
	int hash_padding_array_idx = 0;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	u8 *pICV_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+cryptlen+ivsize;
	u8 key_index, hash_key_index;
	uint32 hash_pad_len = 0;
	int aead_size = 0;
	int HMAC_SHA_1_ICV_LEN = 20, HMAC_MD5_ICV_LEN = 16, HMAC_SHA_256_ICV_LEN = 32, key_size;
	int hash_mode;
	//u8 cmd_setting[28] = {0};
	uint32_t cmdsetting_dmaPtr = 0;
	
	key_index		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;
	hash_key_index  = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	key_size    = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	hash_mode 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].aalgo;
	aead_size	= sizeof(struct ip_esp_hdr);

	IPSEC(" ipsec_shortCut_info_table_index = %d key_index = %d hash_key_index = %d key_size = %d hash_mode = %d ivsize = %d",ipsec_shortCut_info_table_index,key_index, hash_key_index, key_size, hash_mode, ivsize);
	
	DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "[EN]before descriptor");
	{
		hash_padding_array_idx = ((cryptlen+ivsize) /8)-1;
		if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
			hash_pad_len = fc_db.rtk_fc_ipsec_des_hash_padding_ary[hash_padding_array_idx].hash_padlen;
		else if (hash_mode == RTK_FC_AALG_MD5HMAC)
			hash_pad_len = fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[hash_padding_array_idx].hash_padlen;
			
		IPSEC("hash_padlen = %d hash_padding_array_idx = %d", hash_pad_len, hash_padding_array_idx);
		src_aad_plaintext_hash_padding_buffer	= RTK_FC_HELPER_FC_KMALLOC(cryptlen+ivsize +aead_size +hash_pad_len , GFP_ATOMIC);


		//for mix mode, total 4 descriptor

		//[dst desc] ciphertext
		//_rtk_fc_ipsec_encrypt_set_dest_desc(cryptlen+ivsize, pCipher_data);
		_rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(cryptlen+ivsize, pCipher_data, pPktHdr);
		
		IPSEC("cryptlen = %d ivsize = %d pCipher_data = %p, auth_size  = %d",cryptlen, ivsize, pCipher_data, auth_size);
		//[dst desc] hash
		//_rtk_fc_ipsec_encrypt_set_icv_dest_desc(auth_size, HMAC_SHA_1_ICV_LEN-auth_size); //OK
		if(hash_mode == RTK_FC_AALG_SHA1HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_SHA_1_ICV_LEN- auth_size,pICV_data,pPktHdr);	
		else if (hash_mode == RTK_FC_AALG_SHA2_256HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_SHA_256_ICV_LEN- auth_size,pICV_data, pPktHdr);
		else if(hash_mode == RTK_FC_AALG_MD5HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_MD5_ICV_LEN - auth_size,pICV_data,pPktHdr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
		//[src desc] cmd + iv
		_rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(hash_mode,
													cipher_mode,
													ivsize /*ivlen*/, 
													cryptlen+ivsize,  /*alan: 48+16*/
													hash_pad_len, 
													aead_size/*aad len:esp header size*/,
													pPktHdr,
													key_index,
													hash_key_index,
													key_size,
													cmdsetting_ptr[ring_index],
													&cmdsetting_dmaPtr);
		
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else		
		//[src desc] cmd + iv
		_rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(hash_mode,
													cipher_mode,
													ivsize /*ivlen*/, 
													cryptlen+ivsize,  /*alan: 48+16*/
													hash_pad_len, 
													aead_size/*aad len:esp header size*/,
													pPktHdr,
													key_index,
													hash_key_index,
													key_size,
													&cmdsetting_ptr[ring_index],
													&cmdsetting_dmaPtr);
		
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif		
		//[src desc] aad + plaintext + hash_padding
		_rtk_fc_ipsec_descbc_mixmode_set_src_desc(hash_mode,
												aead_size/*aad len:esp header size*/, 
												cryptlen+ivsize, 
												hash_pad_len, 
												pPktHdr, 
												hash_padding_array_idx, 
												src_aad_plaintext_hash_padding_buffer,
												cryptlen+ivsize+hash_pad_len+aead_size);

	}
	
	//dump_packet(RTSKB_DATA(rtskb),RTSKB_LEN(rtskb), "AFTER CRYPTO");
	RTK_FC_HELPER_FC_KFREE(src_aad_plaintext_hash_padding_buffer, cryptlen+ivsize +aead_size +hash_pad_len);
	
	

	return ret;
}

int _rtk_fc_aes_cbc_slow_path_encrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize)
{
	int ret = SUCCESS;
	u8 *src_aad_plaintext_hash_padding_buffer=NULL;
	int hash_padding_array_idx = 0;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	u8 *pICV_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+cryptlen+ivsize;
	u8 key_index, hash_key_index;
	uint32 hash_pad_len = 0;
	int aead_size = 0;
	int HMAC_SHA_1_ICV_LEN = 20, HMAC_MD5_ICV_LEN = 16, HMAC_SHA_256_ICV_LEN = 32, key_size;
	int hash_mode;
	//u8 cmd_setting[28] = {0};
	uint32_t cmdsetting_dmaPtr = 0;
	
	key_index		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;
	hash_key_index  = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	key_size    = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	hash_mode 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].aalgo;
	aead_size	= sizeof(struct ip_esp_hdr);

	IPSEC(" ipsec_shortCut_info_table_index = %d key_index = %d hash_key_index = %d key_size = %d hash_mode = %d ivsize = %d",ipsec_shortCut_info_table_index,key_index, hash_key_index, key_size, hash_mode, ivsize);
	
	DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "[EN]before descriptor");
	{
		hash_padding_array_idx = ((cryptlen+ivsize) /16)-1;
		if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
			hash_pad_len = fc_db.rtk_fc_ipsec_hash_padding_ary[hash_padding_array_idx].hash_padlen;
		else if (hash_mode == RTK_FC_AALG_MD5HMAC)
			hash_pad_len = fc_db.rtk_fc_ipsec_md5_hash_padding_ary[hash_padding_array_idx].hash_padlen;
			
		IPSEC("hash_padlen = %d hash_padding_array_idx = %d", hash_pad_len, hash_padding_array_idx);
		src_aad_plaintext_hash_padding_buffer	= RTK_FC_HELPER_FC_KMALLOC(cryptlen+ivsize +aead_size +hash_pad_len , GFP_ATOMIC);


		//for mix mode, total 4 descriptor

		//[dst desc] ciphertext
		//_rtk_fc_ipsec_encrypt_set_dest_desc(cryptlen+ivsize, pCipher_data);
		_rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(cryptlen+ivsize, pCipher_data, pPktHdr);
		
		IPSEC("cryptlen = %d ivsize = %d pCipher_data = %p, auth_size  = %d",cryptlen, ivsize, pCipher_data, auth_size);
		//[dst desc] hash
		//_rtk_fc_ipsec_encrypt_set_icv_dest_desc(auth_size, HMAC_SHA_1_ICV_LEN-auth_size); //OK
		if(hash_mode == RTK_FC_AALG_SHA1HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_SHA_1_ICV_LEN- auth_size,pICV_data,pPktHdr);	
		else if (hash_mode == RTK_FC_AALG_SHA2_256HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_SHA_256_ICV_LEN- auth_size,pICV_data, pPktHdr);
		else if(hash_mode == RTK_FC_AALG_MD5HMAC)
			_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_size, HMAC_MD5_ICV_LEN - auth_size,pICV_data,pPktHdr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
		//[src desc] cmd + iv
		_rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(hash_mode,
													ivsize /*ivlen*/, 
													cryptlen+ivsize,  /*alan: 48+16*/
													hash_pad_len, 
													aead_size/*aad len:esp header size*/,
													pPktHdr,
													key_index,
													hash_key_index,
													key_size,
													cmdsetting_ptr[ring_index],
													&cmdsetting_dmaPtr);
		
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else		
		//[src desc] cmd + iv
		_rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(hash_mode,
													ivsize /*ivlen*/, 
													cryptlen+ivsize,  /*alan: 48+16*/
													hash_pad_len, 
													aead_size/*aad len:esp header size*/,
													pPktHdr,
													key_index,
													hash_key_index,
													key_size,
													&cmdsetting_ptr[ring_index],
													&cmdsetting_dmaPtr);
		
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif		
		//[src desc] aad + plaintext + hash_padding
		_rtk_fc_ipsec_aescbc_mixmode_set_src_desc(hash_mode,
												aead_size/*aad len:esp header size*/, 
												cryptlen+ivsize, 
												hash_pad_len, 
												pPktHdr, 
												hash_padding_array_idx, 
												src_aad_plaintext_hash_padding_buffer,
												cryptlen+ivsize+hash_pad_len+aead_size);

	}
	
	//dump_packet(RTSKB_DATA(rtskb),RTSKB_LEN(rtskb), "AFTER CRYPTO");
	RTK_FC_HELPER_FC_KFREE(src_aad_plaintext_hash_padding_buffer, cryptlen+ivsize +aead_size +hash_pad_len);
	
	

	return ret;
}



int _rtk_fc_gcm_slow_path_encrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index)
{
	
	int GCM_ICV_LEN = 16, cryp_pad_len = 0, gcm_hw_iv_size = 16;
	uint32 key_index;
	int ret = SUCCESS;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+8;
	u8 gcm_iv_tail[4] = {0x00, 0x00, 0x00, 0x01};
	int AES_BLOCK_SIZE = 16;
	int aead_size = 0;
	int isESN = 0;
	//u8 final_iv_tmp[16] = {0};
	//u8 aad_tmp[12] = {0};
	int key_size;
	uint32_t iv_dma = 0, aead_dma = 0, cmdsetting_dmaPtr = 0;

	IPSEC("ring_index = %d",ring_index);
	
	cryp_pad_len = (AES_BLOCK_SIZE - (cryptlen % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE;
	key_index 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;
	isESN 		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].is_ESN;
	key_size 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	if(isESN)
		aead_size   = 12;
	else
		aead_size   = sizeof(struct ip_esp_hdr);


	
	//ivsize = 16

	IPSEC("crypt len:%d, hash_padding; %d, crypt_padding: %d isESN = %d aead_size = %d",cryptlen,GCM_ICV_LEN-auth_size,cryp_pad_len,isESN,aead_size);
	
	


	//for mix mode, total 4 descriptor

	//[dst desc] ciphertext
	
	_rtk_fc_ipsec_encrypt_set_dest_desc(cryptlen, pCipher_data);

	
	_rtk_fc_ipsec_encrypt_set_icv_dest_desc(auth_size, GCM_ICV_LEN-auth_size, (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+8+cryptlen);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)	 
		_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(cryptlen, aead_size, key_index,1, cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
	
#else
		_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(cryptlen, aead_size, key_index,1, &cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif

	

  	
	//_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(cryptlen, aead_size, key_index,1);

	//memcpy(&iv_ptr[ring_index][0], fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].nonce, 4);

	memcpy(&iv_ptr[ring_index][0], fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].nonce, 4);
	memcpy((&iv_ptr[ring_index][0])+4, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr), 8);
	memcpy((&iv_ptr[ring_index][0])+12, gcm_iv_tail, 4);
	


	//_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, (&final_iv_tmp[0]));
	_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, &iv_ptr[ring_index][0], &iv_dma);
	
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_dma	   = iv_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_pointer = &iv_ptr[ring_index][0];

	if(isESN)
	{
		memcpy((&aead_ptr[ring_index][0])		, (u8 *)(pPktHdr->esph), 4);
		memcpy((&aead_ptr[ring_index][0]) + 4	, &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].seq_no_hi, 4);
		memcpy((&aead_ptr[ring_index][0]) + 4 + 4, (u8 *)(pPktHdr->esph)+4, 4);
	
   		_rtk_fc_ipsec_set_aad_src_desc(aead_size, &aead_ptr[ring_index][0], &aead_dma);
	
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_dma	   	= aead_dma;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_pointer 	= &aead_ptr[ring_index][0];
	}
	else
	{
		memcpy(&aead_ptr[ring_index][0], (u8 *)(pPktHdr->esph),aead_size);
   		_rtk_fc_ipsec_set_aad_src_desc(aead_size, &aead_ptr[ring_index][0], &aead_dma);
	
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_dma	   	= aead_dma;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_pointer 	= &aead_ptr[ring_index][0];
	}
	
   	_rtk_fc_ipsec_set_src_desc(cryptlen, cryp_pad_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+8);

	return ret;
}




int _rtk_fc_egress_ipsec_seqNum_sync(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_smp_nicTx_private_t *pNicTxPriv)
{
	int ret = RTK_FC_RET_DROP;
	int i = 0;
	uint32 seqno = 0;
	int find = 0;
	int hook_index = -1;
	
	
	if((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IPSEC) && pPktHdr->iph && pPktHdr->esph )
	{
		for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i++)
		{
			if( fc_db.fc_ipsec_info[i].valid == 0)
				continue;

			if(fc_db.fc_ipsec_info[i].ip_version == 0)
				IPSEC("ipsec try to set seq num, pPktHdr->iph->daddr = %x(%x), spi = %x(%x)", ntohl(pPktHdr->iph->daddr),  (fc_db.fc_ipsec_info[i].daddr), ntohl(pPktHdr->esph->spi), ntohl(fc_db.fc_ipsec_info[i].spi));
			else
			{
				IPSEC("ipsec try to set seq num, pPktHdr->ip6h->daddr = %x:%x:%x:%x(%x:%x:%x:%x), spi = %x(%x)", ntohl(pPktHdr->ip6h->daddr.s6_addr32[0]), ntohl(pPktHdr->ip6h->daddr.s6_addr32[1])
																											   , ntohl(pPktHdr->ip6h->daddr.s6_addr32[2]), ntohl(pPktHdr->ip6h->daddr.s6_addr32[3]),  
																											   ntohl(fc_db.fc_ipsec_info[i].daddr6.s6_addr32[0]), ntohl(fc_db.fc_ipsec_info[i].daddr6.s6_addr32[1]), 
																											   ntohl(fc_db.fc_ipsec_info[i].daddr6.s6_addr32[2]), ntohl(fc_db.fc_ipsec_info[i].daddr6.s6_addr32[3]), 
																												ntohl(pPktHdr->esph->spi), ntohl(fc_db.fc_ipsec_info[i].spi));
			}
			
			if(fc_db.fc_ipsec_info[i].valid && pPktHdr->iph && pPktHdr->esph &&
			   ntohl(fc_db.fc_ipsec_info[i].spi) == ntohl(pPktHdr->esph->spi)
			)
			{
				if(fc_db.fc_ipsec_info[i].ip_version == 0 &&  (fc_db.fc_ipsec_info[i].daddr)==ntohl(pPktHdr->iph->daddr))
				{
					seqno = ntohl(pPktHdr->esph->seq_no);
					find = 1;
					hook_index = i;
					//if(fc_db.fc_ipsec_info[hook_index].ipid < ntohs(pPktHdr->iph->id))
					//	fc_db.fc_ipsec_info[hook_index].ipid = ntohs(pPktHdr->iph->id)+1;
					break;
				}
				else if(fc_db.fc_ipsec_info[i].ip_version == 1 && !memcmp(&fc_db.fc_ipsec_info[i].daddr6.s6_addr32[0], &pPktHdr->ip6h->daddr.s6_addr32[0], sizeof(struct in6_addr)) )
				{
					seqno = ntohl(pPktHdr->esph->seq_no);
					find = 1;
					hook_index = i;
					break;

				}
			}
		}
		if(find)
		{
			int iv_size,auth_size;
			
			if(fc_db.fc_ipsec_info[hook_index].key_is_set==0)
			{
				if(_rtk_fc_ipsec_get_supported_mode(fc_db.fc_ipsec_info[hook_index].ealgo, fc_db.fc_ipsec_info[hook_index].aalgo) )
				{
					int key_index= -1, hash_key_index = -1;
					
					
					if(fc_db.fc_ipsec_info[hook_index].key_hash_sz>0){
						printk("Set hash key: algo: %d\n", fc_db.fc_ipsec_info[hook_index].ealgo);
						
						ret  = _rtk_fc_crypto_get_hashkey_index(fc_db.fc_ipsec_info[hook_index].key_hash,fc_db.fc_ipsec_info[hook_index].key_hash_sz, &hash_key_index);
						if(ret == SUCCESS)
						{
							printk("hash key index = %d\n",hash_key_index);
							rtk_fc_ipsec_write_hw_keypad(hash_key_index, fc_db.fc_ipsec_info[hook_index].key_hash, fc_db.fc_ipsec_info[hook_index].key_hash_sz);
							
							fc_db.fc_ipsec_info[hook_index].hash_key_index = hash_key_index;
							
							key_pad_usage[hash_key_index].valid = 1;
							key_pad_usage[hash_key_index].key_size = fc_db.fc_ipsec_info[hook_index].key_hash_sz;
							memcpy(&key_pad_usage[hash_key_index].key, fc_db.fc_ipsec_info[hook_index].key_hash, fc_db.fc_ipsec_info[hook_index].key_hash_sz);
						}
					}
					
					if(fc_db.fc_ipsec_info[hook_index].key_sz>0){
						printk("Set cipher key: algo: %d\n", fc_db.fc_ipsec_info[hook_index].ealgo);	
						ret  = _rtk_fc_crypto_get_key_index(&key_index);
						if(ret == SUCCESS)
						{
							printk("key index = %d\n",key_index);
							rtk_fc_ipsec_write_hw_key(key_index, fc_db.fc_ipsec_info[hook_index].key, fc_db.fc_ipsec_info[hook_index].key_sz);
							
							fc_db.fc_ipsec_info[hook_index].key_index = key_index;
							
							key_usage[key_index].valid = 1;
							key_usage[key_index].key_size = fc_db.fc_ipsec_info[hook_index].key_sz;
							memcpy(&key_usage[key_index].key, fc_db.fc_ipsec_info[hook_index].key, fc_db.fc_ipsec_info[hook_index].key_sz);
						}
						else
						{
							printk("Get key index failed!\n");
						}
					}
					
					fc_db.fc_ipsec_info[hook_index].key_is_set = 1;
				}
				

			}
			
			if(!fc_db.controlFuc.ipsec_pe_offload)
			{
				
				
				if(pPktHdr->skip_replace_seqno)
				{
					IPSEC("Need to encrypt!");
					iv_size  = fc_db.fc_ipsec_info[hook_index].iv_len;
					auth_size  = fc_db.fc_ipsec_info[hook_index].auth_len;
					
					ret = _rtk_fc_ipsec_recreate_seqNum(fc_db.fc_ipsec_info[hook_index].ealgo, iv_size, auth_size, pPktHdr, rtskb, hook_index, pNicTxPriv, seqno);
					if(ret == SUCCESS)
					{
						if(unlikely(fc_db.fwdStatistic))
						{
							atomic_inc(&fc_db.statistic.perPortCnt_ipsec_change_seq_num[pPktHdr->ingressPort.macPortIdx]);
						}

					}

				}
				else
				{
					int seq_no = 0;
					
					if(seqno < fc_db.fc_ipsec_info[hook_index].seq_no)
					{
						seq_no = fc_db.fc_ipsec_info[hook_index].seq_no;
						IPSEC("[SLOWPATH]%s(%d): Need to Change seq num from %d to %d!!\n", __func__, __LINE__, seqno, fc_db.fc_ipsec_info[hook_index].seq_no);
					}
					else
						seq_no = seqno;

					
					iv_size  = fc_db.fc_ipsec_info[hook_index].iv_len;
					auth_size  = fc_db.fc_ipsec_info[hook_index].auth_len;
					ret = _rtk_fc_ipsec_recreate_seqNum(fc_db.fc_ipsec_info[hook_index].ealgo, iv_size, auth_size, pPktHdr, rtskb, hook_index, pNicTxPriv, seq_no);
					if(ret == SUCCESS)
					{
						if(unlikely(fc_db.fwdStatistic))
						{
							atomic_inc(&fc_db.statistic.perPortCnt_ipsec_change_seq_num[pPktHdr->ingressPort.macPortIdx]);
						}

					}
				}

			}
			else if(fc_db.fc_ipsec_info[hook_index].pe_crypto_sw_id) //valid
			{
				struct ethhdr *pEtherHdr = (struct ethhdr *)RTSKB_DATA(rtskb);
				uint32 innerIp_offset=0, l2PayloadLen=0;
				uint8 espTrailerLen=0, nextProtocol=IPPROTO_IPIP;

				iv_size  = fc_db.fc_ipsec_info[hook_index].iv_len;
				auth_size  = fc_db.fc_ipsec_info[hook_index].auth_len;
				nextProtocol = *(uint8 *)(RTSKB_DATA(rtskb) + RTSKB_LEN(rtskb) - auth_size - 1);
				espTrailerLen = *(uint8 *)(RTSKB_DATA(rtskb) + RTSKB_LEN(rtskb) - auth_size - 2)/*pad len*/ + 2;
				innerIp_offset = pPktHdr->l4Offset + sizeof(struct ip_esp_hdr) + iv_size ;
				l2PayloadLen = RTSKB_LEN(rtskb) - innerIp_offset - auth_size - espTrailerLen;

				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "before trimming");
				pEtherHdr->h_proto = (nextProtocol==IPPROTO_IPIP) ? htons(0x0800) : htons(0x86dd) ;
				memmove( RTSKB_DATA(rtskb)+14, RTSKB_DATA(rtskb)+innerIp_offset, l2PayloadLen );
				RTK_FC_HOOK_PS_SKB_SKB_TRIM(RTSKB_SKB(rtskb), 14 + l2PayloadLen);
				DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "After trimming");

				//hwlookup from RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID
				pNicTxPriv->isHwlookup 		= 1;
				pNicTxPriv->flow_id 		= fc_db.fc_ipsec_info[hook_index].pe_crypto_sw_id;
				pNicTxPriv->skb 			= RTSKB_SKB(rtskb);
				pNicTxPriv->hwlookup_lspid 	= RT_PE_IPSEC_DMA_LSO_FIRST_ENCRYPT_HWLOOKUP_LSPID;
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // support lspid_map table				
				pNicTxPriv->hwlookup_lspid_mapIdx = fc_db.lspid_map_idx_pe_first_encrypt_hwlookup;
#endif
				TRACE("[HWLOOKUP] PE crypto offload ... first encrypt packet !!");
				RTK_FC_HELPER_MGR_NIC_TX(pNicTxPriv); //rtk_fc_nic_tx
				ret = SUCCESS;
			}
		}	
	}
	return ret;
}

int _rtk_fc_esp_policy_check(int *index, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int encrypt)
{
	int ret = SUCCESS, i;
	if(encrypt == 1)
	{
		DEBUG("Encrypt!");
		if(pPktHdr->iph==NULL)
		{
			IPSEC("pPktHdr->iph is NULL!");
			return FAILED;
		}
		if(pPktHdr->esph==NULL)
		{
			IPSEC("pPktHdr->esph is NULL!");
			return FAILED;
		}
		for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i++)
		{
			if(fc_db.fc_ipsec_info[i].valid ==0)
				continue;

			/*
			printk(KERN_INFO "id: daddr=%08x-%08x-%08x-%08x, spi=0x%08x, proto=0x%02x\n",
			ntohl(x->id.daddr.a6[0]), ntohl(x->id.daddr.a6[1]),
			ntohl(x->id.daddr.a6[2]), ntohl(x->id.daddr.a6[3]), ntohl(x->id.spi), x->id.proto);
			*/
			if(fc_db.fc_ipsec_info[i].valid && fc_db.fc_ipsec_info[i].ip_version == 0 )
			{
				IPSEC("i = %d id:daddr=%08x, spi=0x%08x, pktHdr:daddr = %08x, spi=0x%08x mark.v = 0x%x",i, 
								(fc_db.fc_ipsec_info[i].daddr),
								ntohl(fc_db.fc_ipsec_info[i].spi),
								ntohl(pPktHdr->iph->daddr),
								ntohl(pPktHdr->esph->spi),
								fc_db.fc_ipsec_info[i].x_state->mark.v);
				
				if((fc_db.fc_ipsec_info[i].daddr)==ntohl(pPktHdr->iph->daddr) &&
				   ntohl(fc_db.fc_ipsec_info[i].spi) == ntohl(pPktHdr->esph->spi) &&
				   _rtk_fc_ipsec_get_supported_mode(fc_db.fc_ipsec_info[i].ealgo, fc_db.fc_ipsec_info[i].aalgo) &&
				   fc_db.fc_ipsec_info[i].x_state->mark.v == 0
				)
				{
					
					
					/*
					enum {
					XFRM_POLICY_IN	= 0,
					XFRM_POLICY_OUT	= 1,
					XFRM_POLICY_FWD	= 2,
					XFRM_POLICY_MASK = 3,
					XFRM_POLICY_MAX	= 3
					};
					*/
					IPSEC("[Encrypt]Find ipsec ealgo:%d aalgo:%d!! fc_db.fc_ipsec_info[%d], direction = %d(%s)", fc_db.fc_ipsec_info[i].ealgo, fc_db.fc_ipsec_info[i].aalgo,i,fc_db.fc_ipsec_info[i].direction, name_of_policy_dir[fc_db.fc_ipsec_info[i].direction]);
					ret = SUCCESS;
					*index = i;
					break;
				}
			}
		}

	}
	else
	{
		DEBUG("Decrypt!");
		if(pFcIngressData->ipsec_info.ingress_spi == 0)
		{
			IPSEC("Ingress SPI is 0 (ingress esp header is null)!");
			return FAILED;
		}
		for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM ; i++)
		{
			if(fc_db.fc_ipsec_info[i].valid ==0)
				continue;

			/*
			printk(KERN_INFO "id: daddr=%08x-%08x-%08x-%08x, spi=0x%08x, proto=0x%02x\n",
			ntohl(x->id.daddr.a6[0]), ntohl(x->id.daddr.a6[1]),
			ntohl(x->id.daddr.a6[2]), ntohl(x->id.daddr.a6[3]), ntohl(x->id.spi), x->id.proto);
			*/
			if(fc_db.fc_ipsec_info[i].valid)
			{
				IPSEC("i = %d id:daddr=%08x, spi=0x%08x, pktHdr:daddr = %08x, spi=0x%08x",i, 
								(fc_db.fc_ipsec_info[i].daddr),
								(fc_db.fc_ipsec_info[i].spi),
								pFcIngressData->dstIp, //pFcIngressData->dstIp = ntohl(pPktHdr->iph->daddr);
								ntohl(pFcIngressData->ipsec_info.ingress_spi));  
				if((fc_db.fc_ipsec_info[i].daddr)==pFcIngressData->dstIp &&
				   (fc_db.fc_ipsec_info[i].spi) == ntohl(pFcIngressData->ipsec_info.ingress_spi ) &&
				   _rtk_fc_ipsec_get_supported_mode(fc_db.fc_ipsec_info[i].ealgo, fc_db.fc_ipsec_info[i].aalgo) &&
				   fc_db.fc_ipsec_info[i].x_state->mark.v == 0
				)
				{
					
					
					/*
					enum {
					XFRM_POLICY_IN	= 0,
					XFRM_POLICY_OUT	= 1,
					XFRM_POLICY_FWD	= 2,
					XFRM_POLICY_MASK = 3,
					XFRM_POLICY_MAX	= 3
					};
					*/
					IPSEC("[Decrypt]Find ipsec ealgo:%d aalgo:%d!! fc_db.fc_ipsec_info[%d], direction = %d(%s)", fc_db.fc_ipsec_info[i].ealgo, fc_db.fc_ipsec_info[i].aalgo,i,fc_db.fc_ipsec_info[i].direction, name_of_policy_dir[fc_db.fc_ipsec_info[i].direction]);
					ret = SUCCESS;
					*index = i;
					break;
				}
			}
		}

	}
	
	return ret;
}

int rtk_fc_ipsec_remove_header(int cipher_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{
	uint32 ivlen, icvlen;
	uint32 len_without_l2 = 0;
	int hook_table_index = -1;
	//int final_cyptlen = 0;
	
	if(pPktHdr->esph == NULL)
		return FAILED;
	len_without_l2 = RTSKB_LEN(rtskb) - 14;
	if(pPktHdr->ppph)
		len_without_l2 -=8;
	if(pPktHdr->cvh)
		len_without_l2 -=4;
	if(pPktHdr->svh)
		len_without_l2 -=4;

	hook_table_index = pPktHdr->ipsec_sc_info.table_index;

	ivlen 	= fc_db.fc_ipsec_info[hook_table_index].iv_len;
	icvlen 	= fc_db.fc_ipsec_info[hook_table_index].auth_len; 
	// *cryptlen = len_without_l2 - sizeof(struct iphdr)- sizeof(struct ip_esp_hdr)-ivlen-icvlen;
	pPktHdr->ipsec_sc_info.crypto_plaintext_length = len_without_l2 - sizeof(struct iphdr)- sizeof(struct ip_esp_hdr)-ivlen-icvlen;

	if(fc_db.fc_ipsec_info[hook_table_index].is_NATT)
	{
		if(!pPktHdr->outer_udph)
		{
			IPSEC("NAT-T down stream but don't have udp!");
			return RTK_FC_RET_DROP_IPSEC_SC_NAT_T_HDRERR;
		}
		else
		{
			//*cryptlen -=8;
			pPktHdr->ipsec_sc_info.crypto_plaintext_length -=8;
		}
	}
	IPSEC("len_without_l2 = %d, ivlen = %d icvlen = %d",len_without_l2, ivlen,icvlen);

	//*ret_iv_size = ivlen;
	pPktHdr->ipsec_sc_info.ivsize 			= ivlen;
	pPktHdr->ipsec_sc_info.key_size			= fc_db.fc_ipsec_info[hook_table_index].key_sz; 
	pPktHdr->ipsec_sc_info.key_hash_size	= fc_db.fc_ipsec_info[hook_table_index].key_hash_sz; 
	
	
	return SUCCESS;
}
int rtk_fc_ipsec_insert_header(int mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable,uint32 flowTblIdx)
{	
	//uint8 *pData = RTSKB_DATA(rtskb);
	uint16 hook_table_index = 0;
	uint32 total_header_insert_len = 0;
	uint32 total_tail_insert_len = 0;
	int blksize;
	int clen;
	int alen;
	int plen;
	int ivsize;
	int len_without_l2 = 0;
	u8 tail[64]={0};
	uint8 *pData = RTSKB_DATA(rtskb);
	int insertOff = 0;
	uint16 l3proto = 0;
	u8 tmp[16];// = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
	uint32 seqno;
	uint32 spi;
	uint32 seqno_hi;
	uint8 isESN = 0;
	uint8 isNAT_T = 0;
	int aead_len = 0;
	
	insertOff += (ETH_HLEN);
	//IPSEC("AAt first pData = %p, skb:%p skb->data:%p, skb->len:%d, mode = %d",pData,RTSKB_SKB(rtskb),RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), mode);
			

	if(pPktHdr->svh) insertOff+= VLAN_HLEN;
	if(pPktHdr->cvh) insertOff+= VLAN_HLEN;
	
	hook_table_index = pFlowTable->ipsec_flow_info.ipsec_shortCut_info_table_index;
	isNAT_T = fc_db.fc_ipsec_info[hook_table_index].is_NATT;
	if(fc_db.fc_ipsec_info[hook_table_index].valid ==0 )
	{
		WARNING("Invalid hook table[%d] is not valid",hook_table_index);
		return FAIL;

	}
	
	len_without_l2 = RTSKB_LEN(rtskb) - 14;
	if(pPktHdr->cvh)
		len_without_l2-=4;
	if(pPktHdr->svh)
		len_without_l2-=4;
	if(pPktHdr->ppph)
		len_without_l2-=8;
	
	blksize = ALIGN(fc_db.fc_ipsec_info[hook_table_index].block_size, 4);
	ivsize= fc_db.fc_ipsec_info[hook_table_index].iv_len;
	alen = fc_db.fc_ipsec_info[hook_table_index].auth_len;
	isESN = fc_db.fc_ipsec_info[hook_table_index].is_ESN;
	clen =  ALIGN(len_without_l2 + 2, blksize);
	plen = clen - len_without_l2;
	if(isESN)
	{
		if(mode == RTK_FC_EALG_AESCBC)
			aead_len = 8;
		else if(mode == RTK_FC_EALG_AES_GCM_ICV12)
			aead_len = 12;
	}
	else 
		aead_len = 8;

	
	total_tail_insert_len = plen+alen;

	total_header_insert_len = 20+aead_len+ivsize;//ip + esp + iv
	if(isNAT_T)
		total_header_insert_len  += 8;
		
	pPktHdr->crypto_plaintext_length 	= clen;
	pPktHdr->ipsec_sc_info.valid		= 1;
	pPktHdr->ipsec_sc_info.direction	= RTK_FC_IPSEC_DIR_ENCRYT;
	pPktHdr->ipsec_sc_info.cipher_mode	= mode; 
	pPktHdr->ipsec_sc_info.crypto_plaintext_length = clen;
	pPktHdr->ipsec_sc_info.ivsize 		= ivsize;// *ret_iv_size 	= ivsize;
	pPktHdr->ipsec_sc_info.auth_sizes 	= alen;  // *ret_auth_size 	= alen;
	pPktHdr->ipsec_sc_info.isESN		= isESN; // *ret_is_ESN 	= isESN;
	pPktHdr->ipsec_sc_info.key_size		= fc_db.fc_ipsec_info[hook_table_index].key_sz;
	pPktHdr->ipsec_sc_info.key_hash_size		= fc_db.fc_ipsec_info[hook_table_index].key_hash_sz;
	IPSEC("cipher mode = %d block_size = %d, iv_size = %d auth_len = %d, clen = %d, plen = %d aead_len = %d len_without_l2 = %d ,total_header_insert_len = %d, total_tail_insert_len = %d, isESN = %d isNAT_T = %d", mode,fc_db.fc_ipsec_info[hook_table_index].block_size, ivsize,alen,clen, plen,aead_len, len_without_l2,total_header_insert_len,total_tail_insert_len,isESN, isNAT_T);	
	if(_rtk_fc_skb_cow_head_and_pktHdr_update(RTSKB_SKB(rtskb), total_header_insert_len, rtskb, pPktHdr)<0)
	{
		WARNING("skb head room is not enough");
		return FAIL;
	}
	RTK_FC_HOOK_PS_SKB_SKB_PUSH(RTSKB_SKB(rtskb), total_header_insert_len, &pData);
	memmove(pData, pData + total_header_insert_len, insertOff);
	
	RTSKB_MAC_HEADER(rtskb) -= (total_header_insert_len);
	pPktHdr->eth = (struct ethhdr *)(pData);
	
	if(pPktHdr->svh) pPktHdr->svh = (struct vlan_hdr *)((uint8 *)(pPktHdr->svh) - total_header_insert_len);
	if(pPktHdr->cvh) pPktHdr->cvh = (struct vlan_hdr *)((uint8 *)(pPktHdr->cvh) - total_header_insert_len);
	
	
	if(pPktHdr->cvh){
		l3proto = pPktHdr->cvh->h_vlan_encapsulated_proto;
		pPktHdr->cvh->h_vlan_encapsulated_proto = htons(0x0800);
	}
	else if(pPktHdr->svh){
		l3proto = pPktHdr->svh->h_vlan_encapsulated_proto;
		pPktHdr->svh->h_vlan_encapsulated_proto = htons(0x0800);
	}else{
		l3proto = pPktHdr->eth->h_proto;
		pPktHdr->eth->h_proto = htons(0x0800);
	}
	if(isNAT_T)
		memcpy(pData + insertOff, &fc_db.fc_ipsec_info[hook_table_index].contentBuffer, 20+8+sizeof(struct ip_esp_hdr));
	else
		memcpy(pData + insertOff, &fc_db.fc_ipsec_info[hook_table_index].contentBuffer, 20+sizeof(struct ip_esp_hdr));
				
			
	pPktHdr->iph = (struct iphdr *)(pData + insertOff);
	
	if(isESN)
	{
		if(mode == RTK_FC_EALG_AES_GCM_ICV12)
			pPktHdr->iph->tot_len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len -4 );
		else if(mode == RTK_FC_EALG_AESCBC)
			pPktHdr->iph->tot_len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len);
	}
	else
		pPktHdr->iph->tot_len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len );
	//pPktHdr->iph->id = htons(fc_db.fc_ipsec_info[hook_table_index].ipid);
	fc_db.fc_ipsec_info[hook_table_index].ipid +=1;

	insertOff += sizeof(struct iphdr);

	if(isNAT_T)
	{
		pPktHdr->outer_udph = (struct udphdr *)(pData + insertOff);
		if(isESN)
		{
			if(mode == RTK_FC_EALG_AES_GCM_ICV12)
				pPktHdr->outer_udph->len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len -20 -4 );
			else
				pPktHdr->outer_udph->len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len -20 );	
		}
		else
			pPktHdr->outer_udph->len =  htons(len_without_l2 + total_tail_insert_len + total_header_insert_len -20 );

		insertOff += sizeof(struct udphdr);
	}
	
	
	seqno 	= fc_db.fc_ipsec_info[hook_table_index].seq_no;//;
	spi		= fc_db.fc_ipsec_info[hook_table_index].spi;//;
	if(seqno == 0)
	{	
		fc_db.fc_ipsec_info[hook_table_index].seq_no+=1;
		seqno = 1;
	}
	
	pPktHdr->esph = (struct ip_esp_hdr *)(pData + insertOff);
	pPktHdr->esph->spi = (spi);
	pPktHdr->esph->seq_no = htonl(seqno);

	insertOff += sizeof(struct ip_esp_hdr);

	if(isESN)
	{
		
		seqno_hi = fc_db.fc_ipsec_info[hook_table_index].seq_no_hi;
		if(mode == RTK_FC_EALG_AES_GCM_ICV12)
		{
			memcpy( ((u8*)(pPktHdr->esph))+8, &seqno_hi, 4);
			insertOff += 4;
		}
		if(seqno == 0xffffffff)
		{
			fc_db.fc_ipsec_info[hook_table_index].seq_no_hi+=1;
		}
	}
	
	fc_db.fc_ipsec_info[hook_table_index].seq_no +=1;
	IPSEC("[SHORTCUT]Use %d as seq_num, and update to %d @%s(%d)\n", seqno, fc_db.fc_ipsec_info[hook_table_index].seq_no, __func__,__LINE__);

	if(mode == RTK_FC_EALG_AESCBC)
	{
		// Start insert iph, esph
		{
			do {
				u64 a;

				memcpy(&a, fc_db.fc_ipsec_info[hook_table_index].salt_in + ivsize - 8, 8);

				a |= 1;
				a *= seqno;

				memcpy(&tmp[0] + ivsize - 8, &a, 8);
			} while ((ivsize -= 8));

			memcpy(pData + insertOff, &tmp[0], 16);
			
		}
	}
	else if(mode == RTK_FC_EALG_DESCBC || mode == RTK_FC_EALG_3DESCBC)
	{
		// Start insert iph, esph
		{
			do {
				u64 a;

				memcpy(&a, fc_db.fc_ipsec_info[hook_table_index].salt_in + ivsize - 8, 8);

				a |= 1;
				a *= seqno;

				memcpy(&tmp[0] + ivsize - 8, &a, 8);
			} while ((ivsize -= 8));

			memcpy(pData + insertOff, &tmp[0], 8);
			
		}
	}
	else if(mode == RTK_FC_EALG_AES_GCM_ICV12)
	{
		u8*  iv;
		uint32 _seq_no = ntohl(pPktHdr->esph->seq_no);

		iv = RTK_FC_HELPER_FC_KMALLOC(ivsize,GFP_ATOMIC);
		memcpy(iv+4, (u8 *)&_seq_no, sizeof(uint32));
		
		//nonce-> key last 4 bytes
		//iv -> increment from 1, and do crypto xor
		rtk_fc_crypto_xor(iv,  fc_db.fc_ipsec_info[hook_table_index].salt_in, ivsize);
	
		memcpy(pData + insertOff, iv, ivsize);
		wmb();

		RTK_FC_HELPER_FC_KFREE(iv, ivsize);

	}

	do {
		int j;
		for (j = 0; j < plen - 2; j++)
			tail[j] = j + 1;
	} while (0);
	tail[plen - 2] = plen - 2;
	tail[plen - 1] = IPPROTO_IPIP;//tunnel mode (IP_IN IP)
	
	memcpy(pData+RTSKB_LEN(rtskb), tail,total_tail_insert_len);
	RTK_FC_HOOK_PS_SKB_SKB_PUT(RTSKB_SKB(rtskb), total_tail_insert_len,&pData);
	
	if(isESN && mode == RTK_FC_EALG_AESCBC)
	{
		memcpy(pData+RTSKB_LEN(rtskb)+total_tail_insert_len, &seqno_hi,4); // put the seq_no_hi after ESP trailer
		DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb)+4, "[AES CBC ESN]after put seq_hi to tail");
	}
#if defined(IPSEC_DEBUG)
	//dump_packet(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "ShortCut add trailers");
	IPSEC("Final pData = %p, skb:%p skb->data:%p, skb->len:%d",pData,RTSKB_SKB(rtskb),RTSKB_DATA(rtskb), RTSKB_LEN(rtskb));
	DUMP_PACKET(RTSKB_DATA(rtskb), RTSKB_LEN(rtskb), "after add headers");
#endif
	if(pPktHdr->udph)
		pPktHdr->udph = NULL;
	if(pPktHdr->tcph)
		pPktHdr->tcph = NULL;
	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int rtk_fc_ipsec_test_set_dest_desc(uint32 cryptlen, u8 *pData,rtk_fc_pktHdr_t *pPktHdr)
{


	rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;

	dst_crypto_desc.ctrl.type2.wrd = aes_cbc_decrypt_dest_desc.ctrl.type2.wrd;
	dst_crypto_desc.ctrl.type2.bits.enl = cryptlen;
	dst_crypto_desc.ddbp = fc_db.ipsec_addr.plain_text_dma;

	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, dst_crypto_desc.ctrl.type2.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, dst_crypto_desc.ddbp);
	wmb();

	return SUCCESS;
}

__IRAM_FC_SHORTCUT
static int rtk_fc_ipsec_decrypt_shortCut_set_dest_desc(uint32 cryptlen, u8 *pData,rtk_fc_pktHdr_t *pPktHdr)
{

	uint32_t addr;

	aes_cbc_decrypt_dest_desc.ctrl.type2.bits.enl = cryptlen;
	addr = dma_map_single(&(ni_info_data.pdev->dev), pData, cryptlen, DMA_TO_DEVICE);;//fc_db.ipsec_addr.plain_text_dma;

	
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, aes_cbc_decrypt_dest_desc.ctrl.type2.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, addr);

	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int rtk_fc_ipsec_decrypt_set_dest_desc(uint32 cryptlen)
{
		
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type2.wrd = 0;
    dst_crypto_desc.ctrl.type2.bits.ws  = 1;
    dst_crypto_desc.ctrl.type2.bits.enc = 1;
    dst_crypto_desc.ctrl.type2.bits.fs  = 1;
    dst_crypto_desc.ctrl.type2.bits.enl = cryptlen;
    dst_crypto_desc.ctrl.type2.bits.ls  = 1;
	IPSEC("Ciphertext: cryptlen = %d, dst_crypto_desc.ctrl.type2.wrdd = %x", cryptlen, dst_crypto_desc.ctrl.type2.wrd);
	
  // 	dst_crypto_desc.ddbp = dma_map_single(&(ni_info_data.pdev->dev), dest_cipher_buffer, cryptlen, DMA_TO_DEVICE);
	dst_crypto_desc.ddbp = fc_db.ipsec_addr.plain_text_dma;

		
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	
	return SUCCESS;
}


__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_encrypt_set_dest_desc_for_esn_test(uint32 plaintext_length)
{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type2.wrd = 0;
    dst_crypto_desc.ctrl.type2.bits.ws  = 1;
    dst_crypto_desc.ctrl.type2.bits.enc = 1;
    dst_crypto_desc.ctrl.type2.bits.fs  = 1;
    dst_crypto_desc.ctrl.type2.bits.enl = plaintext_length; // Alan ==> 填payload + iv (48 + 16)
    dst_crypto_desc.ctrl.type2.bits.ls  = 0;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext: plaintext_length = %d, dst_crypto_desc.ctrl.type2.wrdd = %x", plaintext_length, dst_crypto_desc.ctrl.type2.wrd);
#endif	
   	dst_crypto_desc.ddbp = fc_db.ipsec_addr.plain_text_dma;
	//dst_crypto_desc.ddbp = dma_map_single(&(ni_info_data.pdev->dev), address, plaintext_length, DMA_TO_DEVICE);;//fc_db.ipsec_addr.plain_text_dma;


		
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_encrypt_set_dest_desc(uint32 plaintext_length, u8 *address)
{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type2.wrd = 0;
    dst_crypto_desc.ctrl.type2.bits.ws  = 1;
    dst_crypto_desc.ctrl.type2.bits.enc = 1;
    dst_crypto_desc.ctrl.type2.bits.fs  = 1;
    dst_crypto_desc.ctrl.type2.bits.enl = plaintext_length;
    dst_crypto_desc.ctrl.type2.bits.ls  = 0;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext: plaintext_length = %d, dst_crypto_desc.ctrl.type2.wrdd = %x", plaintext_length, dst_crypto_desc.ctrl.type2.wrd);
#endif	
   	//dst_crypto_desc.ddbp = fc_db.ipsec_addr.plain_text_dma;
	dst_crypto_desc.ddbp = dma_map_single(&(ni_info_data.pdev->dev), address, plaintext_length, DMA_TO_DEVICE);;//fc_db.ipsec_addr.plain_text_dma;

	if (dma_mapping_error(&(ni_info_data.pdev->dev),dst_crypto_desc.ddbp)) {
		printk("%s(%d):dma %d bytes error\n",__func__, __LINE__, plaintext_length);
		//return FAILED;
		
	}


		
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(uint32 plaintext_length, u8 *addr, rtk_fc_pktHdr_t *pPktHdr)
{
	uint32_t addrs;
	//dst_crypto_desc.ctrl.type2.wrd = aes_cbc_encrypt_dest_desc.ctrl.type2.wrd;
	aes_cbc_encrypt_dest_desc.ctrl.type2.bits.enl = plaintext_length;
	addrs = dma_map_single(&(ni_info_data.pdev->dev), addr, plaintext_length, DMA_TO_DEVICE);;//fc_db.ipsec_addr.plain_text_dma;
	
	wmb();

	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, aes_cbc_encrypt_dest_desc.ctrl.type2.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, addrs);
	
	return SUCCESS;
}

void _rtk_fc_ipsec_aescbc_iv_create(uint32 saInfo_index, uint8 *pIv, uint32 ivsize, uint32 esp_seq_no)
{
	do {
		uint64 a;

		memcpy(&a, fc_db.fc_ipsec_info[saInfo_index].salt_in + ivsize - 8, 8);

		a |= 1;
		a *= esp_seq_no;

		memcpy(pIv + ivsize - 8, &a, 8);
	} while ((ivsize -= 8));
	
	return;
}

int _rtk_fc_ipsec_aescbc_esn_set_cipher_dest_desc(uint32 crypt_len, u8 *data)
{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type2.wrd = 0;
    dst_crypto_desc.ctrl.type2.bits.ws  = 1;
    dst_crypto_desc.ctrl.type2.bits.enc = 1;
    dst_crypto_desc.ctrl.type2.bits.fs  = 1;
    dst_crypto_desc.ctrl.type2.bits.enl = crypt_len; // Alan ==> 填payload + iv (48 + 16)
    dst_crypto_desc.ctrl.type2.bits.ls  = 1;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext:  dst_crypto_desc.ctrl.type2.wrdd = %x", dst_crypto_desc.ctrl.type2.wrd);
#endif	
   	dst_crypto_desc.ddbp = dma_map_single(&(ni_info_data.pdev->dev), data, crypt_len, DMA_TO_DEVICE);
    
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}

int _rtk_fc_ipsec_aescbc_esn_set_hashresult_desc(uint32 icv_len, u8 *addr)

{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type1.wrd = 0;
    dst_crypto_desc.ctrl.type1.bits.ws  = 1;
    dst_crypto_desc.ctrl.type1.bits.enc = 0;
    dst_crypto_desc.ctrl.type1.bits.fs  = 1;
    dst_crypto_desc.ctrl.type1.bits.adl = icv_len; // Alan ==> 填payload + iv (48 + 16)
    dst_crypto_desc.ctrl.type1.bits.ls  = 1;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext:  dst_crypto_desc.ctrl.type2.wrdd = %x", dst_crypto_desc.ctrl.type2.wrd);
#endif	
   dst_crypto_desc.ddbp = dma_map_single(&(ni_info_data.pdev->dev), addr, icv_len, DMA_TO_DEVICE); //fc_db.ipsec_addr.icv_dma;// 
    
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}

void  _rtk_fc_ipsec_aescbc_esn_set_hash_key(u8 *key_pad)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.pl=128/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), key_pad, 128, DMA_TO_DEVICE);
	
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}



void  _rtk_fc_ipsec_aescbc_esn_hash_key_iv(u8 *key_pad)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.pl=128/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), key_pad, 128, DMA_TO_DEVICE);
	
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}
void  _rtk_fc_ipsec_aescbc_esn_cipher_key(int key_size, u8 *addr)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.rs=1;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.kl=key_size/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), addr, key_size, DMA_TO_DEVICE);//fc_db.ipsec_addr.key_dma;
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}

void  _rtk_fc_ipsec_aescbc_esn_cipher_iv(int iv_size, u8 *addr)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.rs=1;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.il=iv_size/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), addr, iv_size, DMA_TO_DEVICE);//fc_db.ipsec_addr.iv_dma;

	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}

int _rtk_fc_ipsec_aescbc_esn_set_cipheronly_cmdsetting(uint32 cryptlen, uint32 key_index)
{
	{
	    rtk_fc_cryptoEngine_source_desc_t src_desc;
	    rtk_fc_cryptoEngine_command_t *cmd_ptr;

	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)fc_db.ipsec_addr.cmd_setting_ptr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->wrd0.bits.em = 0; //RTK_EM_CRYPTO
		cmd_ptr->wrd0.bits.icvtl = 0x40;
		cmd_ptr->wrd1.bits.cabs = 1;
		cmd_ptr->wrd0.bits.ces = 0;//RTK_CES_AES
		cmd_ptr->wrd0.bits.cm = 1;//RTK_CM_AES_CBC
		cmd_ptr->wrd0.bits.ce = 1;//RTK_CE_CRYPT
		cmd_ptr->wrd0.bits.aks = 2;//RTK_AKS_256
		cmd_ptr->wrd1.bits.elds = cryptlen % 16;
	 	cmd_ptr->wrd2.bits.etl = (cryptlen + 15)/16;; //16 bytes as a unit

	  	rtk_fc_hexdump((unsigned char *)cmd_ptr, sizeof(rtk_fc_cryptoEngine_command_t));
	    
	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
	    src_desc.ctrl.type1.bits.cl = 3;
	 	
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
#if defined(IPSEC_DEBUG)	    
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif

		
	    src_desc.sdbp = fc_db.ipsec_addr.cmd_setting_dma;//dma_map_single(&(ni_info_data.pdev->dev), src_cmd_setting_iv_buffer, 12+16, DMA_TO_DEVICE);
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);

	}
	return SUCCESS;
}

int _rtk_fc_ipsec_aescbc_esn_hashonly_cmdsetting(uint32 hash_len, uint32 key_index, u8 *addr)
{
	{
	    rtk_fc_cryptoEngine_source_desc_t src_desc;
	    rtk_fc_cryptoEngine_command_t *cmd_ptr;

	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)addr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->wrd0.bits.em = 1; //RTK_EM_HASH

	  	cmd_ptr->wrd0.bits.icvtl = 0x40;
		cmd_ptr->wrd0.bits.sh = 1;
		cmd_ptr->wrd0.bits.shf = 1;
		cmd_ptr->wrd0.bits.shl = 1;
		cmd_ptr->wrd1.bits.habs = 1;
		cmd_ptr->wrd0.bits.hm = 1;//RTK_HM_SHA1;
		cmd_ptr->wrd0.bits.he = 1;//RTK_HE_ENABLE;
		
		cmd_ptr->wrd2.bits.etl = (hash_len + 15)/16;
		cmd_ptr->wrd1.bits.elds = hash_len % 16;
		cmd_ptr->apl1 =  (hash_len+64) << 3;
	  
		
	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
	    src_desc.ctrl.type1.bits.cl = 3;
	    src_desc.ctrl.type1.bits.ap = 1;
		
	    src_desc.ctrl.type1.bits.sk = 1;
	    src_desc.ctrl.type1.bits.kl = key_index;//secure key idx 0
	    src_desc.ctrl.type1.bits.pk = 1;
	    src_desc.ctrl.type1.bits.pl = key_index;//secure keypad idx 0
	    
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
#if defined(IPSEC_DEBUG)	    
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif

		
	    src_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), addr, 28/*12+16*/, DMA_TO_DEVICE);//fc_db.ipsec_addr.cmd_setting_dma;//
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);

	}
	return SUCCESS;
}

int _rtk_fc_ipsec_aescbc_mixmode_hmacMD5_set_cmdSetting(uint32 ivlen, uint32 plaintext_length, uint32 hash_padding_len, uint32 aad_len, rtk_fc_pktHdr_t *pPktHdr, uint32 key_index, int key_size, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr)
{
	//rtk_fc_cryptoEngine_source_desc_t src_desc;
	uint32_t addr;
	//memcpy(pCmdSetting, &aes_cbc_encrypt_cmmd_setting, sizeof(rtk_fc_cryptoEngine_command_t));
	pCmdSetting->wrd0 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd0;
	pCmdSetting->wrd1 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd1;
	pCmdSetting->wrd2 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd2;
	if(key_size == 32)
	    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
	else if(key_size == 16)
		pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
	else if(key_size == 24)
		pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192

	pCmdSetting->wrd2.bits.etl = (plaintext_length + 15)/16;
	pCmdSetting->wrd1.bits.elds = plaintext_length % 16;
	pCmdSetting->wrd2.bits.htl = (aad_len + 7)/8;//aad_len
	pCmdSetting->wrd1.bits.aadlds = aad_len % 8;
	pCmdSetting->wrd2.bits.hptl = (hash_padding_len + 7)/8; //hash padding len, 8 bytes alighment
	pCmdSetting->wrd1.bits.plds = hash_padding_len % 8;

	//src_desc.ctrl.type1.wrd = aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd;
	aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
	aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = key_index;
	aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.il = ivlen/4;

	//integrate cmd setting + iv
	memcpy(((u8 *)pCmdSetting)+12, ((u8 *)(pPktHdr->esph)+8), 16);

	addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
	*cmdSetting_dmaPtr = addr;
	
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
	wmb();
	return SUCCESS;
}

int _rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(int hash_mode, int cipher_mode, uint32 ivlen, uint32 plaintext_length, uint32 hash_padding_len, uint32 aad_len, rtk_fc_pktHdr_t *pPktHdr, u8 key_index, u8 hash_key_index, int key_size, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr)
{
	//rtk_fc_cryptoEngine_source_desc_t src_desc;
	uint32_t addr;
	//memcpy(pCmdSetting, &aes_cbc_encrypt_cmmd_setting, sizeof(rtk_fc_cryptoEngine_command_t));
	if(hash_mode ==RTK_FC_AALG_SHA1HMAC || hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
	{
		IPSEC("plaintext_length = %d, aad_len = %d hash_padding_len = %d key_index = %d hash_key_index = %d ivlen = %d", plaintext_length, aad_len,hash_padding_len, key_index, hash_key_index , ivlen);
		pCmdSetting->wrd0 = des_cbc_encrypt_cmmd_setting.wrd0;
		pCmdSetting->wrd1 = des_cbc_encrypt_cmmd_setting.wrd1;
		pCmdSetting->wrd2 = des_cbc_encrypt_cmmd_setting.wrd2;

		//if(key_size == 32)
		//    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		//else if(key_size == 16)
		//	pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		//else if(key_size == 24)
		//	pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192
		
		
		if(hash_mode ==RTK_FC_AALG_SHA1HMAC)
			pCmdSetting->wrd0.bits.hm = RTK_FC_HM_SHA1;
		else if(hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
			pCmdSetting->wrd0.bits.hm = RTK_FC_HM_SHA2_256;


		if(cipher_mode==RTK_FC_EALG_3DESCBC)
			pCmdSetting->wrd0.bits.tride =1 ;

		pCmdSetting->wrd2.bits.etl = (plaintext_length + 7)/8; // DES_BLK_SIZE = 8
		pCmdSetting->wrd1.bits.elds = plaintext_length % 8;
		pCmdSetting->wrd2.bits.htl = (aad_len + 7)/8;//aad_len
		pCmdSetting->wrd1.bits.aadlds = aad_len % 8;
		pCmdSetting->wrd2.bits.hptl = (hash_padding_len + 7)/8; //hash padding len, 8 bytes alighment
		pCmdSetting->wrd1.bits.plds = hash_padding_len % 8;

		//src_desc.ctrl.type1.wrd = aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = hash_key_index;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.il = ivlen/4;

		//integrate cmd setting + iv
		memcpy(((u8 *)pCmdSetting)+12, ((u8 *)(pPktHdr->esph)+8), 16);

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;
		
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, des_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();

	}
	else if(hash_mode == RTK_FC_AALG_MD5HMAC)
	{

		pCmdSetting->wrd0 = des_cbc_encrypt_cmmd_setting_hmacmd5.wrd0;
		pCmdSetting->wrd1 = des_cbc_encrypt_cmmd_setting_hmacmd5.wrd1;
		pCmdSetting->wrd2 = des_cbc_encrypt_cmmd_setting_hmacmd5.wrd2;

		//if(key_size == 32)
		//    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		//else if(key_size == 16)
		//	pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		//else if(key_size == 24)
		//	pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192

		if(cipher_mode==RTK_FC_EALG_3DESCBC)
			pCmdSetting->wrd0.bits.tride =1 ;

		pCmdSetting->wrd2.bits.etl = (plaintext_length + 7)/8; // DES_BLK_SIZE = 8
		pCmdSetting->wrd1.bits.elds = plaintext_length % 8;
		pCmdSetting->wrd2.bits.htl = (aad_len + 7)/8;//aad_len
		pCmdSetting->wrd1.bits.aadlds = aad_len % 8;
		pCmdSetting->wrd2.bits.hptl = (hash_padding_len + 7)/8; //hash padding len, 8 bytes alighment
		pCmdSetting->wrd1.bits.plds = hash_padding_len % 8;

		//src_desc.ctrl.type1.wrd = aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = hash_key_index;
		des_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.il = ivlen/4;

		//integrate cmd setting + iv
		memcpy(((u8 *)pCmdSetting)+12, ((u8 *)(pPktHdr->esph)+8), 16);

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;
		
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, des_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();

	}
	return SUCCESS;
}

int _rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(int hash_mode, uint32 ivlen, uint32 plaintext_length, uint32 hash_padding_len, uint32 aad_len, rtk_fc_pktHdr_t *pPktHdr, u8 key_index, u8 hash_key_index, int key_size, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr)
{
	//rtk_fc_cryptoEngine_source_desc_t src_desc;
	uint32_t addr;
	//memcpy(pCmdSetting, &aes_cbc_encrypt_cmmd_setting, sizeof(rtk_fc_cryptoEngine_command_t));
	if(hash_mode ==RTK_FC_AALG_SHA1HMAC || hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
	{
		//IPSEC("hash mode = %d key_size = %d",hash_mode,key_size);
		pCmdSetting->wrd0 = aes_cbc_encrypt_cmmd_setting.wrd0;
		pCmdSetting->wrd1 = aes_cbc_encrypt_cmmd_setting.wrd1;
		pCmdSetting->wrd2 = aes_cbc_encrypt_cmmd_setting.wrd2;
		if(key_size == 32)
		    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		else if(key_size == 16)
			pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		else if(key_size == 24)
			pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192
		
		
		if(hash_mode ==RTK_FC_AALG_SHA1HMAC)
			pCmdSetting->wrd0.bits.hm = RTK_FC_HM_SHA1;
		else if(hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
			pCmdSetting->wrd0.bits.hm = RTK_FC_HM_SHA2_256;

		pCmdSetting->wrd2.bits.etl = (plaintext_length + 15)/16;
		pCmdSetting->wrd1.bits.elds = plaintext_length % 16;
		pCmdSetting->wrd2.bits.htl = (aad_len + 7)/8;//aad_len
		pCmdSetting->wrd1.bits.aadlds = aad_len % 8;
		pCmdSetting->wrd2.bits.hptl = (hash_padding_len + 7)/8; //hash padding len, 8 bytes alighment
		pCmdSetting->wrd1.bits.plds = hash_padding_len % 8;

		//src_desc.ctrl.type1.wrd = aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = hash_key_index;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.il = ivlen/4;

		//integrate cmd setting + iv
		memcpy(((u8 *)pCmdSetting)+12, ((u8 *)(pPktHdr->esph)+8), 16);

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;
		
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();

	}
	else if(hash_mode == RTK_FC_AALG_MD5HMAC)
	{

		pCmdSetting->wrd0 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd0;
		pCmdSetting->wrd1 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd1;
		pCmdSetting->wrd2 = aes_cbc_encrypt_cmmd_setting_hmacmd5.wrd2;
		if(key_size == 32)
		    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		else if(key_size == 16)
			pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		else if(key_size == 24)
			pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192

		pCmdSetting->wrd2.bits.etl = (plaintext_length + 15)/16;
		pCmdSetting->wrd1.bits.elds = plaintext_length % 16;
		pCmdSetting->wrd2.bits.htl = (aad_len + 7)/8;//aad_len
		pCmdSetting->wrd1.bits.aadlds = aad_len % 8;
		pCmdSetting->wrd2.bits.hptl = (hash_padding_len + 7)/8; //hash padding len, 8 bytes alighment
		pCmdSetting->wrd1.bits.plds = hash_padding_len % 8;

		//src_desc.ctrl.type1.wrd = aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = hash_key_index;
		aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.bits.il = ivlen/4;

		//integrate cmd setting + iv
		memcpy(((u8 *)pCmdSetting)+12, ((u8 *)(pPktHdr->esph)+8), 16);

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;
		
		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_encrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();

	}
	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_descbc_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, u8 *src_aad_plaintext_hash_padding_buffer, uint32 total_len)
{
    rtk_fc_cryptoEngine_source_desc_t  src_cipher_desc;
	
	//for aad + plaintext + hash_padding
	src_cipher_desc.ctrl.type2.wrd = 0;
    src_cipher_desc.ctrl.type2.bits.rs = 1;
    src_cipher_desc.ctrl.type2.bits.a2eo_epl = aadlen;
    src_cipher_desc.ctrl.type2.bits.enl = plaintext_length; 
    src_cipher_desc.ctrl.type2.bits.apl = hash_padding_len; 
    src_cipher_desc.ctrl.type2.bits.ls = 1;
#if defined(IPSEC_DEBUG)
	IPSEC("aadlen = %d, plaintext_length = %d, hash_padding_len = %d",aadlen, plaintext_length, hash_padding_len);
	IPSEC("MIX mode: aad + plaintext + hash_padding = 0x%x",src_cipher_desc.ctrl.type2.wrd);
#endif

  	
   	memcpy(src_aad_plaintext_hash_padding_buffer,  (u8 *)(pPktHdr->esph), aadlen);
	
	
   	memcpy(src_aad_plaintext_hash_padding_buffer+aadlen, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr), plaintext_length); //OK
	if(hash_mode ==RTK_FC_AALG_SHA1HMAC || hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
		memcpy(src_aad_plaintext_hash_padding_buffer+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_des_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
	else if(hash_mode ==RTK_FC_AALG_MD5HMAC )
   		memcpy(src_aad_plaintext_hash_padding_buffer+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
#if defined(IPSEC_DEBUG)
	IPSEC("MIX mode: aad");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (aadlen));

	IPSEC("MIX mode:aad+ plaintext(INCLUDE IV)");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (plaintext_length+aadlen));
		
	IPSEC("MIX mode:aad+ plaintext + hash_padding");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (plaintext_length+hash_padding_len+aadlen));
#endif
	src_cipher_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), src_aad_plaintext_hash_padding_buffer, total_len, DMA_TO_DEVICE);
	
	
	//rtk_fc_reg_io_write32 (pdev->regmap + RTK_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_cipher_desc.ctrl) );
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_cipher_desc.sdbp);

	
	return SUCCESS;
}

__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_aescbc_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, u8 *src_aad_plaintext_hash_padding_buffer, uint32 total_len)
{
    rtk_fc_cryptoEngine_source_desc_t  src_cipher_desc;
	
	//for aad + plaintext + hash_padding
	src_cipher_desc.ctrl.type2.wrd = 0;
    src_cipher_desc.ctrl.type2.bits.rs = 1;
    src_cipher_desc.ctrl.type2.bits.a2eo_epl = aadlen;
    src_cipher_desc.ctrl.type2.bits.enl = plaintext_length; 
    src_cipher_desc.ctrl.type2.bits.apl = hash_padding_len; 
    src_cipher_desc.ctrl.type2.bits.ls = 1;
#if defined(IPSEC_DEBUG)
	IPSEC("aadlen = %d, plaintext_length = %d, hash_padding_len = %d",aadlen, plaintext_length, hash_padding_len);
	IPSEC("MIX mode: aad + plaintext + hash_padding = 0x%x",src_cipher_desc.ctrl.type2.wrd);
#endif

  	
   	memcpy(src_aad_plaintext_hash_padding_buffer,  (u8 *)(pPktHdr->esph), aadlen);
	
	
   	memcpy(src_aad_plaintext_hash_padding_buffer+aadlen, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr), plaintext_length); //OK
	if(hash_mode ==RTK_FC_AALG_SHA1HMAC || hash_mode ==RTK_FC_AALG_SHA2_256HMAC)
		memcpy(src_aad_plaintext_hash_padding_buffer+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
	else if(hash_mode ==RTK_FC_AALG_MD5HMAC )
   		memcpy(src_aad_plaintext_hash_padding_buffer+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_md5_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
#if defined(IPSEC_DEBUG)
	IPSEC("MIX mode: aad");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (aadlen));

	IPSEC("MIX mode:aad+ plaintext(INCLUDE IV)");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (plaintext_length+aadlen));
		
	IPSEC("MIX mode:aad+ plaintext + hash_padding");
	rtk_fc_hexdump(src_aad_plaintext_hash_padding_buffer, (plaintext_length+hash_padding_len+aadlen));
#endif
	src_cipher_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), src_aad_plaintext_hash_padding_buffer, total_len, DMA_TO_DEVICE);
	
	
	//rtk_fc_reg_io_write32 (pdev->regmap + RTK_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_cipher_desc.ctrl) );
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_cipher_desc.sdbp);

	
	return SUCCESS;
}


__IRAM_FC_SHORTCUT
static int _rtk_fc_ipsec_aescbc_shortCut_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, uint32 total_len)
{
	uint32_t addr;

    aes_cbc_encrypt_data_src.ctrl.type2.bits.a2eo_epl = aadlen;
    aes_cbc_encrypt_data_src.ctrl.type2.bits.enl = plaintext_length; 
    aes_cbc_encrypt_data_src.ctrl.type2.bits.apl = hash_padding_len; 
	if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
		memcpy((u8 *)(pPktHdr->esph)+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
  	else if (hash_mode == RTK_FC_AALG_MD5HMAC)
		memcpy((u8 *)(pPktHdr->esph)+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_md5_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
	addr = dma_map_single(&(ni_info_data.pdev->dev), (u8 *)(pPktHdr->esph), total_len, DMA_TO_DEVICE);
	
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_cbc_encrypt_data_src.ctrl.type2.wrd );
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);

	return SUCCESS;
}



__IRAM_FC_SHORTCUT
static int _rtk_fc_ipsec_descbc_shortCut_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, uint32 total_len)
{
	uint32_t addr;
	IPSEC("plaintext_length = %d, aad_len = %d hash_padding_len = %d total_len = %d", plaintext_length, aadlen,hash_padding_len, total_len);
			
    des_cbc_encrypt_data_src.ctrl.type2.bits.a2eo_epl = aadlen;
    des_cbc_encrypt_data_src.ctrl.type2.bits.enl = plaintext_length; 
    des_cbc_encrypt_data_src.ctrl.type2.bits.apl = hash_padding_len; 
	if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC){
		memcpy((u8 *)(pPktHdr->esph)+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_des_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
  	}else if (hash_mode == RTK_FC_AALG_MD5HMAC){
		memcpy((u8 *)(pPktHdr->esph)+aadlen+plaintext_length, fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[hash_padding_array_idx].hash_padding, hash_padding_len); //OK
  	}
		
	addr = dma_map_single(&(ni_info_data.pdev->dev), (u8 *)(pPktHdr->esph), total_len, DMA_TO_DEVICE);
	wmb();
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, des_cbc_encrypt_data_src.ctrl.type2.wrd );
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);

	return SUCCESS;
}



__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_ipsec_cryptEngine_check_err(void)
{
	int ret = SUCCESS;
	int err = 0, cmdok = 0;
	uint32 saadlr = 0;
	uint32 cmdstat  = 0;
	
	do{
		
		cmdstat = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
		cmdok = cmdstat & (1UL << BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK);
		err = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT);
		if( cmdok >0 )
		{
			IPSEC("%s:%d \033[1;31;40m *********************cmdOK!! ********************* \033[m\n", __FUNCTION__, __LINE__);
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat );
			ret = SUCCESS;
			break;
		}
		if(err >0){
			printk ("%s:%d \033[1;31;40m *********************err %x ********************* \033[m\n", __FUNCTION__, __LINE__, err);
			
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT, err);
			if (err & 0x40)
			{
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x20L);
				printk("sum of a2eo = 0x%x\n",saadlr&0x7ff);

			}
			if (err & 0x100)
			{
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x28L);
				printk("sum of apl = 0x%x\n",saadlr&0x7ff);

			}
			if (err & 0x80)
			{
			
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
				printk("sum of etl = 0x%x\n",saadlr&0x7ff);

			}
			if (err & 0x2000)
			{
			
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
				printk("sum of etl = 0x%x\n",saadlr&0x7ff);

			}
			ret = FAILED;
			break;
		}
		else if (!err)
			break;
		
		
	}while(1);

	return ret;
}



__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_ipsec_cryptEngine_polling_result(void)
{
	int countdown = 100000000;
	u64 count = 0;
	int ret = SUCCESS;
	int err = 0, cmdok = 0;
	uint32 saadlr = 0;
	uint32 cmdstat  = 0;
	
	do{
		
		cmdstat = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
		cmdok = cmdstat & (1UL << BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK);
		count++;
		if( cmdok >0 )
		{
			IPSEC("%s:%d \033[1;31;40m *********************cmdOK!! ********************* \033[m\n", __FUNCTION__, __LINE__);
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat );
			ret = SUCCESS;
			break;
		}
		else
		{
			err = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT);
			if(err >0){
				printk ("%s:%d \033[1;31;40m *********************err %x ********************* \033[m\n", __FUNCTION__, __LINE__, err);
				
				rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT, err);
				if (err & 0x40)
				{
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x20L);
					printk("sum of a2eo = 0x%x\n",saadlr&0x7ff);

				}
				if (err & 0x100)
				{
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x28L);
					printk("sum of apl = 0x%x\n",saadlr&0x7ff);

				}
				if (err & 0x80)
				{
				
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
					printk("sum of etl = 0x%x\n",saadlr&0x7ff);

				}
				if (err & 0x2000)
				{
				
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
					printk("sum of etl = 0x%x\n",saadlr&0x7ff);

				}
				ret = FAILED;
				break;
			}
		}
		
		
		countdown --;
		if(countdown ==0 ){
			printk("count down over\n");
			ret = FAILED;
			break;
		}
	}while(1);
	IPSEC("count = %llu",count);
	return ret;
}



__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_des_cbc_mix_mode_decrypt_ipi(int ring_index, int cipher_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint32 cryptlen, int ipsec_shortCut_info_table_index)
{
	uint32 key_index, hash_key_index;
	int ret = SUCCESS;
	int cryp_padding_len;
	int DES_BLOCK_SIZE = 8, key_size;
	u8 *pData=(u8 *)pPktHdr->iph;
	//u8 iv[16] = {0};
	//u8 cmdsetting[28] = {0};
	uint32_t iv_dma = 0;
	uint32_t cmdsetting_dmaPtr = 0;
	
	key_index = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;
	hash_key_index = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	key_size = pPktHdr->ipsec_sc_info.key_size;
	IPSEC("[Decrypt]cryptlen = %d key_index = %d, ivsize = %d cipher_mode = %d",cryptlen, key_index,ivsize,cipher_mode);
		
	
	//1. dest descriptor
	rtk_fc_ipsec_decrypt_shortCut_set_dest_desc(cryptlen,pData, pPktHdr);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	//2. command-setting
	rtk_fc_ipsec_decrypt_des_set_cmdSetting(cryptlen, cipher_mode, ivsize, key_index, hash_key_index, key_size , cmdsetting_ptr[ring_index], pPktHdr, &cmdsetting_dmaPtr);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma     = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else
	//2. command-setting
	rtk_fc_ipsec_decrypt_des_set_cmdSetting(cryptlen, cipher_mode, ivsize, key_index, hash_key_index, key_size , &cmdsetting_ptr[ring_index], pPktHdr, &cmdsetting_dmaPtr);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma     = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif
	//3. key iv
	
	memcpy(&iv_ptr[ring_index][0], ((u8 *)(pPktHdr->esph)+8), ivsize);
	
	_rtk_fc_ipsec_shortCut_set_iv_src_desc(ivsize, key_index, hash_key_index, &iv_ptr[ring_index][0], pPktHdr, &iv_dma);

	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_dma     = iv_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_pointer = &iv_ptr[ring_index][0];
	
	//ret = _rtk_fc_ipsec_cryptEngine_polling_result();
	//4. src descriptor
	
	cryp_padding_len = (DES_BLOCK_SIZE - (cryptlen % DES_BLOCK_SIZE)) % DES_BLOCK_SIZE;

	IPSEC("cryp_padding_len  = %d cryptlen = %d",cryp_padding_len , cryptlen);
	_rtk_fc_ipsec_shortCut_set_src_desc(cryptlen, cryp_padding_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+ivsize, pPktHdr);
	

	return ret;
}

__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_mix_mode_decrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint32 cryptlen, int ipsec_shortCut_info_table_index)
{
	uint32 key_index, hash_key_index;
	int ret = SUCCESS;
	int cryp_padding_len;
	int AES_BLOCK_SIZE = 16, key_size;
	u8 *pData=(u8 *)pPktHdr->iph;
	//u8 iv[16] = {0};
	//u8 cmdsetting[28] = {0};
	uint32_t iv_dma = 0;
	uint32_t cmdsetting_dmaPtr = 0;
	key_index = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;
	hash_key_index = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	key_size = pPktHdr->ipsec_sc_info.key_size;
	IPSEC("[Decrypt]cryptlen = %d key_index = %d, ivsize = %d",cryptlen, key_index,ivsize);
		
	
	//1. dest descriptor
	rtk_fc_ipsec_decrypt_shortCut_set_dest_desc(cryptlen,pData, pPktHdr);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	//2. command-setting
	rtk_fc_ipsec_decrypt_set_cmdSetting(cryptlen, ivsize, key_index, key_size , cmdsetting_ptr[ring_index], pPktHdr, &cmdsetting_dmaPtr);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma     = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else
	//2. command-setting
	rtk_fc_ipsec_decrypt_set_cmdSetting(cryptlen, ivsize, key_index, key_size , &cmdsetting_ptr[ring_index], pPktHdr, &cmdsetting_dmaPtr);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma     = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif
	//3. key iv
	
	memcpy(&iv_ptr[ring_index][0], ((u8 *)(pPktHdr->esph)+8), ivsize);
	
	_rtk_fc_ipsec_shortCut_set_iv_src_desc(ivsize, key_index, hash_key_index, &iv_ptr[ring_index][0], pPktHdr, &iv_dma);

	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_dma     = iv_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_pointer = &iv_ptr[ring_index][0];
	
	//ret = _rtk_fc_ipsec_cryptEngine_polling_result();
	//4. src descriptor
	
	cryp_padding_len = (AES_BLOCK_SIZE - (cryptlen % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE;
	
	_rtk_fc_ipsec_shortCut_set_src_desc(cryptlen, cryp_padding_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+ivsize, pPktHdr);
	

	return ret;
}


__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_ipsec_gcm_shortCut_decrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint32 cryptlen, int ipsec_shortCut_info_table_index)
{
	int ret = SUCCESS;
	uint32 key_index;
	//int origin_len = ntohs(pPktHdr->iph->tot_len);
	//int after_len = 0, len_diff = 0;
	int gcm_hw_iv_size = 16, aead_size = sizeof(struct ip_esp_hdr);
	int cryp_padding_len;
	int AES_BLOCK_SIZE = 16;
	u8 *pData = (u8 *)pPktHdr->iph;
	u8 gcm_iv_tail[4] = {0x00, 0x00, 0x00, 0x01};
	//u8 temp_final_iv[16] = {0};
	uint32_t iv_dma = 0, aead_dma = 0, cmdsetting_dmaPtr = 0;
	int key_size;
	
	key_index =  fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;//ipsec_shortCut_info_table_index;
	key_size = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;//ipsec_shortCut_info_table_index;
	
	IPSEC("[Decrypt]cryptlen = %d key_index = %d, ivsize = %d",cryptlen, key_index,ivsize);


	cryp_padding_len = (AES_BLOCK_SIZE - (cryptlen % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE;
	
	//1. dest descriptor
	//rtk_fc_ipsec_decrypt_set_dest_desc(cryptlen);
	rtk_fc_ipsec_decrypt_shortCut_set_dest_desc(cryptlen,pData,pPktHdr);

	//2. command-setting
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)	 
	_rtk_fc_ipsec_gcm_decrypt_set_cmdSetting(cryptlen, aead_size, key_index, cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
	
#else
	_rtk_fc_ipsec_gcm_decrypt_set_cmdSetting(cryptlen, aead_size, key_index, &cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif

	//_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(cryptlen, aead_size, key_index,0);
	memcpy(&iv_ptr[ring_index][0], fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].nonce, 4);

	memcpy(&iv_ptr[ring_index][0], fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].nonce, 4);
	memcpy( (&iv_ptr[ring_index][0])+4, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr), ivsize);
	memcpy((&iv_ptr[ring_index][0])+4+ivsize, gcm_iv_tail, 4);
	
#if defined(IPSEC_DEBUG)						
	IPSEC("IV with 16 bytes");
	rtk_fc_hexdump((&iv_ptr[ring_index][0]),gcm_hw_iv_size);
	
#endif
	//_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, &iv_ptr[ring_index][0]);
	_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, &iv_ptr[ring_index][0], &iv_dma);

	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_dma     = iv_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_pointer = &iv_ptr[ring_index][0];

	//memcpy(aead			, (u8 *)(pPktHdr->esph),aead_size);
	memcpy(&aead_ptr[ring_index][0], (u8 *)(pPktHdr->esph), aead_size);

	//_rtk_fc_ipsec_set_aad_src_desc(aead_size, (u8 *)(pPktHdr->esph));
    _rtk_fc_ipsec_set_aad_src_desc(aead_size, &aead_ptr[ring_index][0], &aead_dma);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_dma	   	= aead_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_pointer 	= &aead_ptr[ring_index][0];
	//4. src descriptor
	
   
	_rtk_fc_ipsec_set_src_desc(cryptlen, cryp_padding_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+ivsize);

	return ret;

}


__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_ipsec_gcm_shortCut_encrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index, uint8 isESN)
{
	int GCM_ICV_LEN = 16, cryp_pad_len = 0, gcm_hw_iv_size = 16;
	uint32 key_index;
	int ret = SUCCESS;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+ivsize;
	u8 gcm_iv_tail[4] = {0x00, 0x00, 0x00, 0x01};
	int AES_BLOCK_SIZE = 16;
	//u8 iv_tmp[8] = {0};
	//u8 final_iv_tmp[16] = {0};
	//u8 aad_tmp[12] = {0};
	uint16 aead_size = 0;
	uint32_t cmdsetting_dmaPtr = 0;
	int key_size = pPktHdr->ipsec_sc_info.key_size;
	uint32_t iv_dma = 0, aead_dma = 0;

	IPSEC("ipsec_shortCut_info_table_index = %d ring_index  = %d", ipsec_shortCut_info_table_index,ring_index);
	cryp_pad_len = (AES_BLOCK_SIZE - (pPktHdr->crypto_plaintext_length % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE;
	key_index 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;// ipsec_shortCut_info_table_index;

	if(isESN==0)
	{
		aead_size   = sizeof(struct ip_esp_hdr);
	}
	else
	{
		IPSEC("ESN mode!");
		pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
		aead_size   = 12;
	}
	//ivsize = 16

	IPSEC("isESN = %x crypt len:%d, ivsize: %d hash_padding; %d, crypt_padding: %d aead_size = %d key_size = %d,  key_index", isESN, pPktHdr->crypto_plaintext_length, ivsize,GCM_ICV_LEN-auth_sizes,cryp_pad_len,aead_size, key_size, key_index);
	
	/*
		Set cipher destination descriptor
	 */
	 _rtk_fc_ipsec_encrypt_set_dest_desc(pPktHdr->crypto_plaintext_length, pCipher_data);

	/*
		Set ICV
	 */
	_rtk_fc_ipsec_encrypt_set_icv_dest_desc(auth_sizes, GCM_ICV_LEN-auth_sizes, (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->ipsec_sc_info.ivsize+pPktHdr->crypto_plaintext_length);
	
	/*
		Command setting
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)	 
	_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(pPktHdr->crypto_plaintext_length, aead_size, key_index,1, cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];

#else
	_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(pPktHdr->crypto_plaintext_length, aead_size, key_index,1, &cmdsetting_ptr[ring_index], &cmdsetting_dmaPtr, key_size);
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif

	/*
		GCM IV setting: 16 bytes
		nonce (4) +  IV (8) + gcm_iv_tail  (4)
	*/
	//memcpy(&iv_ptr[ring_index][0], ((u8 *)(pPktHdr->esph)+8), ivsize);

	memcpy(&iv_ptr[ring_index][0], fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].nonce, 4);
	
	if(isESN==0)
		memcpy((&iv_ptr[ring_index][0])+4, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr), ivsize);
	else
	{
		memcpy((&iv_ptr[ring_index][0])+4, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+4, ivsize);
		//memcpy(iv_tmp, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+4, ivsize);
	}
	
	memcpy((&iv_ptr[ring_index][0])+12, gcm_iv_tail, 4);
	
	_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, &iv_ptr[ring_index][0], &iv_dma);

	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_dma     = iv_dma;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.iv_pointer = &iv_ptr[ring_index][0];


	/*
		Set AEAD 
	*/
	if(isESN==0)
	{
		memcpy(&aead_ptr[ring_index][0], (u8 *)(pPktHdr->esph),aead_size);
	
    	_rtk_fc_ipsec_set_aad_src_desc(aead_size,&aead_ptr[ring_index][0], &aead_dma);
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_dma	   	= aead_dma;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_pointer 	= &aead_ptr[ring_index][0];
	}
	else
	{
		/*
			AEAD header is modified in rtk_fc_ipsec_insert_header:
			- SPI
			- seq_no_low
			- seq_no_high
			So it needs to do some adjustment for getting the right aead header:
			--> SPI, seq_no_high, seq_no_low
		*/
		IPSEC("non-esn!");
		memcpy(&aead_ptr[ring_index][0]		, (u8 *)(pPktHdr->esph)		, 4);
		memcpy((&aead_ptr[ring_index][0])+4	, (u8 *)(pPktHdr->esph)+8	, 4);
		memcpy((&aead_ptr[ring_index][0])+8	, (u8 *)(pPktHdr->esph)+4	, 4);
		
    	_rtk_fc_ipsec_set_aad_src_desc(aead_size, &aead_ptr[ring_index][0], &aead_dma);
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_dma	   	= aead_dma;
		rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.aead_pointer 	= &aead_ptr[ring_index][0];
	}

	if(isESN==0)
	   	_rtk_fc_ipsec_set_src_desc(pPktHdr->crypto_plaintext_length, cryp_pad_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+ivsize);
	else
		_rtk_fc_ipsec_set_src_desc(pPktHdr->crypto_plaintext_length, cryp_pad_len, (u8 *)(pPktHdr->esph)+sizeof(struct ip_esp_hdr)+4+ivsize);

	return ret;
}


__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_esn_hash_ipi(rtk_fc_mac_port_idx_t macPortIdx, struct rt_skbuff *rtskb, uint16 ivsize, uint16 auth_sizes, int crypt_len, 
																int ipsec_shortCut_info_table_index, u8 *result_addr, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int direction,
																int cipher_mode, u8 *esp_addr)
{
	int aead_size	= 12; // Seq_hi is in the tail
	int key_index, key_size, hash_key_size, hash_len;
	u8 cmd_setting_ptr[28] = {0};
	int ret = 0;
	u8 key_pad[128] = {0};
	int cryp_pad_len = 0;
	int ring_index = -1;
	
	ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, ipsec_shortCut_info_table_index, macPortIdx, rtskb, direction,
										  cipher_mode, ivsize,0, auth_sizes, 
										  crypt_len, NULL, result_addr, pNicTxPriv,0,1, NULL);

	if(ret != SUCCESS)
		return FAILED;

	
	key_index	=  fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index ;
	key_size 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	hash_key_size = fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_hash_sz;
	hash_len	= crypt_len + aead_size;

	/*
		After encrypt, do the hash
	*/


	/*
		Set ICV destination descriptor
	*/
	_rtk_fc_ipsec_aescbc_esn_set_hashresult_desc(auth_sizes, result_addr);

	/*
		Command setting: hash only
	*/

	_rtk_fc_ipsec_aescbc_esn_hashonly_cmdsetting(hash_len, key_index, &cmd_setting_ptr[0]);



	/*
		Set hash key
		hmac: ipad, opad
	*/

	memset(key_pad, 0 , 128);
	memcpy(key_pad, &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_hash[0],hash_key_size); 	// ipad
	memcpy(key_pad+64, &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_hash[0],hash_key_size);// opad

	_rtk_fc_ipsec_aescbc_esn_set_hash_key(key_pad);



	/*
		Set source cipher text to do hashs
	*/
	
	cryp_pad_len = (16 - (hash_len % 16)) % 16; // HW needed: cipher text need to be 16 bytes alignment
	_rtk_fc_ipsec_set_src_desc(hash_len, cryp_pad_len, esp_addr);

	return SUCCESS;
}

__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_esn_encrypt_ipi(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index)
{
	int ret = SUCCESS;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	uint32 key_index, key_size, hash_key_index;
	int crypt_len, cryp_pad_len=  0;
	uint16 aead_size = 0;
	u8 iv_tmp[16] = {0};
	u8 key_tmp[32] = {0};
	
	aead_size	= 8;
	key_index		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;//; ipsec_shortCut_info_table_index;
	hash_key_index 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	crypt_len 	= pPktHdr->crypto_plaintext_length+ivsize;
	key_size 	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_sz;
	
	
	IPSEC("crypt_len = %d\n",crypt_len);
	/*
		ESN Need to encrypt first
	*/
	_rtk_fc_ipsec_aescbc_esn_set_cipher_dest_desc(crypt_len, pCipher_data);

	/*
		Command setting: cipher only
	*/
	_rtk_fc_ipsec_aescbc_esn_set_cipheronly_cmdsetting(crypt_len, key_index);
	
	
	/*
		Set encrypt key
	*/
	memcpy(&key_tmp[0], &fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key[0], key_size);
	_rtk_fc_ipsec_aescbc_esn_cipher_key(key_size, &key_tmp[0]);
	

	/*
		Set IV
	*/

	memcpy(&iv_tmp[0], ((u8 *)(pPktHdr->esph)+aead_size), ivsize);
	_rtk_fc_ipsec_aescbc_esn_cipher_iv(ivsize, &iv_tmp[0]);
	
	/*
		Set plain text
	*/

	cryp_pad_len = (16 - (crypt_len % 16)) % 16; // HW needed: cipher text need to be 16 bytes alignment
	_rtk_fc_ipsec_set_src_desc(crypt_len, cryp_pad_len, (u8 *)(pPktHdr->esph)+aead_size);


	return ret;
}

__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_des_cbc_mix_mode_encrypt_ipi(int ring_index, int cipher_mode, int hash_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index)
{
	int ret = SUCCESS;
	int hash_padding_array_idx = 0;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	u8 *pICV_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->crypto_plaintext_length+ivsize;
	uint8 key_index, hash_key_index;
	uint32 hash_pad_len = 0;
	int HMAC_SHA_1_ICV_LEN = 20, HMAC_MD5_ICV_LEN = 16, HMAC_SHA_256_ICV_LEN = 32;
	uint16 aead_size;
	uint32_t cmdsetting_dmaPtr = 0;
	int key_size = pPktHdr->ipsec_sc_info.key_size;
	
	//key_index	= ipsec_shortCut_info_table_index;
	//PROFILE_START_VAR(perf_0, __FUNCTION__);
	key_index		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;//; ipsec_shortCut_info_table_index;
	hash_key_index	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;
	IPSEC("ipsec_shortCut_info_table_index = %d, key index = %d, hash_key_index = %d, ivsize = %d, auth_sizes = %d", ipsec_shortCut_info_table_index, key_index, hash_key_index, ivsize, auth_sizes);
	
	//for mix mode, total 4 descriptor
	aead_size	= 8;
	
	hash_padding_array_idx = ((pPktHdr->crypto_plaintext_length+ivsize) /8)-1;
	if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
		hash_pad_len = fc_db.rtk_fc_ipsec_des_hash_padding_ary[hash_padding_array_idx].hash_padlen;
	else if(hash_mode == RTK_FC_AALG_MD5HMAC)
		hash_pad_len = fc_db.rtk_fc_ipsec_des_md5_hash_padding_ary[hash_padding_array_idx].hash_padlen;
	IPSEC("pPktHdr->crypto_plaintext_length = %d hash_padlen = %d hash_mode = %d cipher_mode = %d hash_mode = %d, hash_padding_array_idx = %d", pPktHdr->crypto_plaintext_length, hash_pad_len, hash_mode, cipher_mode, hash_mode,hash_padding_array_idx);
	
	//[dst desc] ciphertext
	_rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(pPktHdr->crypto_plaintext_length+ivsize, pCipher_data, pPktHdr);
	
	//[dst desc] hash
	if(hash_mode == RTK_FC_AALG_SHA1HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_SHA_1_ICV_LEN- auth_sizes,pICV_data, pPktHdr);
	else if (hash_mode == RTK_FC_AALG_SHA2_256HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_SHA_256_ICV_LEN- auth_sizes,pICV_data, pPktHdr);
	else if (hash_mode == RTK_FC_AALG_MD5HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_MD5_ICV_LEN- auth_sizes,pICV_data, pPktHdr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	//[src desc] cmd + iv
	_rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(hash_mode,
												cipher_mode,
												ivsize /*ivlen*/, 
												pPktHdr->crypto_plaintext_length+ivsize,  /*alan: 48+16*/
												hash_pad_len, 
												aead_size/*aad len:esp header size*/,
												pPktHdr,
												key_index,
												hash_key_index,
												key_size,
												cmdsetting_ptr[ring_index],
												&cmdsetting_dmaPtr);
	
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else
	//[src desc] cmd + iv
	_rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(hash_mode,
												cipher_mode,
												ivsize /*ivlen*/, 
												pPktHdr->crypto_plaintext_length+ivsize,  /*alan: 48+16*/
												hash_pad_len, 
												aead_size/*aad len:esp header size*/,
												pPktHdr,
												key_index,
												hash_key_index,
												key_size,
												&cmdsetting_ptr[ring_index],
												&cmdsetting_dmaPtr);
	
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif	
	//[src desc] aad + plaintext + hash_padding
	_rtk_fc_ipsec_descbc_shortCut_mixmode_set_src_desc(hash_mode,
														aead_size/*aad len:esp header size*/, 
														pPktHdr->crypto_plaintext_length+ivsize, 
														hash_pad_len, 
														pPktHdr, 
														hash_padding_array_idx, 
														pPktHdr->crypto_plaintext_length+ivsize+hash_pad_len+aead_size);
	
				
	return ret;
}

__IRAM_FC_SHORTCUT
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_mix_mode_encrypt_ipi(int ring_index, int hash_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index)
{
	int ret = SUCCESS;
	int hash_padding_array_idx = 0;
	u8 *pCipher_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	u8 *pICV_data = (u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->crypto_plaintext_length+ivsize;
	uint8 key_index, hash_key_index;
	uint32 hash_pad_len = 0;
	int HMAC_SHA_1_ICV_LEN = 20, HMAC_MD5_ICV_LEN = 16, HMAC_SHA_256_ICV_LEN = 32;
	uint16 aead_size;
	uint32_t cmdsetting_dmaPtr = 0;
	int key_size = pPktHdr->ipsec_sc_info.key_size;
	
	//key_index	= ipsec_shortCut_info_table_index;
	//PROFILE_START_VAR(perf_0, __FUNCTION__);
	key_index		= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].key_index;//; ipsec_shortCut_info_table_index;
	hash_key_index	= fc_db.fc_ipsec_info[ipsec_shortCut_info_table_index].hash_key_index;

	
	//for mix mode, total 4 descriptor
	aead_size	= 8;
	
	hash_padding_array_idx = ((pPktHdr->crypto_plaintext_length+ivsize) /16)-1;
	if(hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
		hash_pad_len = fc_db.rtk_fc_ipsec_hash_padding_ary[hash_padding_array_idx].hash_padlen;
	else if(hash_mode == RTK_FC_AALG_MD5HMAC)
		hash_pad_len = fc_db.rtk_fc_ipsec_md5_hash_padding_ary[hash_padding_array_idx].hash_padlen;
	IPSEC("hash_padlen = %d hash_mode = %d", hash_pad_len, hash_mode);
	
	//[dst desc] ciphertext
	_rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(pPktHdr->crypto_plaintext_length+ivsize, pCipher_data, pPktHdr);
	
	//[dst desc] hash
	if(hash_mode == RTK_FC_AALG_SHA1HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_SHA_1_ICV_LEN- auth_sizes,pICV_data, pPktHdr);
	else if (hash_mode == RTK_FC_AALG_SHA2_256HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_SHA_256_ICV_LEN- auth_sizes,pICV_data, pPktHdr);
	else if (hash_mode == RTK_FC_AALG_MD5HMAC)
		_rtk_fc_ipsec_encrypt_shortCut_set_icv_dest_desc(auth_sizes, HMAC_MD5_ICV_LEN- auth_sizes,pICV_data, pPktHdr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	//[src desc] cmd + iv
	_rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(hash_mode,
												ivsize /*ivlen*/, 
												pPktHdr->crypto_plaintext_length+ivsize,  /*alan: 48+16*/
												hash_pad_len, 
												aead_size/*aad len:esp header size*/,
												pPktHdr,
												key_index,
												hash_key_index,
												key_size,
												cmdsetting_ptr[ring_index],
												&cmdsetting_dmaPtr);
	
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = cmdsetting_ptr[ring_index];
#else
	//[src desc] cmd + iv
	_rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(hash_mode,
												ivsize /*ivlen*/, 
												pPktHdr->crypto_plaintext_length+ivsize,  /*alan: 48+16*/
												hash_pad_len, 
												aead_size/*aad len:esp header size*/,
												pPktHdr,
												key_index,
												hash_key_index,
												key_size,
												&cmdsetting_ptr[ring_index],
												&cmdsetting_dmaPtr);
	
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_dma	   = cmdsetting_dmaPtr;
	rtk_fc_ipsec_ipi.priv_work[ring_index].fc_ipi_ring_info.cmdsetting_pointer = &cmdsetting_ptr[ring_index];
#endif	
	//[src desc] aad + plaintext + hash_padding
	_rtk_fc_ipsec_aescbc_shortCut_mixmode_set_src_desc(hash_mode,
														aead_size/*aad len:esp header size*/, 
														pPktHdr->crypto_plaintext_length+ivsize, 
														hash_pad_len, 
														pPktHdr, 
														hash_padding_array_idx, 
														pPktHdr->crypto_plaintext_length+ivsize+hash_pad_len+aead_size);
	
				
	return ret;
}

#if defined(WEN_TEST)


void _rtk_fc_ipec_gcm_test(void)
{
#if 1
	int key_index = 0;
	int GCM_ICV_LEN = 16, gcm_hw_iv_size = 16, auth_sizes;

	u8 	key[32] = {
			0x8d, 0xa5 , 0xe0 , 0x6f , 0x87 , 0x48 , 0x5b , 0xf9 , 0x05 , 0xc5 , 0x2b , 0x34 , 0x87 , 0x24 , 0xbc , 0x4d,
		 	0xe3 , 0x41 , 0x6b , 0x87 , 0x80 , 0x96 , 0x7c , 0x80 , 0x94 , 0x6d , 0xbc , 0xef , 0xc3 , 0x50 , 0x2e , 0x32
		};
		
	int key_sz = 32;
	
	u8 pkt[32] = {
			0x45, 0x00 , 0x00 , 0x1c , 0x00 , 0x00 , 0x00 , 0x00 , 0xfe , 0x11 , 0x37 , 0x6c , 0xc0 , 0xa8 , 0x03 , 0x0a ,
			0xc0 , 0xa8 , 0x01 , 0x0a , 0x56 , 0xce , 0x2b , 0x67 , 0x00 , 0x08 , 0xf8 , 0x43 , 0x01 , 0x02 , 0x02 , 0x04
			 
		 	};
	int aead_size   = sizeof(struct ip_esp_hdr);
	u8 iv[16] = {0x27, 0x8b , 0xb1 , 0x60 , 0x1f , 0x51 , 0x25 , 0x53 , 0x5c , 0xfe , 0xbc , 0xc2 , 0x00 , 0x00 , 0x00 , 0x01};//from new text 4s
	                                                  
	u8 aead[8] = {0xc3 , 0xbb , 0xc2 , 0xac , 0x00 , 0x00 , 0x00 , 0x01}; // ca d6 99 7e 00 00 00 11
	u8 icv[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint32 crypt_len = 32;
	rtk_fc_cryptoEngine_command_t cmdSetting;
	uint32_t cmdsetting_dmaPtr, iv_dma,aead_dma;
		
	auth_sizes = 12;
    // prepare packet
   
	//RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	
	
	// 1. write key
	rtk_fc_ipsec_write_hw_key(key_index, key, key_sz); //rtkcrypto_write_hw_keypad
	
	
	_rtk_fc_ipsec_encrypt_set_dest_desc(crypt_len, &pkt[0]);
	

    _rtk_fc_ipsec_encrypt_set_icv_dest_desc(auth_sizes, GCM_ICV_LEN-auth_sizes, &icv[0]);
	

	_rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(crypt_len, aead_size, key_index,1, &cmdSetting, &cmdsetting_dmaPtr, key_sz);
	

	
	
	IPSEC("IV with 16 bytes");
	rtk_fc_hexdump(&iv[0], 16);

	_rtk_fc_ipsec_set_iv_src_desc(gcm_hw_iv_size, key_index, &iv[0], &iv_dma);

   IPSEC("AEAD with 8 bytes");
	rtk_fc_hexdump(&aead[0], 8);
	
    _rtk_fc_ipsec_set_aad_src_desc(aead_size,&aead[0], &aead_dma);

	_rtk_fc_ipsec_set_src_desc(crypt_len, 0, &pkt[0]);
	/*enable interrupt*/

	rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0xFFFF);
    _rtk_fc_ipsec_cryptEngine_polling_result();

	
	IPSEC("hw crypto done~");
	IPSEC("cipher result");
	dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(&pkt[0]), crypt_len, DMA_FROM_DEVICE);
	barrier();
	rtk_fc_hexdump(&pkt[0], crypt_len);
	
	IPSEC("hash result");
	dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(&icv[0]), 16, DMA_FROM_DEVICE);
	barrier();
	rtk_fc_hexdump(&icv[0], 16);

	// decrypt
	IPSEC("==================DECRYPT==============");
	


	//RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	/*enable interrupt*/
	rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
#endif
}

int _rtk_fc_ipsec_test_hashonly_cmdsetting(uint32 hash_len, uint32 key_index)
{
	{
	    rtk_fc_cryptoEngine_source_desc_t src_desc;
	    rtk_fc_cryptoEngine_command_t *cmd_ptr;

	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)fc_db.ipsec_addr.cmd_setting_ptr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->em = 1; //RTK_EM_HASH

	  	cmd_ptr->icvtl = 0x40;
		cmd_ptr->sh = 1;
		cmd_ptr->shf = 1;
		cmd_ptr->shl = 1;
		cmd_ptr->habs = 1;
		cmd_ptr->hm = 1;//RTK_HM_SHA1;
		cmd_ptr->he = 1;//RTK_HE_ENABLE;
		
		cmd_ptr->etl = (hash_len + 15)/16;
		cmd_ptr->elds = hash_len % 16;
		cmd_ptr->apl1 =  (hash_len+64) << 3;
	  
		
	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
	    src_desc.ctrl.type1.bits.cl = 3;
	    src_desc.ctrl.type1.bits.ap = 1;
		
	    src_desc.ctrl.type1.bits.sk = 1;
	    src_desc.ctrl.type1.bits.kl = key_index;//secure key idx 0
	    src_desc.ctrl.type1.bits.pk = 1;
	    src_desc.ctrl.type1.bits.pl = key_index;//secure keypad idx 0
	    
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
#if defined(IPSEC_DEBUG)	    
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif

		
	    src_desc.sdbp = fc_db.ipsec_addr.cmd_setting_dma;//dma_map_single(&(ni_info_data.pdev->dev), src_cmd_setting_iv_buffer, 12+16, DMA_TO_DEVICE);
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);

	}
	return SUCCESS;
}

int _rtk_fc_ipsec_test_cipheronly_cmdsetting(uint32 cryptlen, uint32 key_index)
{
	{
	    rtk_fc_cryptoEngine_source_desc_t src_desc;
	    rtk_fc_cryptoEngine_command_t *cmd_ptr;

	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)fc_db.ipsec_addr.cmd_setting_ptr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->em = 0; //RTK_EM_CRYPTO
		cmd_ptr->icvtl = 0x40;
		cmd_ptr->cabs = 1;
		cmd_ptr->ces = 0;//RTK_CES_AES
		cmd_ptr->cm = 1;//RTK_CM_AES_CBC
		cmd_ptr->ce = 1;//RTK_CE_CRYPT
		cmd_ptr->aks = 2;//RTK_AKS_256
		cmd_ptr->elds = cryptlen % 16;
	 	cmd_ptr->etl = (cryptlen + 15)/16;; //16 bytes as a unit

	  	rtk_fc_hexdump((unsigned char *)cmd_ptr, sizeof(rtk_fc_cryptoEngine_command_t));
	    
	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
	    src_desc.ctrl.type1.bits.cl = 3;
	 	
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
#if defined(IPSEC_DEBUG)	    
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif

		
	    src_desc.sdbp = fc_db.ipsec_addr.cmd_setting_dma;//dma_map_single(&(ni_info_data.pdev->dev), src_cmd_setting_iv_buffer, 12+16, DMA_TO_DEVICE);
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);

	}
	return SUCCESS;
}

void  _rtk_fc_ipsec_test_hash_key_iv(u8 *key_pad)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.pl=128/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), key_pad, 128, DMA_TO_DEVICE);
	
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}

void  _rtk_fc_ipsec_test_cipher_key(int key_size)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.rs=1;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.kl=key_size/4;
	data_desc.sdbp = fc_db.ipsec_addr.key_dma;
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}

void  _rtk_fc_ipsec_test_cipher_iv(int iv_size)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.rs=1;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.il=iv_size/4;
	data_desc.sdbp = fc_db.ipsec_addr.iv_dma;

	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
	
}
int _rtk_fc_ipsec_test_set_hashresult_desc(uint32 icv_len)

{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type1.wrd = 0;
    dst_crypto_desc.ctrl.type1.bits.ws  = 1;
    dst_crypto_desc.ctrl.type1.bits.enc = 0;
    dst_crypto_desc.ctrl.type1.bits.fs  = 1;
    dst_crypto_desc.ctrl.type1.bits.adl = icv_len; // Alan ==> 填payload + iv (48 + 16)
    dst_crypto_desc.ctrl.type1.bits.ls  = 1;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext:  dst_crypto_desc.ctrl.type2.wrdd = %x", dst_crypto_desc.ctrl.type2.wrd);
#endif	
   dst_crypto_desc.ddbp = fc_db.ipsec_addr.icv_dma;// dma_map_single(&(ni_info_data.pdev->dev), icv_addr, 12, DMA_TO_DEVICE);
    
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}

int _rtk_fc_ipsec_test_set_cipher_desc(uint32 crypt_len)
{
	//for ciphertext 
    rtk_fc_cryptoEngine_dest_desc_t  dst_crypto_desc;
    dst_crypto_desc.ctrl.type2.wrd = 0;
    dst_crypto_desc.ctrl.type2.bits.ws  = 1;
    dst_crypto_desc.ctrl.type2.bits.enc = 1;
    dst_crypto_desc.ctrl.type2.bits.fs  = 1;
    dst_crypto_desc.ctrl.type2.bits.enl = crypt_len; // Alan ==> 填payload + iv (48 + 16)
    dst_crypto_desc.ctrl.type2.bits.ls  = 1;
#if defined(IPSEC_DEBUG)

	IPSEC("Ciphertext:  dst_crypto_desc.ctrl.type2.wrdd = %x", dst_crypto_desc.ctrl.type2.wrd);
#endif	
   dst_crypto_desc.ddbp = fc_db.ipsec_addr.plain_text_dma;// dma_map_single(&(ni_info_data.pdev->dev), icv_addr, 12, DMA_TO_DEVICE);
    
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&dst_crypto_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) dst_crypto_desc.ddbp);
	return SUCCESS;
}



void _rtk_fc_hash_test(void)
{
	struct crypto_hash  *hash;
	struct hash_desc *desc;
 
	char key[20] ={0x32, 0xfa, 0x6c, 0x1e, 0x17, 0xd5, 0x5e, 0x76, 0x43, 0xdd, 0x4f, 0xd3, 0x4d, 0x5e, 0xd2, 0xab,
					0x73, 0x67, 0x19, 0xd8};
	struct scatterlist sg;
	char data[128] = {
		0xc8, 0x0c, 0xdb, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, // 12 
		/*
		0x10, 0x68, 0x0b, 0x21, 0xb7, 0x24, 0xc2, 0x39, 0x10, 0x0a, 0x85, 0x05, 0x3c, 0xd7, 0xec, 0x61, // 16			
		0x45, 0x00, 0x00, 0x21,	0xcd, 0x3f, 0x00, 0x00, 0x7f, 0x01, 0xe9, 0x41, 0xc0, 0xa8, 0x01, 0x09,  // 48
		0xc0, 0xa8, 0x03, 0x01,	0x08, 0x00, 0xcd, 0xf8, 0x00, 0x01, 0x00, 0x3f, 0x61, 0x62, 0x63, 0x64, 
		0x65, 0x01, 0x02, 0x03,	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x04,
*/
		0x3d, 0x5e, 0x8c, 0x23, 0x05, 0x56, 0xb3, 0xa7, 0x0a, 0x8c, 0xf4, 0xbd, 0x52, 0xb3, 0x00, 0xad,  // 16
		0x37, 0xf0, 0xd2, 0x60, 0xd5, 0xbd, 0xb4, 0xaf, 0xfd, 0xae, 0x0f, 0x3e, 0x57, 0xee, 0xb4, 0xda,  // 48
		0xcc, 0x2f, 0x4d, 0xd6, 0x66, 0x06, 0xbe, 0x3e, 0xe5, 0x73, 0xa9, 0x42, 0x9e, 0x01, 0x02, 0x96, 
		0xb0, 0xc9, 0x22, 0x20, 0xc0, 0x96, 0x71, 0x71, 0x5f, 0x5d, 0x22, 0xee, 0x1b, 0x99, 0xb4, 0x15,
		
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 52, total = 128 = 64*2
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0x4, 0x60
		};
	char digest[20];
 
	//memset(data, 0 ,100);
	//memset(key, 0 ,20);
	
	desc = kmalloc(sizeof(struct hash_desc), GFP_ATOMIC);
	memset(desc, 0 , sizeof(struct hash_desc));	
	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	RTK_FC_HELPER_MGR_TRACEFILTER_SPIN_UNLOCK_BH();
	//hash = rtk_fc_crypto_alloc_hash("hmac(sha1)", 0, CRYPTO_ALG_ASYNC);//crypto_alloc_hash("hmac(sha1)", 0, CRYPTO_ALG_ASYNC);
 	hash = crypto_alloc_hash("hmac(sha1)", 0, CRYPTO_ALG_ASYNC);
	if(IS_ERR(hash)){
		printk(KERN_INFO "Can't alloc hmac\n");
		goto RETURN_BYE ;
	}
 
	desc->tfm = hash;
	desc->flags = 0;
 
	if(crypto_hash_setkey(hash, key, 20))
	//if(rtk_fc_crypto_hash_setkey(hash, key, 20))
	{
		printk(KERN_INFO "crypto_hash_setkey()\n");
		goto RETURN_BYE ;
	}	
 
	if((crypto_hash_init(desc)))
	//if(rtk_fc_crypto_hash_init(desc))
	{
		printk("crypto_hash_init failed\n");
		goto RETURN_BYE;
	}
 
	sg_init_table(&sg, 1);
	sg_set_buf(&sg, data, sizeof(data));
 
	if (crypto_hash_update(desc, &sg, sizeof(data)))
	//if(rtk_fc_crypto_hash_update(desc, &sg, sizeof(data)))
	{
		printk(KERN_INFO "crypto_hash_update()\n");
		goto RETURN_BYE;
	}
 
	if((crypto_hash_final(desc, digest)))
	//if(rtk_fc_crypto_hash_final(desc, digest))
	{
		printk("crypto_hash_final\n");
		goto RETURN_BYE;
	}
 	printk("Hash result: \n");
	rtk_fc_hexdump(digest, 20);

RETURN_BYE:	
	RTK_FC_HELPER_MGR_TRACEFILTER_SPIN_LOCK_BH();
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	return;
  	// https://blog.csdn.net/lhl_blog/article/details/17149051
}


#endif

u8 cipher_data[496] = 
{
	0x71 ,0xca ,0x00 ,0x43 ,0x5d ,0xe8 ,0x0e ,0x19 ,0x48 ,0x32 ,0xbc ,0x86 ,0xe8 ,0x69 ,0xd8 ,0xcc, 
	0xbf ,0x68 ,0x44 ,0xe2 ,0x89 ,0x87 ,0xa7 ,0x0f ,0x89 ,0xc4 ,0x25 ,0x11 ,0x56 ,0xe0 ,0xe0 ,0xf4, 
	0xa1 ,0xf8 ,0x4d ,0xd0 ,0x2e ,0xf5 ,0xe4 ,0xa9 ,0xde ,0x11 ,0xc1 ,0xc1 ,0x41 ,0x06 ,0x4e ,0xc6, 
	0xd4 ,0x33 ,0x37 ,0xa2 ,0xaa ,0xf6 ,0x91 ,0xea ,0x99 ,0x00 ,0x78 ,0xda ,0x57 ,0xf0 ,0x7b ,0x45, 
	0xe2 ,0x33 ,0x39 ,0x4e ,0x44 ,0x29 ,0x2e ,0x40 ,0x3d ,0x32 ,0x78 ,0x61 ,0x43 ,0x8b ,0x32 ,0xc5, 
	0x9f ,0x54 ,0xa6 ,0xc6 ,0x0f ,0xcb ,0x1d ,0x4c ,0xec ,0x33 ,0x46 ,0xbe ,0x2f ,0xc2 ,0x77 ,0xc9, 
	0x0f ,0x5c ,0x6f ,0x95 ,0xc2 ,0xc3 ,0x6d ,0xc6 ,0x0a ,0x52 ,0xf1 ,0x8b ,0xba ,0xd2 ,0xc0 ,0x4b, 
	0x78 ,0x08 ,0xdb ,0xe8 ,0xc5 ,0x28 ,0x86 ,0x5c ,0x21 ,0x25 ,0x75 ,0x43 ,0xa8 ,0x3c ,0x3b ,0xc7, 
	0xf7 ,0x80 ,0x90 ,0xbb ,0x8a ,0x6b ,0xa5 ,0xab ,0x30 ,0x98 ,0x82 ,0x75 ,0xde ,0xe3 ,0x32 ,0x10, 
	0xb0 ,0xef ,0x73 ,0x61 ,0xbb ,0xce ,0x86 ,0xc6 ,0xb1 ,0xa7 ,0xc8 ,0x7a ,0x53 ,0xe3 ,0x8d ,0x5e, 
	0x3d ,0x2e ,0x62 ,0x84 ,0xef ,0xd8 ,0x13 ,0xf6 ,0xee ,0xdd ,0x91 ,0xe9 ,0x53 ,0xaa ,0x7b ,0x6c, 
	0x31 ,0x64 ,0xc1 ,0xb8 ,0xd2 ,0x4d ,0x62 ,0xf3 ,0x62 ,0xc4 ,0xa6 ,0x63 ,0x45 ,0x04 ,0x0f ,0xf6, 
	0x15 ,0xe7 ,0x71 ,0xe7 ,0x0c ,0xe1 ,0xbf ,0x78 ,0xbd ,0xcd ,0xf2 ,0xb8 ,0x3a ,0xf9 ,0x8e ,0x12, 
	0x67 ,0xb2 ,0x50 ,0xa9 ,0xa8 ,0x51 ,0xa1 ,0xdf ,0x1e ,0x07 ,0x99 ,0x93 ,0xf4 ,0x45 ,0x2d ,0xe5, 
	0xba ,0xca ,0xe9 ,0x74 ,0x57 ,0x70 ,0x93 ,0x49 ,0x37 ,0x46 ,0x61 ,0x7b ,0x28 ,0x4d ,0x16 ,0x3b, 
	0xdc ,0x50 ,0x88 ,0xc1 ,0xb1 ,0x89 ,0x7d ,0x17 ,0xbb ,0xa9 ,0x00 ,0x38 ,0xc8 ,0xf2 ,0x1d ,0xbb, 
	0x66 ,0x0f ,0xf3 ,0x71 ,0x1e ,0x88 ,0x61 ,0xc2 ,0x14 ,0x78 ,0x31 ,0x7f ,0x88 ,0xbe ,0xf9 ,0xc7, 
	0x14 ,0x78 ,0x45 ,0x4d ,0xab ,0x09 ,0x52 ,0x81 ,0xe3 ,0x04 ,0xf3 ,0x95 ,0xf9 ,0xb3 ,0x29 ,0xd0, 
	0x7b ,0x0e ,0xfc ,0x38 ,0xda ,0x63 ,0x0d ,0x33 ,0xc7 ,0x56 ,0xee ,0xf3 ,0xc1 ,0xbd ,0x0e ,0x7f, 
	0x10 ,0x89 ,0x5b ,0xa6 ,0xcf ,0x5d ,0x25 ,0x59 ,0x87 ,0xb2 ,0x07 ,0xff ,0x9e ,0x50 ,0xef ,0x8f, 
	0xc6 ,0x4f ,0x07 ,0x15 ,0xb3 ,0xa5 ,0xa4 ,0x64 ,0x5a ,0xdf ,0x38 ,0x82 ,0xf6 ,0xc5 ,0x84 ,0x21, 
	0x4b ,0x10 ,0x76 ,0x14 ,0x53 ,0xf8 ,0x9e ,0xc8 ,0x20 ,0xa6 ,0x5d ,0xa5 ,0xf8 ,0xc0 ,0xb8 ,0x15, 
	0x0f ,0x8b ,0x36 ,0x2b ,0x79 ,0xb4 ,0x07 ,0xd7 ,0x22 ,0xb4 ,0x44 ,0xad ,0xfc ,0x1a ,0xfe ,0x00, 
	0xdb ,0x02 ,0x8f ,0x48 ,0x8d ,0xb6 ,0xcf ,0xf8 ,0x89 ,0x26 ,0x35 ,0x77 ,0xf6 ,0xe0 ,0x3f ,0xfd, 
	0x33 ,0xdd ,0x71 ,0x20 ,0xc3 ,0x8b ,0x08 ,0x86 ,0x83 ,0x44 ,0xc7 ,0xb4 ,0x3d ,0x13 ,0x30 ,0x8f, 
	0x8f ,0x4d ,0x52 ,0xd9 ,0x99 ,0xb2 ,0xbc ,0x76 ,0x3d ,0x00 ,0x7f ,0x58 ,0x18 ,0xc3 ,0xc2 ,0x2b, 
	0x6f ,0x6a ,0x17 ,0x74 ,0x66 ,0x1f ,0xdb ,0x7f ,0x87 ,0xab ,0x5f ,0xdc ,0x92 ,0x48 ,0x6d ,0xf1, 
	0x9f ,0xa7 ,0x71 ,0x8d ,0x92 ,0x48 ,0xec ,0x4f ,0x3a ,0x20 ,0xef ,0xf4 ,0xa7 ,0xa0 ,0xed ,0xd5, 
	0x4d ,0x1d ,0x77 ,0xd5 ,0xc3 ,0xc3 ,0x90 ,0x1a ,0xa7 ,0x12 ,0xfd ,0x23 ,0xaa ,0x23 ,0x58 ,0xc0, 
	0x14 ,0x28 ,0x02 ,0xe9 ,0x83 ,0x51 ,0x17 ,0xf1 ,0x70 ,0x66 ,0x09 ,0x41 ,0x03 ,0x18 ,0x87 ,0xe3, 
	0xa7 ,0x81 ,0x01 ,0x13 ,0x2f ,0x29 ,0x67 ,0xb0 ,0x4f ,0x29 ,0xbf ,0xb2 ,0x55 ,0xcb ,0x75 ,0x04
};

#if 0
void _rtk_fc_aescbc_decrypt_test(void)
{
	int key_index = 0;
	u8 	key[16] = {
		0x14, 0x75, 0xb8 , 0xd3 , 0x4e , 0x5c , 0x16 , 0x75 , 0x4b , 0x5a , 0x3c , 0x7e , 0x01 , 0x33 , 0x35 , 0x0b
	};
	int AES_BLOCK_SIZE = 16;
	int key_sz = 16;
	u8 cmdsetting[28];
	u8 iv[16] = {0x10, 0x7a , 0xa9 , 0x1d , 0xe7 , 0x30 , 0x3d , 0xab , 0xd3 , 0x1b , 0x05 , 0xd2 , 0xe6 , 0xba , 0x80 , 0xc9};
	int cryptlen = 496;
	int ivsize = 16, cryp_padding_len = 0;
	u8 result=  0;

	/* disable interrupt*/
	rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0xFFFF);
	
	// 1. write key
	rtk_fc_ipsec_write_hw_key(key_index, key, key_sz); //rtkcrypto_write_hw_keypad

	IPSEC("[Decrypt]cryptlen = %d key_index = %d, ivsize = %d",cryptlen, key_index,ivsize);
		
	
	//1. dest descriptor
	rtk_fc_ipsec_test_set_dest_desc(cryptlen,&result,NULL);

	//2. command-setting
	rtk_fc_ipsec_decrypt_set_cmdSetting(cryptlen, ivsize, key_index, key_sz , &cmdsetting[0],NULL);

	//3. key iv
	_rtk_fc_ipsec_shortCut_set_iv_src_desc(ivsize, key_index, &iv[0],NULL);
	//ret = _rtk_fc_ipsec_cryptEngine_polling_result();
	//4. src descriptor
	
	cryp_padding_len = (AES_BLOCK_SIZE - (cryptlen % AES_BLOCK_SIZE)) % AES_BLOCK_SIZE;
	
	_rtk_fc_ipsec_shortCut_set_src_desc(cryptlen, cryp_padding_len, &cipher_data[0],NULL);
	
	_rtk_fc_ipsec_cryptEngine_polling_result();
	//dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(result),cryptlen, DMA_FROM_DEVICE);
					
	dump_packet(fc_db.ipsec_addr.plain_text_ptr, cryptlen,"test decrypt");

	/*enable interrupt*/
	rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
}
#endif
void _rtk_fc_ipsec_encrypt_set_icv_dest_desc(int icv_len, int hash_padding_len, u8 *address)
{
	rtk_fc_cryptoEngine_dest_desc_t auth_desc;
	rtk_fc_cryptoEngine_dest_desc_t auth_desc2;
	uint32_t addr;
	uint32_t addr2;
	auth_desc.ctrl.type1.wrd = 0;
	auth_desc.ctrl.type1.bits.ws = 1;
	auth_desc.ctrl.type1.bits.enc = 0;
	auth_desc.ctrl.type1.bits.adl = icv_len;
	if(hash_padding_len ==0)
	{
		auth_desc.ctrl.type1.bits.ls = 1;
	}
	//addr =	fc_db.ipsec_addr.icv_dma;
	addr =  dma_map_single(&(ni_info_data.pdev->dev), address, icv_len, DMA_TO_DEVICE);


	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, auth_desc.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, addr);

	
	if(hash_padding_len)
	{
		auth_desc2.ctrl.type1.wrd = 0;
		auth_desc2.ctrl.type1.bits.ws = 1;
		auth_desc2.ctrl.type1.bits.enc = 0;
		auth_desc2.ctrl.type1.bits.adl = hash_padding_len;
		auth_desc2.ctrl.type1.bits.ls = 1;
		addr2 =	fc_db.ipsec_addr.ipsec_icv_padding_dma;

		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDFWR, *((uint32*)&auth_desc2.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSWR, (uint32) addr2);

	}

}

void _rtk_fc_ipsec_gcm_decrypt_set_cmdSetting(uint32 plaintext_length, uint32 aad_len, uint32 key_index, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr, int key_size)
{
	{
		uint32_t addr;
	  	//  rtk_fc_cryptoEngine_source_desc_t src_desc;
	   	// rtk_fc_cryptoEngine_command_t *cmd_ptr;
		//u8 tmp_iv[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
		IPSEC("plaintext_length: %d, aad_len: %d", plaintext_length,aad_len);
	  
		pCmdSetting->wrd0 = aes_gcm_decrypt_cmmd_setting.wrd0;
		pCmdSetting->wrd1 = aes_gcm_decrypt_cmmd_setting.wrd1;
		pCmdSetting->wrd2 = aes_gcm_decrypt_cmmd_setting.wrd2;

		if(key_size == 32)
		    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		else if(key_size == 16)
			pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		else if(key_size == 24)
			pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192
			
		pCmdSetting->wrd2.bits.etl = (plaintext_length + 15)/16;
		pCmdSetting->wrd1.bits.elds = plaintext_length % 16;

		pCmdSetting->wrd2.bits.htl = (aad_len + 15)/16; // gcm
		pCmdSetting->wrd1.bits.aadlds = aad_len % 16; // gcm

		
		aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.bits.pl = key_index;

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;

		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_gcm_decrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();
		
#if 0		
		IPSEC("plaintext_length: %d, aad_len: %d", plaintext_length,aad_len);
	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)fc_db.ipsec_addr.cmd_setting_ptr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->wrd0.bits.em = 0; //SSH-dec/ESP-enc
		cmd_ptr->wrd0.bits.icvtl = 0x40;
		cmd_ptr->wrd1.bits.cabs = 1;
		
		cmd_ptr->wrd0.bits.ces = 0;// RTK_CES_AES;
		cmd_ptr->wrd0.bits.cm = 8;//RTK_CM_AES_GCM
		if(encrypt==1)
			cmd_ptr->wrd0.bits.ce = 1;
		else
			cmd_ptr->wrd0.bits.ce = 0;
		cmd_ptr->wrd0.bits.aks = 2;
		cmd_ptr->wrd2.bits.etl = (plaintext_length + 15)/16;
		cmd_ptr->wrd1.bits.elds = plaintext_length % 16;

		cmd_ptr->wrd2.bits.htl = (aad_len + 15)/16; // gcm
		cmd_ptr->wrd1.bits.aadlds = aad_len % 16; // gcm

	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
		src_desc.ctrl.type1.bits.cl	= 3;
		src_desc.ctrl.type1.bits.sk = 1;
		src_desc.ctrl.type1.bits.kl = key_index;
		src_desc.ctrl.type1.bits.pk = 1;
		src_desc.ctrl.type1.bits.pl = key_index;
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
		
#if defined(IPSEC_DEBUG)		
		IPSEC("command settings:");
		rtk_fc_hexdump((unsigned char *)cmd_ptr, sizeof(rtk_fc_cryptoEngine_command_t));
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif		

		//integrate cmd setting + iv
	    //memcpy(src_cmd_setting_iv_buffer+12, iv, 16);
		
	    src_desc.sdbp = fc_db.ipsec_addr.cmd_setting_dma;
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);
#endif
	}

}


void _rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(uint32 plaintext_length, uint32 aad_len, uint32 key_index, int encrypt, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr, int key_size)
{
	{
		uint32_t addr;
	  	//  rtk_fc_cryptoEngine_source_desc_t src_desc;
	   	// rtk_fc_cryptoEngine_command_t *cmd_ptr;
		//u8 tmp_iv[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
		IPSEC("plaintext_length: %d, aad_len: %d", plaintext_length,aad_len);
	  
		pCmdSetting->wrd0 = aes_gcm_encrypt_cmmd_setting.wrd0;
		pCmdSetting->wrd1 = aes_gcm_encrypt_cmmd_setting.wrd1;
		pCmdSetting->wrd2 = aes_gcm_encrypt_cmmd_setting.wrd2;

		if(key_size == 32)
		    pCmdSetting->wrd0.bits.aks = 2; //AES_KEYSIZE_256
		else if(key_size == 16)
			pCmdSetting->wrd0.bits.aks = 0; //AES_KEYSIZE_128
		else if(key_size == 24)
			pCmdSetting->wrd0.bits.aks = 1; //AES_KEYSIZE_192
			
		pCmdSetting->wrd2.bits.etl = (plaintext_length + 15)/16;
		pCmdSetting->wrd1.bits.elds = plaintext_length % 16;

		pCmdSetting->wrd2.bits.htl = (aad_len + 15)/16; // gcm
		pCmdSetting->wrd1.bits.aadlds = aad_len % 16; // gcm

		
		aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.kl = key_index;
		aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.bits.pl = key_index;

		addr = dma_map_single(&(ni_info_data.pdev->dev), pCmdSetting, 28, DMA_TO_DEVICE);
		*cmdSetting_dmaPtr = addr;

		wmb();
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_gcm_encrypt_cmmd_setting_src.ctrl.type1.wrd);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
		wmb();
		
#if 0		
		IPSEC("plaintext_length: %d, aad_len: %d", plaintext_length,aad_len);
	    //prepare command setting buffer
	    cmd_ptr = (rtk_fc_cryptoEngine_command_t *)fc_db.ipsec_addr.cmd_setting_ptr;
	    memset((uint8*)cmd_ptr, 0, 12);//no auto-padding, so cmd setting length is 12bytes

	    cmd_ptr->wrd0.bits.em = 0; //SSH-dec/ESP-enc
		cmd_ptr->wrd0.bits.icvtl = 0x40;
		cmd_ptr->wrd1.bits.cabs = 1;
		
		cmd_ptr->wrd0.bits.ces = 0;// RTK_CES_AES;
		cmd_ptr->wrd0.bits.cm = 8;//RTK_CM_AES_GCM
		if(encrypt==1)
			cmd_ptr->wrd0.bits.ce = 1;
		else
			cmd_ptr->wrd0.bits.ce = 0;
		cmd_ptr->wrd0.bits.aks = 2;
		cmd_ptr->wrd2.bits.etl = (plaintext_length + 15)/16;
		cmd_ptr->wrd1.bits.elds = plaintext_length % 16;

		cmd_ptr->wrd2.bits.htl = (aad_len + 15)/16; // gcm
		cmd_ptr->wrd1.bits.aadlds = aad_len % 16; // gcm

	    //set command setting + iv
	    src_desc.ctrl.type1.wrd = 0;
	    src_desc.ctrl.type1.bits.rs = 1;
	    src_desc.ctrl.type1.bits.fs = 1;
		src_desc.ctrl.type1.bits.cl	= 3;
		src_desc.ctrl.type1.bits.sk = 1;
		src_desc.ctrl.type1.bits.kl = key_index;
		src_desc.ctrl.type1.bits.pk = 1;
		src_desc.ctrl.type1.bits.pl = key_index;
	    //set src_desc.w to sdfw_reg
	    //set src_cmd_setting_iv_buffer to sdsw_reg
		
#if defined(IPSEC_DEBUG)		
		IPSEC("command settings:");
		rtk_fc_hexdump((unsigned char *)cmd_ptr, sizeof(rtk_fc_cryptoEngine_command_t));
		IPSEC("src_desc.ctrl.type1.wrd = %x",src_desc.ctrl.type1.wrd);
#endif		

		//integrate cmd setting + iv
	    //memcpy(src_cmd_setting_iv_buffer+12, iv, 16);
		
	    src_desc.sdbp = fc_db.ipsec_addr.cmd_setting_dma;
		
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&src_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) src_desc.sdbp);
#endif
	}

}


void  _rtk_fc_ipsec_set_iv_src_desc(int ivlen, int key_index, u8 *addrs, uint32_t *iv_dma)
{
	uint32_t addr;
	
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.il	= ivlen/4;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.kl 	= key_index;
	aes_gcm_encrypt_iv_src.ctrl.type1.bits.pl 	= key_index;
	
	addr = dma_map_single(&(ni_info_data.pdev->dev), addrs, ivlen, DMA_TO_DEVICE);//fc_db.ipsec_addr.iv_dma;
	*iv_dma = addr;
	wmb();
	
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, aes_gcm_encrypt_iv_src.ctrl.type1.wrd);
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, addr);
#if 0	
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	data_desc.ctrl.type1.wrd = 0;
	data_desc.ctrl.type1.bits.rs=1;
	data_desc.ctrl.type1.bits.fs=1;
	data_desc.ctrl.type1.bits.ls=0;
	data_desc.ctrl.type1.bits.il=ivlen/4;
	data_desc.sdbp = dma_map_single(&(ni_info_data.pdev->dev), addr, ivlen, DMA_TO_DEVICE);//fc_db.ipsec_addr.iv_dma;
	
	data_desc.ctrl.type1.bits.sk = 1;
	data_desc.ctrl.type1.bits.kl = key_index;
	data_desc.ctrl.type1.bits.pk = 1;
	data_desc.ctrl.type1.bits.pl = key_index;
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32*)&data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32) data_desc.sdbp);
#endif
	
}
void  _rtk_fc_ipsec_set_aad_src_desc(int aead_size, u8 *addr, uint32_t *aead_dma)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc_1;
	rtk_fc_cryptoEngine_source_desc_t data_desc_2;
	uint32_t _aead_addr;
	uint32_t cryp_pad_addr;

	
	data_desc_1.ctrl.type2.wrd = 0;
	data_desc_1.ctrl.type2.bits.rs=1;
	data_desc_1.ctrl.type2.bits.a2eo_epl = aead_size;
	
	_aead_addr =  dma_map_single(&(ni_info_data.pdev->dev), addr, aead_size, DMA_TO_DEVICE);//fc_db.ipsec_addr.aad_dma;
	*aead_dma = _aead_addr;
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc_1.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32_t) _aead_addr);

	if( (16-aead_size%16)%16 !=0 )	
	{

		data_desc_2.ctrl.type2.wrd = 0;
		data_desc_2.ctrl.type2.bits.rs=1;
		data_desc_2.ctrl.type2.bits.a2eo_epl = (16-aead_size%16)%16;
		cryp_pad_addr = fc_db.ipsec_addr.ipsec_crypt_padding_dma;
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc_2.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32_t)cryp_pad_addr);


	}
}

void _rtk_fc_ipsec_set_src_desc(int cryptlen, int cryp_pad_len, u8 *address)
{
	rtk_fc_cryptoEngine_source_desc_t data_desc;
	uint32_t addr;
	uint32_t addr2;
	data_desc.ctrl.type2.wrd = 0;
	data_desc.ctrl.type2.bits.rs=1;
	data_desc.ctrl.type2.bits.enl = cryptlen;
	if(cryp_pad_len == 0)
		data_desc.ctrl.type2.bits.ls=1;
	addr = dma_map_single(&(ni_info_data.pdev->dev), address, cryptlen, DMA_TO_DEVICE);//fc_db.ipsec_addr.plain_text_dma;
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc.ctrl));
	rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32_t)addr);

	if(cryp_pad_len)
	{
	
		data_desc.ctrl.type2.wrd = 0;
		data_desc.ctrl.type2.bits.rs=1;
		data_desc.ctrl.type2.bits.enl = cryp_pad_len;
		data_desc.ctrl.type2.bits.ls=1;
		addr2 = fc_db.ipsec_addr.ipsec_crypt_padding_dma; //dma_map_single(&(ni_info_data.pdev->dev), crypt_padding_tmp_dma, cryp_pad_len, DMA_TO_DEVICE);
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDFWR, *((uint32_t*) &data_desc.ctrl));
		rtk_fc_reg_io_write32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSWR, (uint32_t)addr2);
	}
}
int _rtk_fc_ipsec_setup_skbSecurePath(struct rt_skbuff *rtskb, struct xfrm_state *x_state)
{
	int ret = SUCCESS;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	
		struct sec_path *sp;
	
		sp = secpath_set(RTSKB_SKB(rtskb));
		if (!sp) {
			IPSEC("secpath set Failed!!");
			ret = FAILED;
			return ret;
		}
		if(x_state)
		{	
			xfrm_state_hold(x_state);
			RTSKB_SKB(rtskb)->mark = xfrm_smark_get(RTSKB_SKB(rtskb)->mark, x_state);
		
			sp->xvec[sp->len++] = x_state;
	
		}
		else
		{
			WARNING("x_state is NULL\n");
			
			ret = FAILED;
			return ret;
	
		}
		return ret;
#else

	if (!RTSKB_SKB(rtskb)->sp || atomic_read(&RTSKB_SKB(rtskb)->sp->refcnt) != 1) {
		struct sec_path *sp;
		DEBUG("Try to allocate skb->sp!");

		sp = secpath_dup(RTSKB_SKB(rtskb)->sp);
		if (!sp) {
			printk("_rtk_fc_ipsec_setup_skbSecurePath: secpath_dup() returns NULL.\n");
			
			//XFRM_INC_STATS(net, LINUX_MIB_XFRMINERROR);
			
			ret = FAILED;
			return ret;
		}
		if (RTSKB_SKB(rtskb)->sp)
			secpath_put(RTSKB_SKB(rtskb)->sp);
		
		RTSKB_SKB(rtskb)->sp = sp;
		DEBUG("Try to allocate skb->sp success!");
	}
	if (RTSKB_SKB(rtskb)->sp->len == XFRM_MAX_DEPTH) {
	
		WARNING("%s: skb->sp->len == XFRM_MAX_DEPTH (%d).\n", __func__, XFRM_MAX_DEPTH);
		
		//ret = RTK_FC_RET_LRN_SHORTCUT_TEST;
		//RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
		
		ret = FAILED;
		return ret;
	}
	//x = (tinfo.direction == CA_IPSEC_SPD_ENCRYPT) ? iter->xfrm_state_out : iter->xfrm_state_in;
	if(x_state)
	{	
		xfrm_state_hold(x_state);
		RTSKB_SKB(rtskb)->sp->xvec[RTSKB_SKB(rtskb)->sp->len++] = x_state;
		DEBUG("Set skb->sec_path before netif! RTSKB_SKB(rtskb)->sp: %p",RTSKB_SKB(rtskb)->sp);

	}
	else
	{
		WARNING("x_state is NULL\n");
		
		ret = FAILED;
		return ret;

	}
#endif

	return ret;
}


int _rtk_fc_ipsec_debug_set(int val)
{
	if(val > 0)
		ipsec_debug = 1;
	else
		ipsec_debug = 0;
	return SUCCESS;
}


int _rtk_fc_ipsec_debug_get(void)
{
	return ipsec_debug;
}



void rtk_fc_ipsec_dump_ipi_info(void)
{

	{
		int free_idx,sched_idx;
	
		//rtk_fc_ipsec_ring_ctrl_t rtk_fc_ipsec_ipi_ring = {0};
		rtk_fc_ipsec_ipi_ctrl_t *ipi_info = &rtk_fc_ipsec_ipi;

		free_idx = atomic_read(&ipi_info->priv_free_idx);
		sched_idx = atomic_read(&ipi_info->priv_sched_idx);

		printk("%-15s: cpu[%d] [priv] freeIdx %-4d schedIdx %-4d waiting for processing %-4d (total: %d, work cnt:%d)\r\n\t\t\t [global] csd %d\n",
			"IPSEC ipi info",  RTK_FC_IPSEC_IPI_DISPATCH_CPU,
			free_idx, sched_idx, (free_idx+ipi_info->priv_work_cnt-sched_idx)&(ipi_info->priv_work_cnt-1),MAX_FC_IPSEC_QUEUE_SIZE,
			ipi_info->priv_work_cnt, atomic_read(&ipi_info->csd_available));
		printk("[Interrupt]: last done counter num: %d\n",last_count);
		printk(" total enq: %llu total deq: %llu, timeout = %d\n", total_enq_cnt, total_deq_cnt, ipsec_exec_timeout);
		printk(" deq_err_cnt: %llu\n",deq_err_cnt);
		total_enq_cnt = 0;total_deq_cnt = 0;
		
		slow_cnt = 0;
		decrypt_cnt = 0;
		print_num = 0;
		ipsec_exec_timeout = 0;
		deq_err_cnt = 0;
	}

	
}

inline int _rtk_fc_ipsec_desc_fifo_full(void)
{
	return ((rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_SDSR) & 0x00ff) < 6 || 
 		    (rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_DDSR) & 0x00ff) < 3 );
	
}

int _rtk_fc_ipsec_hwlookup_recovery(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr)
{
    int ret = SUCCESS, find = 0, i ;
    int table_index = RXINFO_DST_EXTPIDX(pRxInfo) - RTK_FC_IPSEC_SWID_BASE;
//      int i;
    struct net_device *dev = NULL;

    IPSEC("IPsec table Index: %d", table_index);
	
	if(table_index < 0 || table_index >=RTK_FC_SPEC_XFRM_INFO_MAX_NUM)
	{
		IPSEC("IPSEC hwlookup miss but table index error! (%d)",table_index);
		return FAILED;
	}
	if(fc_db.fc_ipsec_info[table_index].x_state==NULL)
	{
		IPSEC("IPSEC hwlookup miss but table index error! (%d)",table_index);
		return FAILED;

	}
	
	if(pPktHdr->iph==NULL && pPktHdr->outer_iph == NULL)
	{
		IPSEC("IPSEC don't have ip header!");
		return FAILED;

	}
	

    if(table_index < 0)
    {
        WARNING("IPSEC hwlookup miss but table index error! (%d)",table_index);
        return FAILED;
    }



    // allocate sec_path

	for(i=DEVIFIDX_VALID_MIN;i<RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		int ip_addr = 0;
		
		if(fc_db.devGwMacTbl[i].dev )
		{
			RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(fc_db.devGwMacTbl[i].dev, &ip_addr);
			if(pPktHdr->outer_iph)
			{
				if(ip_addr!= 0 && ip_addr == ntohl(pPktHdr->outer_iph->daddr))
				{
					RTSKB_DEV(rtskb) = fc_db.devGwMacTbl[i].dev;
					IPSEC("Find by Comparing outer DIP!");
					find = 1;
					break;
				}
			}
			else 
			{
				if(ip_addr!= 0 && ip_addr == ntohl(pPktHdr->iph->daddr))
				{
					RTSKB_DEV(rtskb) = fc_db.devGwMacTbl[i].dev;
					IPSEC("Find by Comparing inner DIP!");
					find = 1;
					break;
				}


			}
		}
		
	}
	if(find == 0)
	{
	    dev = rtk_fc_getDev_by_fcDevIdx(fc_db.fc_ipsec_info[table_index].devGwMacIdx);
	    if(dev)
	            RTSKB_DEV(rtskb) = dev;
	    else
	    {
	            WARNING("[IPSEC]dev recovery failed!! dev= NULL (fc_db.fc_ipsec_info[%d].intf_idx = %d)", table_index, fc_db.fc_ipsec_info[table_index].devGwMacIdx);
	    }

	}

    IPSEC("Recover dev[%s] before going to netif_rx", RTSKB_DEV(rtskb)->name);

    ret = _rtk_fc_ipsec_setup_skbSecurePath(rtskb, fc_db.fc_ipsec_info[table_index].x_state);


    if(ret != SUCCESS)
    {
            WARNING("IPSEC setup skb fields failed !(%d)",table_index);
            return FAILED;

    }

    return ret;
}

int _rtk_fc_ipsec_shortCut_processing(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_smp_nicTx_private_t *pNicTxPriv)
{
	int ret = SUCCESS;
	int ring_index = -1;
	int aead_size;
#if 1	// No need to check, sc is not fast enough to reach this.
	if(_rtk_fc_ipsec_desc_fifo_full())
	{
		if(printk_ratelimit())
			printk("descriptor not enough! drop! \n");
		if(unlikely(fc_db.fwdStatistic))
		{
			atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_desc_not_enough[pPktHdr->ingressPort.macPortIdx]);
		}
		return RTK_FC_RET_DROP_IPSEC_SC_FIFO_FULL;
	}
#endif	
	
	if(pPktHdr->ipsec_sc_info.direction == RTK_FC_IPSEC_DIR_DECRYT)
	{
		
		ret = rtk_fc_ipsec_remove_header(pPktHdr->ipsec_sc_info.cipher_mode, rtskb, pPktHdr);
					
		if(ret == SUCCESS)
		{
			/*
				Setting up Nic tx priv	
			*/
			pPktHdr->ipsec_sc_info.doHwlookup = 1;
			IPSEC("Decrypt done, ready to do hwlookup, table_index = %d",pPktHdr->ipsec_sc_info.table_index);
			pNicTxPriv->isHwlookup 		= 1;
			pNicTxPriv->flow_id 		= pPktHdr->ipsec_sc_info.table_index+RTK_FC_IPSEC_SWID_BASE;
			pNicTxPriv->skb 			= RTSKB_SKB(rtskb);
			pNicTxPriv->hwlookup_lspid 	= RTK_FC_IPSEC_HWLOOKUP_MACPORT;//pPktHdr->ingressPort.macPortIdx;
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // support lspid_map table				
			pNicTxPriv->hwlookup_lspid_mapIdx = fc_db.lspid_map_idx_decrypt_hwlookup;
#endif			

			aead_size = 8;
			//pNicTxPriv->core_config.bf.txq_index = 3;
			if(unlikely(fc_db.fwdStatistic))
			{
				atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt_hwlookup[pPktHdr->ingressPort.macPortIdx]);
			}
			
			if(pPktHdr->ipsec_sc_info.cipher_mode == RTK_FC_EALG_AESCBC )
			{		
				if(pPktHdr->iph == NULL)
					return FAILED;
			
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_DECRYT,
												  pPktHdr->ipsec_sc_info.cipher_mode, 
												  pPktHdr->ipsec_sc_info.ivsize, 
												  aead_size,
												  pPktHdr->ipsec_sc_info.auth_sizes, 
												  pPktHdr->ipsec_sc_info.crypto_plaintext_length,  
												  (u8 *)(pPktHdr->iph), 
												  NULL, pNicTxPriv,0,0, pPktHdr);

				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				
				ret = _rtk_fc_esp_aes_cbc_mix_mode_decrypt_ipi(ring_index, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize,  
						pPktHdr->ipsec_sc_info.crypto_plaintext_length, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Decrypt failed, so drop packet.");
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
				
			}
			else if(pPktHdr->ipsec_sc_info.cipher_mode == RTK_FC_EALG_DESCBC || pPktHdr->ipsec_sc_info.cipher_mode == RTK_FC_EALG_3DESCBC)
			{		
				if(pPktHdr->iph == NULL)
					return FAILED;
			
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_DECRYT,
												  pPktHdr->ipsec_sc_info.cipher_mode, 
												  pPktHdr->ipsec_sc_info.ivsize, 
												  aead_size,
												  pPktHdr->ipsec_sc_info.auth_sizes, 
												  pPktHdr->ipsec_sc_info.crypto_plaintext_length,  
												  (u8 *)(pPktHdr->iph), 
												  NULL, pNicTxPriv,0,0, pPktHdr);

				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				
				ret = _rtk_fc_esp_des_cbc_mix_mode_decrypt_ipi(ring_index, pPktHdr->ipsec_sc_info.cipher_mode, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize,  
						pPktHdr->ipsec_sc_info.crypto_plaintext_length, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Decrypt failed, so drop packet.");
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
				
			}
			else if(pPktHdr->ipsec_sc_info.cipher_mode == RTK_FC_EALG_3DESCBC )
			{		
				if(pPktHdr->iph == NULL)
					return FAILED;
			
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_DECRYT,
												  pPktHdr->ipsec_sc_info.cipher_mode, 
												  pPktHdr->ipsec_sc_info.ivsize, 
												  aead_size,
												  pPktHdr->ipsec_sc_info.auth_sizes, 
												  pPktHdr->ipsec_sc_info.crypto_plaintext_length,  
												  (u8 *)(pPktHdr->iph), 
												  NULL, pNicTxPriv,0,0, pPktHdr);

				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				
				ret = _rtk_fc_esp_aes_cbc_mix_mode_decrypt_ipi(ring_index, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize,  
						pPktHdr->ipsec_sc_info.crypto_plaintext_length, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Decrypt failed, so drop packet.");
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
				
			}
			else if(pPktHdr->ipsec_sc_info.cipher_mode ==RTK_FC_EALG_AES_GCM_ICV12 )
			{
/*			
				u8 *iv_ptr = NULL;
				dma_addr_t iv_dma;
				
				if(pPktHdr->iph == NULL)
					return FAILED;

				iv_ptr = dma_alloc_coherent(rtk_fc_crypto_dev, pPktHdr->ipsec_sc_info.ivsize, &iv_dma, GFP_ATOMIC);
*/				
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, RTK_FC_IPSEC_DIR_DECRYT,
												  pPktHdr->ipsec_sc_info.cipher_mode, 
												  pPktHdr->ipsec_sc_info.ivsize, 
												  aead_size,
												  pPktHdr->ipsec_sc_info.auth_sizes,
												  pPktHdr->ipsec_sc_info.crypto_plaintext_length, 
												  (u8 *)(pPktHdr->iph), 
												  NULL, pNicTxPriv,0,0, pPktHdr);

				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				
				ret = _rtk_fc_ipsec_gcm_shortCut_decrypt_ipi(ring_index, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize, pPktHdr->ipsec_sc_info.crypto_plaintext_length, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Decrypt failed, so drop packet.");
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_decrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
			}
			
			IPSEC("[Decrypt]After decrypt, try to do hw-lookup!");
		}
		else
		{
			WARNING("Decrypt remove header failed, so drop.");
			return FAILED;
		}
	}
	else if(pPktHdr->ipsec_sc_info.direction == RTK_FC_IPSEC_DIR_ENCRYT)
	{							
		if(pPktHdr->iph == NULL)
			return FAILED;
		if(pPktHdr->ipsec_sc_info.cipher_mode ==RTK_FC_EALG_AESCBC )
		{
			aead_size = 8;
			//pNicTxPriv->core_config.bf.txq_index = 6;
			if(pPktHdr->ipsec_sc_info.isESN)
			{
				
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, pPktHdr->ipsec_sc_info.direction,
											pPktHdr->ipsec_sc_info.cipher_mode, 
											pPktHdr->ipsec_sc_info.ivsize, 
											aead_size,
											pPktHdr->ipsec_sc_info.auth_sizes,
											pPktHdr->crypto_plaintext_length + pPktHdr->ipsec_sc_info.ivsize, 
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), 
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr) + pPktHdr->crypto_plaintext_length + pPktHdr->ipsec_sc_info.ivsize,
											pNicTxPriv, 1, 0, pPktHdr);
				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				
				ret = _rtk_fc_esp_aes_cbc_esn_encrypt_ipi(rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize, pPktHdr->ipsec_sc_info.auth_sizes, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Encrypt failed(%d/%d), so drop.",pPktHdr->ipsec_sc_info.cipher_mode, pPktHdr->ipsec_sc_info.isESN);
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_encrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
				
			}
			else
			{
				
				ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, pPktHdr->ipsec_sc_info.direction,
											pPktHdr->ipsec_sc_info.cipher_mode, 
											pPktHdr->ipsec_sc_info.ivsize, 
											aead_size,
											pPktHdr->ipsec_sc_info.auth_sizes,
											pPktHdr->crypto_plaintext_length + pPktHdr->ipsec_sc_info.ivsize, 
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), 
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->crypto_plaintext_length+pPktHdr->ipsec_sc_info.ivsize, 
											pNicTxPriv, 0, 0, pPktHdr);
				if(ret != SUCCESS)
					return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
				// use 4 descriptor
				ret = _rtk_fc_esp_aes_cbc_mix_mode_encrypt_ipi(ring_index, pPktHdr->ipsec_sc_info.hash_mode, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize, pPktHdr->ipsec_sc_info.auth_sizes, pPktHdr->ipsec_sc_info.table_index);
				if(ret == FAILED)
				{
					WARNING("Encrypt failed(%d/%d), so drop.",pPktHdr->ipsec_sc_info.cipher_mode, pPktHdr->ipsec_sc_info.isESN);
					return FAILED;
				}
				else
				{
					if(unlikely(fc_db.fwdStatistic))
					{
						atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_encrypt[pPktHdr->ingressPort.macPortIdx]);
					}
				}
				
			}
				
		}
		else if(pPktHdr->ipsec_sc_info.cipher_mode ==RTK_FC_EALG_DESCBC || pPktHdr->ipsec_sc_info.cipher_mode ==RTK_FC_EALG_3DESCBC )
		{
			aead_size = 8;
			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, pPktHdr->ipsec_sc_info.direction,
										pPktHdr->ipsec_sc_info.cipher_mode, 
										pPktHdr->ipsec_sc_info.ivsize, 
										aead_size,
										pPktHdr->ipsec_sc_info.auth_sizes,
										pPktHdr->crypto_plaintext_length + pPktHdr->ipsec_sc_info.ivsize, 
										(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr), 
										(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->crypto_plaintext_length+pPktHdr->ipsec_sc_info.ivsize, 
										pNicTxPriv, 0, 0, pPktHdr);
			if(ret != SUCCESS)
				return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
			// use 4 descriptor
			ret = _rtk_fc_esp_des_cbc_mix_mode_encrypt_ipi(ring_index, pPktHdr->ipsec_sc_info.cipher_mode, pPktHdr->ipsec_sc_info.hash_mode, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize, pPktHdr->ipsec_sc_info.auth_sizes, pPktHdr->ipsec_sc_info.table_index);
			if(ret == FAILED)
			{
				WARNING("Encrypt failed(%d/%d), so drop.",pPktHdr->ipsec_sc_info.cipher_mode, pPktHdr->ipsec_sc_info.isESN);
				return FAILED;
			}
			else
			{
				if(unlikely(fc_db.fwdStatistic))
				{
					atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_encrypt[pPktHdr->ingressPort.macPortIdx]);
				}
			}
			
		}
		else if(pPktHdr->ipsec_sc_info.cipher_mode ==RTK_FC_EALG_AES_GCM_ICV12 )
		{
			if(pPktHdr->ipsec_sc_info.isESN==0)
			{
				aead_size   = sizeof(struct ip_esp_hdr);
			}
			else
			{
				aead_size   = 12;
			}
			ret = _rtk_fc_ipsec_set_ipi_work_info(&ring_index, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ingressPort.macPortIdx, rtskb, pPktHdr->ipsec_sc_info.direction,
											pPktHdr->ipsec_sc_info.cipher_mode, 
											pPktHdr->ipsec_sc_info.ivsize, 
											aead_size,
											pPktHdr->ipsec_sc_info.auth_sizes,
											pPktHdr->crypto_plaintext_length,
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->ipsec_sc_info.ivsize, 
											(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+pPktHdr->ipsec_sc_info.ivsize+pPktHdr->crypto_plaintext_length, 
											pNicTxPriv,0,0, pPktHdr);
			if(ret != SUCCESS)
				return RTK_FC_RET_DROP_IPSEC_SC_IPI_FULL;
			
			ret = _rtk_fc_ipsec_gcm_shortCut_encrypt_ipi(ring_index, rtskb, pPktHdr, pPktHdr->ipsec_sc_info.ivsize, pPktHdr->ipsec_sc_info.auth_sizes, pPktHdr->ipsec_sc_info.table_index, pPktHdr->ipsec_sc_info.isESN);
			if(ret == FAILED)
			{
				WARNING("Encrypt failed(%d/%d), so drop.",pPktHdr->ipsec_sc_info.cipher_mode, pPktHdr->ipsec_sc_info.isESN);
				return FAILED;
			}
			else
			{
				if(unlikely(fc_db.fwdStatistic))
				{
					atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_encrypt[pPktHdr->ingressPort.macPortIdx]);
				}
			}
		}
	}
	
	if(unlikely(fc_db.fwdStatistic))
	{
		atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_ipi_enqueue[pPktHdr->ingressPort.macPortIdx]);
	}

	
	return ret;
}

void _rtk_fc_ipsec_flow_flush_all(uint32 lock)
{
	int i ;
	
	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid)
			_rtk_fc_ipsec_flow_flush_by_saInfo_index(i, 0);
	}

	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	return;
}

void _rtk_fc_ipsec_flow_flush_by_saInfo_index(uint32 saInfo_index, uint32 lock)
{
	rtk_fc_ipsec_flowindex_list_t *pTmp_flow_index_list, *pTmp_next_flow_index_list;
	
	IPSEC("Flush flows of sainfo index %u", saInfo_index);

	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	if(!list_empty(&fc_db.ipsec_swFlow_list[saInfo_index]))
	{
		list_for_each_entry_safe(pTmp_flow_index_list, pTmp_next_flow_index_list, &fc_db.ipsec_swFlow_list[saInfo_index], flow_list)	//just return the first entry right behind of head
		{
			
			if(pTmp_flow_index_list->flow_index)
			{
				TABLE("Delete flow index %u, sainfo index %u)\n",pTmp_flow_index_list->flow_index, saInfo_index);
				//RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_FLOW, pTmp_flow_index_list->hash_index);
				rtk_fc_flow_delete(pTmp_flow_index_list->flow_index);
				list_del(&pTmp_flow_index_list->flow_list);
				RTK_FC_HELPER_FC_KFREE(pTmp_flow_index_list, sizeof(rtk_fc_ipsec_flowindex_list_t));
				//RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW, pTmp_flow_index_list->hash_index);
			}
		}
	}

	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	return;
}


void _rtk_fc_ipsec_saInfo_delete_by_index(uint32 saInfo_index, uint32 lock)
{
	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
{
	rt_pe_crypto_request_t crypto_req;
	rt_pe_ret_t pe_ret;

	if(fc_db.fc_ipsec_info[saInfo_index].pe_crypto_sw_id) //valid
	{
		memset(&crypto_req, 0, sizeof(rt_pe_crypto_request_t));
		crypto_req.req_cmd = (fc_db.fc_ipsec_info[saInfo_index].pe_crypto_sw_id & RT_PE_IPSEC_SW_ID_DECRYPTION_BIT) ? RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_DEL : RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_DEL;
		crypto_req.connection_idx = (fc_db.fc_ipsec_info[saInfo_index].pe_crypto_sw_id & RT_PE_IPSEC_SW_ID_CONNECTION_IDX_MASK) - RT_PE_IPSEC_SW_ID_CONNECTION_IDX_BASE;
		pe_ret = rtk_fc_pe_crypto_test(crypto_req, FAIL, 0);
		if(pe_ret != RT_PE_RET_OK)
			WARNING("Fail to call rtk_fc_pe_crypto_test, pe_ret=%d", pe_ret);
	}
}
#endif	
	_rtk_fc_ipsec_flow_flush_by_saInfo_index(saInfo_index, 0);


	
	if(fc_db.fc_ipsec_info[saInfo_index].key_index!=0xff)
	{
		printk("Invalid key usage(%d) for fc_db.fc_ipsec_info[%d]\n",fc_db.fc_ipsec_info[saInfo_index].key_index,saInfo_index);
		if(key_usage[fc_db.fc_ipsec_info[saInfo_index].key_index].valid == 1)
		{
			key_usage[fc_db.fc_ipsec_info[saInfo_index].key_index].valid = 0;
		}

	}
	
	
	
	
	if(fc_db.fc_ipsec_info[saInfo_index].hash_key_index!=0xff)
	{
		printk("Invalid hash key usage(%d) fc_db.fc_ipsec_info[%d]\n",fc_db.fc_ipsec_info[saInfo_index].hash_key_index,saInfo_index);
		if(key_pad_usage[fc_db.fc_ipsec_info[saInfo_index].hash_key_index].valid == 1)
		{
			key_pad_usage[fc_db.fc_ipsec_info[saInfo_index].hash_key_index].valid = 0;
		}
	}

	

	memset(&fc_db.fc_ipsec_info[saInfo_index], 0, sizeof(rtk_fc_ipsec_xfrm_info_t));
	fc_db.fc_ipsec_info[saInfo_index].key_index=0xff;
	fc_db.fc_ipsec_info[saInfo_index].hash_key_index=0xff;

	if(lock) 
		RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	
	return;
}

int _rtk_fc_ipsec_saInfo_add(uint32 direction, struct xfrm_state *x, struct xfrm_policy *xp)
{
	int find = 0, index = -1, i;

	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(x != NULL)
	{
		IPSEC_HOOK("\n---------------------------------------------_rtk_fc_ipsec_saInfo_add-----------------------------------------------------------------\n");
		//IPSEC_HOOK("rtk_fc_cryptoEngine_baseaddr = %p\n",rtk_fc_cryptoEngine_baseaddr);
		for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
		{
			if(fc_db.fc_ipsec_info[i].valid && 
				fc_db.fc_ipsec_info[i].direction == direction &&
				fc_db.fc_ipsec_info[i].ip_version == 0 &&
				fc_db.fc_ipsec_info[i].daddr == ntohl(x->id.daddr.a6[0]) &&
				fc_db.fc_ipsec_info[i].spi == x->id.spi
				
			)
			{
				index = i;
				find = 1;
				fc_db.fc_ipsec_info[i].x_state = x;
				IPSEC_HOOK("Find old entry [%d]!!!\n",index);
				break;
			}
			if(fc_db.fc_ipsec_info[i].valid && 
				fc_db.fc_ipsec_info[i].direction == direction &&
				fc_db.fc_ipsec_info[i].dying == RTK_FC_IPSEC_STATE_DYING_HARD
			)
			{
				index = i;
				find = 1;
				
				IPSEC_HOOK("Find dying entry [%d]!!!\n",index);
				break;
			}
			if(fc_db.fc_ipsec_info[i].valid ==0)
			{
				index = i;
				IPSEC_HOOK("Find New entry![%d]\n",index);
				break;
			}
			if(i == RTK_FC_SPEC_XFRM_INFO_MAX_NUM-1)
			{
				IPSEC_HOOK("Reach Max spec xfrm info table!!!\n");
				break;//TODO: should not be recorded
			}
			
		}

		if (index!=-1) // NEW ONE
		{
			
			
			printk("Add FC hook info with index:%d!!!\n",index);

			_rtk_fc_ipsec_saInfo_delete_by_index(index, 0);
		
			fc_db.fc_ipsec_info[index].x_state = x;

			{
				struct crypto_aead *aead;
				struct aead_geniv_ctx *ctx;
				
				
				
				fc_db.fc_ipsec_info[index].direction= direction;
				fc_db.fc_ipsec_info[index].replay_window  = x->props.replay_window;
				fc_db.fc_ipsec_info[index].spi = x->id.spi;
				fc_db.fc_ipsec_info[index].header_len = x->props.header_len;
				fc_db.fc_ipsec_info[index].tunnel_mode = (x->props.mode == XFRM_MODE_TRANSPORT) ? 0 : 1;;
				fc_db.fc_ipsec_info[index].ip_version = (x->props.family == AF_INET) ? 0 : 1;
				fc_db.fc_ipsec_info[index].dying = 0;
				fc_db.fc_ipsec_info[index].mark_v = x->mark.v;
				fc_db.fc_ipsec_info[index].mark_m = x->mark.m;
				wmb();

				//INIT_LIST_HEAD(&fc_db.fc_ipsec_info[index].flow_index_list);
				if(fc_db.fc_ipsec_info[index].ip_version  == 0)
				{
					fc_db.fc_ipsec_info[index].daddr = ntohl(x->id.daddr.a4);
					fc_db.fc_ipsec_info[index].saddr = ntohl(x->props.saddr.a4);
					IPSEC_HOOK("fc_db.fc_ipsec_info[%d].daddr = %x, x->id.daddr.a4 = %x\n",index, fc_db.fc_ipsec_info[index].daddr, ntohl(x->id.daddr.a6[0]));
					IPSEC_HOOK("fc_db.fc_ipsec_info[%d].saddr = %x, x->props.saddr.a4 = %x\n",index, fc_db.fc_ipsec_info[index].saddr, ntohl(x->props.saddr.a6[0]));
				}
				else
				{
					memcpy(&fc_db.fc_ipsec_info[index].daddr6, &x->id.daddr.in6, sizeof(struct in6_addr));
					memcpy(&fc_db.fc_ipsec_info[index].saddr6, &x->props.saddr.in6, sizeof(struct in6_addr));
					
					
				}
				wmb();
				//fc_ipsec_info[index].protocol = (x->id.proto == IPPROTO_ESP) ? 0 : 1;
				if(x->replay_esn!=NULL)
				{
					IPSEC_HOOK("ESN Find!\n");
					fc_db.fc_ipsec_info[index].is_ESN = 1;
					fc_db.fc_ipsec_info[index].seq_no_hi= (x->replay_esn->oseq_hi);
				}
				fc_db.fc_ipsec_info[index].seq_no	= (x->replay.oseq);
				IPSEC_HOOK("----%s(%d):Update fc_ipsec_info.seq_no = %d fc_db.fc_ipsec_info[index].seq_no = %d\n",__func__,__LINE__, x->replay.oseq, fc_db.fc_ipsec_info[index].seq_no);

				if (x->encap != NULL) {
					//encap: encap_type=0x0002, encap_sport=4500, encap_dport=4500, encap_oa=00000000-00000000-00000000-00000000
					IPSEC_HOOK("encap: encap_type=0x%04x, encap_sport=%u, encap_dport=%u, encap_oa=%08x-%08x-%08x-%08x\n",
							x->encap->encap_type, ntohs(x->encap->encap_sport), ntohs(x->encap->encap_dport),
							ntohl(x->encap->encap_oa.a6[0]), ntohl(x->encap->encap_oa.a6[1]),
							ntohl(x->encap->encap_oa.a6[2]), ntohl(x->encap->encap_oa.a6[3]));
					if(x->encap->encap_type == 0x2) // esp in udp
					{
						fc_db.fc_ipsec_info[index].is_NATT = 1;
						fc_db.fc_ipsec_info[index].encap_sport = ntohs(x->encap->encap_sport);
						fc_db.fc_ipsec_info[index].encap_dport = ntohs(x->encap->encap_dport);
					}
				}
				if(x->data)
				{
					IPSEC_HOOK("\n x->props.aalgo:%d x->props.ealgo:%d\n", x->props.aalgo, x->props.ealgo); // pfkeyv2.h
					
					aead = x->data;	
					fc_db.fc_ipsec_info[index].auth_len = rtk_fc_crypto_aead_authsize(aead);
					fc_db.fc_ipsec_info[index].iv_len = rtk_fc_crypto_aead_ivsize(aead);

					ctx = rtk_fc_crypto_aead_ctx(aead);	
					memcpy(fc_db.fc_ipsec_info[index].salt_in, ctx->salt,fc_db.fc_ipsec_info[index].iv_len);
					
					fc_db.fc_ipsec_info[index].block_size = rtk_fc_crypto_aead_blocksize(aead);
					fc_db.fc_ipsec_info[index].aalgo = x->props.aalgo; //SADB_AALG_SHA1HMAC 
					fc_db.fc_ipsec_info[index].ealgo = x->props.ealgo; //SADB_X_EALG_AESCBC SADB_X_EALG_AES_GCM_ICV12
					
					if (x->aalg != NULL)
					{
						IPSEC_HOOK("x->aalg->alg_name: %s \n",x->aalg->alg_name);
						
						memcpy(&fc_db.fc_ipsec_info[index].key_hash[0], x->aalg->alg_key, (x->aalg->alg_key_len + 7) / 8);
						fc_db.fc_ipsec_info[index].key_hash_sz = (x->aalg->alg_key_len+7)/8; 
						
						
					}
					if (x->ealg != NULL) 
					{
						IPSEC_HOOK("x->ealg->alg_name: %s \n",x->ealg->alg_name);
						
						fc_db.fc_ipsec_info[index].key_sz = (x->ealg->alg_key_len+7)/8; 
						memcpy(&fc_db.fc_ipsec_info[index].key[0],  x->ealg->alg_key, (x->ealg->alg_key_len+7) / 8 );
						
						
						
					}
					
					if (x->aead != NULL) 
					{
						IPSEC_HOOK("x->aead->alg_name: %s \n",x->aead->alg_name);
						fc_db.fc_ipsec_info[index].key_sz = (x->aead->alg_key_len+7)/8 - 4; 
						
						memcpy(&fc_db.fc_ipsec_info[index].key[0],  x->aead->alg_key, fc_db.fc_ipsec_info[index].key_sz );

						memcpy(&fc_db.fc_ipsec_info[index].nonce[0], (u8 *)x->aead->alg_key+fc_db.fc_ipsec_info[index].key_sz, 4);
					}
				}
				fc_db.fc_ipsec_info[index].valid = 1;
				
			}
#if 0			
			if(fc_db.controlFuc.ipsec_en_shortCut)
			{
				if(fc_db.fc_ipsec_info[index].key_is_set==0)
				{
					if(_rtk_fc_ipsec_get_supported_mode(fc_db.fc_ipsec_info[index].ealgo, fc_db.fc_ipsec_info[index].aalgo) )
					{
						
						if(fc_db.fc_ipsec_info[index].key_hash_sz>0){
							IPSEC_HOOK("Set hash key: algo: %d\n", fc_db.fc_ipsec_info[index].ealgo);
							rtk_fc_ipsec_write_hw_keypad(index, fc_db.fc_ipsec_info[index].key_hash, fc_db.fc_ipsec_info[index].key_hash_sz);
						}
						
						if(fc_db.fc_ipsec_info[index].key_sz>0){
							IPSEC_HOOK("Set cipher key: algo: %d\n", fc_db.fc_ipsec_info[index].ealgo);	
							rtk_fc_ipsec_write_hw_key(index, fc_db.fc_ipsec_info[index].key, fc_db.fc_ipsec_info[index].key_sz);
						}
						
						fc_db.fc_ipsec_info[index].key_is_set = 1;
					}
					

				}

			}
#endif		
			printk("================================%s table[%d] done======================================\n",__func__, index);
			
		}
		
	}

	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return 0;
}

int _rtk_fc_ipsec_saInfo_flush_all(void)
{
	int i ;
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid)
		{
			_rtk_fc_ipsec_saInfo_delete_by_index(i, 0);
					
			IPSEC("%s(%d): Find old entry [%d] and set dying = DYING_HARD!!!\n", __func__, __LINE__, i);
		}
		
	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	return 0;
}

int _rtk_fc_ipsec_saInfo_delete(uint32 spi)
{
	int i ;
	printk("\n---------------------------------------------_rtk_fc_ipsec_saInfo_delete------- spi = %x----------------------------------------------------------\n", ntohl(spi));
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid && 
			fc_db.fc_ipsec_info[i].spi == spi
		)
		{	
			_rtk_fc_ipsec_saInfo_delete_by_index(i, 0);
					
			IPSEC_HOOK("%s(%d): Find old entry [%d] and set dying = DYING_HARD!!!\n", __func__, __LINE__, i);
		}
		
	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);

	return 0;
}

int _rtk_fc_ipsec_get_hwnatMode(void)
{
	if((1<<fc_db.controlFuc.hwnat_mode) & HWNAT_MODE_SKIP_FC_FUNC_BITMASK)
		return 0;
	else
		return 1;
	
}
int _rtk_fc_ipsec_get_shortCut_en(void)
{
	if(fc_db.controlFuc.ipsec_en_shortCut == 1)
		return 1;
	else
		return 0;
	
}
int _rtk_fc_ipsec_table_ready(uint32 spi)
{
	int ret = 0,i;
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid && 
			fc_db.fc_ipsec_info[i].spi == spi
		)
		{
			ret = 1;
			break;
		}
	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return ret;
}

int _rtk_fc_ipsec_get_supported_mode(u8 cipher_mode, int hash_mode)
{
	int ret = 0;
	
	switch(cipher_mode)
	{
		case RTK_FC_EALG_AESCBC:
		{
			if(hash_mode == RTK_FC_AALG_MD5HMAC ||hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
				ret = 1;
			else
				ret = 0;
			break;
		}
		case RTK_FC_EALG_DESCBC:
		{
			if(hash_mode == RTK_FC_AALG_MD5HMAC ||hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
				ret = 1;
			else
				ret = 0;
			break;
		}
		case RTK_FC_EALG_3DESCBC:
		{
			if(hash_mode == RTK_FC_AALG_MD5HMAC ||hash_mode == RTK_FC_AALG_SHA1HMAC || hash_mode == RTK_FC_AALG_SHA2_256HMAC)
				ret = 1;
			else
				ret = 0;
			break;
		}
#if 0	// disable due to PE does not support it
		case RTK_FC_EALG_AES_GCM_ICV12:
			ret = 1;
			break;
#endif		
		default:
			ret = 0;
			break;
	}
	return ret;
}



int _rtk_fc_ipsec_saInfo_soft_delete(uint32 spi)
{
	int i ;
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	IPSEC_HOOK("\n---------------------------------------------_rtk_fc_ipsec_saInfo_soft_delete-----------------------------------------------------------------\n");
	for(i = 0 ; i < RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid && 
			fc_db.fc_ipsec_info[i].spi == spi
		)
		{
			fc_db.fc_ipsec_info[i].dying = RTK_FC_IPSEC_STATE_DYING_SOFT;
			
			IPSEC_HOOK("%s(%d):Find old entry [%d] and set dying = DYING_SOFT!!!\n",__func__, __LINE__,i);
			break;
		}
		
	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	return 0;
}


__IRAM_FC_SHORTCUT
void rtk_fc_ipsec_ipi_tasklet(void *info)
{
	rtk_fc_ipsec_ipi_ctrl_t *ipi_ctrl = info;
	tasklet_hi_schedule(&ipi_ctrl->tasklet);
}
__IRAM_FC_SHORTCUT
int _rtk_fc_ipsec_set_ipi_work_info(int *ring_index, int ipsec_hook_table_index, rtk_fc_mac_port_idx_t macPortIdx, struct rt_skbuff *rtskb, int encrypt,
		int cipher_mode, int iv_size, int aead_size, int auth_size, int total_crypt_len, u8  *pCipher_text_address, u8 *pIcv_address, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int isEsn_encrypt, int isEsn_hash, rtk_fc_pktHdr_t *pPktHdr)
{

	/*get index, fill related data for later modify*/	
	rtk_fc_ipsec_ipi_work_info_t *pWorkinfo=NULL;
	int freeidx = 0;
	uint32 ringMaskSize = (MAX_FC_IPSEC_QUEUE_SIZE-1);


	freeidx = atomic_read(&rtk_fc_ipsec_ipi.priv_free_idx);
	pWorkinfo = &rtk_fc_ipsec_ipi.priv_work[freeidx];
	//IPSEC_tmp("_rtk_fc_ipsec_set_ipi_work_info: freeidx = %d, scheduleidx = %d cpu = %d\n",freeidx, atomic_read(&rtk_fc_ipsec_ipi.priv_sched_idx),smp_processor_id());
	prefetch(pWorkinfo);

	if(unlikely(((freeidx+MAX_FC_IPSEC_QUEUE_SIZE-atomic_read(&rtk_fc_ipsec_ipi.priv_sched_idx)) & ringMaskSize) >= (ringMaskSize-FC_IPI_QUEUE_RSVED_CNT)) )		
	{
		if(atomic_read(&rtk_fc_ipsec_ipi.csd_available)==IPI_SCHED) {
			//no free rx_works, lots of skb are waiting to process
			if(unlikely(fc_db.fwdStatistic))
				atomic_inc(&fc_db.statistic.perPortCnt_shortcut_ipsec_ipi_drop[macPortIdx]);
			
			return FAILED;
		}
	}
	*ring_index = freeidx;
//2ENQUEUE:
	//Setup information
	memset(&pWorkinfo->fc_ipi_ring_info, 0, sizeof(rtk_fc_ipsec_ipi_private_t));
	memcpy(&pWorkinfo->fc_ipi_ring_info.nicTx_priv, pNicTxPriv, sizeof(rtk_fc_smp_nicTx_private_t));	
	
	pWorkinfo->fc_ipi_ring_info.skb 					= RTSKB_SKB(rtskb);
	pWorkinfo->fc_ipi_ring_info.cipher_mode 			= cipher_mode;
	pWorkinfo->fc_ipi_ring_info.encrypt 				= encrypt;
	
	pWorkinfo->fc_ipi_ring_info.iv_size 				= iv_size;
	pWorkinfo->fc_ipi_ring_info.aead_size 				= aead_size;
	pWorkinfo->fc_ipi_ring_info.icv_size 				= auth_size;
	pWorkinfo->fc_ipi_ring_info.macPortIdx 				= macPortIdx;
	pWorkinfo->fc_ipi_ring_info.ipsec_hook_table_index  = ipsec_hook_table_index;
	pWorkinfo->fc_ipi_ring_info.pCipher_text_address 	= pCipher_text_address;//(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr);
	pWorkinfo->fc_ipi_ring_info.pIcv_address 			= pIcv_address;//(u8 *)(pPktHdr->esph) + sizeof(struct ip_esp_hdr)+crypt_len+iv_size;
	
	pWorkinfo->fc_ipi_ring_info.crypto_len 				= total_crypt_len;
	pWorkinfo->fc_ipi_ring_info.isEsn_encrypt 			= isEsn_encrypt;
	pWorkinfo->fc_ipi_ring_info.isEsn_hash 				= isEsn_hash;

	
	if(unlikely(ipsec_debug))
	{

		if(pPktHdr){
			pPktHdr->ipsec_sc_info.ring_index = freeidx;
		}
	}

	
	++freeidx;
	smp_mb(); 
	freeidx &= ringMaskSize;
	smp_mb(); 
	atomic_set(&rtk_fc_ipsec_ipi.priv_free_idx, freeidx);		
	if(atomic_read(&rtk_fc_ipsec_ipi.csd_available)==IPI_IDLE)
	{
		atomic_set(&rtk_fc_ipsec_ipi.csd_available, IPI_SCHED);
	}
	total_enq_cnt ++;
	return SUCCESS;

}

__IRAM_FC_SHORTCUT
static void rtk_fc_ipsec_ipi_exec(unsigned long data)
{
	rtk_fc_ipsec_ipi_ctrl_t * priv =  (rtk_fc_ipsec_ipi_ctrl_t *)data;
	rtk_fc_ipsec_ipi_work_info_t *pWorkinfo=NULL;
	//unsigned long int break_jiffies=jiffies+(CONFIG_HZ<<1);
	bool /*timeout = FALSE,*/continue_tx = FALSE;
	int scheduleidx, tmp_cnt = 0;
	uint32 cmdstat = 0, intr_count = 0 , true_done_cnt = 0, this_time_cnt;
	//uint32 cmdok = 0;
	
	local_bh_disable();
RE_RUN:		
	scheduleidx = atomic_read(&priv->priv_sched_idx);
	
	cmdstat = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
	
	intr_count = (cmdstat & 0xff00) >> 8;
	if(intr_count > last_count){
		true_done_cnt = intr_count - last_count;
	}
	else{
		true_done_cnt = (intr_count + (0xFF-last_count)+1)% RTK_FC_HW_MAX_JOBS;
	}

	
	this_time_cnt = true_done_cnt;
	do{
		if(true_done_cnt <= 0)
			break;
	
		do{	
			pWorkinfo = &priv->priv_work[scheduleidx];	
			prefetch(pWorkinfo);
			
			if(pWorkinfo->fc_ipi_ring_info.encrypt == RTK_FC_IPSEC_DIR_ENCRYT) // Encrypt
			{
				
				if(pWorkinfo->fc_ipi_ring_info.cmdsetting_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.cmdsetting_dma, sizeof(rtk_fc_cryptoEngine_command_t), DMA_TO_DEVICE);
				}
				
				if(pWorkinfo->fc_ipi_ring_info.aead_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.aead_dma, pWorkinfo->fc_ipi_ring_info.aead_size, DMA_TO_DEVICE);
				}
				
				if(pWorkinfo->fc_ipi_ring_info.iv_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.iv_dma, pWorkinfo->fc_ipi_ring_info.iv_size, DMA_TO_DEVICE);
				}
				
			
				if(unlikely(pWorkinfo->fc_ipi_ring_info.isEsn_encrypt))
				{
					struct rt_skbuff rtskb;

					
					RTK_FC_HOOK_CONVERTER_SKB(pWorkinfo->fc_ipi_ring_info.skb, &rtskb);
					
					dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(pWorkinfo->fc_ipi_ring_info.pCipher_text_address), pWorkinfo->fc_ipi_ring_info.crypto_len, DMA_FROM_DEVICE);
					wmb(); 	
					IPSEC("[ESN]hw encrypt done");

					DUMP_PACKET(pWorkinfo->fc_ipi_ring_info.skb->data, pWorkinfo->fc_ipi_ring_info.skb->len, "[Encrypt ESN]After IPI");
										
					/*call hash*/

					RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
					_rtk_fc_esp_aes_cbc_esn_hash_ipi(pWorkinfo->fc_ipi_ring_info.macPortIdx, &rtskb, pWorkinfo->fc_ipi_ring_info.iv_size, 
													 pWorkinfo->fc_ipi_ring_info.icv_size, pWorkinfo->fc_ipi_ring_info.crypto_len, 
													 pWorkinfo->fc_ipi_ring_info.ipsec_hook_table_index, 
													 pWorkinfo->fc_ipi_ring_info.pIcv_address, &pWorkinfo->fc_ipi_ring_info.nicTx_priv,
													 pWorkinfo->fc_ipi_ring_info.encrypt, pWorkinfo->fc_ipi_ring_info.cipher_mode,
													 pWorkinfo->fc_ipi_ring_info.pCipher_text_address- sizeof(struct ip_esp_hdr));
					RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
					continue_tx = FALSE;
				}
				else if( unlikely(pWorkinfo->fc_ipi_ring_info.isEsn_hash))
				{
					dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(pWorkinfo->fc_ipi_ring_info.pIcv_address),pWorkinfo->fc_ipi_ring_info.icv_size, DMA_FROM_DEVICE);
					wmb(); 
					
					DUMP_PACKET(pWorkinfo->fc_ipi_ring_info.skb->data, pWorkinfo->fc_ipi_ring_info.skb->len, "[Hash]After IPI");
					continue_tx = TRUE;
				}
				else
				{
					dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(pWorkinfo->fc_ipi_ring_info.pCipher_text_address), pWorkinfo->fc_ipi_ring_info.crypto_len+pWorkinfo->fc_ipi_ring_info.icv_size, DMA_FROM_DEVICE);
					wmb(); 
					DUMP_PACKET(pWorkinfo->fc_ipi_ring_info.skb->data, pWorkinfo->fc_ipi_ring_info.skb->len, "[Encrypt]After IPI");
					
					continue_tx = TRUE;

				}

			
			}
			else if(pWorkinfo->fc_ipi_ring_info.encrypt == RTK_FC_IPSEC_DIR_DECRYT)//decrypt
			{			
				int len_diff = 0;
				uint8 nextHdr = 0, pad_len = 0;

				decrypt_cnt++;
#if 1
				if(pWorkinfo->fc_ipi_ring_info.iv_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.iv_dma, pWorkinfo->fc_ipi_ring_info.iv_size, DMA_TO_DEVICE);
				}
				
				if(pWorkinfo->fc_ipi_ring_info.aead_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.aead_dma, pWorkinfo->fc_ipi_ring_info.aead_size, DMA_TO_DEVICE);
				}
				if(pWorkinfo->fc_ipi_ring_info.cmdsetting_dma)
				{
					dma_unmap_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.cmdsetting_dma, sizeof(rtk_fc_cryptoEngine_command_t), DMA_TO_DEVICE);
				}
#endif					
				dma_sync_single_for_cpu(&(ni_info_data.pdev->dev), virt_to_phys(pWorkinfo->fc_ipi_ring_info.pCipher_text_address),pWorkinfo->fc_ipi_ring_info.crypto_len, DMA_FROM_DEVICE);

				wmb(); 

				nextHdr = pWorkinfo->fc_ipi_ring_info.pCipher_text_address[pWorkinfo->fc_ipi_ring_info.crypto_len-1];
				
				if(unlikely(nextHdr !=0x4 && nextHdr !=0x29))
				{
					if(print_num <=5)
					{
						printk("[Next Hdr Wrong]scheduleidx(%d) decrypt_cnt = %d this_time_cnt = %d\n", scheduleidx,decrypt_cnt, this_time_cnt);
						dump_packet(pWorkinfo->fc_ipi_ring_info.skb->data, pWorkinfo->fc_ipi_ring_info.skb->len, "[Decrypt]Strange packet Next Hdr Wrong");
						printk("scheduleidx : %d\n",scheduleidx);
						print_num++;
						dump_packet(pWorkinfo->fc_ipi_ring_info.pCipher_text_address, pWorkinfo->fc_ipi_ring_info.crypto_len, "[Decrypt]Strange packet: decrypted address");
							
					}
					continue_tx = FALSE;
					deq_err_cnt++;
					RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(pWorkinfo->fc_ipi_ring_info.skb);
				}
				else
				{
					pad_len = pWorkinfo->fc_ipi_ring_info.pCipher_text_address[pWorkinfo->fc_ipi_ring_info.crypto_len-2]+2;

					len_diff = pWorkinfo->fc_ipi_ring_info.icv_size+pWorkinfo->fc_ipi_ring_info.iv_size + 20 + 8 + pad_len;
					
					pWorkinfo->fc_ipi_ring_info.skb->len-= len_diff;
				
					continue_tx = TRUE;

				}
				DUMP_PACKET(pWorkinfo->fc_ipi_ring_info.skb->data, pWorkinfo->fc_ipi_ring_info.skb->len, "[Decrypt]After IPI");
			}

			
			if(continue_tx == TRUE)
			{
/*			
				if(pWorkinfo->fc_ipi_ring_info.encrypt == RTK_FC_IPSEC_DIR_DECRYT)
				{
					dma_map_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.pCipher_text_address, pWorkinfo->fc_ipi_ring_info.crypto_len, DMA_TO_DEVICE);
				}
				else if(pWorkinfo->fc_ipi_ring_info.encrypt == RTK_FC_IPSEC_DIR_ENCRYT)
				{
					dma_map_single(&(ni_info_data.pdev->dev), pWorkinfo->fc_ipi_ring_info.pCipher_text_address, pWorkinfo->fc_ipi_ring_info.crypto_len+pWorkinfo->fc_ipi_ring_info.icv_size, DMA_TO_DEVICE);
				}
*/				
				/*Send with TX info*/
				RTK_FC_HELPER_MGR_NIC_TX(&pWorkinfo->fc_ipi_ring_info.nicTx_priv);
			}

			total_deq_cnt++;
			true_done_cnt--;
			scheduleidx += 1;

			last_count++;
			smp_mb();
			
			if(last_count >=RTK_FC_HW_MAX_JOBS){
				tmp_cnt = (last_count % RTK_FC_HW_MAX_JOBS);
				last_count = tmp_cnt;
			}
			smp_mb(); 
			// get next one
			scheduleidx &= (priv->priv_work_cnt-1);		
			smp_mb(); 
			atomic_set(&priv->priv_sched_idx, scheduleidx);
			
		}while(/*!timeout  && */(true_done_cnt>0) );

	}while(/*!timeout && */(true_done_cnt>0));

	if(no_packet_round_count >= 100)
	{
		atomic_set(&priv->csd_available, IPI_IDLE);// to allow smp_call_function
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
		no_packet_round_count = 0;
	}
	else
	{
		no_packet_round_count ++ ;
		goto RE_RUN;
		
	}
#if 0	
	cmdstat = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
	cmdok = cmdstat & (RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK));

	if(cmdok == 0)
	{
		if(no_packet_round_count >= 1000)
		{
			atomic_set(&priv->csd_available, IPI_IDLE);// to allow smp_call_function
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
			no_packet_round_count = 0;
		}
		else
		{
			no_packet_round_count ++ ;
			goto RE_RUN;
			
		}

	}
#if 0	
	else if(timeout)
	{
		ipsec_exec_timeout++;
		tasklet_hi_schedule(&priv->tasklet);
	}
#endif	
	else
	{
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK));
		
		goto RE_RUN;
		

	}
#endif	
	local_bh_enable();

	return;
}

__IRAM_FC_SHORTCUT
uint32_t rtk_fc_ipsec_process_irq(rtk_fc_ipsec_ipi_ctrl_t *priv)
{
	uint32_t cmdstat=0, cmdok=0, cmdstaterr=0;
	unsigned long flags;
	//mask interrupt mask to avoid interrupt
	rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0x1);
	local_irq_save(flags);

	if(fc_db.controlFuc.ipsec_pe_offload)
	{
		//clear error irq
		uint32 saadlr;
		
		cmdstaterr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT);
		if(cmdstaterr > 0)
		{
			WARNING("*********************cmdstaterr 0x%x*********************", cmdstaterr);
			if (cmdstaterr & 0x40)
			{
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x20L);
				WARNING("sum of a2eo = 0x%x",saadlr&0x7ff);

			}
			if (cmdstaterr & 0x100)
			{
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x28L);
				WARNING("sum of apl = 0x%x",saadlr&0x7ff);

			}
			if (cmdstaterr & 0x80)
			{
			
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
				WARNING("sum of etl = 0x%x",saadlr&0x7ff);

			}
			if (cmdstaterr & 0x2000)
			{
			
				saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
				WARNING("sum of etl = 0x%x",saadlr&0x7ff);

			}
			
			/* clear all test error*/
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT, cmdstaterr);
		}
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
	}
	else
	{
		cmdstat = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
		cmdok = cmdstat & (RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK));
		
		//CRYPTO_UNLOCK(&pdev->lock, lock_flag);
		
		IPSEC("%s:%d cmdstat %08x\n", __FUNCTION__, __LINE__, cmdstat);
		
		if(cmdok > 0){
			/* clear interrupt counter first, then clear cmd_ok */
			//rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, cmdstat | 0xFF0000);
				
			/* clear cmd_ok interrupt */
			//clear ok irq
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK));
			tasklet_hi_schedule(&priv->tasklet);

			//rtk_fc_ipsec_ipi_tasklet(priv);

		}
		else{
			//clear error irq
			uint32 saadlr;

			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat);
			
			cmdstaterr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT);
			if(cmdstaterr > 0){
				printk ("%s:%d \033[1;31;40m *********************cmdstat %x********************* \033[m\n", __FUNCTION__, __LINE__, cmdstat);
				printk ("%s:%d \033[1;31;40m *********************cmdstaterr %x********************* \033[m\n", __FUNCTION__, __LINE__, cmdstaterr);
				if (cmdstaterr & 0x40)
				{
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x20L);
					printk("sum of a2eo = 0x%x\n",saadlr&0x7ff);

				}
				if (cmdstaterr & 0x100)
				{
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x28L);
					printk("sum of apl = 0x%x\n",saadlr&0x7ff);

				}
				if (cmdstaterr & 0x80)
				{
				
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
					printk("sum of etl = 0x%x\n",saadlr&0x7ff);

				}
				if (cmdstaterr & 0x2000)
				{
				
					saadlr = rtk_fc_reg_io_read32 (rtk_fc_cryptoEngine_baseaddr + 0x24L);
					printk("sum of etl = 0x%x\n",saadlr&0x7ff);

				}
				
				/* clear all test error*/
				rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT, cmdstaterr);
			}
			else
			{
				rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR,  cmdstat | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK));
				tasklet_hi_schedule(&priv->tasklet);
			}
			rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
		}
	}
	
	local_irq_restore(flags);
    return 1;
}

__IRAM_FC_SHORTCUT
static irqreturn_t rtk_fc_ipsec_irq_handler(int irq, void *dev)
{
	rtk_fc_ipsec_ipi_ctrl_t *priv = platform_get_drvdata(to_platform_device(dev));
	
	if (!rtk_fc_ipsec_process_irq(priv)) {
			
			return IRQ_HANDLED;
	}

    return IRQ_HANDLED;
}
static int rtk_fc_ipsec_irqaffinity_register(unsigned int irq, int cpu)
{
    if (cpu >= NR_CPUS) {
        pr_err(KERN_ERR "irq affinity set error cpu %d larger than max cpu num\n", cpu);
        return -EINVAL;
    }

	if (cpu_online(cpu)) {
    	irq_set_affinity(irq, cpumask_of(cpu));

    	pr_info("irqaffinity irq %u register irq to cpu %d\n", irq, cpu);
	}

    return 0;
}



static int __devinit rtk_fc_crypto_engine_probe(struct platform_device *pdev)
{
    void *baseaddr;
    struct resource *mem, *irq;
    u32 revision=0;
    struct device_node *np = pdev->dev.of_node;
    int err;
	//int i;

    dev_info(&pdev->dev, "rtk_fc_crypto_engine_probe called!\n");

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!mem || !irq) {
        dev_err(&pdev->dev, "no memory/irq resource for rtk crypto engine\n");
        return -ENXIO;
    }
	rtk_fc_crypto_dev = &pdev->dev;

    baseaddr = devm_ioremap_nocache(&pdev->dev, mem->start, resource_size(mem));
    if (!baseaddr) {
        dev_err(&pdev->dev, "unable to map iomem\n");
        return -ENXIO;
    }
    
    err = of_property_read_u32(np, "revision", &revision);
    if (err) {
        dev_err(&pdev->dev, "unable to get revision\n");
        return -ENXIO;
    }
    printk("%s:%d revision 0x%x baseaddr = %p\n", __FUNCTION__, __LINE__, revision, baseaddr);

  
	
    if(rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_REVISION_ID) == revision){
        dev_info(&pdev->dev, "crypto_probe successfully!\n");
    }
    else{
        dev_err(&pdev->dev, "crypto_probe fail! value 0x%lx\n", rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_REVISION_ID));
        return -ENODEV;
    }

    /* Software Reset & fatal error Reset*/
    rtk_fc_reg_io_write32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_SOFT_RST) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_IPSEC_RST));

    /* little endian*/
#ifndef CONFIG_CPU_BIG_ENDIAN
    rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_SWAPABURSTR, (rtk_fc_reg_io_read32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_SWAPABURSTR) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_KEY_IV_SWAP) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_KEY_PAD_SWAP) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_HASH_INITIAL_VALUE_SWAP) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_DATA_IN_LITTLE_ENDIAN) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_DATA_OUT_LITTLE_ENDIAN) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_SWAPABURSTR_MAC_IN_LITTLE_ENDIAN)));
#endif
	rtk_fc_reg_io_write32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_DBG, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_DBG) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_DBG_ARBITER_MODE) | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_DBG_CLK_EN));

    /* clear all test error*/
    rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT, 0xFFFF);

    /* clear cmd_ok  */
	//rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR) | RTK_FC_BIT_MASK_ENABLE(_IPSCSR_RSTEACONFISR_INTR_MODE));
    //rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR) | RTK_FC_BIT_MASK_ENABLE(_IPSCSR_RSTEACONFISR_CMD_OK) );
    rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK) );

	/* disable interrupt*/
	rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0xFFFF);
	rtk_fc_cryptoEngine_baseaddr = baseaddr;
	printk("%s:%d %p mem->start 0x%llx rtk_fc_cryptoEngine_baseaddr = %p\n", __FUNCTION__, __LINE__, baseaddr, mem->start, rtk_fc_cryptoEngine_baseaddr);

/*	
	for(i = 0 ; i <RTK_FC_HW_MAX_JOBS ; i++)
	{
		iv_ptr[i] = dma_alloc_coherent(&(ni_info_data.pdev->dev), 16, &iv_dma[i], GFP_ATOMIC);
		if (!iv_ptr[i]) {
			printk("dma_alloc_coherent iv_ptr fail, i = %d\n", i );
			return RTK_FC_RET_DROP_IPSEC_DMAALLOC_FAIL;
		}

	}
*/	
	platform_set_drvdata(pdev, &rtk_fc_ipsec_ipi); //store ioremap info. for remove to release

	/*IPI jobs*/
	
	rtk_fc_ipsec_ipi.priv_work_cnt = MAX_FC_IPSEC_QUEUE_SIZE;
	atomic_set(&rtk_fc_ipsec_ipi.csd_available, IPI_IDLE);

	
	tasklet_init(&rtk_fc_ipsec_ipi.tasklet, rtk_fc_ipsec_ipi_exec, (unsigned long)&rtk_fc_ipsec_ipi);
	
	atomic_set(&rtk_fc_ipsec_ipi.priv_free_idx, 0);
	atomic_set(&rtk_fc_ipsec_ipi.priv_sched_idx, 0);

	rtk_fc_ipsec_ipi.csd.func = rtk_fc_ipsec_ipi_tasklet;
	rtk_fc_ipsec_ipi.csd.info = &rtk_fc_ipsec_ipi;
	

	{
		rtk_fc_ipsec_ipi.priv_work = &rtk_fc_ipsec_ipi_ring.priv_work[0];
		memset(&rtk_fc_ipsec_ipi_ring, 0, sizeof(rtk_fc_ipsec_ring_ctrl_t));
	}

	
	if (devm_request_irq(&pdev->dev, irq->start, rtk_fc_ipsec_irq_handler, 0, "rtk_crypto", &pdev->dev)) {
        dev_err(&pdev->dev, "failed to request IRQ\n");
        return -EBUSY;
    }

#if CONFIG_NR_CPUS == 2
	rtk_fc_ipsec_irqaffinity_register(irq->start, 0);//schedule to cpu 1
#elif CONFIG_NR_CPUS == 4
	rtk_fc_ipsec_irqaffinity_register(irq->start, 3);//schedule to cpu 1
#endif

	/*enable interrupt*/
	rtk_fc_reg_io_write32(baseaddr + RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);

    return of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
}
static int __devexit rtk_fc_crypto_engine_remove(struct platform_device *pdev)
{
   
    dev_info(&pdev->dev, "crypto device remove!\n");

	tasklet_kill(&rtk_fc_ipsec_ipi.tasklet);

    //release memory remapping
    if(pdev) devm_iounmap(&pdev->dev, rtk_fc_cryptoEngine_baseaddr);

    return 0;	
}

static const struct of_device_id rtk_fc_cryptoEngine_of_match[] = {
    { .compatible = "realtek,rtk-crypto", },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_fc_cryptoEngine_of_match);
static struct platform_driver rtk_fc_cryptoEngine_driver = {
    .probe  = rtk_fc_crypto_engine_probe,
    .remove = __devexit_p(rtk_fc_crypto_engine_remove),
    .driver = {
        .name  = "rtk-fc-crypto",
        .of_match_table = rtk_fc_cryptoEngine_of_match,
    },
};

static int __devinit rtk_fc_crypto_protect_probe(struct platform_device *pdev)
{
	void *baseaddr;
    struct resource *mem;

    dev_info(&pdev->dev, "protect_probe called!\n");

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem) {
        dev_err(&pdev->dev, "no memory resource for spacc\n");
        return -ENXIO;
    }
	printk("%s:%d %p mem->start 0x%llx\n", __FUNCTION__, __LINE__, baseaddr, mem->start);

    baseaddr = devm_ioremap_nocache(&pdev->dev, mem->start, resource_size(mem));
    if (!baseaddr) {
        dev_err(&pdev->dev, "unable to map iomem\n");
        return -ENXIO;
    }
    platform_set_drvdata(pdev, baseaddr); //store ioremap info. for remove to release
    
   
    

    rtk_fc_reg_io_write32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT) | RTK_FC_CRYPTO_PROTECT_ENABLE);
    if(rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT) & RTK_FC_CRYPTO_PROTECT_ENABLE){
        dev_info(&pdev->dev, "protect_probe successfully!\n");
    }
    else{
        dev_info(&pdev->dev, "protect_probe fail! value 0x%lx\n", rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT));
        return -ENODEV;
    }

    return of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
}

static int __devexit rtk_fc_crypto_protect_remove(struct platform_device *pdev)
{
    void *baseaddr;
    
    dev_info(&pdev->dev, "protect device remove!\n");

    baseaddr = platform_get_drvdata(pdev);

    rtk_fc_reg_io_write32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT) & (~RTK_FC_CRYPTO_PROTECT_ENABLE));
    
    printk("%s:%d value 0x%lx\n", __FUNCTION__, __LINE__, rtk_fc_reg_io_read32(baseaddr+RTK_FC_CRYPTO_REG_PROTECT));

    //release memory remapping
    devm_iounmap(&pdev->dev, baseaddr);

    return 0;
}

static const struct of_device_id rtk_fc_crypto_protect_of_match[] = {
    { .compatible = "realtek,rtl8277c-protect", },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_fc_crypto_protect_of_match);

static struct platform_driver rtk_fc_crypto_protect_driver = {
    .probe  = rtk_fc_crypto_protect_probe,
    .remove = __devexit_p(rtk_fc_crypto_protect_remove),
    .driver = {
        .name  = "rtl8277c-protect",
        .of_match_table = rtk_fc_crypto_protect_of_match,
    },
};

void __rtk_fc_ipsec_init(void)
{
	int err = 0;
	
	//protect device register

	err = platform_driver_register(&rtk_fc_crypto_protect_driver);
	if (err) {
		WARNING("rtk_fc_crypto_protect_driver register failed");
	}

	//crypto device register
    err = platform_driver_register(&rtk_fc_cryptoEngine_driver);
    if (err) {
       // platform_driver_unregister(&rtk_fc_crypto_protect_driver);
		WARNING("rtk_fc_cryptoEngine_driver register failed");

    }
	_rtk_fc_ipsec_descriptor_quick_init();
	hw_crypto_init = 1;
}
void __rtk_fc_ipsec_remove(void)
{
	
	platform_driver_unregister(&rtk_fc_cryptoEngine_driver);
	platform_driver_unregister(&rtk_fc_crypto_protect_driver);
	hw_crypto_init = 0;
}

void __rtk_fc_ipsec_init_check(void)
{
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	if(hw_crypto_init == 0)
	{
		__rtk_fc_ipsec_init();
	}
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
}

void __rtk_fc_ipsec_interrupt_set(uint32 enable)
{
	if(enable)
	{
		// enable interrupt
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0);
	}
	else
	{
		// disable interrupt
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_INTM, 0xFFFF);
	}

	return;
}

void __rtk_fc_ipsec_intr_counting_mode_set(uint32 enable)
{
	uint32 regValue = rtk_fc_reg_io_read32(rtk_fc_cryptoEngine_baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR);
	
	if(enable)
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, regValue | RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_INTR_MODE) );
	else
		rtk_fc_reg_io_write32(rtk_fc_cryptoEngine_baseaddr+RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR, regValue & ~RTK_FC_BIT_MASK_ENABLE(BIT_RTK_FC_IPSCSR_RSTEACONFISR_INTR_MODE) );

	return;
}

void __rtk_fc_ipsec_pe_offload_init(void)
{
	uint32 i;
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	
	for(i=0; i<RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid)
			fc_db.fc_ipsec_info[i].pe_crypto_sw_id = 0;
	}
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)	
	for(i=0; i<MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM; i++)
	{
		fc_db.pe_encrypt_info[i].fc_saInfo_idx = FAIL;
	}
	for(i=0; i<MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM; i++)
	{
		fc_db.pe_decrypt_info[i].fc_saInfo_idx = FAIL;
	}
#endif	
	_rtk_fc_ipsec_flow_flush_all(0);
	__rtk_fc_ipsec_interrupt_set(FALSE);
	fc_db.controlFuc.ipsec_pe_offload = 1;
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
}

void __rtk_fc_ipsec_pe_offload_remove(void)
{
	uint32 i;
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
	
	for(i=0; i<RTK_FC_SPEC_XFRM_INFO_MAX_NUM; i++)
	{
		if(fc_db.fc_ipsec_info[i].valid)
			fc_db.fc_ipsec_info[i].pe_crypto_sw_id = 0;
	}
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)	
	for(i=0; i<MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM; i++)
	{
		fc_db.pe_encrypt_info[i].fc_saInfo_idx = FAIL;
	}
	for(i=0; i<MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM; i++)
	{
		fc_db.pe_decrypt_info[i].fc_saInfo_idx = FAIL;
	}
#endif	
	_rtk_fc_ipsec_flow_flush_all(0);
	__rtk_fc_ipsec_interrupt_set(TRUE);
	fc_db.controlFuc.ipsec_pe_offload = 0;	
	
	RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_IPSEC, 0);
}

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
int32 rtk_fc_pe_crypto_free_connection_index_get(rtk_fc_ipsec_direction_t direction, int32 fc_saInfo_idx)
{
	int32 index=FAIL, i, freeIdx=FAIL, matchIdx=FAIL;

	if(direction==RTK_FC_IPSEC_DIR_ENCRYT)
	{	
		for(i=0; i<MAX_PE_IPSEC_ENCRYPTION_CONNECTION_NUM; i++)
		{
			if(fc_db.pe_encrypt_info[i].fc_saInfo_idx==FAIL) //invalid
			{
				if(freeIdx==FAIL)
					freeIdx = i;
				continue;
			}
			if(fc_db.pe_encrypt_info[i].fc_saInfo_idx == fc_saInfo_idx)
			{
				WARNING("Match pe_encrypt_info[%d], please check it!", i);
				matchIdx = i;
				break;
			}
		}
	}
	else if(direction==RTK_FC_IPSEC_DIR_DECRYT)
	{
		for(i=0; i<MAX_PE_IPSEC_DECRYPTION_CONNECTION_NUM; i++)
		{
			if(fc_db.pe_decrypt_info[i].fc_saInfo_idx==FAIL) //invalid
			{
				if(freeIdx==FAIL)
					freeIdx = i;
				continue;
			}
			if(fc_db.pe_decrypt_info[i].fc_saInfo_idx == fc_saInfo_idx)
			{
				WARNING("Match pDecrypt_info[%d], please check it!", i);
				matchIdx = i;
				break;
			}
		}
	}

	if(matchIdx != FAIL)
		index = matchIdx;
	else if(freeIdx != FAIL)
	{
		IPSEC("[PE] Get freeIdx[%d] of pe crypto %s connection", freeIdx, (direction==RTK_FC_IPSEC_DIR_ENCRYT)?"encrypt":"decrypt");
		index = freeIdx;
	}
	else
	{
		IPSEC("[PE] No free entry of pe crypto %s connection", (direction==RTK_FC_IPSEC_DIR_ENCRYT)?"encrypt":"decrypt");
		index = FAIL;
	}

	return index;
}

void rtk_fc_pe_crypto_conn_info_prepare(rtk_fc_ipsec_direction_t direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr)
{
	rt_pe_ret_t pe_ret;
	rt_pe_crypto_request_t pe_crypto_req;
	int32 pe_crypto_conn_idx=FAIL, fc_saInfo_idx=pPktHdr->ipsec_hook_table_index;
		
	if(fc_db.fc_ipsec_info[fc_saInfo_idx].direction != direction)
	{
		WARNING("IPSEC input direction %u dose not match saInfo_idx %u", direction, fc_saInfo_idx);
		return;
	}
	//PE only supports AES/DES/3DES-CBC MD5/SHA1/SHA2_256
	if(!((fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_AESCBC
			|| fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_DESCBC
			|| fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_3DESCBC) 
			&& (fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_MD5HMAC 
				 || fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA1HMAC
				 || fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA2_256HMAC)))
	{
		IPSEC("[PE] does not support ealgo %u aalgo %u", fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo, fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo);
		return;
	}
			
	if(direction==RTK_FC_IPSEC_DIR_ENCRYT)
	{					
		pe_crypto_conn_idx = rtk_fc_pe_crypto_free_connection_index_get(RTK_FC_IPSEC_DIR_ENCRYT, fc_saInfo_idx);
		if(pe_crypto_conn_idx != FAIL)
		{
			pe_crypto_req.req_cmd = RT_PE_CRYPTO_ENGINE_CMD_ENCRYPT_CONNECTION_ADD;
			pe_crypto_req.connection_idx = pe_crypto_conn_idx;
			pe_crypto_req.connection.encrypt_info.key_idx = fc_db.fc_ipsec_info[fc_saInfo_idx].key_index;
			pe_crypto_req.connection.encrypt_info.hash_key_idx = fc_db.fc_ipsec_info[fc_saInfo_idx].hash_key_index;
			if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_AESCBC)
				pe_crypto_req.connection.encrypt_info.cipher_mode = (fc_db.fc_ipsec_info[fc_saInfo_idx].key_sz==32) ? RT_PE_CRYPTO_EALG_AES_256 : ((fc_db.fc_ipsec_info[fc_saInfo_idx].key_sz==16) ? RT_PE_CRYPTO_EALG_AES_128 : RT_PE_CRYPTO_EALG_AES_192);
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_DESCBC)
				pe_crypto_req.connection.encrypt_info.cipher_mode = RT_PE_CRYPTO_EALG_DES;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_3DESCBC)
				pe_crypto_req.connection.encrypt_info.cipher_mode = RT_PE_CRYPTO_EALG_3DES;
			else	//not support
			{
				IPSEC("[PE] does not support ealgo %u", fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo);
				return;
			}
			if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_MD5HMAC)
				pe_crypto_req.connection.encrypt_info.hash_mode = RT_PE_CRYPTO_AALG_MD5HMAC;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA1HMAC)
				pe_crypto_req.connection.encrypt_info.hash_mode = RT_PE_CRYPTO_AALG_SHA1HMAC;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA2_256HMAC)
				pe_crypto_req.connection.encrypt_info.hash_mode = RT_PE_CRYPTO_AALG_SHA2_256HMAC;
			else	//not support
			{
				IPSEC("[PE] does not support aalgo %u", fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo);
				return;
			}
			pe_crypto_req.connection.encrypt_info.iv_len = fc_db.fc_ipsec_info[fc_saInfo_idx].iv_len;	
			pe_crypto_req.connection.encrypt_info.hash_icv_len = fc_db.fc_ipsec_info[fc_saInfo_idx].auth_len;		
			if(pPktHdr->eth==NULL)
			{
				WARNING("pPktHdr->eth is NULL!");
				return;
			}
			memcpy(pe_crypto_req.connection.encrypt_info.outer_dmac.octet, pPktHdr->eth->h_dest, ETHER_ADDR_LEN);
			memcpy(pe_crypto_req.connection.encrypt_info.outer_smac.octet, pPktHdr->eth->h_source, ETHER_ADDR_LEN);
			if(pPktHdr->iph)
			{
				pe_crypto_req.connection.encrypt_info.outer_isIPv4OrIpv6 = 0;
				pe_crypto_req.connection.encrypt_info.outer_sip.ipv4_addr = ntohl(pPktHdr->iph->saddr);
				pe_crypto_req.connection.encrypt_info.outer_dip.ipv4_addr = ntohl(pPktHdr->iph->daddr);
			}
			else if(pPktHdr->ip6h)
			{
				pe_crypto_req.connection.encrypt_info.outer_isIPv4OrIpv6 = 1;
				memcpy(&pe_crypto_req.connection.encrypt_info.outer_sip.ipv6_addr, &pPktHdr->ip6h->saddr, sizeof(struct in6_addr));
				memcpy(&pe_crypto_req.connection.encrypt_info.outer_dip.ipv6_addr, &pPktHdr->ip6h->daddr, sizeof(struct in6_addr));
			}
			else
			{
				WARNING("pPktHdr->iph and pPktHdr->ip6h are NULL!");
				return;
			}	
			if(pPktHdr->esph==NULL)
			{
				WARNING("pPktHdr->esph is NULL!");
				return;
			}
			pe_crypto_req.connection.encrypt_info.esp_spi = ntohl(pPktHdr->esph->spi);
			pe_crypto_req.connection.encrypt_info.esp_seq_no = ntohl(pPktHdr->esph->seq_no);
			_rtk_fc_ipsec_aescbc_iv_create(fc_saInfo_idx, pe_crypto_req.connection.encrypt_info.iv, RT_PE_IPSEC_IV_LEN, pe_crypto_req.connection.encrypt_info.esp_seq_no);
			pe_crypto_req.connection.encrypt_info.ldpid = pPktHdr->egressPort.macPortIdx;
			pe_crypto_req.connection.encrypt_info.tag_info.external_used.pppoe_sid = (pPktHdr->ppph) ? ntohs(pPktHdr->ppph->sid) : FAIL;
			if(pPktHdr->ppph && pe_crypto_req.connection.encrypt_info.tag_info.external_used.pppoe_sid==FAIL)
			{
				WARNING("pppoe_sid is overflow!");
				return;
			}	
			pe_crypto_req.connection.encrypt_info.tag_info.external_used.svlan_vid = (pPktHdr->svh) ? pPktHdr->svlanid : FAIL;
			pe_crypto_req.connection.encrypt_info.tag_info.external_used.cvlan_vid = (pPktHdr->cvh) ? pPktHdr->cvlanid : FAIL;
			pe_crypto_req.connection.encrypt_info.tag_info.external_used.pon_streamId = (pPktHdr->remarkDec.streamId_en) ? pPktHdr->remarkDec.streamId : FAIL;
			//send IPC to PE
			pe_ret = rtk_fc_pe_crypto_test(pe_crypto_req, fc_saInfo_idx, 0);
			if(pe_ret == RT_PE_RET_OK)
			{
				//record pe_crypto_sw_id
				fc_db.fc_ipsec_info[fc_saInfo_idx].pe_crypto_sw_id = pe_crypto_conn_idx + RT_PE_IPSEC_SW_ID_CONNECTION_IDX_BASE;
			}
			else
			{
				WARNING("Fail to call rtk_fc_pe_crypto_test, pe_ret=%d", pe_ret);
				return;
			}
		}
	}
	else if(direction==RTK_FC_IPSEC_DIR_DECRYT)
	{					
		pe_crypto_conn_idx = rtk_fc_pe_crypto_free_connection_index_get(RTK_FC_IPSEC_DIR_DECRYT, fc_saInfo_idx);
		if(pe_crypto_conn_idx != FAIL)
		{
			pe_crypto_req.req_cmd = RT_PE_CRYPTO_ENGINE_CMD_DECRYPT_CONNECTION_ADD;
			pe_crypto_req.connection_idx = pe_crypto_conn_idx;
			pe_crypto_req.connection.decrypt_info.key_idx = fc_db.fc_ipsec_info[fc_saInfo_idx].key_index;
			pe_crypto_req.connection.decrypt_info.hash_key_idx = fc_db.fc_ipsec_info[fc_saInfo_idx].hash_key_index;
			if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_AESCBC)
				pe_crypto_req.connection.decrypt_info.cipher_mode = (fc_db.fc_ipsec_info[fc_saInfo_idx].key_sz==32) ? RT_PE_CRYPTO_EALG_AES_256 : ((fc_db.fc_ipsec_info[fc_saInfo_idx].key_sz==16) ? RT_PE_CRYPTO_EALG_AES_128 : RT_PE_CRYPTO_EALG_AES_192);
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_DESCBC)
				pe_crypto_req.connection.decrypt_info.cipher_mode = RT_PE_CRYPTO_EALG_DES;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo==RTK_FC_EALG_3DESCBC)
				pe_crypto_req.connection.decrypt_info.cipher_mode = RT_PE_CRYPTO_EALG_3DES;
			else	//not support
			{
				IPSEC("[PE] does not support ealgo %u", fc_db.fc_ipsec_info[fc_saInfo_idx].ealgo);
				return;
			}
			if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_MD5HMAC)
				pe_crypto_req.connection.decrypt_info.hash_mode = RT_PE_CRYPTO_AALG_MD5HMAC;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA1HMAC)
				pe_crypto_req.connection.decrypt_info.hash_mode = RT_PE_CRYPTO_AALG_SHA1HMAC;
			else if(fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo==RTK_FC_AALG_SHA2_256HMAC)
				pe_crypto_req.connection.decrypt_info.hash_mode = RT_PE_CRYPTO_AALG_SHA2_256HMAC;
			else	//not support
			{
				IPSEC("[PE] does not support aalgo %u", fc_db.fc_ipsec_info[fc_saInfo_idx].aalgo);
				return;
			}
			pe_crypto_req.connection.decrypt_info.iv_len = fc_db.fc_ipsec_info[fc_saInfo_idx].iv_len;	
			pe_crypto_req.connection.decrypt_info.hash_icv_len = fc_db.fc_ipsec_info[fc_saInfo_idx].auth_len;
			pe_crypto_req.connection.decrypt_info.outer_isIPv4OrIpv6 = (pFcIngressData->ingressTagif & IPV6_TAGIF) ? 1 : 0 ;
			if(pPktHdr->eth==NULL)
			{
				WARNING("pPktHdr->eth is NULL!");
				return;
			}
			memcpy(pe_crypto_req.connection.decrypt_info.dmac.octet, pFcIngressData->da, ETHER_ADDR_LEN);
			memcpy(pe_crypto_req.connection.decrypt_info.smac.octet, pFcIngressData->sa, ETHER_ADDR_LEN);
			pe_crypto_req.connection.decrypt_info.hwlookup_swId = fc_saInfo_idx + RTK_FC_IPSEC_SWID_BASE;
			pe_crypto_req.connection.decrypt_info.hwlookup_lspid = RT_PE_IPSEC_DMA_LSO_DECRYPT_HWLOOKUP_LSPID;
			//send IPC to PE
			pe_ret = rtk_fc_pe_crypto_test(pe_crypto_req, fc_saInfo_idx, 0);
			if(pe_ret == RT_PE_RET_OK)
			{
				//record pe_crypto_sw_id
				fc_db.fc_ipsec_info[fc_saInfo_idx].pe_crypto_sw_id = (pe_crypto_conn_idx + RT_PE_IPSEC_SW_ID_CONNECTION_IDX_BASE) | RT_PE_IPSEC_SW_ID_DECRYPTION_BIT;
			}
			else
			{
				WARNING("Fail to call rtk_fc_pe_crypto_test, pe_ret=%d", pe_ret);
				return;
			}
		}
	}
	
	return;
}
#endif

#endif
