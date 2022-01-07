; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式
[BITS 32]						; 制作32位的机器语言


; 制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名称
		GLOBAL	_io_hlt			; 函数名


; 函数的实现

[SECTION .text]		; text段，储存程序代码
_io_hlt:	; void io_hlt(void);
		HLT
		RET
