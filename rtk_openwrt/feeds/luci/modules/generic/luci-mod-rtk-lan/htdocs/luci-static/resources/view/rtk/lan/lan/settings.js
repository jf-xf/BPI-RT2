'use strict';
'require view';
'require form';
'require uci';
'require fs';
'require validation';
'require network';
'require tools.rtk.widgets as rtkwidgets';
'require rtk.network as rtkNetwork';

var reload_lan=false;
var reload_fw=false;
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

document.addEventListener("uci-applied", applyPendingTask);
function applyPendingTask(){
	var tasks = [];
	if(reload_lan){
		tasks.push(fs.exec('/sbin/ifup', [ 'lan' ]).catch(function(e) {
			ui.addNotification(null, E('p', e.message));
		}));
	}

	if(reload_fw){
		tasks.push(fs.exec('/etc/init.d/firewall', [ 'reload' ]).catch(function(e) {
			ui.addNotification(null, E('p', e.message));
		}));
	}

	return Promise.all(tasks).then(function() {
		reload_lan = false;
		reload_fw = false;
	});
}

return view.extend({
	render: function(load_result) {
		var m, s, o;
		var hasIPv6=L.hasSystemFeature('ipv6');

		m = new form.Map('network', 'LAN Interface Settings', 'This page is used to configure the LAN interface of your Device. Here you may change the setting for IP addresses, subnet mask, etc..');
		m.chain("dhcp");

		s = m.section(form.NamedSection, 'lan');

		o = s.option(form.DummyValue, '_name', _('Interface Name'));
		o.modalonly = false;
		o.cfgvalue = function(section_id) {
			return Promise.all([
				network.getNetwork(section_id)
			]).then(function(ret){
				return ret[0].getL3Device().device;
			});
		};

		// IPv4 - Static
		o = s.option(form.Value, 'ipaddr', _('IPv4 Address'));
		o.datatype = 'ip4addr("nomask")';
		o.placeholder = _('Add Static IP address...');
		o.validate = validateNull;

		o = s.option(form.Value, 'netmask', _('IPv4 Netmask'));
		o.placeholder = _('Ex:255.255.255.0');
		o.validate = validateNetmask;

		o = s.option(form.ListValue, "addr_gen_mode", _("IPv6 Link-Local Address Mode"));
		o.ucisection = 'D_lan';
		o.value('0',_('Auto'));
		o.value('1',_('Static'));
		o.default = 'Auto';

		o = s.option(form.DynamicList, 'ip6addr', _('IPv6 Link-Local Address'));
		o.depends("addr_gen_mode", "1");
		o.datatype='ip6addr'

		o = s.option(form.ListValue, "dnsv6mode", _("IPv6 DNS Mode"));
		o.ucisection = 'rtkglobals';
		o.value('proxy',_('HGW Proxy'));
		o.value('wan',_('WAN Connection'));
		o.value('static',_('Static'));
		o.default = 'proxy';

		o = s.option(rtkwidgets.NetworkSelect, 'dnsv6wan', _("WAN Interface"));
		o.ucisection = 'rtkglobals';
		o.depends("dnsv6mode", "wan");
		o.valueType="IPv6";
		o.ipmodefilter="IPv6";
		o.ignoreBridge=1;
		o.nocreate=true;
		//o.multiple = true;
		o.rmempty=false;	// Force choise for o.multiple=true
		o.write = function(section_id, formvalue){
			Promise.all([
				network.getNetworks()
			]).then(function(res){
				var networks=res[0];
				var formdata=[];
				var v6dnss=[];
				if(Array.isArray(formvalue))
					formdata=formvalue;
				else
					formdata=[ formvalue ];
				for(var i=0; i < networks.length; i++){
					for(var j=0; j < formdata.length; j++){
						if(networks[i].sid == formdata[j] ){
							v6dnss=v6dnss.concat(networks[i].getDNS6Addrs());
						}
					}
				}

				if(v6dnss.length){
					uci.set("dhcp",	"lan6", "dns", v6dnss);
				}
			});
			return this.super('write', arguments);
		};

		o = s.option(form.DynamicList, '_dns', _('Static IPv6 DNS servers'));
		o.depends("dnsv6mode", "static");
		o.datatype='ip6addr("nomask")'
		o.cast = 'string';
		o.placeholder = _('Ex:2001:4860:4860::8888');
		o.write = function(section_id, formvalue){
			return uci.set("dhcp",	"lan6", "dns", formvalue);
		};
		o.remove = function(section_id){
			var dnsmode=this.section.children.filter(function(o) { return (o.ucioption || o.option) == 'dnsv6mode' })[0].formvalue(section_id);
			if(dnsmode=='proxy')
				return uci.unset("dhcp", "lan6", "dns");
		};
		o.cfgvalue = function(section_id){
			return uci.get("dhcp", "lan6", "dns");
		};

		// we use following skip to avoide ip6class unset when pdmode!=wanpd
		// 1. '_ip6class' and NOT set ucioption='ip6class'
		// 2. rewrite cfgvalue and write function
		o = s.option(form.ListValue, "pdmode", _("Prefix Mode"));
		o.ucisection = 'rtkglobals';
		o.value('wanpd',_('WAN Delegated'));
		o.value('static',_('Static'));
		o.default = 'wanpd';
		o.write = function(section_id, formvalue){
			if (formvalue == 'static'){
				// add local for ULA
				uci.set('network', 'lan', 'ip6class', [ 'lan', 'local' ]);
			}else{
				// add local for ULA
				uci.set('network', 'lan', 'ip6class', [ 'local' ]);
				uci.unset('network', 'lan', 'ip6prefix');
			}

			return this.map.data.set(
				this.uciconfig || this.section.uciconfig || this.map.config,
				this.ucisection || section_id,
				this.ucioption || this.option,
				formvalue);
		};

		o = s.option(form.Value, 'ip6prefix', _('Static Prefix'));
		o.depends("pdmode", "static");
		o.placeholder = _('Enter Static Prefix, Ex:2000:aaaa:bbbb:cccc::/60');
		o.validate = function(section_id, value){
			var m = value.match(/^([0-9a-fA-F:.]+)(?:\/(\d{1,2}))?$/);
			if(m && validation.parseIPv6(m[1]) && m[2] >= 48 && m[2] <= 64){
				return true;
			}
			return _('Invalid IPv6 prefix, prefix length should be 48-64.');
		};

		// we use following skip to avoide ip6class unset when pdmode!=wanpd
		// 1. '_ip6class' and NOT set ucioption='ip6class'
		// 2. rewrite cfgvalue and write function
		o = s.option(rtkwidgets.NetworkSelect, '_ip6class', _("WAN Interface"));
		o.depends("pdmode", "wanpd");
		o.valueType="IPv6";
		o.ipmodefilter="IPv6";
		o.ignoreBridge=1;
		o.nocreate=true;
		o.multiple = true;
		//o.rmempty=false;	// Force choise for o.multiple=true
		o.cfgvalue = function(section_id) {
			return uci.get(
				this.uciconfig || this.section.uciconfig || this.map.config,
				this.ucisection || section_id,
				'ip6class');
		};
		o.write = function(section_id, formvalue){
			var ip6class = [];
			ip6class.push('local');	// add local for ULA
			if(formvalue){
				var values = L.toArray(formvalue);
				for(var i = 0; i < values.length; i++){
					ip6class.push(values[i]);
				}
			}

			if(ip6class.length > 0){
				uci.set('network', section_id, 'ip6class', ip6class);
			}else{
				uci.unset('network', section_id, 'ip6class');
			}
		};
		o.onchange = function(ev, section_id, value){
			reload_lan = true;
		};

		o = s.option(form.Value, 'ula_prefix', _('ULA Prefix'));
		o.ucisection = 'globals';
		o.validate = function(section_id, value){
			if(value){
				var m = value.match(/^([0-9a-fA-F:.]+)(?:\/(\d{1,2}))?$/);
				if(m && validation.parseIPv6(m[1]) && m[2] >= 48 && m[2] <= 64){
					value=m[1].split(':');
					if (value[0].search(/^fd/) != -1){
						return true;
					}else{
						return _('Invalid IPv6 prefix, ula prefix should start from "fd"');
					}
				}
			}else{
				return true;
			}

			return _('Invalid IPv6 prefix, prefix length should be 48-64.');
		};
		o.onchange = function(ev, section_id, value){
			reload_lan = true;
		};

		o = s.option(form.Value, 'mtu', _('MTU'));
		o.ucisection = 'D_lan';
		o.placeholder = '1500';
		o.datatype = 'max(1500)';

		o = s.option(form.Flag, 'wlan_block', _('Ethernet to Wireless Block'));
		o.ucisection = 'rtkglobals';
		o.default = o.disabled;
		o.rmempty = false;
		o.onchange = function(ev, section_id, value){
			reload_fw = true;
		};

		return m.render();
	}
});
