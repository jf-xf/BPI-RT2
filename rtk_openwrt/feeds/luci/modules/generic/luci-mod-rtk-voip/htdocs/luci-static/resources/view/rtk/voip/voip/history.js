'use strict';
'require view';
'require uci';
'require fs';
'require rpc';

var f_voip_uci = 'rtkvoip';

var callGetLocaltime;

callGetLocaltime = rpc.declare({
	object: 'system',
	method: 'info',
	expect: { localtime: 0 }
});

return view.extend({
	load: function() {
		return Promise.all([
			fs.lines('/tmp/call_log.csv'),
			callGetLocaltime()
		]);
	},

	render: function(data) {
		var hists = data[0];
		var localtime = data[1];
		var table = E('table', { 'class': 'table' });

		table.appendChild(
			E('th', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '10%' }, [ _('No.') ]),
				E('td', { 'class': 'td left', 'width': '10%' }, [ _('Status') ]),
				E('td', { 'class': 'td left', 'width': '15%' }, [ _('From') ]),
				E('td', { 'class': 'td left', 'width': '15%' }, [ _('To') ]),
				E('td', { 'class': 'td left', 'width': '10%' }, [ _('Type') ]),
				E('td', { 'class': 'td left', 'width': '10%' }, [ _('Duration') ]),
				E('td', { 'class': 'td left'                 }, [ _('DateTime') ])
			])
		);

		for (var i = 0; i < hists.length; i++) {
			var items = hists[i].trim().split(/,/);
			var num_no = parseInt(items[0]) + 1;
			var hour, min, sec;
			var num_duration, str_duration;

			if (items[6].length > 0)
				num_duration = parseInt(items[6]) - parseInt(items[5]);
			else
				num_duration = localtime - parseInt(items[5]);

			sec  = parseInt(num_duration % 60);
			min  = parseInt(num_duration / 60 % 60);
			hour = parseInt(num_duration / 3600);

			str_duration = (hour < 10 ? '0' : '') + hour.toString() + ':' +
							(min < 10 ? '0' : '') + min.toString() + ':' +
							(sec < 10 ? '0' : '') + sec.toString();

			if (items.length < 8) break;
			table.appendChild(
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '10%' }, [ num_no ]),
					E('td', { 'class': 'td left', 'width': '10%' }, [ _(items[1]) ]),
					E('td', { 'class': 'td left', 'width': '15%' }, [ _(items[2]) ]),
					E('td', { 'class': 'td left', 'width': '15%' }, [ _(items[3]) ]),
					E('td', { 'class': 'td left', 'width': '10%' }, [ _(items[4]) ]),
					E('td', { 'class': 'td left', 'width': '10%' }, [ _(str_duration) ]),
					E('td', { 'class': 'td left'                 }, [ _(items[7]) ])
				])
			);
		};

		return E([], [
			E('h2', {}, [ _('Call History') ]),
			E('div', { 'class': 'cbi-map-descr' }, [ _('This page shows the VoIP call log') ]),
			table
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
