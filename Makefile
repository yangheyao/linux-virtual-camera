test-objs := myvivi.o

obj-$(CONFIG_VIDEO_TEST) += test.o

#KERN_DIR = /home/jonah/Code/repo_rk3399/kernel
#KERN_DIR = /usr/src/linux-headers-5.11.0-37-generic/


#all:
#	make -C $(KERN_DIR) M=`pwd` modules
#clean: 
#	make -C $(KERN_DIR) M=`pwd` modules clean
#	rm -rf modules.order

#obj-m += myvivi.o
#obj-m += jonah.o
