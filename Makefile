obj-m += keylogger.o
KVER = $(shell uname -r)
PWD = $(shell PWD)
#ARCH = arm
OBJS = 

all:
	make -C /lib/modules/$(KVER)/build M=$(PWD) modules 

install:
	make -C /lib/modules/$(KVER)/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/$(KVER)/build M=$(PWD) clean
