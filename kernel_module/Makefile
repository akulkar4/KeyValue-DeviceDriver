#
# By default, the build is done against the running linux kernel source.
# To build against a different kernel source tree, set SYSSRC:
#
#    make SYSSRC=/path/to/kernel/source

ifdef SYSSRC
 KERNEL_SOURCES	 = $(SYSSRC)
else
 KERNEL_UNAME	:= $(shell uname -r)
 KERNEL_SOURCES	 = /lib/modules/$(KERNEL_UNAME)/build
endif


default: modules
.PHONY: default
install: modules_install
	cp keyvalue-blacklist.conf /etc/modprobe.d
	cp 80-keyvalue.rules /etc/udev/rules.d
	cp include/*.h /usr/local/include/keyvalue/
	update-initramfs -u

.PHONY: install


.PHONY:


%::
	$(MAKE) -C $(KERNEL_SOURCES) \
        M=$$PWD $@
