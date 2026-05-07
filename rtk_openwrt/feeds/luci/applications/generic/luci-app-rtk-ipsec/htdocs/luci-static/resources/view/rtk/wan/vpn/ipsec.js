'use strict';
'require view';
'require uci';
'require rpc';
'require form';
'require fs';
'require ui';

return view.extend({
	handleRemoteCert: function(ev) {
		return ui.uploadFile('/etc/swanctl/x509ca/remote.cacert.pem', ev.target)
			.then(L.bind(function(btn, res) {
				ui.showModal(_('Apply Remote cert?'), [
					E('p', _('The uploaded Remote cert archive contains the files listed below. Press "Continue" to restore the cert, or "Cancel" to abort the operation.')),
					E('pre', {}, [ res.stdout ]),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.createHandlerFn(this, function(ev) {
								return fs.remove('/etc/swanctl/x509ca/remote.cacert.pem').finally(ui.hideModal);
							})
						}, [ _('Cancel') ]), ' ',
						E('button', {
							'class': 'btn cbi-button-action important',
							'click': ui.createHandlerFn(this, function(ev) {
								return ui.hideModal();
							})
						}, [ _('Continue') ])
					])
				]);
			}, this, ev.target))
			.catch(function(e) { ui.addNotification(null, E('p', e.message)) })
			.finally(L.bind(function(btn, input) {
				btn.firstChild.data = _('Upload archive...');
			}, this, ev.target));
	},

	handleCert: function(ev) {
		return ui.uploadFile('/etc/swanctl/x509/local.cacert.pem', ev.target)
			.then(L.bind(function(btn, res) {
				ui.showModal(_('Apply cert?'), [
					E('p', _('The uploaded Local cert archive contains the files listed below. Press "Continue" to restore the cert, or "Cancel" to abort the operation.')),
					E('pre', {}, [ res.stdout ]),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.createHandlerFn(this, function(ev) {
								return fs.remove('/etc/swanctl/x509/local.cacert.pem').finally(ui.hideModal);
							})
						}, [ _('Cancel') ]), ' ',
						E('button', {
							'class': 'btn cbi-button-action important',
							'click': ui.createHandlerFn(this, function(ev) {
								return ui.hideModal();
							})
						}, [ _('Continue') ])
					])
				]);
			}, this, ev.target))
			.catch(function(e) { ui.addNotification(null, E('p', e.message)) })
			.finally(L.bind(function(btn, input) {
				btn.firstChild.data = _('Upload archive...');
			}, this, ev.target));
	},

	handleKey: function(ev) {
		return ui.uploadFile('/etc/swanctl/private/local.priv.pem', ev.target)
			.then(L.bind(function(btn, res) {
				ui.showModal(_('Apply Key?'), [
					E('p', _('The uploaded Local Key archive contains the files listed below. Press "Continue" to restore the cert, or "Cancel" to abort the operation.')),
					E('pre', {}, [ res.stdout ]),
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.createHandlerFn(this, function(ev) {
								return fs.remove('/etc/swanctl/private/local.priv.pem').finally(ui.hideModal);
							})
						}, [ _('Cancel') ]), ' ',
						E('button', {
							'class': 'btn cbi-button-action important',
							'click': ui.createHandlerFn(this, function(ev) {
								return ui.hideModal();
							})
						}, [ _('Continue') ])
					])
				]);
			}, this, ev.target))
			.catch(function(e) { ui.addNotification(null, E('p', e.message)) })
			.finally(L.bind(function(btn, input) {
				btn.firstChild.data = _('Upload archive...');
			}, this, ev.target));
	},

	render: function(hosts) {
		var m, s, o;
		var default_mode = [
				'tun'
			];

		m = new form.Map('ipsec', _('IPSec Configuration'),
			_('This page is used to configure the IPSec.'));

		s = m.section(form.TypedSection, 'remote', _('Tunnel definition'));
		s.anonymous = true;

		o = s.option(form.Flag, 'enabled', _('Enable'));

		o = s.option(form.ListValue, 'mode', _('Mode'));
		o.value('transport', _('Transport'));
		o.value('tunnel', _('Tunnel'));
		o.write = function(section_id, value) {
			if (value == 'transport') {
				uci.set('ipsec', section_id, 'mode', 'transport');
				uci.unset('ipsec', section_id, 'tunnel');
				uci.set('ipsec', 'acme', 'transport', default_mode);
			}
			else {
				uci.set('ipsec', section_id, 'mode', 'tunnel');
				uci.unset('ipsec', section_id, 'transport');
				uci.set('ipsec', 'acme', 'tunnel', default_mode);
			}
		};

		o = s.option(form.ListValue, 'keyexchange', _('IKE Version'));
		o.value('ike', _('ALL'));
		o.value('ikev1', _('1'));
		o.value('ikev2', _('2'));

		o = s.option(form.ListValue, 'authentication_method', _('IKE Authentication Method'));
		o.value('psk', _('PreShareKey'));
		o.value('pubkey', _('Pubkey'));

		o = s.option(form.Value, 'pre_shared_key',  _('Preshared Key'), _('The preshared key for the tunnel if authentication is psk'));
		o.depends("authentication_method", "psk");

		o = s.option(form.Value, 'local_ip', _('Local IP Address'), _('Local address to use in IKE negotiation when initiating'));
		o.datatype = 'list(neg(ipmask("true")))';
		o.placeholder = _('any');

		o = s.option(form.Value, 'gateway',  _('Remote IP Address'), _('IP address of the tunnel remote endpoint'));
		o.datatype = 'list(neg(ipmask("true")))';
		o.placeholder = _('any');

		o = s.option(form.Button, 'localcert', _('Local cert'), _('cert pathnames to use for authentication'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Upload archive...');
		o.onclick = this.handleCert;

		o = s.option(form.Button, 'localkey', _('Local key'), _('private key pathnames to use with above certificates'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Upload archive...');
		o.onclick = this.handleKey;

		o = s.option(form.Button, 'remotecert', _('Remote cert'), _('CA certificates that need to lie in remote'));
		o.inputstyle = 'action important';
		o.inputtitle = _('Upload archive...');
		o.onclick = this.handleRemoteCert;

		s = m.section(form.NamedSection, 'ike_proposal', '', _('IKE Proposal(Phase 1)'));
		o = s.option(form.ListValue, 'encryption_algorithm', _('Encryption Method'));
		o.value('des', _('des'));
		o.value('3des', _('3des'));
		o.value('aes128', _('aes128'));
		o.value('aes192', _('aes192'));
		o.value('aes256', _('aes256'));

		o = s.option(form.ListValue, 'hash_algorithm', _('Hash Algorithm'));
		o.value('md5', _('MD5'));
		o.value('sha1', _('SHA1'));

		// dh-group—Diffie-Hellman group for key establishment.
		// group1—768-bit Modular Exponential (MODP) algorithm.
		// group2—1024-bit MODP algorithm.
		// group5—1536-bit MODP algorithm.
		// group14—2048-bit MODP group.
		o = s.option(form.ListValue, 'dh_group', _('Diffie-Hellman Exponentiation'));
		o.value('modp768', _('GROUP1'));
		o.value('modp1024', _('GROUP2'));
		o.value('modp1536', _('GROUP5'));
		o.value('modp2048', _('GROUP14'));

		s = m.section(form.TypedSection, 'tunnel', _('Network Defintion'));
		s.anonymous = true;

		o = s.option(form.DynamicList, 'local_subnet', _('Local Network'));
		o.datatype = 'list(neg(ipmask("true")))';
		o.placeholder = _('any');

		o = s.option(form.DynamicList, 'remote_subnet', _('Remote Network'));
		o.datatype = 'list(neg(ipmask("true")))';
		o.placeholder = _('any');

		o = s.option(form.Value, 'rekeytime',  _('Rekeytime'), _('Duration of the CHILD_SA before rekeying'));
		o = s.option(form.Value, 'rekeybytes',  _('Rekeybytes'), _('Traffics of the CHILD_SA before rekeying'));

		s = m.section(form.NamedSection, 'esp_proposal', '', _('ESP Proposal(Phase 2)'));
		o = s.option(form.ListValue, 'encryption_algorithm', _('Encryption Method'));
		o.value('des', _('des'));
		o.value('3des', _('3des'));
		o.value('aes128', _('aes128'));
		o.value('aes192', _('aes192'));
		o.value('aes256', _('aes256'));

		o = s.option(form.ListValue, 'hash_algorithm', _('Hash Algorithm'));
		o.value('md5', _('MD5'));
		o.value('sha1', _('SHA1'));

		// dh-group—Diffie-Hellman group for key establishment.
		// group1—768-bit Modular Exponential (MODP) algorithm.
		// group2—1024-bit MODP algorithm.
		// group5—1536-bit MODP algorithm.
		// group14—2048-bit MODP group.
		o = s.option(form.ListValue, 'dh_group', _('Diffie-Hellman Exponentiation'));
		o.value('modp768', _('GROUP1'));
		o.value('modp1024', _('GROUP2'));
		o.value('modp1536', _('GROUP5'));
		o.value('modp2048', _('GROUP14'));

		return m.render();
	}
});
