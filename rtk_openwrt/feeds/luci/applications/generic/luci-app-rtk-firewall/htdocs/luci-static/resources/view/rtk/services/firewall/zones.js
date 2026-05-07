'use strict';
'require view';
'require uci';
'require form';

return view.extend({
	render: function() {
		return this.renderZones();
	},

	renderZones: function() {
		var m, s, o;

		m = new form.Map('firewall', _('Firewall - Default Settings'));

		s = m.section(form.TypedSection, 'defaults', _('General Settings'));
		s.anonymous = true;
		s.addremove = false;

		var p = [
			s.option(form.ListValue, 'input', _('Input')),
			s.option(form.ListValue, 'output', _('Output')),
			s.option(form.ListValue, 'forward', _('Forward'))
		];

		for (var i = 0; i < p.length; i++) {
			p[i].value('REJECT', _('reject'));
			p[i].value('DROP', _('drop'));
			p[i].value('ACCEPT', _('accept'));
		}

		s = m.section(form.GridSection, 'forwarding', _('Forwarding Settings'));
		s.anonymous = true;
		s.renderRowActions = function() {return E([]);};
		s.cfgsections = function() {
			var sections = uci.sections('firewall', 'forwarding');
			var filterSections = sections.filter(function(z) { return z.name == 'Incoming-Forwarding' || z.name == 'Outgoing-Forwarding' });
			var section_ids = filterSections.sort(function(a, b) { return a.name > b.name }).map(function(s) { return s['.name'] });
			return section_ids;
		};

		o = s.option(form.Value, 'name', _('Direction'));
		o.readonly = true;
		//o.modalonly = false;

		o = s.option(form.ListValue, 'enabled', _('Action'));
		o.value('0', _('Deny'));
		o.value('1', _('Allow'));
		o.editable = true;

		return m.render();
	}
});
