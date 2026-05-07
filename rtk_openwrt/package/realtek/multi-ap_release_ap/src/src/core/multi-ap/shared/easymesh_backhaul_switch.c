#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "easymesh_backhaul_switch.h"
#include "easymesh_utils.h"
#include "map_logger.h"


#define MAX_DOWN_2G_VXD_TIME 2
#define MAX_DOWN_5G_VXD_TIME 2

#define TRIGGER_THRESHOLD 3
#define BLOCK_THRESHOLD 2

static uint8_t _down_2g_vxd_time = 0;//when 2g vxd is down more than Max_DownVxd_time times, it will stop
static uint8_t _down_5g_vxd_time = 0;

static uint8_t _block_2g_vxd_time = 0;//when 5g vxd is down more than Max_DownVxd_time times, it will stop
static uint8_t _block_5g_vxd_time = 0;

static uint8_t _trigger_5g_to_2g = 0;
static uint8_t _trigger_2g_to_5g = 0;

char *_backhaul_name = NULL;

int _check_blocked_vxd_interface(char *blocked_interface, char *bridge_name)
{
	//down the interface when "brctl showstp br0" show interface "blocking"
	int   counter = -1, i = 0;
	char  checked = 0;
	FILE *in      = NULL;
	char  temp[9];
	int   ch;
	char  vxd[3]   = "vxd";
	char  state[8] = "blocking";
	char  cmd[128] = { 0 };
	// int   res      = 0;


	if (NULL == bridge_name) {
		log_error("bridge name cannot be NULL! \n");
		return -1;
	}

	sprintf(cmd, "brctl showstp %s", bridge_name);

	in = popen(cmd, "re");
	if (in == NULL) {
		puts("Unable to open process");
		return -1;
	}

	while ((ch = fgetc(in)) != EOF) {
		if (checked && i < 8 && ch == state[i]) {
			i++;
		} else if (i == 8) {
			log_info("****Down blocking interface*********\n");
			// char off_vxd_interface[10];
			// strncpy(off_vxd_interface, temp, 9);
			// off_vxd_interface[9] = '\0';
			// if (is_interface_up(off_vxd_interface)) {
			// 	printf("!!!!!!!!!!!down 24G Vxd, %s!!!!!!!!!!!!!", off_vxd_interface);
			// 	sprintf(cmd, "ifconfig %s down", off_vxd_interface);
			// 	system(cmd);
			// }
			strncpy(blocked_interface, temp, 9);
			pclose(in);
			return 1;
		}
		if (counter == 9) {
			if (memcmp(vxd, temp + 6, 3) == 0) {
				checked = 1;
			} else {
				counter = -1;
			}
		}

		if (ch == 'w') {
			counter = 0;
		}
		if (counter >= 0 && counter < 9) {
			temp[counter] = ch;
			counter++;
		}
	}
	pclose(in);
	// for (i = 0; i < 2; i++) {
	// 	if (i == 0)
	// 		in = popen("cat /var/wlan0-vxd/sta_info", "r");
	// 	else
	// 		in = popen("cat /var/wlan1-vxd/sta_info", "r");

	// 	if (in == NULL) {
	// 		puts("Unable to open process");
	// 		return 0;
	// 	}
	// 	counter = 0;
	// 	while ((ch = fgetc(in)) != EOF) {
	// 		if (counter == 30)
	// 			res += ch - '0';
	// 		counter++;
	// 	}
	// 	pclose(in);
	// }
	// if (res == 0)
	// 	return 1;
	return 0;
}

#define PROC_DIR "/proc/net/rtk_wifi6/"
int _check_rssi_value(char *interface_name, char *parameter_name)
{
	//read the rssi value from agent on_vxd_interface/sta_info

	int ret;
	FILE * pipe;
	char * line;

	char command[200];

	ret = 0;

	if ((NULL == interface_name) || (NULL == parameter_name)) {
		return 0;
	}

	int count      = 0;
	int start_line = 0;

	snprintf(command, sizeof(command), PROC_DIR "%s/sta_info", interface_name);

	pipe       = fopen(command, "re");
	start_line = ret;

	if (!pipe) {
		return 0;
	}

	line = malloc(sizeof(char) * 256);

	while (fgets(line, 256, pipe) != NULL) {
		if (count >= start_line) {
			line[strlen(line) - 1] = 0x00;
			char *tmp              = strstr(line, parameter_name);
			if (tmp) {
				tmp += strlen(parameter_name);
				sscanf(tmp, "%d", &ret);
				break;
			}
		}
		count++;
	}

	fclose(pipe);
	free(line);

	log_info("Read the value for rssi is %d under sta info\n", ret);
	return ret;

}

int mixed_backhaul_sta_trigger(char *backhaul_sta, struct easymesh_datamodel *database)
{
	uint8_t rssi                    = 0;
	// char    cmd[128]                = { 0 };
	int result = 0;

	char blocked_backhaul_interface[16] = { 0 };

	// int state_5to2 = 0;
	// int state_2to5 = 0;

	uint8_t configure_state;

	char vxd_24g_interface[32], vxd_5g_interface[32];

	log_info("Trigger rssi time counter on backhaul sta %s\n", backhaul_sta);

	if ((NULL == backhaul_sta)) {
		return 0;
	}

	sprintf(vxd_24g_interface, "%s-vxd", database->radio_name_24g);
	sprintf(vxd_5g_interface, "%s-vxd", database->radio_name_5gl);

	if (backhaul_sta[4] == database->radio_name_5gl[4]) {
		_down_2g_vxd_time = 0;
	}

	if (backhaul_sta[4] == database->radio_name_24g[4]) {
		_down_5g_vxd_time = 0;
	}

	log_info("The time of 5G backhaul sta been down is %d\n", _down_5g_vxd_time);
	log_info("The time of 2G backhaul sta been down is %d\n", _down_2g_vxd_time);

	log_info("The time of 2G backhaul sta been blocked is %d\n", _block_2g_vxd_time);
	log_info("The time of 5G backhaul sta been blocked is %d\n", _block_2g_vxd_time);

	if (1 == _check_blocked_vxd_interface(blocked_backhaul_interface, database->bridge_name)) {//when block time arrive BLOCK_THRESHOLD, down other radio vxd interface
		char  cmd[128]           = { 0 };
		char *backhaul_interface = NULL;
		if (0 == strcmp(blocked_backhaul_interface, vxd_24g_interface)) {
			if (_block_2g_vxd_time > BLOCK_THRESHOLD) {
				backhaul_interface = vxd_5g_interface;
			} else {
				backhaul_interface = vxd_24g_interface;
				_block_2g_vxd_time++;
				_block_5g_vxd_time = 0;
			}
		} else if (0 == strcmp(blocked_backhaul_interface, vxd_5g_interface)) {
			if (_block_5g_vxd_time > BLOCK_THRESHOLD) {
				backhaul_interface = vxd_24g_interface;
			} else {
				backhaul_interface = vxd_5g_interface;
				_block_5g_vxd_time++;
				_block_2g_vxd_time = 0;
			}
		}
		log_info("Backhaul sta %s down after been blocked %d(%d) times! \n", backhaul_interface, _block_2g_vxd_time, _block_5g_vxd_time);
		sprintf(cmd, "ifconfig %s down", backhaul_interface);
		system(cmd);
	}


	rssi            = _check_rssi_value(backhaul_sta, "rssi:");
	configure_state = database->configured_band;

	log_info("Configure state is %d, and rssi is %d\n", configure_state, rssi);

	// if (is_interface_up(vxd_24g_interface) && backhaul_sta[4] == database->radio_name_5gl[4] && rssi >= database->rssi_5g_threshold) {//prefer 5g vxd connection
	// 	printf("\n!!!!%s, %d!!!!!!!!!!!!!Down 24G Vxd!!!!!!!!!!!!!\n", __FUNCTION__, __LINE__);
	// 	sprintf(cmd, "ifconfig %s-vxd down", database->radio_name_24g);
	// 	system(cmd);
	// }
	log_info("The time of achieve trigger radio 5G switch to 24G is %d\n", _trigger_5g_to_2g);
	log_info("The time of achieve trigger radio 24G switch to 5G is %d\n", _trigger_2g_to_5g);


	if (configure_state == MAP_CONFIGURE_BAND_FULL && is_interface_up(vxd_24g_interface) && (backhaul_sta[4] == database->radio_name_24g[4] || backhaul_sta[4] == database->radio_name_5gl[4]) && rssi >= database->rssi_5g_threshold && rssi && _down_2g_vxd_time <= MAX_DOWN_2G_VXD_TIME) {
		log_info("##### Prepare 24G to 5G switch ###\n");
		_trigger_2g_to_5g++;
		_trigger_5g_to_2g = 0;
		if(_trigger_2g_to_5g > TRIGGER_THRESHOLD) {
			log_info("!!!!! 24G do switch to 5G !!!!!\n");
			set_vxd_interface_state(vxd_24g_interface, database->primary_vid, STATE_DOWN);
			set_vxd_interface_state(vxd_5g_interface, database->primary_vid, STATE_UP);
			_block_5g_vxd_time = 0;
			_down_2g_vxd_time++;
			result = 1;
		}
	} else if (configure_state == MAP_CONFIGURE_BAND_FULL && is_interface_up(vxd_5g_interface) && (backhaul_sta[4] == database->radio_name_24g[4] || backhaul_sta[4] == database->radio_name_5gl[4]) && rssi <= database->rssi_24g_threshold && rssi && _down_5g_vxd_time <= MAX_DOWN_5G_VXD_TIME) {
		log_info("##### Prepare 5G to 24G switch ###\n");
		_trigger_5g_to_2g++;
		_trigger_2g_to_5g = 0;
		if(_trigger_5g_to_2g > TRIGGER_THRESHOLD) {
			log_info("!!!!! 5G do switch to 24G !!!!!\n");
			set_vxd_interface_state(vxd_5g_interface, database->primary_vid, STATE_DOWN);
			set_vxd_interface_state(vxd_24g_interface, database->primary_vid, STATE_UP);
			_block_2g_vxd_time = 0;
			_down_5g_vxd_time++;
			result = 1;
		}
	} else {
		_trigger_5g_to_2g = 0;
		_trigger_2g_to_5g = 0;
	}

	return result;
}

void set_vxd_interface_state(char *interface_name, uint16_t primary_vid, uint8_t state)
{
	char interface_buffer[64] = { 0 };

	if (primary_vid) {
		sprintf(interface_buffer, "%s.%d", interface_name, primary_vid);
	} else {
		sprintf(interface_buffer, "%s", interface_name);
	}

	if(STATE_UP == state) {
		if (!is_interface_up(interface_buffer)) {
			ifconfig_helper(interface_name, 1);
			if (primary_vid) {
				ifconfig_helper(interface_buffer, 1);
			}
		}
	} else if(STATE_DOWN == state) {
		ifconfig_helper(interface_buffer, 0);
	}
	log_info("Set backhaul sta state %s as %d\n", interface_buffer, state);
}

void up_backhaul_sta(char *radio_name_5gh, char *radio_name_5gl, char *radio_name_2g, uint16_t primary_vid, uint8_t radio_byte)
{//up on vxd interface connected when controller lost
	char vxd_name[16];

	if (radio_byte & MASK_BAND_5GH) {
		sprintf(vxd_name, "%s-vxd", radio_name_5gh);
		set_vxd_interface_state(vxd_name, primary_vid, STATE_UP);
	}

	if (radio_byte & MASK_BAND_5GL) {
		sprintf(vxd_name, "%s-vxd", radio_name_5gl);
		set_vxd_interface_state(vxd_name, primary_vid, STATE_UP);
	}

	if (radio_byte & MASK_BAND_2G) {
		sprintf(vxd_name, "%s-vxd", radio_name_2g);
		set_vxd_interface_state(vxd_name, primary_vid, STATE_UP);
	}
}

void set_backhaul_interface(char *interface)
{
	if (_backhaul_name) {
		free(_backhaul_name);
	}

	if (NULL == interface) {
		_backhaul_name = NULL;
	} else {
		_backhaul_name = strdup(interface);
	}
}

char *get_backhaul_interface()
{
	return _backhaul_name;
}
