; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; Bootpack的加载目的地
DSKCAC	EQU		0x00100000		; 磁盘缓存的位置
DSKCAC0	EQU		0x00008000		; 磁盘缓存位置(真实模式)

; BOOT_INFO相关
CYLS	EQU		0x0ff0			; 引导扇区设置
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 有关颜色数量的信息。多少位颜色？
SCRNX	EQU		0x0ff4			; 屏幕像素的x
SCRNY	EQU		0x0ff6			; 屏幕像素的y
VRAM	EQU		0x0ff8			; 图形缓冲区的起始地址

		ORG		0xc200			; 这个程序会被读到哪里
; 设置屏幕模式
		MOV		AL,0x13			; VGA图形，320 x 200 x 8位颜色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000		; 此模式下vram为0xa0000 ~ 0xaffff
; 让BIOS告诉我键盘的LED状态
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC关闭所有中断
;	根据AT兼容机的规格，如果要初始化PIC，必须在CLI前执行，否则可能会挂起。
;	初始化PIC
		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; 连续执行OUT，某些机器会无法正常运行
		OUT		0xa1,AL
		CLI						; 禁止CPU级别中断

; 让CPU能访问1MB以上的内存空间，设定A20Gate
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式
[INSTRSET "i486p"]				; 这里说明要使用486指令

		LGDT	[GDTR0]			; 临时设定GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31设置为0
		OR		EAX,0x00000001	; bit0设置为1（为了切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  可读写的段 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; 转送bootpack
		MOV		ESI,bootpack	; 源地址
		MOV		EDI,BOTPAK		; 目的地址
		MOV		ECX,512*1024/4
		CALL	memcpy

; 磁盘数据转送到它本来的位置
; 从启动扇区开始
		MOV		ESI,0x7c00		; 源地址
		MOV		EDI,DSKCAC		; 目的地址
		MOV		ECX,512/4
		CALL	memcpy
; 剩余部分
		MOV		ESI,DSKCAC0+512	; 源地址
		MOV		EDI,DSKCAC+512	; 目的地址
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数变换成字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy

; 必须由asmhead来完成的工作到此完成
; 剩余交给bootpack来完成
; 启动bootpack
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要转的东西时
		MOV		ESI,[EBX+20]	; 源地址
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 目的地址
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b		; 这里指定从第二个段的0x1b开始执行，第二个段基地址为0x280000所以从0x28001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND的结果不是0就跳到waitdbout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 减法结果不为0就跳转memcpy
		RET
; Memcpy如果不忘记插入地址大小前缀，也可以用串指令写

		ALIGNB	16
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以被写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以被执行的段 32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
