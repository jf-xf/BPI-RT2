#include <rtl8367_mapper.h>
#include <rtl8367c_asicdrv.h>



dal_ext_switch_mapper_t      rtl8367_mapper=
{
    .switch_deviceInfo_get          = rtl8367_mapper_switch_deviceInfo_get,

    .port_link_get                  = rtl8367_mapper_port_link_get,
    .port_speedDuplex_get           = rtl8367_mapper_port_speedDuplex_get,

    .stat_port_reset                = rtl8367_mapper_stat_port_reset,
    .stat_port_getAll               = rtl8367_mapper_stat_port_getAll,

    .rate_portEgrBandwidthCtrlRate_get	= rtl8367_mapper_rate_portEgrBandwidthCtrlRate_get,
    .rate_portEgrBandwidthCtrlRate_set	= rtl8367_mapper_rate_portEgrBandwidthCtrlRate_set,
};





void rtl8367_mapper_init(void)
{
    rtl8367c_setAsicRegBit(0x13e0, 8, 1);

    dal_ext_switch_register_mapper(&rtl8367_mapper);

}

