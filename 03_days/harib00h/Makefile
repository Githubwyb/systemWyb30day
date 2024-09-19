all: img

%.bin : %.S
	nasm -w-zeroing -o $*.bin $*.S

# 生成img文件
#   将ipl.S的512B内容写入到软盘镜像的C0-H0-S1的位置，用于启动扇区
#   剩余需要将软盘扩容到2880个扇区，也就是1440KB
#   将编译出来的二进制放到软盘里面，也就放到了0x4400的位置
img: bootloader.bin kernel
	dd if=bootloader.bin of=haribote.img count=1 bs=512
	dd if=/dev/zero of=haribote.img bs=512 seek=1 skip=1 count=2879
	mkdir -p ./floppy
	mount -o loop haribote.img ./floppy -o fat=12
	sleep 1
	cp kernel ./floppy
	sleep 1
	umount ./floppy

kernel: head.bin
	cp head.bin kernel

run :
	qemu-system-x86_64 -enable-kvm -m 4G -smp 1 -drive file=haribote.img,format=raw,if=floppy

clean:
	rm -f kernel
	rm -f haribote.img
	rm -f *.bin
	rm -rf ./floppy
