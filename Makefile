export SKIP_BTF=1

obj-m += hv.o

hv-y := \
    hypervisor.o \
    src/vmx_init.o \
    src/vmexit.o \
    src/ept.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
