obj-m += rootkit.o
rootkit-objs := src/rootkit.o src/hook.o src/hide.o src/persist.o src/comm.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
	make -C companion/

clean:
	make -C $(KDIR) M=$(PWD) clean
	make -C companion/ clean
