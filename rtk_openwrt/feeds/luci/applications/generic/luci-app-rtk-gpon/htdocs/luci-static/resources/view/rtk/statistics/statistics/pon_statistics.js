'use strict';
'require view';
'require dom';
'require ui';
'require rpc';
'require poll';

var callGetPonStatistics = rpc.declare({
	object: 'rtk-rpc',
	method: 'get_pon_statistics',
	expect: { '': {} }
});

var callResetPonStatistics = rpc.declare({
	object: 'rtk-rpc',
	method: 'reset_pon_statistics',
	expect: { '': {} }
});

function renderPonStatistics(result) {
	var info = result;

	var fields = [
		_('Bytes Sent'),					info.bytes_sent,
		_('Bytes Received'),				info.bytes_received,
		_('Packets Sent'),					info.packets_sent,
		_('Packets Received'), 				info.packets_received,
		_('Unicast Packets Sent'),			info.unicast_packets_sent,
		_('Unicast Packets Received'),		info.unicast_packets_received,
		_('Multicast Packets Sent'),		info.multicast_packets_sent,
		_('Multicast Packets Received'),	info.multicast_packets_received,
		_('Broadcast Packets Sent'),		info.broadcast_packets_sent,
		_('Broadcast Packets Received'),	info.broadcast_packets_received,
		_('FEC Errors'),					info.fec_errors,
		_('HEC Errors'),					info.hec_errors,
		_('Packets Dropped'),				info.packets_dropped,
		_('Pause Packets Sent'),			info.pause_packets_sent,
		_('Pause Packets Received'),		info.pause_packets_received
	];

	var table = E('div', { 'class': 'table' });

	for (var i = 0; i < fields.length; i += 2) {
		table.appendChild(E('div', { 'class': 'tr' }, [
			E('div', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
			E('div', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
		]));
	}
	var map = document.querySelector('.cbi-map');
	var content = map.querySelector('[id="pon-statistics"]');
	dom.content(content, function() {
		return E([
			table
		]);
	});
}

return view.extend({
	load: function() {
		return Promise.all([
					L.resolveDefault(callGetPonStatistics())
                ]);
	},
	render: function(data) {
		poll.add(function() {
			return callGetPonStatistics().then(function(result) {
				renderPonStatistics(result);
			});
		});

		return E('div', { class: 'cbi-map' }, [
			E('h2', [ _('PON Statistics') ]),
			E('div', { class: 'cbi-map-descr' }, [_('This page shows the current statistics of PON.')]),
			E('div', { class: 'cbi-section' }, [
				E('div', { 'id': 'pon-statistics' }, [
					E('em', { 'class': 'spinning' }, [ _('Collecting data ...') ])
				])
			])
		]);
	},
	handleSave: null,
	handleSaveApply: null,
	handleReset: function(ev) {
		var message = ui.showModal('', '');
		dom.content(message, E('p', _('Reset Statistics.')));
		return callResetPonStatistics().then(function() {
			callGetPonStatistics().then(function(result) {
				renderPonStatistics(result);
			});
			window.setTimeout(function() {ui.hideModal();}, 2000);
		});
	}
});