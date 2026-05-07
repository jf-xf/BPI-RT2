
include $(LINUXDIR)/.config
include $(ROOTDIR)/.config

CFLAGS += -fPIC -I. -g
#CFLAGS += -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wnested-externs
#CFLAGS += -Wtraditional -Wshadow -Wconversion -Waggregate-return -Wmissing-prototypes
CFLAGS += -Wall -Werror
CFLAGS += -Wno-error=unused-result -Wno-error=unused-variable 
CFLAGS += -Wno-error=unused-parameter -Wno-error=unused-label
CFLAGS += -Wno-error=unused-function -Wno-error=unused-const-variable
CFLAGS += -Wno-error=format-truncation -Wno-error=missing-braces
CFLAGS += -Wno-error=pointer-sign -Wno-error=switch -Wno-error=comment
CFLAGS += -Wno-error=strict-aliasing -Wno-error=format-security
NEED_UNUSED_BUT_SET = $(shell $(CC) -dumpversion | gawk '{print $$1>=4.6?"1":"0"}')
ifeq ("$(NEED_UNUSED_BUT_SET)", "1")
  CFLAGS += -Wno-error=unused-but-set-variable
endif 
CFLAGS += -include $(LINUXDIR)/include/generated/autoconf.h
CFLAGS += -include $(ROOTDIR)/autoconf.h
CFLAGS += -I$(INCDIR)/include -I$(INCDIR)/usr/include 
LDFLAGS += -L./ -L$(INCDIR)/lib -Wl,-rpath-link=$(INCDIR)/lib:$(INCDIR)/usr/lib:$(INCDIR)/usr/local/lib 

DESTDIR:= $(ROMFSDIR) 

exec_y =
libs_y =

exec_y += startup
obj_startup = startup.o web_ctrl.o
LIBS_startup = -lmib

exec_y += mib_util
obj_mib_util = mib_util.o
LIBS_mib_util = -lmib

exec_y += wlan_cal
obj_wlan_cal = wlan_cal.o
LIBS_wlan_cal = -lmib

exec_y += rtk_rpc
obj_rtk_rpc = rtk_rpc.o
LIBS_rtk_rpc = -lmib -luci
ifeq ($(CONFIG_PACKAGE_libubus), y)
LIBS_rtk_rpc += -L$(INCDIR)/usr/lib -lubus -lubox -lblobmsg_json
endif

libs_y += libmib.so
obj_libmib = mib.o mib_sechma.o mib_xml.o mib_uci.o port.o wifi.o wifi_mib.o utility.c
ifeq ($(CONFIG_GPON_FEATURE), y)
obj_libmib += pon.o
endif
LIBS_libmib = -luci -lxml2
CFLAGS += -I$(INCDIR)/usr/include/libxml2

ifeq ($(CONFIG_PACKAGE_kmod-rtk_wifi6_ap), y)
CFLAGS += -DARP_LOOP_DETECTION
CFLAGS += -DCONFIG_RTK_NETLINK_ROUTE_API
CFLAGS += -DCONFIG_RTK_RPC_DYNAMIC_DEBUG
ifeq ($(CONFIG_WLAN_EVENT_INDICATE_GENL), y)
CFLAGS += -DCONFIG_RTK_GENERIC_NETLINK_API
endif
endif

ifeq ($(CONFIG_GPON_FEATURE), y)
  ifeq ($(CONFIG_RTK_OMCI_V1), y)
    CFLAGS                                          += -I$(APOLLODIR)/sdk/src/app/omci_v1/API -I$(APOLLODIR)/sdk/src/app/omci_v1/GOS/inc
    LDFLAGS                                         += -L$(APOLLODIR)/sdk/src/app/omci_v1/LIB -lomci_api
  endif
endif

ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
  CFLAGS                                            += -I$(APOLLODIR)/FleetConntrackDriver/include
  ifeq ($(CONFIG_COMMON_RT_API),y)
    CFLAGS                                          += -I$(APOLLODIR)/FleetConntrackDriver/rt/include
  endif
  obj_libmib                                       += fc_api.o
endif

ifdef CONFIG_RTK_IGMP_MLD_SNOOPING_MODULE
  CFLAGS += -I$(APOLLODIR)/igmpHookModule/rt_igmpHook/include
endif

ifeq ($(CONFIG_RTL8686_SWITCH),y)
  CFLAGS                                            += -I$(APOLLODIR)/sdk/include
  CFLAGS                                            += -I$(APOLLODIR)/sdk/include/common
  CFLAGS                                            += -I$(APOLLODIR)/sdk/include/rtk/rt
  CFLAGS                                            += -I$(APOLLODIR)/sdk/system/include
  CFLAGS                                            += -I$(APOLLODIR)/sdk/src/app/diag_rtk/include
  LDFLAGS                                           += -L$(APOLLODIR)/object/src/app/lib
  LIBS                                              += -lpthread -lrtk
endif

.PHONY: all install-dev install romfs clean rcX

all: $(libs_y) $(exec_y)

startup: $(obj_startup)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_startup)
	
libmib.so: $(obj_libmib)
	$(CC) $(LDFLAGS) -fPIC -shared -o $@ $^ $(LIBS) $(LIBS_libmib)
	
mib_util: $(obj_mib_util)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_mib_util)

wlan_cal: $(obj_wlan_cal)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_wlan_cal)
 
rtk_rpc: $(obj_rtk_rpc)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_rtk_rpc)

.c.o:
	$(CC) -M $(CFLAGS) $< > $@.d
	$(CC) $(CFLAGS) -c $< -o $@ 

install-dev: all
	mkdir -p $(DESTDIR)/include/rtk
	cp -f rtk_interface.h sockmark_define.h  $(DESTDIR)/include/rtk

install: all
	mkdir -p $(DESTDIR)/bin
	cp -af $(exec_y) $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/usr/lib
	cp -af $(libs_y) $(DESTDIR)/usr/lib
	ln -sf mib_util $(DESTDIR)/bin/mib
	ln -sf mib_util $(DESTDIR)/bin/flash

romfs: install

clean:
	-rm -f *.elf *.o *.d *.so $(libs_y) $(exec_y) 

rcX:

