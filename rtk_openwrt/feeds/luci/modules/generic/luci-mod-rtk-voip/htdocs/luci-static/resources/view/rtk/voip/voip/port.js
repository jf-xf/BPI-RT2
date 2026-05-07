'use strict';
'require view';
'require uci';
'require form';

var setData = {
	sid: null,
	option: null,
	flag: null,
	shift: null,
	listName: null,
	listValue: null
};

var f_voip_uci = 'rtkvoip';
var proxyEn_name = [];
var proxyEn_value = [];
var fxsflag_name = [];
var fxsflag_value = [];
const port_max = 16;
const codec_max = 9;
var codec_name = ['G711-ulaw', 'G711-alaw', 'G729', 'G723', 'G726-16k', 'G726-24k', 'G726-32k', 'G726-40k', 'G722'];

/* return bit value */
function resolve_cfg_bit(sid, option, shift)
{
	return ((uci.get(f_voip_uci, sid, option) & (0x01<<shift))? 1:0);
}

function set_cfg_bit(data)
{
	var targetValue;

	if (data.listName.length > 0) {
		for (var idx = 0; idx < data.listName.length; idx++) {
			if (data.listName[idx] == data.sid) {
				targetValue = data.listValue[idx];
				break;
			}
		}
	}

	targetValue=targetValue&(~(0x01<<data.shift)) | (data.flag<<data.shift);

	/* Update value */
	data.listValue[idx] = targetValue;
	uci.set(f_voip_uci, data.sid, data.option, targetValue);
}

function set_section_child(s)
{
	var o;

	for (var i = 0; i < s.children.length; i++) {
		o = s.children[i];
		o.rmempty = false;
		o.retain = true;
	}
}

function render_codec(section_id)
{
	var port_idx = parseInt(section_id.replace('VOIP_PORT', ''));
	var fsize_list = uci.get(f_voip_uci, section_id, 'FRAME_SIZE');
	var precd_list = uci.get(f_voip_uci, section_id, 'PRECEDENCE');
	var max;
	var scripts;
	var table = E('table', { 'class': 'table' }, [
		E('tr', { 'class': 'tr table-titles', 'aligh': 'center' }, [
				E('th', { 'class': 'th', 'rowspan': 2 }, _('Type')),
				E('th', { 'class': 'th', 'rowspan': 2 }, _('Packetiztion')),
				E('th', { 'class': 'th center', 'colspan': 9 }, _('Precedence')),
				E('th', { 'class': 'th', 'rowspan': 2 }, _('Disable')),
		]),
		E('tr', { 'class': 'tr', 'align': 'center' }, [
			E('td', { 'class': 'td' }, '1'),
			E('td', { 'class': 'td' }, '2'),
			E('td', { 'class': 'td' }, '3'),
			E('td', { 'class': 'td' }, '4'),
			E('td', { 'class': 'td' }, '5'),
			E('td', { 'class': 'td' }, '6'),
			E('td', { 'class': 'td' }, '7'),
			E('td', { 'class': 'td' }, '8'),
			E('td', { 'class': 'td' }, '9')
		])
	]);

	if (typeof(fsize_list)!='undefined' && Array.isArray(fsize_list))
		max = Math.min(fsize_list.length,codec_max);
	else
		max = 0;
	scripts = 'var max=%d;\n'.format(max);
	for (var i = 0; i < max; i++) {
		var tds=[];

		tds.push(E('td', { 'class': 'td' }, codec_name[i]));
		tds.push(E('td', { 'class': 'td' }, E('select', { 'name':'fsize%d'.format(port_idx) }, [
					E('option', { 'value': 0 }, '10 ms'),
					E('option', { 'value': 1 }, '20 ms'),
					E('option', { 'value': 2 }, '30 ms'),
					E('option', { 'value': 3 }, '40 ms')])));
		for (var k = 0; k < max; k++)
			tds.push(E('td', { 'class': 'td' }, E('input', { 'type': 'checkbox', 'name':'precd%d'.format(port_idx), 'onclick': 'check_precd(document.getElementsByName(\'precd%d\'), document.getElementsByName(\'codec_disable%d\'), %d, %d, %d)'.format(port_idx,port_idx,i,k,max) })));
		tds.push(E('td', { 'class': 'td' }, E('input', { 'type': 'checkbox', 'name':'codec_disable%d'.format(port_idx) })));

		table.appendChild(
			E('tr', { 'class': 'tr' }, tds)
		);
		scripts += 'document.getElementsByName(\'fsize%d\')[%d].value=%d;\n'.format(port_idx, i, fsize_list[i]);
		if (parseInt(precd_list[i])<max)
			scripts += 'document.getElementsByName(\'precd%d\')[%d].checked=%d;\n'.format(port_idx, i*max+parseInt(precd_list[i]), 1);
		else
			scripts += 'document.getElementsByName(\'codec_disable%d\')[%d].checked=%d;\n'.format(port_idx, i, 1);
	};

	scripts += 'function check_precd(objs, disable_obj, row, col, len)\n{\n'+
		'var x, y;\n'+
		'var i, idx;\n'+
		'var obj;\n\n'+
		'x=-1; y=-1\n'+
		'obj = objs[row * len + col];\n'+
		'disable_obj[row].checked = false;\n\n'+
		'if (obj.checked == true) {\n'+
		'\tfor (i=0; i<len; i++) {\n'+
		'\t\tidx = row * len + i;\n'+
		'\t\tif (obj == objs[idx])\n'+
		'\t\t\tcontinue;\n'+
		'\t\tif (objs[idx].checked == true) {\n'+
		'\t\t\tx = i;\n'+
		'\t\t\tobjs[idx].checked = false;\n'+
		'\t\t\tbreak;\n'+
		'\t\t}\n'+
		'\t}\n\n'+
		'\tfor (i=0; i<len; i++) {\n'+
		'\t\tidx = i * len + col;\n'+
		'\t\tif (obj == objs[idx])\n'+
		'\t\t\tcontinue;\n'+
		'\t\tif (objs[idx].checked == true) {\n'+
		'\t\t\ty = i;\n'+
		'\t\t\tobjs[idx].checked = false;\n'+
		'\t\t\tbreak;\n'+
		'\t\t}\n'+
		'\t}\n\n'+
		'\tif ((x != -1) && (y != -1))\n'+
		'\t\tobjs[x + y * len].checked = true;\n'+
		'}\n'+
		'else {\n'+
		'\tobj.checked = true;\n'+
		'}\n'+
		'}\n';

	return E([], [table, E('script', scripts)]);
}

function arrayEquals(a, b, num)
{
	var aa, bb;

	if (num > 0) {
		aa = a.filter((p, index) => index < num);
		bb = b.filter((p, index) => index < num);
	}
	else {
		aa = a;
		bb = b;
	}

	return Array.isArray(aa) &&
		Array.isArray(bb) &&
		aa.length === bb.length &&
		aa.every((val, index) => val === bb[index]);
}

function write_codec(sid)
{
	var port_idx = parseInt(sid.replace('VOIP_PORT', ''));
	var precd_objs = document.getElementsByName('precd%d'.format(port_idx));
	var fsize_objs = document.getElementsByName('fsize%d'.format(port_idx));
	var disable_objs = document.getElementsByName('codec_disable%d'.format(port_idx));
	var uci_fsize = uci.get(f_voip_uci, sid, 'FRAME_SIZE');
	var uci_precd = uci.get(f_voip_uci, sid, 'PRECEDENCE');
	var form_fsize = [];
	var form_precd = [];
	var precd_val;
	var max;

	if (typeof(uci_fsize)!='undefined' && Array.isArray(uci_fsize))
		max = Math.min(uci_fsize.length,codec_max);
	else
		max = 0;
	for (var row=0; row<max; row++) {
		form_fsize.push(fsize_objs[row].value);
		precd_val = max;
		for (var col=0; col<max; col++) {
			if (precd_objs[row*max+col].checked) {
				precd_val = col;
				break;
			}
		}
		if (disable_objs[row].checked)
			precd_val = max;
		form_precd.push(precd_val.toString());
	}
	if (!arrayEquals(uci_fsize, form_fsize, max)) {
		for (var i=0; i<max; i++)
			uci_fsize[i] = form_fsize[i];
		uci.set(f_voip_uci, sid, 'FRAME_SIZE', uci_fsize);
	}
	if (!arrayEquals(uci_precd, form_precd, max)) {
		for (var i=0; i<max; i++)
			uci_precd[i] = form_precd[i];
		uci.set(f_voip_uci, sid, 'PRECEDENCE', uci_precd);
	}
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load(f_voip_uci)
		]);
	},

	render: function(data) {
		var m, s, o, ss, sss;
		var sectionName;
		var port_num;

		port_num = uci.sections(f_voip_uci, 'voipCfgPortParam').length;
		if (port_num > port_max)
			port_num = port_max;

		m = new form.Map(f_voip_uci, [_('FXS port(s)')]);
		m.tabbed = true;

		for (var port_idx = 0; port_idx < port_num;  port_idx++) {
			var codec_list = [];
			s = m.section(form.NamedSection, 'VOIP_PORT'+port_idx, _('Port')+port_idx);

			/*-- Default Proxy --*/
			o = s.option(form.SectionValue, 'VOIP_PORT'+port_idx, form.NamedSection, 'VOIP_PORT'+port_idx, 'VOIP_PORT'+port_idx, _('Default Proxy'));
			ss = o.subsection;
			o = ss.option(form.ListValue,'DEFAULT_PROXY',_('Select Default Proxy'));
			o.value('0', _('Proxy0'));
			o.value('1', _('Proxy1'));

			/*-- Proxy0/Proxy1 --*/
			for (var proxy_idx = 0; proxy_idx < 2; proxy_idx++) {
				sectionName = 'VOIP_PORT%d_PROXY%d'.format(port_idx, proxy_idx);
				proxyEn_name.push(sectionName);
				proxyEn_value.push(uci.get(f_voip_uci, sectionName, 'PROXY_ENABLE'));
				o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Proxy')+proxy_idx);
				ss = o.subsection;
				o = ss.option(form.Value,'PROXY_DISPLAY_NAME',_('Display Name'));
				o = ss.option(form.Value,'PROXY_NUMBER',_('Number'));
				o = ss.option(form.Value,'PROXY_LOGIN_ID',_('Login ID'));
				o = ss.option(form.Value,'PROXY_PASSWORD',_('Password'));
				o = ss.option(form.Flag,'_tmp1',_('Proxy'));
				o.rmempty = false;
				o.cfgvalue = function(section_id) {
					return resolve_cfg_bit(section_id, 'PROXY_ENABLE', 0);
				};
				o.write = function(section_id, value) {
					setData.sid = section_id;
					setData.option = 'PROXY_ENABLE';
					setData.flag = value;
					setData.shift = 0;
					setData.listName = proxyEn_name;
					setData.listValue = proxyEn_value;
					set_cfg_bit(setData);
				};
				o = ss.option(form.Value,'PROXY_ADDR',_('Proxy Addr'));
				o.depends('_tmp1', '1');
				o.retain = true;
				o.datatype = 'host(1)'
				o = ss.option(form.Value,'PROXY_PORT',_('Proxy Port'));
				o.depends('_tmp1', '1');
				o.default = 5060;
				o.rmempty = false;
				o.retain = true;
				o.datatype = 'port'
				o = ss.option(form.Flag,'_tmp2',_('SIP Subscribe'));
				o.rmempty = false;
				o.cfgvalue = function(section_id) {
					return resolve_cfg_bit(section_id, 'PROXY_ENABLE', 2);
				};
				o.write = function(section_id, value) {
					setData.sid = section_id;
					setData.option = 'PROXY_ENABLE';
					setData.flag = value;
					setData.shift = 2;
					setData.listName = proxyEn_name;
					setData.listValue = proxyEn_value;
					set_cfg_bit(setData);
				};

				o = ss.option(form.Value,'PROXY_DOMAIN_NAME',_('SIP Domain'));
				o = ss.option(form.Value,'REG_EXPIRE',_('Reg Expire(sec)'));
				o.default = 3600;
				o.datatype = 'uinteger'
				o.rmempty = false;
				o = ss.option(form.Flag,'PROXY_OUTBOUND_ENABLE',_('Outbound Proxy'));
				o.rmempty = false;
				o = ss.option(form.Value,'PROXY_OUTBOUND_ADDR',_('Outbound Proxy Addr'));
				o.depends('PROXY_OUTBOUND_ENABLE', '1');
				o.rmempty = false;
				o.retain = true;
				o.datatype = 'host(1)'
				o = ss.option(form.Value,'PROXY_OUTBOUND_PORT',_('Outbound Proxy Port'));
				o.depends('PROXY_OUTBOUND_ENABLE', '1');
				o.default = 5060;
				o.rmempty = false;
				o.retain = true;
				o.datatype = 'port'
				o = ss.option(form.Flag,'_tmp3',_('Enable Session Timer'));
				o.rmempty = false;
				o.cfgvalue = function(section_id) {
					return (uci.get(f_voip_uci, section_id, 'PROXY_SESSION_EXPIRE')!=0? 1:0);
				};
				o.write = function(section_id, value) {
					if (value==0)
						uci.set(f_voip_uci, section_id, 'PROXY_SESSION_EXPIRE', 0);
				};
				o = ss.option(form.Value,'PROXY_SESSION_EXPIRE',_('Session Expire(sec)'));
				o.depends('_tmp3', '1');
				o.default = 1800;
				o.datatype = 'uinteger'
				o.rmempty = false;
				o.retain = true;

				//set_section_child(ss);
			}

			/*-- SIP Advanced --*/
			/* Get per-port FXSPORT_FLAG */
			sectionName = 'VOIP_PORT'+port_idx;
			fxsflag_name.push(sectionName);
			fxsflag_value.push(uci.get(f_voip_uci, sectionName, 'FXSPORT_FLAG'));

			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('SIP Advanced'));
			ss = o.subsection;
			o = ss.option(form.Value,'SIP_PORT',_('SIP Port'));
			o.default = 5060;
			o.datatype = 'port'
			o = ss.option(form.Value,'MEDIA_PORT',_('Media Port'));
			o.default = 9000;
			o.datatype = 'port'
			o = ss.option(form.ListValue,'DTMF_MODE',_('DTMF Relay'));
			o.value('0', _('RFC2833'));
			o.value('1', _('SIP INFO'));
			o.value('2', _('Inband'));
			o.value('3', _('DTMF_delete'));

			o = ss.option(form.Value,'DTMF_RFC2833_PAYLOAD_TYPE',_('DTMF RFC2833 Payload Type'));
			o.depends('DTMF_MODE', '0');
			o.datatype = 'uinteger'
			o = ss.option(form.Value,'DTMF_RFC2833_PI',_('DTMF RFC2833 Packet Interval'));
			o.depends('DTMF_MODE', '0');
			o.datatype = 'uinteger'

			o = ss.option(form.Flag,'FAXMODEM_RFC2833_PT_SAME_DTMF',_('Use DTMF RFC2833 PT as Fax/Modem RFC2833 PT'));

			o = ss.option(form.Value,'FAXMODEM_RFC2833_PT',_('Fax/Modem RFC2833 Payload Type'));
			o.depends('DTMF_MODE', '0');
			o.datatype = 'uinteger'

			o = ss.option(form.Value,'FAXMODEM_RFC2833_PI',_('Fax/Modem RFC2833 Packet Interval'));
			o.depends('DTMF_MODE', '0');
			o.datatype = 'uinteger'

			o = ss.option(form.Value,'SIP_INFO_DURATION',_('SIP INFO Duration (ms)'));
			o.depends('DTMF_MODE', '1');
			o.datatype = 'uinteger'

			o = ss.option(form.Flag,'CALL_WAITING_ENABLE',_('Call Waiting'));
			o = ss.option(form.Flag,'CALL_WAITING_CID',_('Call Waiting Caller ID'));
			o = ss.option(form.Flag,'_fxsflag0',_('Reject Direct IP Call'));
			o.rmempty = false;
			o.cfgvalue = function(section_id) {
				return resolve_cfg_bit(section_id, 'FXSPORT_FLAG', 0);
			};
			o.write = function(section_id, value) {
				setData.sid = section_id;
				setData.option = 'FXSPORT_FLAG';
				setData.flag = value;
				setData.shift = 0;
				setData.listName = fxsflag_name;
				setData.listValue = fxsflag_value;
				set_cfg_bit(setData);
			};

			o = ss.option(form.Flag,'_fxsflag1',_('Send Caller ID hidden'));
			o.rmempty = false;
			o.cfgvalue = function(section_id) {
				return resolve_cfg_bit(section_id, 'FXSPORT_FLAG', 1);
			};
			o.write = function(section_id, value) {
				setData.sid = section_id;
				setData.option = 'FXSPORT_FLAG';
				setData.flag = value;
				setData.shift = 1;
				setData.listName = fxsflag_name;
				setData.listValue = fxsflag_value;
				set_cfg_bit(setData);
			};

			o = ss.option(form.Flag,'_fxsflag3',_('Call Transfer'));
			o.rmempty = false;
			o.cfgvalue = function(section_id) {
				return resolve_cfg_bit(section_id, 'FXSPORT_FLAG', 3);
			};
			o.write = function(section_id, value) {
				setData.sid = section_id;
				setData.option = 'FXSPORT_FLAG';
				setData.flag = value;
				setData.shift = 3;
				setData.listName = fxsflag_name;
				setData.listValue = fxsflag_value;
				set_cfg_bit(setData);
			};

			o = ss.option(form.Flag,'_fxsflag2',_('3-way Conference'));
			o.rmempty = false;
			o.cfgvalue = function(section_id) {
				return resolve_cfg_bit(section_id, 'FXSPORT_FLAG', 2);
			};
			o.write = function(section_id, value) {
				setData.sid = section_id;
				setData.option = 'FXSPORT_FLAG';
				setData.flag = value;
				setData.shift = 2;
				setData.listName = fxsflag_name;
				setData.listValue = fxsflag_value;
				set_cfg_bit(setData);
			};

			o = ss.option(form.ListValue,'_fxsflag4',_('Conference on server/CPE'));
			o.value('0', _('CPE'));
			o.value('1', _('Server'));
			o.rmempty = false;
			o.cfgvalue = function(section_id) {
				return resolve_cfg_bit(section_id, 'FXSPORT_FLAG', 4).toString();
			};
			o.write = function(section_id, value) {
				setData.sid = section_id;
				setData.option = 'FXSPORT_FLAG';
				setData.flag = value;
				setData.shift = 4;
				setData.listName = fxsflag_name;
				setData.listValue = fxsflag_value;
				set_cfg_bit(setData);
			};

			o = ss.option(form.Value,'CONFERENCE_URI',_('Conference-uri'));
			o.depends('_fxsflag4', '1');

			set_section_child(ss);

			/*-- Forward Mode --*/
			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Forward Mode'));
			ss = o.subsection;
			o = ss.option(form.ListValue,'UC_FORWARD_ENABLE',_('Immediate Forward to'));
			o.value('0', _('Off'));
			o.value('1', _('VoIP'));
			//o.value('2', _('PSTN'));
			o.rmempty = false;

			o = ss.option(form.Value,'UC_FORWARD',_('Immediate Number'));
			o.depends({UC_FORWARD_ENABLE: '0', '!reverse': true});

			o = ss.option(form.ListValue,'BUSY_FORWARD_ENABLE',_('Busy Forward to'));
			o.value('0', _('Off'));
			o.value('1', _('VoIP'));
			o.rmempty = false;

			o = ss.option(form.Value,'BUSY_FORWARD',_('Busy Number'));
			o.depends({BUSY_FORWARD_ENABLE: '0', '!reverse': true});

			o = ss.option(form.ListValue,'NA_FORWARD_ENABLE',_('No Answer Forward to'));
			o.value('0', _('Off'));
			o.value('1', _('VoIP'));
			o.rmempty = false;

			o = ss.option(form.Value,'NA_FORWARD',_('No Answer Number'));
			o.depends({NA_FORWARD_ENABLE: '0', '!reverse': true});

			o = ss.option(form.Value,'NA_FORWARD_TIME',_('No Answer Time(sec)'));
			o.depends({NA_FORWARD_ENABLE: '0', '!reverse': true});
			o.datatype = 'uinteger';

			set_section_child(ss);

			/*-- Dial Plan --*/
			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Dial Plan'));
			ss = o.subsection;
			o = ss.option(form.Flag,'DIGITMAP_ENABLE',_('Enable Dialplan'));
			o.rmempty = false;

			o = ss.option(form.Value,'DIAL_PLAN',_('Dial plan'));
			o.depends('DIGITMAP_ENABLE', '1');

			set_section_child(ss);

			/*-- Codec --*/
			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Codec'));
			ss = o.subsection;
			ss.tab('rtp', _('RTP Redundant'));
			ss.tab('codec', _('Codec Settings'));
			ss.tab('option', _('Option'));

			/* tab: RTP Redundant */
			o = ss.taboption('rtp', form.ListValue,'RTP_RED_CODEC',_('Codec'));
			o.value('0', _('Disabled'));
			o.value('1', _('PCM u-law'));
			o.value('2', _('PCM a-law'));
			o.value('3', _('G.729'));
			o.rmempty = false;

			o = ss.taboption('rtp', form.Value,'RTP_RED_PAYLOAD_TYPE',_('Payload Type'));
			o.datatype = 'uinteger'

			/* tab: Codec Settings */
			o = ss.taboption('codec', form.DummyValue,'_header');
			o.cfgvalue = L.bind(function(section_id) {
				return render_codec(section_id);
			}, o);

			o.write = function(section_id, value) {
				write_codec(section_id);
			}

			/* tab: Option */
			o = ss.taboption('option', form.ListValue,'G726_PACK',_('G726 Packing Order'));
			o.value('1', _('Left'));
			o.value('2', _('Right'));
			o.rmempty = false;

			o = ss.taboption('option', form.ListValue,'G7231_RATE',_('G723 Bit Rate'));
			o.value('0', _('6.3k'));
			o.value('1', _('5.3k'));
			o.rmempty = false;

			set_section_child(ss);

			/*-- Hot Line --*/
			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Hot Line'));
			ss = o.subsection;
			o = ss.option(form.Flag,'HOTLINE_ENABLE',_('Use Hot Line'));
			o.rmempty = false;

			o = ss.option(form.Value,'HOTLINE_NUMBER',_('Hot Line Number'));
			o.depends('HOTLINE_ENABLE', '1');
			o.validate = function(section_id, value) {
				var num = this.formvalue(section_id);
				if (num == null || num == '')
					return _('Hot Line Number 	must be specified!');

				return true;
			};

			set_section_child(ss);

			/*-- DND(Don't Disturb) --*/
			o = s.option(form.SectionValue, sectionName, form.GridSection, 'voipCfgPortParam', _('DND\(Don\'t Disturb\)'));
			o.editable = false;
			ss = o.subsection;
			ss.anonymous = true;
			ss.filter = L.bind(function(idx, section_id) {
				if (section_id == 'VOIP_PORT%d'.format(idx))
					return true;
				return false;
			}, o, port_idx);

			o = ss.option(form.ListValue,'DND_MODE',_('DND Mode'));
			o.value('0', _('Always'));
			o.value('1', _('Enable'));
			o.value('2', _('Disable'));
			o.rmempty = false;
			o.textvalue = function(section_id) {
				var mode;
				mode = uci.get(f_voip_uci, section_id, 'DND_MODE');
				switch(mode) {
					case '0':
						return 'Always';
					case '1':
						return 'Enable';
					case '2':
						return 'Disable';
					default:;
				};
				return 'Undefined';
			};

			o = ss.option(form.Value,'DND_FROM_HOUR',_('From(hour)'));
			o.depends('DND_MODE', '1');
			o = ss.option(form.Value,'DND_FROM_MIN',_('From(min)'));
			o.depends('DND_MODE', '1');
			o = ss.option(form.Value,'DND_TO_HOUR',_('To(hour)'));
			o.depends('DND_MODE', '1');
			o = ss.option(form.Value,'DND_TO_MIN',_('To(min)'));
			o.depends('DND_MODE', '1');

			set_section_child(ss);

			/*-- Alarm --*/
			o = s.option(form.SectionValue, sectionName, form.NamedSection, sectionName, sectionName, _('Alarm'));
			ss = o.subsection;
			o = ss.option(form.Flag,'ALARM_ENABLE',_('Enable'));
			o.rmempty = false;
			o = ss.option(form.Value,'ALARM_TIME_HH',_('Time(hour)'));
			o.depends('ALARM_ENABLE', '1');
			o.datatype = 'uinteger';
			o = ss.option(form.Value,'ALARM_TIME_MM',_('Time(min)'));
			o.depends('ALARM_ENABLE', '1');
			o.datatype = 'uinteger';

			set_section_child(ss);
		}

		return m.render();
	}
});
