local uci = require("luci.model.uci").cursor()
local lstr = require("rtk.lstr")

function table.length(T)
  local count = 0
  for _ in pairs(T) do
    count = count + 1
  end
  return count
end

function table.hasallatt(T, ...)
  local args = { ... }
  for _, v in ipairs(args) do
    if not T[v] then
      -- print(v)
      return false
    end
  end
  return true
end

function table.filter(t, filterIter)
  local out = {}

  for i, v in ipairs(t) do
    if filterIter(v, i, t) then table.insert(out, v) end
  end

  return out
end

function table.filterk(t, filterIter)
  local out = {}

  for k, v in pairs(t) do
    if filterIter(v, k, t) then out[k] = v end
  end

  return out
end

function table.copy(t, depth)
  if depth <= 0 then return t end
  local t2 = {}
  for k,v in pairs(t) do
    t2[k] = type(v) == "table" and table.copy(v, depth - 1) or v
  end
  return t2
end

local function _fil(t, att, val)
  if not t[att] and val then
    t[att] = val
  end
end

MAX_COUNT = 1000
local function reduce_t1(arglst, mappers)
  if not arglst then return {} end
  local function _def_mapper(val)
    return val
  end
  mappers = mappers or {}
  local c = MAX_COUNT
  for _, v in ipairs(arglst) do
    if v.val then
      local lst = lstr.ssplit(v.val)
      v["_" .. v.k] = lst
      c = math.min(c, #lst)
    end
  end
  if c == MAX_COUNT then return {} end
  local ans = {}
  for i = 1, c do
    local x = {}
    for _, v in ipairs(arglst) do
      if v.val then
        local f = mappers[v.k] or _def_mapper
        x[v.k] = f(v["_" .. v.k][i])
      end
    end
    table.insert(ans, x)
  end

  return ans
end

local function _red_t1_h1(ds, names, nkm)
  local ans = {}
  nkm = nkm or {}
  for _, v in ipairs(names) do
    local k = nkm[v] or v
    table.insert(ans, {
      k = k,
      val = ds[v]
    })
  end
  return ans
end

local function aggre(items, names, mappers)
  local function _def_mapper(val)
    return val
  end
  local ans = {}
  for _, v in ipairs(names) do
    if #items == 0 then
      ans[v] = ""
    else
      local agg = ""
      for _, v2 in ipairs(items) do
        local vv = v2[v] or ""
        local f = mappers[v] or _def_mapper
        agg = agg .. f(vv) .. ","
      end
      ans[v] = agg:sub(1, #agg - 1)
    end
  end

  return ans
end

local function tappend(t1, t2, names, nkm, nfm)
  local function _def_mapper(val)
    return val
  end

  nkm = nkm or {}
  nfm = nfm or {}
  for _, v in ipairs(names) do
    local k = nkm[v] or v
    local f = nfm[v] or _def_mapper
    if t2[v] and not t1[k] then
      t1[k] = f(t2[v])
    end
  end
end

local function trekey(t, names, nkm, nfm)
  local ans = {}
  tappend(ans, t, names, nkm, nfm)
  return ans
end

local ftds1 = require("rtk.ini")
local bitop = require("rtk.bitop")
local base64 = require("rtk.lbase64")

local function fread(path)
  local f, s
  f = assert(io.open(path, "r"))
  s = f:read("*all")
  f:close()
  return s
end

function fexist(path)
  local f = io.open(path, "r")
  return f ~= nil and io.close(f)
end

----------------
local __2g_on_wlan0 = 1

-- wireless.default_radio(0)0
MAP_CONFIG_2G = 1
MAP_CONFIG_5GL = 0
MAP_CONFIG_5GH = 2

local m_band_radioname = {
  [MAP_CONFIG_2G] = "phy1",
  [MAP_CONFIG_5GL] = "phy0",
  [MAP_CONFIG_5GH] = "phy2"
}

-- wireless.default_radio00.ifname = wlan(1)
IFIDX_2G = 0
IFIDX_5GL = 1
IFIDX_5GH = 2
if __2g_on_wlan0 == 0 then
  IFIDX_2G = 1
  IFIDX_5GL = 0
end

local m_ifidx_radioname = {
  [IFIDX_2G] = "phy1",
  [IFIDX_5GL] = "phy0",
  [IFIDX_5GH] = "phy2"
}

local m_ifidx_band = {
  [IFIDX_2G] = MAP_CONFIG_2G,
  [IFIDX_5GL] = MAP_CONFIG_5GL,
  [IFIDX_5GH] = MAP_CONFIG_5GH
}

BAND_ETH = -1
BAND_2G = 0
BAND_5GL = 1
BAND_5GH = 2
local m_bhband_bh_band_idx = {
  [BAND_ETH] = -1,
  [BAND_2G] = IFIDX_2G,
  [BAND_5GL] = IFIDX_5GL
}

local m_band2_radioname = {
  [BAND_2G] = "phy1",
  [BAND_5GL] = "phy0",
  [BAND_5GH] = "phy2"
}

local function get_bss_name(band, i)
  return "default_radio" .. band .. i
end

MAP_ROLE_DISABLED = 0
MAP_ROLE_CONTROLLER = 1
MAP_ROLE_AGENT = 2

MAP_NETWORK_TEARDOWN = 16
MAP_NETWORK_FRONTHAUL_AP = 32
MAP_NETWORK_BACKHAUL_AP = 64
MAP_NETWORK_FRONTBACK_AP = bitop._or(MAP_NETWORK_FRONTHAUL_AP, MAP_NETWORK_BACKHAUL_AP)
MAP_NETWORK_BACKHAUL_STA = 128

AUTH_DISABLED = 0x01
AUTH_WPA = 0x02
AUTH_WEP = 0x04
AUTH_WPA2 = 0x20
AUTH_WPA3 = 0x40
AUTH_DPP = 0x80
AUTH_WPA_WPA2 = bitop._or(AUTH_WPA, AUTH_WPA2)
AUTH_WPA2_WPA3 = bitop._or(AUTH_WPA2, AUTH_WPA3)
AUTH_WPA2_WPA3_DPP = bitop._or(AUTH_WPA2_WPA3, AUTH_DPP)

WIFI_SEC_NONE = 0
WIFI_SEC_WEP = 1
WIFI_SEC_WPA = 2
WIFI_SEC_WPA2 = 4
WIFI_SEC_WAPI = 8
WIFI_SEC_WPA3 = 16
WIFI_SEC_WPA_WPA2 = bitop._or(WIFI_SEC_WPA, WIFI_SEC_WPA2)
WIFI_SEC_WPA2_WPA3 = bitop._or(WIFI_SEC_WPA2, WIFI_SEC_WPA3)

local m_ucienc_auth = {
  psk = AUTH_WPA,
  psk2 = AUTH_WPA2,
  sae = AUTH_WPA3,
  none = AUTH_DISABLED,
  ["psk-mixed"] = AUTH_WPA_WPA2,
  ["sae-mixed"] = AUTH_WPA2_WPA3
}

local m_auth_ucienc = {}
for k, v in pairs(m_ucienc_auth) do
  m_auth_ucienc[tostring(v)] = k
end

CONFIGURED_BAND_2G = 1
CONFIGURED_BAND_5GL = 2
CONFIGURED_BAND_5GH = 4

BAND_WIDTH_20MHZ = 0
BAND_WIDTH_40MHZ = 1
BAND_WIDTH_80MHZ = 2
BAND_WIDTH_160MHZ = 3
BAND_WIDTH_80_80MHZ = 4

local m_htmode_chw = {
  ["HT20"] = BAND_WIDTH_20MHZ,
  ["HT40"] = BAND_WIDTH_40MHZ,
  ["HT40-"] = BAND_WIDTH_40MHZ,
  ["HE20"] = BAND_WIDTH_20MHZ,
  ["HE40"] = BAND_WIDTH_40MHZ,
  ["HE80"] = BAND_WIDTH_80MHZ,
  ["HE160"] = BAND_WIDTH_160MHZ,
  ["VHT20"] = BAND_WIDTH_20MHZ,
  ["VHT40"] = BAND_WIDTH_40MHZ,
  ["VHT80"] = BAND_WIDTH_80MHZ,
  ["VHT160"] = BAND_WIDTH_160MHZ
}

local m_chw_htmode = {
  [BAND_WIDTH_20MHZ] = "20",
  [BAND_WIDTH_40MHZ] = "40",
  [BAND_WIDTH_80MHZ] = "80",
  [BAND_WIDTH_160MHZ] = "160",
  [BAND_WIDTH_80_80MHZ] = "80+80"
}

local function _conv_auth_enc(auth)
  if auth == AUTH_DISABLED then
    return WIFI_SEC_NONE
  elseif auth == AUTH_WPA then
    return WIFI_SEC_WPA
  elseif auth == AUTH_WPA2 then
    return WIFI_SEC_WPA2
  elseif auth == AUTH_WPA3 then
    return WIFI_SEC_WPA3
  elseif auth == AUTH_WPA_WPA2 then
    return WIFI_SEC_WPA_WPA2
  elseif auth == AUTH_WPA2_WPA3 or auth == AUTH_WPA2_WPA3_DPP then
    return WIFI_SEC_WPA2_WPA3
  else
    return WIFI_SEC_WPA2
  end
end

local function _adjust_auth_dpp(auth, ifname)
  local auth_type = auth
  local akm_path = "/tmp/dpp_akm_"..ifname
  if fexist(akm_path) then
    local akm = fread(akm_path)
    if string.find(akm, "dpp") then
      auth_type = bitop._setBit(auth_type, AUTH_DPP)
    end
  end
  return auth_type
end

local function _adjust_enc_dpp(auth)
  local auth_type
  if bitop._isBitSet(auth, AUTH_DPP) then
    auth_type = bitop._clearBit(auth, AUTH_DPP)
  else
    auth_type = auth
  end
  return m_auth_ucienc[tostring(auth_type)]
end

local function _get_ieee80211w(auth, network_type)
  local ieee80211w
  if bitop._isBitSet(auth, AUTH_WPA3) then
    if bitop._isBitSet(auth, AUTH_WPA2) then
      ieee80211w = 1
    else
      ieee80211w = 2
    end
  end
  local logotest = uci:get("rtkmap", "map", "logotest")
  if tonumber(logotest) == 1 and not ieee80211w then
    if bitop._isBitSet(network_type, MAP_NETWORK_FRONTHAUL_AP) or bitop._isBitSet(network_type, MAP_NETWORK_BACKHAUL_AP) then
      ieee80211w = 1
    end
  end
  return ieee80211w
end

-- enum WIFI_REG_DOMAIN
DOMAIN_FCC = 1
DOMAIN_IC = 2
DOMAIN_ETSI = 3
DOMAIN_SPAIN = 4
DOMAIN_FRANCE = 5
DOMAIN_MKK = 6
DOMAIN_ISRAEL = 7
DOMAIN_MKK1 = 8
DOMAIN_MKK2 = 9
DOMAIN_MKK3 = 10
DOMAIN_NCC = 11
DOMAIN_RUSSIAN = 12
DOMAIN_CN = 13
DOMAIN_GLOBAL = 14
DOMAIN_WORLD_WIDE = 15
DOMAIN_TEST = 16
DOMAIN_5M10M = 17
DOMAIN_SG = 18
DOMAIN_KR = 19
DOMAIN_MAX = 20

local m_country_regdomain = {
  ["US"] = DOMAIN_FCC,
  ["ES"] = DOMAIN_SPAIN,
  ["FR"] = DOMAIN_FRANCE,
  ["IL"] = DOMAIN_ISRAEL,
  ["RU"] = DOMAIN_RUSSIAN,
  ["CN"] = DOMAIN_CN,
  ["SG"] = DOMAIN_SG,
  ["KR"] = DOMAIN_KR,
  ["PT"] = DOMAIN_ETSI,
  ["DE"] = DOMAIN_ETSI,
  ["PL"] = DOMAIN_ETSI,
  ["JP"] = DOMAIN_MKK,
}

VID_INTERVAL = 5

local function get_regdomain(country)
  local x = m_country_regdomain[country]
  if not x then return DOMAIN_WORLD_WIDE end
  return x
end

local map1 = { "max_resend_time", "alme_port_number", "rssi_weightage", "path_weightage", "cu_weightage",
  "roam_score_difference", "min_evaluation_interval", "min_roam_interval", "max_num_device_allowed",
  "throughput_threshold", "pushbutton_gpio_monitoring", "max_bss_per_radio", "map_supported_service",
  "rssi_5g_threshold", "rssi_24g_threshold", "max_renew_resend_time", "mcs_40m_table", "mcs_20m_table",
  "max_sta_number", "max_log_size", "device_name", "radio_name_5gh", "radio_name_5gl", "radio_name_24g",
  "bss_list_update", "vap_prefix", "common_netlink", "stp_monitor_enable", "loop_detect_enable",
  "beacon_request_ch", "backhaul_bss_index", "bridge_name" }

local function execute(cmd)
  print(cmd)
  os.execute(cmd)
end

local function get_backhaul_ssid_key(conf_bsss)
  local backhaul_ssid, backhaul_key
  for _, v in ipairs(conf_bsss) do
    if v.network_type and bitop._and(v.network_type, MAP_NETWORK_BACKHAUL_AP) ~= 0 then
      backhaul_ssid = v.ssid
      backhaul_key = v.network_key
      break
    end
  end
  if not (backhaul_ssid and backhaul_key) then
    backhaul_ssid, backhaul_key = nil, nil
  end
  return backhaul_ssid, backhaul_key
end

local function get_m_ssid_vid(conf)
  local function get_m_ssid_vlanname(conf)
    local d = {}
    for i,v in ipairs(conf.vlans) do
      for _,s in ipairs(v.ssids) do
        d[s] = v.name
      end
    end
    return d
  end

  local pvn = conf.primary_vlan_name or ""
  local d = get_m_ssid_vlanname(conf)
  local curr_vid = conf.primary_vid
  local vids = { [pvn] = curr_vid }
  curr_vid = curr_vid + VID_INTERVAL
  for k,v in pairs(d) do
    if not vids[v] then
      vids[v] = curr_vid
      curr_vid = curr_vid + VID_INTERVAL
    end
  end

  local m_ssid_vid = {}
  for k,v in pairs(d) do
    m_ssid_vid[k] = vids[v]
  end
  return m_ssid_vid
end

local function get_vlans(conf)
  local m = get_m_ssid_vid(conf)
  local ans = {}
  for k,v in pairs(m) do
    table.insert(ans, {
      ssid = k,
      vid = v
    })
  end
  return ans
end
----
local mapconf = {}

mapconf.m_band_radioname = m_band_radioname
mapconf.m_ifidx_radioname = m_ifidx_radioname
mapconf.m_band2_radioname = m_band2_radioname
mapconf.m_ifidx_band = m_ifidx_band
mapconf.m_chw_htmode = m_chw_htmode

function mapconf.get_mapstate_2()
  uci:load("rtkmap")
  local ans = uci:get("rtkmap", "map", "controller")
  if not ans then
    return "0"
  end
  return ans
end

function mapconf.get_mapstate()
  return "1"
end

function mapconf.fill(conf, s)
  local a = ftds1.parse(s)
  tappend(conf, a["global"], map1)
end

function mapconf.fill_fromsetmib(conf, s)
  local function _set_radiodata_fromsetmib(conf, ds_sec)
    local ds = ds_sec
    _fil(conf, "MIB_WLAN_CHAN_NUM", ds["radio_channel"])
    _fil(conf, "MIB_WLAN_CHANNEL_WIDTH", ds["channel_bandwidth"])
    if not conf["_BSS_DATAS"] then
      conf["_BSS_DATAS"] = reduce_t1(_red_t1_h1(ds,
        { "ssid", "network_key", "is_enabled", "need_configure", "bss_type", "encrypt_type", "authentication_type",
          "bss_index", "signed_connector" }, {
          bss_type = "network_type"
        }), {
        is_enabled = tonumber,
        network_type = tonumber,
        need_configure = tonumber,
        ssid = base64.decode,
        network_key = base64.decode
      })
    end
  end

  local a = ftds1.parse(s)
  _fil(conf, "MIB_MAP_CONFIGURED_BAND", a["setmib_global_data"]["configured_band"])
  _fil(conf, "MIB_HW_REG_DOMAIN", a["setmib_global_data"]["hw_reg_domain"])
  _fil(conf, "radios", {})

  if a["dpp"] then
    _fil(conf, "dpp", {})
    local conf_dpp = conf["dpp"]
    if a["dpp"]["signed_connector_1905"] then
      _fil(conf_dpp, "signed_connector_1905", a["dpp"]["signed_connector_1905"])
    end
    if a["dpp"]["netaccess_key"] then
      _fil(conf_dpp, "netaccess_key", a["dpp"]["netaccess_key"])
    end
    if a["dpp"]["csign_key"] then
      _fil(conf_dpp, "csign_key", a["dpp"]["csign_key"])
    end
    if a["dpp"]["pp_key"] then
      _fil(conf_dpp, "pp_key", a["dpp"]["pp_key"])
    end
  end

  local conf_radio = conf["radios"]
  if a["2.4g_setmib_config_data"] then
    _fil(conf_radio, MAP_CONFIG_2G, {})
    _set_radiodata_fromsetmib(conf_radio[MAP_CONFIG_2G], a["2.4g_setmib_config_data"])
  end
  if a["5gl_setmib_config_data"] then
    _fil(conf_radio, MAP_CONFIG_5GL, {})
    _set_radiodata_fromsetmib(conf_radio[MAP_CONFIG_5GL], a["5gl_setmib_config_data"])
  end
  if a["5gh_setmib_config_data"] then
    _fil(conf_radio, MAP_CONFIG_5GH, {})
    _set_radiodata_fromsetmib(conf_radio[MAP_CONFIG_5GH], a["5gh_setmib_config_data"])
  end
end

local function get_default_network_type(w_bss)
  --if w_bss.disabled == "1" then return MAP_NETWORK_TEARDOWN end
  return MAP_NETWORK_FRONTHAUL_AP
end

function mapconf.fill_fromuci(conf)
  uci:load("rtkmap")
  local rtkmap = uci:get_all("rtkmap")
  _fil(conf, "role", tonumber(rtkmap.map.controller))
  _fil(conf, "logotest", tonumber(rtkmap.map.logotest))
  _fil(conf, "device_name", rtkmap.map.device_name)
  _fil(conf, "log_level", tonumber(rtkmap.map.log_level) or 0)
  _fil(conf, "map_profile", rtkmap.map.profile)
  _fil(conf, "MIB_MAP_CONFIGURED_BAND", rtkmap.map.configured_band)
  _fil(conf, "customize_features", tonumber(rtkmap.map.customize_features) or 0)
  _fil(conf, "bh_band_idx", m_bhband_bh_band_idx[tonumber(rtkmap.map.bhband)] or 0)
  _fil(conf, "backhaul_bss_index", tonumber(rtkmap.map.backhaul_bss_index) or 1)
  _fil(conf, "__internal_testing", rtkmap.map.__internal_testing)
  uci:load("network")
  _fil(conf, "bridge_name", uci:get("network", "lan", "device"))
  _fil(conf, "proto", uci:get("network", "lan", "proto"))

  _fil(conf, "vap_prefix", "vap")
  --_fil(conf, "bss_list_update", 1)
  --_fil(conf, "common_netlink", 1)
  --_fil(conf, "stp_monitor_enable", 1)
  --_fil(conf, "loop_detect_enable", 1)
  --_fil(conf, "beacon_request_ch", 1)
end

BSS_CONFIG_PATH="/tmp/bss.config"

function mapconf.fill_vlan(conf)
  uci:load("rtkmap")
  local rtkmap = uci:get_all("rtkmap")

  if fexist(BSS_CONFIG_PATH) then
    -- use case: logo test
    local a = ftds1.parse(fread(BSS_CONFIG_PATH))
    if a["vlan"] then
      _fil(conf, "primary_vid", tonumber(a["vlan"]["primary_vid"]))
      _fil(conf, "default_pcp", tonumber(a["vlan"]["default_pcp"]))
    end
  else
    _fil(conf, "enable_vlan", tonumber(rtkmap.map.enable_vlan))
    _fil(conf, "primary_vid", tonumber(rtkmap.map.primary_vid))
    _fil(conf, "primary_vlan_name", rtkmap.map.primary_vlan_name)

    conf.vlans = {}
    uci:foreach("rtkmap", "vlan", function(x)
      --[[
        x = {
          name="name_of_vlan"
          ssids=[ "ssid1", "ssid2" ]
        }
      ]]--
      if x.name then
        table.insert(conf.vlans, x)
      end
    end)
  end
end

function mapconf.fill_radio(conf)
  local function _set_bssdata_fromfile(conf, ds)
    local ssid_raw = lstr.ssplit(ds["ssid_raw"])
    local network_key_raw = lstr.ssplit(ds["network_key_raw"])
    local authentication_type = lstr.ssplit(ds["authentication_type"])
    local encryption_type = lstr.ssplit(ds["encryption_type"])
    local network_type = lstr.ssplit(ds["network_type"])
    local al_id = lstr.ssplit(ds["al_id"])
    local vid = lstr.ssplit(ds["vid"])
    local bsss = {}
    for i=1,#ssid_raw do
      local bss = {}
      _fil(bss, "is_enabled", 1)
      _fil(bss, "ssid_raw", ssid_raw[i])
      _fil(bss, "network_key_raw", network_key_raw[i])
      _fil(bss, "authentication_type", authentication_type[i])
      _fil(bss, "encryption_type", encryption_type[i])
      _fil(bss, "network_type", network_type[i])
      _fil(bss, "al_id", al_id[i])
      _fil(bss, "vid", vid[i])
      table.insert(bsss, bss)
    end
    conf["_BSS_DATAS"] = bsss
  end
  local function _set_bssdata_fromuci(conf_bss, w_bss)
    _fil(conf_bss, "ssid", w_bss.ssid)
    _fil(conf_bss, "ifname", w_bss.ifname)
    _fil(conf_bss, "network_key", w_bss.key)
    _fil(conf_bss, "is_enabled", w_bss.disabled == "1" and 0 or 1)
    _fil(conf_bss, "mode", w_bss.mode)
    _fil(conf_bss, "network_type", tonumber(w_bss.map_bss_type))
    _fil(conf_bss, "authentication_type", _adjust_auth_dpp(m_ucienc_auth[w_bss.encryption], w_bss.ifname))
    _fil(conf_bss, "encrypt_type", _conv_auth_enc(m_ucienc_auth[w_bss.encryption]))
    _fil(conf_bss, "ieee80211k", tonumber(w_bss.ieee80211k))
    _fil(conf_bss, "bss_transition", tonumber(w_bss.bss_transition))
  end
  local function _set_radiodata_fromuci(_conf, w, band)
    local _name = m_band_radioname[band]
    local w_band = w[_name]
    -- print(band, _name, w_band)
    if not w_band then
      return
    end
    _fil(_conf, band, {})
    local conf = _conf[band]
    _fil(conf, "MIB_REPEATER_SSID1", w_band.repeater_ssid or ("ssid_" .. _name))
    _fil(conf, "MIB_WLAN_CHAN_NUM", w_band.channel)
    _fil(conf, "htmode", w_band.htmode)
    _fil(conf, "MIB_WLAN_CHANNEL_WIDTH", m_htmode_chw[w_band.htmode] or BAND_WIDTH_40MHZ)
    _fil(conf, "vendor_data_nr", w_band.vendor_data_nr)
    _fil(conf, "vendor_oui", w_band.vendor_oui)
    _fil(conf, "vendor_payload", w_band.vendor_payload)
    _fil(conf, "vendor_load_len", w_band.vendor_load_len)
    _fil(conf, "ieee80211k", tonumber(w_band.ieee80211k))
    _fil(conf, "bss_transition", tonumber(w_band.bss_transition))
    if not conf["_BSS_DATAS"] then
      local bsss = {}
      for i = 0, 9 do
        local bss_name = get_bss_name(band, i) -- "default_" .. _name .. i
        local w_bss = w[bss_name]
        if w_bss then
          local bss = {}
          bss.bss_index = i
          _set_bssdata_fromuci(bss, w_bss)
          table.insert(bsss, bss)
        end
      end
      conf["_BSS_DATAS"] = bsss
    else
      for i, v in ipairs(conf["_BSS_DATAS"]) do
        local bss_name = get_bss_name(band, v.bss_index)
        local w_bss = w[bss_name]
        if w_bss then
          _set_bssdata_fromuci(v, w_bss)
        end
      end
    end
  end

  uci:load("wireless")
  local w = uci:get_all("wireless")
  _fil(conf, "MIB_HW_COUNTRY_STR", w.phy0.country or "US")
  _fil(conf, "MIB_HW_REG_DOMAIN", get_regdomain(w.phy0.country) or DOMAIN_GLOBAL)
  local bss_name
  bss_name = get_bss_name(MAP_CONFIG_2G, 0)
  if w[bss_name] then
    _fil(conf, "radio_name_24g", w[bss_name].ifname)
  end
  bss_name = get_bss_name(MAP_CONFIG_5GL, 0)
  if w[bss_name] then
    _fil(conf, "radio_name_5gl", w[bss_name].ifname)
  end
  bss_name = get_bss_name(MAP_CONFIG_5GH, 0)
  if w[bss_name] then
    _fil(conf, "radio_name_5gh", w[bss_name].ifname)
  else
    bss_name = get_bss_name(MAP_CONFIG_5GL, 0)
    if w[bss_name] then
      _fil(conf, "radio_name_5gh", w[bss_name].ifname)
    end
  end

  _fil(conf, "radios", {})
  local conf_radio = conf["radios"]

  if fexist(BSS_CONFIG_PATH) then
    -- use case: logo test
    -- this file will contain bss config from UCC to set downstream agents
    local a = ftds1.parse(fread(BSS_CONFIG_PATH))
    if a["2.4g_config_data"] then
      _fil(conf_radio, MAP_CONFIG_2G, {})
      _set_bssdata_fromfile(conf_radio[MAP_CONFIG_2G], a["2.4g_config_data"])
    end
    if a["5gl_config_data"] then
      _fil(conf_radio, MAP_CONFIG_5GL, {})
      _set_bssdata_fromfile(conf_radio[MAP_CONFIG_5GL], a["5gl_config_data"])
    end
    if a["5gh_config_data"] then
      _fil(conf_radio, MAP_CONFIG_5GH, {})
      _set_bssdata_fromfile(conf_radio[MAP_CONFIG_5GH], a["5gh_config_data"])
    end
  else
    _set_radiodata_fromuci(conf_radio, w, MAP_CONFIG_2G)
    _set_radiodata_fromuci(conf_radio, w, MAP_CONFIG_5GL)
    _set_radiodata_fromuci(conf_radio, w, MAP_CONFIG_5GH)
  end
end

DPP_CSIGN_KEY_PATH = "/tmp/dpp_csign_key"
DPP_NET_ACCESS_KEY_PATH = "/tmp/dpp_netaccess_key"
DPP_PRIVACY_PROTECTION_KEY_PATH = "/tmp/dpp_pp_key"
DPP_CONNECTOR_PATH_1905 = "/tmp/dpp_map_conn"

function mapconf.fill_dpp(conf)
  if not (fexist(DPP_CONNECTOR_PATH_1905) and fexist(DPP_NET_ACCESS_KEY_PATH) and
      fexist(DPP_CSIGN_KEY_PATH) and fexist(DPP_PRIVACY_PROTECTION_KEY_PATH)) then
    return
  end

  _fil(conf, "dpp", {})
  local conf_dpp = conf["dpp"]

  signed_connector_1905 = fread(DPP_CONNECTOR_PATH_1905)
  if signed_connector_1905 then
    _fil(conf_dpp, "signed_connector_1905", signed_connector_1905)
  end

  netaccess_key = fread(DPP_NET_ACCESS_KEY_PATH)
  if netaccess_key then
    _fil(conf_dpp, "netaccess_key", netaccess_key)
  end

  csign_key = fread(DPP_CSIGN_KEY_PATH)
  if csign_key then
    _fil(conf_dpp, "csign_key", csign_key)
  end

  pp_key = fread(DPP_PRIVACY_PROTECTION_KEY_PATH)
  if pp_key then
    _fil(conf_dpp, "pp_key", pp_key)
  end
end

function mapconf.set_save_uci(conf)
  local function _set_uci_radiodata(_conf, band, logotest)
    local radio_name = m_band_radioname[band]
    local conf = _conf[band]
    if not conf then return end
    uci:set("wireless", radio_name, "repeater_ssid", conf.MIB_REPEATER_SSID1)
    uci:set("wireless", radio_name, "channel", conf.MIB_WLAN_CHAN_NUM)
    uci:set("wireless", radio_name, "htmode", conf.htmode)
    uci:set("wireless", radio_name, "ieee80211k", conf.ieee80211k)
    uci:set("wireless", radio_name, "bss_transition", conf.bss_transition)
    local backhaul_ssid, backhaul_key = get_backhaul_ssid_key(conf["_BSS_DATAS"])
    for _, v in ipairs(conf["_BSS_DATAS"]) do
      local bss_name = get_bss_name(band, v.bss_index)
      if backhaul_ssid then
        uci:set("wireless", bss_name, "multi_ap_backhaul_ssid", backhaul_ssid)
        uci:set("wireless", bss_name, "multi_ap_backhaul_key", backhaul_key)
      else
        uci:delete("wireless", bss_name, "multi_ap_backhaul_ssid")
        uci:delete("wireless", bss_name, "multi_ap_backhaul_key")
      end
      if v.need_configure == 1 then
        if v.is_enabled == 1 then
          uci:delete("wireless", bss_name, "disabled")
        else
          uci:set("wireless", bss_name, "disabled", 1)
        end
        uci:set("wireless", bss_name, "ssid", v.ssid)
        uci:set("wireless", bss_name, "key", v.network_key)
        uci:set("wireless", bss_name, "encryption", _adjust_enc_dpp(v.authentication_type))
        uci:set("wireless", bss_name, "map_bss_type", v.network_type)
        uci:set("wireless", bss_name, "ieee80211w", _get_ieee80211w(v.authentication_type, v.network_type))
        uci:set("wireless", bss_name, "ieee80211k", v.ieee80211k)
        uci:set("wireless", bss_name, "bss_transition", v.bss_transition)

        local mode, multi_ap, wps_pushbutton, hidden
        if v.network_type == MAP_NETWORK_FRONTHAUL_AP then
          mode = "ap"
          multi_ap = 2
          wps_pushbutton = 1
        elseif v.network_type == MAP_NETWORK_BACKHAUL_AP then
          mode = "ap"
          multi_ap = 1
          if logotest ~= 1 then hidden = 1 end
        elseif v.network_type == MAP_NETWORK_FRONTBACK_AP then
          mode = "ap"
          multi_ap = 3
          wps_pushbutton = 1
        elseif v.network_type == MAP_NETWORK_BACKHAUL_STA then
          mode = "sta"
          multi_ap = 1
          wps_pushbutton = 1
        end

        if v.network_type and v.mode == mode then
          if multi_ap then
            uci:set("wireless", bss_name, "multi_ap", multi_ap)
          else
            uci:delete("wireless", bss_name, "multi_ap")
          end
          if wps_pushbutton then
            uci:set("wireless", bss_name, "wps_pushbutton", wps_pushbutton)
          else
            uci:delete("wireless", bss_name, "wps_pushbutton")
          end
          if hidden then
            uci:set("wireless", bss_name, "hidden", 1)
          else
            uci:delete("wireless", bss_name, "hidden")
          end
        else
          uci:delete("wireless", bss_name, "multi_ap")
          uci:delete("wireless", bss_name, "hidden")
          uci:delete("wireless", bss_name, "wps_pushbutton")
        end
      end
    end
  end

  --uci:set("rtkmap", "map", "controller", conf.role)
  --uci:set("rtkmap", "map", "log_level", conf.log_level)
  --uci:set("rtkmap", "map", "profile", conf.map_profile)
  --uci:set("rtkmap", "map", "hw_country_str", conf.MIB_HW_COUNTRY_STR)
  --uci:set("rtkmap", "map", "customize_features", conf.cusstomize_features)
  uci:set("rtkmap", "map", "configured_band", conf.MIB_MAP_CONFIGURED_BAND)
  uci:set("rtkmap", "map", "hw_reg_domain", conf.MIB_HW_REG_DOMAIN)
  uci:save("rtkmap")

  uci:set("network", "lan", "proto", conf.proto)
  uci:save("network")

  _set_uci_radiodata(conf.radios, MAP_CONFIG_2G, conf.logotest)
  _set_uci_radiodata(conf.radios, MAP_CONFIG_5GL, conf.logotest)
  _set_uci_radiodata(conf.radios, MAP_CONFIG_5GH, conf.logotest)
  uci:save("wireless")
end

function mapconf.set_save_uci_vlan(conf)
  uci:set("rtkmap", "map", "primary_vlan_name", conf.primary_vlan_name)
  uci:delete_all("rtkmap", "vlan")
  for i,v in ipairs(conf.vlans) do
    local sec = uci:add("rtkmap", "vlan")
    uci:set("rtkmap", sec, "name", v.name)
    uci:set_list("rtkmap", sec, "ssids", v.ssids)
  end
  uci:save("rtkmap")
end

function mapconf.commit_uci(...)
  execute("uci commit")
end

function mapconf.set_mib(conf)
  local function _set_mib_radiodata(_conf, band)
    local conf = _conf[band]
    if not conf then return end
    for _, v in ipairs(conf["_BSS_DATAS"]) do
      execute("iwpriv " .. v.ifname .. " set_mib a4_enable=1")
      execute("iwpriv " .. v.ifname .. " set_mib multiap_bss_type=" .. v.network_type)
    end
  end

  _set_mib_radiodata(conf.radios, MAP_CONFIG_2G)
  _set_mib_radiodata(conf.radios, MAP_CONFIG_5GL)
  _set_mib_radiodata(conf.radios, MAP_CONFIG_5GH)
end

function mapconf.stringify(conf)
  local function _strf_radio(conf, title)
    if not table.hasallatt(conf, "_BSS_DATAS") then
      return ""
    end
    local bss_data_2 = table.filter(conf["_BSS_DATAS"], function(v) return v.is_enabled == 1 end)
    if #bss_data_2 == 0 then return "" end
    local c = {
      number = #bss_data_2
    }
    local ans = "[" .. title .. "]\n"
    local vals = aggre(bss_data_2, { "ssid", "ssid_raw", "network_key", "network_key_raw", "network_type",
      "authentication_type", "encryption_type", "al_id", "vid", "bss_index" }, {
      ssid = base64.encode,
      network_key = base64.encode
    })
    tappend(c, vals,
      { "ssid", "ssid_raw", "network_key", "network_key_raw", "network_type",
      "authentication_type", "encryption_type", "al_id", "vid", "bss_index" })
    if fexist(BSS_CONFIG_PATH) then
      -- use case: logo test
      ans = ans .. ftds1.stringify2(c,
        { "number", "ssid_raw", "network_key_raw", "network_type",
          "authentication_type", "encryption_type", "al_id", "vid" })
    else
      ans = ans .. ftds1.stringify2(c,
        { "number", "ssid", "network_key", "network_type",
          "authentication_type", "bss_index" })
    end
    return ans
  end
  local function _strf_radio_2(conf, title)
    if not table.hasallatt(conf, "vendor_data_nr", "vendor_oui", "vendor_payload") then
      return ""
    end
    local ans = "[" .. title .. "]\n"
    ans = ans .. ftds1.stringify2(conf,
      { "vendor_data_nr", "vendor_oui", "vendor_payload", "vendor_load_len" })
    return ans
  end
  local function _strf_vlan(conf)
    local ans = ""
    if fexist(BSS_CONFIG_PATH) then
      -- use case: logo test
      ans = ftds1.stringify({
        vlan = {
          primary_vid = conf.primary_vid,
          default_pcp = conf.default_pcp
        }
      })
    else
      if conf.enable_vlan == 1 then
        local vlans = get_vlans(conf)
        if vlans then
          ans = ftds1.stringify({
            vlan = {
              primary_vid = conf.primary_vid
            },
            vlan_detail = aggre(vlans, { "ssid", "vid" }, {
              ssid = base64.encode,
            })
          })
        end
      end
    end
    return ans
  end

  ans = ftds1.stringify({
    global = trekey(conf, map1)
  })
  if conf["radios"][MAP_CONFIG_2G] then
    ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_2G], "2.4g_config_data")
    ans = ans .. _strf_radio_2(conf["radios"][MAP_CONFIG_2G], "vendor_data_2.4g")
  end
  if conf["radios"][MAP_CONFIG_5GL] then
    ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_5GL], "5gl_config_data")
    ans = ans .. _strf_radio_2(conf["radios"][MAP_CONFIG_5GL], "vendor_data_5gl")
  end
  if conf["radios"][MAP_CONFIG_5GH] then
    ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_5GH], "5gh_config_data")
    ans = ans .. _strf_radio_2(conf["radios"][MAP_CONFIG_5GH], "vendor_data_5gh")
  end
  ans = ans .. _strf_vlan(conf)
  return ans
end

function mapconf.stringify_mib(conf)
  local function _strf_radio(conf, title)
    -- print(conf.MIB_WLAN_CHAN_NUM, conf.MIB_WLAN_CHANNEL_WIDTH, conf.MIB_REPEATER_SSID1, conf._BSS_DATAS)
    if not table.hasallatt(conf, "MIB_WLAN_CHAN_NUM", "MIB_WLAN_CHANNEL_WIDTH", "MIB_REPEATER_SSID1", "_BSS_DATAS") then
      return ""
    end
    local bss_datas = table.copy(conf["_BSS_DATAS"], 2)
    local backhaul_ssid, backhaul_key = get_backhaul_ssid_key(bss_datas)
    if backhaul_ssid then conf.MIB_REPEATER_SSID1 = backhaul_ssid end
    local c = trekey(conf, { "MIB_WLAN_CHAN_NUM", "MIB_WLAN_CHANNEL_WIDTH", "MIB_REPEATER_SSID1" }, {
      MIB_WLAN_CHAN_NUM = "channel",
      MIB_WLAN_CHANNEL_WIDTH = "channel_bandwidth",
      MIB_REPEATER_SSID1 = "repeater_ssid"
    }, {
      MIB_REPEATER_SSID1 = base64.encode
    })
    c.bss_number = #bss_datas
    local vals = aggre(bss_datas, { "ssid", "network_key", "network_type", "is_enabled", "encrypt_type",
      "authentication_type", "bss_index" }, {
      ssid = base64.encode,
      network_key = base64.encode
    })
    tappend(c, vals,
      { "ssid", "network_key", "network_type", "is_enabled", "encrypt_type", "authentication_type", "bss_index" })
    local ans = "[" .. title .. "]\n"
    ans = ans .. ftds1.stringify2(c,
      { "channel", "channel_bandwidth", "repeater_ssid", "bss_number", "ssid", "network_key", "network_type",
        "is_enabled", "encrypt_type", "authentication_type", "bss_index" })
    return ans
  end
  local function _strf_dpp(conf, title)
    if not table.hasallatt(conf, "signed_connector_1905", "netaccess_key", "csign_key", "pp_key") then
      return ""
    end
    local ans = "[" .. title .. "]\n"
    ans = ans .. ftds1.stringify2(conf,
      { "signed_connector_1905", "netaccess_key", "csign_key", "pp_key" })
    return ans
  end
  local ans = ""
  ans = ans .. ftds1.stringify({
    global = trekey(conf, { "MIB_MAP_CONFIGURED_BAND", "MIB_HW_REG_DOMAIN", "MIB_HW_COUNTRY_STR", "customize_features" },
      {
        MIB_MAP_CONFIGURED_BAND = "configured_band",
        MIB_HW_REG_DOMAIN = "hw_reg_domain",
        MIB_HW_COUNTRY_STR = "hw_country_str"
      })
  })

  -- order must be preserved, or else configureDevice will swap 5gl and 2.4g and cause reload loop due to "interface_name[4] - '0'"
  if __2g_on_wlan0 == 0 then
    if conf["radios"][MAP_CONFIG_5GL] then
      ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_5GL], "mib_info_5gl")
    end
    if conf["radios"][MAP_CONFIG_2G] then
      ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_2G], "mib_info_2.4g")
    end
  else
    if conf["radios"][MAP_CONFIG_2G] then
      ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_2G], "mib_info_2.4g")
    end
    if conf["radios"][MAP_CONFIG_5GL] then
      ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_5GL], "mib_info_5gl")
    end
  end
  if conf["radios"][MAP_CONFIG_5GH] then
    ans = ans .. _strf_radio(conf["radios"][MAP_CONFIG_5GH], "mib_info_5gh")
  end

  if conf["dpp"] then
    ans = ans .. _strf_dpp(conf["dpp"], "dpp")
  end

  return ans
end

function mapconf.stringify_vlan(conf)
  local m = get_m_ssid_vid(conf)
  if not m then return "" end
  local s = ("Primary %d\n"):format(conf.primary_vid)
  for k,v in pairs(m) do
    s = s .. k .. " " .. v .. "\n"
  end
  return s
end

return mapconf
