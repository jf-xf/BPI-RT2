'use strict';
'require dom';
'require view';
'require ui';
'require uci';
'require form';

function get_next_free_sid(section, len) {
	var sid = section + '_' + (++len);

	while (uci.get('obuspa', sid)) {
		sid = section + '_' + (++len);
	}

	return sid;
}

return view.extend({
	load: function () {
		return Promise.all([
			uci.load('obuspa'),
		]);
	},

	render: function (data) {
		var m, s, o;

		m = new form.Map('obuspa', _('TR-369 Settings'), _('This page is used to configure the TR-369 parameters.'));

		s = m.section(form.NamedSection, 'global');
		s.anonymous = true;

		o = s.option(form.Flag, 'enabled', _('Enable TR-369'));
		o.rmempty = false;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		/* Device.LocalAgent.MTP.{i}. */
		s = m.section(form.GridSection, 'mtp', _('LocalAgent MTP'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add');
		s.sectiontitle = function (section_id) {
			return section_id;
		};
		s.handleAdd = function (ev) {
			var section_id = get_next_free_sid('mtp', uci.sections('obuspa', 'mtp').length);

			uci.add('obuspa', 'mtp', section_id);
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		/* Device.LocalAgent.MTP.{i}.Enable */
		o = s.option(form.Flag, 'Enable', _('Enable'));
		o.rmempty = false;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		/* Device.LocalAgent.MTP.{i}.Protocol */
		o = s.option(form.ListValue, 'Protocol', _('Protocol'));
		o.rmempty = false;
		o.value('MQTT', 'MQTT');
		o.value('WebSocket', 'WebSocket');
		o.value('STOMP', 'STOMP');

		/* MQTT */
		/* Device.LocalAgent.MTP.{i}.MQTT.Reference */
		o = s.option(form.ListValue, 'mqtt', _('Reference'));
		o.depends('Protocol', 'MQTT');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		var mqtts = uci.sections('obuspa', 'mqtt');
		for (var i = 0; i < mqtts.length; i++) {
			o.value(mqtts[i]['.name'], mqtts[i]['.name']);
		}

		/* Device.LocalAgent.MTP.{i}.MQTT.ResponseTopicConfigured */
		o = s.option(form.Value, 'ResponseTopicConfigured', _('Response Topic'));
		o.depends('Protocol', 'MQTT');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* WebSocket */
		/* Device.LocalAgent.MTP.{i}.WebSocket.Port */
		o = s.option(form.Value, 'Port', _('Port'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		o.datatype = 'uinteger';
		o.default = '5683';

		/* Device.LocalAgent.MTP.{i}.WebSocket.Path */
		o = s.option(form.Value, 'Path', _('Path'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* Device.LocalAgent.MTP.{i}.WebSocket.EnableEncryption */
		o = s.option(form.Flag, 'EnableEncryption', _('Enable Encryption'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* STOMP */
		/* Device.LocalAgent.MTP.{i}.STOMP.Reference */
		o = s.option(form.ListValue, 'stomp', _('Reference'));
		o.depends('Protocol', 'STOMP');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		var stomps = uci.sections('obuspa', 'stomp');
		for (var i = 0; i < stomps.length; i++) {
			o.value(stomps[i]['.name'], stomps[i]['.name']);
		}

		/* Device.LocalAgent.MTP.{i}.STOMP.Destination */
		o = s.option(form.Value, 'Destination', _('Destination'));
		o.depends('Protocol', 'STOMP');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* Device.LocalAgent.Controller.{i}. */
		s = m.section(form.GridSection, 'controller', _('LocalAgent Controller'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add');
		s.sectiontitle = function (section_id) {
			return section_id;
		};
		s.handleAdd = function (ev) {
			var section_id = get_next_free_sid('controller', uci.sections('obuspa', 'controller').length);

			uci.add('obuspa', 'controller', section_id);
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		/* Device.LocalAgent.Controller.{i}.Enable */
		o = s.option(form.Flag, 'Enable', _('Enable'));
		o.rmempty = false;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		/* Device.LocalAgent.Controller.{i}.EndpointID */
		o = s.option(form.Value, 'EndpointID', _('Endpoint ID'));
		o.rmempty = false;
		o.optional = true;

		/* Device.LocalAgent.Controller.{i}.MTP.1.Protocol */
		o = s.option(form.ListValue, 'Protocol', _('Protocol'));
		o.rmempty = false;
		o.value('MQTT', 'MQTT');
		o.value('WebSocket', 'WebSocket');
		o.value('STOMP', 'STOMP');

		/* MQTT */
		/* Device.LocalAgent.Controller.{i}.MTP.1.MQTT.Reference */
		o = s.option(form.ListValue, 'mqtt', _('Reference'));
		o.depends('Protocol', 'MQTT');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		var mqtts = uci.sections('obuspa', 'mqtt');
		for (var i = 0; i < mqtts.length; i++) {
			o.value(mqtts[i]['.name'], mqtts[i]['.name']);
		}

		/* Device.LocalAgent.Controller.{i}.MTP.1.MQTT.Topic */
		o = s.option(form.Value, 'Topic', _('Topic'));
		o.depends('Protocol', 'MQTT');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* WebSocket */
		/* Device.LocalAgent.Controller.{i}.MTP.1.WebSocket.Host */
		o = s.option(form.Value, 'Host', _('Host'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* Device.LocalAgent.Controller.{i}.MTP.1.WebSocket.Port */
		o = s.option(form.Value, 'Port', _('Port'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		o.datatype = 'uinteger';
		o.default = '5683';

		/* Device.LocalAgent.Controller.{i}.MTP.1.WebSocket.Path */
		o = s.option(form.Value, 'Path', _('Path'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		o.default = '/';

		/* Device.LocalAgent.Controller.{i}.MTP.1.WebSocket.EnableEncryption */
		o = s.option(form.Flag, 'EnableEncryption', _('Enable Encryption'));
		o.depends('Protocol', 'WebSocket');
		o.rmempty = false;
		o.modalonly = true;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		/* STOMP */
		/* Device.LocalAgent.Controller.{i}.MTP.1.STOMP.Reference */
		o = s.option(form.ListValue, 'stomp', _('Reference'));
		o.depends('Protocol', 'STOMP');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		var stomps = uci.sections('obuspa', 'stomp');
		for (var i = 0; i < stomps.length; i++) {
			o.value(stomps[i]['.name'], stomps[i]['.name']);
		}

		/* Device.LocalAgent.Controller.{i}.MTP.1.STOMP.Destination */
		o = s.option(form.Value, 'Destination', _('Destination'));
		o.depends('Protocol', 'STOMP');
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* Device.MQTT.Client. */
		s = m.section(form.GridSection, 'mqtt', _('MQTT Client'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add');
		s.sectiontitle = function (section_id) {
			return section_id;
		};
		s.handleAdd = function (ev) {
			var section_id = get_next_free_sid('mqtt', uci.sections('obuspa', 'mqtt').length);

			uci.add('obuspa', 'mqtt', section_id);
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		o = s.option(form.Flag, 'Enable', _('Enable'));
		o.rmempty = false;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		o = s.option(form.Value, 'BrokerAddress', _('Broker Address'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.Value, 'BrokerPort', _('Broker Port'));
		o.rmempty = false;
		o.optional = true;
		o.datatype = 'uinteger';
		o.default = '1883';

		o = s.option(form.Value, 'Username', _('Username'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.Value, 'Password', _('Password'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.ListValue, 'ProtocolVersion', _('Protocol Version'));
		o.rmempty = false;
		o.modalonly = true;
		o.value('3.1', '3.1');
		o.value('3.1.1', '3.1.1');
		o.value('5.0', '5.0');
		o.default = '5.0';

		o = s.option(form.ListValue, 'TransportProtocol', _('Transport Protocol'));
		o.rmempty = false;
		o.modalonly = true;
		o.value('TCP/IP', 'TCP/IP');
		o.value('TLS', 'TLS');
		o.default = 'TCP/IP';

		o = s.option(form.Value, 'ClientID', _('Client ID'));
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;

		/* Device.STOMP.Connection. */
		s = m.section(form.GridSection, 'stomp', _('STOMP Connection'));
		s.anonymous = true;
		s.addremove = true;
		s.addbtntitle = _('Add');
		s.sectiontitle = function (section_id) {
			return section_id;
		};
		s.handleAdd = function (ev) {
			var section_id = get_next_free_sid('stomp', uci.sections('obuspa', 'stomp').length);

			uci.add('obuspa', 'stomp', section_id);
			m.addedSection = section_id;

			return this.renderMoreOptionsModal(section_id);
		};

		o = s.option(form.Flag, 'Enable', _('Enable'));
		o.rmempty = false;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		o = s.option(form.Value, 'Host', _('Host'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.Value, 'Port', _('Port'));
		o.rmempty = false;
		o.optional = true;
		o.datatype = 'uinteger';
		o.default = '61613';

		o = s.option(form.Value, 'Username', _('Username'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.Value, 'Password', _('Password'));
		o.rmempty = false;
		o.optional = true;

		o = s.option(form.Flag, 'EnableEncryption', _('Enable Encryption'));
		o.rmempty = false;
		o.modalonly = true;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		o = s.option(form.Flag, 'EnableHeartbeats', _('Enable Heartbeats'));
		o.rmempty = false;
		o.modalonly = true;
		o.enabled = '1';
		o.disabled = '0';
		o.default = '1';

		o = s.option(form.Value, 'VirtualHost', _('Virtual Host'));
		o.rmempty = false;
		o.modalonly = true;
		o.optional = true;
		o.default = '/';

		return m.render();
	}
});
