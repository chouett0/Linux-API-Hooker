KERNDIR := /lib/modules/`uname -r`/build
BUILD_DIR := $(shell pwd)
VERBOSE = 0

obj-m := sniff-swrite.o
smallmod-objs := sniff-swrite.o

all:
	bash set_syscall_table_address.sh
	make -C $(KERNDIR) SUBDIRS=$(BUILD_DIR) KBUILD_VERBOSE=$(VERBOSE) modules

clean:
	rm -f *.orm -f *.ko
	rm -f *.mod.c
	rm -f *~
