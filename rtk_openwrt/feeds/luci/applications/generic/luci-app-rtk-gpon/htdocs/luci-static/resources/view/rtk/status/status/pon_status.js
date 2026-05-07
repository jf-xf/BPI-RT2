'use strict';
'require view';
'require dom';
'require rpc';
'require poll';

var callGetPonStatus = rpc.declare({
	object: 'rtk-rpc',
	method: 'get_pon_status',
	expect: { '': {} }
});

function renderPonStatus(result) {
	var info = result;

	var fields = [
		_('Vendor Name'),			info.vender_name,
		_('Part Number'),			info.part_number,
		_('Temperature'),			info.temperature,
		_('Voltage'),				info.voltage,
		_('Tx Power'),				info.tx_power,
		_('Rx Power'), 				info.rx_power,
		_('Bias Current'),			info.bias_current
	];

	var table = E('div', { 'class': 'table' });

	for (var i = 0; i < fields.length; i += 2) {
		table.appendChild(E('div', { 'class': 'tr' }, [
			E('div', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
			E('div', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
		]));
	}
	var map = document.querySelector('.cbi-map');
	var content = map.querySelector('[id="pon-transceiver"]');
	dom.content(content, function() {
		return E([
			table
		]);
	});
}

function renderGponStatus(result) {
	var info = result;

	var fields = [
		_('ONU State'),				info.onu_state,
		_('ONU ID'),				info.onu_id,
		_('LOID Status'),			info.loid_status
	];

	var table = E('div', { 'class': 'table' });

	for (var i = 0; i < fields.length; i += 2) {
		table.appendChild(E('div', { 'class': 'tr' }, [
			E('div', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
			E('div', { 'class': 'td left' }, [ (fields[i + 1] != null) ? fields[i + 1] : '?' ])
		]));
	}

	var map = document.querySelector('.cbi-map');
	var content = map.querySelector('[id="onu-state"]');
	dom.content(content, function() {
		return E([
			table
		]);
	});
}

return view.extend({
	load: function() {
		return Promise.all([
					L.resolveDefault(callGetPonStatus())
                ]);
	},
	render: function(data) {
		poll.add(function() {
			return callGetPonStatus().then(function(result) {
				renderPonStatus(result);
				renderGponStatus(result);
			});
		});

		return E('div', { class: 'cbi-map' }, [
			E('h2', [ _('PON Status') ]),
			E('div', { class: 'cbi-map-descr' }, [_('This page shows the current system status of PON.')]),
			E('div', { class: 'cbi-section' }, [
				E('h3', [ _('PON Status') ]),
				E('div', { 'id': 'pon-transceiver' }, [
					E('em', { 'class': 'spinning' }, [ _('Collecting data ...') ])
				])
			]),
			E('div', { class: 'cbi-section' }, [
				E('h3', [ _('GPON Status') ]),
				E('div', { 'id': 'onu-state' }, [
					E('em', { 'class': 'spinning' }, [ _('Collecting data ...') ])
				])
			])
		]);
	},
	handleSave: null,
	handleSaveApply: null,
	handleReset: null
});