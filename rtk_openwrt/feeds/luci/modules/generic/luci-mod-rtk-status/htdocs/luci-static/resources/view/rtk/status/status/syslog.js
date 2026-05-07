'use strict';
'require view';
'require fs';
'require ui';
'require uci';

return view.extend({
	load: function() {
		return Promise.all([
			L.resolveDefault(fs.read('/www/system.log'), ''),
			uci.load('system')
		]);
	},

	render: function(logdata) {
		var loglines = logdata[0].trim().split(/\n/);
		var secSystem = uci.sections('system', 'system');
		var firstLine = 0;
		console.log(secSystem);

		// NOTICE, INFO, DEBUG, WARNING, ERR, CRIT, ALERT, EMERG .
		for (var i = 0; i < loglines.length; i++) {
			if (secSystem[0]['displaylevel'] == '7') {	// INFO
				if (loglines[i].match('debug')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '6') {	// NOTICE
				if (loglines[i].match('debug') || loglines[i].match('info') ) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '5') {	// WARN
				if (loglines[i].match('debug') || loglines[i].match('info') || loglines[i].match('notice')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '4') {	// ERR
				if (loglines[i].match('debug') || loglines[i].match('info') || loglines[i].match('notice') || loglines[i].match('warn')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '3') {	//CRIT
				if (loglines[i].match('debug') || loglines[i].match('info') || loglines[i].match('notice') || loglines[i].match('warn') || loglines[i].match('err')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '2') {	// ALERT
				if (loglines[i].match('debug') || loglines[i].match('info') || loglines[i].match('notice') || loglines[i].match('warn') || loglines[i].match('err') || loglines[i].match('crit')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
			else if (secSystem[0]['displaylevel'] == '1') {	// EMERG
				if (loglines[i].match('debug') || loglines[i].match('info') || loglines[i].match('notice') || loglines[i].match('warn') || loglines[i].match('err') || loglines[i].match('crit') || loglines[i].match('alert')) {
					if (i == 0 )
						firstLine = 1;
					else
						loglines.splice(i,1);
					i = 0;
				}
			}
		}
		// If The first line is matched, remove this line.
		if (firstLine == 1)
			loglines.splice(0,1);

		return E([], [
			E('h2', {}, [ _('System Log') ]),
			E('div', { 'id': 'content_syslog' }, [
				E('textarea', {
					'id': 'syslog',
					'style': 'font-size:12px',
					'readonly': 'readonly',
					'wrap': 'off',
					'rows': loglines ==null? 1: loglines.length + 1
				}, [ [('Date       Time          Facility.Level Message')], ['\n\n'], loglines ==null? '\n': loglines.join('\n') ])
			])
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
