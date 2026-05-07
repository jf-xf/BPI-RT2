

#ifndef __RT_CRYPTO_EXT_H__
#define __RT_CRYPTO_EXT_H__


/*
 * Include Files
 */
#include <common/rt_type.h>

typedef struct rt_crypto_key_set_s{
	int key_index;
	uint8 key[64];
	int key_sz;
}rt_crypto_key_set_t;

extern int rt_crypto_key_set(rt_crypto_key_set_t *rt_crypto_key_set_cfg, uint32 *key_idx);
extern int rt_crypto_key_get_index(uint32 *idx);
extern int rt_crypto_key_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);

extern int rt_crypto_key_del_by_index(uint32 *idx);

extern int rt_crypto_keyhash_get_index(uint32 *idx);
extern int rt_crypto_keyhash_add_by_index(rt_crypto_key_set_t *rt_crypto_key_set_cfg);
extern int rt_crypto_keyhash_del_by_index(uint32 *idx);

#endif /* __RT_CRYPTO_EXT_H__ */

