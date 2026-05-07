'use strict';
'require view';
'require uci';
'require form';

var f_voip_uci = 'rtkvoip';
const port_max = 16;

// return bit value
function resolve_cfg_bit(sid, option, shift)
{
	return ((uci.get(f_voip_uci, sid, option) & (0x01<<shift))? 1:0);
}

function set_cfg_bit(sid, option, shift, flag)
{
	var thisValue, targetValue;

	thisValue = uci.get(f_voip_uci, sid, option);
	targetValue = thisValue&(~(0x01<<shift)) | (flag<<shift);
	uci.set(f_voip_uci, sid, option, targetValue);
}

function syncPortSetting(option) {
	var port_num;

	port_num = uci.sections(f_voip_uci, 'voipCfgPortParam').length;
	if (port_num > port_max)
		port_num = port_max;

	for (var port_idx = 0; port_idx < port_num;  port_idx++) {
		uci.set(f_voip_uci, 'VOIP_PORT'+port_idx, option, uci.get(f_voip_uci, "VOIP_PORT0", option))
	}
}

function cloneSection(){
//	console.log('cloneSection');
//	console.log(m);

	/* V.152 */
	syncPortSetting("USE_V152");
	syncPortSetting("V152_PAYLOAD_TYPE");
	syncPortSetting("V152_CODEC_TYPE");

	/* T.38 */
	syncPortSetting("T38_USET38");
	syncPortSetting("FAX_MODEM_DET");
	syncPortSetting("T38_PARAM_ENABLE");
	syncPortSetting("T38_MAX_BUFFER");
	syncPortSetting("T38_RATE_MGT");
	syncPortSetting("T38_MAX_RATE");
	syncPortSetting("T38_ENABLE_ECM");
	syncPortSetting("T38_ECC_SIGNAL");
	syncPortSetting("T38_ECC_DATA");
	syncPortSetting("T38_ENABLE_SPOOF");
	syncPortSetting("T38_DUPLICATE_NUM");

	/* DSP */
	syncPortSetting("JITTER_DELAY");
	syncPortSetting("MAX_DELAY");
	syncPortSetting("JITTER_FACTOR");
	syncPortSetting("ECHO_TAIL");
	syncPortSetting("ECHO_LEC");
	syncPortSetting("ECHO_NLP");
	syncPortSetting("VAD");
	syncPortSetting("VAD_THRESHOLD");
	syncPortSetting("SID_MODE");
	syncPortSetting("SID_LEVEL");
	syncPortSetting("SID_GAIN");
	syncPortSetting("CNG");
	syncPortSetting("CNG_THRESHOLD");
	syncPortSetting("PLC");
	syncPortSetting("RTCP_INTERVAL_ENABLE");
	syncPortSetting("RTCP_INTERVAL");
	syncPortSetting("RTCPXR");
	syncPortSetting("FAX_MODEM_RFC2833");
	syncPortSetting("SPEAKERAGC");
	syncPortSetting("SPK_AGC_LVL");
	syncPortSetting("SPK_AGC_GU");
	syncPortSetting("SPK_AGC_GD");
	syncPortSetting("MICAGC");
	syncPortSetting("MIC_AGC_LVL");
	syncPortSetting("MIC_AGC_GU");
	syncPortSetting("MIC_AGC_GD");
	syncPortSetting("MIC_AGC_LVL");
	syncPortSetting("CALLER_ID_MODE");
	syncPortSetting("CID_DTMF_MODE");
	syncPortSetting("FLASH_HOOK_TIME_MIN");
	syncPortSetting("FLASH_HOOK_TIME");
	syncPortSetting("SPK_VOICE_GAIN");
	syncPortSetting("MIC_VOICE_GAIN");
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load(f_voip_uci)
		]);
	},

	render: function() {
		var m, s, o, ss;

		m = new form.Map(f_voip_uci, _('Advance'));

		s = m.section(form.NamedSection, 'VOIP_PORT0', _('Advanced Settings'));

		s.tab('v152', _('V.152'));
		s.tab('t38', _('T.38'));
		s.tab('dsp', _('DSP'));

		/* V.152 */
		o = s.taboption('v152', form.Flag, 'USE_V152', _('V.152'));
		o.rmempty = false;

		o = s.taboption('v152', form.Value, 'V152_PAYLOAD_TYPE', _('V.152 Payload Type'));
		o.depends('USE_V152', '1');
		o.datatype = 'uinteger';
		o.rmempty = false;
		o.retain = true;

		o = s.taboption('v152', form.ListValue, 'V152_CODEC_TYPE', _('V.152 Codec Type'));
		o.value('0', 'PCM u-law');
		o.value('1', 'PCM a-law');
		o.depends('USE_V152', '1');
		o.retain = true;

		/* T.38 */
		// T.38(Fax)
		o = s.taboption('t38', form.SectionValue, 'VOIP_PORT0', form.NamedSection, 'VOIP_PORT0', 'VOIP_PORT0', _('T.38\(Fax\)'));
		ss = o.subsection;

		o = ss.option(form.Flag, 'T38_USET38', _('T.38'));
		o.rmempty = false;

		o = ss.option(form.ListValue, 'FAX_MODEM_DET', _('Fax Modem Detection Mode'));
		o.value('0', 'AUTO');
		o.value('1', 'FAX');
		o.value('2', 'MODEM');
		o.value('3', 'AUTO_2');
		o.depends('T38_USET38', '1');
		o.retain = true;

		// T.38(Customize parameters)
		o = s.taboption('t38', form.SectionValue, 'VOIP_PORT0', form.NamedSection, 'VOIP_PORT0', 'VOIP_PORT0', _('T.38\(Customize parameters\)'));
		ss = o.subsection;

		o = ss.option(form.Flag, 'T38_PARAM_ENABLE', _('Customize parameters'));
		o.rmempty = false;

		o = ss.option(form.Value, 'T38_MAX_BUFFER', _('Max Buffer'));
		o.depends('T38_PARAM_ENABLE', '1');
		o.datatype = 'uinteger';
		o.rmempty = false;
		o.retain = true;

		o = ss.option(form.ListValue, 'T38_RATE_MGT', _('TCF'));
		o.value('0', 'Local TCF');
		o.value('1', 'Remote TCF');
		o.depends('T38_PARAM_ENABLE', '1');
		o.retain = true;

		o = ss.option(form.ListValue, 'T38_MAX_RATE', _('Max Rate'));
		o.value('0', '2400');
		o.value('1', '4800');
		o.value('2', '7200');
		o.value('3', '9600');
		o.value('4', '12000');
		o.value('5', '14400');
		o.depends('T38_PARAM_ENABLE', '1');
		o.retain = true;

		o = ss.option(form.Flag, 'T38_ENABLE_ECM', _('ECM'));
		o.depends('T38_PARAM_ENABLE', '1');
		o.rmempty = false;
		o.retain = true;

		o = ss.option(form.ListValue, 'T38_ECC_SIGNAL', _('ECC Signal'));
		o.value('0'); o.value('1'); o.value('2'); o.value('3'); o.value('4'); o.value('5'); o.value('6'); o.value('7');
		o.depends('T38_ENABLE_ECM', '1');
		o.retain = true;

		o = ss.option(form.ListValue, 'T38_ECC_DATA', _('ECC Data'));
		o.value('0'); o.value('1'); o.value('2');
		o.depends('T38_ENABLE_ECM', '1');
		o.retain = true;

		o = ss.option(form.Flag, 'T38_ENABLE_SPOOF', _('Spoofing'));
		o.depends('T38_PARAM_ENABLE', '1');
		o.rmempty = false;
		o.retain = true;

		o = ss.option(form.ListValue, 'T38_DUPLICATE_NUM', _('Packet Duplicate Num'));
		o.value('0'); o.value('1'); o.value('2');
		o.depends('T38_PARAM_ENABLE', '1');
		o.retain = true;

		/* DSP */
		o = s.taboption('dsp', form.SectionValue, '_jitter', form.GridSection, 'voipCfgPortParam', '', _('Jitter Buffer Control'));
		o.editable = false;
		o.modalonly = false;
		ss = o.subsection;
		ss.anonymous = true;
		ss.filter = function(section_id) {
			if (section_id == 'VOIP_PORT0')
				return true;
			return false;
		}

		o = ss.option(form.ListValue, 'JITTER_DELAY', _('Min delay\(ms\)'));
		o.value('0', '20'); o.value('1', '30'); o.value('2', '40'); o.value('3', '50'); o.value('4', '60'); o.value('5', '70'); o.value('6', '80');
		o.value('7', '90'); o.value('8', '100'); o.value('9', '110'); o.value('10', '120'); o.value('11', '130'); o.value('12', '140'); o.value('13', '150');
		o.value('14', '160'); o.value('15', '170'); o.value('16', '180'); o.value('17', '190'); o.value('18', '200'); o.value('19', '210');
		o.rmempty = false;
		o.textvalue = function(section_id) {
			var val;
			val = uci.get(f_voip_uci, section_id, 'JITTER_DELAY');
			return ((parseInt(val)+1)*10 + 10);
		}

		o = ss.option(form.ListValue, 'MAX_DELAY', _('Max delay\(ms\)'));
		o.value('0', '130'); o.value('1', '140'); o.value('2', '150'); o.value('3', '160'); o.value('4', '170'); o.value('5', '180'); o.value('6', '190');
		o.value('7', '200'); o.value('8', '210') ; o.value('9', '220'); o.value('10', '230'); o.value('11', '240'); o.value('12', '250'); o.value('13', '260');
		o.value('14', '270'); o.value('15', '280'); o.value('16', '290'); o.value('17', '300'); o.value('18', '310'); o.value('19', '320');
		o.rmempty = false;
		o.textvalue = function(section_id) {
			var val;
			val = uci.get(f_voip_uci, section_id, 'MAX_DELAY');
			return ((parseInt(val)+1)*10 + 120);
		}

		o = ss.option(form.Value, 'JITTER_FACTOR', _('Optimization Factor'));
		o.rmempty = false;

		o = s.taboption('dsp', form.Value, 'ECHO_TAIL', _('LEC Tail Length'), _('2~32 ms'));
		o.datatype = 'range(2, 32)';
		o.rmempty = false;
		o = s.taboption('dsp', form.Flag, 'ECHO_LEC', _('LEC'));
		o.rmempty = false;
		o = s.taboption('dsp', form.Flag, 'ECHO_NLP', _('NLP'));
		o.rmempty = false;
		o = s.taboption('dsp', form.Flag, 'VAD', _('VAD'));
		o.rmempty = false;
		o = s.taboption('dsp', form.Value, 'VAD_THRESHOLD', _('VAD Amp. Threshold'), _('0 < Amp < 200'));
		o.datatype = 'range(1, 199)';
		o.rmempty = false;
		o.retain = true;
		o.depends('VAD', '1');

		o = s.taboption('dsp', form.ListValue, 'SID_MODE', _('SID Noise'));
		o.value('0', _('Disable configuration'));
		o.value('1', _('Fixed noise leve'));
		o.value('2', _('Adjust noise leve'));

		o = s.taboption('dsp', form.Value, 'SID_LEVEL', _('Noise Level'), _('0 < Level < 127 dBov'));
		o.datatype = 'range(1, 126)';
		o.rmempty = false;
		o.retain = true;
		o.depends('SID_MODE', '1');

		o = s.taboption('dsp', form.Value, 'SID_GAIN', _('Noise Level'), _('-127 ~ 127 dBov, 0:Not Change'));
		o.datatype = 'range(-126, 126)';
		o.rmempty = false;
		o.retain = true;
		o.depends('SID_MODE', '2');

		o = s.taboption('dsp', form.Flag, 'CNG', _('CNG'));
		o.rmempty = false;
		o = s.taboption('dsp', form.Value, 'CNG_THRESHOLD', _('CNG Amp.'), _('0 < Amp < 200, 0 means no limit for Max. Amp'));
		o.datatype = 'range(0, 199)';
		o.rmempty = false;
		o.retain = true;
		o.depends('CNG', '1');

		o = s.taboption('dsp', form.Flag, 'PLC', _('PLC'));
		o.rmempty = false;

		o = s.taboption('dsp', form.Flag, 'RTCP_INTERVAL_ENABLE', _('RTCP'));
		o.rmempty = false;
		o = s.taboption('dsp', form.Value, 'RTCP_INTERVAL', _('Interval(sec.)'));
		o.datatype = 'uinteger';
		o.rmempty = false;
		o.retain = true;
		o.depends('RTCP_INTERVAL_ENABLE', '1');

		o = s.taboption('dsp', form.Flag, 'RTCPXR', _('RTCP XR'));
		o.rmempty = false;

		o = s.taboption('dsp', form.MultiValue, '_tmp1', _('Fax/Modem RFC2833 Support)'));
		o.value('1', _('Fax/Modem RFC2833 Relay(For TX)'));
		o.value('2', _('Fax/Modem Inband Removal(For TX)'));
		o.value('4', _('Fax/Modem Tone Play(For RX)'));
		o.cfgvalue = function(section_id) {
			var myarr = [], val;
			val = uci.get(f_voip_uci, section_id, 'FAX_MODEM_RFC2833')

			/* bit 0~2 for (RFC2833 Relay, Inband Removal and Tone Play) */
			for (var i = 0; i < 3; i++) {
				if (val & (1<<i))
					myarr.push((1<<i).toString());
			}
			return myarr;
		};
		o.write = function(section_id, value) {
			var val=0;

			for (var i = 0; i < value.length; i++)
				val+=parseInt(value[i]);

			uci.set(f_voip_uci, section_id, 'FAX_MODEM_RFC2833', val);
		}

		/* Speaker AGC */
		o = s.taboption('dsp', form.Flag, 'SPEAKERAGC', _('Speaker AGC'));
		o.rmempty = false;

		o = s.taboption('dsp', form.ListValue, 'SPK_AGC_LVL', _('Require level'));
		for (var i = 1; i <= 9; i++)
			o.value(i);
		o.depends('SPEAKERAGC', '1');
		o.retain = true;

		o = s.taboption('dsp', form.ListValue, 'SPK_AGC_GU', _('Max gain up(dB)'));
		for (var i = 1; i <= 9; i++)
			o.value(i);
		o.depends('SPEAKERAGC', '1');
		o.retain = true;

		o = s.taboption('dsp', form.ListValue, 'SPK_AGC_GD', _('Max gain down(dB)'));
		for (var i = -1; i >= -9; i--)
			o.value(i);
		o.depends('SPEAKERAGC', '1');
		o.retain = true;

		/* MIC AGC */
		o = s.taboption('dsp', form.Flag, 'MICAGC', _('MIC AGC'));
		o.rmempty = false;

		o = s.taboption('dsp', form.ListValue, 'MIC_AGC_LVL', _('Require level'));
		for (var i = 1; i <= 9; i++)
			o.value(i);
		o.depends('MICAGC', '1');
		o.retain = true;

		o = s.taboption('dsp', form.ListValue, 'MIC_AGC_GU', _('Max gain up(dB)'));
		for (var i = 1; i <= 9; i++)
			o.value(i);
		o.depends('MICAGC', '1');
		o.retain = true;

		o = s.taboption('dsp', form.ListValue, 'MIC_AGC_GD', _('Max gain down(dB)'));
		for (var i = -1; i >= -9; i--)
			o.value(i);
		o.depends('MICAGC', '1');
		o.retain = true;

		o = s.taboption('dsp', form.ListValue, '_cid_2_0', _('Caller ID Mode'));
		o.value('0', 'FSK_BELLCALL');
		o.value('1', 'FSK_ETSI');
		o.value('2', 'FSK_BT');
		o.value('3', 'FSK_NTT');
		o.value('4', 'DTMF');
		/* CALLER_ID_MODE bit[2-0] */
		o.cfgvalue = function(section_id) {
			return (uci.get(f_voip_uci, section_id, 'CALLER_ID_MODE') & 0x07).toString();
		}
		o.write = function(section_id, value) {
			uci.set(f_voip_uci, section_id, 'CALLER_ID_MODE', uci.get(f_voip_uci, section_id, 'CALLER_ID_MODE') & (~0x07) | value);
		}

		o = s.taboption('dsp', form.Flag, '_cid7', _('FSK Date & Time Sync'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'CALLER_ID_MODE', 7);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'CALLER_ID_MODE', 7, value);
		};

		o = s.taboption('dsp', form.Flag, '_cid6', _('Reverse Polarity before Caller ID'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'CALLER_ID_MODE', 6);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'CALLER_ID_MODE', 6, value);
		};

		o = s.taboption('dsp', form.Flag, '_cid5', _('Short Ring before Caller ID'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'CALLER_ID_MODE', 5);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'CALLER_ID_MODE', 5, value);
		};

		o = s.taboption('dsp', form.Flag, '_cid4', _('Dual Tone before Caller ID'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'CALLER_ID_MODE', 4);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'CALLER_ID_MODE', 4, value);
		};

		o = s.taboption('dsp', form.Flag, '_cid3', _('Caller ID Prior First Ring'));
		o.rmempty = false;
		o.cfgvalue = function(section_id) {
			return resolve_cfg_bit(section_id, 'CALLER_ID_MODE', 3);
		};
		o.write = function(section_id, value) {
			set_cfg_bit(section_id, 'CALLER_ID_MODE', 3, value);
		};

		o = s.taboption('dsp', form.ListValue, '_dtmf_1_0', _('Caller ID DTMF Start Digit'));
		o.value('0', 'DTMF_A');
		o.value('1', 'DTMF_B');
		o.value('2', 'DTMF_C');
		o.value('3', 'DTMF_D');
		o.cfgvalue = function(section_id) {
			return (uci.get(f_voip_uci, section_id, 'CID_DTMF_MODE') & 0x03).toString();
		}
		o.write = function(section_id, value) {
			uci.set(f_voip_uci, section_id, 'CID_DTMF_MODE', uci.get(f_voip_uci, section_id, 'CID_DTMF_MODE') & (~0x03) | value);
		}

		o = s.taboption('dsp', form.ListValue, '_dtmf_3_2', _('Caller ID DTMF End Digit'));
		o.value('0', 'DTMF_A');
		o.value('1', 'DTMF_B');
		o.value('2', 'DTMF_C');
		o.value('3', 'DTMF_D');
		o.cfgvalue = function(section_id) {
			return (uci.get(f_voip_uci, section_id, 'CID_DTMF_MODE') >> 2).toString();
		}
		o.write = function(section_id, value) {
			uci.set(f_voip_uci, section_id, 'CID_DTMF_MODE', uci.get(f_voip_uci, section_id, 'CID_DTMF_MODE') & (~0x0c) | (value << 2));
		}

		o = s.taboption('dsp', form.Value, 'FLASH_HOOK_TIME_MIN', _('Min Flash Time'), _('Space:10, Min:80, Max:2000'));
		o.datatype = 'range(80, 2000)';
		o.rmempty = false;
		o = s.taboption('dsp', form.Value, 'FLASH_HOOK_TIME', _('Max Flash Time'), _('Space:10, Min:80, Max:2000'));
		o.datatype = 'range(80, 2000)';
		o.rmempty = false;

		o = s.taboption('dsp', form.Value, 'SPK_VOICE_GAIN', _('Speaker Voice Gain (dB)'), _('[ -32~31 ],Mute:-32'));
		o.datatype = 'range(-32, 31)';
		o.rmempty = false;

		o = s.taboption('dsp', form.Value, 'MIC_VOICE_GAIN', _('Mic Voice Gain (dB)'), _('[ -32~31 ],Mute:-32'));
		o.datatype = 'range(-32, 31)';
		o.rmempty = false;

		return m.render();
	},

	handleSave: function(ev) {
		var tasks = [];

		document.getElementById('maincontent').querySelectorAll('.cbi-map').forEach(function(map) {
			tasks.push(DOM.callClassMethod(map, 'save', cloneSection));
		});

		return Promise.all(tasks);
	}
});
