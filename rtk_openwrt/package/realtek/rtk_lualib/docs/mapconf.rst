
SCHEMA of configuration object of MAP
=====================================

The entries which are not assigned are from configuration files of multi-ap daemons, which does not have a proper documentation. Please find the developer of those daemons for clarification.

{
  role = rtkmap.map.controller,
  logotest = rtkmap.map.logotest,
  device_name = rtkmap.map.device_name,
  log_level = rtkmap.map.log_level,
  map_profile = rtkmap.map.profile,
  MIB_MAP_CONFIGURED_BAND = rtkmap.map2.configured_band,
  MIB_HW_COUNTRY_STR = rtkmap.map2.hw_country_str,
  customize_features = rtkmap.map2.customize_features,
  MIB_HW_REG_DOMAIN = rtkmap.map2.hw_reg_domain,
  bh_band_idx = rtkmap.map2.bh_band_idx,
  bridge_name = network.lan.device,
  bss_list_update,
  vap_prefix,
  common_netlink,
  stp_monitor_enable,
  loop_detect_enable,
  beacon_request_ch,
  backhaul_bss_index,
  radios = {
    radio1 = {
      MIB_REPEATER_SSID1,
      MIB_WLAN_CHAN_NUM,
      htmode = wireless.radio1.htmode,
      MIB_WLAN_CHANNEL_WIDTH = f(wireless.radio1.htmode),
      _BSS_DATAS = [
        {
          ssid = wireless.default_radio00.ssid,
          ifname = wireless.default_radio00.ifname,
          network_key = wireless.default_radio00.key,
          is_enabled = f(wireless.default_radio00.disabled),
          need_configure,
          network_type = wireless.default_radio00.network_type,
          authentication_type = f(wireless.default_radio00.encryption),
          bss_index,
          signed_connector
        },
        ...
      ]
    },
    radio0 = {
      ...
    },
    radio2 = {
      ...
    }
  }
}
