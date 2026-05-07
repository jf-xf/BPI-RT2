'use strict';
'require view';
'require uci';
'require form';

var f_voip_uci = 'rtkvoip';

/* return bit value */
function resolve_cfg_bit(sid, option, shift)
{
	return ((uci.get(f_voip_uci, sid, option) & (0x01<<shift))? 1:0);
}

function set_cfg_bit(sid, option, flag, shift)
{
	var targetValue;

	targetValue=uci.get(f_voip_uci, sid, option)&(~(0x01<<shift)) | ((flag&0x01)<<shift);
	uci.set(f_voip_uci, sid, option, targetValue);
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load(f_voip_uci)
		]);
	},

	render: function() {
		var m, s, o;

		m = new form.Map(f_voip_uci, [_('Others')]);
		s = m.section(form.NamedSection, 'VOIP', _('Other Settings'));

		s.tab('dial', _('Dial Option'));
		s.tab('alarm', _('Off-Hook Alarm'));
		s.tab('detect', _('FXS Pulse Dial Detection'));
		s.tab('sip', _('SIP Settings'));
		s.tab('sip_opt', _('SIP Options'));

		/* Dial Option */
		o = s.taboption('dial', form.Value, '_tmp1', _('Auto-Dial Time'), _('3 ~ 9 sec., 0 for disabled'));
		o.rmempty = false;
		o.datatype = 'or(0, range(3, 9))';
		o.cfgvalue = function(section_id) {
			return (uci.get(f_voip_uci, section_id, 'AUTO_DIAL') & 0x0f);
		};
		o.write = function(section_id, value) {
			var val;
			val = uci.get(f_voip_uci, section_id, 'AUTO_DIAL')&(~0x0f) | (value&0x0f)
			uci.set(f_voip_uci, section_id, 'AUTO_DIAL', val);
		};

		o = s.taboption('dial', form.Flag, '_tmp2', _('Dial-out by Hash Key'));
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'AUTO_DIAL', 4);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'AUTO_DIAL', value, 4);
		};

		/* Off-hook Alarm */
		o = s.taboption('alarm', form.Value, 'OFF_HOOK_ALARM', _('Off-Hook Alarm Time'), _('10 ~ 60 sec., 0 for disabled'));
		o.rmempty = false;
		o.datatype = 'or(0, range(10, 60))';

		/* FXS Pulse Dial Detection */
		o = s.taboption('detect', form.Flag, 'PULSE_DIAL_DETECT', _('Detection'));
		o.rmempty = false;
		o = s.taboption('detect', form.Value, 'PULSE_DET_PAUSE', _('Inter-digit Pause Duration'), _('msec'));
		o.depends('PULSE_DIAL_DETECT', '1');
		o.rmempty = false;
		o.retain = true;
		o.datatype = 'uinteger';

		/* SIP Settings */
		o = s.taboption('sip', form.Flag, 'tmp3', _('SIP Prack'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return !resolve_cfg_bit(section_id, 'RFC_FLAGS', 0);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value=='0'? '1':'0', 0);
		};

		o = s.taboption('sip', form.Flag, 'tmp4', _('SIP Server Rendundacy'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 5);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 5);
		};

		o = s.taboption('sip', form.Flag, 'tmp5', _('SIP CLIR anonymouse from header'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 4);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 4);
		};

		o = s.taboption('sip', form.Flag, 'tmp6', _('Non-SIP INBOX call'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 6);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 6);
		};

		o = s.taboption('sip', form.Flag, 'tmp7', _('Hook Flash Relay setting\(SIP INFO\)'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 1);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 1);
		};

		o = s.taboption('sip', form.Value, 'MINI_SESSION_EXPIRE', _('SIP Min-SE'), _('sec'));
		o.datatype = 'uinteger';
		o.rmempty = false;

		o = s.taboption('sip', form.Flag, 'tmp8', _('user=phone'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 20);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 20);
		};

		o = s.taboption('sip', form.Flag, 'tmp9', _('# to %23'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 18);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 18);
		};

		/* SIP Options */
		o = s.taboption('sip_opt', form.Flag, 'tmp10', _('SIP options'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'RFC_FLAGS', 7);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'RFC_FLAGS', value, 7);
		};

		o = s.taboption('sip_opt', form.Value, 'HEARTBEAT_CYCLE', _('Options interval time'), _('sec'));
		/* Setting option in VOIP_PORT0_PROXY0 section */
		o.ucisection = 'VOIP_PORT0_PROXY0';
		o.datatype = 'uinteger';
		o.rmempty = false;

		return m.render();
	}
});
