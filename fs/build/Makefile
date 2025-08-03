obj-m += myfs.o

# 内核源码路径
KDIR = /lib/modules/$(shell uname -r)/build

# 默认构建目标
all:
	make -C $(KDIR) M=$(PWD) modules

# 清理
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.c *.mod *.symvers *.order