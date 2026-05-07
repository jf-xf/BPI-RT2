'use strict';
'require view';
'require uci';
'require form';
'require ui';
'require network';
'require rpc';
'require rtk.network as rtkNetwork';

var lanIntfSid='lan';

// get all device infomation, and use it create port list
var callGetDeviceStatus = rpc.declare({
	object: 'network.device',
	method: 'status',
	expect: { '': {} }
});

return view.extend({
	load: function(){
		return Promise.all([
			network.getNetworks()
		]);
	},

	render: function(result) {
		var m, s, o;
		var networks=result[0];
		m = new form.Map('network', _('Bridge Group Settings'));
		m.chain("wireless");

		s = m.section(form.GridSection, 'interface');
		s.anonymous = true;
		s.addremove = true;

		s.load=function(section_id){
			return Promise.all([
				network.getNetworks(),
			]).then(L.bind(function(data){
				networks = data[0];
			}, this));
		};

		s.filter = function(section_id) {
			var intf=networks.find(function(e){
				return e.sid == section_id;
			});
			var dev=intf?intf.getDevice():null;
			if(dev && dev.config.type === 'bridge')
				return true;

			return false;
		};

		s.renderRowActions = function(section_id) {
			var tdEl = this.super('renderRowActions', [ section_id, _('Edit') ]),
				disabled = (section_id==lanIntfSid)? true : false;

			tdEl.lastChild.childNodes[1].disabled = disabled;

			return tdEl;
		};

		s.handleAdd = function(ev, name){
			var m2 = new form.Map('network'),
				s2 = m2.section(form.NamedSection, '_new_');
			var name;

			s2.render = function() {
				return Promise.all([
					{},
					this.renderUCISection('_new_')
				]).then(this.renderContents.bind(this));
			};

			name = s2.option(form.Value, 'name', _('Name'));
			name.rmempty = false;
			name.datatype = 'uciname';
			name.placeholder = _('New Bridge name…');
			name.validate = function(section_id, value) {
				if (value == null || value == '')
					return _('The bridge name should not be empty');

				var intfs=uci.sections('network', 'interface');
				for(var i=0; intfs && i<intfs.length; i++){
					if(intfs[i]['.name'] == value){
						return _('The bridge name is already used');
					}
				}

				var devs=uci.sections('network', 'device');
				for(var i=0; devs && i<devs.length; i++){
					if(devs[i]['type'] != 'bridge')
						continue;

					if(devs[i]['name'] == value){
						return _('The bridge name is already used');
					}
				}

				if (value.length > 15)
					return _('The interface name is too long');

				return true;
			};

			m2.render().then(L.bind(function(nodes) {
				ui.showModal(_('Add new Bridge interface...'), [
					nodes,
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.hideModal
						}, _('Cancel')), ' ',

						E('button', {
							'class': 'cbi-button cbi-button-positive important',
							'click': ui.createHandlerFn(this, function(ev) {
								var nameval = name.isValid('_new_') ? name.formvalue('_new_') : null;
								if (nameval == null || nameval == '')
									return;

								return this.map.save(function() {
									var intf_sid = uci.add('network', 'interface', nameval);
									uci.set('network', intf_sid, "device", nameval);

									var d_sid = uci.add('network', 'device', 'D_%s'.format(nameval));
									uci.set('network', d_sid, "name", nameval);
									uci.set('network', d_sid, "type", 'bridge');

									m.children[0].addedSection = d_sid;

									ui.hideModal();
									ui.showModal(null, E('p', { 'class': 'spinning' }, [ _('Loading data…') ]));
								}).then(L.bind(network.getNetworks, network))
								.then(L.bind(m.children[0].renderMoreOptionsModal, m.children[0], nameval));
							})
						}, _('Create Bridge'))
					])
				], 'cbi-modal');

				nodes.querySelector('[id="%s"] input[type="text"]'.format(name.cbid('_new_'))).focus();
			}, this));
		};

		s.handleRemove = function(section_id, ev) {
			//all ports assign to lan
			var devIdx=0;
			// lookup all wifi-device
			var devs=uci.sections('wireless', 'wifi-device');
			var intfs=uci.sections('wireless', 'wifi-iface');
			for(var i=0;i<devs.length;i++){
				var ifIdx=0;
				for(var j=0;j<intfs.length;j++){
					// lookup all wifi-iface
					if (intfs[j]['device'] == devs[i]['.name']){
						if(intfs[j]['network']  == section_id){
							uci.set('wireless', intfs[j]['.name'], 'network', lanIntfSid);
						}

						ifIdx++;
					}
				}

				devIdx++;
			}

			var ports=uci.get('network', rtkNetwork.getDevSidByIntfSid(section_id), 'ports');
			ports=ports?ports:[];
			if(ports.length){
				var lan_sid=rtkNetwork.getDevSidByIntfSid(lanIntfSid);
				var lan_ports=uci.get('network', lan_sid, 'ports');
				lan_ports=lan_ports?lan_ports:[]
				uci.set('network', lan_sid, 'ports', lan_ports.concat(ports));
			}

			uci.remove('network', rtkNetwork.getDevSidByIntfSid(section_id));
			uci.remove('network', section_id);
			return m.save(null, true);
		};

		// show interface setting
		o = s.option(form.DummyValue, '_brname');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			return E('strong', uci.get('network', section_id, 'device'));
		};

		o = s.option(form.MultiValue, '_ports', _('Bridge Group Port'));
		o.rmempty=false;
		var ethports=[];
		var devs = uci.sections('network', 'device');
		var brdevs = devs.filter(function(e) { return e.type == 'bridge'});
		for(var i=0; i<brdevs.length; i++){
			var ports=brdevs[i]['ports']?brdevs[i]['ports']:[];
			ethports=ethports.concat(ports.filter(function(port){ return port.substring(0,3) == 'eth'}));
		}
		ethports=ethports.sort();
		for(var i=0; i<ethports.length; i++){
			o.value(ethports[i], rtkNetwork.getNetdevDisplay(ethports[i]));
		}

		var wlports=[];
		var wlIfaces = uci.sections('wireless', 'wifi-iface');
		var apIfaces = wlIfaces.filter(function(e) { return e.mode == 'ap'});
		for(var i=0; i<apIfaces.length; i++){
			wlports=wlports.concat(rtkNetwork.getWifiNameBySid(apIfaces[i]['.name']));
		}
		//wlports=wlports.sort();
		for(var i=0; i<wlports.length; i++){
			o.value(wlports[i], wlports[i]);
		}

		var brWanports=[];
		var brWandevs = devs.filter(function(e) { return (e.ifname == 'veip0' && e.rtk_chmode == "Bridged");});
		for(var i=0; i<brWandevs.length; i++){
			brWanports=brWanports.concat(brWandevs[i]['name']);
		}
		brWanports=brWanports.sort();
		for(var i=0; i<brWanports.length; i++){
			o.value(brWanports[i], brWanports[i]);
		}

		o.cfgvalue=function(section_id){
			var ports=uci.get('network', rtkNetwork.getDevSidByIntfSid(section_id), "ports");
			ports=ports?ports:[];
			ports=ports.filter(function(e) {
							// Hint: uci value is following cfgvalue function result, So need filter out wifi name
							if(rtkNetwork.getWifiSidByName(e)){
								return false;
							}
							return true;
						});
			var wifaces=uci.sections('wireless', 'wifi-iface');
			for(var i=0; i<wifaces.length;i++){
				if(wifaces[i]['network'] == section_id){
					ports.push(rtkNetwork.getWifiNameBySid(wifaces[i]['.name']));
				}
			}
			return ports;
		};

		o.write=function(section_id, formvalue){
			var devIdx=0;
			// lookup all wifi-device
			var devs=uci.sections('wireless', 'wifi-device');
			var intfs=uci.sections('wireless', 'wifi-iface');
			for(var i=0;i<devs.length;i++){
				var ifIdx=0;
				for(var j=0;j<intfs.length;j++){
					// lookup all wifi-iface
					if (intfs[j]['device'] == devs[i]['.name']){
						var wlname='';
						if(intfs[j]['ifname']){
							wlname=intfs[j]['ifname'];
						}else{
							wlname="wlan%s".format((ifIdx==0)?devIdx.toString() : "%d-%d".format(devIdx, ifIdx));
						}

						var found=false;
						for(var idx=0;idx<formvalue.length; idx++){
							if(formvalue[idx] == wlname){
								found=true;
							}
						}

						if(found){
							uci.set('wireless', intfs[j]['.name'], 'network', section_id);
						}else{
							if(intfs[j]['network']  == section_id){
								uci.set('wireless', intfs[j]['.name'], 'network', lanIntfSid);
							}
						}

						ifIdx++;
					}
				}

				devIdx++;
			}

			// filter out wireless interfaces
			formvalue=formvalue.filter(function(e) {
				var wifiSid=rtkNetwork.getWifiSidByName(e);
				if(wifiSid){
					return false;
				}

				return true;
			});

			var devname=uci.get('network', section_id, 'device');
			// lookup all bridge device
			uci.sections('network', 'device', function(s) {
				if (s['type'] == 'bridge' && s['ports']){
					if (s['name'] == devname){
						// re-assign port to lan
						var del_ports=s['ports'].filter(function(e) {
							for(var idx=0;idx<formvalue.length; idx++){
								if(e == formvalue[idx]){
									return false;
								}
							}
							return true;
						});

						if(del_ports.length){
							var lan_sid=rtkNetwork.getDevSidByIntfSid(lanIntfSid);
							var lan_ports=uci.get('network', lan_sid, 'ports');
							lan_ports=lan_ports?lan_ports:[]
							uci.set('network', lan_sid, 'ports', lan_ports.concat(del_ports));
						}
					}else{
						// remove same ports
						var after_ports=s['ports'].filter(function(e) {
							for(var idx=0;idx<formvalue.length; idx++){
								if(e == formvalue[idx]){
									return false;
								}
							}
							return true;
						});
						if(after_ports.length){
							uci.set('network', s['.name'], 'ports', after_ports);
						}else{
							uci.unset('network', s['.name'], 'ports');
						}
					}
				}
			});

			if(formvalue.length){
				return uci.set('network', rtkNetwork.getDevSidByIntfSid(section_id), 'ports', formvalue);
			}else{
				return uci.unset('network',	rtkNetwork.getDevSidByIntfSid(section_id), 'ports');
			}
			//return this.super('write', arguments);	// it wiil take original formvalue
		}

		o.textvalue=function(section_id){
			var cfgvalue=L.toArray(this.super('textvalue', arguments));

			var table = E('table', { 'class': 'table' });
			for (var i = 0; i < cfgvalue.length; i++) {
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left'}, [ rtkNetwork.getNetdevDisplay(cfgvalue[i]) ])
				]));
			}

			return table;
		}

		return m.render();
	}
});
