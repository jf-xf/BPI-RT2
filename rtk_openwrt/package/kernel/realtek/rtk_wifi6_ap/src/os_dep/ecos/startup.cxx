#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>


externC void setup_wifi_module_param(void);
externC int rtk_wfo_pcie_init(void);

class rtk_wlan_init_class {
public:
	rtk_wlan_init_class(void) {
		setup_wifi_module_param();
		rtk_wfo_pcie_init();
	}
};

// And here's an instance of the class just to make the code run before compat
static rtk_wlan_init_class _rtk_wlan_init CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_COMPAT+400);
