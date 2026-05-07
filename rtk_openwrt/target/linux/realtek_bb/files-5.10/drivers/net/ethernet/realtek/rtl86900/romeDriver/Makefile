ifeq ($(CONFIG_RT_EDP_API),y)
include $(APOLLODIR)/romeDriver/Makefile.edp

else #!EDP

ifeq ($(CONFIG_RG_G3_SERIES),y)
include $(APOLLODIR)/romeDriver/Makefile.g3
else
include $(APOLLODIR)/romeDriver/Makefile.apollo
endif

endif #EDP

