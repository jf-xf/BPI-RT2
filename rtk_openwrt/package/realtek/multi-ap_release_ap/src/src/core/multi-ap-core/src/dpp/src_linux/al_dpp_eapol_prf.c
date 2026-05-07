/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  1905 PTK Generation through Extensible Authentication Protocol over LAN (EAPOL)
 *  Pseudo-Random Function for PTK Derivation
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include "platform.h"
#include "packet_tools.h"

#include "platform_crypto.h"

// PRF for Generating PTK
INT8U dpp_ieee_prf_bits(INT32U hmac_len,
                        INT8U *key, INT32U key_len,
                        const char *label,
                        INT8U *data, INT32U data_len,
                        INT8U *out_buf, INT16U out_buf_len_bits)
{
	INT8U(*hmac_func)
	(INT8U *, INT32U, INT8U, INT8U **, INT32U *, INT8U *) = NULL;

	INT16U counter = 1;
	INT32U pos, plen;
	INT8U  hash[SHA512_MAC_LEN];
	INT8U *addr[4];
	INT32U len[4];
	INT8U  counter_le[2], length_le[2];
	INT16U buf_len = (out_buf_len_bits + 7) / 8;

	INT16U aux16;
	INT8U *p;

	// Select HMAC Function
	switch (hmac_len) {
	case SHA256_MAC_LEN:
		hmac_func = PLATFORM_HMAC_SHA256;
		break;
	case SHA384_MAC_LEN:
		hmac_func = PLATFORM_HMAC_SHA384;
		break;
	case SHA512_MAC_LEN:
		hmac_func = PLATFORM_HMAC_SHA512;
		break;
	default:
		PLATFORM_PRINTF_WARNING("HMAC-SHA%d not supported \n", hmac_len * 8);
		return 0;
	}

	// HMAC
	addr[0] = counter_le;
	len[0]  = 2;
	addr[1] = (INT8U *)label;
	len[1]  = PLATFORM_STRLEN(label);
	addr[2] = data;
	len[2]  = data_len;
	addr[3] = length_le;
	len[3]  = sizeof(length_le);

	// update length in bits (Little Endian)
	p     = length_le;
	aux16 = out_buf_len_bits;
	_I2B_LE(&aux16, &p);

	pos = 0;
	while (pos < buf_len) {

		// find remaining payload length
		plen = buf_len - pos;

		// update counter (little endian)
		p     = counter_le;
		aux16 = counter;
		_I2B_LE(&aux16, &p);

		// hash and append result to output buffer
		if (plen >= hmac_len) {
			if (hmac_func(key, key_len, 4, addr, len, &out_buf[pos]) != 1)
				return 0;
			pos += hmac_len;
		} else {
			if (hmac_func(key, key_len, 4, addr, len, hash) != 1)
				return 0;
			PLATFORM_MEMCPY(&out_buf[pos], hash, plen);
			pos += plen;
			break;
		}
		counter++;
	}

	// Mask out unused bits in the last octet if it does not use all the bits
	if (out_buf_len_bits % 8) {
		INT8U mask = 0xff << (8 - out_buf_len_bits % 8);
		out_buf[pos - 1] &= mask;
	}

	PLATFORM_MEMSET(hash, 0, sizeof(hash));
	return 1;
}