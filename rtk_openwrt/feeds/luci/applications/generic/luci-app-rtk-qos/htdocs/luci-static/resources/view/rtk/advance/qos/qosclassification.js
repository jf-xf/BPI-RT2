'use strict';
'require view';
'require ui';
'require uci';
'require form';
'require network';
'require tools.widgets as widgets';
'require tools.rtk.firewall as fwtool';
'require rtk.network as rtkNetwork';
function validateNull(sid, s) {
    if (s == null || s == '')
            return _("This item should NOT be NULL");

    return true;
}

function rule_type_txt(s) {
	var t = uci.get('qos', s, 'rule_type')
	switch(t) {
		case '1':
			s = {
				target: t,
				port: uci.get('qos', s, 'lan_interface')
			}
			return fwtool.fmt(_('Port- %{port}'), s);
		case '2':
			s = {
				target: t,
				ethertype: uci.get('qos', s, 'ether_type')
			}
			return fwtool.fmt(_('EtherType- 0x%{ethertype}'), s);
		case '3':
			s = {
				target: t,
                                ip_version: uci.get('qos', s, 'ip_version'),
				ipv4: (uci.get('qos', s, 'ip_version') == '1') ? 1 : null,
				ipv6: (uci.get('qos', s, 'ip_version') == '2') ? 1 : null,
				ipv4v6: (uci.get('qos', s, 'ip_version') == '3') ? 1 : null,
				display_version: (uci.get('qos', s, 'ip_version') != '0') ? 1 : null,
                                proto: uci.get('qos', s, 'proto'),
				display_proto: (uci.get('qos', s, 'proto') == 'na') ? null : 1,
                                srchost: uci.get('qos', s, 'srchost'),
                                dsthost: uci.get('qos', s, 'dsthost'),
                                srcports: uci.get('qos', s, 'srcports'),
                                dstports: uci.get('qos', s, 'dstports'),

			}
			return fwtool.fmt(_('IP/Protocol: <br> %{display_version?ip version-%{ipv4?IPv4} %{ipv6?IPv6} %{ipv4v6?IPv4v6} <br>} %{display_proto?proto-%{proto} <br>} %{srchost?source ipr-%{srchost} <br>} %{dsthost?destination ip-%{dsthost} <br>} %{srcports?source port-%{srcports} <br>} %{dstports?destination port-%{dstports}}'), s);
		case '4':
			s = {
				target: t,
				src_mac: uci.get('qos', s, 'src_mac'),
				dst_mac: uci.get('qos', s, 'dst_mac'),
			}
			return fwtool.fmt(_('Mac Address: <br> source- %{src_mac} <br> destination- %{dst_mac}'), s);
		case '5':
			s = {
				target: t,
				wan_interface: uci.get('qos', s, 'wan_interface')
			}
			return fwtool.fmt(_('WAN Interface- %{wan_interface}'), s);
		default :
			return ' ';
	}
}

function rule_DSCP_txt(s) {
	var t = uci.get('qos', s, 'DSCP')
	switch(t) {
		case 'na':
			return ' ';
			break;
		case '0':
			return 'default(000000)';
			break;
		case '8':
			return 'CS1(001000)';
			break;
		case '10':
			return 'AF11(001010)';
			break;
		case '12':
			return 'AF12(001100)';
			break;
		case '14':
			return 'AF13(001110)';
			break;
		case '16':
			return 'CS2(010000)';
			break;
		case '18':
			return 'AF21(010010)';
			break;
		case '20':
			return 'AF22(010100)';
			break;
		case '22':
			return 'AF23(010110)';
			break;
		case '24':
			return 'CS3(011000)';
			break;
		case '26':
			return 'AF31(011010)';
			break;
		case '28':
			return 'AF32(011100)';
			break;
		case '30':
			return 'AF33(011110)';
			break;
		case '32':
			return 'CS4(100000)';
			break;
		case '34':
			return 'AF41(100010)';
			break;
		case '36':
			return 'AF42(100100)';
			break;
		case '38':
			return 'AF43(100110)';
			break;
		case '40':
			return 'CS5(101000)';
			break;
		case '46':
			return 'EF(101110)';
			break;
		default :
			return ' ';
	}
}

return view.extend({

	load: function() {
		return Promise.all([
			uci.load('qos')
		]);
	},

	load: function() {
		return network.getDevices();
	},

        getIfname: function() {
                return this._ubus('l3_device') || 'veip-%s'.format(this.sid);
        },

        getDevices: function() {
                return null;
        },

        containsDevice: function(ifname) {
                return (network.getIfnameOf(ifname) == this.getIfname());
        },


	render: function(data) {
		var m, s, o, ss;
		m = new form.Map('qos', _('QoS Classification'), _('This page is used to add or delete classicification rule.'));

		s = m.section(form.GridSection, 'classify', _('Classification'), );
		s.addremove = true;
		s.anonymous = true;
		//s.sortable  = true;
		s.addbtntitle = _('Add');

		o = s.option(form.Value, 'comment', _('Name'));
		//o.optional = true;
		o.modalonly = false;
		o.datatype = 'maxlength(16)';

		o = s.option(form.ListValue, 'target', _('Queue'));
		//.optional = true;
		o.validate = validateNull;
		o.modalonly = false;
		o.default = 'Q8';
		o.value('Q1', _('Q1'));
		o.value('Q2', _('Q2'));
		o.value('Q3', _('Q3'));
		o.value('Q4', _('Q4'));
		o.value('Q5', _('Q5'));
		o.value('Q6', _('Q6'));
		o.value('Q7', _('Q7'));
		o.value('Q8', _('Q8'));
		o = s.option(form.ListValue, 'DSCP', _('DSCP mark'));
		//o.optional = true;
		o.modalonly = false;
		o.textvalue = function(s) {
			return rule_DSCP_txt(s);
		};
		o.default = 'na';
		o.value('na', _(' '));
		o.value('0', _('default(000000)'));
		o.value('8', _('CS1(001000)'));
		o.value('10', _('AF11(001010)'));
		o.value('12', _('AF12(001100)'));
		o.value('14', _('AF13(001110)'));
		o.value('16', _('CS2(010000)'));
		o.value('18', _('AF21(010010)'));
		o.value('20', _('AF22(010100)'));
		o.value('22', _('AF23(010110)'));
		o.value('24', _('CS3(011000)'));
		o.value('26', _('AF31(011010)'));
		o.value('28', _('AF32(011100)'));
		o.value('30', _('AF33(011110)'));
		o.value('32', _('CS4(100000)'));
		o.value('34', _('AF41(100010)'));
		o.value('36', _('AF42(100100)'));
		o.value('38', _('AF43(100110)'));
		o.value('40', _('CS5(101000)'));
		o.value('46', _('EF(101110)'));
		
		o = s.option(form.ListValue, '8021p', _('802.1p'));
		o.modalonly = false;
		o.default = 'na';
		o.value('na', _(' '));
		o.value('0', _('0'));
		o.value('1', _('1'));
		o.value('2', _('2'));
		o.value('3', _('3'));
		o.value('4', _('4'));
		o.value('5', _('5'));
		o.value('6', _('6'));
		o.value('7', _('7'));
	
		o = s.option(form.Value, 'wan_interface', _('WanIf'));
		o.modalonly = false;

		o = s.option(form.DummyValue, 'rule_type', _('Rule Detail'));
		o.modalonly = false;
		//1.portbase 2.etherType 3.IP/PORT 4.MAC address 5.WANinterface
	        o.textvalue = function(s) {
            	    return E('small', [
				rule_type_txt(s), E('br')
                	]);
        	};


		o = s.option(form.Value, 'comment', _('Name'));
		o.modalonly = true;
		o.datatype = 'maxlength(16)';

		o = s.option(form.ListValue, 'target', _('Queue'));
		o.validate = validateNull;
		o.modalonly = true;
		o.default = 'Q8';
		o.value('Q1', _('Q1'));
		o.value('Q2', _('Q2'));
		o.value('Q3', _('Q3'));
		o.value('Q4', _('Q4'));
		o.value('Q5', _('Q5'));
		o.value('Q6', _('Q6'));
		o.value('Q7', _('Q7'));
		o.value('Q8', _('Q8'));

		o = s.option(form.ListValue, 'DSCP', _('DSCP mark'));
		o.modalonly = true;
		o.textvalue = function(s) {
			return rule_DSCP_txt(s);
		};
		o.default = 'na';
		o.value('na', _(' '));
		o.value('0', _('default(000000)'));
		o.value('8', _('CS1(001000)'));
		o.value('10', _('AF11(001010)'));
		o.value('12', _('AF12(001100)'));
		o.value('14', _('AF13(001110)'));
		o.value('16', _('CS2(010000)'));
		o.value('18', _('AF21(010010)'));
		o.value('20', _('AF22(010100)'));
		o.value('22', _('AF23(010110)'));
		o.value('24', _('CS3(011000)'));
		o.value('26', _('AF31(011010)'));
		o.value('28', _('AF32(011100)'));
		o.value('30', _('AF33(011110)'));
		o.value('32', _('CS4(100000)'));
		o.value('34', _('AF41(100010)'));
		o.value('36', _('AF42(100100)'));
		o.value('38', _('AF43(100110)'));
		o.value('40', _('CS5(101000)'));
		o.value('46', _('EF(101110)'));
		
		o = s.option(form.ListValue, '8021p', _('802.1p'));
		o.modalonly = true;
		o.default = 'na';
		o.value('na', _(' '));
		o.value('0', _('0'));
		o.value('1', _('1'));
		o.value('2', _('2'));
		o.value('3', _('3'));
		o.value('4', _('4'));
		o.value('5', _('5'));
		o.value('6', _('6'));
		o.value('7', _('7'));
	

		o = s.option(form.ListValue, 'rule_type', _('Rule type'));
		o.modalonly = true;
		//1.portbase 2.etherType 3.IP/PORT 4.MAC address 5.WANinterface
		o.textvalue = function(s) {
			return rule_type_txt(s);
		};
		o.value('1', _('Port'));
		o.value('2', _('Ether Type'));
		o.value('3', _('IP/Protocol'));
		o.value('4', _('MAC Address'));
		o.value('5', _('WAN Interface'));
		o.widget = 'radio';
		o.default = '1';


		// Tab Port base 
		o = s.option(form.ListValue, 'lan_interface', _('Physical Port'));
		o.depends("rule_type", "1");
		o.modalonly = true;
		o.default = 'all';
		var ethports=[];
		var brdevs = uci.sections('network', 'device').filter(function(e) { return e.type == 'bridge'});
		for(var i=0; i<brdevs.length; i++){
			var ports=brdevs[i]['ports']?brdevs[i]['ports']:[];
			ethports=ethports.concat(ports.filter(function(port){ return port.substring(0,3) == 'eth'}));
		}
		ethports=ethports.sort();
		o.value('all', _('All Ports'));
		for(var i=0; i<ethports.length; i++){
			console.log(ethports[i]);
			o.value(ethports[i], rtkNetwork.getNetdevDisplay(ethports[i]));
		}
		
		o = s.option(form.ListValue, 'dscp', _('DSCP Pattern'));
		o.depends("rule_type", "1");
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.value('na', _(' '));
		o.value('0', _('default(000000)'));
		o.value('8', _('CS1(001000)'));
		o.value('10', _('AF11(001010)'));
		o.value('12', _('AF12(001100)'));
		o.value('14', _('AF13(001110)'));
		o.value('16', _('CS2(010000)'));
		o.value('18', _('AF21(010010)'));
		o.value('20', _('AF22(010100)'));
		o.value('22', _('AF23(010110)'));
		o.value('24', _('CS3(011000)'));
		o.value('26', _('AF31(011010)'));
		o.value('28', _('AF32(011100)'));
		o.value('30', _('AF33(011110)'));
		o.value('32', _('CS4(100000)'));
		o.value('34', _('AF41(100010)'));
		o.value('36', _('AF42(100100)'));
		o.value('38', _('AF43(100110)'));
		o.value('40', _('CS5(101000)'));
		o.value('46', _('EF(101110)'));


		// Tab Ether Type base
		o = s.option(form.Value, 'ether_type', _('Ethernet Type: 0x'));
		o.depends("rule_type", "2");
		o.modalonly = true;
		
		// Tab IP Protocol base
		o = s.option(form.ListValue, 'ip_version', _('IP Version:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.default = '0';
		o.value('0', _('Please select IP version'));
		o.value('1', _('IPv4'));
		o.value('2', _('IPv6'));
		o.value('3', _('IPv4/IPv6'));

		o = s.option(form.ListValue, 'proto', _('Protocol:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.value('na', _(' '));
		o.value('tcp', _('TCP'));
		o.value('udp', _('UDP'));
		o.value('icmp', _('ICMP'));
		o.value('tcp,udp', _('TCP/UDP'));
/*
		o = s.option(form.ListValue, 'dscp', _('DSCP Pattern'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.value('na', _(' '));
		o.value('0', _('default(000000)'));
		o.value('8', _('CS1(001000)'));
		o.value('10', _('AF11(001010)'));
		o.value('12', _('AF12(001100)'));
		o.value('14', _('AF13(001110)'));
		o.value('16', _('CS2(010000)'));
		o.value('18', _('AF21(010010)'));
		o.value('20', _('AF22(010100)'));
		o.value('22', _('AF23(010110)'));
		o.value('24', _('CS3(011000)'));
		o.value('26', _('AF31(011010)'));
		o.value('28', _('AF32(011100)'));
		o.value('30', _('AF33(011110)'));
		o.value('32', _('CS4(100000)'));
		o.value('34', _('AF41(100010)'));
		o.value('36', _('AF42(100100)'));
		o.value('38', _('AF43(100110)'));
		o.value('40', _('CS5(101000)'));
		o.value('42', _('EF(101110)'));
*/
		o = s.option(form.Value, 'srchost', _('Source IP:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.datatype = 'or(ipaddr,"ignore")';

		o = s.option(form.Value, 'dsthost', _('Destination IP:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.datatype = 'or(ipaddr,"ignore")';
		
		o = s.option(form.Value, 'srcports', _('Source Port:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.datatype = 'portrange';

		o = s.option(form.Value, 'dstports', _('Destination  Port:'));
		o.depends("rule_type", "3");
		o.modalonly = true;
		o.datatype = 'portrange';

		// Tab Mac base
		o = s.option(form.Value, 'src_mac', _('Source MAC:'));
		o.depends("rule_type", "4");
		o.modalonly = true;
		o.datatype = 'macaddr';

		o = s.option(form.Value, 'dst_mac', _('Destination MAC:'));
		o.depends("rule_type", "4");
		o.modalonly = true;
		o.datatype = 'macaddr';


		// Tab Wan interface base 
		o = s.option(widgets.NetworkSelect, 'wan_interface', _('WAN:'));
		o.depends("rule_type", "5");
		o.modalonly = true;
		o.rmempty = false;
		o.nocreate = true;
		o.exclude  = 'lan';

		return m.render();

	}
});
