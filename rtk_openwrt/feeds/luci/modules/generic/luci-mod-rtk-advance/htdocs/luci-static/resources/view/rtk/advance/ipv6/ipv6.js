'use strict';
'require view';
'require uci';
'require form';
'require fs';

function disable_ipv6(section_id, value){
	var devname=uci.get('network', section_id, 'device');
	if(devname){
		// using device method, go through all device section
		var devs=uci.sections('network', 'device');
		for(var k=0;k<devs.length;k++){
			if(uci.get('network', devs[k]['.name'], 'name') == devname){
				if(uci.get('network', devs[k]['.name'], 'ipv6') != value){
					uci.set('network', devs[k]['.name'], 'ipv6', value);
				}
			}
		}
	}else{
		// using ifname.
		if(uci.get('network', section_id, 'ipv6') != value){
			uci.set('network', section_id, 'ipv6', value);
		}
	}
};

return view.extend({
	render: function() {
		if (L.hasSystemFeature('ipv6')){
			var m, s, o;

			m = new form.Map('network', _("IPv6 Configuration"));
			m.chain('dhcp');

			s = m.section(form.NamedSection, 'rtkglobals');
			s.anonymous = true;

			o = s.option(form.ListValue, 'ipv6', _('IPv6'));
			o.value("0", _("Disable"));
			o.value("1", _("Enable"));
			o.default = "1";
			o.write = function(section_id, formvalue){
				// Step 1: disable all ipv6 interface
				var sections = uci.sections('network', 'device');
				var rtkWans = sections.filter(function(e) { return e.name.indexOf('veip0') != -1 });
				for (var j = 0; j < rtkWans.length; j++) {
					if(rtkWans[j]['rtk_ipmode'].indexOf('IPv6') != -1){
						var interfaces=L.toArray(rtkWans[j]['interface']);
						for(var i=0;i<interfaces.length; i++){
							if(interfaces[i] == '%s_v4'.format(rtkWans[j]['.name'])){
								// disable ipv6 on ipv4 interface
								disable_ipv6(interfaces[i], formvalue);
							}else if(interfaces[i] == '%s_v6'.format(rtkWans[j]['.name'])){
								// disable ipv6 interface
								if(formvalue == '0'){
									uci.set('network', interfaces[i], 'disabled', '1');
								}else{
									uci.set('network', interfaces[i], 'disabled', '0');
								}
							}
						}
					}
				}

				disable_ipv6('lan', formvalue);

				// Step 2: disable all ipv6 lan service
				if(formvalue == '0'){
					uci.set('dhcp', 'lan', 'ra', 'disabled');
					uci.set('dhcp', 'lan', 'dhcpv6', 'disabled');
				}else{
					uci.set('dhcp', 'lan', 'ra', 'server');
					uci.set('dhcp', 'lan', 'dhcpv6', 'server');
				}

				return this.super('write', arguments);
			};

			return m.render();
		}else{
			return null;
		}
	},

	// fs.write called in save phase.
	// if user resume changes, fs.write can not be resume.
	// so we hidden save button
	handleSave: null,
	handleSaveApply: function(ev, mode){
		var tasks = [];

		document.getElementById('maincontent')
			.querySelectorAll('.cbi-map').forEach(function(map) {
				tasks.push(DOM.callClassMethod(map, 'save'));
			});

		return Promise.all(tasks).then(function() {
			classes.ui.changes.apply(mode == '0');
		});
	}
});
