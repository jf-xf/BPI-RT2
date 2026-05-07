/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  Key Storage Abstraction Layer for GTK and PTK
 *
 *  Keystore is a File I/O-based key storage designed for saving 1905 GTK
 *  and PTKs. The purpose of keystore is to retain the keys as a file in
 *  the file system. Therefore, even if the daemon restarts due to reinit
 *  or autoconfiguration, the new daemon process can still find the
 *  previously derived keys.
 *
 *  Keystore does all the File I/O work internally. You should only call
 *  the functions provided instead of modifying the file directly.
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _DPP_KEYSTORE_H_
#define _DPP_KEYSTORE_H_

#include "al_dpp_eapol.h"

#define MAC_ADDR_LEN 6

/**
 * dpp_keystore_get_gtk - Get DPP 1905 GTK from keystore
 *
 * Returns: If the GTK is found, it returns the pointer of statically cached GTK.
 * Otherwise, it returns NULL.
 */
struct dpp_gtk *dpp_keystore_get_gtk();

/**
 * dpp_keystore_get_ptk - Get DPP 1905 PTK from keystore
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 *
 * Returns: If the PTK is found, it returns the pointer of statically cached PTK.
 * Otherwise, it returns NULL.
 */
struct dpp_ptk *dpp_keystore_get_ptk(INT8U *mac_addr);

/**
 * dpp_keystore_set_gtk - Set DPP 1905 GTK to keystore
 *
 * @gtk: the pointer of the GTK input
 * Returns: 1 for successfully set and sync. 0 for otherwise.
 */
INT8U dpp_keystore_set_gtk(struct dpp_gtk *gtk);

/**
 * dpp_keystore_set_ptk - Set DPP 1905 PTK to keystore
 *
 * @mac_addr: the MAC address of the peer device (in 6 octets)
 * @ptk: the pointer of the PTK input
 * Returns: 1 for successfully set and sync. 0 for otherwise.
 */
INT8U dpp_keystore_set_ptk(INT8U *mac_addr, struct dpp_ptk *ptk);

/**
 * dpp_keystore_remove_all - Remove all keys (GTK and PTKs) from keystore
 *
 * Returns: 1 for successfully set and sync. 0 for otherwise.
 */
INT8U dpp_keystore_remove_all();

INT8U *dpp_get_connected(INT8U *connected_nr);

#endif /* _DPP_KEYSTORE_H_ */