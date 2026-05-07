'use strict';
'require view';
'require form';
'require fs';
'require uci';
'require ui';

var tableMain = E('table', { 'class': 'table' , 'id': 'main'});
var level = 0;
var which = [0];

function shownmain() {
	location.reload();
}

function showneigh(level, which) {
	return Promise.all([
		L.resolveDefault(fs.read('/tmp/topology_json'), '')
	]).then(L.bind(function(data){
		var rawdata, jsondata;
		var table = E('table', { 'class': 'table' , 'id': 'info1'});

		//console.log(data[0]);
		if (data[0] != '') {
			rawdata = JSON.parse(data[0]);  // string change to object.
		}
		else {
			return E('div', { 'class': 'tr' }, [
					E('strong', 'There is no any Topology data.')
				]);
		}
		//console.log(jsondata.device_name);

		table.appendChild(E('tr', { 'class': 'tr' }, [
			E('td', { 'class': 'td center' }, _('MAC Address')),
			E('td', { 'class': 'td center' }, _('RSSI')),
			E('td', { 'class': 'td center' }, _('Connected Band')),
			E('td', { 'class': 'td center' }, _('Downlink')),
			E('td', { 'class': 'td center' },  _('UPlink'))
		]));

		jsondata = rawdata;
		//console.log(level);
		for (var i=1 ; i <= level; i++) {
			//console.log(which[i]);
			jsondata = jsondata.child_devices[which[i]];
		}

		for (var i = 0; i < jsondata.station_info.length ; i++) {
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td center' }, [ jsondata.station_info[i].station_mac ]),
				E('td', { 'class': 'td center' }, [ jsondata.station_info[i].station_rssi ]),
				E('td', { 'class': 'td center' }, [ jsondata.station_info[i].station_connected_band ]),
				E('td', { 'class': 'td center' }, [ jsondata.station_info[i].station_downlink]),
				E('td', { 'class': 'td center' }, [ jsondata.station_info[i].station_uplink ])
			]));
		}

		var table2 = E('table', { 'class': 'table' , 'id': 'neigh1'});
		table2.appendChild(E('tr', { 'class': 'tr' }, [
			E('td', { 'class': 'td center' }, _('MAC Address')),
			E('td', { 'class': 'td center' }, _('Name')),
			E('td', { 'class': 'td center' }, _('RSSI')),
			E('td', { 'class': 'td center' }, _(''))
		]));

		for (var i = 0; i < jsondata.neighbor_devices.length ; i++) {
			table2.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td center' }, [ jsondata.neighbor_devices[i].neighbor_mac ]),
				E('td', { 'class': 'td center' }, [ jsondata.neighbor_devices[i].neighbor_name ]),
				E('td', { 'class': 'td center' }, [ jsondata.neighbor_devices[i].neighbor_rssi ])
			]));
		}

		var md = ui.showModal(_('EasyMesh Device Details Table'), [
			E('div', { 'class': 'tr' }, [
				E('strong', 'Neighbor RSSI (excluding parent AP) :')
			]),
			table2,
			E('div', { 'class': 'tr' }, []),
			E('div', { 'class': 'tr' }, []),
			E('div', { 'class': 'tr' }, [
				E('strong', 'Station Info :')
			]),
			table,
			E('div', { 'class': 'left' }, [
				E('button', {
					'class': 'btn',
					'click': ui.createHandlerFn(this, showneigh, level, which)
				}, _('Refresh')), '    ',
				E('button', {
					'class': 'btn',
					'click': ui.hideModal
				}, _('Close'))
			])
		]);
	}, this));
}

function printOtherNeigh(jsonObject, tableMain, level, which) {
	var level_E = E([]);

	for (var i = 0; i < level ; i++) {
		level_E.appendChild(E('td', { 'class': 'td left' }, []));
	}

	tableMain.appendChild(E('tr', { 'class': 'tr' }, [
		level_E,
		E('td', { 'class': 'td left', 'width' : '20em'}, [ jsonObject.device_name ]),
		E('td', { 'class': 'td center', 'width' : '15em' }, [ jsonObject.mac_address ]),
		E('td', { 'class': 'td center', 'width' : '15em' }, [ jsonObject.ip_addr ]),
		E('button', {
			'class': 'cbi-button cbi-button-action important',
			'title': _('Show Device Detail'),
			'click': ui.createHandlerFn(this, showneigh, level, which)
		}, _('Show Details'))
	]));

	if (0 != jsonObject["child_devices"].length) {
		level = level+1;
		for (var i = 0; i < jsonObject["child_devices"].length ; i++) {
			which[level] = i;
			printOtherNeigh(jsonObject["child_devices"][i], tableMain, level, which);
		}
	}

}

return view.extend({
		load: function() {
			return Promise.all([
				L.resolveDefault(fs.read('/tmp/topology_json'), ''),
				uci.load('wireless')
			]);
		},

		render: function(data) {
			var jsondata;

			if (data[0] != '') {
				jsondata = JSON.parse(data[0]);  // string change to object.
			}
			else {
				return E('div', { 'class': 'tr' }, [
						E('strong', 'There is no any Topology data.')
					]);
			}

			//console.log(jsondata.device_name);
			printOtherNeigh(jsondata, tableMain, level, which);

			return E([], [
				E('h2', { 'class': 'tr' }, [ _('EasyMesh Network Topology') ]),
				E('p', { 'class': 'tr', 'style': 'color: #ABABAB;' }, [ _('This page display the topology of EasyMesh network\n\n') ]),
				E('div', { 'class': 'table' }, [
					E('div', { 'class': 'tr' }, [
						E('div', { 'class': 'td left' }, [
							E('strong', 'Network Topology :')
						])
					]),
					tableMain
				]),
				E('div', { 'class': 'tr', 'style': 'color: white;' }, ['white']),
				E('div', { 'class': 'left' }, [
					E('button', {
						'class': 'btn',
						'click': ui.createHandlerFn(this, shownmain)
					}, _('Refresh')),
				])
			]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
