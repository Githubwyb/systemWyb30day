[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_io_hlt
[FILE "bootpack.c"]
[SECTION .text]
	GLOBAL	_write_mem8
_write_mem8:
	PUSH	EBP
	MOV	EBP,ESP
	MOV	EAX,DWORD [8+EBP]
	MOV	EDX,DWORD [12+EBP]
	MOV	BYTE [EAX],DL
	POP	EBP
	RET
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EBX
	MOV	EBX,655360
L7:
	PUSH	10
	PUSH	EBX
	INC	EBX
	CALL	_write_mem8
	CMP	EBX,720894
	POP	EAX
	POP	EDX
	JLE	L7
L8:
	CALL	_io_hlt
	JMP	L8