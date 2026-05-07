'use strict';
'require view';
'require form';
'require fs';

var rawData = '{"lan":[]}';

return view.extend({
		load: function() {
			return Promise.all([
				fs.exec_direct('/sbin/diag', [ 'rt_port', 'get', 'status', 'port', 'all' ]),
				fs.exec_direct('/sbin/diag', [ 'rt_port', 'get', 'auto-nego', 'port', 'all', 'state' ])
			]);
		},

		render: function(data) {
			// parser port status result and transform to json data format to fit JSONMap
			var portStatus = data[0].trim().split(/\n/).map(function(line) {
				return line.replace(/^<\d+>/, '');
			});
			var negoStatus = data[1].trim().split(/\n/).map(function(line) {
				return line.replace(/^<\d+>/, '');
			});

			var formData = JSON.parse(rawData);
			for(var i=0;i<portStatus.length;i++){
			var portArray=portStatus[i].split(/\s+/);	//split whitespace
				// it still can not get port/netdev mapping, so we assume port 0-3 is default lan port.
				if(portArray[0].match(/^[0-3]/)){
					for(var j=0;j<negoStatus.length;j++){
						// Port 0 Auto-Nego state: Enable
						var negoArray=negoStatus[j].split(/\s+/);	//split whitespace
						if(portArray[0] == negoArray[2]){
							formData['lan'].push({"name":"LAN %d".format(parseInt(portArray[0])+1),"port":portArray[0],"status":portArray[1],"speed":portArray[2],"duplex":portArray[3], "nego":negoArray[5]});
						}
					}
				}
			}
			rawData = JSON.stringify(formData);

			var m, s, o;

			m = new form.JSONMap(formData, _('LAN Status'));
			//m.readonly = !L.hasViewPermission();

			s = m.section(form.GridSection, 'lan', _('Settings'));
			s.anonymous=true;
			s.renderRowActions = function(section_id){
				return E([]);
			};

			o = s.option(form.DummyValue, 'name', _('Name'));
			o.cfgvalue = function(section_id) {
				return o.map.data.get('json', section_id, 'name');
			};

			o = s.option(form.DummyValue, '_status', _("Status"));
			o.cfgvalue = function(section_id) {
				if(o.map.data.get('json', section_id, 'status') != 'Up'){
					return 'Not Connected';
				}else{
					return '%s, %sMbps, %s Duplex'.format(o.map.data.get('json', section_id, 'status'), o.map.data.get('json', section_id, 'speed'), o.map.data.get('json', section_id, 'duplex'));
				}
			};

			o = s.option(form.ListValue, 'mode', _('Mode'));
			o.editable=true;
			o.value('auto', _('Auto Negotiation'));
			o.value('10h', _('10Mbps Half Duplex'));
			o.value('10f', _('10Mbps Full Duplex'));
			o.value('100h', _('100Mbps Half Duplex'));
			o.value('100f', _('100Mbps Full Duplex'));
			o.cfgvalue = function(section_id) {
				if(o.map.data.get('json', section_id, 'nego') == 'Enable'){
					return 'auto';
				}else{
					var rate=o.map.data.get('json', section_id, 'speed');
					var duplex=o.map.data.get('json', section_id, 'duplex');
					return '%s%s'.format(rate.slice(0,rate.search(/M/i)), (duplex=="Half")?'h':'f');
				}
			};
			o.write = function(section_id, formvalue){
				//var cmd=[];
				var port=o.map.data.get('json', section_id, 'port');
				if(formvalue == "auto"){
					//cmd.push("/sbin/diag rt_port set auto-nego port %d state enable".format(o.map.data.get('json', section_id, 'port')));
					fs.exec('/sbin/diag', [ 'rt_port', 'set', 'auto-nego', 'port', port, 'state', 'enable' ]);
				}else{
					//cmd.push("/sbin/diag rt_port set auto-nego port %d state disable".format(o.map.data.get('json', section_id, 'port')));
					//cmd.push("/sbin/diag rt_port set phy-force port %d ability %s flow-control disable".format(o.map.data.get('json', section_id, 'port'),formvalue));
					fs.exec('/sbin/diag', [ 'rt_port', 'set', 'auto-nego', 'port', port, 'state', 'disable' ]);
					fs.exec('/sbin/diag', [ 'rt_port', 'set', 'phy-force', 'port', port, 'ability', formvalue, 'flow-control', 'disable' ]);
				}
			};

			return m.render();
		},


		handleSave: function(ev) {
			var tasks = [];

			document.getElementById('maincontent')
				.querySelectorAll('.cbi-map').forEach(function(map) {
					tasks.push(DOM.callClassMethod(map, 'save'));
				});

			return Promise.all(tasks).then(function(){
				setTimeout(function(){
					location.reload();
				}, 3000);
			});
		},

		handleSaveApply: null
});
