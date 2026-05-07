#!/bin/sh

#wlan0
val=`grep HW_XCAP_WLAN0_AX /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set xcap=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_A_WLAN0 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set thermalA=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_B_WLAN0 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set thermalB=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_C_WLAN0 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set thermalC=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_D_WLAN0 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set thermalD=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_cck_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_cck_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep -m 1 HW_2G_CCK_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_cck_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_cck_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_bw40_1s_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_bw40_1s_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_bw40_1s_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 2G_bw40_1s_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 5G_bw40_1s_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 5G_bw40_1s_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 5G_bw40_1s_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan0 flash_set 5G_bw40_1s_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_2G_CCK_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set 2G_rx_gain_cck=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_2G_OFDM_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set 2G_rx_gain_ofdm=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GL_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set 5G_rx_gain_low=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GM_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set 5G_rx_gain_mid=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GH_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set 5G_rx_gain_high=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_CUSTOM_PARA_PATH /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set para_path=$val"
echo $iwcmd && eval $iwcmd

#wlan1
val=`grep HW_XCAP_WLAN1_AX /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set xcap=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_A_WLAN1 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set thermalA=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_B_WLAN1 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set thermalB=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_C_WLAN1 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set thermalC=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_THERMAL_D_WLAN1 /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan0 flash_set thermalD=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_cck_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_cck_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep -m 1 HW_2G_CCK_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_cck_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_CCK_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_cck_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_bw40_1s_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_bw40_1s_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_bw40_1s_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_2G_BW40_1S_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 2G_bw40_1s_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_A /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 5G_bw40_1s_tssi_A=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_B /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 5G_bw40_1s_tssi_B=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_C /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 5G_bw40_1s_tssi_C=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_5G_BW40_1S_TSSI_D /config/config_hs.xml | awk -F '"' '{print $4}' | sed 's/,//g'`
iwcmd="iwpriv wlan1 flash_set 5G_bw40_1s_tssi_D=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_2G_CCK_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set 2G_rx_gain_cck=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_2G_OFDM_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set 2G_rx_gain_ofdm=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GL_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set 5G_rx_gain_low=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GM_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set 5G_rx_gain_mid=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_RX_GAINGAP_5GH_AB /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set 5G_rx_gain_high=$val"
echo $iwcmd && eval $iwcmd

val=`grep HW_CUSTOM_PARA_PATH /config/config_hs.xml | awk -F '"' '{print $4}'`
iwcmd="iwpriv wlan1 flash_set para_path=$val"
echo $iwcmd && eval $iwcmd

