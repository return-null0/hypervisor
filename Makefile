export SKIP_BTF=1

obj-m += hypervisor.o
hypervisor-objs := src/vmx_init.o src/vmexit.o src/ept.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
