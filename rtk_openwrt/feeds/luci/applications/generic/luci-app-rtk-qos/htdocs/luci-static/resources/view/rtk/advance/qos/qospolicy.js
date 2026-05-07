'use strict';
'require dom';
'require view';
'require ui';
'require uci';
'require form';
'require rpc';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('luci'),
			uci.load('qos')
		]);
	},

	render: function(data) {
		var m, s, o;

		m = new form.Map('qos', _('IPQoS Configuration'),_(''));

		//IP QoS Configuration
		s = m.section(form.TypedSection, 'interface', _('IP QoS'), _('IP QoS'));
		s.anonymous = true;
		s.filter = function(section_id) {
			return (section_id== 'veip0');
		};

		o = s.option(form.Flag, 'enabled', _('Enable IP Qos'));
		o.modalonly = true;

		//IP QoS Configuration
		s = m.section(form.TypedSection, 'classgroup', _('QoS Queue Config'), _('QoS Queue Config'));
		s.anonymous = true;

		s.filter = function(section_id) {
			return ( section_id== 'Default');
		};

		o = s.option(form.ListValue, 'mode', _('Policy:'));
		o.rmempty = true;
		o.modalonly = true;
		o.widget = 'radio';
		o.value('prio', _('PRIO	'));
		o.value('wrr', _('WRR'));

		//
		s = m.section(form.GridSection, 'class', _('Class'));
		s.addremove = false;
		s.anonymous = true;
		//s.sortable  = true;
		s.sectiontitle = function(section_id) {
				return section_id;
		};
		
		o = s.option(form.DummyValue, 'policy', _('Policy'));
		o.modalonly = false;
		o.textvalue = function(section_id) {
			return uci.get('qos', 'Default', 'mode');
		};

		o = s.option(form.Value, 'order', _('Priority'));
		o.modalonly = false;
		o.editable = false;
		
		o = s.option(form.Value, 'weight', _('Weight'));
		//o.modalonly = false;
		o.editable = true;
		o.datatype = 'uinteger';
		o.placeholder = 0;

		o = s.option(form.Flag, 'enabled', _('Enable'));
		o.editable = true;

		//QoS Bandwidth Config
		s = m.section(form.TypedSection, 'interface', _('QoS Bandwidth Config'), _('QoS Bandwidth Config'));
		s.anonymous = true;

		s.filter = function(section_id) {
			return ( section_id== 'veip0');
		};

		o = s.option(form.Flag, 'bandwidth_enabled', _('User Defined Bandwidth'));
		o.modalonly = true;

		o = s.option(form.Value, 'upload', _('Total Bandwidth Limit:'));
		o.optional    = true;
		o.placeholder = 1000000;
		o.datatype    = 'uinteger';

		return m.render();
	}
});
