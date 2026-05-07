-include $(ROOTDIR)/.config

EXEC :=
INST_EXEC :=

# Wlan driver Kconfig options that requires AX_UDPserver for MP
AX_OPTIONS := RTL8852AE
AX_OPTIONS += RTL8852CE
AX_OPTIONS += RTL8832BR
AX_OPTIONS += RTL8192XB

BUILD_AX := $(foreach opt,$(AX_OPTIONS),$(value CONFIG_$(opt)))
BUILD_AX := $(BUILD_AX)$(foreach opt,$(AX_OPTIONS),$(value CPTCFG_$(opt)))

BUILD_AX := $(strip $(BUILD_AX))

ifneq ($(BUILD_AX),)
EXEC += AX_UDPserver
endif

#$(info BUILD_AX is "$(BUILD_AX)")

# Wlan driver Kconfig options that requires 11N_UDPserver for MP
NON_AX_OPTIONS := RTL8192CD
NON_AX_OPTIONS += WLAN_8192FE
NON_AX_OPTIONS += USB_RTL8192SU_SOFTAP

BUILD_11N := $(foreach opt,$(NON_AX_OPTIONS),$(value CONFIG_$(opt)))
BUILD_11N := $(BUILD_11N)$(foreach opt,$(NON_AX_OPTIONS),$(value CPTCFG_$(opt)))

BUILD_11N := $(strip $(BUILD_11N))

#$(info BUILD_11N is "$(BUILD_11N)")

ifneq ($(BUILD_11N),)
EXEC += 11N_UDPserver
INST_EXEC += /etc/orf
INST_EXEC += /etc/irf
endif

#$(info EXEC is $(EXEC))

# Default, not possible?
EXEC ?= UDPserver

INST_EXEC += $(addprefix /bin/, $(EXEC))

#$(info INST_EXEC is $(INST_EXEC) )

all: $(EXEC)

$(EXEC): %: %.o
	@echo Building $@
	$(CC) $(LDFLAGS) -I$(ROOTDIR)/include -o $@ $< $(LDLIBS)

romfs:
	@for s in $(INST_EXEC); do \
		echo $(ROMFSINST) $${s} ;\
        	$(ROMFSINST) $${s} ; \
	done

clean:
	-rm -f $(EXEC) *.elf *.o
