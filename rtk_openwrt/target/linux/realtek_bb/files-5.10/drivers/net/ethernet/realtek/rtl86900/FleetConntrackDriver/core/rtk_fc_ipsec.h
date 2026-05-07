/*
 * Copyright (C) 2017 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include "rtk_fc_struct.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_driver.h"

#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)


rtk_fc_ret_t _rtk_fc_ipsec_cryptEngine_polling_result(void);
rtk_fc_ret_t _rtk_fc_ipsec_cryptEngine_check_err(void);

rtk_fc_ret_t _rtk_fc_ipsec_gcm_shortCut_decrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint32 cryptlen, int ipsec_shortCut_info_table_index);
rtk_fc_ret_t _rtk_fc_ipsec_gcm_shortCut_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index, uint8 isESN);
rtk_fc_ret_t _rtk_fc_ipsec_gcm_shortCut_encrypt_test(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index, uint8 isESN);

int _rtk_fc_gcm_slow_path_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index);
int _rtk_fc_gcm_slow_path_decrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index);


void _rtk_fc_ipsec_encrypt_set_icv_dest_desc(int icv_len, int hash_padding_len, u8 *addr);
void _rtk_fc_ipsec_gcm_encrypt_set_cmdSetting(uint32 plaintext_length, uint32 aad_len, uint32 key_index, int encrypt, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr, int key_size);
void _rtk_fc_ipsec_gcm_decrypt_set_cmdSetting(uint32 plaintext_length, uint32 aad_len, uint32 key_index, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr, int key_size);

void  _rtk_fc_ipsec_set_iv_src_desc(int ivlen, int key_index, u8 *addrs, uint32_t *iv_dma);
void  _rtk_fc_ipsec_set_aad_src_desc(int aead_size, u8 *addr, uint32_t *aead_dma);
void _rtk_fc_ipsec_set_src_desc(int cryptlen, int cryp_pad_len, u8 *addr);
int _rtk_fc_ipsec_get_hwnatMode(void);
int _rtk_fc_ipsec_get_shortCut_en(void);
int _rtk_fc_ipsec_get_supported_mode(u8 cipher_mode, int hash_mode);
int _rtk_fc_ipsec_table_ready(uint32 spi);

int _rtk_fc_ipsec_setup_skbSecurePath(struct rt_skbuff *rtskb, struct xfrm_state *x_state);
int _rtk_fc_ipsec_shortCut_processing(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_smp_nicTx_private_t *pNicTxPriv);
void rtk_fc_ipsec_dump_ipi_info(void);
inline int _rtk_fc_ipsec_desc_fifo_full(void);

int _rtk_fc_ipsec_hwlookup_recovery(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_ipsec_insert_header(int mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable,uint32 flowTblIdx);
int rtk_fc_ipsec_remove_header(int cipher_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_ipsec_table_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx);
int _rtk_fc_aes_cbc_slow_path_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);
int _rtk_fc_aes_cbc_slow_path_encrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);
int _rtk_fc_des_cbc_slow_path_encrypt_ipi(int ring_index, int cipher_mode, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);

int _rtk_fc_aes_cbc_slow_path_decrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);
int _rtk_fc_gcm_slow_path_encrypt_ipi(int ring_index, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index);

rtk_fc_ret_t _rtk_fc_esp_aes_cbc_mix_mode_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index);
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_mix_mode_decrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint32 cryptlen, int ipsec_shortCut_info_table_index);
void _rtk_fc_ipec_gcm_test(void);
void _rtk_fc_ipec_gcm_test1(void);

void rtk_fc_hexdump(unsigned char *buf, unsigned int len);
void rtk_fc_reg_to_dev32_big(void *addr, const unsigned char *src, unsigned long nword);
void rtk_fc_reg_to_dev32_little(void *addr, const unsigned char *src, unsigned long nword);
void rtk_fc_reg_to_dev32_s(void *addr, const unsigned char *src, unsigned long nword);
int rtk_fc_ipsec_decrypt_set_dest_desc(uint32 cryptlen);
int _rtk_fc_ipsec_encrypt_set_dest_desc(uint32 plaintext_length, u8 *address);
int _rtk_fc_ipsec_aescbc_mixmode_set_cmdSetting(int hash_mode, uint32 ivlen, uint32 plaintext_length, uint32 hash_padding_len, uint32 aad_len, rtk_fc_pktHdr_t *pPktHdr, u8 key_index, u8 hash_key_index, int key_size, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr);

int _rtk_fc_ipsec_descbc_mixmode_set_cmdSetting(int hash_mode, int cipher_mode, uint32 ivlen, uint32 plaintext_length, uint32 hash_padding_len, uint32 aad_len, rtk_fc_pktHdr_t *pPktHdr, u8 key_index, u8 hash_key_index, int key_size, rtk_fc_cryptoEngine_command_t *pCmdSetting, uint32_t *cmdSetting_dmaPtr);
int _rtk_fc_ipsec_aescbc_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, u8 *src_aad_plaintext_hash_padding_buffer, uint32 total_len);
int _rtk_fc_ipsec_descbc_mixmode_set_src_desc(int hash_mode, uint32 aadlen, uint32 plaintext_length, uint32 hash_padding_len, rtk_fc_pktHdr_t *pPktHdr, uint32 hash_padding_array_idx, u8 *src_aad_plaintext_hash_padding_buffer, uint32 total_len);

void _rtk_fc_ipsec_dump_key_usage(void);


void  _rtk_fc_ipsec_aescbc_esn_set_hash_key(u8 *key_pad);

void rtk_fc_crypto_xor(u8 *dst, const u8 *src, unsigned int size);

rtk_fc_ret_t _rtk_fc_esp_aes_cbc_esn_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index);
int _rtk_fc_ipsec_encrypt_shortCut_set_dest_desc(uint32 plaintext_length, u8 *addr, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_esn_hash_ipi(rtk_fc_mac_port_idx_t macPortIdx, struct rt_skbuff *rtskb, uint16 ivsize, uint16 auth_sizes, int crypt_len, 
																int ipsec_shortCut_info_table_index, u8 *result_addr, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int direction,
																int cipher_mode, u8 *esp_addr);
rtk_fc_ret_t _rtk_fc_esp_aes_cbc_esn_encrypt_ipi(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 ivsize, uint16 auth_sizes, int ipsec_shortCut_info_table_index);

void _rtk_fc_ipsec_aescbc_iv_create(uint32 saInfo_index, uint8 *pIv, uint32 ivsize, uint32 esp_seq_no);
int _rtk_fc_ipsec_aescbc_esn_set_cipher_dest_desc(uint32 crypt_len, u8 *data);
int _rtk_fc_ipsec_aescbc_esn_set_cipheronly_cmdsetting(uint32 cryptlen, uint32 key_index);
void  _rtk_fc_ipsec_aescbc_esn_cipher_key(int key_size, u8 *addr);
void  _rtk_fc_ipsec_aescbc_esn_cipher_iv(int iv_size, u8 *addr);

int _rtk_fc_ipsec_debug_get(void);
int _rtk_fc_ipsec_debug_set(int val);


int _rtk_fc_ipsec_aescbc_esn_set_hashresult_desc(uint32 icv_len, u8 *addr);

int _rtk_fc_ipsec_aescbc_esn_hashonly_cmdsetting(uint32 hash_len, uint32 key_index, u8 *addr);
int _rtk_fc_aes_cbc_esn_slow_path_encrypt_ipi(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);

void  _rtk_fc_ipsec_aescbc_esn_hash_key_iv(u8 *key_pad);
int _rtk_fc_aes_cbc_esn_slow_path_encrypt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int auth_size, int cryptlen, int ipsec_shortCut_info_table_index, int ivsize);


int _rtk_fc_ipsec_recreate_seqNum(int cipher_mode, int iv_size, int auth_size, rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, int ipsec_shortCut_info_table_index, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int seq_no);



int rtk_fc_ipsec_write_hw_keypad(int index, u8 *key, int key_sz);

int rtk_fc_ipsec_write_hw_key(int index, u8 *key, int key_sz);
void rtk_fc_mix_mode_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index);
void rtk_fc_mix_mode_hmacMD5_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index);
void rtk_fc_des_mix_mode_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index);
void rtk_fc_des_mix_mode_hmacMD5_calculate_hash_padding(uint32 isHMAC, uint32 msglen, uint32 aadlen, int index);
int _rtk_fc_crypto_key_set(rt_crypto_key_set_t *rt_crypto_key_set_cfg, int *key_index);
int _rtk_fc_crypto_key_get_index(uint32 *index);
int _rtk_fc_crypto_key_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);
int _rtk_fc_crytpo_key_del_by_index(int index);
int _rtk_fc_crypto_keyhash_get_index(uint32 *index);
int _rtk_fc_crypto_keyhash_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);
int _rtk_fc_crytpo_keyhash_del_by_index(int index);


int _rtk_fc_ipsec_set_ipi_work_info(int *ring_index, int ipsec_hook_table_index, rtk_fc_mac_port_idx_t macPortIdx, struct rt_skbuff *rtskb, int encrypt, int cipher_mode, int iv_size, int aead_size, int auth_size, int total_crypt_len, u8  *pCipher_text_address, u8 *pIcv_address, rtk_fc_smp_nicTx_private_t *pNicTxPriv, int isEsn_encrypt, int isEsn_hash, rtk_fc_pktHdr_t *pPktHdr);

int _rtk_fc_esp_setting(int index, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, bool *swOnly, int direction);
int _rtk_fc_esp_policy_check(int *index, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int encrypt);

int _rtk_fc_egress_ipsec_seqNum_sync(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_smp_nicTx_private_t *pNicTxPriv);
void _rtk_fc_ipsec_flow_flush_all(uint32 lock);
void _rtk_fc_ipsec_flow_flush_by_saInfo_index(uint32 saInfo_index, uint32 lock);
void _rtk_fc_ipsec_saInfo_delete_by_index(uint32 saInfo_index, uint32 lock);
int _rtk_fc_ipsec_saInfo_add(uint32 direction, struct xfrm_state *x, struct xfrm_policy *xp);
int _rtk_fc_ipsec_saInfo_delete(uint32 spi);
int _rtk_fc_ipsec_saInfo_soft_delete(uint32 spi);
int _rtk_fc_ipsec_saInfo_flush_all(void);

void __rtk_fc_ipsec_init(void);
void __rtk_fc_ipsec_remove(void);

void _rtk_fc_aescbc_decrypt_test(void);
void __rtk_fc_ipsec_init_check(void);
void __rtk_fc_ipsec_interrupt_set(uint32 enable);
void __rtk_fc_ipsec_intr_counting_mode_set(uint32 enable);
void __rtk_fc_ipsec_pe_offload_init(void);
void __rtk_fc_ipsec_pe_offload_remove(void);
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
void rtk_fc_pe_crypto_conn_info_prepare(rtk_fc_ipsec_direction_t direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr);
#endif


#if defined(WEN_TEST)
void _rtk_fc_hash_test(void);
void _rtk_fc_ipec_gcm_test(void);
void _rtk_fc_ipec_cbc_test(void);
void _rtk_fc_ipec_gcm_esn_test(void);

int _rtk_fc_ipsec_test_set_hashresult_desc(uint32 icv_len);
int _rtk_fc_ipsec_test_set_cipher_desc(uint32 crypt_len);

int _rtk_fc_ipsec_test_hashonly_cmdsetting(uint32 hash_len, uint32 key_index);
int _rtk_fc_ipsec_test_cipheronly_cmdsetting(uint32 hash_len, uint32 key_index);

void  _rtk_fc_ipsec_test_hash_key_iv(u8 *key_pad);
void  _rtk_fc_ipsec_test_cipher_key(int iv_size);
void  _rtk_fc_ipsec_test_cipher_iv(int iv_size);

#endif

#endif
