'use strict';
'require view';
'require uci';
'require form';
'require rpc';
'require network';
'require ui';
'require validation';
'require tools.rtk.widgets as rtkwidgets';

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

var callDUIDHints = rpc.declare({
        object: 'luci-rpc',
        method: 'getDUIDHints',
        expect: { '': {} }
});

var callLuciDHCPLeases = rpc.declare({
	object: 'luci-rpc',
	method: 'getDHCPLeases',
	expect: { '': {} }
});

function handleCreateStaticLease6(lease, ev) {
	ev.currentTarget.classList.add('spinning');
	ev.currentTarget.disabled = true;
	ev.currentTarget.blur();

	var cfg = uci.add('dhcp', 'host'),
		ip6arr = lease.ip6addr ? validation.parseIPv6(lease.ip6addr) : null;

	if(!ip6arr){
		ip6arr = lease.ip6addrs[0].split("/")[0] ? validation.parseIPv6(lease.ip6addrs[0].split("/")[0]) : null;
	}

	if (ip6arr){
		uci.set('dhcp', cfg, 'name', lease.hostname);
		uci.set('dhcp', cfg, 'duid', lease.duid.toUpperCase());
		uci.set('dhcp', cfg, 'mac', lease.macaddr);
		uci.set('dhcp', cfg, 'hostid', (ip6arr[6] * 0xFFFF + ip6arr[7]).toString(16));
	}

	return uci.save()
		.then(L.bind(L.ui.changes.init, L.ui.changes))
		.then(L.bind(L.ui.changes.displayChanges, L.ui.changes));
};

return view.extend({
	load: function() {
		return Promise.all([
			callHostHints(),
			callDUIDHints()
		]);
	},

	handleShowClients: function(ev) {
		return Promise.all([
			callLuciDHCPLeases(),
			network.getHostHints()
		]).then(function(data) {
			var leases6 = Array.isArray(data[0].dhcp6_leases) ? data[0].dhcp6_leases : [];
			var machints = data[1].getMACHints(false);
			var hosts = uci.sections('dhcp', 'host');
			var isReadonlyView = !L.hasViewPermission();
			var isDUIDStatic={};

			for (var i = 0; i < hosts.length; i++) {
				var host = hosts[i];

				if (host.duid) {
					var duid = host.duid.toUpperCase();
					isDUIDStatic[duid] = true;
				}
			};

			var table6 = E('table', { 'class': 'table leases6' }, [
				E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'th' }, _('Host')),
					E('th', { 'class': 'th' }, _('IPv6 Address')),
					E('th', { 'class': 'th' }, _('DUID')),
					E('th', { 'class': 'th' }, _('Lease Time Remaining')),
					isReadonlyView ? E([]) : E('th', { 'class': 'th cbi-section-actions' }, _('Static Lease'))
				])
			]);

			cbi_update_table(table6, leases6.map(L.bind(function(lease) {
				var exp, rows;

				if (lease.expires === false)
					exp = E('em', _('unlimited'));
				else if (lease.expires <= 0)
					exp = E('em', _('expired'));
				else
					exp = '%t'.format(lease.expires);

				var hint = lease.macaddr ? machints.filter(function(h) { return h[0] == lease.macaddr })[0] : null,
					host = null;

				if (hint && lease.hostname && lease.hostname != hint[1] && lease.ip6addr != hint[1])
					host = '%s (%s)'.format(lease.hostname, hint[1]);
				else if (lease.hostname)
					host = lease.hostname;
				else if (hint)
					host = hint[1];

				rows = [
					host || '-',
					lease.ip6addrs ? lease.ip6addrs.join(' ') : lease.ip6addr,
					lease.duid,
					exp
				];

				if (!isReadonlyView && lease.duid != null) {
					var duid = lease.duid.toUpperCase();
					rows.push(E('button', {
						'class': 'cbi-button cbi-button-apply',
						'click': L.bind(handleCreateStaticLease6, this, lease),
						'disabled': isDUIDStatic[duid]
					}, [ _('Set Static') ]));
				}

				return rows;
			}, this)), E('em', _('There are no active leases')));

			ui.showModal(_('Active DHCP Clients'), [
				table6,
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

	render: function(data) {
		if (L.hasSystemFeature('odhcpd')){
			var m, s, o;
			var hosts = data[0];
			var duids = data[1];

			m = new form.Map('dhcp', _("DHCPv6 Configuration"));
			s = m.section(form.TypedSection, 'dhcp');
			s.anonymous = true;
			s.filter = function(section_id) {
				if(section_id == "lan")
					return true;

				return false;
			};

			o = s.option(form.ListValue, 'dhcpv6', _('Mode'));
			o.datatype = 'string';
			o.value("disabled", _("Disable"));
			o.value("relay", _("Relay"));
			o.value("server", _("Server"));
			o.default = "server";

			o = s.option(form.DynamicList, 'ntp', _('NTP Server'));
			o.depends({ dhcpv6: "server" });
			o.placeholder = '2001:4860:4860::8888';

			o = s.option(rtkwidgets.NetworkSelect, '_relayIntf', _("Up Streaming Interface"));
			o.depends("dhcpv6", "relay");
			o.valueType="IPv6";
			o.ipmodefilter="IPv6";
			o.ignoreBridge=1;
			o.nocreate=true;
			o.rmempty=false;	// Force choise for o.multiple=true
			o.write=function(section_id, formvalue) {
				var sid="relay_%s".format(section_id);
				var entry=uci.get('dhcp', sid);
				console.log(entry);
				if(!entry){
					uci.add('dhcp', 'dhcp', sid);
				}

				uci.set('dhcp', sid, 'interface', formvalue);
				uci.set('dhcp', sid, 'dhcpv6', 'relay');
				uci.set('dhcp', sid, 'master', '1');
			};
			o.remove=function(section_id) {
				var sid="relay_%s".format(section_id);
				uci.remove('dhcp', sid);
			};
			o.cfgvalue = function(section_id) {
				var sid="relay_%s".format(section_id);
				return uci.get('dhcp', sid, 'interface');
			};

			o = s.option(form.Button, '_clibtn', _('Show Client'));
			o.depends({dhcpv6:"server"});
			o.inputstyle = 'action important';
			o.inputtitle = _('Show Client');
			o.onclick = this.handleShowClients;

			/* Mac-base Assignment */
			s = m.section(form.GridSection, 'host', _('Static Leases'));
			s.anonymous = true;
			s.addremove = true;
			s.addbtntitle = _('Add');
			s.cfgsections = function() {
				// only ipv6 host section allowed to configuration.
				var sections = uci.sections('dhcp', 'host');
				var filterSections = sections.filter(function(e) { return e.hostid ? true: false || e.duid ? true: false; });
				var section_ids = filterSections.map(function(s) { return s['.name'] });
				return section_ids;
			};

			o = s.option(form.Value, 'mac', _('MAC Address'));
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

			Object.keys(hosts).forEach(function(mac) {
				var hint = hosts[mac].name || hosts[mac].ipv6;
				o.value(mac, hint ? '%s (%s)'.format(mac, hint) : mac);
			});

			o = s.option(form.Value, 'duid', _('DUID'));
			o.datatype = 'and(rangelength(20,36),hexstring)';
			Object.keys(duids).forEach(function(duid) {
				o.value(duid.toUpperCase(), '%s (%s)'.format(duid, duids[duid].hostname || duids[duid].macaddr || duids[duid].ip6addr || '?'));
			});
			o.validate = function(section, value) {
				if (value == null || value == '')
					return true;

				var leases = uci.sections('dhcp', 'host');
				for (var i = 0; i < leases.length; i++)
					if (leases[i]['.name'] != section && leases[i].duid == value)
						return _('The DUID(%s) is already used by another static lease').format(value);

				return true;
			};

			o = s.option(form.Value, 'hostid', _('Host ID'));
			o.datatype = 'string';
			o.validate = function(section, value) {
				var duid = this.section.formvalue(section, 'duid'),
					mac = this.section.formvalue(section, 'mac');

				if ((duid == null || duid == '') && (mac == null || mac == ''))
					return _('One of MAC of DUID must be specified!');

				if(value.length > 16 ||(!value.match(/^([0-9a-fA-F]+)$/))){
					return _('It should be hex string and maximun length is 16');
				}

				return true;
			};
			o.placeholder = '0000abcd43219874';

			o = s.option(form.Value, 'leasetime', _('Lease Time'));
			o.validate = validateNull;
			o.optional = true;
			o.default = '12h';

			return m.render();
		}else{
			return null;
		}
	}
});
