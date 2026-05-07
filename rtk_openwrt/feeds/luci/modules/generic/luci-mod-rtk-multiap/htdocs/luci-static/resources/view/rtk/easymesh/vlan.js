'use strict';
'require view';
'require uci';
'require rpc';
'require form';
'require ui';
'require tools.rtk.widgets as rtkWidget';

var nullEmpty = 0;
var primaryEmpty = 1;
var defaultEmpty = 2;
var bothEmpty = 3;

var groupEmpty = 0;
var primaryValueArray = [];
var defaultValueArray = [];

function getAvailSSID() {
	var totalnum = 0;
	var wifi = uci.sections('wireless', 'wifi-iface');
	var sband = uci.get('rtkmap', 'map', 'configured_band');
	var filter_backhaul = 'default_radio01';

	if (sband == '5g')
		filter_backhaul = 'default_radio01';
	else
		filter_backhaul = 'default_radio11';

	for (var i = 0; i < wifi.length; i++) {
		// Filter Backhaul
		if (wifi[i].disabled != '1' && wifi[i]['.name'] != filter_backhaul)
			totalnum++;
	}

	return totalnum;
}

function getSetedSSIDNum(primaryArray, defaultArray) {
	var totalnum = 0;
	var quest = uci.sections('rtkmap', 'guest');

	totalnum = primaryArray.length;
	totalnum += defaultArray.length

	for (var k = 0; k < quest.length; k++) {
		var guestArray = uci.get('rtkmap', quest[k]['.name'], 'guestNetwork');

		if (guestArray) {
			totalnum += guestArray.length;
		//console.log(quest[k]['.name']+' num is '+guestArray.length);
		}
	}

	//console.log('Seted SSID Total num is '+totalnum);
	return totalnum;
}

function getGuestSetedSSIDNum() {
	var totalnum = 0;
	var quest = uci.sections('rtkmap', 'guest');

	for (var k = 0; k < quest.length; k++) {
		var guestArray = uci.get('rtkmap', quest[k]['.name'], 'guestNetwork');

		if (guestArray) {
			totalnum += guestArray.length;
		//console.log(quest[k]['.name']+' num is '+guestArray.length);
		}
	}

	//console.log('Seted SSID Total num is '+totalnum);
	return totalnum;
}

function listAvailSSID(o) {
	var wifi = uci.sections('wireless', 'wifi-iface');
	var sband = uci.get('rtkmap', 'map', 'configured_band');
	var filter_backhaul = 'default_radio01';

	if (sband == '5g')
		filter_backhaul = 'default_radio01';
	else
		filter_backhaul = 'default_radio11';

	for (var i = 0; i < wifi.length; i++) {
		// Filter Backhaul
		if (wifi[i].disabled != '1' && wifi[i]['.name'] != filter_backhaul)
			o.value(wifi[i].ssid, '%s'.format(wifi[i].ssid));
	}

	return;
}

function addSSID_when_empty(type, typeValueArray) {
	var found;
	var valueArray = [];
	var count = 0;
	var wifi = uci.sections('wireless', 'wifi-iface');
	var quest = uci.sections('rtkmap', 'guest');
	var sband = uci.get('rtkmap', 'map', 'configured_band');
	var filter_backhaul = 'default_radio01';

	if (sband == '5g')
		filter_backhaul = 'default_radio01';
	else
		filter_backhaul = 'default_radio11';

	for (var i = 0; i < wifi.length; i++) {
		if (wifi[i].disabled != '1' && wifi[i]['.name'] != filter_backhaul) {
			found = 0;
			for (var j = 0; j < typeValueArray.length; j++) {
				if (typeValueArray[j] == wifi[i].ssid) {
					//if (type == 'primary')
					//	console.log('empty: Found  '+typeValueArray[j]+' in default');
					//else
					//	console.log('empty: Found  '+typeValueArray[j]+' in primary');
					found = 1;
					break;
				}
			}

			if (!found && quest.length > 0 ) {
				for (var k = 0; k < quest.length; k++) {
					var guestArray = uci.get('rtkmap', quest[k]['.name'], 'guestNetwork');

					if (guestArray) {
						for (var l = 0; l < guestArray.length; l++) {
							if (guestArray[l] == wifi[i].ssid) {
								//console.log('empty: Found  '+guestArray[l]+' in '+quest[k]['.name']);
								found = 1;
								break;
							}
						}
					}
				}
			}

			if (!found) {
				//console.log('empty: Include '+wifi[i].ssid+' to '+type);
				valueArray[count] = wifi[i].ssid;
				count++;
			}
		}
	}

	if (type == 'primary') {
		if (count == 0) {
			// Primary Network can't be left empty.
			ui.showModal(_(''), [
				E('p', _('Primary Network can not be left empty! Please check again!')),
				E('div', { 'class': 'right' }, [

					E('button', {
						'class': 'btn cbi-button-action important',
						'click': ui.createHandlerFn(this, function(ev) {
							return ui.hideModal();
						})
					}, [ _('DISMISS') ])
				])
			]);
		}
		else
			uci.set('rtkmap', 'map', 'primary',  valueArray);
	}
	else if (type == 'default_guest') {
		// Default Network can be left empty when all SSIDs be added into Primary Network/Guest Networks.
		if (count != 0) {
			//console.log('addSSID_when_empty: set to default. valueArray is '+valueArray.toString());
			uci.set('rtkmap', 'map', 'default_guest',  valueArray);
		}
	}
	return;
}

function addSSID_into_default(primaryArray, defaultArray) {
	var found;
	var valueArray = defaultArray;
	var count = defaultArray.length;
	var wifi = uci.sections('wireless', 'wifi-iface');
	var quest = uci.sections('rtkmap', 'guest');
	var sband = uci.get('rtkmap', 'map', 'configured_band');
	var filter_backhaul = 'default_radio01';

	if (sband == '5g')
		filter_backhaul = 'default_radio01';
	else
		filter_backhaul = 'default_radio11';

	for (var i = 0; i < wifi.length; i++) {
		if (wifi[i].disabled != '1' && wifi[i]['.name'] != filter_backhaul) {
			found = 0;
			if (primaryArray) {
				for (var j = 0; j < primaryArray.length; j++) {
					if (primaryArray[j] == wifi[i].ssid) {
						//console.log('default: Found  '+primaryArray[j]+' in primary');
						found = 1;
						break;
					}
				}
			}

			if (!found && defaultArray) {
				for (var jj = 0; jj < defaultArray.length; jj++) {
					if (defaultArray[jj] == wifi[i].ssid) {
						//console.log('default: Found  '+defaultArray[jj]+' in default');
						found = 1;
						break;
					}
				}
			}

			if (!found && quest.length > 0 ) {
				for (var k = 0; k < quest.length; k++) {
					var guestArray = uci.get('rtkmap', quest[k]['.name'], 'guestNetwork');

					if (guestArray) {
						for (var l = 0; l < guestArray.length; l++) {
							if (guestArray[l] == wifi[i].ssid) {
								//console.log('default: Found  '+guestArray[l]+' in '+quest[k]['.name']);
								found = 1;
								break;
							}
						}
					}
				}
			}

			if (!found) {
				//console.log('default: Include '+wifi[i].ssid+' to default');
				valueArray[count] = wifi[i].ssid;
				count++;
			}
		}
	}

	if (valueArray.length != 0) {
		//console.log('addSSID_into_default: valueArray is '+valueArray.toString());
		uci.set('rtkmap', 'map', 'default_guest',  valueArray);
	}
	else {
		//console.log('addSSID_into_default: delete default.');
		uci.unset('rtkmap', 'map', 'default_guest');
	}
	return;
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('wireless'),
			uci.load('rtkmap')
		]);
	},

	render: function() {
		var m, s, ss, so, o, wifi_ifaces;
		var map_role = uci.get('rtkmap', 'map', 'map_role');

		wifi_ifaces = uci.sections('wireless', 'wifi-iface');

		m = new form.Map('rtkmap', _('EasyMesh Vlan'),
			_('This page configurse the vlan setting of EasyMesh network.'));

		s = m.section(form.TypedSection, 'rtk_map');
		s.anonymous = true;

		o = s.option(form.Flag, 'enable_vlan', _('Enable Guest Network'));
		o.rmempty = false;
		o.default = o.disabled;
		o.readonly = (map_role == 1)? false:true;

		o = s.option(rtkWidget.EasyMeshSelect, 'primary', _('Primary Network SSID'));
		o.depends('enable_vlan', '1');
		o.readonly = (map_role == 1)? false:true;
		listAvailSSID(o);
		o.validate = function(section_id, value) {
			var max = getAvailSSID();

			groupEmpty = nullEmpty;
			if (value.length == 0) {
				groupEmpty |= primaryEmpty;
				primaryValueArray = [];
				return true;
			}

			primaryValueArray = value.split(" ");
			if (primaryValueArray.length >= max)
				return _('SSIDs are all added into Primary Network! Please check again!');
			uci.set('rtkmap', 'map', 'primary',  primaryValueArray);
			return true;
		};
		o.remove = function(section_id){
			// If the Primary Network is empty, set ALL SSIDs which is not on Default Network and Guest Neteorks.
			// If no SSID can be added into Primary Network, Display a error message "Primary Network can not be left empty! Please check again!".
			addSSID_when_empty('primary', defaultValueArray);
		};
		o.write = function(section_id, value) {
			this.super('write', arguments);
			addSSID_into_default(primaryValueArray, defaultValueArray);
		};

		o = s.option(rtkWidget.EasyMeshSelect, 'default_guest', _('Default Guest Network SSID'));
		o.depends('enable_vlan', '1');
		o.readonly = (map_role == 1)? false:true;
		listAvailSSID(o);
		o.validate = function(section_id, value) {
			var max = getAvailSSID();
			var setGuestTotalNum = getGuestSetedSSIDNum();

			if (value.length == 0) {
				groupEmpty |= defaultEmpty;
				defaultValueArray = [];
				if (groupEmpty == bothEmpty)  // The two gropus of Primary and Default Network can't be empty.
					return _('Primary and Default Guest Network are not allowed to be left empty!');
			} else {
				defaultValueArray = value.split(" ");

				if (groupEmpty == primaryEmpty && defaultValueArray.length+setGuestTotalNum >= max)
					return _('Primary Network can not be left empty. Please check again!');

				if (defaultValueArray.length >= max)
					return _('SSIDs are all added into Default Guest Network! Please check again!');
				uci.set('rtkmap', 'map', 'default_guest',  defaultValueArray);
			}
			return true;
		};
		o.remove = function(section_id){
			// If the Default Guest Network is empty, set ALL SSIDs which is not on Primary Network and Guest Neteorks.
			// If no SSID can be added into Default Network, Let default Network is empty.
			addSSID_when_empty('default_guest', primaryValueArray);
		};
		o.write = function(section_id, value) {
			this.super('write', arguments);
			if (groupEmpty != primaryEmpty) {
				addSSID_into_default(primaryValueArray, defaultValueArray);
			}
		};

		o = s.option(form.SectionValue, '__guest__', form.GridSection, 'guest', _('Add More Guest Network'));
		o.depends('enable_vlan', '1');
		ss = o.subsection;
		ss.addremove = (map_role == 1)? true:false;
		ss.anonymous = true;
		ss.sortable  = true;

		ss.handleAdd = function(ev) {
			var name, quest_section;
			var uciguest = uci.sections('rtkmap', 'guest');

			name = 'guest%d'.format(uciguest.length);

			var config_name = this.uciconfig || this.map.config,
				section_id = this.map.data.add(config_name, this.sectiontype, name),
				mapNode = this.getPreviousModalMap(),
				prevMap = mapNode ? dom.findClassInstance(mapNode) : this.map;

			prevMap.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		ss.renderRowActions = function(section_id) {
			var btns;
			btns = [
				E('button', {
					'class': 'cbi-button cbi-button-action important',
					'title': _('Edit this network'),
					'click': ui.createHandlerFn(this, 'renderMoreOptionsModal', section_id)
				}, _('Edit')),
				E('button', {
					'class': 'cbi-button cbi-button-negative remove',
					'title': _('Delete this network'),
					'click': ui.createHandlerFn(this, 'handleRemove', section_id)
				}, _('DELETE'))
			];

			return E('td', { 'class': 'td middle cbi-section-actions' }, E('div', btns));
		};
		ss.handleRemove = function(section_id, ev) {
			if (groupEmpty == bothEmpty)  // The two gropus of Primary and Default Network can't be empty both.
				return;

			form.TypedSection.prototype.handleRemove.apply(this, [section_id, ev]);
			if (groupEmpty != primaryEmpty)
				addSSID_into_default(primaryValueArray, defaultValueArray);
			return;
		};

		so = ss.option(rtkWidget.EasyMeshSelect, 'guestNetwork', _('Guest Network SSID'));
		listAvailSSID(so);
		so.validate = function(section_id, value) {
			var max = getAvailSSID();
			var setTotalNum = getSetedSSIDNum(primaryValueArray, defaultValueArray);
			var valueArray = [];

			if (groupEmpty == bothEmpty)  // The two gropus of Primary and Default Network can't be empty.
				return _('Primary and Default Guest Network are not allowed to be left empty!');

			if (value.length !=0)
				valueArray = value.split(" ");

			if (groupEmpty == primaryEmpty && valueArray.length+setTotalNum >= max)
				return _('Primary Network can not be left empty. Please check again!');

			return true;
		};
		so.remove = function(section_id) {
			this.super('remove', arguments);
			addSSID_into_default(primaryValueArray, defaultValueArray);
			uci.remove('rtkmap', section_id);
		};
		so.write = function(section_id, value) {
			this.super('write', arguments);
			uci.set('rtkmap', 'map', 'enable_vlan',  '1');
			if (groupEmpty != primaryEmpty) {
				addSSID_into_default(primaryValueArray, defaultValueArray);
			}
		};

		return m.render();
	}
});
