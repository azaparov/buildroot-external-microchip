obj-m += $(addsuffix .o, $(notdir $(basename $(wildcard $(BR2_EXTERNAL_MCHP_PATH)/package/power_down_module/*.c))))
ccflags-y := -DDEBUG -g -std=gnu99 -Wno-declaration-after-statement

.PHONY: all clean

all:
	$(MAKE) -C '$(LINUX_DIR)' M='$(PWD)' modules


clean:
	$(MAKE) -C '$(LINUX_DIR)' M='$(PWD)' clean

