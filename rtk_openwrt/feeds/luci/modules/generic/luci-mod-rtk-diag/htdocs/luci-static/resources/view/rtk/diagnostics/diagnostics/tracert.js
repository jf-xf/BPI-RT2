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
		var exec = 'traceroute',
		    //addr = ev.currentTarget.parentNode.previousSibling.value,
			addr= document.getElementById('host').value,
			proto= document.getElementById('proto').value,
			iface= document.getElementById('wandev').value,
			probes= document.getElementById('probes').value,
			timeout= document.getElementById('timeout').value,
			datasize= document.getElementById('datasize').value,
			dscp= document.getElementById('dscp').value,
			hopcount= document.getElementById('hopcount').value;
		var args, tos, tosStr;

		tos = Number(dscp);
		tos = ( tos & 0x3f ) << 2;
		//console.log(tos);
		tosStr = tos.toString();
		//console.log(tosStr);
		if (proto == 'icmp') {
			args = (iface != 'Any') ?   [ '-4', '-I', '-q', probes, '-i', iface, '-w', timeout, '-t', tosStr, '-n', '-m', hopcount, addr, datasize] :
										[ '-4', '-I', '-q', probes, '-w', timeout, '-t', tosStr, '-n', '-m', hopcount, addr, datasize];
		}
		else {
			args = (iface != 'Any') ?   [ '-4', '-q', probes, '-i', iface, '-w', timeout, '-t', tosStr, '-n', '-m', hopcount, addr, datasize] :
										[ '-4', '-q', probes, '-w', timeout, '-t', tosStr, '-n', '-m', hopcount, addr, datasize];
		}
		return this.handleCommand(exec, args);
	},

	load: function() {
		return Promise.all([
			rtkNetwork.getNetworkByIPMode("IPv4", 1)
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
			E('h2', {}, [ _('Traceroute Diagnostics') ]),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'Protocol')
					]),
					E('select', { 'class': 'cbi-input-select', 'id': 'proto' }, [
                            E('option', { 'value': 'icmp' }, 'ICMP'),
                            E('option', { 'value': 'udp' }, 'UDP')
                    ])
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
							'value': '56'
						})
					])
				]),
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'th left' }, [
						E('strong', 'DSCP')
					]),
					E('div', { 'class': 'td left' }, [
						E('input', {
							'id' : 'dscp',
							'style': 'margin:5px 0',
							'type': 'text',
							'value': '0'
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
