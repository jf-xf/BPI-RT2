#include "ndisc.h"

struct ndisc_intfsetting_args
{
	struct uci_section *iface_s;
	struct uci_section *dmmap_s;
};

enum {
	DADTransmits=0,
	RetransTimer,
	RtrSolicitationInterval,
	MaxRtrSolicitations,
	UnicastSolicit,
	McastSolicit,
	PARAM_MAX
};

enum {
	SYSFS_DEF_PATH=0,
	SYSFS_CUR_PATH,
	SYSCTL_PATH,
	TYPE_MAX
};

char *config_path[PARAM_MAX][TYPE_MAX]={
{//DADTransmits
"/proc/sys/net/ipv6/conf/default/router_solicitations",	// For Default
"/proc/sys/net/ipv6/conf/%s/router_solicitations",	// For get current setting
"net.ipv6.conf.%s.router_solicitations"			// For set sysfs by sysctl
},
{//RetransTimer
"/proc/sys/net/ipv6/neigh/default/retrans_time_ms",	// For Default
"/proc/sys/net/ipv6/neigh/%s/retrans_time_ms",		// For get current setting
"net.ipv6.neigh.%s.retrans_time_ms"			// For set sysfs by sysctl
},
{//RtrSolicitationInterval
"/proc/sys/net/ipv6/conf/default/router_solicitation_interval",	// For Default
"/proc/sys/net/ipv6/conf/%s/router_solicitation_interval",	// For get current setting
"net.ipv6.conf.%s.router_solicitation_interval"			// For set sysfs by sysctl
},
{//MaxRtrSolicitations
"/proc/sys/net/ipv6/conf/default/router_solicitations",	// For Default
"/proc/sys/net/ipv6/conf/%s/router_solicitations",	// For get current setting
"net.ipv6.conf.%s.router_solicitations"			// For set sysfs by sysctl
},
{//UnicastSolicit
"/proc/sys/net/ipv6/neigh/default/ucast_solicit",	// For Default
"/proc/sys/net/ipv6/neigh/%s/ucast_solicit",		// For get current setting
"net.ipv6.neigh.%s.ucast_solicit"			// For set sysfs by sysctl
},
{//McastSolicit
"/proc/sys/net/ipv6/neigh/default/mcast_solicit",	// For Default
"/proc/sys/net/ipv6/neigh/%s/mcast_solicit",		// For get current setting
"net.ipv6.neigh.%s.mcast_solicit"			// For set sysfs by sysctl
}
};

/*************************************************************
* INIT
**************************************************************/


/**************************************************************************
* LINKER
***************************************************************************/


/*************************************************************
* COMMON Functions
**************************************************************/
static char *get_config_path(void *data, unsigned int param, unsigned int path_type){
	char *device=NULL, *sysfs=NULL;
	if(path_type == SYSFS_DEF_PATH){
		dmasprintf(&sysfs, config_path[param][path_type]);
	}else{
		if(!data)
			return NULL;

		dmuci_get_value_by_section_string(((struct ndisc_intfsetting_args *)data)->iface_s, "device", &device);
		if(!device || !DM_LSTRCMP(device, "\0"))
			return NULL;
		dmasprintf(&sysfs, config_path[param][path_type], device);
	}
	return sysfs;
}

static int get_section_by_parameter(struct uci_section **s, char *sysctl)
{
	uci_foreach_option_eq("rtk_sysctl", "sysctl", "key", sysctl, *s){
		return 0;
	}
	dmuci_add_section("rtk_sysctl", "sysctl", s);
	dmuci_set_value_by_section(*s, "key", sysctl);
	return 0;
}

static int get_sysfs_value(char *sysfs, char **value){
	char buf[64]={0};
	if(dm_read_sysfs_file(sysfs, buf, sizeof(buf)) != -1){
		dmasprintf(value, "%s", buf);
	}
	return 0;
}

#if 0
static int get_sysfs_value_by_parameter(void *data, unsigned int param, char **value)
{
	char *sysfs=NULL;
	if(!data || !value)
		return -1;

	dmasprintf(&sysfs, "%s", get_config_path(data, param, SYSFS_CUR_PATH));
	get_sysfs_value(sysfs, value);
	//fprintf(stderr, "[%s]case %u --> %s=%s\n", __FUNCTION__, param, sysfs, *value);
	return 0;
}
#endif

static int set_sysfs_value_by_parameter(void *data, unsigned int param, char *value)
{
	struct uci_section *sysctl_s=NULL;
	char *sysfs=NULL;
	if(!data && !value)
		return -1;

	dmasprintf(&sysfs, "%s", get_config_path(data, param, SYSCTL_PATH));
	get_section_by_parameter(&sysctl_s, sysfs);
	dmuci_set_value_by_section(sysctl_s, "value", value);
	//fprintf(stderr, "[%s]case %u --> (%p)%s=%s\n", __FUNCTION__, param, sysctl_s, sysfs, value);
	return 0;
}

static int get_IP_IPv6Capable(struct uci_section *data, char **value)
{
	char *device = NULL, *sysfs = NULL, buf[16]={0};
	dmasprintf(value, "1");
	dmuci_get_value_by_section_string(data, "device", &device);
	dmasprintf(&sysfs, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", device);
	if(dm_read_sysfs_file(sysfs, buf, sizeof(buf)) == -1)
		return -1;
	dmasprintf(value, "%s", buf);

        return 0;
}

static bool is_ndisc_intf_section_exist(char *sec_name)
{
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(bbfdm, "dmmap_ndisc", "interface", "iface_name", sec_name, s) {
		return true;
	}

	return false;
}

static void dmmap_synchronizeNeighborDiscoveryInterface(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct uci_section *s = NULL, *stmp = NULL;

	uci_path_foreach_sections_safe(bbfdm, "dmmap_ndisc", "interface", stmp, s) {
		struct uci_section *iface_s = NULL;
		char *added_by_controller = NULL;
		char *iface_name = NULL;

		dmuci_get_value_by_section_string(s, "added_by_controller", &added_by_controller);
		if (DM_LSTRCMP(added_by_controller, "1") == 0)
			continue;

		dmuci_get_value_by_section_string(s, "iface_name", &iface_name);
		if (DM_STRLEN(iface_name))
			get_config_section_of_dmmap_section("network", "interface", iface_name, &iface_s);

		if (!iface_s)
			dmuci_delete_by_section(s, NULL, NULL);
	}

	uci_foreach_sections("network", "interface", s) {
		struct uci_section *ndisc_s = NULL;
		char *device = NULL;

		 dmuci_get_value_by_section_string(s, "device", &device);
 		 if (DM_STRCHR(device, '@'))
			 continue;

		// Skip this interface section if its name is equal to loopback
		if (strcmp(section_name(s), "loopback") == 0)
			continue;

		if (is_ndisc_intf_section_exist(section_name(s)))
			continue;

		dmuci_add_section_bbfdm("dmmap_ndisc", "interface", &ndisc_s);
		dmuci_set_value_by_section(ndisc_s, "iface_name", section_name(s));
	}
}

/*************************************************************
* ENTRY METHOD
**************************************************************/
/*#Device.NeighborDiscovery.InterfaceSetting.{i}.!UCI:dmmap_ndisc/interface*/
static int browseNeighborDiscoveryInterfaceSettingInst(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct ndisc_intfsetting_args curr_ndisc_intf_args = {0};
	struct uci_section *dmmap_s = NULL;
	char *inst = NULL;

	dmmap_synchronizeNeighborDiscoveryInterface(dmctx, parent_node, prev_data, prev_instance);
	uci_path_foreach_sections(bbfdm, "dmmap_ndisc", "interface", dmmap_s) {
		struct uci_section *iface_s = NULL;
		char *iface_name = NULL;

		dmuci_get_value_by_section_string(dmmap_s, "iface_name", &iface_name);
		if (DM_STRLEN(iface_name))
			get_config_section_of_dmmap_section("network", "interface", iface_name, &iface_s);

		curr_ndisc_intf_args.iface_s = iface_s;
		curr_ndisc_intf_args.dmmap_s = dmmap_s;

		inst = handle_instance(dmctx, parent_node, dmmap_s, "ndisc_intf_instance", "ndisc_intf_alias");

		if (DM_LINK_INST_OBJ(dmctx, parent_node, (void *)&curr_ndisc_intf_args, inst) == DM_STOP)
			break;
	}
	return 0;
}

/*************************************************************
* GET & SET PARAM
**************************************************************/
static int get_NeighborDiscovery_enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = "1";
	return 0;
}

static int set_NeighborDiscovery_enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	bool b;

	switch (action) {
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			return 0;
		case VALUESET:
			string_to_bool(value, &b);
			//dmuci_set_value("system", "ntp", "enabled", b ? "1" : "0");
			return 0;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSettingNumberOfEntries!UCI:dmmap_ndisc/interface/*/
static int get_NeighborDiscovery_InterfaceSettingNumberOfEntries(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	int cnt = get_number_of_entries(ctx, data, instance, browseNeighborDiscoveryInterfaceSettingInst);
	dmasprintf(value, "%d", cnt);
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.Enable!UCI:dmmap_ndisc/interface,@i-1/enable*/
static int get_NeighborDiscovery_IntfSetting_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", "0");
	*value = ((*value)[0] == '0') ? "0" : "1";
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	bool b;
	switch (action) {
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			return 0;
		case VALUESET:
			string_to_bool(value, &b);
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", b ? "1" : "0");
			if(b){
				char *tmpvalue=NULL;
				get_sysfs_value(get_config_path(NULL, UnicastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, UnicastSolicit, tmpvalue);
				get_sysfs_value(get_config_path(NULL, McastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, McastSolicit, tmpvalue);
			}else{
				set_sysfs_value_by_parameter(data, UnicastSolicit, "0");
				set_sysfs_value_by_parameter(data, McastSolicit, "0");
			}
			if(b){
				//set_sysfs_value_by_parameter(data, DADTransmits, dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "DADTransmits", "1"));
				set_sysfs_value_by_parameter(data, RetransTimer, dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RetransTimer", "1000"));

                                char *value_sec=NULL;
                                dmasprintf(&value_sec, "%u", (int)(atoi(dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RtrSolicitationInterval", "4000"))/1000));
				set_sysfs_value_by_parameter(data, RtrSolicitationInterval, value_sec);
				set_sysfs_value_by_parameter(data, MaxRtrSolicitations, dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "MaxRtrSolicitations", "3"));

				char *NUDEnable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "NUDEnable", "1");
				if(NUDEnable[0] == '1'){
					char *tmpvalue=NULL;
					get_sysfs_value(get_config_path(NULL, UnicastSolicit, SYSFS_DEF_PATH), &tmpvalue);
					set_sysfs_value_by_parameter(data, UnicastSolicit, tmpvalue);
					get_sysfs_value(get_config_path(NULL, McastSolicit, SYSFS_DEF_PATH), &tmpvalue);
					set_sysfs_value_by_parameter(data, McastSolicit, tmpvalue);
				}else{
					set_sysfs_value_by_parameter(data, UnicastSolicit, "0");
					set_sysfs_value_by_parameter(data, McastSolicit, "0");
				}
			}else{
				// Reset to default value for disable
				char *tmpvalue=NULL;
				//get_sysfs_value(get_config_path(NULL, DADTransmits, SYSFS_DEF_PATH), &tmpvalue);
				//set_sysfs_value_by_parameter(data, DADTransmits, tmpvalue);

				get_sysfs_value(get_config_path(NULL, RetransTimer, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, RetransTimer, tmpvalue);

				get_sysfs_value(get_config_path(NULL, RtrSolicitationInterval, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, RtrSolicitationInterval, tmpvalue);

				get_sysfs_value(get_config_path(NULL, MaxRtrSolicitations, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, MaxRtrSolicitations, tmpvalue);

				get_sysfs_value(get_config_path(NULL, UnicastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, UnicastSolicit, tmpvalue);

				get_sysfs_value(get_config_path(NULL, McastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, McastSolicit, tmpvalue);
			}
			return 0;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.Status!UCI:dmmap_ndisc/interface,@i-1/interface*/
static int get_NeighborDiscovery_IntfSetting_Status(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *disable_ipv6=NULL;
	get_IP_IPv6Capable(((struct ndisc_intfsetting_args *)data)->iface_s, &disable_ipv6);
	if (!DM_LSTRCMP(disable_ipv6, "1")){
        	*value = "Error_Misconfigured";
	}else{
		get_NeighborDiscovery_IntfSetting_Enable(refparam,ctx,data,instance, value);
        	*value = ((*value)[0] == '1') ? "Enabled" : "Disabled";
	}
        return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.Alias!UCI:dmmap_ndisc/interface,@i-1/ndisc_intf_alias*/
static int get_NeighborDiscovery_IntfSetting_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct ndisc_intfsetting_args *)data)->dmmap_s, "ndisc_intf_alias", value);
	if ((*value)[0] == '\0')
		dmasprintf(value, "cpe-%s", instance);
        return 0;
}

static int set_NeighborDiscovery_IntfSetting_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_string(value, -1, 64, NULL, NULL))
				return FAULT_9007;
			return 0;
		case VALUESET:
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "ndisc_intf_alias", value);
			return 0;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.Interface!UCI:dmmap_ndisc/interface,@i-1/iface_name*/
static int get_NeighborDiscovery_IntfSetting_Interface(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *linker = NULL;
	dmuci_get_value_by_section_string(((struct ndisc_intfsetting_args *)data)->dmmap_s, "iface_name", &linker);
	adm_entry_get_linker_param(ctx, "Device.IP.Interface.", linker, value);
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_Interface(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	char *allowed_objects[] = {"Device.IP.Interface.", NULL};
	char *linker = NULL;

	switch (action) {
		case VALUECHECK:
			if (dm_validate_string(value, -1, 256, NULL, NULL))
				return FAULT_9007;

			if (dm_entry_validate_allowed_objects(ctx, value, allowed_objects))
				return FAULT_9007;
			break;
		case VALUESET:
			adm_entry_get_linker_value(ctx, value, &linker);
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "iface_name", linker ? linker : "");
			break;
	}
	return 0;
}

#if 0
/*#Device.NeighborDiscovery.InterfaceSetting.{i}.DADTransmits!UCI:dmmap_ndisc/interface,@i-1/dad_transmits*/
static int get_NeighborDiscovery_IntfSetting_DADTransmits(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "DADTransmits", "1");
	//get_sysfs_value_by_parameter(data, DADTransmits, value);
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_DADTransmits(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_unsignedInt(value, RANGE_ARGS{{"1",NULL}}, 1))
				return FAULT_9007;
                        break;
		case VALUESET:
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "DADTransmits", value);
			char *enable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", "0");
			if(enable[0]=='1'){
				set_sysfs_value_by_parameter(data, DADTransmits, value);
			}
			break;
	}
	return 0;
}
#endif

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.RetransTimer!UCI:dmmap_ndisc/interface,@i-1/retrans_time_ms*/
static int get_NeighborDiscovery_IntfSetting_RetransTimer(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RetransTimer", "1000");
	//get_sysfs_value_by_parameter(data, RetransTimer, value);
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_RetransTimer(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_unsignedInt(value, RANGE_ARGS{{"10",NULL}}, 1))
				return FAULT_9007;
                        break;
		case VALUESET:
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RetransTimer", value);
			char *enable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", "0");
			if(enable[0]=='1'){
				set_sysfs_value_by_parameter(data, RetransTimer, value);
			}
			break;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.RtrSolicitationInterval!UCI:dmmap_ndisc/interface,@i-1/router_solicitation_interval_ms*/
static int get_NeighborDiscovery_IntfSetting_RtrSolicitationInterval(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RtrSolicitationInterval", "4000");
	//get_sysfs_value_by_parameter(data, RtrSolicitationInterval, value);
	//dmasprintf(value, "%d", atoi(*value)*1000);
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_RtrSolicitationInterval(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_unsignedInt(value, RANGE_ARGS{{"4000",NULL}}, 1))
				return FAULT_9007;
                        break;
		case VALUESET:
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RtrSolicitationInterval", value);
			char *enable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", "0");
			if(enable[0]=='1'){
				char *value_sec=NULL;
				dmasprintf(&value_sec, "%u", (int)(atoi(value)/1000));
				set_sysfs_value_by_parameter(data, RtrSolicitationInterval, value_sec);
			}
			break;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.MaxRtrSolicitations!UCI:dmmap_ndisc/interface,@i-1/router_solicitations*/
static int get_NeighborDiscovery_IntfSetting_MaxRtrSolicitations(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "MaxRtrSolicitations", "3");
	//get_sysfs_value_by_parameter(data, MaxRtrSolicitations, value);
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_MaxRtrSolicitations(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_unsignedInt(value, RANGE_ARGS{{NULL,NULL}}, 1))
				return FAULT_9007;
                        break;
		case VALUESET:
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "MaxRtrSolicitations", value);
			char *enable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "Enable", "0");
			if(enable[0]=='1'){
				char *RSEnable = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RSEnable", "1");
				if(RSEnable[0] == '1'){
					set_sysfs_value_by_parameter(data, RtrSolicitationInterval, value);
				}
			}
			break;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.NUDEnable!UCI:dmmap_ndisc/interface,@i-1/NUDEnable*/
static int get_NeighborDiscovery_IntfSetting_NUDEnable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "NUDEnable", "1");
	*value = ((*value)[0] == '0') ? "0" : "1";
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_NUDEnable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	bool b;
	switch (action) {
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			return 0;
		case VALUESET:
			string_to_bool(value, &b);
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "NUDEnable", b ? "1" : "0");
			if(b){
				char *tmpvalue=NULL;
				get_sysfs_value(get_config_path(NULL, UnicastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, UnicastSolicit, tmpvalue);
				get_sysfs_value(get_config_path(NULL, McastSolicit, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, McastSolicit, tmpvalue);
			}else{
				set_sysfs_value_by_parameter(data, UnicastSolicit, "0");
				set_sysfs_value_by_parameter(data, McastSolicit, "0");
			}
			return 0;
	}
	return 0;
}

/*#Device.NeighborDiscovery.InterfaceSetting.{i}.RSEnable!UCI:dmmap_ndisc/interface,@i-1/RSEnable*/
static int get_NeighborDiscovery_IntfSetting_RSEnable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RSEnable", "1");
	*value = ((*value)[0] == '0') ? "0" : "1";
	return 0;
}

static int set_NeighborDiscovery_IntfSetting_RSEnable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	bool b;
	switch (action) {
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			return 0;
		case VALUESET:
			string_to_bool(value, &b);
			dmuci_set_value_by_section(((struct ndisc_intfsetting_args *)data)->dmmap_s, "RSEnable", b ? "1" : "0");
			if(b){
				set_sysfs_value_by_parameter(data, MaxRtrSolicitations, dmuci_get_value_by_section_fallback_def(((struct ndisc_intfsetting_args *)data)->dmmap_s, "MaxRtrSolicitations", "3"));
			}else{
				char *tmpvalue=NULL;
				get_sysfs_value(get_config_path(NULL, MaxRtrSolicitations, SYSFS_DEF_PATH), &tmpvalue);
				set_sysfs_value_by_parameter(data, MaxRtrSolicitations, tmpvalue);
			}
			return 0;
	}
	return 0;
}

/**********************************************************************************************************************************
*                                            OBJ & LEAF DEFINITION
***********************************************************************************************************************************/
/* *** Device.NeighborDiscovery. *** */
DMOBJ tNeighborDiscoveryObj[] = {
/* OBJ, permission, addobj, delobj, checkdep, browseinstobj, nextdynamicobj, dynamicleaf, nextobj, leaf, linker, bbfdm_type, uniqueKeys, version*/
{"InterfaceSetting", &DMREAD, NULL, NULL, NULL, browseNeighborDiscoveryInterfaceSettingInst, NULL, NULL, NULL, tNeighborDiscoveryInterfaceSettingParams, NULL, BBFDM_BOTH, NULL, "2.12"},
{0}
};

DMLEAF tNeighborDiscoveryParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"Enable", &DMWRITE, DMT_BOOL, get_NeighborDiscovery_enable, set_NeighborDiscovery_enable, BBFDM_BOTH, "2.12"},
{"InterfaceSettingNumberOfEntries", &DMREAD, DMT_UNINT, get_NeighborDiscovery_InterfaceSettingNumberOfEntries, NULL, BBFDM_BOTH, "2.12"},
{0}
};

DMLEAF tNeighborDiscoveryInterfaceSettingParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"Enable", &DMWRITE, DMT_BOOL, get_NeighborDiscovery_IntfSetting_Enable, set_NeighborDiscovery_IntfSetting_Enable, BBFDM_BOTH, "2.12"},
{"Status", &DMREAD, DMT_STRING, get_NeighborDiscovery_IntfSetting_Status, NULL, BBFDM_BOTH, "2.12"},
{"Alias", &DMWRITE, DMT_STRING, get_NeighborDiscovery_IntfSetting_Alias, set_NeighborDiscovery_IntfSetting_Alias, BBFDM_BOTH, "2.12"},
{"Interface", &DMWRITE, DMT_STRING, get_NeighborDiscovery_IntfSetting_Interface, set_NeighborDiscovery_IntfSetting_Interface, BBFDM_BOTH, "2.12"},
//{"DADTransmits", &DMWRITE, DMT_UNINT, get_NeighborDiscovery_IntfSetting_DADTransmits, set_NeighborDiscovery_IntfSetting_DADTransmits, BBFDM_BOTH, "2.12"},
{"RetransTimer", &DMWRITE, DMT_UNINT, get_NeighborDiscovery_IntfSetting_RetransTimer, set_NeighborDiscovery_IntfSetting_RetransTimer, BBFDM_BOTH, "2.12"},
{"RtrSolicitationInterval", &DMWRITE, DMT_UNINT, get_NeighborDiscovery_IntfSetting_RtrSolicitationInterval, set_NeighborDiscovery_IntfSetting_RtrSolicitationInterval, BBFDM_BOTH, "2.12"},
{"MaxRtrSolicitations", &DMWRITE, DMT_UNINT, get_NeighborDiscovery_IntfSetting_MaxRtrSolicitations, set_NeighborDiscovery_IntfSetting_MaxRtrSolicitations, BBFDM_BOTH, "2.12"},
{"NUDEnable", &DMWRITE, DMT_BOOL, get_NeighborDiscovery_IntfSetting_NUDEnable, set_NeighborDiscovery_IntfSetting_NUDEnable, BBFDM_BOTH, "2.12"},
{"RSEnable", &DMWRITE, DMT_BOOL, get_NeighborDiscovery_IntfSetting_RSEnable, set_NeighborDiscovery_IntfSetting_RSEnable, BBFDM_BOTH, "2.12"},
{0}
};
