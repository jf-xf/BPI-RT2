/***********************************************************
  AX UDP Daemon for PHL architecture
 ***********************************************************/
//#define CHECK_LEN
//#define CONFIG_PHL_TEST_MP

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "AX_UDPserver.h"
#include "./phl_types.h"
#include "./phl_test_mp_def.h"

#ifndef CONFIG_CPU_BIG_ENDIAN
#include <endian.h>
#include <stddef.h>
#endif

#define CHECK_LEN 0

/*socket related*/
int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t addr_len;

/*sub ioctl command used for phl mp test*/
char phl_get_io_cmd[] = {"phl_get_io "};
char phl_set_io_cmd[] = {"phl_set_io "};
char phl_init_test_cmd[] = {"phl_test_init "};
char phl_deinit_test_cmd[] = {"phl_test_deinit "};
char flash_set_cmd[] = {"flash set "};
char flash_get_cmd[] = {"flash get "};

int rtw_phl_ioctl(int skfd, char *inifname, void *buffer, unsigned int buffer_size){

    int err;
    int ret;

    struct ifreq ifr;
    union iwreq_data u;

    err = 0;
    memset(&u, 0, sizeof(union iwreq_data));
    u.data.pointer = buffer;
    u.data.length = (unsigned short)buffer_size;

#if CHECK_LEN
    printf("\n[debug] u.data.length to ioctl = %d\n", u.data.length); //cannot larger than 2000
#endif

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, inifname, strlen(inifname));
    ifr.ifr_ifru.ifru_data = &u;

    err = ioctl(skfd, SIOCDEVPRIVATE, &ifr);

    if(u.data.length == 0){
	*(char*)buffer = 0;
    }

    return err;
}

bool rtw_check_flash_op(struct argshell *src){

    if(((u8*)src->buf)[0] == MP_CLASS_FLASH){
		return true;
    }

    return false;
}

void sync_shadowmap(struct mp_flash_arg *arg, char *ifname){

	char item[64], data[64], wlan_interface[6];
	char tmp[64];
	char iwpriv_cmd[128];

	sscanf(arg->flash_cmd, "flash set %s %s", tmp, data);

	if (0 == strcmp(tmp , "HW_XCAP_WLAN0_AX")){
		strcpy(wlan_interface, "wlan1");
		strcpy(item, "xcap");
	} else if (0 == strcmp(tmp , "HW_XCAP_WLAN1_AX")){
		strcpy(wlan_interface, "wlan0");
		strcpy(item, "xcap");
	} else if (0 == strcmp(tmp, "HW_THERMAL_A_WLAN0")){
		strcpy(wlan_interface, "wlan1");
		strcpy(item, "thermalA");
	} else if (0 == strcmp(tmp, "HW_THERMAL_A_WLAN1")){
		strcpy(wlan_interface, "wlan0");
		strcpy(item, "thermalA");
	} else if (0 == strcmp(tmp, "HW_THERMAL_B_WLAN0")){
		strcpy(wlan_interface, "wlan1");
		strcpy(item, "thermalB");
	} else if (0 == strcmp(tmp, "HW_THERMAL_B_WLAN1")){
		strcpy(wlan_interface, "wlan0");
		strcpy(item, "thermalB");
	} else if (0 == strcmp(tmp, "HW_THERMAL_C_WLAN0")){
		strcpy(wlan_interface, "wlan1");
		strcpy(item, "thermalC");
	} else if (0 == strcmp(tmp, "HW_THERMAL_C_WLAN1")){
		strcpy(wlan_interface, "wlan0");
		strcpy(item, "thermalC");
	} else if (0 == strcmp(tmp, "HW_THERMAL_D_WLAN0")){
		strcpy(wlan_interface, "wlan1");
		strcpy(item, "thermalD");
	} else if (0 == strcmp(tmp, "HW_THERMAL_D_WLAN1")){
		strcpy(wlan_interface, "wlan0");
		strcpy(item, "thermalD");
	} else if (0 == strcmp(tmp, "HW_2G_CCK_TSSI_A")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_cck_tssi_A");

	} else if (0 == strcmp(tmp, "HW_2G_BW40_1S_TSSI_A")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_bw40_1s_tssi_A");

	} else if (0 == strcmp(tmp, "HW_5G_BW40_1S_TSSI_A")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_bw40_1s_tssi_A");

	} else if (0 == strcmp(tmp, "HW_2G_CCK_TSSI_B")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_cck_tssi_B");

	} else if (0 == strcmp(tmp, "HW_2G_BW40_1S_TSSI_B")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_bw40_1s_tssi_B");

	} else if (0 == strcmp(tmp, "HW_5G_BW40_1S_TSSI_B")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_bw40_1s_tssi_B");

	} else if (0 == strcmp(tmp, "HW_2G_CCK_TSSI_C")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_cck_tssi_C");

	} else if (0 == strcmp(tmp, "HW_2G_BW40_1S_TSSI_C")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_bw40_1s_tssi_C");

	} else if (0 == strcmp(tmp, "HW_5G_BW40_1S_TSSI_C")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_bw40_1s_tssi_C");

	} else if (0 == strcmp(tmp, "HW_2G_CCK_TSSI_D")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_cck_tssi_D");

	} else if (0 == strcmp(tmp, "HW_2G_BW40_1S_TSSI_D")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_bw40_1s_tssi_D");

	} else if (0 == strcmp(tmp, "HW_5G_BW40_1S_TSSI_D")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_bw40_1s_tssi_D");

	} else if (0 == strcmp(tmp, "HW_RX_GAINGAP_2G_OFDM_AB")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_rx_gain_ofdm");

	} else if (0 == strcmp(tmp, "HW_RX_GAINGAP_2G_CCK_AB")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "2G_rx_gain_cck");

	} else if (0 == strcmp(tmp, "HW_RX_GAINGAP_5GL_AB")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_rx_gain_low");

	} else if (0 == strcmp(tmp, "HW_RX_GAINGAP_5GM_AB")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_rx_gain_mid");

	} else if (0 == strcmp(tmp, "HW_RX_GAINGAP_5GH_AB")){
		strcpy(wlan_interface, ifname);
		strcpy(item, "5G_rx_gain_high");

	}

	sprintf(iwpriv_cmd, "iwpriv %s flash_set %s=%s", wlan_interface, item, data);
	system(iwpriv_cmd);
}

#ifndef CONFIG_CPU_BIG_ENDIAN
void endian_convert(struct argshell *args, int direction)
{
	struct mp_arg_hdr *hdr = (struct mp_arg_hdr *)args->buf;
	struct mp_config_arg *config_arg = (struct mp_config_arg *)args->buf;
	struct mp_tx_arg *tx_arg = (struct mp_tx_arg *)args->buf;
	struct mp_rx_arg *rx_arg = (struct mp_rx_arg *)args->buf;
	struct mp_efuse_arg *efuse_arg = (struct mp_efuse_arg *)args->buf;
	struct mp_reg_arg *reg_arg = (struct mp_reg_arg *)args->buf;
	struct mp_txpwr_arg *txpwr_arg = (struct mp_txpwr_arg *)args->buf;
	struct mp_cal_arg *cal_arg = (struct mp_cal_arg *)args->buf;
	struct mp_flash_arg *flash_arg = (struct mp_flash_arg *)args->buf;
	int idx = 0;

	if (args->type != 0)
		return;

	args->len = endian_convert16(args->len);

	switch (hdr->mp_class) {
	case MP_CLASS_CONFIG:
		config_arg->chipid = endian_convert32(config_arg->chipid);
		config_arg->offset = endian_convert32(config_arg->offset);
		break;	
	case MP_CLASS_TX:
		tx_arg->plcp_case_id = endian_convert16(tx_arg->plcp_case_id);
		tx_arg->tx_cnt = endian_convert16(tx_arg->tx_cnt);
		tx_arg->period = endian_convert16(tx_arg->period);
		tx_arg->tx_time = endian_convert16(tx_arg->tx_time);
		tx_arg->tx_ok = endian_convert32(tx_arg->tx_ok);
		tx_arg->dbw = endian_convert32(tx_arg->dbw);
		tx_arg->source_gen_mode = endian_convert32(tx_arg->source_gen_mode);
		tx_arg->locked_clk = endian_convert32(tx_arg->locked_clk);
		tx_arg->dyn_bw = endian_convert32(tx_arg->dyn_bw);
		tx_arg->ndp_en = endian_convert32(tx_arg->ndp_en);
		tx_arg->long_preamble_en = endian_convert32(tx_arg->long_preamble_en);
		tx_arg->stbc = endian_convert32(tx_arg->stbc);
		tx_arg->gi = endian_convert32(tx_arg->gi);
		tx_arg->tb_l_len = endian_convert32(tx_arg->tb_l_len);
		tx_arg->tb_ru_tot_sts_max = endian_convert32(tx_arg->tb_ru_tot_sts_max);
		tx_arg->vht_txop_not_allowed = endian_convert32(tx_arg->vht_txop_not_allowed);
		tx_arg->tb_disam = endian_convert32(tx_arg->tb_disam);
		tx_arg->doppler = endian_convert32(tx_arg->doppler);
		tx_arg->he_ltf_type = endian_convert32(tx_arg->he_ltf_type);
		tx_arg->ht_l_len = endian_convert32(tx_arg->ht_l_len);
		tx_arg->preamble_puncture = endian_convert32(tx_arg->preamble_puncture);
		tx_arg->he_mcs_sigb = endian_convert32(tx_arg->he_mcs_sigb);
		tx_arg->he_dcm_sigb = endian_convert32(tx_arg->he_dcm_sigb);
		tx_arg->he_sigb_compress_en = endian_convert32(tx_arg->he_sigb_compress_en);
		tx_arg->max_tx_time_0p4us = endian_convert32(tx_arg->max_tx_time_0p4us);
		tx_arg->ul_flag = endian_convert32(tx_arg->ul_flag);
		tx_arg->tb_ldpc_extra = endian_convert32(tx_arg->tb_ldpc_extra);
		tx_arg->bss_color = endian_convert32(tx_arg->bss_color);
		tx_arg->sr = endian_convert32(tx_arg->sr);
		tx_arg->beamchange_en = endian_convert32(tx_arg->beamchange_en);
		tx_arg->he_er_u106ru_en = endian_convert32(tx_arg->he_er_u106ru_en);
		tx_arg->ul_srp1 = endian_convert32(tx_arg->ul_srp1);
		tx_arg->ul_srp2 = endian_convert32(tx_arg->ul_srp2);
		tx_arg->ul_srp3 = endian_convert32(tx_arg->ul_srp3);
		tx_arg->ul_srp4 = endian_convert32(tx_arg->ul_srp4);
		tx_arg->mode = endian_convert32(tx_arg->mode);
		tx_arg->group_id = endian_convert32(tx_arg->group_id);
		tx_arg->ppdu_type = endian_convert32(tx_arg->ppdu_type);
		tx_arg->txop = endian_convert32(tx_arg->txop);
		tx_arg->tb_strt_sts = endian_convert32(tx_arg->tb_strt_sts);
		tx_arg->tb_pre_fec_padding_factor = endian_convert32(tx_arg->tb_pre_fec_padding_factor);
		tx_arg->cbw = endian_convert32(tx_arg->cbw);
		tx_arg->txsc = endian_convert32(tx_arg->txsc);
		tx_arg->tb_mumimo_mode_en = endian_convert32(tx_arg->tb_mumimo_mode_en);
		tx_arg->nominal_t_pe = endian_convert32(tx_arg->nominal_t_pe);
		tx_arg->ness = endian_convert32(tx_arg->ness);
		tx_arg->n_user = endian_convert32(tx_arg->n_user);
		tx_arg->tb_rsvd = endian_convert32(tx_arg->tb_rsvd);
		tx_arg->plcp_usr_idx = endian_convert32(tx_arg->plcp_usr_idx);
		tx_arg->mcs = endian_convert32(tx_arg->mcs);
		tx_arg->mpdu_len = endian_convert32(tx_arg->mpdu_len);
		tx_arg->n_mpdu = endian_convert32(tx_arg->n_mpdu);
		tx_arg->fec = endian_convert32(tx_arg->fec);
		tx_arg->dcm = endian_convert32(tx_arg->dcm);
		tx_arg->aid = endian_convert32(tx_arg->aid);
		tx_arg->scrambler_seed = endian_convert32(tx_arg->scrambler_seed);
		tx_arg->random_init_seed = endian_convert32(tx_arg->random_init_seed);
		tx_arg->apep = endian_convert32(tx_arg->apep);
		tx_arg->ru_alloc = endian_convert32(tx_arg->ru_alloc);
		tx_arg->nss = endian_convert32(tx_arg->nss);
		tx_arg->txbf = endian_convert32(tx_arg->txbf);
		tx_arg->pwr_boost_db = endian_convert32(tx_arg->pwr_boost_db);
		tx_arg->data_rate = endian_convert32(tx_arg->data_rate);
		tx_arg->pref_AC_0 = endian_convert32(tx_arg->pref_AC_0);
		tx_arg->aid12_0 = endian_convert32(tx_arg->aid12_0);
		tx_arg->ul_mcs_0 = endian_convert32(tx_arg->ul_mcs_0);
		tx_arg->macid_0 = endian_convert32(tx_arg->macid_0);
		tx_arg->ru_pos_0 = endian_convert32(tx_arg->ru_pos_0);
		tx_arg->ul_fec_code_0 = endian_convert32(tx_arg->ul_fec_code_0);
		tx_arg->ul_dcm_0 = endian_convert32(tx_arg->ul_dcm_0);
		tx_arg->ss_alloc_0 = endian_convert32(tx_arg->ss_alloc_0);
		tx_arg->ul_tgt_rssi_0 = endian_convert32(tx_arg->ul_tgt_rssi_0);
		tx_arg->pref_AC_1 = endian_convert32(tx_arg->pref_AC_1);
		tx_arg->aid12_1 = endian_convert32(tx_arg->aid12_1);
		tx_arg->ul_mcs_1 = endian_convert32(tx_arg->ul_mcs_1);
		tx_arg->macid_1 = endian_convert32(tx_arg->macid_1);
		tx_arg->ru_pos_1 = endian_convert32(tx_arg->ru_pos_1);
		tx_arg->ul_fec_code_1 = endian_convert32(tx_arg->ul_fec_code_1);
		tx_arg->ul_dcm_1 = endian_convert32(tx_arg->ul_dcm_1);
		tx_arg->ss_alloc_1 = endian_convert32(tx_arg->ss_alloc_1);
		tx_arg->ul_tgt_rssi_1 = endian_convert32(tx_arg->ul_tgt_rssi_1);
		tx_arg->pref_AC_2 = endian_convert32(tx_arg->pref_AC_2);
		tx_arg->aid12_2 = endian_convert32(tx_arg->aid12_2);
		tx_arg->ul_mcs_2 = endian_convert32(tx_arg->ul_mcs_2);
		tx_arg->macid_2 = endian_convert32(tx_arg->macid_2);
		tx_arg->ru_pos_2 = endian_convert32(tx_arg->ru_pos_2);
		tx_arg->ul_fec_code_2 = endian_convert32(tx_arg->ul_fec_code_2);
		tx_arg->ul_dcm_2 = endian_convert32(tx_arg->ul_dcm_2);
		tx_arg->ss_alloc_2 = endian_convert32(tx_arg->ss_alloc_2);
		tx_arg->ul_tgt_rssi_2 = endian_convert32(tx_arg->ul_tgt_rssi_2);
		tx_arg->pref_AC_3 = endian_convert32(tx_arg->pref_AC_3);
		tx_arg->aid12_3 = endian_convert32(tx_arg->aid12_3);
		tx_arg->ul_mcs_3 = endian_convert32(tx_arg->ul_mcs_3);
		tx_arg->macid_3 = endian_convert32(tx_arg->macid_3);
		tx_arg->ru_pos_3 = endian_convert32(tx_arg->ru_pos_3);
		tx_arg->ul_fec_code_3 = endian_convert32(tx_arg->ul_fec_code_3);
		tx_arg->ul_dcm_3 = endian_convert32(tx_arg->ul_dcm_3);
		tx_arg->ss_alloc_3 = endian_convert32(tx_arg->ss_alloc_3);
		tx_arg->ul_tgt_rssi_3 = endian_convert32(tx_arg->ul_tgt_rssi_3);
		tx_arg->ul_bw = endian_convert32(tx_arg->ul_bw);
		tx_arg->gi_ltf = endian_convert32(tx_arg->gi_ltf);
		tx_arg->num_he_ltf = endian_convert32(tx_arg->num_he_ltf);
		tx_arg->ul_stbc = endian_convert32(tx_arg->ul_stbc);
		tx_arg->pkt_doppler = endian_convert32(tx_arg->pkt_doppler);
		tx_arg->ap_tx_power = endian_convert32(tx_arg->ap_tx_power);
		tx_arg->user_num = endian_convert32(tx_arg->user_num);
		tx_arg->pktnum = endian_convert32(tx_arg->pktnum);
		tx_arg->pri20_bitmap = endian_convert32(tx_arg->pri20_bitmap);
		tx_arg->datarate = endian_convert32(tx_arg->datarate);
		tx_arg->mulport_id = endian_convert32(tx_arg->mulport_id);
		tx_arg->pwr_ofset = endian_convert32(tx_arg->pwr_ofset);
		tx_arg->f2p_mode = endian_convert32(tx_arg->f2p_mode);
		tx_arg->frexch_type = endian_convert32(tx_arg->frexch_type);
		tx_arg->sigb_len = endian_convert32(tx_arg->sigb_len);
		tx_arg->cmd_qsel = endian_convert32(tx_arg->cmd_qsel);
		tx_arg->ls = endian_convert32(tx_arg->ls);
		tx_arg->fs = endian_convert32(tx_arg->fs);
		tx_arg->total_number = endian_convert32(tx_arg->total_number);
		tx_arg->seq = endian_convert32(tx_arg->seq);
		tx_arg->length = endian_convert32(tx_arg->length);
		tx_arg->cmd_type = endian_convert32(tx_arg->cmd_type);
		tx_arg->cmd_sub_type = endian_convert32(tx_arg->cmd_sub_type);
		tx_arg->dl_user_num = endian_convert32(tx_arg->dl_user_num);
		tx_arg->bw = endian_convert32(tx_arg->bw);
		tx_arg->tx_power = endian_convert32(tx_arg->tx_power);
		tx_arg->fw_define = endian_convert32(tx_arg->fw_define);
		tx_arg->ss_sel_mode = endian_convert32(tx_arg->ss_sel_mode);
		tx_arg->next_qsel = endian_convert32(tx_arg->next_qsel);
		tx_arg->twt_group = endian_convert32(tx_arg->twt_group);
		tx_arg->dis_chk_slp = endian_convert32(tx_arg->dis_chk_slp);
		tx_arg->ru_mu_2_su = endian_convert32(tx_arg->ru_mu_2_su);
		tx_arg->dl_t_pe = endian_convert32(tx_arg->dl_t_pe);
		tx_arg->sigb_ch1_len = endian_convert32(tx_arg->sigb_ch1_len);
		tx_arg->sigb_ch2_len = endian_convert32(tx_arg->sigb_ch2_len);
		tx_arg->sigb_sym_num = endian_convert32(tx_arg->sigb_sym_num);
		tx_arg->sigb_ch2_ofs = endian_convert32(tx_arg->sigb_ch2_ofs);
		tx_arg->dis_htp_ack = endian_convert32(tx_arg->dis_htp_ack);
		tx_arg->tx_time_ref = endian_convert32(tx_arg->tx_time_ref);
		tx_arg->pri_user_idx = endian_convert32(tx_arg->pri_user_idx);
		tx_arg->ampdu_max_txtime = endian_convert32(tx_arg->ampdu_max_txtime);
		tx_arg->d3_group_id = endian_convert32(tx_arg->d3_group_id);
		tx_arg->twt_chk_en = endian_convert32(tx_arg->twt_chk_en);
		tx_arg->twt_port_id = endian_convert32(tx_arg->twt_port_id);
		tx_arg->twt_start_time = endian_convert32(tx_arg->twt_start_time);
		tx_arg->twt_end_time = endian_convert32(tx_arg->twt_end_time);
		tx_arg->apep_len = endian_convert32(tx_arg->apep_len);
		tx_arg->tri_pad = endian_convert32(tx_arg->tri_pad);
		tx_arg->ul_t_pe = endian_convert32(tx_arg->ul_t_pe);
		tx_arg->rf_gain_idx = endian_convert32(tx_arg->rf_gain_idx);
		tx_arg->fixed_gain_en = endian_convert32(tx_arg->fixed_gain_en);
		tx_arg->ul_gi_ltf = endian_convert32(tx_arg->ul_gi_ltf);
		tx_arg->ul_doppler = endian_convert32(tx_arg->ul_doppler);
		tx_arg->d6_ul_stbc = endian_convert32(tx_arg->d6_ul_stbc);
		tx_arg->ul_mid_per = endian_convert32(tx_arg->ul_mid_per);
		tx_arg->ul_cqi_rrp_tri = endian_convert32(tx_arg->ul_cqi_rrp_tri);
		tx_arg->sigb_dcm = endian_convert32(tx_arg->sigb_dcm);
		tx_arg->sigb_comp = endian_convert32(tx_arg->sigb_comp);
		tx_arg->d7_doppler = endian_convert32(tx_arg->d7_doppler);
		tx_arg->d7_stbc = endian_convert32(tx_arg->d7_stbc);
		tx_arg->mid_per = endian_convert32(tx_arg->mid_per);
		tx_arg->gi_ltf_size = endian_convert32(tx_arg->gi_ltf_size);
		tx_arg->sigb_mcs = endian_convert32(tx_arg->sigb_mcs);
		tx_arg->macid_u0 = endian_convert32(tx_arg->macid_u0);
		tx_arg->ac_type_u0 = endian_convert32(tx_arg->ac_type_u0);
		tx_arg->mu_sta_pos_u0 = endian_convert32(tx_arg->mu_sta_pos_u0);
		tx_arg->dl_rate_idx_u0 = endian_convert32(tx_arg->dl_rate_idx_u0);
		tx_arg->dl_dcm_en_u0 = endian_convert32(tx_arg->dl_dcm_en_u0);
		tx_arg->ru_alo_idx_u0 = endian_convert32(tx_arg->ru_alo_idx_u0);
		tx_arg->pwr_boost_u0 = endian_convert32(tx_arg->pwr_boost_u0);
		tx_arg->agg_bmp_alo_u0 = endian_convert32(tx_arg->agg_bmp_alo_u0);
		tx_arg->ampdu_max_txnum_u0 = endian_convert32(tx_arg->ampdu_max_txnum_u0);
		tx_arg->user_define_u0 = endian_convert32(tx_arg->user_define_u0);
		tx_arg->user_define_ext_u0 = endian_convert32(tx_arg->user_define_ext_u0);
		tx_arg->ul_addr_idx_u0 = endian_convert32(tx_arg->ul_addr_idx_u0);
		tx_arg->ul_dcm_u0 = endian_convert32(tx_arg->ul_dcm_u0);
		tx_arg->ul_fec_cod_u0 = endian_convert32(tx_arg->ul_fec_cod_u0);
		tx_arg->ul_ru_rate_u0 = endian_convert32(tx_arg->ul_ru_rate_u0);
		tx_arg->ul_ru_alo_idx_u0 = endian_convert32(tx_arg->ul_ru_alo_idx_u0);
		tx_arg->macid_u1 = endian_convert32(tx_arg->macid_u1);
		tx_arg->ac_type_u1 = endian_convert32(tx_arg->ac_type_u1);
		tx_arg->mu_sta_pos_u1 = endian_convert32(tx_arg->mu_sta_pos_u1);
		tx_arg->dl_rate_idx_u1 = endian_convert32(tx_arg->dl_rate_idx_u1);
		tx_arg->dl_dcm_en_u1 = endian_convert32(tx_arg->dl_dcm_en_u1);
		tx_arg->ru_alo_idx_u1 = endian_convert32(tx_arg->ru_alo_idx_u1);
		tx_arg->pwr_boost_u1 = endian_convert32(tx_arg->pwr_boost_u1);
		tx_arg->agg_bmp_alo_u1 = endian_convert32(tx_arg->agg_bmp_alo_u1);
		tx_arg->ampdu_max_txnum_u1 = endian_convert32(tx_arg->ampdu_max_txnum_u1);
		tx_arg->user_define_u1 = endian_convert32(tx_arg->user_define_u1);
		tx_arg->user_define_ext_u1 = endian_convert32(tx_arg->user_define_ext_u1);
		tx_arg->ul_addr_idx_u1 = endian_convert32(tx_arg->ul_addr_idx_u1);
		tx_arg->ul_dcm_u1 = endian_convert32(tx_arg->ul_dcm_u1);
		tx_arg->ul_fec_cod_u1 = endian_convert32(tx_arg->ul_fec_cod_u1);
		tx_arg->ul_ru_rate_u1 = endian_convert32(tx_arg->ul_ru_rate_u1);
		tx_arg->ul_ru_alo_idx_u1 = endian_convert32(tx_arg->ul_ru_alo_idx_u1);
		tx_arg->macid_u2 = endian_convert32(tx_arg->macid_u2);
		tx_arg->ac_type_u2 = endian_convert32(tx_arg->ac_type_u2);
		tx_arg->mu_sta_pos_u2 = endian_convert32(tx_arg->mu_sta_pos_u2);
		tx_arg->dl_rate_idx_u2 = endian_convert32(tx_arg->dl_rate_idx_u2);
		tx_arg->dl_dcm_en_u2 = endian_convert32(tx_arg->dl_dcm_en_u2);
		tx_arg->ru_alo_idx_u2 = endian_convert32(tx_arg->ru_alo_idx_u2);
		tx_arg->pwr_boost_u2 = endian_convert32(tx_arg->pwr_boost_u2);
		tx_arg->agg_bmp_alo_u2 = endian_convert32(tx_arg->agg_bmp_alo_u2);
		tx_arg->ampdu_max_txnum_u2 = endian_convert32(tx_arg->ampdu_max_txnum_u2);
		tx_arg->user_define_u2 = endian_convert32(tx_arg->user_define_u2);
		tx_arg->user_define_ext_u2 = endian_convert32(tx_arg->user_define_ext_u2);
		tx_arg->ul_addr_idx_u2 = endian_convert32(tx_arg->ul_addr_idx_u2);
		tx_arg->ul_dcm_u2 = endian_convert32(tx_arg->ul_dcm_u2);
		tx_arg->ul_fec_cod_u2 = endian_convert32(tx_arg->ul_fec_cod_u2);
		tx_arg->ul_ru_rate_u2 = endian_convert32(tx_arg->ul_ru_rate_u2);
		tx_arg->ul_ru_alo_idx_u2 = endian_convert32(tx_arg->ul_ru_alo_idx_u2);
		tx_arg->macid_u3 = endian_convert32(tx_arg->macid_u3);
		tx_arg->ac_type_u3 = endian_convert32(tx_arg->ac_type_u3);
		tx_arg->mu_sta_pos_u3 = endian_convert32(tx_arg->mu_sta_pos_u3);
		tx_arg->dl_rate_idx_u3 = endian_convert32(tx_arg->dl_rate_idx_u3);
		tx_arg->dl_dcm_en_u3 = endian_convert32(tx_arg->dl_dcm_en_u3);
		tx_arg->ru_alo_idx_u3 = endian_convert32(tx_arg->ru_alo_idx_u3);
		tx_arg->pwr_boost_u3 = endian_convert32(tx_arg->pwr_boost_u3);
		tx_arg->agg_bmp_alo_u3 = endian_convert32(tx_arg->agg_bmp_alo_u3);
		tx_arg->ampdu_max_txnum_u3 = endian_convert32(tx_arg->ampdu_max_txnum_u3);
		tx_arg->user_define_u3 = endian_convert32(tx_arg->user_define_u3);
		tx_arg->user_define_ext_u3 = endian_convert32(tx_arg->user_define_ext_u3);
		tx_arg->ul_addr_idx_u3 = endian_convert32(tx_arg->ul_addr_idx_u3);
		tx_arg->ul_dcm_u3 = endian_convert32(tx_arg->ul_dcm_u3);
		tx_arg->ul_fec_cod_u3 = endian_convert32(tx_arg->ul_fec_cod_u3);
		tx_arg->ul_ru_rate_u3 = endian_convert32(tx_arg->ul_ru_rate_u3);
		tx_arg->ul_ru_alo_idx_u3 = endian_convert32(tx_arg->ul_ru_alo_idx_u3);
		tx_arg->pkt_id_0 = endian_convert32(tx_arg->pkt_id_0);
		tx_arg->valid_0 = endian_convert32(tx_arg->valid_0);
		tx_arg->ul_user_num_0 = endian_convert32(tx_arg->ul_user_num_0);
		tx_arg->pkt_id_1 = endian_convert32(tx_arg->pkt_id_1);
		tx_arg->valid_1 = endian_convert32(tx_arg->valid_1);
		tx_arg->ul_user_num_1 = endian_convert32(tx_arg->ul_user_num_1);
		tx_arg->pkt_id_2 = endian_convert32(tx_arg->pkt_id_2);
		tx_arg->valid_2 = endian_convert32(tx_arg->valid_2);
		tx_arg->ul_user_num_2 = endian_convert32(tx_arg->ul_user_num_2);
		tx_arg->pkt_id_3 = endian_convert32(tx_arg->pkt_id_3);
		tx_arg->valid_3 = endian_convert32(tx_arg->valid_3);
		tx_arg->ul_user_num_3 = endian_convert32(tx_arg->ul_user_num_3);
		tx_arg->pkt_id_4 = endian_convert32(tx_arg->pkt_id_4);
		tx_arg->valid_4 = endian_convert32(tx_arg->valid_4);
		tx_arg->ul_user_num_4 = endian_convert32(tx_arg->ul_user_num_4);
		tx_arg->pkt_id_5 = endian_convert32(tx_arg->pkt_id_5);
		tx_arg->valid_5 = endian_convert32(tx_arg->valid_5);
		tx_arg->ul_user_num_5 = endian_convert32(tx_arg->ul_user_num_5);
		break;		
	case MP_CLASS_RX:
		rx_arg->rx_ok = endian_convert32(rx_arg->rx_ok);
		rx_arg->rx_err = endian_convert32(rx_arg->rx_err);
		rx_arg->phy0_user0_rxevm = endian_convert32(rx_arg->phy0_user0_rxevm);
		rx_arg->phy0_user1_rxevm = endian_convert32(rx_arg->phy0_user1_rxevm);
		rx_arg->phy0_user2_rxevm = endian_convert32(rx_arg->phy0_user2_rxevm);
		rx_arg->phy0_user3_rxevm = endian_convert32(rx_arg->phy0_user3_rxevm);
		rx_arg->phy1_user0_rxevm = endian_convert32(rx_arg->phy1_user0_rxevm);
		rx_arg->phy1_user1_rxevm = endian_convert32(rx_arg->phy1_user1_rxevm);
		rx_arg->phy1_user2_rxevm = endian_convert32(rx_arg->phy1_user2_rxevm);
		rx_arg->phy1_user3_rxevm = endian_convert32(rx_arg->phy1_user3_rxevm);
		rx_arg->rpl_rssi = endian_convert16(rx_arg->rpl_rssi);
		rx_arg->mp_fltr_rx_ok_cnt = endian_convert32(rx_arg->mp_fltr_rx_ok_cnt);
		rx_arg->mp_fltr_rx_err_cnt = endian_convert32(rx_arg->mp_fltr_rx_err_cnt);
		break;
	case MP_CLASS_EFUSE:
		efuse_arg->io_offset = endian_convert16(efuse_arg->io_offset);
		efuse_arg->io_value = endian_convert32(efuse_arg->io_value);
		efuse_arg->buf_len = endian_convert16(efuse_arg->buf_len);
		break;
	case MP_CLASS_REG:
		reg_arg->io_offset = endian_convert32(reg_arg->io_offset);
		reg_arg->io_value = endian_convert32(reg_arg->io_value);
		break;
	case MP_CLASS_TXPWR:
		txpwr_arg->txpwr = endian_convert16(txpwr_arg->txpwr);
		txpwr_arg->txpwr_index = endian_convert16(txpwr_arg->txpwr_index);
		txpwr_arg->tssi = endian_convert32(txpwr_arg->tssi);
		txpwr_arg->rate = endian_convert16(txpwr_arg->rate);
		txpwr_arg->table_item = endian_convert16(txpwr_arg->table_item);
		txpwr_arg->txpwr_ref = endian_convert16(txpwr_arg->txpwr_ref);
		txpwr_arg->tssi_de_offset = endian_convert32(txpwr_arg->tssi_de_offset);
		txpwr_arg->dbm = endian_convert32(txpwr_arg->dbm);
		txpwr_arg->pout = endian_convert32(txpwr_arg->pout);
		txpwr_arg->online_tssi_de = endian_convert32(txpwr_arg->online_tssi_de);
		break;
	case MP_CLASS_CAL:
		cal_arg->io_value = endian_convert16(cal_arg->io_value);
		cal_arg->xdbm = endian_convert32(cal_arg->xdbm);
		cal_arg->avg = endian_convert32(cal_arg->avg);
		cal_arg->fft = endian_convert32(cal_arg->fft);
		cal_arg->point = endian_convert32(cal_arg->point);
		cal_arg->upoint = endian_convert32(cal_arg->upoint);
		cal_arg->start_point = endian_convert32(cal_arg->start_point);
		cal_arg->stop_point = endian_convert32(cal_arg->stop_point);
		cal_arg->buf = endian_convert32(cal_arg->buf);
		for (idx = 0; idx < 450; idx++)
			cal_arg->outbuf[idx] = endian_convert32(cal_arg->outbuf[idx]);
		break;
		break;
	default:
		break;
	}
}
#else
void endian_convert(struct argshell *args, int direction)
{
}
#endif

int main()
{
	struct infoshell mp_buffer; // mp_context buffer
	FILE *fp;
    int round = 1;
    int ret;
    int err;

    char ifname[6];
    char copy_input[2047];	// for linux driver ioctl
    char cmd_buf[64];

    // init UDP socket and configure
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //init server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CONNECT_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    //init client address
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CONNECT_PORT);
    client_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    memset(&(client_addr.sin_zero), '\0', 8);

    // bind server socket
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
	printf("socket bind failed. close socket. line: %d\n", __LINE__);
	close(sockfd);
    }

    addr_len = sizeof(struct sockaddr);
    /*TODO: send phl_test_init*/


    // Welcome message of MP daemon
    printf("AX MP Deamon Starts\n");

    // daemon keep listening for MP UI tool
    while (true) {

		//printf("====================================================================\n");
		//printf("[Round %d] \n", round);
		//memset(mp_buffer, 0, sizeof(mp_buffer));
		//printf("\n>>>>> receiving from client...\n");

		ret = recvfrom(sockfd, (void *)&mp_buffer, sizeof(mp_buffer), 0, (struct sockaddr*)&client_addr, &addr_len);

		if (ret == -1) {
		    printf("socket error, recv failed. line: %d\n", __LINE__);
		    close(sockfd);
		    exit(1);
		}

		//printf("[UDP server] recv: %d bytes\n", ret);

	//============================================================================================
#if CHECK_LEN
		/*put debug print here*/
		printf("[UDP server] argshell->len = %d\n", mp_buffer.arg_ctx.len);
		printf("[UDP server] sizeof(argshell) = %d\n", sizeof(struct argshell));
#endif
	//============================================================================================
		/*parse infoshell content*/
		strncpy(ifname, mp_buffer.device_id, 6);

		/* flash operations that write back to flash HW */
		if (rtw_check_flash_op(&mp_buffer.arg_ctx)){

			struct mp_flash_arg *get_arg= (struct mp_flash_arg*) mp_buffer.arg_ctx.buf;
			//printf("[UDP server] %s\n", get_arg->flash_cmd);

			switch (get_arg->cmd){

				case MP_FLASH_CMD_READ:
					snprintf(cmd_buf, sizeof(cmd_buf), "%s > /tmp/MP.txt", get_arg->flash_cmd);
					system(cmd_buf);

					if ((fp = fopen("/tmp/MP.txt", "r")) == NULL){
						printf("[UDP server] cannot open flash calibration temp file!!\n");
						get_arg->status = false;
					}

					else {
						//printf("[UDP server] load flash from MP.txt!!\n");
						fgets(get_arg->table, sizeof(get_arg->table), fp);
						get_arg->status = true;
					}

					break;

				case MP_FLASH_CMD_WRITE:
					system(get_arg->flash_cmd);
					get_arg->status = true;
					sync_shadowmap(get_arg, ifname); /*write flash data to shadowmap*/
					break;

				default:
					printf("[UDP server] invalid flash calibration cmd!!\n");
					get_arg->status = false;
					break;
			}

			/* send flash calibration result to DUT*/
			ret = sendto(sockfd, &mp_buffer, sizeof(struct infoshell), 0, (const struct sockaddr*)&client_addr, addr_len);

		}

		else{

			/*fix endian issue*/
			endian_convert(&mp_buffer.arg_ctx, TO_DRIVER);

			/*run cmd*/
			memcpy(copy_input, phl_set_io_cmd, strlen(phl_set_io_cmd));
			memcpy(copy_input + strlen(phl_set_io_cmd), (void*)&mp_buffer.arg_ctx, sizeof(struct argshell));

			err = rtw_phl_ioctl(sockfd, ifname, copy_input, sizeof(struct argshell)+strlen(phl_set_io_cmd)+1);

			if (err < 0){
				printf("interface does not accept private ioctl\n");
			}

			memcpy(&mp_buffer.arg_ctx, (struct argshell *)&copy_input, sizeof(struct argshell));

			/*fix endian issue*/
			endian_convert(&mp_buffer.arg_ctx, FROM_DRIVER);

			ret = sendto(sockfd, &mp_buffer, sizeof(struct infoshell), 0, (const struct sockaddr*)&client_addr, addr_len);
			// printf("[UDP server] send: %d bytes\n", ret);

		}

		memset(&mp_buffer, 0, sizeof(struct infoshell));
		round++;
	}

	printf("socket closed. line: %d\n", __LINE__);

	/*TODO: send phl_test_deinit*/

	close(sockfd);
	return 0;
}
