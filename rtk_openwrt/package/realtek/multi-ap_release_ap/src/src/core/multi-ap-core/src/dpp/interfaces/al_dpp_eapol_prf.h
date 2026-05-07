/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  1905 PTK Generation through Extensible Authentication Protocol over LAN (EAPOL)
 *  Pseudo-Random Function for PTK Derivation
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _DPP_EAPOL_PRF_H_
#define _DPP_EAPOL_PRF_H_

/**
 * dpp_ieee_prf_bits - IEEE Std 802.11ac-2013, 11.6.1.7.2 Key derivation function
 * (also listed in IEEE 802.11r, 8.5.1.5.2)
 *
 * @hmac_len: Length of HMAC-SHA256/384/512 function output
 *            (e.g., SHA256_MAC_LEN = HMAC-SHA256, SHA384_MAC_LEN = HMAC-SHA384)
 * @key: Key for KDF
 * @key_len: Length of the key in bytes
 * @label: A unique label for each purpose of the PRF
 * @data: Extra data to bind into the key
 * @data_len: Length of the data
 * @out_buf: Buffer for the generated pseudo-random key
 * @out_buf_len_bits: Number of bits of key to generate
 * Returns: 0 on success, -1 on failure
 *
 * This function is used to derive new, cryptographically separate keys from a
 * given key. If the requested buf_len is not divisible by eight, the least
 * significant 1-7 bits of the last octet in the output are not part of the
 * requested output.
 */
INT8U dpp_ieee_prf_bits(INT32U hmac_len,
                        INT8U *key, INT32U key_len,
                        const char *label,
                        INT8U *data, INT32U data_len,
                        INT8U *out_buf, INT16U out_buf_len_bits);

#endif