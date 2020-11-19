obj-m := fake_eth.o
KDIR := /lib/modules/`uname -r`/build
PWD := `pwd`
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

