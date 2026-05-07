/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  Counter Storage Abstraction Layer (for ETC, ERC, and dfCounter)
 *
 *  Counter Storage is a File I/O-based counter storage designed for
 *  saving 1905 encryption related counters (such as encryption
 *  transmission counter, encryption reception counter, and decryption
 *  failure). The purpose of this storage is to retain the counter
 *  values as a file in the file system. Therefore, even if the daemon
 *  restarts due to reinit or autoconfiguration, the new daemon process
 *  can still find the previously accumulated counter values.
 *
 *  Counter Storage does all the File I/O work internally. You should only
 *  call the functions provided instead of modifying the file directly.
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _AL_DPP_COUNTERS_H_
#define _AL_DPP_COUNTERS_H_

/**
 * dpp_counters_get_etc - Get encryption transmission counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 *
 * Returns: a 64-bit unsigned counter, 0 if mac_addr is not found.
 */
INT64U dpp_counters_get_etc(INT8U *mac_addr);

/**
 * dpp_counters_get_erc - Get encryption reception counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 *
 * Returns: a 64-bit unsigned counter, 0 if mac_addr is not found.
 */
INT64U dpp_counters_get_erc(INT8U *mac_addr);

/**
 * dpp_counters_get_dfc - Get decryption failure counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 *
 * Returns: a 64-bit unsigned counter, 0 if mac_addr is not found.
 */
INT16U dpp_counters_get_dfc(INT8U *mac_addr);

/**
 * dpp_counters_set_etc - Set encryption transmission counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 * @enc_tx_ctr: the 64-bit unsigned counter value
 *
 * Returns: 1 for successfully set and sync, 0 for otherwise.
 */
INT8U dpp_counters_set_etc(INT8U *mac_addr, INT64U enc_tx_ctr);

/**
 * dpp_counters_set_erc - Set encryption reception counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 * @enc_rx_ctr: the 64-bit unsigned counter value
 *
 * Returns: 1 for successfully set and sync, 0 for otherwise.
 */
INT8U dpp_counters_set_erc(INT8U *mac_addr, INT64U enc_rx_ctr);

/**
 * dpp_counters_set_dfc - Set decryption failure counter
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 * @df_ctr: the 16-bit unsigned counter value
 *
 * Returns: 1 for successfully set and sync, 0 for otherwise.
 */
INT8U dpp_counters_set_dfc(INT8U *mac_addr, INT16U df_ctr);

/**
 * dpp_counters_reset - Reset counter values of a specific device
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 *
 * Returns: 1 for successfully set and sync, 0 for otherwise.
 */
INT8U dpp_counters_reset(INT8U *mac_addr);

/**
 * dpp_counters_remove_all - Delete all counter values in the storage
 *
 * Returns: 1 for successfully set and sync, 0 for otherwise.
 */
INT8U dpp_counters_remove_all();

#endif /* _AL_DPP_COUNTERS_H_ */