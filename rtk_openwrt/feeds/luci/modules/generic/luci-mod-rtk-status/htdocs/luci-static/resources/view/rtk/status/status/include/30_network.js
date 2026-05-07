'use strict';
'require baseclass';
'require rtk.network as rtkNetwork';
'require dom';

function renderbox(rtk, ipv6, both) {
	if(rtk['config']['rtk_chmode'] == 'Bridged'){
		return E('div', { class: 'ifacebox' }, [
			E('div', { class: 'ifacebox-head center active' },
				E('strong', rtk['config']['.name'])),
			E('div', { class: 'ifacebox-body left' }, [
				L.itemlist(E('span'), [
					_('Protocol'), rtk['config']['rtk_chmode'] || E('em', _('Not connected')),
				])
			])
		]);
	}else{
		var ifc = (ipv6 && rtk.networks.length > 1) ? rtk.networks[1] : rtk.networks[0];
		var dev = (ifc && ifc.getL3Device()),
			active = (dev && ifc && ifc.getProtocol() != 'none'),
			addrs = (ipv6 ? ifc && ifc.getIP6Addrs() : ifc && ifc.getIPAddrs()) || [],
			dnssrv = (ipv6 ? ifc && ifc.getDNS6Addrs() : ifc && ifc.getDNSAddrs()) || [],
			expires = (ipv6 ? null : ifc && ifc.getExpiry()),
			uptime = (ifc && ifc.getUptime());

		if (both) {
			var ifc6 = rtk.networks[1];
			var addrs6 = ifc6.getIP6Addrs() || [],
				dnssrv6 = ifc6.getDNS6Addrs() || [],
				expires6 = null;

			return E('div', { class: 'ifacebox' }, [
				E('div', { class: 'ifacebox-head center ' + (active ? 'active' : '') },
					E('strong', rtk['config']['.name'])),
				E('div', { class: 'ifacebox-body left' }, [
					L.itemlist(E('span'), [
						//_('Protocol'), ifc.getI18n() || E('em', _('Not connected')),
						_('Protocol'), rtk['config']['rtk_chmode'] || E('em', _('Not connected')),
						_('Prefix Delegated'), ipv6 ? ifc.getIP6Prefix() : null,
						_('Address'), addrs[0],
						_('Address'), addrs[1],
						_('Address'), addrs[2],
						_('Address'), addrs[3],
						_('Address'), addrs[4],
						_('Address'), addrs[5],
						_('Address'), addrs[6],
						_('Address'), addrs[7],
						_('Address'), addrs[8],
						_('Address'), addrs[9],
						_('Gateway'), (uptime > 0) ? (ipv6 ? (ifc.getGateway6Addr() || '::') : (ifc.getGatewayAddr() || '0.0.0.0')) : null,
						_('DNS') + ' 1', dnssrv[0],
						_('DNS') + ' 2', dnssrv[1],
						_('DNS') + ' 3', dnssrv[2],
						_('DNS') + ' 4', dnssrv[3],
						_('DNS') + ' 5', dnssrv[4],
						_('Expires'), (expires != null && expires > -1) ? '%t'.format(expires) : null,
						_('Status'), (uptime > 0) ? '%s(%t)'.format(_('Connected'), uptime) : _('Disconnect'),
					]),
						L.itemlist(E('span'), [
							//_('Protocol'), ifc.getI18n() || E('em', _('Not connected')),
							//_('Protocol'), rtk['config']['rtk_chmode'] || E('em', _('Not connected')),
							_(''), '',
							_('Prefix Delegated'), ifc6.getIP6Prefix(),
							_('Address'), addrs6[0],
							_('Address'), addrs6[1],
							_('Address'), addrs6[2],
							_('Address'), addrs6[3],
							_('Address'), addrs6[4],
							_('Address'), addrs6[5],
							_('Address'), addrs6[6],
							_('Address'), addrs6[7],
							_('Address'), addrs6[8],
							_('Address'), addrs6[9],
							_('Gateway'), (uptime > 0) ? (ifc6.getGateway6Addr() || '::') : null,
							_('DNS') + ' 1', dnssrv6[0],
							_('DNS') + ' 2', dnssrv6[1],
							_('DNS') + ' 3', dnssrv6[2],
							_('DNS') + ' 4', dnssrv6[3],
							_('DNS') + ' 5', dnssrv6[4],
							_('Expires'), (expires != null && expires > -1) ? '%t'.format(expires) : null,
							_('Status'), (uptime > 0) ? '%s(%t)'.format(_('Connected'), uptime) : _('Disconnect'),
						]),
						E('div', {}, renderBadge(
							L.resource('icons/%s.png').format(dev ? dev.getType() : 'ethernet_disabled'), null,
							_('Device'), dev ? dev.getI18n() : null,
							_('MAC address'), (uptime > 0) ? dev.getMAC() : '-'
						))
					])
				]);
		} else {
			return E('div', { class: 'ifacebox' }, [
				E('div', { class: 'ifacebox-head center ' + (active ? 'active' : '') },
					E('strong', rtk['config']['.name'])),
				E('div', { class: 'ifacebox-body left' }, [
					L.itemlist(E('span'), [
						//_('Protocol'), ifc.getI18n() || E('em', _('Not connected')),
						_('Protocol'), rtk['config']['rtk_chmode'] || E('em', _('Not connected')),
						_('Prefix Delegated'), ipv6 ? ifc.getIP6Prefix() : null,
						_('Address'), addrs[0],
						_('Address'), addrs[1],
						_('Address'), addrs[2],
						_('Address'), addrs[3],
						_('Address'), addrs[4],
						_('Address'), addrs[5],
						_('Address'), addrs[6],
						_('Address'), addrs[7],
						_('Address'), addrs[8],
						_('Address'), addrs[9],
						_('Gateway'), (uptime > 0) ? (ipv6 ? (ifc.getGateway6Addr() || '::') : (ifc.getGatewayAddr() || '0.0.0.0')) : null,
						_('DNS') + ' 1', dnssrv[0],
						_('DNS') + ' 2', dnssrv[1],
						_('DNS') + ' 3', dnssrv[2],
						_('DNS') + ' 4', dnssrv[3],
						_('DNS') + ' 5', dnssrv[4],
						_('Expires'), (expires != null && expires > -1) ? '%t'.format(expires) : null,
						_('Status'), (uptime > 0) ? '%s(%t)'.format(_('Connected'), uptime) : _('Disconnect'),
					]),
					E('div', {}, renderBadge(
						L.resource('icons/%s.png').format(dev ? dev.getType() : 'ethernet_disabled'), null,
						_('Device'), dev ? dev.getI18n() : '-',
						_('MAC address'), (uptime > 0) ? dev.getMAC() : '-'
					))
				])
			]);
		}
	}
}

return baseclass.extend({
	title: _('WAN Configuration'),

	load: function() {
		return Promise.all([
			rtkNetwork.getNetworkByIPMode('IPv4'),
			rtkNetwork.getNetworkByIPMode('IPv6')
		]);
	},

	render: function(data) {
		var rtkwans = data[0];
		var rtkwansv6 = data[1];
		var both = false;
		var netstatus = E('div', { 'class': 'network-status-table' });
		var netstatusv6 = E('div', { 'class': 'network-status-table' });

		// display interface : bridge, ipv4, ipv4 and ipv6.
		for (var i = 0; i < rtkwans.length; i++){
			var rtkwan = rtkwans[i];
			both = false;
			if(rtkwan['config']['rtk_chmode'] == 'Bridged' || rtkwan['config']['rtk_ipmode'].indexOf('IPv4') != -1){
				for (var j = 0; j < rtkwansv6.length; j++){
					var rtkwanv6 = rtkwansv6[j];
					if(rtkwanv6['config']['rtk_chmode'] != 'Bridged' && rtkwanv6['config']['rtk_ipmode'].indexOf('IPv6') != -1 &&  (rtkwan['config']['.name'] ==  rtkwanv6['config']['.name'])){
						both = true;
						break;
					}
				}

				netstatus.appendChild(renderbox(rtkwan, false, both));
			}
		}

		// display interface is ipv6 only
		for (var i = 0; i < rtkwansv6.length; i++){
			var rtkwanv6 = rtkwansv6[i];
			both = false;
			if(rtkwanv6['config']['rtk_chmode'] != 'Bridged' && rtkwanv6['config']['rtk_ipmode'].indexOf('IPv6') != -1){
				for (var j = 0; j < rtkwans.length; j++){
					var rtkwan = rtkwans[j];
					if(rtkwan['config']['rtk_chmode'] != 'Bridged' && rtkwan['config']['rtk_ipmode'].indexOf('IPv4') != -1 &&  (rtkwan['config']['.name'] ==  rtkwanv6['config']['.name'])){
						both = true;
						break;
					}
				}
				if (both == false) // ipv6 only
					netstatusv6.appendChild(renderbox(rtkwanv6, true, false));
			}
		}

		return E([
			netstatus,
			netstatusv6
		]);
	}
});
