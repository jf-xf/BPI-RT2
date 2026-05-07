'use strict';
'require view';
'require form';
'require uci';
'require rtk.network as rtkNetwork';
'require tools.rtk.widgets as rtkwidgets';

return view.extend({

	render: function (res) {
		var m, s, o;

		m = new form.Map('mcproxy', _('MLDproxy'));

		s = m.section(form.GridSection, 'instance', _('Proxy Instance'));
		s.anonymous = true;

		o = s.option( form.Value, 'downstream', _("Downstream Interface"));
		o.readonly=true;

		o = s.option( rtkwidgets.NetworkSelect, 'upstream', _("Upstream Interfaces"));
		o.valueType = "netdev";
		o.ipmodefilter = "IPv6";
		o.ignoreBridge = 1;
		o.multiple = true;//false: option, true: list
		o.nocreate = true;
		o.textvalue=function(section_id){
			var cfgvalue=L.toArray(this.super('textvalue', arguments));

			var table = E('table', { 'class': 'table' });
			for (var i = 0; i < cfgvalue.length; i++) {
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left'}, [ rtkNetwork.getNetdevDisplay(cfgvalue[i]) ])
				]));
			}

			if(!cfgvalue.length){
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left'}, [ _("unspecifed") ])
				]));
			}

			return table;
		};

		//advance setting
		s = m.section(form.TypedSection, 'mcproxy', _('Advance'));
		s.anonymous = true;

		o = s.option( form.Value, 'qri', _('Query Response Interval:(s)'));
		o.default = '10';

		o = s.option( form.Value, 'qi', _('Query Interval:(s)'));
		o.default = '125';

		o = s.option( form.Value, 'lmqi', _('Last Listener Query Interval:(s)'));
		o.default = '10';

		o = s.option( form.Value, 'rv', _('Robustness'));
		o.default = '2';

		return m.render();
	}
});
