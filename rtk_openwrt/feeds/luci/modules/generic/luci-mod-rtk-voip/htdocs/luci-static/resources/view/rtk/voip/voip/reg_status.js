'use strict';
'require view';
'require uci';
'require form';
'require fs';

var f_voip_uci = 'rtkvoip';

function transform_reg_status(status)
{
	switch (status)
	{
		case '0':
			return 'Not Registered';
		case '1':
			return 'Registered';
		case '2':
		case '5':
			return 'Registering';
		case '3':
			return 'Register Failed';
		case '4':
			return 'VoIP Restart';
		default:
			return 'Error';
	}
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load(f_voip_uci),
			fs.lines('/tmp/voipstatus')
		]);
	},

	render: function(data) {
		var ports = uci.sections(f_voip_uci, 'voipCfgPortParam');
		var table = E('table', { 'class': 'table' });

		table.appendChild(
			E('th', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '20%' }, [ _('Port') ]),
				E('td', { 'class': 'td left', 'width': '40%' }, [ _('Number') ]),
				E('td', { 'class': 'td left'                 }, [ _('Status') ])
			])
		);

		const str_status = data[1][0];

		if (str_status !== undefined) {
			const array_status = str_status.split('');

			for (var i = 0; i < ports.length; i++) {
				var port = 'VOIP_PORT%d'.format(i);
				var proxy = 'VOIP_PORT%d_PROXY%d'.format(i, uci.get(f_voip_uci, port, 'DEFAULT_PROXY'));
				var number = uci.get(f_voip_uci, proxy, 'PROXY_NUMBER');
				var reg_status = transform_reg_status(array_status[i]);

				table.appendChild(
					E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td left', 'width': '20%' }, [ i+1 ]),
						E('td', { 'class': 'td left', 'width': '40%' }, [ number ? number : "" ]),
						E('td', { 'class': 'td left'                 }, [ reg_status ])
					])
				);
			}
		}
		else {
			table.appendChild(
				E('tr', { 'class': 'tr' }, [ _('Error reading status file !') ])
			);
		}

		return E([], [
			E('h2', {}, [ _('Register Status') ]),
			E('div', { 'class': 'cbi-map-descr' }, [ _('This page shows the register status of port') ]),
			table
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
