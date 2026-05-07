dtb-$(CONFIG_RTK_SOC_RTL8198D) += rtl8198d_engboard.dtb
dtb-$(CONFIG_RTK_SOC_RTL9607C) += rtl9607c_engboard.dtb

obj-$(CONFIG_BUILTIN_DTB)       += $(addsuffix .o, $(dtb-y))
ifeq ($(CONFIG_LUNA_MEMORY_AUTO_DETECTION),y)
$(shell cd arch/mips/boot/dts/realtek; \
	ln -f -s memory-16.dts memory/memory.dts; \
	ln -f -s memory/memory.dts memory.dts)
else
$(shell cd arch/mips/boot/dts/realtek; \
	ln -f -s memory-$(CONFIG_SOC_MEM_SIZE).dts memory/memory.dts; \
	ln -f -s memory/memory.dts memory.dts)
endif
