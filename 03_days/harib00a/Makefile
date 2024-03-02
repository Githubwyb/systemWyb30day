all: img

boot:
	nasm -w-zeroing -o bootloader ipl.S

# 生成img文件
#   将ipl.S的512B内容写入到软盘镜像的C0-H0-S1的位置，用于启动扇区
#   剩余需要将软盘扩容到2880个扇区，也就是1440KB
img: boot
	dd if=bootloader of=haribote.img count=1 bs=512
	dd if=/dev/zero of=haribote.img bs=512 seek=1 skip=1 count=2879

run :
	qemu-system-x86_64 -enable-kvm -m 4G -smp 1 -fda haribote.img

clean :
	rm -f bootloader haribote.img
