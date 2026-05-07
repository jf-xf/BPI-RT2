'use strict';
'require dom';
'require view';
'require ui';
'require uci';
'require form';
'require rpc';
'require fs';

return view.extend({
	load: function () {
		return Promise.all([
			uci.load('cwmp'),
		]);
	},

	handle_cert_upload: function (ev) {
		return ui.uploadFile('/tmp/acs-ca-cert.pem', ev.target)
			.then(L.bind(function (btn, res) {
				ui.showModal(_('Uptate CA Certificate?'), [
					E('p', _('The CA file was uploaded. Press "Continue" to uptate, or "Cancel" to abort the operation.')),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.createHandlerFn(this, function (ev) {
								return fs.remove('/tmp/acs-ca-cert.pem').finally(ui.hideModal);
							})
						}, [_('Cancel')]), ' ',
						E('button', {
							'class': 'btn cbi-button-action important',
							'click': ui.createHandlerFn(this, 'handle_cert_uptate', btn)
						}, [_('Continue')])
					])
				]);
			}, this, ev.target))
			.finally(L.bind(function (btn, input) {
				btn.firstChild.data = _('Upload CA file...');
			}, this, ev.target));
	},

	handle_cert_uptate: function (btn, ev) {
		return fs.exec('/bin/mkdir', ['-p', '/etc/icwmpd/certs'])
			.then(L.bind(function (btn, res) {
				if (res.code != 0) {
					ui.addNotification(null, E('p', _('The certs directory is not exist.')));
					return fs.remove('/tmp/acs-ca-cert.pem');
				}
				return fs.exec('/bin/cp', ['-f', '/tmp/acs-ca-cert.pem', '/etc/icwmpd/certs/acs-ca-cert.pem']);
			}, this, ev.target))
			.catch(function (e) { ui.addNotification(null, E('p', e.message)) })
			.finally(ui.hideModal);
	},

	render: function (data) {
		var m, s, o;

		m = new form.Map('cwmp', _('TR-069 Settings'), _('This page is used to configure the TR-069 parameters.'));

		s = m.section(form.NamedSection, 'cpe');
		s.anonymous = true;

		o = s.option(form.Flag, 'enable', _('Enable TR-069'));
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';
		o.rmempty = false;

		s = m.section(form.NamedSection, 'acs', '', _('ACS'));
		s.anonymous = true;

		o = s.option(form.Value, 'url', _('URL:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Value, 'userid', _('Username:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Value, 'passwd', _('Password:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Flag, 'periodic_inform_enable', _('Periodic Inform:'));
		o.enabled = 'true';
		o.disabled = 'false';
		o.default = 'true';
		o.rmempty = false;

		o = s.option(form.Value, 'periodic_inform_interval', _('Periodic Inform Interval:'));
		o.datatype = 'uinteger';
		o.default = '0';
		o.rmempty = false;

		s = m.section(form.NamedSection, 'cpe', '', _('Connection Request'));
		s.anonymous = true;

		o = s.option(form.Value, 'userid', _('Username:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Value, 'passwd', _('Password:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Value, 'path', _('Path:'));
		o.datatype = 'string';
		o.optional = true;
		o.rmempty = false;

		o = s.option(form.Value, 'port', _('Port:'));
		o.datatype = 'uinteger';
		o.default = '7547';
		o.rmempty = false;

		s = m.section(form.NamedSection, 'acs', '', _('Certificate Management'));
		s.anonymous = true;
		o = s.option(form.Button, 'cert_upload', _('CA Certificate'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Upload CA file...');
		o.onclick = L.bind(this.handle_cert_upload, this);

		return m.render();
	}
});
