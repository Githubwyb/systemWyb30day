[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_table_rgb
	EXTERN	_io_load_eflags
	EXTERN	_io_cli
	EXTERN	_io_out8
	EXTERN	_io_store_eflags
	EXTERN	_io_hlt
[FILE "bootpack.c"]
[SECTION .data]
_table_rgb:
	DB	-1
	DB	-1
	DB	-1
	DB	-1
	DB	-1
	DB	-52
	DB	-1
	DB	-1
	DB	-103
	DB	-1
	DB	-1
	DB	102
	DB	-1
	DB	-1
	DB	51
	DB	-1
	DB	-1
	DB	0
	DB	-1
	DB	-52
	DB	-1
	DB	-1
	DB	-52
	DB	-52
	DB	-1
	DB	-52
	DB	-103
	DB	-1
	DB	-52
	DB	102
	DB	-1
	DB	-52
	DB	51
	DB	-1
	DB	-52
	DB	0
	DB	-1
	DB	-103
	DB	-1
	DB	-1
	DB	-103
	DB	-52
	DB	-1
	DB	-103
	DB	-103
	DB	-1
	DB	-103
	DB	102
	DB	-1
	DB	-103
	DB	51
	DB	-1
	DB	-103
	DB	0
	DB	-1
	DB	102
	DB	-1
	DB	-1
	DB	102
	DB	-52
	DB	-1
	DB	102
	DB	-103
	DB	-1
	DB	102
	DB	102
	DB	-1
	DB	102
	DB	51
	DB	-1
	DB	102
	DB	0
	DB	-1
	DB	51
	DB	-1
	DB	-1
	DB	51
	DB	-52
	DB	-1
	DB	51
	DB	-103
	DB	-1
	DB	51
	DB	102
	DB	-1
	DB	51
	DB	51
	DB	-1
	DB	51
	DB	0
	DB	-1
	DB	0
	DB	-1
	DB	-1
	DB	0
	DB	-52
	DB	-1
	DB	0
	DB	-103
	DB	-1
	DB	0
	DB	102
	DB	-1
	DB	0
	DB	51
	DB	-1
	DB	0
	DB	0
	DB	102
	DB	-1
	DB	-1
	DB	102
	DB	-1
	DB	-52
	DB	102
	DB	-1
	DB	-103
	DB	102
	DB	-1
	DB	102
	DB	102
	DB	-1
	DB	51
	DB	102
	DB	-1
	DB	0
	DB	102
	DB	-52
	DB	-1
	DB	102
	DB	-52
	DB	-52
	DB	102
	DB	-52
	DB	-103
	DB	102
	DB	-52
	DB	102
	DB	102
	DB	-52
	DB	51
	DB	102
	DB	-52
	DB	0
	DB	102
	DB	-103
	DB	-1
	DB	102
	DB	-103
	DB	-52
	DB	102
	DB	-103
	DB	-103
	DB	102
	DB	-103
	DB	102
	DB	102
	DB	-103
	DB	51
	DB	102
	DB	-103
	DB	0
	DB	102
	DB	102
	DB	-1
	DB	102
	DB	102
	DB	-52
	DB	102
	DB	102
	DB	-103
	DB	102
	DB	102
	DB	102
	DB	102
	DB	102
	DB	51
	DB	102
	DB	102
	DB	0
	DB	102
	DB	51
	DB	-1
	DB	102
	DB	51
	DB	-52
	DB	102
	DB	51
	DB	-103
	DB	102
	DB	51
	DB	102
	DB	102
	DB	51
	DB	51
	DB	102
	DB	51
	DB	0
	DB	102
	DB	0
	DB	-1
	DB	102
	DB	0
	DB	-52
	DB	102
	DB	0
	DB	-103
	DB	102
	DB	0
	DB	102
	DB	102
	DB	0
	DB	51
	DB	102
	DB	0
	DB	0
	DB	-52
	DB	-1
	DB	-1
	DB	-52
	DB	-1
	DB	-52
	DB	-52
	DB	-1
	DB	-103
	DB	-52
	DB	-1
	DB	102
	DB	-52
	DB	-1
	DB	51
	DB	-52
	DB	-1
	DB	0
	DB	-52
	DB	-52
	DB	-1
	DB	-52
	DB	-52
	DB	-52
	DB	-52
	DB	-52
	DB	-103
	DB	-52
	DB	-52
	DB	102
	DB	-52
	DB	-52
	DB	51
	DB	-52
	DB	-52
	DB	0
	DB	-52
	DB	-103
	DB	-1
	DB	-52
	DB	-103
	DB	-52
	DB	-52
	DB	-103
	DB	-103
	DB	-52
	DB	-103
	DB	102
	DB	-52
	DB	-103
	DB	51
	DB	-52
	DB	-103
	DB	0
	DB	-52
	DB	102
	DB	-1
	DB	-52
	DB	102
	DB	-52
	DB	-52
	DB	102
	DB	-103
	DB	-52
	DB	102
	DB	102
	DB	-52
	DB	102
	DB	51
	DB	-52
	DB	102
	DB	0
	DB	-52
	DB	51
	DB	-1
	DB	-52
	DB	51
	DB	-52
	DB	-52
	DB	51
	DB	-103
	DB	-52
	DB	51
	DB	102
	DB	-52
	DB	51
	DB	51
	DB	-52
	DB	51
	DB	0
	DB	-52
	DB	0
	DB	-1
	DB	-52
	DB	0
	DB	-52
	DB	-52
	DB	0
	DB	-103
	DB	-52
	DB	0
	DB	102
	DB	-52
	DB	0
	DB	51
	DB	-52
	DB	0
	DB	0
	DB	51
	DB	-1
	DB	-1
	DB	51
	DB	-1
	DB	-52
	DB	51
	DB	-1
	DB	-103
	DB	51
	DB	-1
	DB	102
	DB	51
	DB	-1
	DB	51
	DB	51
	DB	-1
	DB	0
	DB	51
	DB	-52
	DB	-1
	DB	51
	DB	-52
	DB	-52
	DB	51
	DB	-52
	DB	-103
	DB	51
	DB	-52
	DB	102
	DB	51
	DB	-52
	DB	51
	DB	51
	DB	-52
	DB	0
	DB	51
	DB	-103
	DB	-1
	DB	51
	DB	-103
	DB	-52
	DB	51
	DB	-103
	DB	-103
	DB	51
	DB	-103
	DB	102
	DB	51
	DB	-103
	DB	51
	DB	51
	DB	-103
	DB	0
	DB	51
	DB	102
	DB	-1
	DB	51
	DB	102
	DB	-52
	DB	51
	DB	102
	DB	-103
	DB	51
	DB	102
	DB	102
	DB	51
	DB	102
	DB	51
	DB	51
	DB	102
	DB	0
	DB	51
	DB	51
	DB	-1
	DB	51
	DB	51
	DB	-52
	DB	51
	DB	51
	DB	-103
	DB	51
	DB	51
	DB	102
	DB	51
	DB	51
	DB	51
	DB	51
	DB	51
	DB	0
	DB	51
	DB	0
	DB	-1
	DB	51
	DB	0
	DB	-52
	DB	51
	DB	0
	DB	-103
	DB	51
	DB	0
	DB	102
	DB	51
	DB	0
	DB	51
	DB	51
	DB	0
	DB	0
	DB	-103
	DB	-1
	DB	-1
	DB	-103
	DB	-1
	DB	-52
	DB	-103
	DB	-1
	DB	-103
	DB	-103
	DB	-1
	DB	102
	DB	-103
	DB	-1
	DB	51
	DB	-103
	DB	-1
	DB	0
	DB	-103
	DB	-52
	DB	-1
	DB	-103
	DB	-52
	DB	-52
	DB	-103
	DB	-52
	DB	-103
	DB	-103
	DB	-52
	DB	102
	DB	-103
	DB	-52
	DB	51
	DB	-103
	DB	-52
	DB	0
	DB	-103
	DB	-103
	DB	-1
	DB	-103
	DB	-103
	DB	-52
	DB	-103
	DB	-103
	DB	-103
	DB	-103
	DB	-103
	DB	102
	DB	-103
	DB	-103
	DB	51
	DB	-103
	DB	-103
	DB	0
	DB	-103
	DB	102
	DB	-1
	DB	-103
	DB	102
	DB	-52
	DB	-103
	DB	102
	DB	-103
	DB	-103
	DB	102
	DB	102
	DB	-103
	DB	102
	DB	51
	DB	-103
	DB	102
	DB	0
	DB	-103
	DB	51
	DB	-1
	DB	-103
	DB	51
	DB	-52
	DB	-103
	DB	51
	DB	-103
	DB	-103
	DB	51
	DB	102
	DB	-103
	DB	51
	DB	51
	DB	-103
	DB	51
	DB	0
	DB	-103
	DB	0
	DB	-1
	DB	-103
	DB	0
	DB	-52
	DB	-103
	DB	0
	DB	-103
	DB	-103
	DB	0
	DB	102
	DB	-103
	DB	0
	DB	51
	DB	-103
	DB	0
	DB	0
	DB	0
	DB	-1
	DB	-1
	DB	0
	DB	-1
	DB	-52
	DB	0
	DB	-1
	DB	-103
	DB	0
	DB	-1
	DB	102
	DB	0
	DB	-1
	DB	51
	DB	0
	DB	-1
	DB	0
	DB	0
	DB	-52
	DB	-1
	DB	0
	DB	-52
	DB	-52
	DB	0
	DB	-52
	DB	-103
	DB	0
	DB	-52
	DB	102
	DB	0
	DB	-52
	DB	51
	DB	0
	DB	-52
	DB	0
	DB	0
	DB	-103
	DB	-1
	DB	0
	DB	-103
	DB	-52
	DB	0
	DB	-103
	DB	-103
	DB	0
	DB	-103
	DB	102
	DB	0
	DB	-103
	DB	51
	DB	0
	DB	-103
	DB	0
	DB	0
	DB	102
	DB	-1
	DB	0
	DB	102
	DB	-52
	DB	0
	DB	102
	DB	-103
	DB	0
	DB	102
	DB	102
	DB	0
	DB	102
	DB	51
	DB	0
	DB	102
	DB	0
	DB	0
	DB	51
	DB	-1
	DB	0
	DB	51
	DB	-52
	DB	0
	DB	51
	DB	-103
	DB	0
	DB	51
	DB	102
	DB	0
	DB	51
	DB	51
	DB	0
	DB	51
	DB	0
	DB	0
	DB	0
	DB	-1
	DB	0
	DB	0
	DB	-52
	DB	0
	DB	0
	DB	-103
	DB	0
	DB	0
	DB	102
	DB	0
	DB	0
	DB	51
	DB	0
	DB	0
	DB	0
	RESB	120
[SECTION .text]
	GLOBAL	_init_palette
_init_palette:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	_table_rgb
	PUSH	255
	PUSH	0
	CALL	_set_palette
	LEAVE
	RET
_set_palette:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	PUSH	ECX
	MOV	EBX,DWORD [8+EBP]
	MOV	EDI,DWORD [12+EBP]
	MOV	ESI,DWORD [16+EBP]
	CALL	_io_load_eflags
	MOV	DWORD [-16+EBP],EAX
	CALL	_io_cli
	PUSH	EBX
	PUSH	968
	CALL	_io_out8
	CMP	EBX,EDI
	POP	EAX
	POP	EDX
	JLE	L7
L9:
	MOV	EAX,DWORD [-16+EBP]
	MOV	DWORD [8+EBP],EAX
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	JMP	_io_store_eflags
L7:
	MOV	AL,BYTE [ESI]
	INC	EBX
	SHR	AL,2
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	MOV	AL,BYTE [1+ESI]
	SHR	AL,2
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	MOV	AL,BYTE [2+ESI]
	SHR	AL,2
	ADD	ESI,3
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	ADD	ESP,24
	CMP	EBX,EDI
	JLE	L7
	JMP	L9
	GLOBAL	_boxfill8
_boxfill8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	SUB	ESP,12
	MOV	EDI,DWORD [28+EBP]
	MOV	AL,BYTE [16+EBP]
	MOV	EBX,DWORD [20+EBP]
	MOV	ESI,DWORD [32+EBP]
	MOV	BYTE [-13+EBP],AL
	CMP	EBX,EDI
	JG	L22
L20:
	MOV	ECX,DWORD [24+EBP]
	CMP	ECX,ESI
	JG	L24
	MOV	EDX,DWORD [12+EBP]
	MOV	EAX,DWORD [8+EBP]
	IMUL	EDX,ECX
	ADD	EAX,EBX
	MOV	DWORD [-20+EBP],EAX
	MOV	EAX,EDX
	ADD	EAX,DWORD [-20+EBP]
L19:
	MOV	DL,BYTE [-13+EBP]
	INC	ECX
	MOV	BYTE [EAX],DL
	ADD	EAX,DWORD [12+EBP]
	CMP	ECX,ESI
	JLE	L19
L24:
	INC	EBX
	CMP	EBX,EDI
	JLE	L20
L22:
	ADD	ESP,12
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	CALL	_init_palette
	PUSH	171
	PUSH	319
	PUSH	0
	PUSH	0
	PUSH	194
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	172
	PUSH	319
	PUSH	172
	PUSH	0
	PUSH	79
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	173
	PUSH	319
	PUSH	173
	PUSH	0
	PUSH	0
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	199
	PUSH	319
	PUSH	174
	PUSH	0
	PUSH	79
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	176
	PUSH	59
	PUSH	176
	PUSH	3
	PUSH	0
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	196
	PUSH	2
	PUSH	176
	PUSH	2
	PUSH	0
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	196
	PUSH	59
	PUSH	196
	PUSH	3
	PUSH	158
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	195
	PUSH	59
	PUSH	177
	PUSH	59
	PUSH	158
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	197
	PUSH	59
	PUSH	197
	PUSH	2
	PUSH	215
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	197
	PUSH	60
	PUSH	176
	PUSH	60
	PUSH	215
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	176
	PUSH	316
	PUSH	176
	PUSH	273
	PUSH	158
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	196
	PUSH	273
	PUSH	177
	PUSH	273
	PUSH	158
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	197
	PUSH	316
	PUSH	197
	PUSH	273
	PUSH	0
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	PUSH	197
	PUSH	317
	PUSH	176
	PUSH	317
	PUSH	0
	PUSH	320
	PUSH	655360
	CALL	_boxfill8
	ADD	ESP,56
L26:
	CALL	_io_hlt
	JMP	L26
