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
		var out = document.querySelector('.command-output');
			    out.style.display = '';

		for (var i = 0; i < buttons.length; i++)
			buttons[i].setAttribute('disabled', 'true');

		return fs.exec(exec, args).then(function(res) {
			dom.content(out, [ res.stdout || '', res.stderr || '' ]);
		}).catch(function(err) {
			//ui.addNotification(null, E('p', [ err ]))
			return fs.exec_direct(exec, args).then(function(res1) {
				return dom.content(out, res1);
			});
		}).finally(function() {
			for (var i = 0; i < buttons.length; i++)
			buttons[i].removeAttribute('disabled');
		});
	},

	handleTraceroute: function(ev, cmd) {
		var exec = 'traceroute6',
		    //addr = ev.currentTarget.parentNode.previousSibling.value,
			addr= document.getElementById('host').value,
			iface= document.getElementById('wandev').value,
			probes= document.getElementById('probes').value,
			timeout= document.getElementById('timeout').value,
			datasize= document.getElementById('datasize').value,
			hopcount= document.getElementById('hopcount').value;
		var args;

		args = (iface != 'Any') ?   [ '-q', probes, '-i', iface, '-w', timeout, '-n', '-m', hopcount, addr, datasize] :
									[ '-q', probes, '-w', timeout, '-n', '-m', hopcount, addr, datasize];

		return this.handleCommand(exec, args);
	},

	load: function() {
		return Promise.all([
			rtkNetwork.getNetworkByIPMode("IPv6", 1)
		]);
	},

	render: function(res) {
		var route_host = uci.get('luci', 'diag', 'route') || 'openwrt.org';
		var rtkwans = res[0];
		var table = E('select', { 'class': 'cbi-input-select', 'id': 'wandev' });

		table.appendChild(E('option', { 'value': 'Any' }, 'Any'));

		for (var j=0;j<rtkwans.length;j++){
			var found = 0;
			for(var k=0;k<rtkwans[j].networks.length;k++){
				var ip6addrs=rtkwans[j].networks[k].getIP6Addrs();
				if(ip6addrs.length){
					found=1;
					break;
				}
			}

			if(found){
				table.appendChild(E('option', { 'value': rtkwans[j].l3dev }, rtkwans[j].sid));
			}
		}
				
		return E([], [
			E('h2', {}, [ _('Traceroute6 Diagnostics') ]),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Host Address')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id': 'host',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': route_host
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Number Of Tries')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id' : 'probes',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': '3'
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Time Out')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id' : 'timeout',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': '5'
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Data Size')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id' : 'datasize',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': '72'
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Max HopCount')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id' : 'hopcount',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': '30'
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'WAN Interface')
					]),
					table
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, [
						E('span', { 'class': 'diag-action' }, [
							E('button', {
								'class': 'cbi-button cbi-button-action',
								'click': ui.createHandlerFn(this, 'handleTraceroute')
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
