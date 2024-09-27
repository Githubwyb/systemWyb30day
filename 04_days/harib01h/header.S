# head.S bootloader下一步入口，存放于软盘中的二进制内核文件头部
#include <uapi/asm/processor-flags.h>

#define GDT_ENTRY_KERNEL_CS 12
#define GDT_ENTRY_KERNEL_DS 13

# boot_info结构体成员对应的地址，给c语言读取使用
#define CYLS	 0x0ff0
#define LEDS	 0x0ff1
# 有关颜色数量的信息。多少位颜色
#define VMODE	 0x0ff2
# 屏幕像素的x
#define SCRNX	 0x0ff4
# 屏幕像素的y
#define SCRNY	 0x0ff6
# 图形缓冲区的起始地址
#define VRAM	 0x0ff8

# 栈起始位置，共1MB。0x002fffff-0x003fffff
#define STACK    	0x003fffff
# C语言代码所在内存位置
#define C_CODE		0x00280000
# 软盘数据所在内存位置
#define	DISK_ADDR		0x00100000
# 软盘数据所在内存位置（真实模式）
#define DISK_ADDR_REAL	0x00008000

.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector

.global bootsect_start
bootsect_start:
	.code16		# 后面代码是16位代码段，cpu默认开启在真实模式，所以要使用16位汇编开始

	# 设置屏幕模式
	movb		$0x13,%al			# VGA图形，320 x 200 x 8位颜色
	movb		$0x00,%ah
	int			$0x10
	# 记录画面参数到内存中，给c获取使用
	movb		$0x08,(VMODE)
	movw		$320,(SCRNX)
	movw		$200,(SCRNY)
	movl		$0x000a0000,(VRAM) # 此模式下vram为0xa0000 ~ 0xaffff
	# 让BIOS告诉我键盘的LED状态
	movb		$0x02,%ah
	int			$0x16				# keyboard BIOS
	movb		%al,(LEDS)			# 将键盘led灯状态放到内存中给c读取
	# 屏幕显示字符串
	movw 		$msg,%si		# msg是一个符号，也是一个地址，赋值地址给si
	call 		puts

	# PIC关闭所有中断
	#	根据AT兼容机的规格，如果要初始化PIC，必须在CLI前执行，否则可能会挂起。
	#	初始化PIC
	movb 	$0xff,%al
	outb 	%al,$0x21		# 禁止主PIC中断
	nop
	outb 	%al,$0x21		# 禁止从PIC终端
	cli						# 禁止所有中断

# 让CPU能访问1MB以上的内存空间，设定A20Gate
seta20gate:
	# enable A20 第一步
	call	waitkbdout
	movb	$0xd1,%al
	outb	%al,$0x64
	call	waitkbdout
	# enable A20 第二步
	movb	$0xdf,%al
	outb	%al,$0x60
	call	waitkbdout

	# 切换到保护模式需要临时设置以下gdt
	lgdt    gdtdesc
	movl	%cr0,%eax
	orl		$X86_CR0_PE,%eax	# PE设置为1切换到保护模式
	movl	%eax,%cr0

	jmp protcseg

msg:
  	.asciz "\n\nkernel is loading"

try:
  	.asciz "\n\ntry it again"

# 显示字符到屏幕上
puts:
	movb (%si),%al
	add $1,%si
	cmp $0,%al
	je over
	movb $0x0e,%ah
	movw $15,%bx
	int $0x10
	jmp puts
over:
	ret

protcseg:
	# Set up the protected-mode data segment registers
	movw    $(GDT_ENTRY_KERNEL_DS*8), %ax   # Our data segment selector
	movw    %ax, %ds                		# -> DS: Data Segment
	movw    %ax, %es                		# -> ES: Extra Segment
	movw    %ax, %fs                		# -> FS
	movw    %ax, %gs                		# -> GS
	movw    %ax, %ss                		# -> SS: Stack Segment
	# 设置栈指针到栈顶，调用c函数
	movl    $STACK,%esp

	# 拷贝c代码到内存中
	movl	$start_kernel,%esi		# 源地址
	movl	$C_CODE,%edi			# 目的地址
	movl	$(512*1024/4),%ecx		# 拷贝512K，注意后面的C语言编译超过512K就要修改这里
	call	asmmemcpy

	# 拷贝软盘数据到内存对应位置
	# 拷贝启动扇区
	movl	$0x00007c00,%esi		# 源地址
	movl	$DISK_ADDR,%edi			# 目的地址
	movl	$(512/4),%ecx			# 拷贝512B
	call	asmmemcpy
	# 拷贝剩余数据
	movl	$(DISK_ADDR_REAL+512),%esi		# 源地址
	movl	$(DISK_ADDR+512),%edi			# 目的地址
	movb	(CYLS),%cl						# 把cyls地址内的值赋值给cl
	# 将ecx乘上512*8*2/4，cyls是bootloader读取的软盘中柱面数，一个柱面18个磁道，一个磁道2个扇区，一个扇区512B
	imul	$(512*18*2/4),%ecx
	subl	$(512/4),%ecx					# 去除启动扇区
	call	asmmemcpy

	# 调用c语言第一个指令，位置是CS段的首地址
	ljmp	$(GDT_ENTRY_KERNEL_CS*8),$0x0000

spin:
  	jmp spin

hlt_cpu:
	hlt
	jmp hlt_cpu

asmmemcpy:
	movl		(%esi),%eax		# 把esi寄存器存储的地址对应的值赋值给eax
	addl		$4,%esi			# 将esi存储的地址加4
	movl		%eax,(%edi)		# 将eax寄存器内存储的值赋值给edi存储的地址指向的位置
	addl		$4,%edi			# 将edi存储的地址加4
	subl		$1,%ecx 		# 将ECX减1，剩余拷贝4字节数
	jnz			asmmemcpy		# jump not zero 减法结果不为0就跳转asmmemcpy
	ret

waitkbdout:
	inb		$0x64,%al		# 读取0x64端口到al寄存器
	testb	$0x2,%al		# 对比读出来的值是不是0x2
	jnz     waitkbdout		# 0也就是不相等，循环继续
	ret

# 段描述结构体
# struct desc_struct {
#     u16 limit0;         // 段管理的内存上限low
#     u16 base0;          // 段的对应的内存实际地址low
#     u16 base1 : 8;      // 段的对应的内存实际地址mid
#     u16 type : 4;
#     u16 s : 1;          // 系统段为1，普通段为0
#     u16 dpl : 2;
#     u16 p : 1;
#     u16 limit1 : 4;     // 段管理的内存上限low
#     u16 avl : 1;
#     u16 l : 1;
#     u16 d : 1;
#     u16 g : 1;          // 为1就是4K为单位定义上限（上限1，管理内存4K），为0则以一个字节为单位
#     u16 base2 : 8;      // 段的对应的内存实际地址high
# } __attribute__((packed));
# 第0个段是空的，全部是0
# 第12个段是代码段，c代码所在位置，从0x00280000开始，0x9a代表可读可执行，内存上限为0x7ffff，g = 0代表字节为单位，512K
# 	按照结构体和16位低位高位计算 0xffff, 0x0000, 0x9a28, 0x0047
# 第13个段是数据段，cpu管理的总内存，从0x00000000开始，0x92代表可读可写，内存上限为0xfffff，g = 1代表4K为单位，4G
# 	按照结构体和16位低位高位计算 0xffff, 0x0000, 0x9200, 0x00cf
.p2align	2	# 按照2^2 = 4字节对齐
gdt:
	.word	0x0000,0x0000,0x0000,0x0000		# 0  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 1  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 2  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 3  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 4  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 5  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 6  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 7  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 8  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 9  null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 10 null seg
	.word	0x0000,0x0000,0x0000,0x0000		# 11 null seg
	.word	0xffff,0x0000,0x9a28,0x0047		# 12 给c代码使用的段，代码段
	.word	0xffff,0x0000,0x9200,0x00cf		# 13 可读写的段，数据段

gdtdesc:
  	.word   8*14-1                          # sizeof(gdt) - 1
  	.long   gdt                             # address gdt
start_kernel:
