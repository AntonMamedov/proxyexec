obj-m += proxyexec.o
proxyexec-objs := src/proxyexec.o src/kln.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
