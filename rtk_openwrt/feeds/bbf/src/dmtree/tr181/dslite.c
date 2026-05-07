#include "dslite.h"

struct dslite_intfsetting_args
{
	struct uci_section *tunnel_s;
	struct uci_section *tunneled_s;
	struct uci_section *dmmap_s;
};

char *EndpointAssignmentPrecedenceType[] = {"DHCPv6", "Static", NULL};
char *EndpointAddressTypePrecedenceType[] = {"FQDN", "IPv6Address", NULL};

/*************************************************************
* INIT
**************************************************************/


/**************************************************************************
* LINKER
***************************************************************************/


/*************************************************************
* COMMON Functions
**************************************************************/
static bool is_dslite_intf_section_exist(char *sec_name)
{
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(bbfdm, "dmmap_dslite", "interface", "tunnel_name", sec_name, s) {
		return true;
	}

	return false;
}

static void dmmap_synchronizeDSLiteInterface(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct uci_section *s = NULL, *stmp = NULL;

	uci_path_foreach_sections_safe(bbfdm, "dmmap_dslite", "interface", stmp, s) {
		struct uci_section *tunnel_s = NULL;
		char *added_by_controller = NULL;
		char *tunnel_name = NULL;

		dmuci_get_value_by_section_string(s, "added_by_controller", &added_by_controller);
		if (DM_LSTRCMP(added_by_controller, "1") == 0)
			continue;

		dmuci_get_value_by_section_string(s, "tunnel_name", &tunnel_name);
		if (DM_STRLEN(tunnel_name))
			get_config_section_of_dmmap_section("network", "interface", tunnel_name, &tunnel_s);

		if (!tunnel_s)
			dmuci_delete_by_section(s, NULL, NULL);
	}

	uci_foreach_sections("network", "interface", s) {
		struct uci_section *dslite_s = NULL;
		char *proto = NULL;
		char *tunneled_name = NULL;

		dmuci_get_value_by_section_string(s, "proto", &proto);
		if (DM_LSTRCMP(proto, "dslite") != 0)
			continue;

		if (is_dslite_intf_section_exist(section_name(s)))
			continue;

		dmuci_add_section_bbfdm("dmmap_dslite", "interface", &dslite_s);
		dmuci_set_value_by_section(dslite_s, "tunnel_name", section_name(s));
		dmuci_get_value_by_section_string(s, "tunlink", &tunneled_name);
		if(DM_STRLEN(tunneled_name))
			dmuci_set_value_by_section(dslite_s, "tunneled_name", tunneled_name);
	}
}

/*************************************************************
* ENTRY METHOD
**************************************************************/
/*#Device.DSLite.InterfaceSetting.{i}.!UCI:network/interface/dmmap_dslite*/
static int browseDSLiteInterfaceSettingInst(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct dslite_intfsetting_args curr_dslite_intf_args = {0};
	struct uci_section *dmmap_s = NULL;
	char *inst = NULL;

	dmmap_synchronizeDSLiteInterface(dmctx, parent_node, prev_data, prev_instance);
	uci_path_foreach_sections(bbfdm, "dmmap_dslite", "interface", dmmap_s) {
		struct uci_section *tunnel_s = NULL, *tunneled_s;
		char *tunnel_name = NULL;
		char *tunneled_name = NULL;

		dmuci_get_value_by_section_string(dmmap_s, "tunnel_name", &tunnel_name);
		if (DM_STRLEN(tunnel_name))
			get_config_section_of_dmmap_section("network", "interface", tunnel_name, &tunnel_s);

		dmuci_get_value_by_section_string(dmmap_s, "tunneled_name", &tunneled_name);
		if (DM_STRLEN(tunneled_name))
			get_config_section_of_dmmap_section("network", "interface", tunneled_name, &tunneled_s);

		curr_dslite_intf_args.tunnel_s = tunnel_s;
		curr_dslite_intf_args.tunneled_s = tunneled_s;
		curr_dslite_intf_args.dmmap_s = dmmap_s;

		inst = handle_instance(dmctx, parent_node, dmmap_s, "dslite_intf_instance", "dslite_intf_alias");

		if (DM_LINK_INST_OBJ(dmctx, parent_node, (void *)&curr_dslite_intf_args, inst) == DM_STOP)
			break;
	}
	return 0;
}

/*************************************************************
* GET & SET PARAM
**************************************************************/
static int get_DSLite_enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = "1";
	return 0;
}

static int set_DSLite_enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
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

/*#Device.DSLite.InterfaceSettingNumberOfEntries!UCI:network/interface/*/
static int get_DSLite_InterfaceSettingNumberOfEntries(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	int cnt = get_number_of_entries(ctx, data, instance, browseDSLiteInterfaceSettingInst);
	dmasprintf(value, "%d", cnt);
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.Enable!UCI:network/interface,@i-1/iface_dslite*/
static int get_DSLite_IntfSetting_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->tunnel_s, "disabled", value);
	*value = ((*value)[0] == '1') ? "0" : "1";
	return 0;
}

static int set_DSLite_IntfSetting_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	bool b;
	switch (action) {
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			return 0;
		case VALUESET:
			string_to_bool(value, &b);
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->tunnel_s, "disabled", b ? "0" : "1");
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.Status!UCI:network/interface,@i-1/iface_dslite*/
static int get_DSLite_IntfSetting_Status(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	get_DSLite_IntfSetting_Enable(refparam,ctx,data,instance, value);
        *value = ((*value)[0] == '1') ? "Enabled" : "Disabled";
        return 0;
}


/*#Device.DSLite.InterfaceSetting.{i}.Alias!UCI:dmmap_dslite/interface,@i-1/dslite_intf_alias*/
static int get_DSLite_IntfSetting_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->dmmap_s, "dslite_intf_alias", value);
	if ((*value)[0] == '\0')
		dmasprintf(value, "cpe-%s", instance);
        return 0;
}

static int set_DSLite_IntfSetting_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_string(value, -1, 64, NULL, NULL))
				return FAULT_9007;
			return 0;
		case VALUESET:
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->dmmap_s, "dslite_intf_alias", value);
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.EndpointAssignmentPrecedence!UCI:network/interface,@i-1/dslite_mode*/
static int get_DSLite_IntfSetting_EndpointAssignmentPrecedence(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->tunneled_s, "dslite_mode", value);
	if (DM_LSTRCMP(*value, EndpointAssignmentPrecedenceType[1]) != 0)
		dmasprintf(value, "%s", EndpointAssignmentPrecedenceType[0]);
        return 0;
}

static int set_DSLite_IntfSetting_EndpointAssignmentPrecedence(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action) {
		case VALUECHECK:
			if (dm_validate_string(value, -1, -1, EndpointAssignmentPrecedenceType, NULL))
				return FAULT_9007;
			return 0;
		case VALUESET:
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->tunneled_s, "dslite_mode", value);
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.EndpointAddressTypePrecedence!UCI:network/interface,@i-1/dslite_mode*/
static int get_DSLite_IntfSetting_EndpointAddressTypePrecedence(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *EndpointAssignmentPrecedence=NULL;
	get_DSLite_IntfSetting_EndpointAssignmentPrecedence(refparam,ctx,data,instance, &EndpointAssignmentPrecedence);
	if (DM_LSTRCMP(EndpointAssignmentPrecedence, EndpointAssignmentPrecedenceType[0]) == 0){
		dmasprintf(value, "%s", EndpointAddressTypePrecedenceType[0]);
	}else{
		dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->dmmap_s, "EndpointAddressTypePrecedence", value);
		if (DM_LSTRCMP(*value, EndpointAddressTypePrecedenceType[1]) != 0)
			dmasprintf(value, "%s", EndpointAddressTypePrecedenceType[0]);
	}
        return 0;
}

static int set_DSLite_IntfSetting_EndpointAddressTypePrecedence(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	char *EndpointAssignmentPrecedence=NULL;
	switch (action) {
		case VALUECHECK:
			get_DSLite_IntfSetting_EndpointAssignmentPrecedence(refparam,ctx,data,instance, &EndpointAssignmentPrecedence);
			if (DM_LSTRCMP(EndpointAssignmentPrecedence, EndpointAssignmentPrecedenceType[1]) != 0)
				return FAULT_9007;
			if (dm_validate_string(value, -1, -1, EndpointAddressTypePrecedenceType, NULL))
				return FAULT_9007;
			return 0;
		case VALUESET:
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->dmmap_s, "EndpointAddressTypePrecedence", value);
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.EndpointAddressInUse!UCI:network/interface,@i-1/dslite_peeraddr*/
static int get_DSLite_IntfSetting_EndpointAddressInUse(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->tunnel_s, "peeraddr", value);
	if((*value)[0] != '\0'){
		char cmd[128]={0}, out[64]={0};
		snprintf(cmd, sizeof(cmd), "resolveip -6 %s", *value);
		FILE *fp = popen(cmd, "r");
		if (fp == NULL) {
			*value = "\0";
			return 0;
		}

		while (fgets(out, sizeof(out) - 1, fp) != NULL) {
			dmasprintf(value, "%s", out);
			break;
		}

		pclose(fp);
	}
        return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.EndpointName!UCI:network/interface,@i-1/dslite_peeraddr*/
static int get_DSLite_IntfSetting_EndpointName(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->tunnel_s, "peeraddr", value);
        return 0;
}

static int set_DSLite_IntfSetting_EndpointName(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	char *EndpointAssignmentPrecedence=NULL;
	switch (action) {
		case VALUECHECK:
			if (dm_validate_string(value, 1, 256, NULL, NULL))
				return FAULT_9007;
			return 0;
		case VALUESET:
			get_DSLite_IntfSetting_EndpointAssignmentPrecedence(refparam,ctx,data,instance, &EndpointAssignmentPrecedence);
			if (DM_LSTRCMP(EndpointAssignmentPrecedence, EndpointAssignmentPrecedenceType[1]) != 0)
				return FAULT_9007;
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->tunnel_s, "peeraddr", value);
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.EndpointAddress!UCI:network/interface,@i-1/dslite_peeraddr*/
static int get_DSLite_IntfSetting_EndpointAddress(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->tunnel_s, "peeraddr", value);
        return 0;
}

static int set_DSLite_IntfSetting_EndpointAddress(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	char *EndpointAssignmentPrecedence=NULL;
	switch (action) {
		case VALUECHECK:
			get_DSLite_IntfSetting_EndpointAssignmentPrecedence(refparam,ctx,data,instance, &EndpointAssignmentPrecedence);
			if (DM_LSTRCMP(EndpointAssignmentPrecedence, EndpointAssignmentPrecedenceType[1]) != 0)
				return FAULT_9007;
			if (dm_validate_string(value, -1, 45, NULL, IPAddress))
				return FAULT_9007;
			return 0;
		case VALUESET:
			dmuci_set_value_by_section(((struct dslite_intfsetting_args *)data)->tunnel_s, "peeraddr", value);
			return 0;
	}
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.Origin!UCI:network/interface,@i-1/dslite_mode*/
static int get_DSLite_IntfSetting_Origin(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	get_DSLite_IntfSetting_EndpointAssignmentPrecedence(refparam, ctx, data, instance, value);
        return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.TunnelInterface*/
static int get_DSLite_IntfSetting_TunnelInterface(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *linker = NULL;
	char *device = NULL;
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->dmmap_s, "tunnel_name", &device);
	linker = dmstrdup(device);
	adm_entry_get_linker_param(ctx, "Device.IP.Interface.", linker, value);
	return 0;
}

/*#Device.DSLite.InterfaceSetting.{i}.TunneledInterface*/
static int get_DSLite_IntfSetting_TunneledInterface(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *linker = NULL;
	char *device = NULL;
	dmuci_get_value_by_section_string(((struct dslite_intfsetting_args *)data)->dmmap_s, "tunneled_name", &device);
	linker = dmstrdup(device);
	adm_entry_get_linker_param(ctx, "Device.IP.Interface.", linker, value);
	return 0;
}


/**********************************************************************************************************************************
*                                            OBJ & LEAF DEFINITION
***********************************************************************************************************************************/
/* *** Device.DSLite. *** */
DMOBJ tDSLiteObj[] = {
/* OBJ, permission, addobj, delobj, checkdep, browseinstobj, nextdynamicobj, dynamicleaf, nextobj, leaf, linker, bbfdm_type, uniqueKeys, version*/
{"InterfaceSetting", &DMREAD, NULL, NULL, NULL, browseDSLiteInterfaceSettingInst, NULL, NULL, NULL, tDSLiteInterfaceSettingParams, NULL, BBFDM_BOTH, NULL, "2.2"},
{0}
};

DMLEAF tDSLiteParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"Enable", &DMWRITE, DMT_BOOL, get_DSLite_enable, set_DSLite_enable, BBFDM_BOTH, "2.2"},
{"InterfaceSettingNumberOfEntries", &DMREAD, DMT_UNINT, get_DSLite_InterfaceSettingNumberOfEntries, NULL, BBFDM_BOTH, "2.0"},
{0}
};

DMLEAF tDSLiteInterfaceSettingParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"Enable", &DMWRITE, DMT_BOOL, get_DSLite_IntfSetting_Enable, set_DSLite_IntfSetting_Enable, BBFDM_BOTH, "2.2"},
{"Status", &DMREAD, DMT_STRING, get_DSLite_IntfSetting_Status, NULL, BBFDM_BOTH, "2.2"},
{"Alias", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_Alias, set_DSLite_IntfSetting_Alias, BBFDM_BOTH, "2.2"},
{"EndpointAssignmentPrecedence", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_EndpointAssignmentPrecedence, set_DSLite_IntfSetting_EndpointAssignmentPrecedence, BBFDM_BOTH, "2.2"},
{"EndpointAddressTypePrecedence", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_EndpointAddressTypePrecedence, set_DSLite_IntfSetting_EndpointAddressTypePrecedence, BBFDM_BOTH, "2.2"},
{"EndpointAddressInUse", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_EndpointAddressInUse, NULL, BBFDM_BOTH, "2.2"},
{"EndpointName", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_EndpointName, set_DSLite_IntfSetting_EndpointName, BBFDM_BOTH, "2.2"},
{"EndpointAddress", &DMWRITE, DMT_STRING, get_DSLite_IntfSetting_EndpointAddress, set_DSLite_IntfSetting_EndpointAddress, BBFDM_BOTH, "2.2"},
{"Origin", &DMREAD, DMT_STRING, get_DSLite_IntfSetting_Origin, NULL, BBFDM_BOTH, "2.2"},
{"TunnelInterface", &DMREAD, DMT_STRING, get_DSLite_IntfSetting_TunnelInterface, NULL, BBFDM_BOTH, "2.2"},
{"TunneledInterface", &DMREAD, DMT_STRING, get_DSLite_IntfSetting_TunneledInterface, NULL, BBFDM_BOTH, "2.2"},
{0}
};
