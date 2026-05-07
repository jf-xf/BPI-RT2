'use strict';
'require view';
'require uci';
'require rpc';
'require form';
'require fs';
'require ui';
'require dom';
'require poll';

var pollFn=null;
var need_apply_5g = false;
var need_apply_2g = false;
var poll_count = 0;
var timeout = 30;
var scan_all ='0';
var scan_5g ='2';
var scan_2g ='1';
var device5G = 0;
var device2G = 1;
var device5G_nr = 2;
var device2G_nr = 3;
var scan_time_max = 3;
var scan_time = 1;
var data_incorrect = 0;
var g_data_incorrect = 0;
var data_correct = 0;
var g_data_correct = 0;
var all_data_correct = 3;

var nd_running_result_title = [E('h3', {}, [ _('Please wait for the scan result ...') ]), E('h2', {}, [ _('Channel Scan Result') ])];
var nd_result_title = [E('h2', {}, [ _('Channel Scan Result') ])];
var nd_scan_title = [E('h3', {}, [ _('Please wait for the scan result ...') ])];
var nd_timeout_title = [E('h3', {}, [ _('Finished with timeout !') ])];

function stopOrRescan(table, sband) {
	var finished = false;

	if (data_incorrect) {
		if (scan_time >= scan_time_max) {
			poll.remove(pollFn);
			pollFn = null;
			scan_time = 1;
			finished = true;
			table.push(E('div', { 'class': 'tr' }, [
				E('h3', 'The scanned channel report of radio%d is incorrect !!'.format(data_incorrect-1))]));
		}
		else {
			if (sband == scan_all) {
				if ((g_data_correct & 0x1) == 0x1)	// 5G date is correct
					sband = '1';                  	// re-scan 2G
				if ((g_data_correct & 0x2) == 0x2)	// 2G date is correct
					sband = '2';                  	// re-scan 5G
			}
			//console.log('re-Scan: sband '+sband);
			fs.exec('/etc/wlan/scan.sh', [ sband ]);
			scan_time++;
			//table.push(...nd_scan_title);
		}
	}
	else {
		poll.remove(pollFn);
		pollFn = null;
		finished = true;
	}
	poll_count = 0;
	return finished;
}

function stopOrRescanALL(table) {
	var finished = false;
	var sband;

	if (g_data_correct != all_data_correct) {
		if (scan_time >= scan_time_max) {
			poll.remove(pollFn);
			pollFn = null;
			scan_time = 1;
			finished = true;

			if ((g_data_correct & 0x1) == 0) {  // 5G  Report does not exist
				table.push(E('div', { 'class': 'tr' }, [
					E('h3', 'The scanned channel report of radio0 does not exist !!')]));
			}
			if ((g_data_correct & 0x2) == 0) {  // 2G Report does not exist
				table.push(E('div', { 'class': 'tr' }, [
					E('h3', 'The scanned channel report of radio1 does not exist !!')]));
			}
		}
		else {
			if ((g_data_correct & 0x1) == 0x1)	// 5G date is correct
				sband = '1';                  	// re-scan 2G
			else                               	// 2G date is correct
				sband = '2';                   	// re-scan 5G
			//console.log('re-ScanALL: sband '+sband);
			fs.exec('/etc/wlan/scan.sh', [ sband ]);
			scan_time++;
			//table.push(...nd_scan_title);
		}
	}
	else {
		poll.remove(pollFn);
		pollFn = null;
		finished = true;
	}
	poll_count = 0;
	return finished;
}

function ChannelSet(best_channel_5g, best_channel_2g) {
	var check5G;
	var check2G;

	if (need_apply_5g == true) {
		check5G = document.getElementById('band0').checked;
		if (check5G == true) {
			fs.remove('/tmp/map_channel_scan_report_5G');
			fs.remove('/tmp/map_channel_scan_nr_5G');
			uci.set('wireless', 'phy0', 'channel', best_channel_5g);
		}
	}

	if (need_apply_2g == true) {
		check2G = document.getElementById('band1').checked;
		if (check2G == true) {
			fs.remove('/tmp/map_channel_scan_report_2G');
			fs.remove('/tmp/map_channel_scan_nr_2G');
			uci.set('wireless', 'phy1', 'channel', best_channel_2g);
		}
	}

	if (check5G == true || check2G == true) {
		uci.save().then(function() {
			ui.changes.apply();
		});
	}
}

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('wireless'),
			uci.load('rtkmap')
		]);
	},

	handleScan: function(ev) {
		var sband= document.getElementById('band').value;

		poll_count = 0;
		fs.exec('/etc/wlan/scan.sh', [ sband ]);
		pollFn = this.pollData;
		poll.add(pollFn, 1);
		g_data_incorrect = 0;
		g_data_correct = 0;
	},

	pollData: function() {
		return Promise.all([
			L.resolveDefault(fs.read('/tmp/map_channel_scan_report_5G'), ''),
			L.resolveDefault(fs.read('/tmp/map_channel_scan_report_2G'), ''),
			L.resolveDefault(fs.read('/tmp/map_channel_scan_nr_5G'), ''),
			L.resolveDefault(fs.read('/tmp/map_channel_scan_nr_2G'), '')
		]).then(L.bind(function(data){
			var table = [];
			var nd_data = [];
			var nd_selection = [];
			var nd_apply = [];
			var out = document.querySelector('pre');
			var sband= document.getElementById('band').value;
			var best_channel_5g = 0;
			var best_channel_2g = 0;
			var channel_5g_num = 0;
			var channel_2g_num = 0;
			var display = ['off', 'off' ];

			//console.log('Hi: poll_count is  '+poll_count);
			poll_count = poll_count + 1;

			if (sband == scan_all) {
				display[device5G] = 'on';
				display[device2G] = 'on';
			} else if (sband == scan_5g) {
				display[device5G] = 'on';
				display[device2G] = 'off';
			} else if (sband == scan_2g) {
				display[device5G] = 'off';
				display[device2G] = 'on';
			}

			if ((sband == scan_all && (data[device5G] == '' && data[device2G] == '')) ||
				(sband == scan_5g && data[device5G] == '') ||
				(sband == scan_2g && data[device2G] == '')) {

				//table.push(E('div', { 'class': 'tr' }, [
				//		E('strong', 'There are no scanned channel information here.')]));

				if (poll_count < timeout) {
					table.push(...nd_scan_title);
				} else {
					if(pollFn) {
						if (scan_time >= scan_time_max) {
							poll.remove(pollFn);
							pollFn = null;
							poll_count = 0;
							scan_time = 1;
							table.push(...nd_timeout_title);
						}
						else {
							//console.log('No data : reScan');
							poll_count = 0;
							fs.exec('/etc/wlan/scan.sh', [ sband ]);
							scan_time++;
							table.push(...nd_scan_title);
						}
					}
				}
				return dom.content(out, table);
			}

			// get Total radio0 scanned channel
			if (data[device5G_nr] != '') {
				var channel_5g_nr = data[device5G_nr].trim().split(/ /);
				channel_5g_num = channel_5g_nr[1];
			}

			// get Total radio1 scanned channel
			if (data[device2G_nr] != '') {
				var channel_2g_nr = data[device2G_nr].trim().split(/ /);
				channel_2g_num = channel_2g_nr[1];
			}

			// band = 0, 5G; band = 1, 2.4G
			for (var band = 0; band < 2; band++) {
				if (data[band] != '' && display[band] == 'on') {
					var result_lines = data[band].trim().split(/\n/);
					var score, best_score;
					var cur_best_channel;
					var channel_num = 0;
					var op_list=[], channe_llist=[], status_list=[], utilization_list=[], noise_list=[], neighbor_nr_list=[], count=0;

					cur_best_channel = (band == device5G) ? uci.get('wireless', 'phy0', 'channel'): uci.get('wireless', 'phy1', 'channel');
					for (var i = 0; i < result_lines.length; i++) {
						var op_tmp, channel_tmp, status_tmp, utilization_tmp, noise_tmp, neighbor_nr_tmp;

						if (result_lines[i].match('op_class')) {
							op_tmp = result_lines[i].trim().split(/ /);
							op_list[count] = op_tmp[1];
						}
						else if (result_lines[i].match('channel')) {
							channel_tmp = result_lines[i].trim().split(/ /);
							channe_llist[count] = channel_tmp[1];
						}
						else if (result_lines[i].match('scan_status')) {
							status_tmp = result_lines[i].trim().split(/ /);
							status_list[count] = status_tmp[1];
						}
						else if (result_lines[i].match('utilization')) {
							utilization_tmp = result_lines[i].trim().split(/ /);
							utilization_list[count] = utilization_tmp[1];
						}
						else if (result_lines[i].match('noise')) {
							noise_tmp = result_lines[i].trim().split(/ /);
							noise_list[count] = noise_tmp[1];
						}
						else if (result_lines[i].match('neighbor_nr')) {
							neighbor_nr_tmp = result_lines[i].trim().split(/ /);
							neighbor_nr_list[count] = neighbor_nr_tmp[1];
							count++;
						}
					}

					if (band == device5G)
						channel_num = channel_5g_num;
					else
						channel_num = channel_2g_num;

					if (count == 0 || (channel_num != 0 && count != channel_num)) {
						//table.push(E('div', { 'class': 'tr' }, [
						//	E('strong', 'The radio%d scanned channel report is incorrect !!'.format(band))]));
						data_incorrect = band+1;
						g_data_incorrect |= data_incorrect;
						if ((g_data_correct & data_incorrect) == data_incorrect)
							g_data_correct = g_data_correct - data_incorrect;
					}
					else {
						var best_channel = 0;

						data_correct = band+1;
						g_data_correct |= data_correct;
						if ((g_data_incorrect & data_correct) == data_correct)
							g_data_incorrect = g_data_incorrect - data_correct;

						nd_data.push(E('div', { 'class': 'tr', 'style': 'color: white;' }, ['white']));
						nd_data.push(E('table', { 'class': 'table' , 'id': 'result%d'.format(band)}));
						nd_data.push(E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td center' }, [ 'OP Class' ]),
							E('td', { 'class': 'td center' }, [ 'Channel' ]),
							E('td', { 'class': 'td center' }, [ 'Score' ])]));

						for (var i = 0; i < count; i++) {
							var cu_weight = 0, noise_weight = 0, neighbor_weight = 100;
							var score = ((255 - utilization_list[i]) * 100) / 255 * cu_weight + (100 - noise_list[i]) * noise_weight + (100 - neighbor_nr_list[i]) * neighbor_weight;
							score = score/(cu_weight + noise_weight + neighbor_weight);

							if (i == 0) {
								best_channel = channe_llist[i];
								best_score = score;
							}
							else if (score > best_score) {
								best_channel = channe_llist[i];
								best_score = score;
							}

							nd_data.push(E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td center' }, [ op_list[i] ]),
								E('td', { 'class': 'td center' }, [ channe_llist[i] ]),
								E('td', { 'class': 'td center' }, [ score ])]));
						}

						if (band == device5G)
							best_channel_5g = best_channel;
						else if (band == device2G)
							best_channel_2g = best_channel;

						if (best_channel != 0 && cur_best_channel != best_channel) {
							if (band == device5G)
								need_apply_5g = true;
							else if (band == device2G)
								need_apply_2g = true;

							nd_selection.push(E('tr', { 'class': 'tr' }, [
								E('td', { 'class': 'td left' }, [ E('input', { 'id': 'band%d'.format(band), 'type': 'checkbox', 'name': 'band%d'.format(band)})]),
								E('td', { 'class': 'td center', 'style': 'color: blue;'}, [ 'radio%d switch to best channel '.format(band) + best_channel ])
							]));
						}
					}
				}
			}

			if(pollFn) {
				var finished = false;

				// timeout
				if (poll_count >= timeout) {
					if (sband == scan_all)
						finished = stopOrRescanALL(table);
					else
						finished = stopOrRescan(table, sband);
				}
				// scan for ALL
				else if (sband == scan_all && data[device5G] != '' && data[device2G] != '' && data[device5G_nr] != '' && data[device2G_nr] != '') {
					//console.log('scan_all finished!');
					finished = stopOrRescan(table, sband);
				}
				// scan for 5GHz
				else if (sband == scan_5g && data[device5G] != '' && data[device5G_nr] != '') {
					//console.log('scan_5g finished!');
					finished = stopOrRescan(table, sband);
				}
				// scan for 2.4GHz
				else if (sband == scan_2g && data[device2G] != '' && data[device2G_nr] != '') {
					//console.log('scan_2g finished!');
					finished = stopOrRescan(table, sband);
				}
				// when one Report or NR file does not exist, and one report data is incorrect.
				else if (data_incorrect !=0) {
					//console.log('when one Report or NR file does not exist, and one report data is incorrect.');
					finished = stopOrRescan(table, sband);
				}

				if (finished) { /* Remove data */
					if (data[device5G])
						fs.remove('/tmp/map_channel_scan_report_5G');
					if (data[device2G])
						fs.remove('/tmp/map_channel_scan_report_2G');
					if (data[device5G_nr])
						fs.remove('/tmp/map_channel_scan_nr_5G');
					if (data[device2G_nr])
						fs.remove('/tmp/map_channel_scan_nr_2G');
				}

				// When polling date is finished, check if need to display "Apply".
				if (g_data_correct && finished && (need_apply_5g || need_apply_2g)) {
					nd_apply.push(E('tr', { 'class': 'tr' }, [
						E('td', { 'class': 'td left' }, [
							E('button', {
								'class': 'cbi-button cbi-button-action',
								'click': ui.createHandlerFn(this, ChannelSet, best_channel_5g, best_channel_2g)
							}, _('Apply'))
						])
					]));
				}
			}

			//console.log('g_data_incorrect '+g_data_incorrect);
			//console.log('g_data_correct '+g_data_correct);
			if (g_data_correct) {
				if (!finished)
					table.push(...nd_running_result_title, ...nd_selection, ...nd_apply, ...nd_data);
				else
					table.push(...nd_result_title, ...nd_selection, ...nd_apply, ...nd_data);
			}
			else {
				if (!finished)
					table.push(...nd_scan_title);
				else {
					if (sband == scan_all) {
						if (data_incorrect == '2') {	// This run is 2G scan failed, if the 5G scan is failed also, need to print the 5G error message.
							if ((g_data_incorrect & 0x1) == 0 && (g_data_correct & 0x1) == 0) { // 5G
								table.push(E('div', { 'class': 'tr' }, [
									E('h3', 'The scanned channel report of radio0 does not exist !!')]));
							}
							else if ((g_data_incorrect & 0x1) == 0x1) {
								table.push(E('div', { 'class': 'tr' }, [
									E('h3', 'The scanned channel report of radio0 is incorrect !!')]));
							}
						}

						if (data_incorrect == '1') {	// This run is 5G scan failed, if the 2G scan is failed also, need to print the 2G error message.
							if ((g_data_incorrect & 0x2) == 0 && (g_data_correct & 0x2) == 0)  { // 2G
								table.push(E('div', { 'class': 'tr' }, [
									E('h3', 'The scanned channel report of radio1 does not exist !!')]));
							}
							else if ((g_data_incorrect & 0x2) == 0x2) {
								table.push(E('div', { 'class': 'tr' }, [
									E('h3', 'The scanned channel report of radio1 is incorrect !!')]));
							}
						}
					}
				}
			}

			data_incorrect = 0;
			data_correct = 0;
			dom.content(out, table);

		}, this));
	},

	render: function() {

		var map_role = uci.get('rtkmap', 'map', 'map_role');
		var table1 = E('select', { 'class': 'cbi-input-select', 'id': 'band', 'disabled': map_role=='1'? null:map_role });
		table1.appendChild(E('option', { 'value': '2' }, '5G'));
		table1.appendChild(E('option', { 'value': '1' }, '2.4G'));
		table1.appendChild(E('option', { 'value': '0' }, 'ALL'));

		return E([], [
			E('h2', {}, [ _('EasyMesh Channel Scan') ]),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width' : '200em' }, [
						E('strong', 'Scan Band ')
					]),
					table1
				])
			]),
			E('div', { 'class': 'table' }, [
				E('div', { 'class': 'tr' }, [
					E('div', { 'class': 'td left' }, [
						E('span', { 'class': 'diag-action' }, [
							E('button', {
								'class': 'cbi-button cbi-button-action',
								'click': ui.createHandlerFn(this, 'handleScan'),
								'disabled': map_role=='1'? null:map_role
							}, [ _('Start') ])
						])
					])
				])
			]),
			E('pre', { 'style': 'width:100%'})
		]);
	},

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
