MODULE_NAME := kfetch_mod_311551137
obj-m += $(MODULE_NAME).o
PWD := /home/june/Desktop/kfetch_mod/311551137

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	@sudo insmod $(MODULE_NAME).ko

check:
	@sudo lsmod | grep $(MODULE_NAME)

unload:
	@sudo rmmod $(MODULE_NAME).ko