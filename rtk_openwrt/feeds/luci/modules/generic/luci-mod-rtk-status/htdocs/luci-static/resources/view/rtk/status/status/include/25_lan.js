'use strict';
'require baseclass';
'require fs';
'require network';
'require uci';
'require rpc';

var callGetLANPDs = rpc.declare({
        object: 'network.interface.lan',
        method: 'status',
        expect: { 'ipv6-prefix-assignment': [] }
});

return baseclass.extend({
	title: _('LAN Configuration'),

	load: function() {
		return Promise.all([
			network.getDevices(),
			L.resolveDefault(callGetLANPDs(), [])
		]);
	},

	render: function(data) {
		var fields = [];
		var devices = data[0];
		var lanDev = devices ? devices.filter(function(n){ return n.getName() == 'br-lan'})[0] : null;
		if(!lanDev) return;
		var addr = lanDev.getIPAddrs();
		var dhcpv4 = uci.get('dhcp', 'lan', 'dhcpv4');
		var v6addr = lanDev.getIP6Addrs();
		var lla=[];
		var ula=[];
		var multiAddr=[];
		var gla=[];
		var haveMac = false;
		var PDs=data[1];

		for (var j=0; j<v6addr.length; j++){
			var tmp=v6addr[j].split("/")[0].split(':');
			if(tmp.length){
				if((parseInt(tmp[0], 16) & 0xffc0) == 0xfe80){
					lla.push(v6addr[j]);
				}else if((parseInt(tmp[0], 16) & 0xfe00) == 0xfc00){
					ula.push(v6addr[j]);
				}else if((parseInt(tmp[0], 16) & 0xff00) == 0xff00){
					multiAddr.push(v6addr[j]);
				}else{
					gla.push(v6addr[j]);
				}
			}
		}

		var table = E('table', { 'class': 'table' });
		// display LAN IPv4 Addr
		if(lanDev.isUp()){
			haveMac = true;
			fields = [
				_('IPv4 Address'),     addr[0] ? "%s".format(addr[0].split("/")[0]) : null,
				_('Subnet Mask'),      addr[0] ? "%s".format(addr[0].split("/")[1]) : null,
				_('DHCP Server'),      dhcpv4=="server" ? "Enable" : "Disabled",
				_('Mac Address'),      lanDev.getMAC(),
			];

			for (var i = 0; i < fields.length; i += 2) {
				if(fields[i + 1] != null){
					table.appendChild(E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
						E('td', { 'class': 'td left' }, [ fields[i + 1] ])
					]));
				}
			}
		}else{
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left' }, [ 'LAN interface is Down' ]),
			]));
		}

		// display IPv6 LAN Addr
		if(lanDev.isUp()){
			fields = [
				_('IPv6 Address'),     gla[0] ? "%s/%s".format(gla[0].split("/")[0], network.maskToPrefix(gla[0].split("/")[1], true)) : null,
				_('IPv6 Address'),     gla[1] ? "%s/%s".format(gla[1].split("/")[0], network.maskToPrefix(gla[1].split("/")[1], true)) : null,
				_('IPv6 Address'),     gla[2] ? "%s/%s".format(gla[2].split("/")[0], network.maskToPrefix(gla[2].split("/")[1], true)) : null,
				_('IPv6 Address'),     gla[3] ? "%s/%s".format(gla[3].split("/")[0], network.maskToPrefix(gla[3].split("/")[1], true)) : null,
				_('IPv6 Address'),     gla[4] ? "%s/%s".format(gla[4].split("/")[0], network.maskToPrefix(gla[4].split("/")[1], true)) : null,
				_('IPv6 Address'),     gla[5] ? "%s/%s".format(gla[5].split("/")[0], network.maskToPrefix(gla[5].split("/")[1], true)) : null,
				_('IPv6 Link-local Address'),     lla[0] ? "%s/%s".format(lla[0].split("/")[0], network.maskToPrefix(lla[0].split("/")[1], true)) : null,
				_('IPv6 Unique-local Address'),   ula[0] ? "%s/%s".format(ula[0].split("/")[0], network.maskToPrefix(ula[0].split("/")[1], true)) : null,
				_('Mac Address'),      !haveMac ? lanDev.getMAC() : null,
			];

			for (var i = 0; i < fields.length; i += 2) {
				if(fields[i + 1] != null){
					table.appendChild(E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
						E('td', { 'class': 'td left' }, [ fields[i + 1] ])
					]));
				}
			}
		}else{
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left' }, [ 'LAN interface is Down' ]),
			]));
		}

		// display prefix which is applied to LAN
		if(PDs.length){
			for (var i = 0; i < PDs.length; i++) {
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '33%' }, [ _('Prefix') ]),
					E('td', { 'class': 'td left' }, [ '%s/%d'.format(PDs[i].address, PDs[i].mask) ])
				]));
			}
		}else{
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left' }, [ _('No Active Prefix') ]),
			]));
		}

		return table;
	}
});
