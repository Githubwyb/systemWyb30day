TOOLPATH = ../../z_tools
INCLUDE = -I$(TOOLPATH)/haribote/ -I./include/ -I./arch/x86/include/ -I./arch/x86/include/uapi/ -I.
CFLAGS  = -Wall -Werror -Wno-int-to-pointer-cast -Wno-unused -Wno-array-bounds
# -fno-pie 生成位置相关代码，位置无关代码暂时不清楚为什么会运行有问题，访问全局变量失败导致调色板设置有问题
# -fno-stack-protector gcc默认会添加栈安全检查，但是这个会依赖glibc的库，导致undefined reference to `__stack_chk_fail'的问题
CFLAGS += -MD -Os -fno-pie -nostdinc -nostdlib -fno-builtin -fno-stack-protector -fno-omit-frame-pointer -D_SIZE_T -DCONFIG_X86_32 -m32
LDFLAGS	= -m elf_i386 -no-pie

all: img

bootloader : bootloader.S
	nasm -w-zeroing -o $@.bin $@.S

#下面四个命令通过模式匹配获取当前目录下的所有C文件
SRCDIR = ./ ./lib/ ./init/

C_SOURCES = $(foreach d,$(SRCDIR),$(wildcard $(d)*.c))
C_OBJS = $(patsubst %.c,%.o,$(C_SOURCES))
C_DEPS = $(patsubst %.c,%.d,$(C_SOURCES))

%.o : %.c
	gcc $(INCLUDE) $(CFLAGS) -c -o $*.o $*.c

# 生成img文件
#   将ipl.S的512B内容写入到软盘镜像的C0-H0-S1的位置，用于启动扇区
#   剩余需要将软盘扩容到2880个扇区，也就是1440KB
#   将编译出来的二进制放到软盘里面，也就放到了0x4400的位置
img: bootloader kernel
	dd if=bootloader.bin of=haribote.img count=1 bs=512 &>/dev/null
	dd if=/dev/zero of=haribote.img bs=512 seek=1 skip=1 count=2879 &>/dev/null
	mkdir -p ./floppy
	mount -o loop haribote.img ./floppy -o fat=12
	sleep 1
	cp kernel ./floppy
	sleep 1
	umount ./floppy
	rm -rf ./floppy

kernel: header cobjs
	cat cobjs >> header
	cp header kernel

# kernel存在软盘的0x4400，拷贝到0x8000后就是0xc400
# header.S启动从bootsect_start开始
# header.S只能保留text段，因为cobjs在header.S最后面，多了段会导致拷贝磁盘出现偏移问题
header: header.S
	gcc $(INCLUDE) $(CFLAGS) -c -o $@.bin $<
	ld $(LDFLAGS) -N -e bootsect_start -Ttext 0xc400 -o $@.out $@.bin
	objdump -S -D $@.out > $@.asm
	objcopy -S -O binary -j .text $@.out $@

asmfunc.o: asmfunc.S
	gcc $(INCLUDE) $(CFLAGS) -c -o $@ $<

# c语言起始函数为start_kernel
cobjs: $(C_OBJS) asmfunc.o
	ld $(LDFLAGS) -N -T cobjs.ld -o $@.out $^
	objdump -S -D $@.out > $@.asm
	objcopy -S -O binary $@.out $@

run :
	qemu-system-x86_64 -enable-kvm -m 4G -smp 1 -fda haribote.img

clean:
	rm -f kernel
	rm -f cobjs cobjs.out cobjs.asm
	rm -f header header.bin header.out header.asm
	rm -f bootloader.bin haribote.img
	rm -f $(C_OBJS) $(C_DEPS)
	rm -f *.o *.d *.bin *.asm *.out
