'use strict';
'require view';
'require ui';
'require uci';
'require form';
'require tools.rtk.firewall as fwtool';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('firewall')
		]);
	},

	render: function(data) {
		if (fwtool.checkLegacySNAT())
			return fwtool.renderMigration();
		else
			return this.renderForwards(data);
	},

	renderForwards: function(data) {
		var hosts = data[0],
		    m, s, o;

		m = new form.Map('firewall', _('DMZ Configuration'),
			_('A Demilitarized Zone is used to provide Internet services without sacrificing unauthorized access to its local private network. Typically, the DMZ host contains devices accessible to Internet traffic, such as Web (HTTP) servers, FTP servers, SMTP (e-mail) servers and DNS servers.'));

		s = m.section(form.TypedSection, 'redirect', _('Rules'));
		s.anonymous = true;

		s.filter = function(section_id) {
			return (uci.get('firewall', section_id, 'target') != 'SNAT' &&
			uci.get('firewall', section_id, 'name') == 'DMZ');
		};

		o = s.option(form.Flag, 'enabled', _('DMZ Host'));
		o.default = o.enabled;
		o.editable = true;

		o = s.option(form.Value, 'dest_ip', _('DMZ Host IP Address'), _(''), 'ipv4', hosts);
		o.datatype = 'list(neg(ipmask("true")))';
		o.placeholder = _('any');

		return m.render();
	}
});
