'use strict';
'require view';
'require dom';
'require ui';
'require rpc';
'require form';

var callGetGponSetting = rpc.declare({
	object: 'rtk-rpc',
	method: 'get_gpon_settings',
	expect: { '': {} }
});

var callSetGponSetting = rpc.declare({
	object: 'rtk-rpc',
	method: 'set_gpon_settings',
	params: [ 'name', 'value' ],
	expect: { '': {} }
});

var callApplyGponSetting = rpc.declare({
	object: 'rtk-rpc',
	method: 'apply_gpon_settings',
	expect: { '': {} }
});

var settings;
var changes_name = [];
var changes_value = [];

function valueChange(name, value) {
	if(settings['mib'][name] != value){
		//callSetGponSetting( name, value);
		settings['mib'][name]=value;
		changes_name.push(name);
		changes_value.push(value);
	}
}

function validateNullandLength(sid, s) {
	if (s == null || s == '')
		return _("This item should NOT be NULL");
	
	return true;
}
return view.extend({
	load: function() {
		return Promise.all([
					L.resolveDefault(callGetGponSetting())
                ]);
	},
	render: function(data) {
		var m, s, o;
		settings = data[0];

		m = new form.JSONMap(settings, _('OMCI Information'), _('This page is used to configure the parameters for your GPON network access.'));

		s = m.section(form.NamedSection, 'mib');

		o = s.option(form.Value, 'PON_VENDOR_ID', _('OMCI Vendor ID'));
		o.placeholder = _('Configure OMCI Vendor ID...');
		o.validate = validateNullandLength;
		o.write = function(section_id, formvalue){
			valueChange( 'PON_VENDOR_ID', formvalue);
		};
		
		o = s.option(form.DummyValue, 'OMCI_SW_VER1', _('OMCI Software Version 1'));
		o.modalonly = false;

		o = s.option(form.DummyValue, 'OMCI_SW_VER2', _('OMCI Software Version 2'));
		o.modalonly = false;

		o = s.option(form.DummyValue, 'OMCC_VER', _('OMCC Version'));
		o.modalonly = false;
		//o.cfgvalue = "0x"+form.DummyValue.toString(16);
		o.cfgvalue = function(section_id) {
			return "0x" + Number(settings['mib']['OMCC_VER']).toString(16);
		};

		o = s.option(form.DummyValue, 'OMCI_TM_OPT', _('Traffic Managament Option'));
		o.modalonly = false;

		o = s.option(form.DummyValue, 'HW_CWMP_PRODUCTCLASS', _('CWMP Product Class'));
		o.modalonly = false;

		o = s.option(form.DummyValue, 'HW_HWVER', _('HW Version'));
		o.modalonly = false;

		return m.render();
	},
	handleSave: null,
	handleSaveApply: function(ev, mode) {
		var map = document.querySelector('.cbi-map');

		return dom.callClassMethod(map, 'save').then(function() {
			var message = ui.showModal('', '');
			
			if(changes_name.length > 0){
				for (var i = 0; i < changes_name.length; i++) {
					//settings['mib'][changes_name[i]]=changes_value[i];
					callSetGponSetting( changes_name[i], changes_value[i]);
				}
				dom.content(message, E('p', _('Apply and keep settings')));
			}
			else{
				dom.content(message, E('p', _('There are no changes to apply')));
			}
		}).then(function() {

			//if config change, execute function to apply
			if(changes_name.length > 0){
				callApplyGponSetting();
			}

			window.setTimeout(function() {ui.hideModal();}, 1000);
			
			for (var i = 0; i < changes_name.length; i++) {
				changes_name.pop();
				changes_value.pop();
			}
		});
	},
	
	handleReset: null
});

