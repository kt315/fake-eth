MOD_NAME := fake_eth.ko
obj-m := fake_eth.o
KDIR := /lib/modules/`uname -r`/build
KDIR_INSTALL := /lib/modules/`uname -r`/kernel/drivers/net
PWD := `pwd`
# ccflags-y := $(ccflags-y) -xc -E -v

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	cp -f $(MOD_NAME) $(KDIR_INSTALL)
	depmod

remove:
	rm -vf $(KDIR_INSTALL)/$(MOD_NAME)
	depmod
