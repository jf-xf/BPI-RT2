'use strict';
'require uci';
'require ui';
'require fs';
'require baseclass';
'require network';

var rootdev = 'veip0';

function strcmp(a, b) {
	if (a > b)
		return 1;
	else if (a < b)
		return -1;
	else
		return 0;
}

function isValidNetmask(mask) {
	// Check that the input is a string
	if (typeof mask !== 'string') {
		return false;
	}

	// Split the mask into its four components
	const components = mask.split('.');

	// Check that there are exactly four components
	if (components.length !== 4) {
		return false;
	}

	// Check that each component is a valid number between 0 and 255
	for (let i = 0; i < components.length; i++) {
		const component = parseInt(components[i]);
		if (isNaN(component) || component < 0 || component > 255) {
			return false;
		}
	}
	// Check that the components form a contiguous sequence of 1s followed by 0s
	let numOnes = 0;
	let numZeros = 0;
	let leadingZero = false;
	for (let i = 0; i < components.length; i++) {
		const binary = parseInt(components[i]).toString(2);
		const padding = '00000000'.substring(binary.length);
		const bits = padding + binary;

		for (let j = 0; j < bits.length; j++) {
			const bit = bits.charAt(j);
			if (bit === '1') {
				if (leadingZero)
					return false;
				numOnes++;
			} else {
				leadingZero = true;
				numZeros++;
			}
		}
	}

	/*
	if (numOnes === 32 || numZeros === 32) {
		return false;
	} else {
		return true;
	}
	*/
	return true;
}

function networkSort(a, b) {
	return strcmp(a.getName(), b.getName());
}

function enumerateNetworks(section_ids, L3DevName, intfName, ipmode, chmode, ignoreBridge, data ) {
	var networks=data[0];
	var rtkEnv=data[1];
	var sections = uci.sections('network', 'device');
	var rtkWans = sections.filter(function(e) { return e.name.indexOf(rootdev) != -1 });
	var rv = [];

	section_ids=L.toArray(section_ids);

	// all device section which has rootdev prefix
	for (var idx = 0; idx < rtkWans.length; idx++){
		// filter section_id
		if(section_ids.length && section_ids.find(function(sid) { return sid == rtkWans[idx]['.name'] }) == null){
			continue;
		}

		// filter ipmode
		if(ipmode && rtkWans[idx]['rtk_ipmode'] && rtkWans[idx]['rtk_ipmode'].indexOf(ipmode) == -1)
			continue;

		// filter chmode
		if(chmode && chmode != rtkWans[idx]['rtk_chmode'])
			continue;

		// filter ignoreBridge
		if(ignoreBridge == 1 && 'Bridged' == rtkWans[idx]['rtk_chmode'])
			continue;

		var wanidx=parseInt(rtkWans[idx]['.name'].substr(rootdev.length+1));
		var startBit=rtkEnv.SOCK_MARK.SOCK_MARK_WAN_INDEX_START;
		var endBit=rtkEnv.SOCK_MARK.SOCK_MARK_WAN_INDEX_END;
		var rtkEnvObj={};
		rtkEnvObj['rtkEnv']=rtkEnv;
		if(!(startBit < 1 || wanidx < 0)){
			rtkEnvObj['wan_mark']=((wanidx+1) << startBit);
		}
		if(!(startBit < 1	|| endBit < 1 || endBit < startBit)){
			rtkEnvObj['wan_mask']=(((1<<(endBit-startBit+1))-1) << startBit);
		}
		rtkEnvObj['tabid']=wanidx+rtkEnv.RT_TBL.SKB_MARK_RT_TBL;

		var foundIntfName=0, foundL3DevName=0;
		// new a rtkWan object and append related to networks.
		var rtkWan = this.instantiateNetwork(rtkWans[idx]['.name'], rtkEnvObj);
		if(rtkWan){

			var subrv = [];
			var interfaces=L.toArray(rtkWan.config.interface);

			for (var j = 0; j < interfaces.length; j++){
				// filter interface section name
				if( !intfName || (intfName && intfName == interfaces[j]) )
					foundIntfName = 1;

				for (var i = 0; i < networks.length; i++){
					if(networks[i].sid == interfaces[j]){
						if( !L3DevName || (L3DevName && L3DevName == networks[i].getL3Device().getName()) )
							foundL3DevName = 1;
						subrv.push(networks[i]);
						var l3dev = networks[i].getL3Device();
						rtkWan.l3dev = l3dev ? l3dev.getName(): "";
					}
				}
			}

			subrv.sort(networkSort);
			rtkWan.networks = subrv;
			if( (foundIntfName == 1  && foundL3DevName == 1) 
				|| interfaces.length == 0	// for Bridged mode
			){
				rv.push(rtkWan);
			}
		}
	}

	return rv;
}

function renderIfaceBadge(rtkwan, network) {
	var span = E('span', { 'class': 'ifacebadge' }, rtkwan['config']['.name'] + ': ');
	var devices = [];
	if(network){
		devices = network.isBridge() ? network.getDevices() : L.toArray(network.getDevice());

		for (var j = 0; j < devices.length && devices[j]; j++) {
			span.appendChild(E('img', {
				'title': '(%s) %s'.format(network.sid.substring(rtkwan['config']['.name'].length+1), devices[j].getI18n()),
				'src': L.resource('icons/%s%s.png'.format(devices[j].getType(), devices[j].isUp() ? '' : '_disabled'))
			}));
		}
	}else{
		//unspecified network case ==> display all network status for this rtkwan
		for(var idx=0; idx < rtkwan.networks.length; idx++){
			devices = rtkwan.networks[idx].isBridge() ? rtkwan.networks[idx].getDevices() : L.toArray(rtkwan.networks[idx].getDevice());

			for (var j = 0; j < devices.length && devices[j]; j++) {
				span.appendChild(E('img', {
					'title': '(%s) %s'.format(rtkwan.networks[idx].sid.substring(rtkwan['config']['.name'].length+1), devices[j].getI18n()),
					'src': L.resource('icons/%s%s.png'.format(devices[j].getType(), devices[j].isUp() ? '' : '_disabled'))
				}));
			}
		}
	}

	if (!devices.length) {
		span.appendChild(E('em', { 'class': 'hide-close' }, _('(no interfaces attached)')));
		span.appendChild(E('em', { 'class': 'hide-open' }, '-'));
	}
	return span;
}

function getWifiSidByName(name){
	var devIdx=0;
	// lookup all wifi-device
	var devs=uci.sections('wireless', 'wifi-device');
	var intfs=uci.sections('wireless', 'wifi-iface');
	for(var i=0;i<devs.length;i++){
		var ifIdx=0;
		for(var j=0;j<intfs.length;j++){
			// lookup all wifi-iface
			if (intfs[j]['device'] == devs[i]['.name']){
				var wlname='';
				if(intfs[j]['ifname']){
					wlname=intfs[j]['ifname'];
				}else{
					wlname="wlan%s".format((ifIdx==0)?devIdx.toString() : "%d-%d".format(devIdx, ifIdx));
				}

				if(name == wlname){
					return intfs[j]['.name'];
				}

				ifIdx++;
			}
		}

		devIdx++;
	}

	return null;
}

function getWifiNameBySid(sid){
	var devIdx=0;
	// lookup all wifi-device
	var devs=uci.sections('wireless', 'wifi-device');
	var intfs=uci.sections('wireless', 'wifi-iface');
	for(var i=0;i<devs.length;i++){
		var ifIdx=0;
		for(var j=0;j<intfs.length;j++){
			// lookup all wifi-iface
			if (intfs[j]['device'] == devs[i]['.name']){
				var wlname='';
				if(intfs[j]['ifname']){
					wlname=intfs[j]['ifname'];
				}else{
					wlname="wlan%s".format((ifIdx==0)?devIdx.toString() : "%d-%d".format(devIdx, ifIdx));
				}

				if(sid == intfs[j]['.name']){
					return wlname;
				}

				ifIdx++;
			}
		}

		devIdx++;
	}

	return null;
}

var rtkNetwork;

/**
 * @class rtkNetwork
 * @memberof LuCI
 * @hideconstructor
 * @classdesc
 *
 * It provides methods to enumerate Realtek interfaces, to query
 * current configuration details and to manipulate settings.
 */
rtkNetwork = baseclass.extend(/** @lends LuCI.rtkNetwork.prototype */ {
	/* private */
	instantiateNetwork: function(name, rtkEnvObj) {
		if (name == null)
			return null;

		var protoClass = RtkWan;
		return new protoClass(name, rtkEnvObj, this);
	},

	/**
	 * render html badge with given rtkWan, network.
	 *
	 * @method
	 *
	 * @param {LuCI.rtkNetwork.RtkWan} rtkwan
	 *
	 * @param {LuCI.network.Protocol} network
	 *
	 * @returns {HTML render rtkwan interface badge}
	 */
	renderIfaceBadge: renderIfaceBadge,

	/**
	 * get device sid by interface sid.
	 *
	 * @method
	 *
	 * @param {string} sid
	 * The ID of interface in /etc/config/network.
	 *
	 * @returns {null|string}
	 * Returns the device sid or `null` if the requested device is
	 * not found.
	 */
	getDevSidByIntfSid: function(sid){
		var intf=uci.get('network', sid);
		var d_sid=null;
		if(intf && intf['.type'] == 'interface' && intf['device']){
			uci.sections('network', 'device', function(dev){
				if(dev['name'] == intf['device']){
					d_sid=dev['.name'];
				}
			});
		}

		return d_sid;
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network exclude 'Bridged' chmode.
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkExcludeBrMode: function() {
		return this.getNetworkByFilter([], null, null, null, null, 1);
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given ipmode.
	 *
	 * @param {string} chmode
	 * The chmode of the network get, e.g. `Bridged` or `IPoE` or `PPPoE`
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkByChMode: function(chmode) {
		return this.getNetworkByFilter([], null, null, null, chmode, null);
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given ipmode.
	 *
	 * @param {string} ipmode
	 * The ipmode of the network get, e.g. 
	 * `IPv4` for IPv4 and IPv4/IPv6
	 * `IPv6` for IPv6 and IPv4/IPv6
	 * `IPv4/IPv6` for IPv4/IPv6
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkByIPMode: function(ipmode, ignoreBridge) {
		return this.getNetworkByFilter([], null, null, ipmode, null, ignoreBridge);
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given interface section name.
	 *
	 * @param {string} intfName
	 * The logical interface name of the network get, e.g. `veip0_0_v4` or `veip0_1_v6`.
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkByIntfName: function(intfName) {
		return this.getNetworkByFilter([], null, intfName, null, null, null);
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given L3 Device name.
	 *
	 * @param {string} L3DevName
	 * The Level 3 Device name of the network get, e.g. `veip0` or `veip0.35` or `pppveip0_2`.
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkByL3DevName: function(L3DevName, ignoreBridge) {
		return this.getNetworkByFilter([], L3DevName, null, null, null, ignoreBridge);
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given L3 Device name.
	 *
	 * @param {string} L3DevName
	 * The Level 3 Device name of the network get, e.g. `veip0` or `veip0.35` or `pppveip0_2`.
	 * @param {string} intfName
	 * The logical interface name of the network get, e.g. `veip0_0_v4` or `veip0_1_v6`.
	 * @param {string} ipmode
	 * The ipmode of the network get, e.g. 
	 * 	`IPv4` for IPv4 and IPv4/IPv6
	 * 	`IPv6` for IPv6 and IPv4/IPv6
	 * 	`IPv4/IPv6` for IPv4/IPv6
	 * @param {string} chmode
	 * The chmode of the network get, e.g. `Bridged` or `IPoE` or `PPPoE`
	 * @param {Boolean} ignoreBridge
	 * 	null or 1.
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetworkByFilter: function(section_ids, L3DevName, intfName, ipmode, chmode, ignoreBridge) {
		return Promise.all([
			network.getNetworks(),
			this.getRtkEnv()
		]).then(L.bind(enumerateNetworks,this, section_ids, L3DevName, intfName, ipmode, chmode, ignoreBridge));
	},

	/**
	 * Get a {@link LuCI.rtkNetwork.RtkWan} instance describing
	 * the realtek network with the given device section name.
	 *
	 * @param {string} name
	 * The logical device name of the realtek network get, e.g. `veip0_0` or `veip0_1`.
	 *
	 * @returns {Promise<null|LuCI.rtkNetwork.RtkWan>}
	 * Returns a promise resolving to a
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instance describing
	 * the network or `null` if the network did not exist.
	 */
	getNetwork: function(section_ids) {
		return this.getNetworkByFilter(section_ids, null, null, null, null, null);
	},

	/**
	 * Gets an array containing all known Realtek networks.
	 *
	 * @returns {Promise<Array<RtkWan>>}
	 * Returns a promise resolving to a name-sorted array of
	 * {@link LuCI.rtkNetwork.RtkWan} subclass instances
	 * describing all known networks.
	 */
	getNetworks: function() {
		return this.getNetworkByFilter([], null, null, null, null, null);
	},

	getBridgeName: function() {
		var sections = uci.sections('network', 'device');
		var brdevs = sections.filter(function(e) { return e.type == 'bridge' && e.name.indexOf('br') != -1 });
		for (var i = 0; i < brdevs.length; i++) {
			return brdevs[i]['name'];
		}
	},

	deleteNetwork: function(name) {
		var section = (name != null) ? uci.get('network', name) : null;
		if (section != null && section['.type'] == 'device'){
			// Remove rtk policy setting
			uci.sections('network', 'policy', function(s) {
				if (s['.name'] == '%s_pmap'.format(section['.name']) && s['device'] == section['.name']){
					uci.remove('network', s['.name']);
				}
			});
			switch(section['rtk_chmode']){
			case 'Bridged':
				var sections = uci.sections('network', 'device');
				var brDevs = sections.filter(function(e) { return e.type == 'bridge' && e.name.indexOf('br') != -1 });
				for (var j = 0; j < brDevs.length; j++) {
					if(brDevs[j]['type'] == 'bridge'){
						var ports=uci.get('network', brDevs[j]['.name'], 'ports');
						uci.set('network', brDevs[j]['.name'], 'ports', ports.filter(function(e) { return e !== section['name'] }));
					}
				}
				break;
			case 'IPoE':
			case 'PPPoE':
				var intfs = section['interface'];
				if(intfs == null)
					break;

				for(var i=0; i < intfs.length; i++){
					// Remove rtk rule/route setting
					uci.sections('network', null, function(s) {
						switch (s['.type']) {
						case 'route':
						case 'route6':
						case 'rule':
						case 'rule6':
							if (s['.name'].indexOf(intfs[i]) != -1){
								uci.remove('network', s['.name']);
							}
							break;
						}
					});
				}
				break;
			default:
				ui.addNotification(_('Delete Network Error'), E('p', [ _('Unknown Channel Mode ')+section['rtk_chmode'] ]));
				break;
			}
		}
	},

	/**
	 * get wifi-iface section_id by wifi netdev name.
	 *
	 * @method
	 *
	 * @param {string} wifi netdev name
	 *
	 * @returns {wifi-iface section_id | NULL}
	 */
	getWifiSidByName: getWifiSidByName,

	/**
	 * get wifi netdev name by wifi-iface section_id.
	 *
	 * @method
	 *
	 * @param {string} wifi-iface section_id
	 *
	 * @returns {wifi netdev name | NULL}
	 *
	 */
	getWifiNameBySid: getWifiNameBySid,

	getRtkEnv: function(){
		return fs.read('/etc/profile.d/rtk_script_env.sh').then(function(res){
			var lines = res.trim().split(/\n/);
			var Obj= {};
			var priObj={}, tblObj={},markObj={};
			for(var i=0;i<lines.length;i++){
				var array=lines[i].trim().split(/[=]/);
				if(array.length==2){
					if(array[0].search(/^RULE_PRI_/) != -1){
						priObj[array[0]]=parseInt(array[1]);
					}else if(array[0].search(/^.*_RT_TBL/) != -1){
						tblObj[array[0]]=parseInt(array[1]);
					}else if(array[0].search(/^SOCK_MARK/) != -1){
						markObj[array[0]]=parseInt(array[1]);
					}
				}
			}

			Obj['RULE_PRI']=priObj;
			Obj['RT_TBL']=tblObj;
			Obj['SOCK_MARK']=markObj;
			return Obj;
		});
	},

	/**
	 * Get a luci display name for net_device.
	 * Be noted to load network uci before using it.
	 *
	 * @param {string} devname
	 * The name of system net_device.
	 *
	 * @returns {string}
	 * The corresponding display name or null if not resolved for the specified net_device.
	 */
	getNetdevDisplay: function(devname) {
		var dispName;

		var itfs = uci.sections('network', 'interface');
		var devs = uci.sections('network', 'device');

		/* eth0.2 ==> LAN1 */
		if (devname.indexOf('eth0.') == 0 && devname.length == 6 && parseInt(devname[5]) >= 2)
			dispName = 'LAN%d'.format(parseInt(devname[5])-1);
		/* wlan */
		else if (devname.indexOf('wlan') == 0)
			dispName = devname;
		else {
			for (var i=0; i < itfs.length; i++) {
				/* PPPoE */
				if (itfs[i].proto == 'pppoe') {
					if (itfs[i].pppname == devname) {
						for (var k=0; k < devs.length; k++) {
							if (devs[k].name == itfs[i].device)
								dispName = devs[k]['.name'];
						}
					}
				}
				else
				if (itfs[i].device == devname) {
					for (var k=0; k < devs.length; k++) {
						if (devs[k].name == devname)
							dispName = devs[k]['.name'];
					}
				}
			}
		}

		//return (dispName != null) ? dispName : devname;
		return dispName;
	},
	isValidNetmask: isValidNetmask,
});

/**
 * @class
 * @memberof LuCI.rtkNetwork
 * @hideconstructor
 * @classdesc
 *
 * The `rtkNetwork.RtkWan` class which describe logical UCI networks 
 * defined by `config device` sections in `/etc/config/network`.
 * There are two member
 *   config    : it show config device section for this RtkWan
 *   networks  : it show LuCI.network.Protocols which attach to this device.
 *
 */
var RtkWan = baseclass.extend(/** @lends LuCI.rtkNetwork.RtkWan.prototype */ {
	__init__: function(name, rtkEnvObj, rtkNetwork) {
		this.sid = name;
		this.rtkNetwork = rtkNetwork;
		this.rtk = rtkEnvObj;
		var section = (name != null) ? uci.get('network', name) : null;

		if (section != null && section['.type'] == 'device') {
			this.config = section;
		}else{
			this.config = null;
		}
	},

	/**
	 * Get the name of the associated logical Realtek interface.
	 *
	 * @returns {string}
	 * Returns the logical interface name, such as `veip0_0` or `veip0_1`.
	 */
	getName: function() {
		return this.sid;
	},

	/**
	 * Read the given UCI option value of this network.
	 *
	 * @param {string} opt
	 * The UCI option name to read.
	 *
	 * @returns {null|string|string[]}
	 * Returns the UCI option value or `null` if the requested option is
	 * not found.
	 */
	get: function(opt) {
		return uci.get('network', this.sid, opt);
	},

	/**
	 * Set the given UCI option of this network to the given value.
	 *
	 * @param {string} opt
	 * The name of the UCI option to set.
	 *
	 * @param {null|string|string[]} val
	 * The value to set or `null` to remove the given option from the
	 * configuration.
	 */
	set: function(opt, val) {
		return uci.set('network', this.sid, opt, val);
	},

	updateBridgePorts: function(isAdd, name){
		var sections = uci.sections('network', 'device');
		var brDevs = sections.filter(function(e) { return e.name == 'br-lan' });
		for (var j = 0; j < brDevs.length; j++) {
			if(brDevs[j]['type'] == 'bridge'){
				var ports=uci.get('network', brDevs[j]['.name'], 'ports');
				var new_ports=[];
				var found=0;

				for (var k = 0; k < ports.length; k++) {
					if(ports[k] != name){
						if(!isAdd){
							new_ports.push(ports[k]);
						}
					}else{
						found = 1;
					}
				}

				if(!isAdd){
					uci.set('network', brDevs[j]['.name'], 'ports', new_ports);
				}else if(!found){
					ports.push(name);
					uci.set('network', brDevs[j]['.name'], 'ports', ports);
				}
			}
		}
	},

	setDevName: function(chmode, newDevName){
		var oldDevName=this.get('name');
		var oldchmode=this.get('rtk_chmode');

		this.set('name', newDevName);
		for(var i=0;i<this.networks.length;i++){
			if(i==0)
				this.networks[i].set('device', newDevName);
		}

		if(oldchmode == 'Bridged'){
			this.updateBridgePorts(false, oldDevName);
		}

		if(chmode == 'Bridged'){
			this.updateBridgePorts(true, newDevName);
		}
	},

	setPortMapping: function(newports){
		newports=L.toArray(newports);
		var pmap_sid="%s_pmap".format(this.sid);
		if(!uci.get('network', pmap_sid)){
			uci.add('network', 'policy', pmap_sid);
		}
		uci.set('network', pmap_sid, 'mark', this.rtk.wan_mark);
		uci.set('network', pmap_sid, 'mask', this.rtk.wan_mask);
		uci.set('network', pmap_sid, 'device', this.sid);
		uci.set('network', pmap_sid, 'type', 'port');

		var policys = uci.sections('network', 'policy');
		for(var i=0; i < policys.length; i++){
			if(newports.length == 0){
				if(policys[i]['.name'] == pmap_sid){
					uci.remove('network', policys[i]['.name']);
					continue;
				}
			}else{
				if(policys[i]['.name'] == pmap_sid){
					if(newports.length > 0)
						uci.set('network', policys[i]['.name'], 'ports', newports);
					else{
						uci.remove('network', policys[i]['.name']);
					}
				}else{
					if(policys[i]['ports'] && policys[i]['ports'].length > 0){
						var ports = policys[i]['ports'];
						for(var j=0; j<newports.length;j++){
							ports = ports.filter(function(port){ return port != newports[j]; });
						}
						if(ports.length > 0)
							uci.set('network', policys[i]['.name'], 'ports', ports);
						else{
							uci.remove('network', policys[i]['.name']);
						}
					}
				}
			}
		}

		var pd_bind_sid="A_pd_bind_%s".format(this.sid);
		if(newports.length == 0){
			uci.remove('network', pd_bind_sid);
		}else{
			if(!uci.get('network', pd_bind_sid)){
				uci.add('network', 'alias', pd_bind_sid);
			}

			uci.set('network', pd_bind_sid, 'interface', 'lan');
			uci.set('network', pd_bind_sid, 'proto', 'static');
			uci.set('network', pd_bind_sid, 'ip6assign', '60');
			uci.set('network', pd_bind_sid, 'ip6class', L.toArray('%s_v6'.format(this.sid)));
			uci.set('network', pd_bind_sid, 'ip6tables', 'main');
			uci.set('network', pd_bind_sid, 'ip6tables2', '260');
		}

		Promise.all([
			uci.load('dhcp')
		]).then(L.bind(function(wan_mark, wan_mask, data){
			if(newports.length == 0){
				uci.remove('dhcp', pd_bind_sid);
			}else{
				if(!uci.get('dhcp', pd_bind_sid)){
					uci.add('dhcp', 'dhcp', pd_bind_sid);
					uci.set('dhcp', pd_bind_sid, 'interface', pd_bind_sid);
					uci.set('dhcp', pd_bind_sid, 'dhcpv4', 'disabled');
					uci.set('dhcp', pd_bind_sid, 'dhcpv6', uci.get('dhcp', 'lan', 'dhcpv6'));
					uci.set('dhcp', pd_bind_sid, 'ra', uci.get('dhcp', 'lan', 'ra'));
					uci.set('dhcp', pd_bind_sid, 'ra_slaac', uci.get('dhcp', 'lan', 'ra_slaac'));
					uci.set('dhcp', pd_bind_sid, 'v6_fixed_assigned_length', uci.get('dhcp', 'lan', 'v6_fixed_assigned_length'));
					uci.set('dhcp', pd_bind_sid, 'v6_pd_route_tableid', uci.get('dhcp', 'lan', 'v6_pd_route_tableid'));
					uci.set('dhcp', pd_bind_sid, 'ra_flags', null);
					uci.set('dhcp', pd_bind_sid, 'ra_flags', uci.get('dhcp', 'lan', 'ra_flags'));
					uci.set('dhcp', pd_bind_sid, 'filter_mark', wan_mark);
					uci.set('dhcp', pd_bind_sid, 'filter_mark_mask', wan_mask);
				}
			}

			return true;
		}, this, this.rtk.wan_mark, this.rtk.wan_mask));

		return true;
	},

	getRTKWanMark: function(){
		return this.rtk.wan_mark;
	},

	getRTKWanMask: function(){
		return this.rtk.wan_mask;
	},

	getRTKTableID: function(){
		return this.rtk.tabid;
	},

	getRTKEnv: function(){
		return this.rtk.rtkEnv;
	},
});

return rtkNetwork;
