'use strict';
'require view';
'require ui';
'require uci';
'require form';
'require tools.widgets as widgets';
'require rtk.network as rtkNetwork';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('qos')
		]);
	},

	render: function(data) {
		var m, s, o;

		m = new form.Map('qos', _('IP QoS Traffic Shaping'), _('IP QoS Traffic Shaping'));

		s = m.section(form.GridSection, 'ratelimit', _('Traffic Shaping'), );
		s.addremove = true;
		s.anonymous = true;
		s.addbtntitle = _('Add');

		o = s.option(form.Value, 'comment', _('Name'));
		o.datatype = 'maxlength(16)';

		o = s.option(widgets.NetworkSelect, 'wan_interface', _('WAN:'));
		o.modalonly = true;
		o.optional = true;
		o.rmempty = true;
		o.nocreate = true;
		o.exclude  = 'lan';

		o = s.option(form.ListValue, 'ip_version', _('IP Version:'));
		o.default = '0';
		o.value('0', _('Please select IP version'));
		o.value('1', _('IPv4'));
		o.value('2', _('IPv6'));
		
		o = s.option(form.ListValue, 'direction', _('Direction:'));
		o.default = '0';
		o.value('0', _('Upstream'));
		o.value('1', _('Downstream'));
		
		o = s.option(form.ListValue, 'proto', _('Protocol:'));
		o.default = 'none';
		o.value('none', _('NONE'));
		o.value('tcp', _('TCP'));
		o.value('udp', _('UDP'));
		o.value('icmp', _('ICMP'));
		
		o = s.option(form.Value, 'srchost', _('Source IP:'));
		o.datatype = 'or(ipaddr,"ignore")';

		o = s.option(form.Value, 'dsthost', _('Destination IP:'));
		o.datatype = 'or(ipaddr,"ignore")';

		o = s.option(form.Value, 'srcports', _('Source Port:'));
		o.datatype = 'portrange';

		o = s.option(form.Value, 'dstports', _('Destination  Port:'));
		o.datatype = 'portrange';

		o = s.option(form.Value, 'rate_limit', _('Rate Limit:'));
		o.datatype    = 'uinteger';

		return m.render();
	}
});
