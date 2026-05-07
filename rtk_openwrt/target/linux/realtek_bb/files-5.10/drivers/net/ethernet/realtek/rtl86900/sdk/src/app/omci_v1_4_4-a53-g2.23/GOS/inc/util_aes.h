#ifndef _UTIL_AES_H_
#define _UTIL_AES_H_

#include <stdint.h>
#include "gos_type.h"


// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.


void AES128_ECB_encrypt(uint8_t *aes_key, uint8_t *key, uint8_t *encryped_key);
void AES128_ECB_decrypt(uint8_t *aes_key, uint8_t *encryped_key, uint8_t *key);

void AES128_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES128_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

GOS_ERROR_CODE aes128_cmac_compute(uint8_t key[], uint8_t msg[], uint8_t msg_len, uint8_t mac_size, uint8_t mac[]);

#endif //_UTIL_AES_H_
