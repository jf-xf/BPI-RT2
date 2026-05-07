
EXEC = igmpproxy
OBJS = mroute-api.o lib.o ifvc.o syslog.o timeout.o main.o kern.o

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#CONFIG_IP_IGMPV3=y
#endif
ifdef CONFIG_BOA_WEB_E8B_CH
CONFIG_IP_IGMPV3=n
endif
#ifeq ($(CONFIG_IP_IGMPV3),y)
CFLAGS += -D_SUPPORT_IGMPV3_
OBJS   += igmpv3.o
DEPH   += igmpproxy.h
#endif

#ap
ifeq ($(CONFIG_RTK_DEV_AP),y)
ifeq ($(CONFIG_RTL_MULTI_ETH_WAN),y)
CFLAGS += -D MULTICAST_LANWAN_BIND
ifdef CONFIG_RTK_LIB_MIB
CFLAGS += -I$(INCDIR)/include -I$(INCDIR)/include/rtk -include $(LINUXDIR)/include/generated/autoconf.h -include $(ROOTDIR)/autoconf.h
LDFLAGS += -Wl,-rpath-link=$(INCDIR)/lib:$(INCDIR)/usr/lib:$(INCDIR)/usr/local/lib -lmib
endif
endif
endif

CFLAGS += -Wall -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@))) $(LIBMIB)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o *.bak

