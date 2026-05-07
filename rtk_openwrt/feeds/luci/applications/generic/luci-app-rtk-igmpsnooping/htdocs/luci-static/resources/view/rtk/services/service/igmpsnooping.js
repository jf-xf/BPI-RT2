'use strict';
'require view';
'require form';
'require uci';
'require fs';
'require dom';

return view.extend({
	render: function () {
		var m, s, o;

		m = new form.Map('igmpsnooping', _('igmpsnooping'), _('Embedded IGMP/MLD snooping switch'));

		s = m.section(form.TypedSection, 'snooping');
		s.anonymous = true;

		o = s.option(form.Flag, 'igmpsnoopingenabled', _('IGMP Enable'));
		o.modalonly = false;
		o.editable = true;
		o.rmempty=false;
		o.write = function(section_id, value) {
			var config_name = this.uciconfig || this.map.config;
			uci.set(config_name, section_id, 'igmpsnoopingenabled', value);
		};

		o = s.option(form.Flag, 'mldsnoopingenabled', _('MLD Enable'));
		o.modalonly = false;
		o.editable = true;
		o.rmempty=false;
		o.write = function(section_id, value) {
			var config_name = this.uciconfig || this.map.config;
			uci.set(config_name, section_id, 'mldsnoopingenabled', value);
		};

		return m.render();
	},
});