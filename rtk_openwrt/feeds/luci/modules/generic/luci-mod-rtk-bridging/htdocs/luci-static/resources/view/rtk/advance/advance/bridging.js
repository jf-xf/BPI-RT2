'use strict';
'require view';
'require form';
'require uci';
'require fs';
'require ui';
'require dom';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('network')
		]);
	},

	handleShowmacs: function(ev) {
		return fs.exec('/usr/sbin/brctl', [ 'showmacs', 'br-lan' ]).then(function(res) {
			var loglines = res.stdout.trim().split(/\n/);
			var message = ui.showModal('', '');
			var table = E('table', { 'class': 'table' , 'id': 'macs'});
			var data0=loglines[0].trim().split(/\t/);
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td center' }, [ data0[0] ]),
					E('td', { 'class': 'td center' }, [ data0[1] ]),
					E('td', { 'class': 'td center' }, [ data0[3] ]),
					E('td', { 'class': 'td center' }, [ data0[2] ]),
					E('td', { 'class': 'td center' }, [ data0[4] ])
				]));

				for (var i = 1; i < loglines.length; i++) {
					var data=loglines[i].trim().split(/\t/);
					table.appendChild(E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td center' }, [ data[0] ]),
						E('td', { 'class': 'td center' }, [ data[1] ]),
						E('td', { 'class': 'td center' }, [ data[2] ]),
						E('td', { 'class': 'td center' }, [ '' ]),
						E('td', { 'class': 'td center' }, [ data[4] ])
					]));
				}

			var form = E([], [
				E('h4', {}, [ _('Bridge Forwarding Database') ]),
				E('div', { 'id': 'content_macs' }, [
					table,
					E('button', {
						'class': 'cbi-button cbi-button-action',
						'click': ui.createHandlerFn(this, function(ev) {
							var out = document.querySelector('[id="macs"]');

							fs.exec('/usr/sbin/brctl', [ 'showmacs', 'br-lan' ]).then(function(res) {
								loglines = res.stdout.trim().split(/\n/);
								console.log(loglines.length);

								var table2 = E('table', { 'class': 'table' , 'id': 'macs'});
								var data20=loglines[0].trim().split(/\t/);
									table2.appendChild(E('tr', { 'class': 'tr' }, [
										E('td', { 'class': 'td center' }, [ data20[0] ]),
										E('td', { 'class': 'td center' }, [ data20[1] ]),
										E('td', { 'class': 'td center' }, [ data20[3] ]),
										E('td', { 'class': 'td center' }, [ data20[2] ]),
										E('td', { 'class': 'td center' }, [ data20[4] ])
									]));
								for (var i = 1; i < loglines.length; i++) {
									var data2=loglines[i].trim().split(/\t/);
										table2.appendChild(E('tr', { 'class': 'tr' }, [
											E('td', { 'class': 'td center' }, [data2[0] ]),
											E('td', { 'class': 'td center' }, [data2[1] ]),
											E('td', { 'class': 'td center' }, [data2[2] ]),
											E('td', { 'class': 'td center' }, ['' ]),
											E('td', { 'class': 'td center' },[ data2[4] ])
										]));
								}
								dom.content(out, [ table2  ]);
								return;
							})
						})
					}, _('Refresh')),' ',
					E('button', {
						'class': 'btn',
						'click': ui.hideModal
					}, _('Close')), ' '
				])				
			]);
			dom.content(message, [ form ]);
			return;
		}).catch(function(err) {
			ui.addNotification(null, E('p', {}, _('Unable to load log data: ' + err.message)));
			return '';
		});
	},

	render: function(load_result) {
		var m, s, o;

		m = new form.Map('network', 'Bridging Configuration', 'This page is used to configure the bridge parameters. Here you may change the setting or view some information on the bridge and its attached ports.');

		s = m.section(form.TypedSection, 'device', _(''));
		s.anonymous = true;

		s.filter = function(section_id) {
			return (uci.get('network', section_id, 'name') == 'br-lan');
		};

		o = s.option(form.Value, 'ageing_time', _('Ageing Time'), _('Timeout in seconds for learned MAC addresses in the forwarding database'));
		o.default = '7200';
		
		o = s.option(form.Flag, 'stp', _('802.1d Spanning Tree'));
		o.default = o.disabled;
		o.editable = true;

		o = s.option(form.Button, 'show_macs', _('Bridge Forwarding Database'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Show Macs');
		o.onclick = this.handleShowmacs;

		return m.render();
	}
});

