'use strict';
'require view';
'require dom';
'require fs';
'require ui';
'require uci';
'require rtk.network as rtkNetwork';

return view.extend({
	handleCommand: function(exec, args) {
		var buttons = document.querySelectorAll('.diag-action > .cbi-button');

		for (var i = 0; i < buttons.length; i++)
			buttons[i].setAttribute('disabled', 'true');

		return fs.exec(exec, args).then(function(res) {
			var out = document.querySelector('.command-output');
			    out.style.display = '';

			dom.content(out, [ res.stdout || '', res.stderr || '' ]);
		}).catch(function(err) {
			ui.addNotification(null, E('p', [ err ]))
		}).finally(function() {
			for (var i = 0; i < buttons.length; i++)
				buttons[i].removeAttribute('disabled');
		});
	},

	handlePing: function(count, ev, cmd) {
		var iface= document.getElementById('wandev').value;
		var addr= document.getElementById('host').value;
		var exec = 'ping',
		    //addr = ev.currentTarget.parentNode.previousSibling.value,
			args = (iface != 'Any') ? [ '-4', '-c', '5', '-I', iface, '-W', '1', addr ] : [ '-4', '-c', '5', '-W', '1', addr ] ;

		return this.handleCommand(exec, args);
	},

	load: function() {
		return Promise.all([
			rtkNetwork.getNetworkByIPMode("IPv4", 1)
		]);
	},

	render: function(res) {
		var count,
			rtkwans = res[0],
			ping_host = uci.get('luci', 'diag', 'ping') || 'openwrt.org';
		var table = E('select', { 'class': 'cbi-input-select', 'id': 'wandev' });

		table.appendChild(E('option', { 'value': 'Any' }, 'Any'));

		for (var j=0;j<rtkwans.length;j++){
			var found = 0;
			for(var k=0;k<rtkwans[j].networks.length;k++){
				var ipaddrs=rtkwans[j].networks[k].getIPAddrs();
				if(ipaddrs.length){
					found=1;
					break;
				}
			}

			if(found){
				table.appendChild(E('option', { 'value': rtkwans[j].l3dev }, rtkwans[j].sid));
			}
		}

		return E([], [
			E('h2', {}, [ _('Ping Diagnostics') ]),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'WAN Interface')
					]),
					table
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Host Address')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id': 'host',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': ping_host
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, [
						E('span', { 'class': 'diag-action' }, [
							E('button', {
								'class': 'cbi-button cbi-button-action',
								'click': ui.createHandlerFn(this, 'handlePing')
							}, [ _('Start') ])
						])
					])
				])
			]),
			E('pre', { 'class': 'command-output', 'style': 'display:none' })
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
