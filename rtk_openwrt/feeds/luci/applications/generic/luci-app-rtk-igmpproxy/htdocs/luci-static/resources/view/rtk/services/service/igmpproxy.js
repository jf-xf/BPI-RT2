'use strict';
'require view';
'require form';
'require uci';
'require rtk.network as rtkNetwork';
'require tools.rtk.widgets as rtkwidgets';

return view.extend({

	render: function (res) {
		var m, s, o;
		
		m = new form.Map('igmpproxy', _('IGMPproxy'));
		
		s = m.section(form.TypedSection, 'phyint', _('Proxy Instance'));
		s.anonymous = true;
		s.filter = function(section_id){
			if(uci.get('igmpproxy', section_id, 'direction') == 'downstream'){
					return true;
				}
			return false;
		};
		
		//general setting
		o = s.option( rtkwidgets.NetworkSelect, '_ip4wan_sel', _("WAN Interface Select"));
		o.valueType="IPv4";
		o.ipmodefilter="IPv4";
		o.ignoreBridge=1;
		o.multiple = true;
		o.nocreate=true;
		
		o.cfgvalue = function(section_id) {
			var select_v4intf_arr=[];
			var sections;
			
			sections=uci.sections('igmpproxy', 'phyint');
			for(var i=1; i<sections.length;i++){
				if(sections[i]['direction']!='upstream')
					continue;
				select_v4intf_arr.push(sections[i]['network']);
			}
			return select_v4intf_arr;
		};

		o.remove = function(section_id){
			var sections=uci.sections('igmpproxy', 'phyint');
			//delete upstreams if select none
			for(var i = 0; i < sections.length; i++){
				if(sections[i]['direction'] == 'upstream'){
					uci.remove('igmpproxy', sections[i]['.name'] );
				}
			}
		}

		o.write = function(section_id, formvalue){
			formvalue = L.toArray(formvalue);
			var sections=uci.sections('igmpproxy', 'phyint');

			//delete upstreams first
			for(var i = 0; i < sections.length; i++){
				if(sections[i]['direction'] == 'upstream'){
					//console.log(sections[i]);
					uci.remove('igmpproxy', sections[i]['.name'] );
				}
			}
			
			if(formvalue){
				for(var i = 0; i < formvalue.length; i++){
					var new_section_id = uci.add('igmpproxy', 'phyint');
					uci.set('igmpproxy', new_section_id, 'network', formvalue[i]);
					uci.set('igmpproxy', new_section_id, 'zone', 'wan');
					uci.set('igmpproxy', new_section_id, 'direction', 'upstream');
					uci.set('igmpproxy', new_section_id, 'altnet', '0.0.0.0/0');
				}
			}
		};

		//advance setting
		s = m.section(form.TypedSection, 'igmpproxy', 'Advance');
		s.anonymous = true;
		
		o = s.option( form.Value, 'query_response_interval', _('Query Response Interval:(s)'));
		o.default = '10';

		o = s.option( form.Value, 'query_interval', _('Query Interval:(s)'));
		o.default = '125';

		o = s.option( form.Value, 'last_listener_query_interval', _('Last Listener Query Interval:(s)'));
		o.default = '10';
		
		o = s.option( form.Value, 'robustness', _('Robustness'));
		o.default = '2';
		
		o = s.option( form.Value, 'last_listener_query_count', _('Last Listener Query Count'));
		o.default = '2';

		return m.render();
	}
});
