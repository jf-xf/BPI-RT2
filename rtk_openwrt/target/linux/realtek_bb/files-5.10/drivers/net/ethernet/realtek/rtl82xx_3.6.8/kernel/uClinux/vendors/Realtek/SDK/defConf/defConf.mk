
SDK_CONFIG_DIR := kernel/uClinux/vendors/Realtek/SDK
DEFAULT_CONFIG_DIR := $(SDK_CONFIG_DIR)/defConf
-include $(DEFAULT_CONFIG_DIR)/defConf_rtk.mk
#
# To build by default config
#
PUBLIC_PROJECTS:=8380 8390 8380u 8380uextc 8380uextc_ovcpu 8390u 8380tk 8390tk 83xx 83xxu 83xxuextc 83xxuextc_ovcpu 8396msc 9300 9300u 9300extc 9300uextc 9300uextc_ovcpu n9300uextc 9310extc 9310uextc 93xxextc 93xxuextc 3chips 9300tk 9300uaq 9300kaq 9300nb 9300nbu 9310 9310u 9310nb 93xx 93xxu 4chips 9310sqa 9310sdn 9310usmp 9310smp phyOnly 9310k4smp 9310k4usmp 93xxk4 93xxk4u 4chipsk4
.PHONY : all-conditions  $(PUBLIC_PROJECTS) $(INTERNAL_PROJECTS)
all-conditions: $(PUBLIC_PROJECTS) $(INTERNAL_PROJECTS)

DEF_LINUX_KERNEL_PATH=$(shell cat kernel/uClinux/vendors/Realtek/SDK/defConf/$@/root.config|grep CONFIG_LINUXDIR|cut -d= -f 2)
SET_LINUX_KERNEL_PATH=$(shell cat $(KERNEL_DIR)/$(OS_TYPE)/.config|grep CONFIG_LINUXDIR|cut -d= -f 2)
MAKE_TIME=$(shell date '+%Y%m%d-%H%M%S')

$(PUBLIC_PROJECTS) $(INTERNAL_PROJECTS):
	@rm -f you_make_*
	@touch you_make_$@___$(MAKE_TIME)
ifdef save
	@cp $(KERNEL_DIR)/$(OS_TYPE)/.config $(DEFAULT_CONFIG_DIR)/$@/root.config
	@cp $(KERNEL_DIR)/$(OS_TYPE)/$(SET_LINUX_KERNEL_PATH)/.config $(DEFAULT_CONFIG_DIR)/$@/config.$(SET_LINUX_KERNEL_PATH)
	@cp $(SDK_DIR)/config/.config $(DEFAULT_CONFIG_DIR)/$@/sdk.config
	@cp $(LOADER_DIR)/.config $(DEFAULT_CONFIG_DIR)/$@/uboot.config

	@printf "$@ configuration files updated.\n"
else ifdef sdkconfig
	@cp $(DEFAULT_CONFIG_DIR)/$@/sdk.config $(SDK_DIR)/config/.config
	@yes "" | make -C $(SDK_DIR)/config config
else
	make -C $(KERNEL_DIR)/$(OS_TYPE) distclean
	@cp $(DEFAULT_CONFIG_DIR)/$@/root.config $(KERNEL_DIR)/$(OS_TYPE)/.config
	@cp $(DEFAULT_CONFIG_DIR)/$@/config.$(DEF_LINUX_KERNEL_PATH) $(SDK_CONFIG_DIR)/config.$(DEF_LINUX_KERNEL_PATH)
	@cp $(DEFAULT_CONFIG_DIR)/$@/sdk.config $(SDK_CONFIG_DIR)/config.sdk
	@cp $(DEFAULT_CONFIG_DIR)/$@/uboot.config $(SDK_CONFIG_DIR)/config.u-boot
	@make -C $(KERNEL_DIR)/$(OS_TYPE) config
	@if [ -d $(MODULE_DIR) ]; then \
		$(MAKE) new_all-c; \
	fi
	@make -C $(KERNEL_DIR)/$(OS_TYPE) kernel_prepare
	@make -C $(KERNEL_DIR)/$(OS_TYPE) kernel_scripts
ifndef config
	@make -C $(KERNEL_DIR)/$(OS_TYPE)
	$(if $(filter $@,9300.linux),make -C model/x86 -f Makefile.$@)
endif
endif


help: $(NEW_MAKE_HELP) $(EXTRA_HELP) $(PACKAGE_HELP)
	@printf "\nBasic commands:\n"
	@printf "   make menuconfig         (go to config menu to set each option)\n"
	@printf "   make                    (build code)\n"
	@printf "\nBuild by default configuration:\n"
	@printf "   make 8380               (8380 kernl mode)\n"
	@printf "   make 8390               (8390 kernel mode)\n"
	@printf "   make 8380u              (8380 user mode)\n"
	@printf "   make 8390u              (8390 user mode)\n"
	@printf "   make 83xx               (8380,8390 kernel mode)\n"
	@printf "   make 83xxu              (8380,8390 user mode)\n"
	@printf "   make 8396msc            (8396,8214QF,8295R kernel mode)\n"
	@printf "   make 9300               (9300 kernel mode)\n"
	@printf "   make 9300k2             (9300 kernel mode with kernel 2.6.19)\n"
	@printf "   make 9300u              (9300 user mode)\n"
	@printf "   make 9310               (9310 kernel mode)\n"
	@printf "   make 9310u              (9310 user mode)\n"
	@printf "   make 93xx               (9300 and 9310 kernel mode)\n"
	@printf "   make 93xxu              (9300 and 9310 user mode)\n"
	@printf "   make 3chips             (8380,8390,9300 kernel mode)\n"
	@printf "   make 4chips             (8380,8390,9300,9310 kernel mode)\n"
	@printf "   make all-conditions     (for most above)\n"
	@printf "\n"
	@printf "Apply default configuration only (no compile):\n"
	@printf "   append "config=1"      (e.g. make 8380 config=1)\n"
	@printf "Save current configuration:\n"
	@printf "   append "save=1"        (e.g. make 8380 save=1)\n"
	@printf "\n"
	@printf "ALL_PJ=$(PUBLIC_PROJECTS) $(INTERNAL_PROJECTS)\n"
