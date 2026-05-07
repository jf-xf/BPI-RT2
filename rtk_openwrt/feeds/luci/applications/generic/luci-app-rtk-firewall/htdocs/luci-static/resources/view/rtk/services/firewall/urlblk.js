'use strict';
'require view';
'require uci';
'require rpc';
'require form';

function url_txt(s) {
	var str = uci.get('firewall', s, 'fqdn');
	
	//var str = 'This is Apple.';
	//var newStr = str.replace('Apple', 'Banana');
	
	return _('%q')
				.replace(/%s/g, str).replace(/%q/g, '"' + str + '"').replace(/,/g, '"   "');
	//return newStr;
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

		m = new form.Map('firewall', _('Firewall - URL Blocking'),

			_('This page is used to configure the Blocked FQDN(Such as tw.yahoo.com).'));
		s = m.section(form.TypedSection, 'include', _('General Settings'));
		s.anonymous = true;

		s.filter = function(section_id) {
			return (uci.get('firewall', section_id, 'path') == '/etc/firewall_urlRule.user');
		};

		o = s.option(form.Flag, 'enabled', _('URL Blocking'));
		o.default = o.enabled;
		o.editable = true;
		o.rmempty = false;

		s = m.section(form.GridSection, 'dropURL', _('Rules'));
		s.addremove = false;
		s.anonymous = true;
		s.sortable  = true;

		s.sectiontitle = function(section_id) {
			return _('Blocked List');
		};

		if (!uci.get('firewall', 'urlID')) {
			uci.add('firewall', 'dropURL', 'urlID');
		}

		o = s.option(form.DynamicList, 'fqdn', _('URL'));
		o.datatype = 'host(0)';
		o.load = function(section_id) {
			return uci.get('firewall', 'urlID', 'fqdn');
		};
		o.textvalue = function(s) {
			return E('small', [
				url_txt(s)
			]);
		};
		
		return m.render();
	}
});