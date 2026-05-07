DIR_RTL8367 = $(DIR_LINUX)/drivers/net/ethernet/realtek/rtl8367
$(shell ln -fs $(DIR_LINUX)/include/linux/string.h $(DIR_RTL8367)/string.h)

EXTRA_CFLAGS += -DCONFIG_SDK_KERNEL_LINUX
EXTRA_CFLAGS += -I$(KDIR)
EXTRA_CFLAGS += -I$(DIR_RTL8367)
EXTRA_CFLAGS += -DMDC_MDIO_OPERATION

ifeq ($(CONFIG_LUNA_G3_SERIES), y)
EXTRA_CFLAGS += -I$(ROOTDIR)/ca_packages/ca-plat/1.0-r0/ca-plat-1.0/inc \
		-I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/aal-gen2/include \
		-I$(ROOTDIR)/ca_packages/ca-network-engine/1.0-r0/ca-network-engine-1.0/cortina-api/include
else
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
endif

ifeq ($(CONFIG_EXTERNAL_SWITCH),y)
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/include
EXTRA_CFLAGS += -I$(APOLLODIR)/sdk/system/include
EXTRA_CFLAGS += -I$(DIR_RTL8367)/mapper
endif


ifeq ($(CONFIG_DAL_RTL8367C),y)
EXTRA_CFLAGS += -I$(DIR_RTL8367)/dal/rtl8367c
endif

ifeq ($(CONFIG_DAL_RTL8367D),y)
EXTRA_CFLAGS += -I$(DIR_RTL8367)/dal/rtl8367d
endif

MODULE_NAME = rtl8367
obj-$(CONFIG_RTL8367_MODULE) += $(MODULE_NAME).o
$(MODULE_NAME)-objs := \
	main.o \
	rtl8367_ioctl.o \
	acl.o \
	chip.o \
	cpu.o \
	dot1x.o \
	eee.o \
	i2c.o \
	igmp.o \
	interrupt.o \
	l2.o \
	leaky.o \
	led.o \
	mirror.o \
	oam.o \
	port.o \
	ptp.o \
	qos.o \
	rate.o \
	rldp.o \
	rtk_switch.o \
	stat.o \
	storm.o \
	svlan.o \
	trap.o \
	trunk.o \
	vlan.o \
	dal/dal_mgmt.o \
	leds-rtk-hw.o

ifeq ($(CONFIG_DAL_RTL8367C),y)
$(MODULE_NAME)-objs += \
	dal/rtl8367c/dal_rtl8367c_acl.o \
	dal/rtl8367c/dal_rtl8367c_cpu.o \
	dal/rtl8367c/dal_rtl8367c_dot1x.o \
	dal/rtl8367c/dal_rtl8367c_eee.o \
	dal/rtl8367c/dal_rtl8367c_i2c.o \
	dal/rtl8367c/dal_rtl8367c_igmp.o \
	dal/rtl8367c/dal_rtl8367c_interrupt.o \
	dal/rtl8367c/dal_rtl8367c_l2.o \
	dal/rtl8367c/dal_rtl8367c_leaky.o \
	dal/rtl8367c/dal_rtl8367c_led.o \
	dal/rtl8367c/dal_rtl8367c_mapper.o \
	dal/rtl8367c/dal_rtl8367c_mirror.o \
	dal/rtl8367c/dal_rtl8367c_oam.o \
	dal/rtl8367c/dal_rtl8367c_port.o \
	dal/rtl8367c/dal_rtl8367c_ptp.o \
	dal/rtl8367c/dal_rtl8367c_qos.o \
	dal/rtl8367c/dal_rtl8367c_rate.o \
	dal/rtl8367c/dal_rtl8367c_rldp.o \
	dal/rtl8367c/dal_rtl8367c_stat.o \
	dal/rtl8367c/dal_rtl8367c_storm.o \
	dal/rtl8367c/dal_rtl8367c_svlan.o \
	dal/rtl8367c/dal_rtl8367c_switch.o \
	dal/rtl8367c/dal_rtl8367c_trap.o \
	dal/rtl8367c/dal_rtl8367c_trunk.o \
	dal/rtl8367c/dal_rtl8367c_vlan.o \
	dal/rtl8367c/rtl8367c_asicdrv_acl.o \
	dal/rtl8367c/rtl8367c_asicdrv.o \
	dal/rtl8367c/rtl8367c_asicdrv_cputag.o \
	dal/rtl8367c/rtl8367c_asicdrv_dot1x.o \
	dal/rtl8367c/rtl8367c_asicdrv_eav.o \
	dal/rtl8367c/rtl8367c_asicdrv_eee.o \
	dal/rtl8367c/rtl8367c_asicdrv_fc.o \
	dal/rtl8367c/rtl8367c_asicdrv_green.o \
	dal/rtl8367c/rtl8367c_asicdrv_hsb.o \
	dal/rtl8367c/rtl8367c_asicdrv_i2c.o \
	dal/rtl8367c/rtl8367c_asicdrv_igmp.o \
	dal/rtl8367c/rtl8367c_asicdrv_inbwctrl.o \
	dal/rtl8367c/rtl8367c_asicdrv_interrupt.o \
	dal/rtl8367c/rtl8367c_asicdrv_led.o \
	dal/rtl8367c/rtl8367c_asicdrv_lut.o \
	dal/rtl8367c/rtl8367c_asicdrv_meter.o \
	dal/rtl8367c/rtl8367c_asicdrv_mib.o \
	dal/rtl8367c/rtl8367c_asicdrv_mirror.o \
	dal/rtl8367c/rtl8367c_asicdrv_misc.o \
	dal/rtl8367c/rtl8367c_asicdrv_oam.o \
	dal/rtl8367c/rtl8367c_asicdrv_phy.o \
	dal/rtl8367c/rtl8367c_asicdrv_port.o \
	dal/rtl8367c/rtl8367c_asicdrv_portIsolation.o \
	dal/rtl8367c/rtl8367c_asicdrv_qos.o \
	dal/rtl8367c/rtl8367c_asicdrv_rldp.o \
	dal/rtl8367c/rtl8367c_asicdrv_rma.o \
	dal/rtl8367c/rtl8367c_asicdrv_scheduling.o \
	dal/rtl8367c/rtl8367c_asicdrv_storm.o \
	dal/rtl8367c/rtl8367c_asicdrv_svlan.o \
	dal/rtl8367c/rtl8367c_asicdrv_trunking.o \
	dal/rtl8367c/rtl8367c_asicdrv_unknownMulticast.o \
	dal/rtl8367c/rtl8367c_asicdrv_vlan.o \
	dal/rtl8367c/rtl8367c_smi.o
endif

ifeq ($(CONFIG_DAL_RTL8367D),y)
$(MODULE_NAME)-objs += \
	dal/rtl8367d/dal_rtl8367d_acl.o \
	dal/rtl8367d/dal_rtl8367d_cpu.o \
	dal/rtl8367d/dal_rtl8367d_dot1x.o \
	dal/rtl8367d/dal_rtl8367d_eee.o \
	dal/rtl8367d/dal_rtl8367d_igmp.o \
	dal/rtl8367d/dal_rtl8367d_interrupt.o \
	dal/rtl8367d/dal_rtl8367d_l2.o \
	dal/rtl8367d/dal_rtl8367d_leaky.o \
	dal/rtl8367d/dal_rtl8367d_led.o \
	dal/rtl8367d/dal_rtl8367d_mapper.o \
	dal/rtl8367d/dal_rtl8367d_mirror.o \
	dal/rtl8367d/dal_rtl8367d_port.o \
	dal/rtl8367d/dal_rtl8367d_qos.o \
	dal/rtl8367d/dal_rtl8367d_rate.o \
	dal/rtl8367d/dal_rtl8367d_rldp.o \
	dal/rtl8367d/dal_rtl8367d_stat.o \
	dal/rtl8367d/dal_rtl8367d_storm.o \
	dal/rtl8367d/dal_rtl8367d_svlan.o \
	dal/rtl8367d/dal_rtl8367d_switch.o \
	dal/rtl8367d/dal_rtl8367d_trap.o \
	dal/rtl8367d/dal_rtl8367d_trunk.o \
	dal/rtl8367d/dal_rtl8367d_vlan.o \
	dal/rtl8367d/rtl8367d_asicdrv.o \
	dal/rtl8367d/rtl8367d_smi.o
endif


ifeq ($(CONFIG_EXTERNAL_SWITCH),y)
$(MODULE_NAME)-objs += \
	mapper/rtl8367_mapper.o \
	mapper/rtl8367_mapper_switch.o \
	mapper/rtl8367_mapper_port.o \
	mapper/rtl8367_mapper_stat.o \
	mapper/rtl8367_mapper_rate.o
endif
