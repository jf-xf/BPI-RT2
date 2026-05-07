'use strict';
'require ui';
'require uci';
'require form';
'require network';
'require firewall';
'require fs';
'require rtk.network as rtkNetwork';

var CBINetworkSelect = form.ListValue.extend({
	__name__: 'CBI.NetworkSelect',

	/*
	 *	It support following option to filter out rtkWan entry
	 *		L3DevName
	 *		intfName
	 *		ipmodefilter
	 *		chmodefilter
	 *		ignoreBridge
	 *	It support following option to change UCI config value in dropdown item
	 *		valueType
	 *			if null, value is rtkwan section name
	 *			if 'IPv4', value is interface section name for IPv4 of rtkwan.
	 *			if 'IPv6', value is interface section name for IPv6 of rtkwan.
	 *			if 'netdev' value is net device name for this rtk section.
	 */

	load: function(section_id) {
		return rtkNetwork.getNetworkByFilter([], this.L3DevName, this.intfName, this.ipmodefilter, this.chmodefilter, this.ignoreBridge).then(L.bind(function(networks) {
			this.rtkWans = networks;
			return this.super('load', section_id);
		}, this));
	},

	renderWidget: function(section_id, option_index, cfgvalue) {
		var values = L.toArray((cfgvalue != null) ? cfgvalue : this.default),
		    choices = {},
		    checked = {};

		for (var i = 0; i < values.length; i++)
			checked[values[i]] = true;

		values = [];

		if (!this.multiple && (this.rmempty || this.optional))
			choices[''] = E('em', _('unspecified'));

		for (var idx=0; idx < this.rtkWans.length; idx++) {
			var rtkWan=this.rtkWans[idx];
			// For valueType
			//   if null, value is rtkwan section name
			//   if 'IPv4', value is interface section name for IPv4 of rtkwan.
			//   if 'IPv6', value is interface section name for IPv6 of rtkwan.
			//   if 'IPv4ORv6', value is interface section name for IPv4ORIPv6 of rtkwan.
			//   if 'netdev' value is net device name for this rtk section.
			switch(this.valueType){
			case 'IPv4':	// IPv4 section name of config interface
			case 'IPv6':	// IPv6 section name of config interface
			case 'IPv4ORv6':// For Layer 3 Device(IPv4 or IPv6)
				if(rtkWan['config']['rtk_chmode'] == 'Bridged')
					break;

				if(this.valueType != 'IPv4ORv6') {
					if(rtkWan['config']['rtk_ipmode'].indexOf(this.valueType) == -1)
						break;
				}

				var intfname, intfnamev4, intfnamev6;
				if(this.valueType == 'IPv4'){
					intfname='%s_v4'.format(rtkWan['config']['.name']);
				}else if(this.valueType == 'IPv6'){
					intfname='%s_v6'.format(rtkWan['config']['.name']);
				}else {
					intfnamev4='%s_v4'.format(rtkWan['config']['.name']);
					intfnamev6='%s_v6'.format(rtkWan['config']['.name']);
				}

				for (var i = 0; i < rtkWan.networks.length; i++) {
					var network = rtkWan.networks[i],
						name = network.getName();

					if(this.valueType != 'IPv4ORv6') {
						if(intfname != name)
							continue;
					}else {
						if((intfnamev4 != name) && (intfnamev6 != name))
							continue;
					}

					if (checked[name])
						values.push(name);

					choices[name] = rtkNetwork.renderIfaceBadge(rtkWan, network);
					break;
				}
				break;
			case 'netdev':	// for physical netdev
				choices[rtkWan['config']['name']] = rtkNetwork.renderIfaceBadge(rtkWan, null);
				if (checked[rtkWan['config']['name']])
					values.push(rtkWan['config']['name']);
				break;
			default:	// section name of config device
				choices[rtkWan['config']['.name']] = rtkNetwork.renderIfaceBadge(rtkWan, null);
				if (checked[rtkWan['config']['.name']])
					values.push(rtkWan['config']['.name']);
				break;
			}
		}

		var widget = new ui.Dropdown(this.multiple ? values : values[0], choices, {
			id: this.cbid(section_id),
			sort: true,
			multiple: this.multiple,
			optional: this.optional || this.rmempty,
			disabled: (this.readonly != null) ? this.readonly : this.map.readonly,
			select_placeholder: E('em', _('unspecified')),
			display_items: this.display_size || this.size || 3,
			dropdown_items: this.dropdown_size || this.size || 5,
			datatype: this.multiple ? 'list(string)' : 'string',
			validate: L.bind(this.validate, this, section_id),
			create: !this.nocreate,
			create_markup: '' +
				'<li data-value="{{value}}">' +
					'<span class="ifacebadge" style="background:repeating-linear-gradient(45deg,rgba(204,204,204,0.5),rgba(204,204,204,0.5) 5px,rgba(255,255,255,0.5) 5px,rgba(255,255,255,0.5) 10px)">' +
						'{{value}}: <em>('+_('create')+')</em>' +
					'</span>' +
				'</li>'
		});

		return widget.render();
	}
});

var CBIZoneSelect = form.ListValue.extend({
	__name__: 'CBI.ZoneSelect',

	load: function(section_id) {
		return Promise.all([ 
			firewall.getZones(), 
			network.getNetworks(),
			rtkNetwork.getNetworkByFilter([], this.L3DevName, this.intfName, this.ipmodefilter, this.chmodefilter, this.ignoreBridge)
		]).then(L.bind(function(zn) {
			this.zones = zn[0];
			this.networks = zn[1];
			this.rtkWans = zn[2];
			return this.super('load', section_id);
		}, this));
	},

	filter: function(section_id, value) {
		return true;
	},

	lookupZone: function(name) {
		return this.zones.filter(function(zone) { return zone.getName() == name })[0];
	},

	lookupNetwork: function(name) {
		return this.networks.filter(function(network) { return network.getName() == name })[0];
	},

	lookupRtkNetwork: function(name) {
		return this.rtkWans.filter(function(rtkwan) { 
			for(var i=0; i<rtkwan.networks.length; i++){
				if(rtkwan.networks[i].getName() == name)
					return true;
			}

			return false;
		})[0];
	},

	renderWidget: function(section_id, option_index, cfgvalue) {
		var values = L.toArray((cfgvalue != null) ? cfgvalue : this.default),
			isOutputOnly = false,
			choices = {};

		var displayZone = this.displayZone ? L.toArray(this.displayZone) : [];
		var hideZone = this.hideZone ? L.toArray(this.hideZone) : [];

		if (this.option == 'dest') {
			for (var i = 0; i < this.section.children.length; i++) {
				var opt = this.section.children[i];
				if (opt.option == 'src') {
					var val = opt.cfgvalue(section_id) || opt.default;
					isOutputOnly = (val == null || val == '');
					break;
				}
			}

			this.title = isOutputOnly ? _('Output zone') :  _('Destination zone');
		}

		if (this.allowlocal) {
			choices[''] = E('span', {
				'class': 'zonebadge',
				'style': firewall.getZoneColorStyle(null)
			}, [
				E('strong', _('Device')),
				(this.allowany || this.allowlocal)
					? E('span', ' (%s)'.format(this.option != 'dest' ? _('output') : _('input'))) : ''
			]);
		}
		else if (!this.multiple && (this.rmempty || this.optional)) {
			choices[''] = E('span', {
				'class': 'zonebadge',
				'style': firewall.getZoneColorStyle(null)
			}, E('em', _('unspecified')));
		}

		if (this.allowany) {
			choices['*'] = E('span', {
				'class': 'zonebadge',
				'style': firewall.getZoneColorStyle(null)
			}, [
				E('strong', _('Any zone')),
				(this.allowany && this.allowlocal && !isOutputOnly) ? E('span', ' (%s)'.format(_('forward'))) : ''
			]);
		}

		for (var i = 0; i < this.zones.length; i++) {
			var zone = this.zones[i],
				name = zone.getName(),
				networks = zone.getNetworks(),
				ifaces = [];

			if (!this.filter(section_id, name))
				continue;

			if (displayZone.length && !displayZone.find(function(zone){ return zone == name; }))
				continue;

			var filterArray = hideZone.filter(function(ignreZone){
				return ignreZone == name;
			});

			if (filterArray.length)
				continue;

			if(name == 'lan'){
				for (var j = 0; j < networks.length; j++) {
					var network = this.lookupNetwork(networks[j]);

					if (!network)
						continue;

					var span = E('span', {
						'class': 'ifacebadge' + (network.getName() == this.network ? ' ifacebadge-active' : '')
					}, network.getName() + ': ');

					var devices = network.isBridge() ? network.getDevices() : L.toArray(network.getDevice());

					for (var k = 0; k < devices.length; k++) {
						span.appendChild(E('img', {
							'title': devices[k].getI18n(),
							'src': L.resource('icons/%s%s.png'.format(devices[k].getType(), devices[k].isUp() ? '' : '_disabled'))
						}));
					}

					if (!devices.length)
						span.appendChild(E('em', _('(empty)')));

					ifaces.push(span);
				}
			}else{
				for (var k = 0; k < this.rtkWans.length; k++) {
					var rtkwan = this.rtkWans[k];
					var found = 0;
					for (var j = 0; j < networks.length; j++) {
						var network = rtkwan.networks.filter(function(network) { return network.getName() == networks[j] })[0];
						if (!network)
							continue;

						found = 1;
						break;
					}

					if(found){
						var span = rtkNetwork.renderIfaceBadge(rtkwan, null);
						ifaces.push(span);
					}
				}
			}

			if (!ifaces.length)
				ifaces.push(E('em', _('(empty)')));

			choices[name] = E('span', {
				'class': 'zonebadge',
				'style': firewall.getZoneColorStyle(zone)
			}, [ E('strong', name) ].concat(ifaces));
		}

		var widget = new ui.Dropdown(values, choices, {
			id: this.cbid(section_id),
			sort: true,
			multiple: this.multiple,
			optional: this.optional || this.rmempty,
			disabled: (this.readonly != null) ? this.readonly : this.map.readonly,
			select_placeholder: E('em', _('unspecified')),
			display_items: this.display_size || this.size || 3,
			dropdown_items: this.dropdown_size || this.size || 5,
			validate: L.bind(this.validate, this, section_id),
			create: !this.nocreate,
			create_markup: '' +
				'<li data-value="{{value}}">' +
					'<span class="zonebadge" style="background:repeating-linear-gradient(45deg,rgba(204,204,204,0.5),rgba(204,204,204,0.5) 5px,rgba(255,255,255,0.5) 5px,rgba(255,255,255,0.5) 10px)">' +
						'<strong>{{value}}:</strong> <em>('+_('create')+')</em>' +
					'</span>' +
				'</li>'
		});

		var elem = widget.render();

		if (this.option == 'src') {
			elem.addEventListener('cbi-dropdown-change', L.bind(function(ev) {
				var opt = this.map.lookupOption('dest', section_id),
					val = ev.detail.instance.getValue();

				if (opt == null)
					return;

				var cbid = opt[0].cbid(section_id),
					label = document.querySelector('label[for="widget.%s"]'.format(cbid)),
					node = document.getElementById(cbid);

				L.dom.content(label, val == '' ? _('Output zone') : _('Destination zone'));

				if (val == '') {
					if (L.dom.callClassMethod(node, 'getValue') == '')
						L.dom.callClassMethod(node, 'setValue', '*');

					var emptyval = node.querySelector('[data-value=""]'),
						anyval = node.querySelector('[data-value="*"]');

					L.dom.content(anyval.querySelector('span'), E('strong', _('Any zone')));

					if (emptyval != null)
						emptyval.parentNode.removeChild(emptyval);
				}
				else {
					var anyval = node.querySelector('[data-value="*"]'),
						emptyval = node.querySelector('[data-value=""]');

					if (emptyval == null) {
						emptyval = anyval.cloneNode(true);
						emptyval.removeAttribute('display');
						emptyval.removeAttribute('selected');
						emptyval.setAttribute('data-value', '');
					}

					if (opt[0].allowlocal)
						L.dom.content(emptyval.querySelector('span'), [
							E('strong', _('Device')), E('span', ' (%s)'.format(_('input')))
						]);

					L.dom.content(anyval.querySelector('span'), [
						E('strong', _('Any zone')), E('span', ' (%s)'.format(_('forward')))
					]);

					anyval.parentNode.insertBefore(emptyval, anyval);
				}
			}, this));
		}
		else if (this.option == 'rtk_conntype') {
			elem.addEventListener('cbi-dropdown-change', L.bind(function(ev) {
				var val = ev.detail.instance.getValue();
				var cbid = this.cbid(section_id),
					label = document.querySelector('label[for="widget.%s"]'.format(cbid)),
					node = document.getElementById(cbid);

				if(val.toString().indexOf('Other') != -1){
					for(var i=0;i<val.length;i++){
						if(val[i] == 'Other')
							continue;

						var emptyval = node.querySelector('[data-value="%s"]'.format(val[i]));
						if (emptyval != null){
							widget.toggleItem(node, emptyval, false);
						}
					}

					var lis = node.querySelectorAll('li[data-value]');
					for (var i = 0; i < lis.length; i++) {
						var value = lis[i].getAttribute('data-value');
						if (value !== 'Other')
							lis[i].setAttribute('unselectable', '');
					}
				}else{
					var lis = node.querySelectorAll('li[data-value]');
					for (var i = 0; i < lis.length; i++) {
						lis[i].removeAttribute('unselectable', '');
					}
				}
			}, this));
		}
		else if (isOutputOnly) {
			var emptyval = elem.querySelector('[data-value=""]');
			emptyval.parentNode.removeChild(emptyval);
		}

		return elem;
	}
});

var CBIMultiValue = form.ListValue.extend(/** @lends LuCI.form.MultiValue.prototype */ {
        __name__: 'CBI.MultiValue',

        __init__: function() {
                this.super('__init__', arguments);
                this.placeholder = _('-- Please choose --');
        },

        /**
         * Allows to specify the [display_items]{@link LuCI.ui.Dropdown.InitOptions}
         * property of the underlying dropdown widget. If omitted, the value of
         * the `size` property is used or `3` when `size` is unspecified as well.
         *
         * @name LuCI.form.MultiValue.prototype#display_size
         * @type number
         * @default null
         */

        /**
         * Allows to specify the [dropdown_items]{@link LuCI.ui.Dropdown.InitOptions}
         * property of the underlying dropdown widget. If omitted, the value of
         * the `size` property is used or `-1` when `size` is unspecified as well.
         *
         * @name LuCI.form.MultiValue.prototype#dropdown_size
         * @type number
         * @default null
         */

        /** @private */
        renderWidget: function(section_id, option_index, cfgvalue) {
				var loadWIRELESS = L.resolveDefault(uci.load('rtkmap'));
				return Promise.all([ loadWIRELESS ]).then(L.bind(function(res) {
					var value = (cfgvalue != null) ? cfgvalue : this.default,
						choices = this.transformChoices();

					var widget = new ui.Dropdown(L.toArray(value), choices, {
							id: this.cbid(section_id),
							sort: this.keylist,
							multiple: true,
							optional: this.optional || this.rmempty,
							select_placeholder: this.placeholder,
							display_items: this.display_size || this.size || 3,
							dropdown_items: this.dropdown_size || this.size || -1,
							validate: L.bind(this.validate, this, section_id),
							disabled: (this.readonly != null) ? this.readonly : this.map.readonly
					});

					var elem = widget.render();
					var uciGuest;
					uciGuest = uci.sections('rtkmap', 'guest');
					//console.log(uciGuest.length);
					//console.log(uciGuest[0].guestNetwork.length);
					//console.log(uciGuest[0].guestNetwork[0]);

					elem.addEventListener('mousedown', L.bind(function(ev) {
						var cbid = this.cbid(section_id);
						console.log('mousedown');
						if (this.option == 'primary') {
							var node = document.getElementById(cbid);
							var lis = node.querySelectorAll('li[data-value]');
							var nodeDefault = document.getElementById('cbid.rtkmap.map.default_guest');
							var lisDefault = nodeDefault.querySelectorAll('li[data-value]');

							for (var i = 0; i < lis.length; i++) {
								var value = lis[i].getAttribute('data-value');
								var found = false;

								for(var j = 0; j < lisDefault.length; j++) {
									var value2 = lisDefault[j].getAttribute('data-value');
									if (lisDefault[j].hasAttribute('selected') && value == value2)
										found = true;
								}

								for(var k = 0; k < uciGuest.length; k++) {
									if (uciGuest[k].guestNetwork != null) {
										for(var l = 0; l < uciGuest[k].guestNetwork.length; l++) {
											if (value == uciGuest[k].guestNetwork[l])
												found = true;
										}
									}
								}

								if (found == true) {
									lis[i].setAttribute('unselectable', '');
									//console.log('find ' + value);
								}
								else {
									lis[i].removeAttribute('unselectable', '');
									//console.log('primary notfind ' + value);
								}
							}
						}else if (this.option == 'default_guest') {
							var node = document.getElementById(cbid);
							var lis = node.querySelectorAll('li[data-value]');
							var nodePrimary = document.getElementById('cbid.rtkmap.map.primary');
							var lisPrimary = nodePrimary.querySelectorAll('li[data-value]');

							for (var i = 0; i < lis.length; i++) {
								var value = lis[i].getAttribute('data-value');
								var found = false;

								for(var j = 0; j < lisPrimary.length; j++) {
									var value2 = lisPrimary[j].getAttribute('data-value');
									if (lisPrimary[j].hasAttribute('selected') && value == value2)
										found = true;
								}

								for(var k = 0; k < uciGuest.length; k++) {
									if (uciGuest[k].guestNetwork != null) {
										for(var l = 0; l < uciGuest[k].guestNetwork.length; l++) {
											if (value == uciGuest[k].guestNetwork[l])
												found = true;
										}
									}
								}

								if (found == true) {
									lis[i].setAttribute('unselectable', '');
									//console.log('find ' + value);
								}
								else {
									lis[i].removeAttribute('unselectable', '');
									//console.log('default_guest notfind ' + value);
								}
							}
						}else if (this.option == 'guestNetwork') {
							//console.log(section_id);
							//console.log(cbid);
							var node = document.getElementById(cbid);
							var lis = node.querySelectorAll('li[data-value]');
							var nodePrimary = document.getElementById('cbid.rtkmap.map.primary');
							var lisPrimary = nodePrimary.querySelectorAll('li[data-value]');
							var nodeDefault = document.getElementById('cbid.rtkmap.map.default_guest');
							var lisDefault = nodeDefault.querySelectorAll('li[data-value]');

							for (var i = 0; i < lis.length; i++) {
								var value = lis[i].getAttribute('data-value');
								var found = false;

								for(var j = 0; j < lisPrimary.length; j++) {
									var value2 = lisPrimary[j].getAttribute('data-value');
									if (lisPrimary[j].hasAttribute('selected') && value == value2)
										found = true;
								}

								for(var k = 0; k < lisDefault.length; k++) {
									var value2 = lisDefault[k].getAttribute('data-value');
									if (lisDefault[k].hasAttribute('selected') && value == value2)
										found = true;
								}

								for(var k = 0; k < uciGuest.length; k++) {
									//console.log(uciGuest[k]['.name'])
									if (uciGuest[k].guestNetwork != null) {
										if (uciGuest[k]['.name'] != section_id) {
											for(var l = 0; l < uciGuest[k].guestNetwork.length; l++) {
												if (value == uciGuest[k].guestNetwork[l])
													found = true;
											}
										}
									}
								}

								if (found == true) {
									lis[i].setAttribute('unselectable', '');
									//console.log('guest find ' + value);
								}
								else {
									lis[i].removeAttribute('unselectable', '');
									//console.log('quest notfind ' + value);
								}
							}
						}

					}, this));

					return elem;
				}, this));
        },

});

return L.Class.extend({
	NetworkSelect: CBINetworkSelect,
	ZoneSelect: CBIZoneSelect,
	EasyMeshSelect: CBIMultiValue
});
