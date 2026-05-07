'use strict';
'require view';
'require form';
'require network';
'require firewall';
'require uci';
'require dom';
'require fs';
'require poll';
'require tools.widgets as widgets';

function iface_updown(up, id, ev, force) {
	var row = document.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(id)),
		dsc = row.querySelector('[data-name="_ifacestat"] > div'),
		btns = row.querySelectorAll('.cbi-section-actions .reconnect, .cbi-section-actions .down');

	btns[+!up].blur();
	btns[+!up].classList.add('spinning');

	btns[0].disabled = true;
	btns[1].disabled = true;

	if (!up) {
		L.resolveDefault(fs.exec_direct('/usr/libexec/rtk/luci-peeraddr')).then(function(res) {
			var info = null; try { info = JSON.parse(res); } catch(e) {}

			var intfs=[];
			var sameIntf=false;

			if(L.isObject(info) && Array.isArray(info.inbound_interfaces)){
				for(var j=0; j< intfs.length; j++){
					if(info.inbound_interfaces.filter(function(i) { return i == intfs[j] })[0]){
						sameIntf = true;
					}
				}
			}

			if (sameIntf) {
				ui.showModal(_('Confirm disconnect'), [
					E('p', _('You appear to be currently connected to the device via the "%h" interface. Do you really want to shut down the interface?').format(id)),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'cbi-button cbi-button-neutral',
							'click': function(ev) {
								btns[1].classList.remove('spinning');
								btns[1].disabled = false;
								btns[0].disabled = false;

								ui.hideModal();
							}
						}, _('Cancel')),
						' ',
						E('button', {
							'class': 'cbi-button cbi-button-negative important',
							'click': function(ev) {
								dsc.setAttribute('disconnect', '');
								dom.content(dsc, E('em', _('Interface is shutting down...')));

								ui.hideModal();
							}
						}, _('Disconnect'))
					])
				]);
			}
			else {
				dsc.setAttribute('disconnect', '');
				dom.content(dsc, E('em', _('Interface is shutting down...')));
			}
		});
	}
	else {
		dsc.setAttribute(up ? 'reconnect' : 'disconnect', force ? 'force' : '');
		dom.content(dsc, E('em', up ? _('Interface is reconnecting...') : _('Interface is shutting down...')));
	}
}

function validateNull(section_id, value) {
	if (value == null || value == '')
		return _("This item should NOT be NULL");

	return true;
}

function validateIPrange(section_id, value) {
	if (value == null || value == '')
		return _("This item should NOT be NULL");

	var ipAddr = value.split('-');
	if(!ipAddr || ipAddr.length != 2){
		return _("Wrong format, Ex:11.11.11.200-220");
	}

	var re = /^(\d+\.\d+\.\d+\.\d+)$/;
	if(!ipAddr[0].match(re)){
		return _('valid IPv4 address');
	}

	return true;
}

function render_status(node, section_id, networks, with_device) {
	var ifc=networks.filter(function(n) { return n.getName() == section_id })[0];
	var desc = null, c = [];
	var markdel = 0, changecount = 0, ipaddrs = null, errors = null, maindev = null, macaddr = null;
	var ip6addrs = null;

	if(!ifc){
		return;
	}

	if(ifc){
		if (!uci.get('network', ifc.getName())){
			markdel = 1;
		}

		ipaddrs =  ifc.getIPAddrs(),
		ip6addrs = ifc.getIP6Addrs();
		errors = ifc.getErrors(),
		maindev = ifc.getL3Device() || ifc.getDevice(),
		macaddr = maindev ? maindev.getMAC() : null;
	}

	return L.itemlist(node, [
		_('Protocol'), (!markdel && ifc) ? ifc.getI18n() : null,
		_('Uptime'),   (!markdel && ifc && !changecount && ifc.isUp()) ? '%t'.format(ifc.getUptime()) : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[0] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[1] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[2] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[3] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[4] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[0] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[1] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[2] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[3] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[4] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[5] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[6] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[7] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[8] : null,
		_('IPv6'),     (!markdel && ip6addrs) ? ip6addrs[9] : null,
		_('Error'),    (!markdel && errors) ? errors[0] : null,
		_('Error'),    (!markdel && errors) ? errors[1] : null,
		_('Error'),    (!markdel && errors) ? errors[2] : null,
		_('Error'),    (!markdel && errors) ? errors[3] : null,
		_('Error'),    (!markdel && errors) ? errors[4] : null,
		null, markdel ? E('em', _('Interface is marked for deletion')) : null
	]);
}

return view.extend({
	poll_status: function(map, networks) {
		var resolveZone = null;

		var vpndevs = uci.sections('network', 'interface');
		for (var j = 0; j < vpndevs.length; j++) {
			if(vpndevs[j]['proto'] != 'pptp') continue;

			var row = map.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(vpndevs[j]['.name']));
			if (row == null)
				continue;

			var dsc = row.querySelector('[data-name="_ifacestat"] > div');
			render_status(dsc, vpndevs[j]['.name'], networks, false);

			var btn1 = row.querySelector('.cbi-section-actions .reconnect');
			var btn2 = row.querySelector('.cbi-section-actions .down');
			var ifc=networks.filter(function(n) { return n.getName() == vpndevs[j]['.name'] })[0];
			var disabled = ifc ? !ifc.isUp() : true;
			btn1.disabled = btn1.classList.contains('spinning') || btn2.classList.contains('spinning');
			btn2.disabled = btn1.classList.contains('spinning') || btn2.classList.contains('spinning') || disabled;
		}
		return Promise.all([ resolveZone, network.flushCache() ]);
	},

	render:function(){
		var m,s,o,ss;
		m=new form.Map('network', _('PPTP'),_('it is use to configure pptp client/server settings'));
		m.chain('pptpd');

		s=m.section(form.TypedSection,'service');
		s.uciconfig = 'pptpd';
		s.anonymous=true;

		s.tab('general',_('General'));
		s.tab('client', _('Client Settings'));
		s.tab('server', _('Server Settings'));

		o=s.taboption('general', form.ListValue, 'mode', _('Mode'));
		o.value('client',_('Client'));
		o.value('server',_('Server')); 
		o.write = function(section_id, formvalue){
			if(formvalue == 'client'){
				uci.unset('pptpd', 'pptpd', 'enabled');
				var vpndev=uci.sections('network', 'interface');
				for (var j = 0; j < vpndev.length; j++) {
					if(vpndev[j]['proto'] == 'pptp'){
						uci.set('network', vpndev[j]['.name'], 'disabled', 0);
					}
				}
			}else{
				uci.set('pptpd', 'pptpd', 'enabled', 1);
				var vpndev=uci.sections('network', 'interface');
				for (var j = 0; j < vpndev.length; j++) {
					if(vpndev[j]['proto'] == 'pptp'){
						uci.set('network', vpndev[j]['.name'], 'disabled', 1);
					}
				}
			}

			return this.map.data.set(
					this.uciconfig || this.section.uciconfig || this.map.config,
					this.ucisection || section_id,
					this.ucioption || this.option,
					formvalue);
		};

		// for Server mode
		o=s.taboption('server', form.SectionValue, '_service', form.TypedSection, 'service');
		o.depends({mode: "server"});

		ss = o.subsection;
		ss.uciconfig = 'pptpd';
		ss.addremove = false;
		ss.anonymous = true;

		o=ss.option(form.Flag, 'enabled', _('Enable'));
		o.depends({mode: "server"});

		o=ss.option(form.Value, 'localip', _('Local IP'));
		o.datatype='ip4addr';
		o.depends({mode: "server", enabled: "1"});
		o.validate = validateNull;
		o.default = '11.11.11.1'

		o=ss.option(form.Value, 'remoteip', _('Remote IP'));
		o.datatype='string';
		o.depends({mode: "server", enabled: "1"});
		o.validate = validateNull;
		o.default = '11.11.11.200-220'

		o=ss.option(form.Flag, 'auth', _('Authentication'));
		o.depends({mode: "server", enabled: "1"});

		o=ss.option(form.ListValue, 'authmode', _('Authentication Mode'));
		o.datatype='string';
		o.depends({mode: "server", auth: "1"});
		o.value('',_('auto'));
		o.value('require-chap',_('CHAP'));
		o.value('require-mschap',_('MS-CHAP'));
		o.value('require-mschap-v2',_('MS-CHAPv2'));

		o=ss.option(form.ListValue, 'mppe', _('MPPE'));
		o.datatype='string';
		o.depends({mode: "server", authmode: "require-mschap"});
		o.depends({mode: "server", authmode: "require-mschap-v2"});
		o.value('nomppe',_('Disable MPPE'));
		o.value('mppe required,stateless',_('MPPE 40/56/128bit'));
		o.value('mppe required,no56,no128,stateless',_('MPPE 40bit'));
		o.value('mppe required,no40,no128,stateless',_('MPPE 56bit'));
		o.value('mppe required,no40,no56,stateless',_('MPPE 128bit'));
		o.default='mppe required,no40,no56,stateless';

		o = s.taboption('server', form.SectionValue, '_login', form.GridSection, 'login', _('Secret Account'));
		o.depends({mode: "server"});
		o.uciconfig = 'pptpd';

		ss = o.subsection;
		ss.uciconfig = 'pptpd';
		ss.anonymous=true;
		ss.addremove=true;
		ss.addbtntitle=_('Add');

		ss.handleAdd = function(ev) {
			// if map have multiple configs, if map.config is not same with target config, it will add/delete fail.
			// So, we dynamic change map.config.
			this.map.config = 'pptpd';
			var section_id = this.map.data.add('pptpd', 'login'),
				mapNode = this.getPreviousModalMap(),
				prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

			prevMap.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		ss.renderMoreOptionsModal = function(section_id, ev) {
			// if map have multiple configs, if map.config is not same with target config, it will add/delete fail.
			// So, we dynamic change map.config.
			this.map.config = 'pptpd';
			return this.super('renderMoreOptionsModal', arguments);
		};

		o=ss.option(form.Value, 'username', _('UserName'));
		o.uciconfig = 'pptpd';
		o.datatype='string';
		o.default="";
		o.validate = validateNull;

		o=ss.option(form.Value, 'password', _('Password'));
		o.datatype='string';

		o.default="";
		o.validate = validateNull;

		// for Client mode
		o = s.taboption('client', form.SectionValue, '_client', form.GridSection, 'interface');
		o.depends({mode: "client"});
		
		ss = o.subsection;
		ss.uciconfig = 'network';
		ss.anonymous=true;
		ss.addremove=true;
		ss.addbtntitle=_('Add');
		ss.filter = function(section_id){
			if(uci.get('network', section_id, 'proto') == 'pptp'){
				return true;
			}

			return false;
		};

		ss.load = function() {
			return Promise.all([
				network.getNetworks(),
				firewall.getZones()
			]).then(L.bind(function(data) {
				this.networks = data[0];
				this.zones = data[1];
			}, this));
		};

		ss.tab('general', _('General Settings'));
		ss.tab('advanced', _('Advanced Settings'));

		ss.addModalOptions = function(s) {
			var o = s.taboption('general', form.ListValue, 'rtk_ppp_authmode', _('Authentication Mode'));
			o.value('auto', _('AUTO'));
			o.value('pap', _('PAP'));
			o.value('chap', _('CHAP'));
			o.value('mschap', _('MSCHAP'));
			o.value('mschapv2', _('MSCHAPv2'));
			o.value('eap', _('EAP'));
			o.default = 'auto';

			o = s.taboption('general', form.ListValue,'rtk_ppp_mppe',_('MPPE'));
			o.datatype='string';
			o.depends({rtk_ppp_authmode: "mschap"});
			o.depends({rtk_ppp_authmode: "mschapv2"});
			o.value('nomppe',_('Disable MPPE'));
			o.value('mppe required,stateless',_('MPPE 40/56/128bit'));
			o.value('mppe required,no56,no128,stateless',_('MPPE 40bit'));
			o.value('mppe required,no40,no128,stateless',_('MPPE 56bit'));
			o.value('mppe required,no40,no56,stateless',_('MPPE 128bit'));
			o.default='mppe required,no40,no56,stateless';

			o = s.taboption('general', form.Value, 'server', _('PPTP Server'));
			o.datatype = 'host(0)';

			s.taboption('general', form.Value, 'username', _('UserName'));

			o = s.taboption('general', form.Value, 'password', _('Password'));
			o.password = true;

			if (L.hasSystemFeature('ipv6')) {
				o = s.taboption('advanced', form.ListValue, 'ppp_ipv6', _('Obtain IPv6 Address'), _('Enable IPv6 negotiation on the PPP link'));
				o.ucioption = 'ipv6';
				o.value('auto', _('Automatic'));
				o.value('0', _('Disabled'));
				o.value('1', _('Manual'));
				o.default = '0';
			}

			o = s.taboption('general', form.Flag, 'defaultroute', _('Default Gateway'));
			o.rmempty=false;
		};

		ss.handleAdd = function(ev) {
			var nameval=null;
			for (var i = 0;; i++) {
				nameval = 'pptp%d'.format(i);
				if (uci.get('network', nameval) == null){
					break;
				}
			}

			// if map have multiple configs, if map.config is not same with target config, it will add/delete fail.
			// So, we dynamic change map.config.
			this.map.config = 'network';
			var section_id = this.map.data.add('network', 'interface', nameval),
				mapNode = this.getPreviousModalMap(),
				prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

			// fix protocol to pptp
			uci.set('network', section_id, 'proto', 'pptp');
			prevMap.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		ss.renderMoreOptionsModal = function(section_id, ev) {
			// if map have multiple configs, if map.config is not same with target config, it will add/delete fail.
			// So, we dynamic change map.config.
			this.map.config = 'network';
			return this.super('renderMoreOptionsModal', arguments);
		};

		ss.renderRowActions = function(section_id) {
			var tdEl = this.super('renderRowActions', [ section_id, _('Edit') ]);

			dom.content(tdEl.lastChild, [
				E('button', {
					'class': 'cbi-button cbi-button-neutral reconnect',
					'click': iface_updown.bind(this, true, section_id),
					'title': _('Reconnect this interface'),
				}, _('Restart')),
				E('button', {
					'class': 'cbi-button cbi-button-neutral down',
					'click': iface_updown.bind(this, false, section_id),
					'title': _('Shutdown this interface'),
				}, _('Stop')),
				tdEl.lastChild.firstChild,
				tdEl.lastChild.lastChild
			]);

			return tdEl;
		};

		o = ss.option(form.DummyValue, '_ifacestat');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			var node = E('div', { 'id': '%s-ifc-description'.format(section_id) });
			render_status(node, section_id, this.section.networks, false);
			return node;
		};

		return m.render().then(L.bind(function(m, nodes) {
			poll.add(L.bind(function() {
				var tasks = [];

				var vpndevs = uci.sections('network', 'interface');
				for (var i = 0; i < vpndevs.length; i++) {
					if(vpndevs[i]['proto'] != 'pptp') continue;

					var row = nodes.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(vpndevs[i]['.name']));
					if(!row) continue;
					var dsc = row.querySelector('[data-name="_ifacestat"] > div'),
						btn1 = row.querySelector('.cbi-section-actions .reconnect'),
						btn2 = row.querySelector('.cbi-section-actions .down');

					if (dsc.getAttribute('reconnect') == '') {
						dsc.setAttribute('reconnect', '1');
						tasks.push(fs.exec('/sbin/ifup', [vpndevs[i]['.name']]).catch(function(e) {
							ui.addNotification(null, E('p', e.message));
						}));
					}else if (dsc.getAttribute('disconnect') == '') {
						dsc.setAttribute('disconnect', '1');
						tasks.push(fs.exec('/sbin/ifdown', [vpndevs[i]['.name']]).catch(function(e) {
							ui.addNotification(null, E('p', e.message));
						}));
					}else if (dsc.getAttribute('reconnect') == '1') {
						dsc.removeAttribute('reconnect');
						btn1.classList.remove('spinning');
						btn1.disabled = false;
					}else if (dsc.getAttribute('disconnect') == '1') {
						dsc.removeAttribute('disconnect');
						btn2.classList.remove('spinning');
						btn2.disabled = false;
					}
				}

				return Promise.all(tasks)
					.then(L.bind(network.getNetworks, network))
					.then(L.bind(this.poll_status, this, nodes));
			}, this), 3);

			return nodes;
		}, this, m));
	}
});
