'use strict';
'require view';
'require form';

var f_voip_uci = 'rtkvoip';

return view.extend({
	render: function() {
		var m, s, o;

		m = new form.Map(f_voip_uci, [_('Tone')]);
		s = m.section(form.TypedSection, 'voipCfgParam', _('Select Country'));
		s.anonymous = true;
		o = s.option(form.ListValue, 'TONE_OF_COUNTRY', _('Country'));
		o.value(0, _('USA'));
		o.value(1, _('UK'));
		o.value(2, _('AUSTRALIA'));
		o.value(3, _('HONG KONG'));
		o.value(4, _('JAPAN'));
		o.value(5, _('SWEDEN'));
		o.value(6, _('GERMANY'));
		o.value(7, _('FRANCE'));
		o.value(8, _('TAIWAN'));
		o.value(9, _('BELGUIM'));
		o.value(10, _('FINLAND'));
		o.value(11, _('ITALY'));
		o.value(12, _('CHINA'));
		o.value(13, _('RUSSIAN'));
		o.value(14, _('SPAIN'));
		o.value(15, _('INDIA'));
		o.value(16, _('GUATEMALA'));
		o.value(17, _('CUSTOMER'));
		o.value(18, _('BRAZIL'));
		o.value(19, _('NEW_ZEALAND'));

		return m.render();
	}
});
