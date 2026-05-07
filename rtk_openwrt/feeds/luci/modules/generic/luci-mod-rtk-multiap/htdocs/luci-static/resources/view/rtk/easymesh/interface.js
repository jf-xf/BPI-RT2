'use strict';
'require view';
'require uci';
'require rpc';
'require form';
'require fs';
'require ui';

var deviceBand = '5g';

function getRandomInt(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1) + min); // The maximum is inclusive and the minimum is inclusive
}

function getRandomSSID() {
	var ssid = '';
	var ssidDic = [	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
					'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
					'1','2','3','4','5','6','7','8','9','0'];

	ssid = "%s%s%s%s%s%s%s%s%s%s".format('EasyMeshBH-',
				ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)],
				ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)], ssidDic[getRandomInt(0, 61)]);

	return ssid;
}

function getRandomKEY() {
	var key = '';
	var KeyDic =  [	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
					'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
					'1','2','3','4','5','6','7','8','9','0',
					'~','!','@','#','0','^','&','*','(',')','_','+','{','}','[',']',':',';','.','.','?'];

	key = "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s".format(
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)],
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)],
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)],
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)],
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)],
				KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)], KeyDic[getRandomInt(0, 82)]);

	return key;
}

function set5G(role) {
	var ssid = '';
	var key = '';
	var wifi_ifaces;

	wifi_ifaces = uci.sections('wireless', 'wifi-iface');

	uci.unset('wireless', 'default_radio01', 'disabled');
	uci.set('wireless', 'default_radio11', 'disabled', '1');

	if (role == '1') {
		// Backhaul AP for 5GHz
		uci.set('wireless', 'default_radio01', 'multi_ap', '1');
		uci.set('wireless', 'default_radio01', 'map_bss_type', '64');
		uci.set('wireless', 'default_radio01', 'a4_enable', '1');
		ssid = getRandomSSID();
		uci.set('wireless', 'default_radio01', 'ssid', ssid);
		key = getRandomKEY();
		uci.set('wireless', 'default_radio01', 'key', key);
		uci.set('wireless', 'default_radio01', 'encryption', 'sae-mixed');
		uci.unset('wireless', 'default_radio01', 'disabled');
		uci.set('wireless', 'default_radio01', 'hidden', '1');

		// Fronthaul AP for 5GHz
		uci.set('wireless', 'default_radio00', 'multi_ap', '2');
		uci.set('wireless', 'default_radio00', 'multi_ap_backhaul_ssid', ssid);
		uci.set('wireless', 'default_radio00', 'multi_ap_backhaul_key', key);
		uci.set('wireless', 'default_radio00', 'map_bss_type', '32');
		uci.set('wireless', 'default_radio00', 'wps_pushbutton', '1');
		uci.set('wireless', 'default_radio00', 'a4_enable', '1');

		// delete Backhaul AP for 2.4GHz
		uci.unset('wireless', 'default_radio11', 'multi_ap');
		uci.unset('wireless', 'default_radio11', 'map_bss_type');
		uci.unset('wireless', 'default_radio11', 'a4_enable');
		uci.set('wireless', 'default_radio11', 'disabled', '1');

		// delete Fronthaul AP for 2.4GHz
		uci.unset('wireless', 'default_radio10', 'multi_ap');
		uci.unset('wireless', 'default_radio10', 'multi_ap_backhaul_ssid');
		uci.unset('wireless', 'default_radio10', 'multi_ap_backhaul_key');
		uci.unset('wireless', 'default_radio10', 'map_bss_type');
		uci.unset('wireless', 'default_radio10', 'a4_enable');

		// for 5G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'ap' && wifi_ifaces[i]['.name'] != 'default_radio00' && wifi_ifaces[i]['.name'] != 'default_radio01') {
				uci.set('wireless', wifi_ifaces[i]['.name'], 'multi_ap', '2');
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
		}

		// for 2.4G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'ap') {  // for ALL AP on 2.4G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'multi_ap', '2');
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 2.4G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
		}
	}
	else if (role == '0') {
		// for 5G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'ap' && wifi_ifaces[i]['.name'] != 'default_radio00' && wifi_ifaces[i]['.name'] != 'default_radio01') {
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'multi_ap');
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
		}

		// for 2.4G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'ap') {  // for ALL AP on 2.4G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'multi_ap');
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 2.4G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
		}
	}
}

function set2G(role) {
	var ssid = '';
	var key = '';
	var wifi_ifaces;

	wifi_ifaces = uci.sections('wireless', 'wifi-iface');

	uci.set('wireless', 'default_radio01', 'disabled', '1');
	uci.unset('wireless', 'default_radio11', 'disabled');

	if (role == '1') {
		// Backhaul AP for 2.4GHz
		uci.set('wireless', 'default_radio11', 'multi_ap', '1');
		uci.set('wireless', 'default_radio11', 'map_bss_type', '64');
		uci.set('wireless', 'default_radio11', 'a4_enable', '1');
		ssid = getRandomSSID();
		uci.set('wireless', 'default_radio11', 'ssid', ssid);
		key = getRandomKEY();
		uci.set('wireless', 'default_radio11', 'key', key);
		uci.set('wireless', 'default_radio11', 'encryption', 'sae-mixed');
		uci.unset('wireless', 'default_radio11', 'disabled');
		uci.set('wireless', 'default_radio11', 'hidden', '1');

		// Fronthaul AP for 2.4GHz
		uci.set('wireless', 'default_radio10', 'multi_ap', '2');
		uci.set('wireless', 'default_radio10', 'multi_ap_backhaul_ssid', ssid);
		uci.set('wireless', 'default_radio10', 'multi_ap_backhaul_key', key);
		uci.set('wireless', 'default_radio10', 'map_bss_type', '32');
		uci.set('wireless', 'default_radio10', 'wps_pushbutton', '1');
		uci.set('wireless', 'default_radio10', 'a4_enable', '1');

		// delete Backhaul AP for 5GHz
		uci.unset('wireless', 'default_radio01', 'multi_ap');
		uci.unset('wireless', 'default_radio01', 'map_bss_type');
		uci.unset('wireless', 'default_radio01', 'a4_enable');
		uci.set('wireless', 'default_radio01', 'disabled', '1');

		// delete Fronthaul AP for 5GHz
		uci.unset('wireless', 'default_radio00', 'multi_ap');
		uci.unset('wireless', 'default_radio00', 'multi_ap_backhaul_ssid');
		uci.unset('wireless', 'default_radio00', 'multi_ap_backhaul_key');
		uci.unset('wireless', 'default_radio00', 'map_bss_type');
		uci.unset('wireless', 'default_radio00', 'a4_enable');

		// for 2.4G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'ap' && wifi_ifaces[i]['.name'] != 'default_radio10' && wifi_ifaces[i]['.name'] != 'default_radio11') {
				uci.set('wireless', wifi_ifaces[i]['.name'], 'multi_ap', '2');
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
		}

		// for 5G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'ap') {  // for ALL AP on 5G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'multi_ap', '2');
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.set('wireless', wifi_ifaces[i]['.name'], 'map_bss_type', '32');
			}
		}
	}
	else if (role == '0') {
		// for 2.4G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'ap' && wifi_ifaces[i]['.name'] != 'default_radio10' && wifi_ifaces[i]['.name'] != 'default_radio11') {
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'multi_ap');
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
			if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
		}

		// for 5G
		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'ap') {  // for ALL AP on 5G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'multi_ap');
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
			if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode == 'sta') {  // for vxd(client) on 5G
				uci.unset('wireless', wifi_ifaces[i]['.name'], 'map_bss_type');
			}
		}
	}
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('wireless')
		]);
	},

	handleWps: function(ev) {
		//"/bin/hostapd_cli wps_pbc -i wlan0": [ "exec" ],
		var device = uci.get('rtkmap', 'map', 'configured_band');
		if (device == '5g') {
			//console.log('run 5g');
			fs.exec('/usr/sbin/hostapd_cli', [ '-i', 'wlan0', 'wps_pbc' ]);
		}
		else {
			//console.log('run 24g');
			fs.exec('/usr/sbin/hostapd_cli', [ '-i', 'wlan1', 'wps_pbc' ]);
		}
	},

	render: function() {
		var m, s, o;

		m = new form.Map('rtkmap', _('EasyMesh General Settings'),
			_('This page is used to configure the parameters for EasyMesh feature of your Access Point.'));

		s = m.section(form.TypedSection, 'rtk_map');
		s.anonymous = true;

		o = s.option(form.Value, 'device_name',  _('Device Name'));

		o = s.option(form.ListValue, 'map_role', _('Role'));
		o.value('0', _('Disabled'));
		o.value('1', _('Controller'));
		o.validate = function(section_id, value) {
			if (value == '1') {
				var security_5g_type = uci.get('wireless', 'default_radio00', 'encryption');
				var security_2g_type = uci.get('wireless', 'default_radio10', 'encryption');
				/*
				if (((security_5g_type != 'psk2') && (security_5g_type != 'sae-mixed') && (security_5g_type != 'psk2+ccmp') && (security_5g_type != 'psk2+tkip+ccmp') && (security_5g_type != 'psk2+aes') &&
					(security_5g_type != 'psk2+tkip+aes') && (security_5g_type != 'psk-mixed+tkip+ccmp') && (security_5g_type != 'psk-mixed+tkip+aes') && (security_5g_type != 'psk-mixed+ccmp') &&
					(security_5g_type != 'psk-mixed+aes') && (security_5g_type != 'psk-mixed'))
					||
					((security_2g_type != 'psk2') && (security_2g_type != 'sae-mixed') && (security_2g_type != 'psk2+ccmp') && (security_2g_type != 'psk2+tkip+ccmp') && (security_2g_type != 'psk2+aes') &&
					(security_2g_type != 'psk2+tkip+aes') && (security_2g_type != 'psk-mixed+tkip+ccmp') && (security_2g_type != 'psk-mixed+tkip+aes') && (security_2g_type != 'psk-mixed+ccmp') &&
					(security_2g_type != 'psk-mixed+aes') && (security_2g_type != 'psk-mixed'))
					) {
					return _('Because something wrong with your security settings. Please be aware that EasyMesh upports WPA2 with PSK or sae-mixed. Please ensure security type is WPA2 or sae-mixed and password is not null.');
				}
				*/
				if (( security_5g_type == 'psk-mixed+tkip' || security_5g_type == 'psk2+tkip' || (security_5g_type.indexOf('psk-mixed') == -1 && security_5g_type.indexOf('psk2') == -1 && security_5g_type.indexOf('sae-mixed') == -1) ) ||
				    ( security_2g_type == 'psk-mixed+tkip' || security_2g_type == 'psk2+tkip' || (security_2g_type.indexOf('psk-mixed') == -1 && security_2g_type.indexOf('psk2') == -1 && security_2g_type.indexOf('sae-mixed') == -1) )
				) {
					return _('Because something wrong with your security settings. Please be aware that EasyMesh upports WPA2 with PSK or SAE-MIXED. Please ensure security type is WPA2 or SAE-MIXED and password is not null.');
				}
			}
			return true;
		};
		o.write = function(section_id, value) {
			if (value == '1') {
				var wifi_ifaces;

				uci.set('rtkmap', section_id, 'map_role', '1');

				wifi_ifaces = uci.sections('wireless', 'wifi-iface');

				// Set 11k for 5GHz
				uci.set('wireless', 'phy0', 'ieee80211k', '1');
				for (var i = 0; i < wifi_ifaces.length; i++) {
					if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode != 'sta') {
						uci.set('wireless', wifi_ifaces[i]['.name'], 'ieee80211k', '1');
					}
				}

				// Set 11v for 5GHz
				uci.set('wireless', 'phy0', 'bss_transition', '1');
				for (var i = 0; i < wifi_ifaces.length; i++) {
					if (wifi_ifaces[i].device == 'phy0' && wifi_ifaces[i].mode != 'sta') {
						uci.set('wireless', wifi_ifaces[i]['.name'], 'bss_transition', '1');
					}
				}

				// Set 11k for 2.4GHz
				uci.set('wireless', 'phy1', 'ieee80211k', '1');
				for (var i = 0; i < wifi_ifaces.length; i++) {
					if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode != 'sta') {
						uci.set('wireless', wifi_ifaces[i]['.name'], 'ieee80211k', '1');
					}
				}

				// Set 11v for 2.4GHz
				uci.set('wireless', 'phy1', 'bss_transition', '1');
				for (var i = 0; i < wifi_ifaces.length; i++) {
					if (wifi_ifaces[i].device == 'phy1' && wifi_ifaces[i].mode != 'sta') {
						uci.set('wireless', wifi_ifaces[i]['.name'], 'bss_transition', '1');
					}
				}

				if (deviceBand == '5g')
					set5G(value);
				else if (deviceBand == '2g')
					set2G(value);
			}
			else {
				uci.set('rtkmap', section_id, 'map_role', '0');

				// delete Fronthaul AP for 5GHz
				uci.unset('wireless', 'default_radio00', 'multi_ap');
				uci.unset('wireless', 'default_radio00', 'multi_ap_backhaul_ssid');
				uci.unset('wireless', 'default_radio00', 'multi_ap_backhaul_key');
				uci.unset('wireless', 'default_radio00', 'map_bss_type');
				uci.unset('wireless', 'default_radio00', 'a4_enable');

				// delete Fronthaul AP for 2.4GHz
				uci.unset('wireless', 'default_radio10', 'multi_ap');
				uci.unset('wireless', 'default_radio10', 'multi_ap_backhaul_ssid');
				uci.unset('wireless', 'default_radio10', 'multi_ap_backhaul_key');
				uci.unset('wireless', 'default_radio10', 'map_bss_type');
				uci.unset('wireless', 'default_radio10', 'a4_enable');

				// delete Backhaul AP for 5GHz
				uci.unset('wireless', 'default_radio01', 'multi_ap');
				uci.unset('wireless', 'default_radio01', 'map_bss_type');
				uci.unset('wireless', 'default_radio01', 'a4_enable');

				// delete Backhaul AP for 2.4GHz
				uci.unset('wireless', 'default_radio11', 'multi_ap');
				uci.unset('wireless', 'default_radio11', 'map_bss_type');
				uci.unset('wireless', 'default_radio11', 'a4_enable');

				if (deviceBand == '5g')
					set5G(value);
				else if (deviceBand == '2g')
					set2G(value);
				// remove /tmp/topology_json file(comment it. Because driver will do this)
				//fs.remove('/tmp/topology_json');
			}
		};

		o = s.option(form.ListValue, 'configured_band', _('Backhaul AP'));
		o.value('5g', _('radio0(5GHz)'));
		o.value('2g', _('radio1(2.4GHz)'));
		o.depends('map_role', '1');
		o.validate = function(section_id, value) {
			if (value == '5g')
				deviceBand = '5g';
			else if (value == '2g')
				deviceBand = '2g';

			return true;
		};
		o.write = function(section_id, value) {
			var map_role = uci.get('rtkmap', 'map', 'map_role');
			var ssid = '';
			var key = '';

			if (value == '5g') {
				uci.set('rtkmap', section_id, 'configured_band', '5g');
				set5G(map_role);
			}
			else if (value == '2g') {
				uci.set('rtkmap', section_id, 'configured_band', '2g');
				set2G(map_role);
			}
		};

		o = s.option(form.Button, 'wps_trigger', _('WPS Trigger'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Start PBC');
		o.onclick = this.handleWps;
		o.depends('map_role', '1');

		return m.render();
	}
});
