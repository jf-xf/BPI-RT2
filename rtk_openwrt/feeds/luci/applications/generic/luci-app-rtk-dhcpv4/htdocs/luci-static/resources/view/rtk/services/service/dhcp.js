'use strict';
'require view';
'require uci';
'require form';
'require rpc';
'require fs';
'require ui';
'require network';
'require rtk.network as rtkNetwork';

function validateNull(sid, s) {
	if (s == null || s == '')
		return _("This item should NOT be NULL");

	return true;
}

var callHostHints = rpc.declare({
	object: 'luci-rpc',
	method: 'getHostHints',
	expect: { '': {} }
});

var callLuciDHCPLeases = rpc.declare({
	object: 'luci-rpc',
	method: 'getDHCPLeases',
	expect: { '': {} }
});

function handleCreateStaticLease(lease, ev) {
	ev.currentTarget.classList.add('spinning');
	ev.currentTarget.disabled = true;
	ev.currentTarget.blur();

	var cfg = uci.add('dhcp', 'host');
	uci.set('dhcp', cfg, 'name', lease.hostname);
	uci.set('dhcp', cfg, 'ip', lease.ipaddr);
	uci.set('dhcp', cfg, 'mac', lease.macaddr.toUpperCase());

	return uci.save()
		.then(L.bind(L.ui.changes.init, L.ui.changes))
		.then(L.bind(L.ui.changes.displayChanges, L.ui.changes));
};

return view.extend({
	load: function() {
		return Promise.all([
			network.getNetworks(),	// for relay setting
			callHostHints()
		]);
	},

	handleShowClients: function(ev) {
		return Promise.all([
			callLuciDHCPLeases(),
			network.getHostHints()
		]).then(function(data) {
			var leases = Array.isArray(data[0].dhcp_leases) ? data[0].dhcp_leases : [];
			var machints = data[1].getMACHints(false);
			var hosts = uci.sections('dhcp', 'host');
			var isReadonlyView = !L.hasViewPermission();
			var isMACStatic={};

			for (var i = 0; i < hosts.length; i++) {
				var host = hosts[i];

				if (host.mac) {
					var macs = L.toArray(host.mac);
					for (var j = 0; j < macs.length; j++) {
						var mac = macs[j].toUpperCase();
						isMACStatic[mac] = true;
					}
				}
			};

			var table = E('table', { 'class': 'table lases' }, [
				E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'th' }, _('Hostname')),
					E('th', { 'class': 'th' }, _('IPv4 Address')),
					E('th', { 'class': 'th' }, _('MAC Address')),
					E('th', { 'class': 'th' }, _('Lease Time Remaining')),
					isReadonlyView ? E([]) : E('th', { 'class': 'th cbi-section-actions' }, _('Static Lease'))
				])
			]);

			cbi_update_table(table, leases.map(L.bind(function(lease) {
				var exp, rows;

				if (lease.expires === false)
					exp = E('em', _('unlimited'));
				else if (lease.expires <= 0)
					exp = E('em', _('expired'));
				else
					exp = '%t'.format(lease.expires);

				var hint = lease.macaddr ? machints.filter(function(h) { return h[0] == lease.macaddr })[0] : null,
					host = null;

				if (hint && lease.hostname && lease.hostname != hint[1])
					host = '%s (%s)'.format(lease.hostname, hint[1]);
				else if (lease.hostname)
					host = lease.hostname;

				rows = [
					host || '-',
					lease.ipaddr,
					lease.macaddr,
					exp
				];

				if (!isReadonlyView && lease.macaddr != null) {
					var mac = lease.macaddr.toUpperCase();
					rows.push(E('button', {
						'class': 'cbi-button cbi-button-apply',
						'click': L.bind(handleCreateStaticLease, this, lease),
						'disabled': isMACStatic[mac]
					}, [ _('Set Static') ]));
				}

				return rows;
			}, this)), E('em', _('There are no active leases')));

			ui.showModal(_('Active DHCP Clients'), [
				table,
				E('div', { 'class': 'right' }, [
					E('button', {
						'class': 'btn',
						'click': ui.createHandlerFn(this, function(ev) {
							return ui.hideModal();
						})
					}, [ _('Cancel') ])
				])
			]);
		})
	},

	render: function(load_result) {
		if (L.hasSystemFeature('dnsmasq') || L.hasSystemFeature('odhcpd')){
			var networks = load_result[0];
			var hosts = load_result[1];
			var m, s, o, ss;

			m = new form.Map('dhcp', _('DHCP Settings'), _('This page is used to configure DHCP Server.'));
			m.chain('network');

			s = m.section(form.TypedSection, 'dnsmasq');
			s.anonymous = true;
			s.filter = function(section_id) {
				if(section_id == "dnsmasq")
					return true;

				return false;
			};

			o = s.option(form.ListValue, 'dhcpv4mode', _('DHCPv4 Mode'), _("Specifies whether DHCPv4 Mode should be server/disabled/relay."));
			o.datatype = 'string';
			o.value("disabled", _("Disable"));
			o.value("relay", _("Relay"));
			o.value("server", _("Server"));
			o.default = "server";

			o = s.option(form.SectionValue, '_dhcp', form.TypedSection, 'dhcp');
			ss = o.subsection;
			ss.addremove = false;
			ss.anonymous = true;
			ss.filter = function(section_id) {
				if(section_id == "lan")
					return true;

				return false;
			};

			o = ss.option(form.Flag, 'dhcpv4', _('Server Enable'), _("Specifies DHCPv4 Server should be enable/disabled."));
			o.depends({"dhcp.dnsmasq.dhcpv4mode":"server"});
			o.rmempty = false;
			o.enabled = 'server';
			o.disabled = 'disabled';
			o.default = o.enabled;

			o = ss.option(form.Value, 'start', _('Start'), _("Specifies the offset from the network address of the underlying interface to calculate the minimum address that may be leased to clients. It may be greater than 255 to span subnets."));
			o.depends({dhcpv4:"server"});
			o.validate = validateNull;
			o.optional = true;
			o.default = '100';

			o = ss.option(form.Value, 'limit', _('Limit'), _("Specifies the size of the address pool (e.g. with start=100, limit=150, maximum address will be .249)"));
			o.depends({dhcpv4:"server"});
			o.validate = validateNull;
			o.optional = true;
			o.default = '150';

			o = ss.option(form.Value, 'leasetime', _('Max Lease Time'), _("Specifies the lease time of addresses handed out to clients, for example 12h or 30m"));
			o.depends({dhcpv4:"server"});
			o.validate = validateNull;
			o.optional = true;
			o.default = '12h';

			o = ss.option(form.Value, 'domain', _('Local Domain'), _('Local domain suffix appended to DHCP names and hosts file entries.'));
			o.depends({dhcpv4:"server"});
			o.ucisection = 'dnsmasq';
			o.write=function(section_id, formvalue) {
				if(formvalue){
					uci.set('dhcp', this.ucisection || section_id, 'local', '/%s/'.format(formvalue));
					return this.super('write', arguments);
				}else{
					uci.unset('dhcp', this.ucisection || section_id, 'local');
					return this.super('remove', arguments);
				}
			};
			o.remove=function(section_id) {};	//don't want to remove domain option

			o = ss.option(form.ListValue, '_dnsmode', _('DNS Option'));
			o.depends({dhcpv4:"server"});
			o.value("", _("DNS Proxy"));
			o.value("manully", _("Manually"));
			o.write=o.remove=function(){};
			o.cfgvalue=function(section_id){
				var dhcp_opt=uci.get('dhcp', section_id, 'dhcp_option_force');
				if(dhcp_opt && dhcp_opt.length){
					for(var i=0; i<dhcp_opt.length;i++){
						var arrays=dhcp_opt[i].split(",");
						if(arrays[0] == '6'){
							return "manully";
						}
					}
				}

				return '';
			};

			o = ss.option(form.DynamicList, 'dns', _('Use Custom DNS Servers'));
			o.depends({ _dnsmode: "manully" });
			o.datatype='ip4addr("nomask")';
			o.placeholder = _('8.8.8.8');
			o.cfgvalue=function(section_id){
				var dhcp_opt=uci.get('dhcp', section_id, 'dhcp_option_force');
				if(dhcp_opt && dhcp_opt.length){
					for(var i=0; i<dhcp_opt.length;i++){
						var arrays=dhcp_opt[i].split(",");
						if(arrays[0] == '6'){
							arrays.shift();
							return arrays;
						}
					}
				}

				return [];
			};
			o.write=function(section_id, formvalue){
				var ucivalue='6,%s'.format(formvalue.toString());
				var dhcp_opt=uci.get('dhcp', section_id, 'dhcp_option_force');
				if(dhcp_opt && dhcp_opt.length){
					for(var i=0; i<dhcp_opt.length;i++){
						var arrays=dhcp_opt[i].split(",");
						if(arrays[0] == '6'){
							dhcp_opt[i]=ucivalue;
						}
					}
				}else{
					dhcp_opt=[ ucivalue ];
				}
				uci.set('dhcp', section_id, 'dhcp_option_force', dhcp_opt);
			};
			o.remove=function(section_id){
				var newarrays=[];
				var dhcp_opt=uci.get('dhcp', section_id, 'dhcp_option_force');
				if(dhcp_opt && dhcp_opt.length){
					for(var i=0; i<dhcp_opt.length;i++){
						var arrays=dhcp_opt[i].split(",");
						if(arrays[0] != '6'){
							newarrays.push(dhcp_opt[i]);
						}
					}
					if(newarrays.length)
						uci.set('dhcp', section_id, 'dhcp_option_force', newarrays);
					else
						uci.unset('dhcp', section_id, 'dhcp_option_force');
				}else{
					uci.unset('dhcp', section_id, 'dhcp_option_force');
				}
			};

			// portfilter section is Realtek customize section
			// it will drop package in ebtable input chain.
			// For now, it only support following options
			// config portfilter 'v4_portfilter'
			// 		list proto	dhcp	--> drop dhcp protocol request
			//		list ports	eth0.2	--> packet come from this port
			o = ss.option(form.MultiValue, 'ports', _('Port-Based Filter'), _('it will block DHCP request on selected port.'));
			o.ucisection="v4_portfilter".format(s.section);
			o.depends({dhcpv4:"server"});
			var ethports=[];
			var devs = uci.sections('network', 'device');
			var brdevs = devs.filter(function(e) { return e.type == 'bridge'});
			for(var i=0; i<brdevs.length; i++){
				var ports=brdevs[i]['ports']?brdevs[i]['ports']:[];
				ethports=ethports.concat(ports.filter(function(port){ return port.substring(0,3) == 'eth'}));
			}
			ethports=ethports.sort();
			for(var i=0; i<ethports.length; i++){
				o.value(ethports[i], '%s'.format(rtkNetwork.getNetdevDisplay(ethports[i])));
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

			o.write=function(section_id, formvalue){
				var portfilter=uci.get('dhcp', this.ucisection || section_id);
				if(!portfilter){
					uci.add('dhcp', 'portfilter', this.ucisection || section_id);
					uci.set('dhcp', this.ucisection || section_id, 'proto', [ 'dhcp' ]);
				}
				return this.super('write', arguments);
			};
			o.remove=function(section_id){
				uci.remove('dhcp', this.ucisection || section_id);
				return this.super('remove', arguments);
			}

			o = ss.option(form.Button, '_clibtn', _('Show Client'));
			o.depends({dhcpv4:"server"});
			o.inputstyle = 'action important';
			o.inputtitle = _('Show Client');
			o.onclick = this.handleShowClients;

			/* Relay */
			o = s.option(form.SectionValue, '_relay', form.GridSection, 'relay');
			o.depends({"dhcp.dnsmasq.dhcpv4mode":"relay"});
			ss = o.subsection;
			ss.anonymous = true;
			ss.addremove = true;
			ss.addbtntitle = _('Add Relay');
			ss.handleAdd = function(ev, name) {
				var config_name = this.uciconfig || this.map.config,
				    section_id = this.map.data.add(config_name, this.sectiontype, name),
				    mapNode = this.getPreviousModalMap(),
				    prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

				var intf=uci.get('dhcp', 'lan', 'interface');

				// Force listen on lan interface
				for(var i=0; i < networks.length; i++){
					if(networks[i].sid == intf){
						uci.set('dhcp', section_id, 'local_addr', networks[i].getIPAddr());
						break;
					}
				}

				uci.set('dhcp', section_id, 'disabled', 0);	// enable by default
				prevMap.addedSection = section_id;

				return this.renderMoreOptionsModal(section_id);
			}
			ss.filter = function(section_id) {
				var intf=uci.get('dhcp', 'lan', 'interface');
				var target_addr='';
				var curr_addr='';

				// Force listen on lan interface
				for(var i=0; i < networks.length; i++){
					if(networks[i].sid == intf){
						target_addr=networks[i].getIPAddr();
						break;
					}
				}

				curr_addr = uci.get('dhcp', section_id, 'local_addr');
				if(target_addr != '' && target_addr == curr_addr)
					return true;

				return false;
			};

			o = ss.option(form.Flag, 'disabled', _('Disabled'));
			o.rmempty = false;
			o.editable = true;
			o.default = false;

			o = ss.option(form.Value, 'server_addr', _('Remote Server IP Address'));
			o.datatype='ip4addr("nomask")';
			o.placeholder = _('8.8.8.8');
			o.validate = validateNull;

			/* Mac-base Assignment */
			o = s.option(form.SectionValue, '_host', form.GridSection, 'host', _('Static Leases'));
			o.depends({"dhcp.dnsmasq.dhcpv4mode":"server"});
			ss = o.subsection;
			ss.anonymous = true;
			ss.addremove = true;
			ss.addbtntitle = _('Add');
			ss.cfgsections = function() {
				// only ipv4 host section allowed to configuration.
				var sections = uci.sections('dhcp', 'host');
				var filterSections = sections.filter(function(e) { return e.ip? true: false; });
				var section_ids = filterSections.map(function(s) { return s['.name'] });
				return section_ids;
			};
			ss.handleAdd = function(ev, name) {
				var config_name = this.uciconfig || this.map.config,
				    section_id = this.map.data.add(config_name, this.sectiontype, name),
				    mapNode = this.getPreviousModalMap(),
				    prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

				// this option is used to identify section belone to which server.
				uci.set('dhcp', section_id, 'dhcp', 'lan');
				uci.set('dhcp', section_id, 'enable', 1);	// enable by default
				prevMap.addedSection = section_id;

				return this.renderMoreOptionsModal(section_id);
			}

			o = ss.option(form.Flag, 'enable', _('Enable'));
			o.rmempty = false;
			o.editable = true;
			o.default = true;

			o = ss.option(form.Value, 'mac', _('<abbr title="Media Access Control">MAC</abbr>-Address'));
			o.datatype = 'list(unique(macaddr))';
			o.rmempty  = true;
			o.cfgvalue = function(section) {
				var macs = uci.get('dhcp', section, 'mac'),
					result = [];

				if (!Array.isArray(macs))
					macs = (macs != null && macs != '') ? macs.split(/\ss+/) : [];

				for (var i = 0, mac; (mac = macs[i]) != null; i++)
					if (/^([0-9a-fA-F]{1,2}):([0-9a-fA-F]{1,2}):([0-9a-fA-F]{1,2}):([0-9a-fA-F]{1,2}):([0-9a-fA-F]{1,2}):([0-9a-fA-F]{1,2})$/.test(mac))
						result.push('%02X:%02X:%02X:%02X:%02X:%02X'.format(
	                                                parseInt(RegExp.$1, 16), parseInt(RegExp.$2, 16),
							parseInt(RegExp.$3, 16), parseInt(RegExp.$4, 16),
							parseInt(RegExp.$5, 16), parseInt(RegExp.$6, 16)));

				return result.length ? result.join(' ') : null;
			};

			o.renderWidget = function(section_id, option_index, cfgvalue) {
				var node = form.Value.prototype.renderWidget.apply(this, [section_id, option_index, cfgvalue]),
					ipopt = this.section.children.filter(function(o) { return o.option == 'ip' })[0];

				node.addEventListener('cbi-dropdown-change', L.bind(function(ipopt, section_id, ev) {
					var mac = ev.detail.value.value;
					if (mac == null || mac == '' || !hosts[mac] || !hosts[mac].ipv4)
						return;

					var ip = ipopt.formvalue(section_id);
					if (ip != null && ip != '')
						return;

					var node = ipopt.map.findElement('id', ipopt.cbid(section_id));
					if (node)
						dom.callClassMethod(node, 'setValue', hosts[mac].ipv4);
				}, this, ipopt, section_id));

				return node;
			};

			Object.keys(hosts).forEach(function(mac) {

				var hint = hosts[mac].name || hosts[mac].ipv4;
				o.value(mac, hint ? '%s (%s)'.format(mac, hint) : mac);
			});

			o = ss.option(form.Value, 'ip', _('<abbr title="Internet Protocol Version 4">IPv4</abbr>-Address'));
			o.datatype = 'ip4addr';
			o.validate = function(section, value) {
				var mac = this.map.lookupOption('mac', section),
					name = this.map.lookupOption('name', section),
					m = mac ? mac[0].formvalue(section) : null,
					n = name ? name[0].formvalue(section) : null;

				if ((m == null || m == '') && (n == null || n == ''))
					return _('One of hostname or mac address must be specified!');

				return true;
			};

			Object.keys(hosts).forEach(function(mac) {
				if (hosts[mac].ipv4) {
					var hint = hosts[mac].name;
					o.value(hosts[mac].ipv4, hint ? '%s (%s)'.format(hosts[mac].ipv4, hint) : hosts[mac].ipv4);
				}
			});

			o = ss.option(form.Value, 'leasetime', _('Lease Time'));
			o.validate = validateNull;
			o.optional = true;
			o.default = '12h';

			return m.render();
		}else{
			return null;
		}
	}
});
