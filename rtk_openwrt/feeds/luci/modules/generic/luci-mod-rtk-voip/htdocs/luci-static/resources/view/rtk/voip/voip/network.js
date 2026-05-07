'use strict';
'require view';
'require uci';
'require form';

var f_voip_uci = 'rtkvoip';

return view.extend({
	render: function() {
		var m, s, o;

		m = new form.Map(f_voip_uci, [_('Network')]);
		s = m.section(form.TypedSection, 'voipCfgParam', _('DSCP Flag'));
		s.anonymous = true;
		o = s.option(form.Value, 'SIP_DSCP', _('SIP DSCP'), _('0 ~ 63'));
		o.datatype = 'uinteger';
		o.rmempty = false;

		o = s.option(form.Value, 'RTP_DSCP', _('RTP DSCP'), _('0 ~ 63'));
		o.datatype = 'uinteger';
		o.rmempty = false;

		return m.render();
	}
});
