'use strict';
'require view';
'require uci';
'require poll';
'require rpc';
'require rtk.network as rtkNetwork';

var callLuciRpcDeviceStats = rpc.declare({
	object: 'luci-rpc',
	method: 'getNetworkDevices',
	expect: { '': {} }
});

var pollInterval = 3;

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('network')
		]);
	},

	updateCounter: function(devobj) {
		var rows = [];
		var netdev = Object.entries(devobj).sort();

		for (var i = 0; i < netdev.length; i++)
		{
			var c  = netdev[i][1];
			var dispName;

			if (c.name == 'eth0' || c.name == 'nas0' || c.name == 'lo')
				continue;
			if (c.flags.up == false)
				continue;
			if ((dispName = rtkNetwork.getNetdevDisplay(c.name)) == null)
				continue;

			dispName += ' (%s)'.format(c.name);
			rows.push([
				dispName,
				c.stats.rx_packets, c.stats.rx_errors, c.stats.rx_dropped,
				c.stats.tx_packets, c.stats.tx_errors, c.stats.tx_dropped
			]);
		}

		cbi_update_table('#counters', rows, E('em', _('No information available')));
	},

	pollData: function() {
		poll.add(L.bind(function() {
			var tasks = [
				L.resolveDefault(callLuciRpcDeviceStats(), [])
			];

			return Promise.all(tasks).then(L.bind(function(datasets) {
				this.updateCounter(datasets[0]);

			}, this));
		}, this), pollInterval);
	},

	render: function() {
		var v = E([], [
			E('div', { 'class': 'cbi-section-node' }, [
				E('table', { 'class': 'table', 'id': 'counters' }, [
					E('tr', { 'class': 'tr table-titles' }, [
						E('th', { 'class': 'th col-2 hide-xs' }, [ _('Interface') ]),
						E('th', { 'class': 'th col-2' }, [ _('RX pkt') ]),
						E('th', { 'class': 'th col-7' }, [ _('RX err') ]),
						E('th', { 'class': 'th col-7' }, [ _('RX drop') ]),
						E('th', { 'class': 'th col-4' }, [ _('TX pkt') ]),
						E('th', { 'class': 'th col-4' }, [ _('TX err') ]),
						E('th', { 'class': 'th col-4' }, [ _('TX drop') ])
					]),
					E('tr', { 'class': 'tr placeholder' }, [
						E('td', { 'class': 'td' }, [
							E('em', {}, [ _('Collecting data...') ])
						])
					])
				])
			])
		]);

		this.pollData();

		return v;
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
