/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  DPP Reconfiguration
 *
 *  Copyright (C) 2022 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _DPP_RECONFIG_H_
#define _DPP_RECONFIG_H_

#include "al_dpp.h"

/**
 * dpp_rx_reconfig_announcement - Receive DPP Reconfiguration Announcement
 * Validate the incoming DPP Reconfiguration Announcement stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @buf: DPP Reconfiguration Announcement message to be parsed
 * @len: length of the DPP Reconfiguration Announcement message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_reconfig_announcement(struct dpp_authentication *auth, INT8U *buf, INT32U len);

/**
 * dpp_build_reconfig_auth_req - Build DPP Reconfiguration Authentication Request
 * Using the information stored in 'auth' to generate DPP Reconfiguration Authentication Request
 *
 * @auth: DPP authentication struct stored with authentication information
 * @reconfig_auth_req_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @reconfig_auth_req_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_reconfig_auth_req(struct dpp_authentication *auth, INT8U **reconfig_auth_req_msg, INT32U *reconfig_auth_req_msg_len);

/**
 * dpp_rx_reconfig_auth_resp - Receive DPP Reconfiguration Authentication Response
 * Validate the incoming DPP Reconfiguration Authentication Response stored in 'buf', by comparing the information stored in 'auth'.
 *
 * @auth: DPP authentication struct stored with authentication information
 * @conn_out: Signed connector parsed from the DPP Reconfiguration Authentication Response
 * @buf: DPP Reconfiguration Authentication Response message to be parsed
 * @len: length of the DPP Reconfiguration Authentication Response message
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_rx_reconfig_auth_resp(struct dpp_authentication *auth, struct dpp_connector *conn_out, INT8U *buf, INT32U len);

/**
 * dpp_build_reconfig_auth_conf - Build DPP Reconfiguration Authentication Confirm
 * Using the information stored in 'auth' to generate DPP Reconfiguration Authentication Confirm
 *
 * @auth: DPP authentication struct stored with authentication information
 * @reconfig_auth_conf_msg: upon successful execution, it stores the address of dynamically generated message buffer
 * @reconfig_auth_conf_msg_len: upon successful execution, it stores the length of the message buffer
 * Returns: 1 on success, 0 on failure
 */
INT8U dpp_build_reconfig_auth_conf(struct dpp_authentication *auth, INT8U **reconfig_auth_conf_msg, INT32U *reconfig_auth_conf_msg_len);

#endif