#
# Copyright (C) 2014 Realtek Semiconductor Corp. 
# All Rights Reserved.
#
# This program is the proprietary software of Realtek Semiconductor
# Corporation and/or its licensors, and only be used, duplicated, 
# modified or distributed under the authorized license from Realtek. 
#
# ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
# THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
#
# Purpose : Compile procedure of OMCI
#
# Feature : The file includes the following modules and sub-modules
#           (1) Makefile
#

RELEASE_VERSION=1.9.1.8.1.9.1.8.1.9.1.8


MIB_DIR  		= MIB
FEATURE_DIR     = FAL/src/features
VOIP_RTK_DIR    = VOIP/src/rtk
BIN_LIB_TABLES_DIR	:= LIB/omci
DRV_RG_DIR		= DRV/platform/rtl9607_rg
ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
DRV_RG_DIR		= DRV/platform/rtl9607_fc
endif
DRV_RTK_DIR		= DRV/platform/rtl9607_rtk
# library
LIB_GOS			= omci_gos
BIN_LIB_GOS		= lib$(LIB_GOS).so
LIB_MIB			= omci_mib
BIN_LIB_MIB		= lib$(LIB_MIB).so
LIB_OMCI_API    = omci_api
BIN_LIB_OMCI_API= lib$(LIB_OMCI_API).so
LIB_FAL         = omci_fal
BIN_LIB_FAL     = lib$(LIB_FAL).so
LIB_VOICE		= omci_voice
BIN_LIB_VOICE	= lib$(LIB_VOICE).so

# library list

# binary
BIN_OMCI_APP	= omci_app
BIN_OMCI_CLI	= omcicli
BIN_OMCI_DRV	= omcidrv.ko
BIN_PF_RTK		= pf_rtk.ko
BIN_PF_RG		= pf_rg.ko
ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
BIN_PF_RG		= pf_fc.ko
endif
# initial script
RUN_OMCI_SCRIPT = "runomci.sh"

all: clean TGT_DRV_RG install

TGT_DRV_RG:
ifeq ($(CONFIG_APOLLO_ROMEDRIVER),y)
	@if [ -f ${DRV_RG_DIR}/omci_pf_rtl9607.c ] ; then \
		$(MAKE) -C $(DRV_RG_DIR) all; \
		cp -af ${DRV_RG_DIR}/$(BIN_PF_RG) ${DRV_RG_DIR}/$(BIN_PF_RG)_$(RELEASE_VERSION); \
		$(MAKE) -C $(DRV_RG_DIR) clean; \
	fi
endif

ifeq ($(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE),y)
	@if [ -f ${DRV_RG_DIR}/omci_pf_rtl9607.c ] ; then \
		$(MAKE) -C $(DRV_RG_DIR) all; \
		cp -af ${DRV_RG_DIR}/$(BIN_PF_RG) ${DRV_RG_DIR}/$(BIN_PF_RG)_$(RELEASE_VERSION); \
		$(MAKE) -C $(DRV_RG_DIR) clean; \
	fi
endif

clean:
	rm -rf $(APOLLODIR)/object/src/app/lib/$(BIN_LIB_FAL) $(APOLLODIR)/object/src/app/lib/$(BIN_LIB_OMCI_API) $(APOLLODIR)/object/src/app/lib/$(BIN_LIB_VOICE)
	rm -rf $(ROMFSDIR)/bin/$(BIN_OMCI_APP) $(ROMFSDIR)/bin/$(BIN_OMCI_CLI) $(ROMFSDIR)/lib/omci
	rm -rf $(ROMFSDIR)/lib/$(BIN_LIB_GOS) $(ROMFSDIR)/lib/$(BIN_LIB_MIB) $(ROMFSDIR)/lib/$(BIN_LIB_FAL) $(ROMFSDIR)/lib/$(BIN_LIB_VOICE) $(ROMFSDIR)/lib/$(BIN_LIB_OMCI_API)
	rm -rf $(ROMFSDIR)/lib/features/internal $(ROMFSDIR)/lib/voip $(ROMFSDIR)/lib/modules/features $(ROMFSDIR)/lib/modules/$(BIN_PF_RG) $(ROMFSDIR)/lib/modules/$(BIN_PF_RTK)
	rm -rf $(ROMFSDIR)/lib/modules/$(BIN_OMCI_DRV)

install:
	@if [ ! -d $(ROMFSDIR)/bin ] ; then \
		mkdir -p $(ROMFSDIR)/bin; \
	fi
	@if [ -f "$(BIN_OMCI_APP).$(RELEASE_VERSION)" ] ; then \
		cp -af $(BIN_OMCI_APP).$(RELEASE_VERSION) $(ROMFSDIR)/bin/$(BIN_OMCI_APP); \
		cp -af $(BIN_OMCI_CLI).$(RELEASE_VERSION) $(ROMFSDIR)/bin/$(BIN_OMCI_CLI); \
	fi
	@if [ ! -d $(ROMFSDIR)/lib/omci ] ; then \
		mkdir -p $(ROMFSDIR)/lib/omci; \
	fi
	@if [ -f "$(BIN_LIB_GOS).$(RELEASE_VERSION)" ] ; then \
		find $(MIB_DIR)/src/tables/ -name "*.$(RELEASE_VERSION)" | xargs -i cp {} $(ROMFSDIR)/lib/omci/ ;\
		find $(ROMFSDIR)/lib/omci/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv ;\
		$(STRIP) $(ROMFSDIR)/lib/omci/*.so; \
		cp -af $(BIN_LIB_GOS).$(RELEASE_VERSION) $(ROMFSDIR)/lib/$(BIN_LIB_GOS); \
		cp -af $(BIN_LIB_MIB).$(RELEASE_VERSION) $(ROMFSDIR)/lib/$(BIN_LIB_MIB); \
		cp -af $(BIN_LIB_FAL).$(RELEASE_VERSION) $(APOLLODIR)/object/src/app/lib/; \
		cp -af $(BIN_LIB_FAL).$(RELEASE_VERSION) $(ROMFSDIR)/lib/$(BIN_LIB_FAL); \
		cp -af $(BIN_LIB_VOICE).$(RELEASE_VERSION)  $(APOLLODIR)/object/src/app/lib/; \
		cp -af $(BIN_LIB_VOICE).$(RELEASE_VERSION) $(ROMFSDIR)/lib/$(BIN_LIB_VOICE); \
		cp -af $(BIN_LIB_OMCI_API).$(RELEASE_VERSION) $(APOLLODIR)/object/src/app/lib/; \
		cp -af $(BIN_LIB_OMCI_API).$(RELEASE_VERSION) $(ROMFSDIR)/lib/$(BIN_LIB_OMCI_API); \
		find $(APOLLODIR)/object/src/app/lib/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv; \
	fi
	@if [ ! -d $(ROMFSDIR)/lib/features/internal ] ; then \
        mkdir -p $(ROMFSDIR)/lib/features/internal; \
    fi
	find $(FEATURE_DIR) -name "*.$(RELEASE_VERSION)" | xargs --no-run-if-empty -i cp {} $(ROMFSDIR)/lib/features/internal/
	find $(ROMFSDIR)/lib/features/internal/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv
	@if [ ! -d $(ROMFSDIR)/lib/voip ] ; then \
		mkdir -p $(ROMFSDIR)/lib/voip; \
	fi
	find $(VOIP_RTK_DIR) -name "*.$(RELEASE_VERSION)" | xargs --no-run-if-empty -i cp {} $(ROMFSDIR)/lib/voip/
	find $(ROMFSDIR)/lib/voip/ -name "*.$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv
	@if [ ! -d $(ROMFSDIR)/lib/modules/features ] ; then \
		mkdir -p $(ROMFSDIR)/lib/modules/features; \
	fi
	@if [ -f DRV/$(BIN_OMCI_DRV)_$(RELEASE_VERSION) ] ; then \
		cp -af DRV/$(BIN_OMCI_DRV)_$(RELEASE_VERSION) $(ROMFSDIR)/lib/modules/$(BIN_OMCI_DRV); \
		if [ -d $(DRV_RG_DIR) ] ; then \
			cp -af $(DRV_RG_DIR)/$(BIN_PF_RG)_$(RELEASE_VERSION) $(ROMFSDIR)/lib/modules/$(BIN_PF_RG); \
		fi; \
		if [ -d $(DRV_RTK_DIR) ] ; then \
			cp -af $(DRV_RTK_DIR)/$(BIN_PF_RTK)_$(RELEASE_VERSION)  $(ROMFSDIR)/lib/modules/$(BIN_PF_RTK); \
		fi; \
	fi
	find DRV/features/ -name "*.ko_$(RELEASE_VERSION)" | xargs --no-run-if-empty -i cp {} $(ROMFSDIR)/lib/modules/features/
	find $(ROMFSDIR)/lib/modules/features/ -name "*.ko_$(RELEASE_VERSION)" | sed -e p -e "s/.$(RELEASE_VERSION)//g" | xargs --no-run-if-empty -n2 mv
	@if [ ! -d $(ROMFSDIR)/etc ] ; then \
		mkdir -p $(ROMFSDIR)/etc; \
	fi
	cp -af $(RUN_OMCI_SCRIPT)  $(ROMFSDIR)/etc/

