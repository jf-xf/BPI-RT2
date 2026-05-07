#
# Makefile for the Realtek NIC drivers.
#

obj-$(CONFIG_RTL8686NIC) += re8686_ext.o
obj-$(CONFIG_RTL865X_ETH_PRIV_SKB) += re_privskb.o
ifeq ($(CONFIG_RTK_NIC_RX_POOL),y)
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/nicPool/
endif
ifeq ($(CONFIG_RTL9607C_SERIES),y)
ifeq ($(CONFIG_RTL8686_SWITCH),y)
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/include
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/system/include
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/include/hal/chipdef/rtl9607c
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/romeDriver
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net
CFLAGS_re8686_rtl9607c.o += -DCONFIG_SDK_KERNEL_LINUX -D__KERNEL__
endif
ifeq ($(CONFIG_COMMON_RT_API),y)
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/system/include/
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/include/
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/sdk/include/rtk/rt/
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/rt/include/
ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
CFLAGS_re8686_rtl9607c.o += -I${shell pwd}/drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/
endif
endif
obj-$(CONFIG_RTL9607C_SERIES) += re8686_rtl9607c.o
obj-$(CONFIG_RTL8686_TEST) += re8686_rtl9607c_test.o
endif

