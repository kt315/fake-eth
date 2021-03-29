obj-m := fake_eth.o
KDIR := /lib/modules/`uname -r`/build
PWD := `pwd`
# ccflags-y := $(ccflags-y) -xc -E -v

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
