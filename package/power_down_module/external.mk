POWER_DOWN_MODULE_VERSION = 1.0
POWER_DOWN_MODULE_SITE = $(BR2_EXTERNAL_MCHP_PATH)/package/power_down_module
POWER_DOWN_MODULE_SITE_METHOD = local
$(eval $(kernel-module))
$(eval $(generic-package))

