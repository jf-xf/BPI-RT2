#
# Realtek Semiconductor Corp.
#
MAJOR_VERSION = 2
MINOR_VERSION = 12

EXEC = rtkbosa
OBJS = rtkbosa_utils.o realtek.o semtech.o uxfastic.o rtkbosa.o

RC_LEVEL_RTKBOSA=/etc/init.d/rc3

CFLAGS += -DRTKBOSA_MAJOR_VERSION=$(MAJOR_VERSION)
CFLAGS += -DRTKBOSA_MINOR_VERSION=$(MINOR_VERSION)

CFLAGS += -I./
CFLAGS += -I$(ROOTDIR)/user/boa/src/LINUX

LDFLAGS += -L./ -lm $(LIBRTK)
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
LDFLAGS += -lgsl -lgslcblas
endif
LDFLAGS += $(LIBMIB)
LDFLAGS += $(LIBVOIP)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

rcX:
	$(ROMFSINST) rtkbosa.sh /etc/scripts/rtkbosa.sh
ifeq ($(CONFIG_LUNA_G3_SERIES),y)
	$(ROMFSINST) -a "modprobe ca-rtk" $(RC_LEVEL_RTKBOSA)
endif
	$(ROMFSINST) -a "/bin/sh /etc/scripts/rtkbosa.sh" $(RC_LEVEL_RTKBOSA)

