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

function validateNullandLength(sid, s){
	//var pon_speed =mib_get('PON_SPEED');
	var pon_speed = settings['mib']['PON_SPEED'];
	var length;
		
	if (s == null || s == '')
		return _("This item should NOT be NULL");

	if (pon_speed == '0')
		length = 10;
	else
		length = 36;

	if(s.length > length)
		return _("This item length is too long!");
		
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
		
		m = new form.JSONMap(settings, _('GPON Settings'), _('This page is used to configure the parameters for your GPON network access.'));

		s = m.section(form.NamedSection, 'mib');

		o = s.option(form.Value, 'LOID', _('LOID'));
		o.placeholder = _('Configure GPON LOID...');
		o.maxlength = 31;
		o.write = function(section_id, formvalue){
			valueChange( 'LOID', formvalue);
		};
		
		o = s.option(form.Value, 'LOID_PASSWD', _('LOID Password'));
		o.placeholder = _('Configure GPON LOID Password...');
		o.maxlength = 36;
		o.write = function(section_id, formvalue){
			valueChange( 'LOID_PASSWD', formvalue);
		};
		
		o = s.option(form.Value, 'GPON_PLOAM_PASSWD', _('PLOAM Password'));
		o.placeholder = _('Configure GPON PLOAM password...');
		o.validate = validateNullandLength;
		o.maxlength = 36;
		o.write = function(section_id, formvalue){
			valueChange( 'GPON_PLOAM_PASSWD', formvalue);
		};

		o = s.option(form.DummyValue, 'GPON_SN', _('Serial Number'));
		o.modalonly = false;

		o = s.option(form.Value, 'OMCI_OLT_MODE', _('OMCI OLT Mode'));
		o.placeholder = _('Configure OMCI OLT Mode...');
		o.datatype = 'integer';
		o.value(0, _("Default Mode"));
		o.value(1, _("Huawei OLT Mode"));
		o.value(2, _("ZTE OLT Mode"));
		o.value(3, _("Customized Mode"));
		o.default = 0;
		o.write = function(section_id, formvalue){
			switch(formvalue){
			case '0':
				valueChange('OMCC_VER', '128'); //0x80
				valueChange('OMCI_TM_OPT', '2');
				break;
			case '1':
				valueChange('OMCC_VER', '134'); //0x86
				valueChange('OMCI_TM_OPT', '2');
				break;
			case '2':
				valueChange('OMCC_VER', '160'); //0xA0
				valueChange('OMCI_TM_OPT', '2');
				break;
			case '3':
				valueChange('OMCC_VER', '128'); //0x80
				valueChange('OMCI_TM_OPT', '2');
				break;
			}
			valueChange( 'OMCI_OLT_MODE', formvalue);
		};

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
