--[[
 *
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
--]]

package.path = package.path .. ';/etc/ca/?.lua;/usr/lib/lua/rtk/?.lua'
package.cpath = package.cpath .. ';/etc/ca/?'

local bitop = require("rtk.bitop")
local uci = require("luci.model.uci").cursor()

INTERFACE_NUM = 5
ROOT_IDX = 0
VXD_IDX = 5

RADIO_NUM = 2
RADIO_24G_IDX = 1
RADIO_5GL_IDX = 0
RADIO_5GH_IDX = 2

IFACE_24G_IDX = 0
IFACE_5GL_IDX = 1
IFACE_5GH_IDX = 2

LATEST_PROFILE = 4

MAP_NETWORK_TEARDOWN = 16
MAP_NETWORK_FRONTHAUL_AP = 32
MAP_NETWORK_BACKHAUL_AP = 64

AUTH_WPA2 = tonumber("0x0020", 16)
AUTH_SAE = tonumber("0x0040", 16)
AUTH_DPP = tonumber("0x0080", 16)

ENCRYPT_TKIP = tonumber("0x0020", 16)
ENCRYPT_CCMP = tonumber("0x0040", 16)
ENCRYPT_AES = tonumber("0x0080", 16)

BSS_24G_COUNT=0
BSS_5GL_COUNT=0
BSS_5GH_COUNT=0

BRIDGE_NAME="br-lan"

ETH_PATH="/tmp/eth_interfaces"
BSS_CONFIG_PATH="/tmp/bss.config"

function capture(cmd, filter, need_all)
  logger.log("capture:", cmd)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  if not filter then return s end
  logger.log("string:", s, "filter:", filter)
  local result = nil
  for w in string.gmatch(s, filter) do
    if result then
      result = result .. " " .. w
    else
      result = w
    end
    if not need_all then return result end
  end
  return result
end

function capture_all(cmd, filter)
  return capture(cmd, filter, true)
end

function expect(cmd, filter, raw)
  logger.log("expect:", cmd)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  print(s)
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  if not filter then return s end
  logger.log("string:", s, "filter:", filter)
  filter = string.format("(%s)", filter)
  logger.debug(s)
  local result = nil
  for w in string.gmatch(s, filter) do
    if result then
      result = result .. " " .. w
    else
      result = w
    end
  end
  return result
end

function sleep(n)
  execute("sleep " .. tonumber(n))
end

function execute(cmd)
  logger.log("execute:", cmd)
  os.execute(cmd)
end

function split(str, sep)
  if sep == nil then
    sep = "%s"
  end
  local result={}
  for w in string.gmatch(str, "([^"..sep.."]+)") do
    table.insert(result, w)
  end
  return result
end

local function fexist(name)
  local f = io.open(name, "r")
  if f ~= nil then
    io.close(f)
    return true
  else
    return false
  end
end

local function fwrite(path, s)
  if not s then return end
  local f
  f = assert(io.open(path, "w+"))
  f:write(s)
  f:close()
end

local function fread(path)
  local f, s
  f = assert(io.open(path, "r"))
  s = f:read("*all")
  f:close()
  return s
end

local function fremove(path)
  os.remove(path)
end

function string_to_hex(str)
  local result = ""
  for i = 1, #str do
    local hex = string.format("%02X", string.byte(str:sub(i,i)))
    result = result..hex
  end
  result = string.lower(result)
  return result
end

function dev_reset_default(program, role)
  local command
  local profile = tonumber(string.match(program, "mapr(%d+)"))
  if not profile or profile > LATEST_PROFILE then
    return "ERROR,Unsupported profile"
  end

  logger.log("Resetting Realtek AP to", role, "with profile", profile)

  execute("killall -9 map_checker map_agent map_controller")

  -- set lan and wan ports
  uci:set("network", "lan", "proto", 'static')
  uci:set("network", "WAN", "proto", 'static')

  -- turn off dhcp
  uci:delete("dhcp", "lan", "ra_slaac")
  uci:set("dhcp", "lan", "ignore", '1')

  -- set map logotest
  uci:set("rtkmap", "map", "logotest", '1')

  if (string.match(role, "agent")) then
    execute("map_reset -a "..profile)
  else
    execute("map_reset -c "..profile)
  end
  return "COMPLETE,Success"
end

function format_mac(ruid)
  ruid = string.gsub(ruid, "0x", "")
  local result = nil
  for w in string.gmatch(ruid, "([%a%d][%a%d])") do
    if result then
      result = result .. ":" .. w
    else
      result = w
    end
  end
  return string.upper(result)
end

function get_radio_index_on_mac(mac)
  local result = nil
  local command = "ifconfig | grep "..mac
  result = capture(command, "wlan(%d)%s+Link")
  logger.log("Radio for", mac, "is", result)
  return result
end

function get_radio_interface_on_mac(mac)
  local result = nil
  local command = "ifconfig | grep "..mac
  result = capture(command, "(wlan[%d])[^%s]*%s+Link")
  logger.log("Radio for", mac, "is", result)
  return result
end

function read_proc_file(interface, file, entry, filter)
  local command = "cat /var/"..interface.."/"..file
  command = command.." | grep "..entry
  if not filter then
    filter = string.format("%s:(.*)",entry)
  end
  local result = capture_all(command, filter)
  if not result then
    return nil
  end
  result = string.gsub(result, "%(Len %d+%)%s+", "")
  return result
end

function get_addr_on_radio_ssid(ssid, uci_index)
  local radio_name = nil
  local interface_ssid = nil
  local bssid = nil
  logger.log("uci_index", uci_index)

  for i=0,5 do
    radio_name = "default_radio"..uci_index..i
    ifname = uci:get("wireless", radio_name, "ifname")
    logger.log("Checking ssid for", ifname)
    interface_ssid = uci:get("wireless", radio_name, "ssid")
    logger.log("SSID found is", interface_ssid)
    if (interface_ssid) then
      if (interface_ssid == ssid) then
        logger.log("Found match", ifname)
        command = "ifconfig "..ifname
        bssid = capture(command,"HWaddr%s+([%a%d:]+)")
        return bssid
      else
        logger.log("SSID ["..interface_ssid.."] does not match ["..ssid.."]")
      end
    else
      logger.log("No SSID is found in the interface "..ifname)
    end
  end
  return nil
end

function dev_start_buffer(message_types)
  local command = "rm -f /tmp/map_msgbuf ; echo " ..message_types.. " > /tmp/map_msgbuf_on"
  logger.log("Started logging messages to /tmp/map_msgbuf: "..message_types)
  execute(command)
  return "Success"
end

function dev_stop_buffer()
  execute("rm /tmp/map_msgbuf_on")
  logger.log("Stopped logging messages. Messages found at /tmp/map_msgbuf")
  return "Success"
end

function dev_get_frame_info(message, MID, filter, return_content)
  local f = io.open("/tmp/map_msgbuf",'r')
  if f ~= nil then
    io.close(f)
  else
    return "MessageTypePresent,No"
  end
  logger.log("Checking for "..return_content..message.." messages with MID:"..MID.." and the following filter: "..filter)
  if string.match(return_content, "TLV") then
    local tlvs = {}
    local response
    --Capture all TLVs
    if string.match(MID, "NA") then
      for line in io.lines("/tmp/map_msgbuf") do
        if string.match(line, message) then
          local temp= split(line, ",")
          local tlv = temp[6]
          table.insert(tlvs,tlv)
        end
      end
    else
    -- OR Capture only those that match MID
      for line in io.lines("/tmp/map_msgbuf") do
        if string.match(line, message) then
          local temp= split(line, ",")
          local mid = temp[4]
          local tlv = temp[6]
          if string.match(string.lower(mid),MID) then
            table.insert(tlvs,tlv)
          end
        end
      end
    end

    -- Return TLVs according to filter settings
    -- Lua starts indexing from 1
    if string.match(filter, "first") then
      response = tlvs[1]
    elseif string.match(filter, "last") then
      response = tlvs[#tlvs]
    elseif string.match(filter, "all") then
      response = table.concat(tlvs, ",")
    else
      logger.log("FILTER NOT IMPLEMENTED - "..filter)
    end

    -- Search and return the TLV if MID is specified
    if response ~= nil and #response > 0  then
      return "MessageTypePresent,Yes,TLVList,"..response
    else
      return "MessageTypePresent,No"
    end
  end
end

function dev_exec_action(dpp_action_type, dpp_crypto_identifier, dpp_bootstrapping_data, DPPBS)
  local result = "Success"

  --Get Bootstrapping Info
  if string.match(dpp_action_type, "GetLocalBootstrap") then
    if string.match(dpp_crypto_identifier, "P-256") then
      local uri = capture("cat /tmp/dpp_bootstrap_uri")
      logger.log(uri)
      result = "BootstrappingData,"..string_to_hex(uri)
      return "status,COMPLETE,"..result
    else
      logger.log("Unknown DPP Crypto Identifier")
      result = "UnknownDPPCryptoIdentifier"
      return "status,ERROR,"..result
    end
  end

  --Setting Bootstrapping Info
  if string.match(dpp_action_type, "SetPeerBootstrap") then
    local hex_to_char = {}
    for idx = 0, 255 do
      hex_to_char[("%02X"):format(idx)] = string.char(idx)
      hex_to_char[("%02x"):format(idx)] = string.char(idx)
    end
    local ascii
    ascii = dpp_bootstrapping_data:gsub("(..)", hex_to_char)
    execute("echo \"" ..ascii.. "\" > /tmp/dpp_bootstrap_peer_uri_set")
    return "status,COMPLETE,"..result
  end
end

function dev_get_parameter(parameter, ruid, ssid, mac_addr)
  local command
  if string.match(string.lower(parameter), "alid") then
    command = "ifconfig "..BRIDGE_NAME
    local alid = capture(command,"HWaddr%s+([%a%d:]+)")
    logger.log("alid:", alid)
    if not alid then
      return "status,ERROR,not found"
    end
    return "status,COMPLETE,ALid,"..alid
  elseif string.match(string.lower(parameter), "bssid") then
    local mac = format_mac(ruid)
    local radio_index = get_radio_index_on_mac(mac)
    logger.log("radio_index:", radio_index)
    if not radio_index then
      return "status,ERROR,not found"
    end
    if tonumber(radio_index) == IFACE_24G_IDX then
      uci_index = RADIO_24G_IDX
    elseif tonumber(radio_index) == IFACE_5GL_IDX then
      uci_index = RADIO_5GL_IDX
    else
      uci_index = RADIO_5GH_IDX
    end
    local bssid = get_addr_on_radio_ssid(ssid, uci_index)
    logger.log("bssid:", bssid)
    if not bssid then
      return "status,ERROR,not found"
    end
    bssid = format_mac(bssid)
    return "status,COMPLETE,bssid,"..bssid
  elseif string.match(string.lower(parameter), "macaddr") then
    local mac = format_mac(ruid)
    local radio_index = get_radio_index_on_mac(mac)
    local macaddr = nil
    logger.log("radio_index:", radio_index)
    if not radio_index then
      return "status,ERROR,not found"
    end
    if tonumber(radio_index) == IFACE_24G_IDX then
      uci_index = RADIO_24G_IDX
    elseif tonumber(radio_index) == IFACE_5GL_IDX then
      uci_index = RADIO_5GL_IDX
    else
      uci_index = RADIO_5GH_IDX
    end
    if not ssid then
      radio_name = "default_radio"..uci_index..VXD_IDX
      macaddr = uci:get("wireless", radio_name, "macaddr")
    else
      macaddr = get_addr_on_radio_ssid(ssid, uci_index)
    end
    logger.log("macaddr:", macaddr)
    if not macaddr then
      return "status,ERROR,not found"
    end
    macaddr = format_mac(macaddr)
    return "status,COMPLETE,macaddr,"..macaddr
  elseif string.match(string.lower(parameter), "passphrase") then
    local command = "cat /tmp/passphrase"
    local data = capture(command)
    local hash = {}
    logger.log("data: "..data)
    if (mac_addr) then
      mac_addr = string.gsub(mac_addr, "%s+", "")
    else
      return "status,ERROR,not found"
    end
    for ruid,key in string.gmatch(data, "([%x:]+)_([%a%d%p]+)") do
      ruid = string.upper(ruid)
      if (hash[ruid] == nil) then
        hash[ruid] = key
      else
        hash[ruid] = hash[ruid].."_"..key
      end
    end
    for ruid,key in pairs(hash) do
      if ruid == mac_addr then
        logger.log("passphrase: ", key)
        return "status,COMPLETE,passphrase,"..key
      end
    end
    return "status,ERROR,Unable to retrieve passphrase of "..mac_addr
  elseif string.match(string.lower(parameter), "pmk") then
    local command = "cat /tmp/pmk"
    local data = capture(command)
    local hash = {}
    logger.log("data: "..data)
    if (mac_addr) then
      mac_addr = string.gsub(mac_addr, "%s+", "")
    else
      return "status,ERROR,STA_MACAddress not found"
    end
    for ruid,key in string.gmatch(data, "([%x:]+)_([%x]+)") do
      ruid = string.upper(ruid)
      if (hash[ruid] == nil) then
        hash[ruid] = key
      else
        hash[ruid] = hash[ruid].."_"..key
      end
    end
    for ruid,key in pairs(hash) do
      if ruid == mac_addr then
        logger.log("PMK: ", key)
        return "status,COMPLETE,PMK,"..key
      end
    end
    return "status,ERROR,Unable to retrieve PMK of "..mac_addr
  elseif string.match(string.lower(parameter), "ptk") then
    command = "cat /tmp/ptk"
    local data = capture(command)
    local hash = {}
    logger.log("data: "..data)
    if (mac_addr) then
      mac_addr = string.gsub(mac_addr, "%s+", "")
    else
      return "status,ERROR,STA_MACAddress not found"
    end
    for ruid,key in string.gmatch(data, "([%x:]+)_([%x]+)") do
      ruid = string.upper(ruid)
      if (hash[ruid] == nil) then
        hash[ruid] = key
      else
        hash[ruid] = hash[ruid].."_"..key
      end
    end
    for ruid,key in pairs(hash) do
      if ruid == mac_addr then
        logger.log("PTK: ", key)
        return "status,COMPLETE,PTK,"..key
      end
    end
    return "status,ERROR,Unable to retrieve PTK of "..mac_addr
  end
  return "status,COMPLETE,Success"
end

function set_backhaul(backhaul,type)
  local radio_name = nil
  -- reset multiap_bss_type of all radios
  for i=0,(RADIO_NUM-1) do
    for j=0,INTERFACE_NUM do
      radio_name = "default_radio"..i..j
      uci:set("wireless", radio_name, "map_bss_type", '0')
    end
  end
  if string.match(string.lower(backhaul), "eth") then
    logger.log("Set backhaul to ethernet")
    -- disable all vxd interfaces
    for i=0,(RADIO_NUM-1) do
      radio_name = "default_radio"..i..VXD_IDX
      uci:set("wireless", radio_name, "disabled", 1)
    end
    -- enable all eth interfaces
    if fexist(ETH_PATH) then
      -- this means that thet eth interfaces were previously removed
      found = fread(ETH_PATH)
      fremove(ETH_PATH)
      interfaces = string.gmatch(found, "%S+")
      eth_interfaces = {}
      for interface in interfaces do
        table.insert(eth_interfaces, interface)
      end
      uci:set("network", "@device[0]", "ports", eth_interfaces)
      uci:commit("network")
    else
      return "status,ERROR,Cannot find eth interfaces"
    end
    -- enable all eth interfaces with VID
    -- eth_interfaces = capture_all("brctl show", "(eth%d*%.%d*.%d*)")
    -- for i,interface in ipairs(split(eth_interfaces)) do
    --   execute("ifconfig "..interface.. " up")
    -- end
  else
    logger.log("Set backhaul to wifi, mac", backhaul)
    backhaul = format_mac(backhaul)
    local radio_index = tonumber(get_radio_index_on_mac(backhaul))
    if not radio_index or radio_index >= RADIO_NUM then
      return "status,ERROR,Cannot find radio"
    end
    local bsta_interface = "wlan"..radio_index.."-vxd"
    logger.log("Selected "..bsta_interface.." as backhaul interface")
    if tonumber(radio_index) == IFACE_24G_IDX then
      uci_index = RADIO_24G_IDX
    elseif tonumber(radio_index) == IFACE_5GL_IDX then
      uci_index = RADIO_5GL_IDX
    else
      uci_index = RADIO_5GH_IDX
    end
    for i=0,(RADIO_NUM-1) do
      radio_name = "default_radio"..i..VXD_IDX
      if i == uci_index then
        uci:delete("wireless", radio_name, "disabled")
        uci:set("wireless", radio_name, "map_bss_type", '128')
      else
        uci:set("wireless", radio_name, "disabled", 1)
      end
    end
    execute("ifconfig "..bsta_interface.." up; brctl addif "..BRIDGE_NAME.." "..bsta_interface)
  end
  execute("rm /tmp/map_lock")
  -- commit uci
  uci:commit("wireless")
  sleep(60)
  if (type ~= nil) and (not string.match(string.lower(backhaul), "eth")) and (string.match(type, "dpp")) then
    -- Generate URI after wpa_supplicant starts after map_reinit
    backhaul = format_mac(backhaul)
    local radio_index = get_radio_index_on_mac(backhaul)
    if not radio_index then
      return "status,ERROR,Cannot find radio"
    end
    local vxd_interface = "wlan"..radio_index.."-vxd"
    local mac = capture("iw dev "..vxd_interface.." info", "addr ([%a%d:]+)")
    logger.log(mac)
    local prime256PVK = capture("cat /tmp/dpp_bootstrap_key")
    local command = "wpa_cli -i "..vxd_interface.." dpp_bootstrap_gen type=qrcode mac="..mac.." key="..prime256PVK
    local uri_index = capture(command)
    execute("wpa_cli -i "..vxd_interface.." dpp_bootstrap_get_uri "..uri_index)

    -- create and execute bash script for DPP chirping
    command = "echo 'for i in ''$''(seq 4); do wpa_cli -i "..vxd_interface.." dpp_chirp own="..uri_index.." multi_ap=1; sleep 30; done' > /tmp/dpp_chirp.sh"
    execute(command)
    execute("/bin/sh /tmp/dpp_chirp.sh &")
  end
  return "status,COMPLETE,Success"
end

function init_bss_data(bss_data)
  if bss_data then
    return bss_data
  end
  local bss = {}
  bss['number'] = 0
  return bss
end

function append_bss_data(bss, new_entry, key)
  if not bss[key] then
    bss[key] = key..' = '..new_entry
  else
    bss[key] = bss[key]..','..new_entry
  end
  return bss
end

function fill_up_bss_data(bss, alid, ssid, auth, encrypt, key, bss_type, vid)
  bss['number'] = bss['number'] + 1
  append_bss_data(bss, alid, "al_id")
  append_bss_data(bss, ssid, "ssid_raw")
  append_bss_data(bss, key, "network_key_raw")
  auth = string.gsub(auth, "0x", "")
  append_bss_data(bss, tonumber(auth, 16), "authentication_type")
  encrypt = string.gsub(encrypt, "0x", "")
  append_bss_data(bss, tonumber(encrypt, 16), "encryption_type")
  append_bss_data(bss, bss_type, "network_type")
  append_bss_data(bss, vid, "vid")
end

function fill_up_tear_down_bss_data(bss, alid)
  bss['number'] = bss['number'] + 1
  append_bss_data(bss, alid, "al_id")
  append_bss_data(bss, "teardown", "ssid_raw")
  append_bss_data(bss, "teardown", "network_key_raw")
  append_bss_data(bss, 16, "network_type")
  append_bss_data(bss, 0, "authentication_type")
  append_bss_data(bss, 0, "encryption_type")
  append_bss_data(bss, 0, "vid")
end

function get_bss_type(backhaul, fronthaul)
  local bss_type = nil
  if string.match(backhaul, "1") then
    bss_type = MAP_NETWORK_BACKHAUL_AP
  end
  if string.match(fronthaul, "1") then
    bss_type = MAP_NETWORK_FRONTHAUL_AP
  end

  if not bss_type then
    bss_type = MAP_NETWORK_TEARDOWN
  end
  return bss_type
end

function parse_vlan_info(bss_config, bss_info, ssid)
  local first, last
  local primary_vid, pcp, vid, vlan_ssid  = nil
  vid = 0
  local t,l,v
  local escaped_pattern = string.gsub(bss_info, "-", "%%-")
  first,last = string.find(bss_config, escaped_pattern)
  first = string.find(bss_config, "bss_info", last + 1)
  if not first then
    first = -1
  end
  local vlan_tlvs = string.sub(bss_config, last +1, first - 1)
  print(vlan_tlvs)
  for t,l,v in string.gmatch(vlan_tlvs, "tlv_type%d,0x([%a%d]+),tlv_length%d,0x([%a%d]+),tlv_value%d,([%a%d%s%-{}]+)") do
    if string.match(string.upper(t), "B5") then
      primary_vid, pcp = string.match(v, "{0x([%a%d]+)%s+0x([%a%d]+)}")
      primary_vid = tonumber(primary_vid, 16)
      pcp = tonumber(pcp, 16)
      logger.log("primary_vid:", primary_vid)
      logger.log("pcp:", pcp)
    elseif string.match(string.upper(t), "B6") then
      v = string.gsub(v, "[{}]", "")
      vlan_ssid, vid = string.match(v, "0x[%a%d]+%s+0x[%a%d]+%s+([%a%d%-]+)%s+0x([%a%d]+)")
      vid = tonumber(vid, 16)
      logger.log("ssid", vlan_ssid, "vid", vid)
      if ssid ~= vlan_ssid then
        vid = 0
        logger.log("SSID ["..ssid.."] and ["..vlan_ssid.."] does not match! Reset vid to 0.")
      end
    end
  end
  return primary_vid, pcp, vid
end

function parse_bss_info(bss_config)
  local bss_info = {}
  local bss = nil

  for bss in string.gmatch(bss_config, "(bss_info%d,[%a%d%s-_:]+)") do
    local alid, band, ssid, auth, encrypt, key, backhaul, fronthaul = string.match(bss, "bss_info%d,([%a%d:]+) (%d+x) ([%a%d-_]+) (0x%x+) (0x%d+) ([%a%d]+) (%d) (%d)")
    if band == nil then
      local alid, band = string.match(bss, "bss_info%d,([%a%d:]+) (%d+x)")
      bss_info[band] = init_bss_data(bss_info[band])
      fill_up_tear_down_bss_data(bss_info[band], alid)
      return bss_info
    end
    local bss_type = get_bss_type(backhaul, fronthaul)
    logger.log(alid, band, ssid, auth, encrypt, key, backhaul, fronthaul, bss_type)
    local primary_vid, pcp, vid = parse_vlan_info(bss_config, bss, ssid)
    if primary_vid then
      bss_info['primary_vid'] = primary_vid
    end
    if pcp then
      bss_info['default_pcp'] = pcp
    end
    bss_info[band] = init_bss_data(bss_info[band])
    fill_up_bss_data(bss_info[band], alid, ssid, auth, encrypt, key, bss_type, vid)
  end
  return bss_info
end

function bss_info_to_stream(bss_info)
  local stream = ''
  local keys = {"8x", "11x", "12x"}

  if bss_info['primary_vid'] and bss_info['default_pcp'] then
    stream = string.format("[vlan]\nprimary_vid = %s\ndefault_pcp = %s\n\n", bss_info['primary_vid'], bss_info['default_pcp'])
  end

  for _,k in ipairs(keys) do
    local bss = bss_info[k]
    if bss and bss['number'] and bss['number'] > 0 then
      if k == "8x" then
        stream = stream..'[2.4g_config_data]\n\n'
      elseif k == "11x" then
        stream = stream..'[5gl_config_data]\n\n'
      elseif k == "12x" then
        stream = stream..'[5gh_config_data]\n\n'
      end

      stream = stream .. "number = " .. bss['number'] ..'\n'
      stream = stream .. bss['ssid_raw'] ..'\n'
      stream = stream .. bss['network_key_raw'] ..'\n'
      stream = stream .. bss['authentication_type'] ..'\n'
      stream = stream .. bss['encryption_type'] ..'\n'
      stream = stream .. bss['network_type'] ..'\n'
      stream = stream .. bss['al_id'] ..'\n'
      stream = stream .. bss['vid'] ..'\n\n'
    end
  end
  logger.log("Config Data:\n\n"..stream)
  return stream
end

function set_bss_config(bss_config)
  local bss_info = parse_bss_info(bss_config)
  local bss_stream = bss_info_to_stream(bss_info)
  fwrite(BSS_CONFIG_PATH, bss_stream)
  execute("service rtk_multiap restart")
  sleep(20)

  return "status,COMPLETE,Success"
end

function dev_send_1905(alid, message_type, tlvs)
  logger.log(string.format("dev_send_1905 to %s, type %04x", alid, message_type))

  stream = ""
  for word in tlvs:gmatch('[^,%s]+') do
    --print(word)
    word = string.gsub(word, "[,{}]", '')
    if string.find(word, 'tlv_type') then
      --skip this regex, do nothing
    elseif string.find(word, 'tlv_length') then
      --skip this regex, do nothing
    elseif string.find(word, 'tlv_value') then
      --skip this regex, do nothing
    elseif string.find(word, '0x') then
      stream = stream..string.gsub(word, "0x", '')
    elseif string.find(word, ':') then
      stream = stream..string.gsub(word, ":", '')
    else
      tlv_byte = ""
      tlv_hex = ""
      for i = 1,string.len(word) do
        tlv_byte = string.byte(word, i)
        tlv_hex = tlv_hex..string.format("%x",tlv_byte)
      end
      --print(tlv_hex)
      stream = stream..tlv_hex
    end
  end
  stream = stream.."0000"
  print(stream)
  local fp = io.open("/tmp/tlv", "w+")
  for w in string.gmatch(stream, "([%a%d][%a%d])") do
    fp:write(string.char(tonumber(w, 16)))
  end
  fp:close()
  local command = string.format("hle_entity -a 127.0.0.1:8888 -m ALME-MULTIAP-COMMAND.request %s 0x%04x /tmp/tlv", alid, message_type)
  local mid = capture(command, "MID:%s*(.+)")
  return "MID,"..mid
end

function dev_set_rfeature(bssid, assos_disallow)
  local macaddr = string.upper(bssid)
  local result = "FAIL"

  if string.find(bssid, "0x") then
    macaddr = format_mac(bssid)
  end

  local disallow = 0
  if string.match(assos_disallow, "Enable") then
    disallow = 1
  end

  local radio_intf = get_radio_interface_on_mac(macaddr)
  if fexist("/proc/net/rtl8852ae/"..radio_intf) then
    logger.log(radio_intf .. " is using g6 driver 32ar")
    result = capture_all("hostapd_cli -i ".. radio_intf .." set mbo_assoc_disallow "..disallow, "(%a+)")
    if not string.match(result, "FAIL") then
      result = "Success"
    end
  elseif fexist("/proc/net/rtk_wifi6/"..radio_intf) then
    logger.log(radio_intf .. " is using g6 driver 32br")
    result = capture_all("hostapd_cli -i ".. radio_intf .." set mbo_assoc_disallow "..disallow, "(%a+)")
    if not string.match(result, "FAIL") then
      result = "Success"
    end
  else
    execute("iwpriv ".. radio_intf .." set_mib multiap_assoc_disallowed="..disallow)
    result = "Success"
  end

  return result
end

function dev_set_trigger(trigger, action)
  logger.log("Trigger: "..trigger)
  logger.log("Action: "..action)
  return "Success"
end

function start_wps_registration(band)
  uci:commit("rtkmap")
  sleep(5)
  execute("echo 1 > /tmp/virtual_push_button")
  return "Success"
end

function parse_ucc_command(line, excludes)
  line = string.gsub(line, excludes, "")
  local result = {}
  for k, v in string.gmatch(line, "([^,]+),([^,]+)") do
    result[k] = v
  end
  return result
end

function process_ucc_command(line)
  local ret = "Success"
  local ucc_cmd = nil

  logger.log("Command Received:", line)

  if (string.find(line, "ca_get_version") == 1) then
    return "status,COMPLETE,Control Agent v1.0.0"
  elseif (string.match(line, "device_get_info")) then
    return "status,COMPLETE,vendor,Realtek,model,RTL8832AR,version,1.3.2"
  elseif (string.match(line, "dev_reset_default,")) then
    ucc_cmd = parse_ucc_command(line, "dev_reset_default,")
    ret = dev_reset_default(ucc_cmd['program'], ucc_cmd['devrole'])
    return "status,"..ret
  elseif (string.match(line, "dev_get_parameter,")) then
    ucc_cmd = parse_ucc_command(line, "dev_get_parameter,")
    return dev_get_parameter(ucc_cmd['parameter'], ucc_cmd['ruid'], ucc_cmd['ssid'], ucc_cmd['STA_MACAddress'])
  elseif (string.match(line, "dev_set_config,")) then
    ucc_cmd = parse_ucc_command(line, "dev_set_config,")
    if ucc_cmd['backhaul'] then
      return set_backhaul(ucc_cmd['backhaul'], ucc_cmd['type'])
    end
    local last = nil
    _, last = string.find(line, ucc_cmd['program'])
    local bss_config = string.sub(line, last + 2, -1)
    return set_bss_config(bss_config)
  elseif (string.match(line, "dev_send_1905,")) then
    ucc_cmd = parse_ucc_command(line, "dev_send_1905,")
    local last = nil
    _, last = string.find(line, ucc_cmd['MessageTypeValue'])
    local tlv_stream = string.sub(line, last + 2, -1)
    ret = dev_send_1905(ucc_cmd['DestALid'], ucc_cmd['MessageTypeValue'], tlv_stream)
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "dev_set_rfeature,")) then
    ucc_cmd = parse_ucc_command(line, "dev_set_rfeature,")
    ret = dev_set_rfeature(ucc_cmd['bssid'], ucc_cmd['Assoc_Disallow'])
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "start_wps_registration,")) then
    ucc_cmd = parse_ucc_command(line, "start_wps_registration,")
    ret = start_wps_registration(ucc_cmd['band'])
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "dev_start_buffer,")) then
    ucc_cmd = parse_ucc_command(line, "dev_start_buffer,")
    ret = dev_start_buffer(ucc_cmd["MessageTypes"])
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "dev_stop_buffer,")) then
    ret = dev_stop_buffer()
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "dev_get_frame_info,")) then
    ucc_cmd = parse_ucc_command(line, "dev_get_frame_info,")
    ret = dev_get_frame_info(ucc_cmd["message"], ucc_cmd["MID"], ucc_cmd["filter"], ucc_cmd["returncontent"])
    return "status,COMPLETE,"..ret
  elseif (string.match(line, "dev_exec_action,")) then
    ucc_cmd = parse_ucc_command(line, "dev_exec_action,")
    ret = dev_exec_action(ucc_cmd["DPPActionType"], ucc_cmd["DPPCryptoIdentifier"], ucc_cmd["DPPBootstrappingData"], ucc_cmd["DPPBS"])
    return ret
  elseif (string.match(line, "dev_set_trigger,")) then
    ucc_cmd = parse_ucc_command(line, "dev_set_trigger,")
    ret = dev_set_trigger(ucc_cmd["trigger"], ucc_cmd["action"])
    return "status,COMPLETE,"..ret
  end

  return "status,ERROR,Invalid command"
end

function start_server(port)
  local socket = require("socket")
  local server = assert(socket.bind("*", port))
  local client = nil
  local line, err
  local result = nil

  logger.log("Control Agent is running at", port)

  while true do
    if client == nil then
      client = server:accept()
    end

    line, err = client:receive()

    if err then
      logger.log("connection error: ", err)
      client:close()
      client = nil
    else
      -- Send immediate respone to UCC
      client:send("status,RUNNING\n")
      result = process_ucc_command(line)
      logger.log(result)
      client:send(result.."\n")
    end
  end

  if (client ~= nil) then
    client:close()
  end
  if (server ~= nil) then
    server:close()
  end
end

local port = port or 7000

if arg then
  port = arg[1] or port
end

if pcall(os.execute("rm /var/log/ca.log")) then
  print("Log cleared.")
end

logger = require("logger")

logger.init("/var/log/ca.log")

logger.log("############ CA Started ############")

map_profile = 2

start_server(port)
