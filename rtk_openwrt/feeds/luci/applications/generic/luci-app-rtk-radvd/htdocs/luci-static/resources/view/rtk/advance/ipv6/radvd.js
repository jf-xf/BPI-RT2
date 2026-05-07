'use strict';
'require view';
'require fs';
'require ui';
'require uci';
'require form';
'require rpc';
'require dom';

function validateNull(sid, s) {
	if (s == null || s == '')
		return _("This item should NOT be NULL");

	return true;
}

function addRAFlags(section_id, formvalue){
	if(formvalue != 'managed-config'
	&& formvalue != 'other-config'
	&& formvalue != 'home-agent'
	&& formvalue != 'none'){
		return;
	}

	var found=0;
	var ra_flags=uci.get('dhcp', section_id, 'ra_flags');
	var output_flags=[];
	for (var i = 0; i < ra_flags.length; i++) {
		if(ra_flags[i] == formvalue){
			found = 1;
		}
	}

	if(!found){
		ra_flags.push(formvalue);
		uci.set('dhcp', section_id, 'ra_flags', ra_flags);
	}

	return;
}

function delRAFlags(section_id, formvalue){
	var ra_flags=uci.get('dhcp', section_id, 'ra_flags');
	var new_flags=[];
	var output_flags=[];
	for (var i = 0; i < ra_flags.length; i++) {
		if(ra_flags[i] != formvalue){
			new_flags.push(ra_flags[i]);
		}
	}

	if(new_flags.length){
		uci.set('dhcp', section_id, 'ra_flags', new_flags);
	}else{
		new_flags.push('none');
		uci.set('dhcp', section_id, 'ra_flags', new_flags);
	}
	return;
}

return view.extend({
	render: function() {
		var m, s, o;
		
		m = new form.Map('dhcp', _("RA Configuration"));
		s = m.section(form.TypedSection, 'dhcp');
		s.anonymous = true;
		s.filter = function(section_id) {
			if(section_id == "lan")
				return true;
		
			return false;
		};
		
		o = s.option(form.ListValue, 'ra', _('Mode'));
		o.datatype = 'string';
		o.value("disabled", _("Disable"));
		o.value("server", _("Enable"));
		o.default = "";
		
		o = s.option(form.Value, 'ra_maxinterval', _('Max RA Interval'));
		o.depends({ra:"disabled", "!reverse":true});
		o.validate = validateNull;
		o.optional = true;
		o.default = '600';
		
		o = s.option(form.Value, 'ra_mininterval', _('Min RA Interval'));
		o.depends({ra:"disabled", "!reverse":true});
		o.validate = validateNull;
		o.optional = true;
		o.default = '200';
		
		o = s.option(form.Flag, 'ra_management', _('AdvManageFlag'));
		o.enabled='managed-config';
		o.depends({ra:"disabled", "!reverse":true});
		o.write = addRAFlags;
		o.remove = function(section_id){
			return delRAFlags(section_id, this.enabled);
		};
		o.cfgvalue= function(section_id){
			var ra_flags=uci.get('dhcp', section_id, 'ra_flags');
			for (var i = 0; i < ra_flags.length; i++) {
				if(ra_flags[i] == 'managed-config'){
					console.log(ra_flags[i]);
					return this.enabled;
				}
			}
			return 0;
		};
		
		o = s.option(form.Flag, 'ra_other', _('AdvOtherConfigFlag'));
		o.enabled='other-config';
		o.depends({ra:"disabled", "!reverse":true});
		o.write = addRAFlags;
		o.remove = function(section_id){
			return delRAFlags(section_id, this.enabled);
		};
		o.cfgvalue= function(section_id){
			var ra_flags=uci.get('dhcp', section_id, 'ra_flags');
			for (var i = 0; i < ra_flags.length; i++) {
				if(ra_flags[i] == 'other-config'){
					console.log(ra_flags[i]);
					return this.enabled;
				}
			}
			return 0;
		};
		
		return m.render();
	}
});
