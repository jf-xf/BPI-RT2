/*
 * Copyright (C) 2021 iopsys Software Solutions AB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 *	Author: <Name> <Surname> <name.surname@iopsys.eu>
 */

#include "optical.h"

/**************************************************************************
* LINKER
***************************************************************************/
static int get_optical_linker(char *refparam, struct dmctx *dmctx, void *data, char *instance, char **linker)
{
	*linker = "veip0";
	return 0;
}

/*************************************************************
* ENTRY METHOD
**************************************************************/
static int dmmap_synchronizeOpticalInterface(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct uci_section *s = NULL;

	if (!is_dmmap_section_exist_eq("dmmap", "optical", "name", "nas0")) {
		dmuci_add_section_bbfdm("dmmap", "optical", &s);
		dmuci_set_value_by_section(s, "name", "nas0");
		dmuci_set_value_by_section(s, "optical_instance", "1");
	}

	return 0;
}

static int browseOpticalInterfaceInst(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	struct uci_section *s = NULL;
	char *inst = NULL;

	dmmap_synchronizeOpticalInterface(dmctx, parent_node, prev_data, prev_instance);
	uci_path_foreach_sections(bbfdm, "dmmap", "optical", s) {
		inst = handle_instance(dmctx, parent_node, s, "optical_instance", "optical_alias");
		if (DM_LINK_INST_OBJ(dmctx, parent_node, (void *)s, inst) == DM_STOP)
			break;
	}
	return 0;
}

/*************************************************************
* GET & SET PARAM
**************************************************************/
static int pon_statistics_ubus(const char *name, char **value)
{
	json_object *res = NULL;

	dmubus_call("rtk-rpc", "get_pon_statistics", UBUS_ARGS{0}, 0, &res);
	DM_ASSERT(res, *value = "0");
	*value = dmjson_get_value(res, 1, name);
	return 0;
}

static int get_Optical_InterfaceNumberOfEntries(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	int cnt = get_number_of_entries(ctx, data, instance, browseOpticalInterfaceInst);
	dmasprintf(value, "%d", cnt);
	return 0;
}

static int get_OpticalInterface_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = "1";
	return 0;
}

static int set_OpticalInterface_Enable(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action)	{
		case VALUECHECK:
			if (dm_validate_boolean(value))
				return FAULT_9007;
			break;
		case VALUESET:
			//TODO
			break;
	}
	return 0;
}

static int get_OpticalInterface_Status(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	char *device_name  = NULL;
	dmuci_get_value_by_section_string((struct uci_section *)data, "name", &device_name);
	return get_net_device_status(device_name, value);
}

static int get_OpticalInterface_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string((struct uci_section *)data, "optical_alias", value);
	if ((*value)[0] == '\0')
		dmasprintf(value, "cpe-%s", instance);
	return 0;
}

static int set_OpticalInterface_Alias(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action)	{
		case VALUECHECK:
			if (dm_validate_string(value, -1, 64, NULL, NULL))
				return FAULT_9007;
			break;
		case VALUESET:
			dmuci_set_value_by_section((struct uci_section *)data, "optical_alias", value);
			break;
	}
	return 0;
}

static int get_OpticalInterface_Name(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	dmuci_get_value_by_section_string((struct uci_section *)data, "name", value);
	return 0;
}

static int get_OpticalInterface_LastChange(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	json_object *res = NULL;
	char *result=NULL, *pon_mode=NULL;
	dmubus_call("rtk-rpc", "mib_get", UBUS_ARGS{{"name", "PON_MODE", String}}, 1, &res);

	result = dmjson_get_value(res, 1, "result");
	if (result && !DM_LSTRCMP(result, "1")) {
		pon_mode = dmjson_get_value(res, 1, "value");
		if (pon_mode && !DM_LSTRCMP(pon_mode, "0")) {
			// Ethernet WAN
			*value = "0";
			return 0;
		}
	}

	int i = 0;
	char out[2048] = {0};
	FILE *fp = popen("/bin/omcicli get authuptime", "r");
	if (fp == NULL) {
		*value = "0";
		return 0;
	}

	while (fgets(out, sizeof(out) - 1, fp) != NULL) {
		char *p = strtok(out, " ");
		i = 0;
		while (p) {
			strip_lead_trail_whitespace(p);

			if (i == 4) {
				long i_val = 0;
				char *endval = NULL;
				char *dot = NULL;

				if ((dot = strchr(p, '.')) != NULL)
					*dot = '\0';

				i_val = strtol(p, &endval, 10);

				if (*endval != 0) {
					*value = "0";
				}
				else {
					dmasprintf(value, "%ld", i_val);
				}

				break;
			}

			p = strtok(NULL, " ");
			i++;
		}

		if ((*value)[0] != '\0')
			break;
	}

	pclose(fp);

	if ((*value)[0] == '\0')
		*value = "0";

	return 0;
}

static int get_OpticalInterface_LowerLayers(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = "";
	return 0;
}

static int set_OpticalInterface_LowerLayers(char *refparam, struct dmctx *ctx, void *data, char *instance, char *value, int action)
{
	switch (action)	{
		case VALUECHECK:
			if (dm_validate_string_list(value, -1, -1, 1024, -1, -1, NULL, NULL))
				return FAULT_9007;
			break;
		case VALUESET:
			break;
	}
	return 0;
}

static int get_OpticalInterface_Upstream(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	*value = "1";
	return 0;
}

static int get_OpticalInterface_OpticalSignalLevel(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	json_object *res = NULL;
	char *result = NULL, *dBm = NULL;

	dmubus_call("rtk-rpc", "get_ponmisc_transceiver", UBUS_ARGS{{"name", "rx-power", String}}, 1, &res);
	DM_ASSERT(res, *value = "-65536");

	result = dmjson_get_value(res, 1, "result");
	if (result && DM_LSTRCMP(result, "1") == 0) {
		dBm = dmjson_get_value(res, 1, "value");
		if (dBm && DM_LSTRCMP(dBm, "-inf") != 0) {
			char *p = strtok(dBm, " ");

			if (p) {
				char *endval = NULL;
				double val = 0;

				strip_lead_trail_whitespace(p);
				val = strtod(p, &endval);
				dmasprintf(value, "%d", (int)(val * 1000));
			}
		}
	}

	if ((*value)[0] == '\0')
		*value = "-65536";

	return 0;
}

/* This parameter was DEPRECATED in 2.15.
static int get_OpticalInterface_LowerOpticalThreshold(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return 0;
}
*/

/* This parameter was DEPRECATED in 2.15.
static int get_OpticalInterface_UpperOpticalThreshold(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return 0;
}
*/

static int get_OpticalInterface_TransmitOpticalLevel(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	json_object *res = NULL;
	char *result = NULL, *dBm = NULL;

	dmubus_call("rtk-rpc", "get_ponmisc_transceiver", UBUS_ARGS{{"name", "tx-power", String}}, 1, &res);
	DM_ASSERT(res, *value = "-65536");

	result = dmjson_get_value(res, 1, "result");
	if (result && DM_LSTRCMP(result, "1") == 0) {
		dBm = dmjson_get_value(res, 1, "value");
		if (dBm && DM_LSTRCMP(dBm, "-inf") != 0) {
			char *p = strtok(dBm, " ");

			if (p) {
				char *endval = NULL;
				double val = 0;

				strip_lead_trail_whitespace(p);
				val = strtod(p, &endval);
				dmasprintf(value, "%d", (int)(val * 1000));
			}
		}
	}

	if ((*value)[0] == '\0')
		*value = "-65536";

	return 0;
}


/* This parameter was DEPRECATED in 2.15.
static int get_OpticalInterface_LowerTransmitPowerThreshold(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return 0;
}
*/

/* This parameter was DEPRECATED in 2.15.
static int get_OpticalInterface_UpperTransmitPowerThreshold(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return 0;
}
*/

static int get_OpticalInterfaceStats_BytesSent(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("bytes_sent", value);
}

static int get_OpticalInterfaceStats_BytesReceived(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("bytes_received", value);
}

static int get_OpticalInterfaceStats_PacketsSent(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("packets_sent", value);
}

static int get_OpticalInterfaceStats_PacketsReceived(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("packets_received", value);
}

static int get_OpticalInterfaceStats_ErrorsSent(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("errors_packets_sent", value);
}

static int get_OpticalInterfaceStats_ErrorsReceived(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("errors_packets_received", value);
}

static int get_OpticalInterfaceStats_DiscardPacketsSent(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("packets_dropped", value);
}

static int get_OpticalInterfaceStats_DiscardPacketsReceived(char *refparam, struct dmctx *ctx, void *data, char *instance, char **value)
{
	return pon_statistics_ubus("packets_dropped_received", value);
}

/**********************************************************************************************************************************
*                                            OBJ & PARAM DEFINITION
***********************************************************************************************************************************/
/* *** Device.Optical. *** */
DMOBJ tOpticalObj[] = {
/* OBJ, permission, addobj, delobj, checkdep, browseinstobj, nextdynamicobj, dynamicleaf, nextobj, leaf, linker, bbfdm_type, uniqueKeys, version*/
{"Interface", &DMREAD, NULL, NULL, NULL, browseOpticalInterfaceInst, NULL, NULL, tOpticalInterfaceObj, tOpticalInterfaceParams, get_optical_linker, BBFDM_BOTH, LIST_KEY{"Alias", "Name", NULL}, "2.4"},
{0}
};

DMLEAF tOpticalParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"InterfaceNumberOfEntries", &DMREAD, DMT_UNINT, get_Optical_InterfaceNumberOfEntries, NULL, BBFDM_BOTH, "2.4"},
{0}
};

/* *** Device.Optical.Interface.{i}. *** */
DMOBJ tOpticalInterfaceObj[] = {
/* OBJ, permission, addobj, delobj, checkdep, browseinstobj, nextdynamicobj, dynamicleaf, nextobj, leaf, linker, bbfdm_type, uniqueKeys, version*/
{"Stats", &DMREAD, NULL, NULL, NULL, NULL, NULL, NULL, NULL, tOpticalInterfaceStatsParams, NULL, BBFDM_BOTH, NULL, "2.4"},
{0}
};

DMLEAF tOpticalInterfaceParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"Enable", &DMWRITE, DMT_BOOL, get_OpticalInterface_Enable, set_OpticalInterface_Enable, BBFDM_BOTH, "2.4"},
{"Status", &DMREAD, DMT_STRING, get_OpticalInterface_Status, NULL, BBFDM_BOTH, "2.4"},
{"Alias", &DMWRITE, DMT_STRING, get_OpticalInterface_Alias, set_OpticalInterface_Alias, BBFDM_BOTH, "2.4"},
{"Name", &DMREAD, DMT_STRING, get_OpticalInterface_Name, NULL, BBFDM_BOTH, "2.4"},
{"LastChange", &DMREAD, DMT_UNINT, get_OpticalInterface_LastChange, NULL, BBFDM_BOTH, "2.4"},
{"LowerLayers", &DMWRITE, DMT_STRING, get_OpticalInterface_LowerLayers, set_OpticalInterface_LowerLayers, BBFDM_BOTH, "2.4"},
{"Upstream", &DMREAD, DMT_BOOL, get_OpticalInterface_Upstream, NULL, BBFDM_BOTH, "2.4"},
{"OpticalSignalLevel", &DMREAD, DMT_INT, get_OpticalInterface_OpticalSignalLevel, NULL, BBFDM_BOTH, "2.4"},
//{"LowerOpticalThreshold", &DMREAD, DMT_INT, get_OpticalInterface_LowerOpticalThreshold, NULL, BBFDM_BOTH, "2.4"},
//{"UpperOpticalThreshold", &DMREAD, DMT_INT, get_OpticalInterface_UpperOpticalThreshold, NULL, BBFDM_BOTH, "2.4"},
{"TransmitOpticalLevel", &DMREAD, DMT_INT, get_OpticalInterface_TransmitOpticalLevel, NULL, BBFDM_BOTH, "2.4"},
//{"LowerTransmitPowerThreshold", &DMREAD, DMT_INT, get_OpticalInterface_LowerTransmitPowerThreshold, NULL, BBFDM_BOTH, "2.4"},
//{"UpperTransmitPowerThreshold", &DMREAD, DMT_INT, get_OpticalInterface_UpperTransmitPowerThreshold, NULL, BBFDM_BOTH, "2.4"},
{0}
};

/* *** Device.Optical.Interface.{i}.Stats. *** */
DMLEAF tOpticalInterfaceStatsParams[] = {
/* PARAM, permission, type, getvalue, setvalue, bbfdm_type, version*/
{"BytesSent", &DMREAD, DMT_UNLONG, get_OpticalInterfaceStats_BytesSent, NULL, BBFDM_BOTH, "2.4"},
{"BytesReceived", &DMREAD, DMT_UNLONG, get_OpticalInterfaceStats_BytesReceived, NULL, BBFDM_BOTH, "2.4"},
{"PacketsSent", &DMREAD, DMT_UNLONG, get_OpticalInterfaceStats_PacketsSent, NULL, BBFDM_BOTH, "2.4"},
{"PacketsReceived", &DMREAD, DMT_UNLONG, get_OpticalInterfaceStats_PacketsReceived, NULL, BBFDM_BOTH, "2.4"},
{"ErrorsSent", &DMREAD, DMT_UNINT, get_OpticalInterfaceStats_ErrorsSent, NULL, BBFDM_BOTH, "2.4"},
{"ErrorsReceived", &DMREAD, DMT_UNINT, get_OpticalInterfaceStats_ErrorsReceived, NULL, BBFDM_BOTH, "2.4"},
{"DiscardPacketsSent", &DMREAD, DMT_UNINT, get_OpticalInterfaceStats_DiscardPacketsSent, NULL, BBFDM_BOTH, "2.4"},
{"DiscardPacketsReceived", &DMREAD, DMT_UNINT, get_OpticalInterfaceStats_DiscardPacketsReceived, NULL, BBFDM_BOTH, "2.4"},
{0}
};

