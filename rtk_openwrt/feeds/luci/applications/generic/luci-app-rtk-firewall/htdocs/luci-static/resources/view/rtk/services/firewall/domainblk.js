'use strict';
'require view';
'require uci';
'require rpc';
'require form';

function url_txt(s) {
	var str = uci.get('firewall', s, 'domain');
	
	return _('%q')
				.replace(/%s/g, str).replace(/%q/g, '"' + str + '"').replace(/,/g, '"   "');
}

return view.extend({
	load: function() {
		return Promise.all([			
			uci.load('luci'),
			uci.load('firewall')
		]);
	},

	render: function(hosts) {
		var m, s, o;

		m = new form.Map('firewall', _('Firewall - Domain Blocking'),
			_('This page is used to configure the Blocked Domain(Such as yahoo.com).'));

		s = m.section(form.TypedSection, 'include', _('General Settings'));
		s.anonymous = true;

		s.filter = function(section_id) {
			return (uci.get('firewall', section_id, 'path') == '/etc/firewall_domainRule.user');
		};

		o = s.option(form.Flag, 'enabled', _('Domain Blocking'));
		o.default = o.enabled;
		o.editable = true;
		o.rmempty = false;

		s = m.section(form.GridSection, 'dropDomain', _('Rules'));
		s.addremove = false;
		s.anonymous = true;
		s.sortable  = true;

		s.sectiontitle = function(section_id) {
			return _('Blocked List');
		};

		if (!uci.get('firewall', 'domainID')) {
			uci.add('firewall', 'dropDomain', 'domainID');
		}

		o = s.option(form.DynamicList, 'domain', _('Domain'));
		o.datatype = 'host(0)';
		o.load = function(section_id) {
			return uci.get('firewall', 'domainID', 'domain');
		};
		o.textvalue = function(s) {
			return E('small', [
				url_txt(s)
			]);
		};
		
		return m.render();
	}
});