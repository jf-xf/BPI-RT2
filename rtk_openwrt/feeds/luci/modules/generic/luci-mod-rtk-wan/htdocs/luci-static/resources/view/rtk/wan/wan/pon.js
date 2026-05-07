'use strict';
'require view';
'require form';
'require uci';
'require ui';
'require poll';
'require dom';
'require network';
'require firewall';
'require fs';
'require tools.rtk.widgets as widgets';
'require rtk.network as rtkNetwork';
'require rpc';

var rootdev = 'veip0';
var rtkEnv;

var reload_fw=false;
document.addEventListener("uci-applied", applyPendingTask);
function applyPendingTask(){
        var tasks = [];
        if(reload_fw){
			tasks.push(fs.exec('/etc/init.d/firewall', [ 'reload' ]).catch(function(e) {
				ui.addNotification(null, E('p', e.message));
			}));
        }

        return Promise.all(tasks).then(function() {
			reload_fw = false;
        });
}

// get all device infomation, and use it create port list
var callGetDeviceStatus = rpc.declare({
	object: 'network.device',
	method: 'status',
	expect: { '': {} }
});

function initDefaultZones(zones, sid){
	for(var i=0; i < zones.length; i++){
		if(zones[i].getName() == 'INTERNET' || zones[i].getName() == 'nat' || zones[i].getName() == 'wan')
			zones[i].addNetwork(sid);
		else
			zones[i].deleteNetwork(sid);
	}
}

function updateBridgePorts(isAdd, name){
	var sections = uci.sections('network', 'device');
	var brDevs = sections.filter(function(e) { return e.name == 'br-lan' });

	for (var j = 0; j < brDevs.length; j++) {
		if(brDevs[j]['type'] == 'bridge'){
			var ports=uci.get('network', brDevs[j]['.name'], 'ports');
			var new_ports=[];
			var found=0;

			for (var k = 0; k < ports.length; k++) {
				if(ports[k] != name){
					if(!isAdd){
						new_ports.push(ports[k]);
					}
				}else{
					found = 1;
				}
			}

			if(!isAdd){
				uci.set('network', brDevs[j]['.name'], 'ports', new_ports);
			}else if(!found){
				ports.push(name);
				uci.set('network', brDevs[j]['.name'], 'ports', ports);
			}
		}
	}
}

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
			intfs.push('%s_v4'.format(id));
			if(L.hasSystemFeature('ipv6') && (uci.get('network', 'rtkglobals', 'ipv6') == '1')){
				intfs.push('%s_v6'.format(id));
			}

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

// go through all device section and check vlan setting
// return "section_id" if found same vlan setting
//        false if not found.
function isVlanUesd(ifname, excludeSid, type, vid){
	var sections = uci.sections('network', 'device');
	var rtkdevs = sections.filter(function(e) { return e.name.indexOf(rootdev) != -1 });

	//init type and vid for undefine case
	type = (!type) ? '' : type;
	vid = (!vid) ? 0 : vid;
	for (var i = 0; i < rtkdevs.length; i++) {
		// exclude same section id
		if(excludeSid == rtkdevs[i]['.name'])
			continue;

		// looking for all same root device entry
		if(rtkdevs[i]['ifname'] == ifname){
			if(type == (!rtkdevs[i]['type']?'':rtkdevs[i]['type'])){
				if(type == '' || rtkdevs[i]['vid'] == vid){
					return rtkdevs[i]['.name'];
				}
			}
		}
	}

	return false;
}

function validateNull(section_id, value) {
	if (value == null || value == '')
		return _("This item should NOT be NULL");

	return true;
}

function validateNetmask(section_id, value) {
	if (rtkNetwork.isValidNetmask(value) == false)
		return _("Expecting: valid IPv4 netmask");
	return true;
}

function count_changes(section_id) {
	var changes = ui.changes.changes, n = 0;

	if (!L.isObject(changes))
		return n;

	if (Array.isArray(changes.network))
		for (var i = 0; i < changes.network.length; i++)
			n += (changes.network[i][1] == section_id);

	if (Array.isArray(changes.dhcp))
		for (var i = 0; i < changes.dhcp.length; i++)
			n += (changes.dhcp[i][1] == section_id);

	return n;
}

function render_status(node, section_id, rtknetwork, with_device) {
	if(rtknetwork['config']['rtk_chmode'] == 'Bridged'){
		return;
	}

	var sid_v4='%s_v4'.format(section_id);
	var sid_v6='%s_v6'.format(section_id);
	var ifc=rtknetwork.networks.filter(function(n) { return n.getName() == sid_v4 })[0];
	var ifc6=rtknetwork.networks.filter(function(n) { return n.getName() == sid_v6 })[0];
	var desc = null, c = [];
	var markdel = 0, changecount = 0, ipaddrs = null, errors = null, maindev = null, macaddr = null;
	var v6markdel = 0, v6changecount = 0, ip6addrs = null, v6errors = null, v6maindev = null, v6macaddr = null;

	if(!ifc && !ifc6){
		return;
	}

	if(ifc){
		if (!uci.get('network', ifc.getName())){
			markdel = 1;
		}

		changecount = with_device ? 0 : count_changes(ifc.getName()),
		ipaddrs = changecount ? [] : ifc.getIPAddrs(),
		errors = ifc.getErrors(),
		maindev = ifc.getL3Device() || ifc.getDevice() || (ifc6 && (ifc6.getL3Device() || ifc6.getDevice())),
		macaddr = maindev ? maindev.getMAC() : null;
	}

	if(ifc6){
		if (!uci.get('network', ifc6.getName())){
			v6markdel = 1;
		}

		v6changecount = with_device ? 0 : count_changes(ifc6.getName()),
		ip6addrs = v6changecount ? [] : ifc6.getIP6Addrs();
		v6errors = ifc6.getErrors(),
		v6maindev = ifc6.getL3Device() || ifc6.getDevice(),
		v6macaddr = v6maindev ? v6maindev.getMAC() : null;
	}

	return L.itemlist(node, [
		_('MAC'),      macaddr ? macaddr : (v6macaddr ? v6macaddr : null),
		_('IPv4 Protocol'), (!markdel && ifc) ? ifc.getI18n() : null,
		_('IPv6 Protocol'), (!v6markdel && ifc6) ? ifc6.getI18n() : null,
		//_('IPv4 Uptime'),   (!markdel && ifc && !changecount && ifc.isUp()) ? '%t'.format(ifc.getUptime()) : null,
		//_('IPv6 Uptime'),   (!v6markdel && ifc6 && !v6changecount && ifc6.isUp()) ? '%t'.format(ifc6.getUptime()) : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[0] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[1] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[2] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[3] : null,
		_('IPv4'),     (!markdel && ipaddrs) ? ipaddrs[4] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[0] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[1] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[2] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[3] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[4] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[5] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[6] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[7] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[8] : null,
		_('IPv6'),     (!v6markdel && ip6addrs) ? ip6addrs[9] : null,
		_('IPv6-PD'),  (v6markdel || v6changecount) ? null : ((ifc6) ? ifc6.getIP6Prefix() : null),
		_('Error'),    (!markdel && errors) ? errors[0] : null,
		_('Error'),    (!markdel && errors) ? errors[1] : null,
		_('Error'),    (!markdel && errors) ? errors[2] : null,
		_('Error'),    (!markdel && errors) ? errors[3] : null,
		_('Error'),    (!markdel && errors) ? errors[4] : null,
		_('Error'),    (!v6markdel && v6errors) ? v6errors[0] : null,
		_('Error'),    (!v6markdel && v6errors) ? v6errors[1] : null,
		_('Error'),    (!v6markdel && v6errors) ? v6errors[2] : null,
		_('Error'),    (!v6markdel && v6errors) ? v6errors[3] : null,
		_('Error'),    (!v6markdel && v6errors) ? v6errors[4] : null,
		null, markdel ? E('em', _('IPv4 Interface is marked for deletion')) : null,
		null, v6markdel ? E('em', _('IPv6 Interface is marked for deletion')) : null,
		null, (changecount+v6changecount) ? E('a', {
			href: '#',
			click: L.bind(ui.changes.displayChanges, ui.changes)
		}, _('Interface has %d pending changes').format((changecount+v6changecount))) : null
	]);
}

return view.extend({
	poll_status: function(map, rtkWans) {
		var resolveZone = null;

		for (var j = 0; j < rtkWans.length; j++) {
			var sid=rtkWans[j].getName();
			var row = map.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(sid));
			if (row == null)
				continue;

			var dsc = row.querySelector('[data-name="_ifacestat"] > div');
			var box = row.querySelector('[data-name="_ifacebox"] .ifacebox-body');
			render_status(dsc, sid, rtkWans[j], false);

			var btn1 = row.querySelector('.cbi-section-actions .reconnect');
			var btn2 = row.querySelector('.cbi-section-actions .down');
			var ifc=rtkWans[j].networks.filter(function(n) { return n.getName() == '%s_v4'.format(sid) })[0];
			var ifc6=rtkWans[j].networks.filter(function(n) { return n.getName() == '%s_v6'.format(sid) })[0];
			var disabled = ifc ? !ifc.isUp() : (ifc6 ? !ifc6.isUp() : true);
			btn1.disabled = btn1.classList.contains('spinning') || btn2.classList.contains('spinning');
			btn2.disabled = btn1.classList.contains('spinning') || btn2.classList.contains('spinning') || disabled;
		}
		return Promise.all([ resolveZone, network.flushCache() ]);
	},

	load: function() {
		return Promise.all([
			network.getDSLModemType(),
			uci.changes(),
			L.resolveDefault(fs.stat('/usr/sbin/icwmpd'), {}),
			L.resolveDefault(fs.stat('/usr/sbin/obuspa'), {})
		]);
	},

	render: function(load_result) {
		var m, s, o, ss, oo;
		var hasIPv6=L.hasSystemFeature('ipv6') && (uci.get('network', 'rtkglobals', 'ipv6') == '1');
		var hasCWMP = (load_result[2].size > 0);
		var hasUSP = (load_result[3].size > 0);

		m = new form.Map('network');
		m.chain('dhcp');
		m.chain('firewall');
		if (hasCWMP == true) {
			m.chain('cwmp');
		}
		if (hasUSP == true) {
			m.chain('obuspa');
		}
		m.tabbed = true;

		s = m.section(form.GridSection, 'device', _('Interfaces'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add new interface...');
		s.cfgsections = function() {
			// only root device and related devices allowed to configuration, such as veip0, veip0.xx etc.
			var sections = uci.sections('network', 'device');
			var filterSections = sections.filter(function(e) { return e.name.indexOf(rootdev) != -1 });
			var section_ids = filterSections.sort(function(a, b) { return a.name > b.name }).map(function(s) { return s['.name'] });

			return section_ids;
		};

		// collect network/firwall information
		s.load = function() {
			return Promise.all([
				rtkNetwork.getNetwork(this.cfgsections()),
				firewall.getZones(),
			]).then(L.bind(function(data) {
				this.rtkWans = data[0];
				this.zones = data[1];
			}, this));
		};

		s.tab('general', _('General Settings'));
		s.tab('IPv4', _('IPv4 Settings'));
		s.tab('IPv6', _('IPv6 Settings'));
		s.tab('firewall', _('Firewall Settings'));

		// For new entry, we delete uci entry when user click dismiss.
		s.handleModalSave = function(/* ... */) {
			var mapNode = this.getPreviousModalMap(),
				prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

			// add addedSection in handleAdd for new entry,
			// we delete addedSection when user save it
			delete this.map.children[0].addedSection;

			return this.super('handleModalSave', arguments);
			//return this.super('handleModalSave', arguments)
			//	.then(function() { delete prevMap.addedSection });
		},

		// For new entry, we delete uci entry when user click dismiss.
		s.handleModalCancel = function(modalMap, ev, isSaving) {
			var config_name = this.uciconfig || this.map.config,
				mapNode = this.getPreviousModalMap(),
				prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

			if (prevMap.addedSection != null && !isSaving)
					this.map.data.remove(config_name, prevMap.addedSection);

			delete prevMap.addedSection;

			// add addedSection in handleAdd for new entry,
			// if user click dismiss, we delete previous entry
			if(this.map.children[0].addedSection != null){
				this.handleRemove(this.map.children[0].addedSection);
			}

			return this.super('handleModalCancel', arguments);
		},

		s.addModalOptions = function(s) {
			var hasIPv6=L.hasSystemFeature('ipv6') && (uci.get('network', 'rtkglobals', 'ipv6') == '1');
			var rtkwan=this.rtkWans.filter(function(n) { return n.getName() == s.section })[0];
			var sid_v4='%s_v4'.format(s.section);
			var zones = this.zones;
			var ifc=rtkwan.networks.filter(function(n) { return n.getName() == sid_v4 })[0];
			var dev=ifc ? ifc.getL3Device() || ifc.getDevice() : null;
			if(hasIPv6){
				var sid_v6='%s_v6'.format(s.section);
				var ifc6=rtkwan.networks.filter(function(n) { return n.getName() == sid_v6 })[0];
				var dev6=ifc6 ? ifc6.getL3Device() || ifc6.getDevice() : null;
			}
			var v4_proto_ipoe, v6_proto_ipoe;

			// general tab
			o = s.taboption('general', form.ListValue, 'type', _('VLAN'));
			o.value('', _('Disable'));
			o.value('8021q', _('VLAN (802.1q)'));
			o.default = '';
			o.validate = function(section_id, value) {
				var ifname = uci.get('network', section_id, 'ifname');
				var type=this.section.children.filter(function(o) { return o.option == 'type' })[0].formvalue(section_id);
				var vid=this.section.children.filter(function(o) { return o.option == 'vid' })[0].formvalue(section_id);
				var ret=isVlanUesd(ifname, section_id, type, vid);
				if(ret){
					return _('This VLAN setting is already used');
				}
				return true;
			};
			o.remove = function(section_id){
				var chmode=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_chmode' })[0].formvalue(section_id);
				rtkwan.setDevName(chmode, rootdev);
				return this.super('remove', arguments);
			};

			o = s.taboption('general', form.Value, 'vid', _('vid'));
			//o.readonly = true;
			o.depends('type', '8021q');
			o.datatype = 'range(1, 4095)';
			o.placeholder = _('Ex:1 ~ 4095');
			o.validate = function(section_id, value) {
				var ifname = uci.get('network', section_id, 'ifname');
				var ret=isVlanUesd(ifname, section_id, '8021q', value);
				if(ret){
					return _('This VLAN setting is already used');
				}
				return true;
			};
			o.write = function(section_id, formvalue){
				var chmode=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_chmode' })[0].formvalue(section_id);
				rtkwan.setDevName(chmode, '%s.%d'.format(rootdev, formvalue));
				return this.super('write', arguments);
			};

			o = s.taboption('general', form.ListValue, 'rtk_chmode', _('Channel Mode'));
			o.value('Bridged', _('Bridged'));
			o.value('IPoE', _('IPoE'));
			o.value('PPPoE', _('PPPoE'));
			o.default = 'IPoE';
			o.onchange = function(ev, section_id, formvalue){
				var type=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'type' })[0].formvalue(section_id);
				var vid=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'vid' })[0].formvalue(section_id);
				var ipmode=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_ipmode' })[0].formvalue(section_id);

				var devname = rootdev;
				if(type == '8021q' && vid){
					devname = '%s.%d'.format(rootdev, vid);
				}

				switch(formvalue){
				case 'Bridged':
					return Promise.all([
						network.deleteNetwork('%s_v4'.format(section_id)),
						hasIPv6 ? network.deleteNetwork('%s_v6'.format(section_id)) : null,
					]).then(L.bind(function(option, ev, section_id, formvalue) {
						uci.add('network', 'interface', sid_v4);
						uci.set('network', sid_v4, 'device', devname);
						uci.set('network', section_id, 'interface', [ sid_v4 ]);
						return option.super('write', arguments);
					}, this, this, ev, section_id, formvalue));
					break;
				case 'IPoE':
				case 'PPPoE':
					var intfArray=[];
					switch(ipmode){
					case 'IPv4':
						uci.set('network', section_id, 'ipv4', 1);
						uci.set('network', section_id, 'ipv6', 0);

						intfArray.push(sid_v4);
						uci.add('network', 'interface', sid_v4);
						uci.set('network', sid_v4, 'device', devname);
						uci.set('network', sid_v4, 'defaultroute', '1');
						uci.set('network', sid_v4, 'upstream', 1);	// mark this is upstream interface
						if(formvalue == 'PPPoE'){
							uci.set('network', sid_v4, 'ipv4', 1);
							uci.set('network', sid_v4, 'ipv6', 0);
							uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long
						}else{
							var reqopts=uci.get('network', sid_v4, 'reqopts');
							reqopts=(!reqopts)?[]:reqopts.filter(function(value){
								return value != "125";
							});

							reqopts.push("125");//Vendor specific information
							uci.set('network', sid_v4, 'reqopts', reqopts);
						}

						initDefaultZones(zones, sid_v4);
						break;
					case 'IPv6':
						uci.set('network', section_id, 'ipv4', 0);
						uci.set('network', section_id, 'ipv6', 1);

						if (hasIPv6) {
							intfArray.push(sid_v6);
							uci.add('network', 'interface', sid_v6);

							if(formvalue == 'PPPoE'){
								// change v6 interface to ppp interface
								uci.set('network', sid_v6, 'device', '@%s'.format(sid_v4));
							}else{
								uci.set('network', sid_v6, 'device', devname);
							}
							uci.set('network', sid_v6, 'defaultroute', '1');
							uci.set('network', sid_v6, 'iface_dslite', 0);	// IPv4/IPv6 don't need dslite
							uci.set('network', sid_v6, 'upstream', 1);	// mark this is upstream interface

							initDefaultZones(zones, sid_v6);

							if(formvalue == 'PPPoE'){
								// For PPPoE, it use sid_v4 to create PPPoE session
								intfArray.push(sid_v4);
								uci.add('network', 'interface', sid_v4);
								uci.set('network', sid_v4, 'device', devname);
								uci.set('network', sid_v4, 'proto', 'pppoe');
								uci.set('network', sid_v4, 'ipv4', 0);
								uci.set('network', sid_v4, 'ipv6', 1);
								uci.set('network', sid_v4, 'defaultroute', '1');
								uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long

								initDefaultZones(zones, sid_v4);
							}
						}
						break;
					case 'IPv4/IPv6':
						uci.set('network', section_id, 'ipv4', 1);
						uci.set('network', section_id, 'ipv6', 1);

						if (hasIPv6) {
							intfArray.push(sid_v4);
							uci.add('network', 'interface', sid_v4);
							uci.set('network', sid_v4, 'device', devname);
							uci.set('network', sid_v4, 'defaultroute', '1');
							uci.set('network', sid_v4, 'upstream', 1);	// mark this is upstream interface
							if(formvalue == 'PPPoE'){
								uci.set('network', sid_v4, 'ipv4', 1);
								uci.set('network', sid_v4, 'ipv6', 1);
								uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long
							}

							initDefaultZones(zones, sid_v4);

							intfArray.push(sid_v6);
							uci.add('network', 'interface', sid_v6);
							uci.set('network', sid_v6, 'device', '@%s'.format(sid_v4));
							uci.set('network', sid_v6, 'defaultroute', '1');
							uci.set('network', sid_v6, 'iface_dslite', 0);	// IPv4/IPv6 don't need dslite
							uci.set('network', sid_v6, 'upstream', 1);	// mark this is upstream interface

							initDefaultZones(zones, sid_v6);
						}
						break;
					}
					break;
				}
				uci.set('network', section_id, 'interface', intfArray);
				return this.super('onchange', arguments);
			};
			o.write=function(section_id, formvalue){
				var type=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'type' })[0].formvalue(section_id);
				var vid=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'vid' })[0].formvalue(section_id);
				var devname = rootdev;
				if(type == '8021q' && vid){
					devname = '%s.%d'.format(rootdev, vid);
				}
				rtkwan.setDevName(formvalue, devname);
				return this.super('write', arguments);
			};

			o = s.taboption('general', form.ListValue, 'rtk_ipmode', _('IP Protocol'));
			o.value('IPv4', _('IPv4'));
			o.value('IPv6', _('IPv6'));
			o.value('IPv4/IPv6', _('IPv4/IPv6'));
			o.depends({ rtk_chmode: "Bridged", "!reverse": true });
			o.default = "IPv4";
			o.write = function(section_id, formvalue){
				var type=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'type' })[0].formvalue(section_id);
				var vid=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'vid' })[0].formvalue(section_id);
				var chmode=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_chmode' })[0].formvalue(section_id);
				var intfArray=[];
				var rm_v4=false;
				var rm_v6=false;

				var devname = rootdev;
				if(type == '8021q' && vid){
					devname = '%s.%d'.format(rootdev, vid);
				}

				switch(formvalue){
				case 'IPv4':
					uci.set('network', section_id, 'ipv4', 1);
					uci.set('network', section_id, 'ipv6', 0);

					intfArray.push(sid_v4);
					uci.add('network', 'interface', sid_v4);
					uci.set('network', sid_v4, 'device', devname);
					uci.set('network', sid_v4, 'defaultroute', '1');
					uci.set('network', sid_v4, 'upstream', 1);	// mark this is upstream interface
					if(chmode == 'PPPoE'){
						uci.set('network', sid_v4, 'ipv4', 1);
						uci.set('network', sid_v4, 'ipv6', 0);
						uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long
					}else{
						var reqopts=uci.get('network', sid_v4, 'reqopts');
						reqopts=(!reqopts)?[]:reqopts.filter(function(value){
							return value != "125";
						});

						reqopts.push("125"); //Vendor specific information
						uci.set('network', sid_v4, 'reqopts', reqopts);
					}

					initDefaultZones(zones, sid_v4);
					rm_v6=true;
					break;
				case 'IPv6':
					uci.set('network', section_id, 'ipv4', 0);
					uci.set('network', section_id, 'ipv6', 1);

					if (hasIPv6) {
						intfArray.push(sid_v6);
						uci.add('network', 'interface', sid_v6);

						if(chmode == 'PPPoE'){
							// change v6 interface to ppp interface
							uci.set('network', sid_v6, 'device', '@%s'.format(sid_v4));
						}else{
							uci.set('network', sid_v6, 'device', devname);
						}
						uci.set('network', sid_v6, 'defaultroute', '1');
						uci.set('network', sid_v6, 'iface_dslite', 0);	// IPv4/IPv6 don't need dslite
						uci.set('network', sid_v6, 'upstream', 1);	// mark this is upstream interface

						initDefaultZones(zones, sid_v6);

						if(chmode == 'PPPoE'){
							// For PPPoE, it use sid_v4 to create PPPoE session
							intfArray.push(sid_v4);
							uci.add('network', 'interface', sid_v4);
							uci.set('network', sid_v4, 'device', devname);
							uci.set('network', sid_v4, 'proto', 'pppoe');
							uci.set('network', sid_v4, 'ipv4', 0);
							uci.set('network', sid_v4, 'ipv6', 1);
							uci.set('network', sid_v4, 'defaultroute', '1');
							uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long

							initDefaultZones(zones, sid_v4);
						}else{
							rm_v4=true;
						}
					}
					break;
				case 'IPv4/IPv6':
					uci.set('network', section_id, 'ipv4', 1);
					uci.set('network', section_id, 'ipv6', 1);

					if (hasIPv6) {
						intfArray.push(sid_v4);
						uci.add('network', 'interface', sid_v4);
						uci.set('network', sid_v4, 'device', devname);
						uci.set('network', sid_v4, 'defaultroute', '1');
						uci.set('network', sid_v4, 'upstream', 1);	// mark this is upstream interface
						if(chmode == 'PPPoE'){
							uci.set('network', sid_v4, 'ipv4', 1);
							uci.set('network', sid_v4, 'ipv6', 1);
							uci.set('network', sid_v4, 'pppname', 'ppp%s'.format(section_id));	// To avoid ppp interface too long
						}

						initDefaultZones(zones, sid_v4);

						intfArray.push(sid_v6);
						uci.add('network', 'interface', sid_v6);
						uci.set('network', sid_v6, 'device', '@%s'.format(sid_v4));
						uci.set('network', sid_v6, 'defaultroute', '1');
						uci.set('network', sid_v6, 'iface_dslite', 0);	// IPv4/IPv6 don't need dslite
						uci.set('network', sid_v6, 'upstream', 1);	// mark this is upstream interface

						initDefaultZones(zones, sid_v6);
					}
					break;
				}

				uci.set('network', section_id, 'rtk_ipmode', formvalue);
				uci.set('network', section_id, 'interface', intfArray);

				// setup realtek customization options
				var wan_mark=rtkwan.getRTKWanMark();
				var wan_mask=rtkwan.getRTKWanMask();
				var tabid=rtkwan.getRTKTableID();
				var rtkEnv=rtkwan.getRTKEnv();

				var conntype=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_conntype' })[0].formvalue(section_id);
				var found = conntype.find(function(name) { return name == "INTERNET"; });
				var defgw = found ? 1 : 0;

				for(var i=0; i<intfArray.length;i++){
					var isV4=true;

					if (intfArray[i] == sid_v4){
						isV4=true;
						uci.set('network', intfArray[i], 'ip4table', (defgw==1)?'main':tabid);
						uci.set('network', intfArray[i], 'ip4table2', tabid);
					}else{
						isV4=false;
						uci.set('network', intfArray[i], 'ip6table', (defgw==1)?'main':tabid);
						uci.set('network', intfArray[i], 'ip6table2', tabid);
					}

					// section_id should be "veip0_x"
					uci.set('network', intfArray[i], 'wan_mark', wan_mark);
					uci.set('network', intfArray[i], 'wan_mask', wan_mask);

					// setup policy rule for port mapping
					var rule_sid;
					// Mark Policy route
					rule_sid="RU_%s_mark".format(intfArray[i]);
					//uci.remove('network', rule_sid);
					uci.add('network', isV4?'rule':'rule6', rule_sid);
					uci.set('network', rule_sid, 'priority', rtkEnv.RULE_PRI.RULE_PRI_SKB_MARK_RT);
					uci.set('network', rule_sid, 'invert', 0);
					uci.set('network', rule_sid, 'mark', "0x%x/0x%x".format(wan_mark,wan_mask));
					uci.set('network', rule_sid, 'lookup', tabid);

					// oif Policy route
					rule_sid="RU_%s_oif".format(intfArray[i]);
					//uci.remove('network', rule_sid);
					uci.add('network', isV4?'rule':'rule6', rule_sid);
					uci.set('network', rule_sid, 'priority', rtkEnv.RULE_PRI.RULE_PRI_OIF_RT);
					uci.set('network', rule_sid, 'invert', 0);
					uci.set('network', rule_sid, 'out', intfArray[i]);
					uci.set('network', rule_sid, 'lookup', tabid);

					// defualt prohibit
					rule_sid="RU_%s_def".format(intfArray[i]);
					//uci.remove('network', rule_sid);
					uci.add('network', isV4?'rule':'rule6', rule_sid);
					uci.set('network', rule_sid, 'priority', rtkEnv.RULE_PRI.RULE_PRI_SKB_MARK_PROHIBIT);
					uci.set('network', rule_sid, 'invert', 0);
					uci.set('network', rule_sid, 'mark', "0x%x/0x%x".format(wan_mark,wan_mask));
					uci.set('network', rule_sid, 'action', 'prohibit');

					if(isV4==false){
						var ro_sid="RO_%s_wan_lla".format(intfArray[i]);
						//uci.remove('network', ro_sid);
						uci.add('network', isV4?'route':'route6', ro_sid);
						uci.set('network', ro_sid, 'target', 'fe80::');
						uci.set('network', ro_sid, 'netmask', 64);
						uci.set('network', ro_sid, 'metric', 256);
						uci.set('network', ro_sid, 'interface', intfArray[i]);
						uci.set('network', ro_sid, 'table', tabid);
						uci.set('network', ro_sid, 'proto', 'kernel');
					}
				}

				return Promise.all([
					(rm_v4==true)? network.deleteNetwork('%s_v4'.format(section_id)) : null,
					(hasIPv6 && rm_v6==true)? network.deleteNetwork('%s_v6'.format(section_id)) : null,
				]).then(L.bind(function(res) {
					if(res[0] == true){
						uci.remove('network', sid_v4);
					}
					if(res[1] == true){
						uci.remove('network', sid_v6);
					}
					return;
				}, this));
				//return this.super('write', arguments);
			};

			o = s.taboption('general', form.Flag, '_napt', _('NAPT'));
			o.enabled = 1;
			o.disabled = 0;
			o.depends({ rtk_chmode: "Bridged", "!reverse": true })
			o.write = function(section_id, formvalue){
				var zone=zones.find(function(zone) { return zone.getName() == 'nat'; });
				zone.addNetwork(sid_v4);
				zone.addNetwork(sid_v6);
				return;
			};
			o.remove = function(section_id){
				var zone=zones.find(function(zone) { return zone.getName() == 'nat'; });
				zone.deleteNetwork(sid_v4);
				zone.deleteNetwork(sid_v6);
				return;
			};
			o.cfgvalue = function(section_id) {
				var zone=zones.find(function(zone) { return zone.getName() == 'nat'; });
				var networks=zone?zone.getNetworks():[];
				if(networks)
					return networks.find(function(name) { return name == sid_v4 || name == sid_v6 ; })? 1:0;
				else
					return 0;
			};

			o = s.taboption('general', widgets.ZoneSelect, 'rtk_conntype', _('Connection Type'));
			o.depends({ rtk_chmode: "Bridged", "!reverse": true });
			o.nocreate = true;
			o.multiple = true;
			o.modalonly = true;
			//o.hideZone = 'lan wan nat';
			o.displayZone = 'INTERNET TR069 Voice Other';
			//o.rmempty=false;	// force choise conntype.
			o.default = 'INTERNET';
			o.validate=function(section_id, value) {
				if(value=='')
					return 'Choose at least one item.';
				return true;
			};
			o.write = function(section_id, formvalue){
				var defgw=false;
				var displayZones=L.toArray(this.displayZone);
				var ipmode = this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'rtk_ipmode' })[0].formvalue(section_id);
				for(var i=0; i < zones.length; i++){
					if(zones[i].getName() == 'wan'){
						zones[i].addNetwork(sid_v4);
						zones[i].addNetwork(sid_v6);
					}

					var found=false;
					if(!displayZones.find(function(name) { return name == zones[i].getName(); })){
						continue;
					}

					for(var j=0; j < formvalue.length; j++){
						if(zones[i].getName() == formvalue[j]){
							zones[i].addNetwork(sid_v4);
							zones[i].addNetwork(sid_v6);
							found = true;

							if(zones[i].getName() == "INTERNET"){
								defgw=true;
							}
							if (zones[i].getName() == "TR069") {
								if (hasCWMP == true) {
									if (ipmode == 'IPv6') {
										uci.set('cwmp', 'cpe', 'default_wan_interface', sid_v6);
									}
									else {
										uci.set('cwmp', 'cpe', 'default_wan_interface', sid_v4);
									}
								}
								if (hasUSP == true) {
									if (ipmode == 'IPv6') {
										uci.set('obuspa', 'global', 'interface', sid_v6);
									}
									else {
										uci.set('obuspa', 'global', 'interface', sid_v4);
									}
								}
							}
						}
					}

					if(!found){
						zones[i].deleteNetwork(sid_v4);
						zones[i].deleteNetwork(sid_v6);
					}
				}

				if(defgw){
					uci.set('network', sid_v4, 'ip4table', 'main');
					uci.set('network', sid_v6, 'ip6table', 'main');
				}else{
					uci.set('network', sid_v4, 'ip4table', uci.get('network', sid_v4, 'ip4table2'));
					uci.set('network', sid_v6, 'ip6table', uci.get('network', sid_v6, 'ip6table2'));
				}
			};
			o.cfgvalue = function(section_id) {
				var cfgvalue=[];
				for(var i=0; i < zones.length; i++){
					if(zones[i].getNetworks().find(function(name) { return name == sid_v4 || name == sid_v6 ; })){
						cfgvalue.push(zones[i].getName());
					}
				}
				return cfgvalue.length?cfgvalue:L.toArray(this.default);
			};

			// PPPoE Protocol always in sid_v4 section, if enable ipv6, it create another sid_v6 to control v6 protocol
			o = s.taboption('general', form.ListValue, 'rtk_ppp_authmode', _('Authentication Mode'));
			o.ucisection = sid_v4;
			o.depends({ rtk_chmode: "PPPoE" });
			o.value('auto', _('AUTO'));
			o.value('pap', _('PAP'));
			o.value('chap', _('CHAP'));
			o.value('mschap', _('MSCHAP'));
			o.value('mschapv2', _('MSCHAPv2'));
			o.value('eap', _('EAP'));
			o.default = 'auto';

			o = s.taboption('general', form.ListValue,'rtk_ppp_mppe',_('MPPE'));
			o.ucisection = sid_v4;
			o.datatype='string';
			o.depends({rtk_ppp_authmode: "mschap"});
			o.depends({rtk_ppp_authmode: "mschapv2"});
			o.value('nomppe',_('Disable MPPE'));
			o.value('mppe required,stateless',_('MPPE 40/56/128bit'));
			o.value('mppe required,no56,no128,stateless',_('MPPE 40bit'));
			o.value('mppe required,no40,no128,stateless',_('MPPE 56bit'));
			o.value('mppe required,no40,no56,stateless',_('MPPE 128bit'));
			o.default='mppe required,no40,no56,stateless';

			o = s.taboption('general', form.Value, 'username', _('UserName'));
			o.ucisection = sid_v4;
			o.depends({ rtk_chmode: "PPPoE" });
			o.validate = validateNull;

			o = s.taboption('general', form.Value, 'password', _('Password'));
			o.ucisection = sid_v4;
			o.password = true;
			o.depends({ rtk_chmode: "PPPoE" });
			o.validate = validateNull;

			o = s.taboption('general', form.ListValue,'_dial_type',_('Type'));
			o.ucisection = sid_v4;
			o.value('0',_('Continuous'));
			o.value('1',_('Connect on Demand'));
			o.depends({ rtk_chmode: "PPPoE" });
			o.cfgvalue = function(section_id) {
				var demand = uci.get('network', sid_v4, 'demand');
				if(!demand || demand == '0')
					return '0';
				else
					return '1';
			};
			o.write=o.remove=function(section_id, formvalue){};

			o = s.taboption('general', form.Value, 'demand', _('Idle Time(sec)'), _('Close inactive connection after the given amount of seconds, use 0 to persist connection'));
			o.ucisection = sid_v4;
			o.default='30';
			o.datatype    = 'uinteger';
			o.depends({_dial_type: '1'});

			o = s.taboption('general', form.Value, 'ac', _('Access Concentrator'), _('Leave empty to autodetect'));
			o.ucisection = sid_v4;
			o.placeholder = _('auto');
			o.depends({ rtk_chmode: "PPPoE" });

			o = s.taboption('general', form.Value, 'service', _('Service Name'), _('Leave empty to autodetect'));
			o.ucisection = sid_v4;
			o.placeholder = _('auto');
			o.depends({ rtk_chmode: "PPPoE" });

			o = s.taboption('general', form.Value, 'mtu', _('MTU'));
			o.depends({ rtk_chmode: "Bridged", "!reverse": true })
			o.placeholder = dev ? (dev.getMTU() || (dev6 && dev6.getMTU()) || '1500') : (dev6 ? (dev6.getMTU() || '1500') : '1500');
			o.datatype    = 'max(1500)';
			o.write = function(section_id, formvalue){
				uci.set('network', sid_v4, 'mtu', formvalue);

				if(hasIPv6){
					uci.set('network', sid_v6, 'mtu', formvalue);
				}

				return this.map.data.set(
						this.uciconfig || this.section.uciconfig || this.map.config,
						this.ucisection || section_id,
						this.ucioption || this.option,
						formvalue);
			};

			o = s.taboption('general', form.MultiValue, 'ports', _('Port Mapping'));
			o.ucisection="%s_pmap".format(s.section);
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

			o.write = o.remove = function(section_id, formvalue){
				return rtkwan.setPortMapping(formvalue);
			};
			o.onchange = function(ev, section_id, value){
				reload_fw = true;
			};

			// IPv4 tab, set IPv4 interface section directly
			v4_proto_ipoe = s.taboption('IPv4', form.ListValue, 'v4_proto_ipoe', _('IPv4 Address Mode'));
			v4_proto_ipoe.ucisection = sid_v4;
			v4_proto_ipoe.value('dhcp', 'DHCP');
			v4_proto_ipoe.value('static', 'Static');
			v4_proto_ipoe.default = 'dhcp';
			v4_proto_ipoe.depends({rtk_ipmode: "IPv4", rtk_chmode: "IPoE"});
			if (hasIPv6) {
				v4_proto_ipoe.depends({rtk_ipmode: "IPv4/IPv6", rtk_chmode: "IPoE"});
			}
			v4_proto_ipoe.write = function(section_id, formvalue){
				uci.set('network', sid_v4, 'proto', formvalue);
				if (formvalue == 'static')
					uci.set('network', sid_v4, 'force_link', '0');
			};
			v4_proto_ipoe.remove = function(section_id) {
				if(uci.get('network', section_id, 'rtk_chmode') == 'PPPoE'){
					uci.set('network', sid_v4, 'proto', 'pppoe');
				}
			};
			v4_proto_ipoe.cfgvalue = function(section_id) {
				return uci.get('network', sid_v4, 'proto');
			};

			// IPv4 - Static
			o = s.taboption('IPv4', form.Value, 'ipaddr', _('IPv4 Address'));
			o.ucisection = sid_v4;
			o.datatype = 'ip4addr("nomask")';
			o.placeholder = _('Add Static IP address…');
			o.depends({ v4_proto_ipoe: "static" });
			o.validate = validateNull;

			o = s.taboption('IPv4', form.Value, 'netmask', _('IPv4 Netmask'));
			o.ucisection = sid_v4;
			o.placeholder = _('Ex:255.255.255.0');
			o.depends({ v4_proto_ipoe: "static" });
			o.validate = validateNetmask;

			o = s.taboption('IPv4', form.Value, 'gateway', _('IPv4 Gateway'));
			o.ucisection = sid_v4;
			o.datatype = 'ip4addr("nomask")';
			o.placeholder = _('Add Gateway address…');
			o.depends({ v4_proto_ipoe: "static" });
			o.validate = validateNull;

			// IPv4 DNS Setting
			o = s.taboption('IPv4', form.Flag, 'peerdns', _('Request DNS'));
			o.ucisection = sid_v4;
			o.depends({ v4_proto_ipoe: "dhcp" });
			o.default = o.enabled;

			o = s.taboption('IPv4', form.DynamicList, 'dns', _('Use Custom DNS Servers'));
			o.ucisection = sid_v4;
			o.depends({ v4_proto_ipoe: "dhcp", peerdns: "0" });
			o.depends({ v4_proto_ipoe: "static" });
			o.datatype='ip4addr("nomask")';
			o.cast     = 'string';
			o.placeholder = _('Ex:8.8.8.8');

			if (hasIPv6) {
				// IPv6 tab, set IPv6 interface section directly
				v6_proto_ipoe = s.taboption('IPv6', form.ListValue, 'v6_proto_ipoe', _('IPv6 Address Mode'));
				v6_proto_ipoe.ucisection = sid_v6;
				v6_proto_ipoe.value('static', 'Static');
				v6_proto_ipoe.value('slaac', 'SLAAC');
				v6_proto_ipoe.value('dhcpv6', 'DHCPv6');
				v6_proto_ipoe.value('auto', _('AUTO DETECT'));
				v6_proto_ipoe.default = 'auto';
				v6_proto_ipoe.depends({rtk_ipmode: "IPv6"});
				v6_proto_ipoe.depends({rtk_ipmode: "IPv4/IPv6"});
				v6_proto_ipoe.write = function(section_id, formvalue){
					//reqaddress,reqprefix
					switch(formvalue){
					case 'slaac':
						uci.set('network', sid_v6, 'proto', 'dhcpv6');
						uci.set('network', sid_v6, 'reqaddress', 'none');	// Disable Stateful DHCPv6 Address
						break;
					case 'dhcpv6':
						uci.set('network', sid_v6, 'proto', 'dhcpv6');
						uci.set('network', sid_v6, 'reqaddress', 'force');	// Enable Stateful DHCPv6 Address
						break;
					case 'auto':
						uci.set('network', sid_v6, 'proto', 'dhcpv6');
						uci.set('network', sid_v6, 'reqaddress', 'auto');	// RTK Feature, Auto Detect M/O bit
						break;
					case 'static':
						uci.set('network', sid_v6, 'proto', 'static');
						uci.set('network', sid_v6, 'force_link', '0');
						break;
					}
				};
				v6_proto_ipoe.remove = function(section_id) {
					if(uci.get('network', section_id, 'rtk_chmode') == 'PPPoE'){
						uci.set('network', sid_v6, 'proto', 'pppoe');
					}
				};
				v6_proto_ipoe.cfgvalue = function(section_id) {
					var proto=uci.get('network', sid_v6, 'proto');

					if(proto == 'static'){
						return 'static'
					}else if(proto == 'dhcpv6'){
						var req_iana=uci.get('network', sid_v6, 'reqaddress');
						if(req_iana == 'none'){
							return 'slaac';
						}else if(req_iana == 'force'){
							return 'dhcpv6';
						}else if(req_iana == 'auto'){
							return 'auto';
						}
					}
				};

				o = s.taboption('IPv6', form.DynamicList, 'ip6addr', _('IPv6 Address'));
				o.ucisection = sid_v6;
				o.datatype = 'ip6addr';
				o.placeholder = _('Ex: 2001:0:1:2::3/64');
				o.depends({ v6_proto_ipoe: "static" });


				o = s.taboption('IPv6', form.Value, 'ip6gw', _('IPv6 Gateway'));
				o.ucisection = sid_v6;
				o.datatype = 'ip6addr("nomask")';
				o.placeholder = _('Ex: 2001:0:1:2::1');
				o.depends({ v6_proto_ipoe: "static" });

				o = s.taboption('IPv6', form.Flag, 'reqprefix', _('Request Prefix'));
				o.ucisection = sid_v6;
				o.disabled='no';
				o.enabled='auto';
				o.default = o.enabled;
				o.rmempty=false;	// keep option even user disable.
				o.depends({ v6_proto_ipoe: "slaac" });
				o.depends({ v6_proto_ipoe: "dhcpv6" });
				o.depends({ v6_proto_ipoe: "auto" });

				// IPv6 DNS Setting
				o = s.taboption('IPv6', form.Flag, 'v6peerdns', _('Request DNS'));
				o.ucisection = sid_v6;
				o.depends({ v6_proto_ipoe: "slaac" });
				o.depends({ v6_proto_ipoe: "dhcpv6" });
				o.depends({ v6_proto_ipoe: "auto" });
				o.default = o.enabled;
				o.write = function(section_id, formvalue){
					uci.set('network', sid_v6, 'peerdns', formvalue);
				}
				o.remove = function(section_id) {
					uci.unset('network', sid_v6, 'peerdns');
				}
				o.cfgvalue = function(section_id) {
					return uci.get('network', sid_v6, 'peerdns');
				};

				o = s.taboption('IPv6', form.DynamicList, 'v6dns', _('Use Custom DNS Servers'));
				o.ucisection = sid_v6;
				o.depends({ v6_proto_ipoe: "slaac", v6peerdns: "0" });
				o.depends({ v6_proto_ipoe: "dhcpv6", v6peerdns: "0" });
				o.depends({ v6_proto_ipoe: "auto", v6peerdns: "0" });
				o.depends({ v6_proto_ipoe: "static" });
				o.datatype='ip6addr("nomask")'
				o.cast = 'string';
				o.placeholder = _('Ex:2001:4860:4860::8888');
				o.write = function(section_id, formvalue){
					uci.set('network', sid_v6, 'dns', formvalue);
				}
				o.remove = function(section_id) {
					uci.unset('network', sid_v6, 'dns');
				}
				o.cfgvalue = function(section_id) {
					return uci.get('network', sid_v6, 'dns');
				};

				o = s.taboption('IPv6', form.Flag, 'disabled', _('DS-Lite'));
				o.ucisection = "ds_%s".format(s.section);
				o.rmempty=false;	// keep option even user disable.
				o.disabled = 1;
				o.enabled = 0;
				o.default = o.disabled;
				o.depends({rtk_ipmode: "IPv6"});
				o.write = function(section_id, formvalue){
					var sid = "ds_%s".format(section_id);

					var sections = uci.sections('network', 'interface');
					var intfs = sections.filter(function(e) { return e['.name'] == sid });
					if(intfs.length == 0){
						uci.add('network', 'interface', sid);
						uci.set('network', sid, 'proto', 'dslite');
						uci.set('network', sid, 'tunlink', sid_v6);
					}

					uci.set('network', sid, 'disabled', formvalue);
				};
				o.remove = function(section_id){
					var sid = "ds_%s".format(section_id);
					uci.remove('network', sid);
				};

				o = s.taboption('IPv6', form.ListValue, 'dslite_mode', _('DS-Lite Address Mode'));
				o.ucisection = sid_v6;
				o.value('DHCPv6', 'DHCPv6');
				o.value('Static', 'Static');
				o.depends({disabled: 0});
				o.write = function(section_id, formvalue){
					uci.set('network', sid_v6, 'dslite_mode', formvalue);
				};
				o.remove = function(section_id){
					uci.unset('network', sid_v6, 'dslite_mode');
				};

				o = s.taboption('IPv6', form.Value, 'peeraddr', _('DS-Lite Peer Address'));
				o.ucisection = "ds_%s".format(s.section);
				o.placeholder = _('2001:4860:4860::8888');
				o.depends({dslite_mode: "Static"});
				o.remove = function(section_id){
					uci.unset('network', "ds_%s".format(section_id), 'peeraddr');
				};
			}
		},

		s.renderRowActions = function(section_id) {
			var tdEl = this.super('renderRowActions', [ section_id, _('Edit') ]);
			var dynamic, disabled, net;

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

		s.handleAdd = function(ev) {
			var zones = this.zones;
			var nameval=null;
			for (var i = 0;; i++) {
				nameval = '%s_%d'.format(rootdev, i);
				if (uci.get('network', nameval) == null){
					break;
				}
			}
			return this.map.save(function() {
				var section_id = uci.add('network', 'device', nameval);
				uci.set('network', section_id, 'name', rootdev);
				uci.set('network', section_id, 'ifname', rootdev);

				var sid_v4="%s_v4".format(nameval);
				uci.add('network', 'interface', sid_v4);
				uci.set('network', sid_v4, 'device', rootdev);
				uci.set('network', sid_v4, 'proto', 'dhcp');
				uci.set('network', sid_v4, 'defaultroute', '1');
				uci.set('network', sid_v4, 'upstream', 1);	// mark this is upstream interface
				initDefaultZones(zones, sid_v4);

				// it used to remove device entry when user click dismiss for new entry.
				// detail please check handleModalSave and handleModalCancel
				m.children[0].addedSection = section_id;
			}).then(L.bind(m.children[0].renderMoreOptionsModal, m.children[0], nameval));
		};

		s.handleRemove = function(section_id, ev) {
			var hasIPv6=L.hasSystemFeature('ipv6') && (uci.get('network', 'rtkglobals', 'ipv6') == '1');
			return Promise.all([
				network.deleteNetwork('%s_v4'.format(section_id)),
				hasIPv6 ? network.deleteNetwork('%s_v6'.format(section_id)) : null,
				rtkNetwork.deleteNetwork(section_id)
			]).then(L.bind(function(section_id, ev) {
				return form.GridSection.prototype.handleRemove.apply(this, [section_id, ev]);
			}, this, section_id, ev));
		}

		// show interface setting
		o = s.option(form.DummyValue, '_ifacebox');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			var zone = this.section.zones.filter(function(z) { return z.getName() == 'wan' });
			var conntype_zones=this.section.zones.filter(function(z) { return z.getName() == "INTERNET" || z.getName() == "TR069" || z.getName() == "Voice" || z.getName() == "Other"; });
			var list=[];
			for(var i=0; i<conntype_zones.length;i++){
				if(conntype_zones[i].getNetworks().find(function(name) { return name == "%s_v4".format(section_id) || name == "%s_v6".format(section_id) ; })){
					list.push(conntype_zones[i].getName());
				}
			}
			var node = E('div', { 'class': 'ifacebox' }, [
				E('div', {	'class': 'ifacebox-head',
							'style': firewall.getZoneColorStyle(zone[0])
				}, E('strong', section_id)),
				L.itemlist(E('div', {'class': 'ifacebox-body', 'id': '%s-ifc-devices'.format(section_id), 'data-network': section_id}),
					[
						_('VLAN'), (!uci.get('network', section_id, 'type'))?'disable':uci.get('network', section_id, 'vid'),
						_('IP Protocol'), uci.get('network', section_id, 'rtk_ipmode'),
						_('Channel Mode'), uci.get('network', section_id, 'rtk_chmode'),
						_('Connection Type'), list.length ? list : "Other"
					])
			]);
			return node;
		};

		o = s.option(form.DummyValue, '_ifacestat');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			var node = E('div', { 'id': '%s-ifc-description'.format(section_id) });
			var rtkwan=this.section.rtkWans.filter(function(n) { return n.getName() == section_id })[0];
			render_status(node, section_id, rtkwan, false);
			return node;
		};

		return m.render().then(L.bind(function(m, nodes) {
			poll.add(L.bind(function() {
				var section_ids = m.children[0].cfgsections(),
					tasks = [];

				for (var i = 0; i < section_ids.length; i++) {
					var row = nodes.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(section_ids[i])),
						dsc = row.querySelector('[data-name="_ifacestat"] > div'),
						btn1 = row.querySelector('.cbi-section-actions .reconnect'),
						btn2 = row.querySelector('.cbi-section-actions .down');

					if (dsc.getAttribute('reconnect') == '') {
						dsc.setAttribute('reconnect', '1');
						tasks.push(fs.exec('/sbin/ifup', ['%s_v4'.format(section_ids[i])]).catch(function(e) {
							ui.addNotification(null, E('p', e.message));
						}));
						if (hasIPv6) {
							tasks.push(fs.exec('/sbin/ifup', ['%s_v6'.format(section_ids[i])]).catch(function(e) {
								ui.addNotification(null, E('p', e.message));
							}));
						}
					}else if (dsc.getAttribute('disconnect') == '') {
						dsc.setAttribute('disconnect', '1');
						tasks.push(fs.exec('/sbin/ifdown', ['%s_v4'.format(section_ids[i])]).catch(function(e) {
							ui.addNotification(null, E('p', e.message));
						}));
						if (hasIPv6) {
							tasks.push(fs.exec('/sbin/ifdown', ['%s_v6'.format(section_ids[i])]).catch(function(e) {
								ui.addNotification(null, E('p', e.message));
							}));
						}
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
					.then(L.bind(rtkNetwork.getNetworks, rtkNetwork))
					.then(L.bind(this.poll_status, this, nodes));
			}, this), 3);

			return nodes;
		}, this, m));
	}
});
