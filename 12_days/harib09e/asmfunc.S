.code32

# 定义全局符号，这个类似于C语言的export
.global io_hlt

# 指定.text段
.section .text

# 函数实现
io_hlt:
    hlt
    ret

.global inthandler20
.global asm_inthandler20
asm_inthandler20:
    push	%es
    push	%ds
    pushal
    mov		%esp,%eax
    push	%eax

    mov		%ss,%ax
    mov		%ax,%ds
    mov		%ax,%es
    call	inthandler20

    pop		%eax
    popal
    pop		%ds
    pop		%es
    iret

.global inthandler21
.global asm_inthandler21
asm_inthandler21:
    push	%es
    push	%ds
    pushal
    mov		%esp,%eax
    push	%eax

    mov		%ss,%ax
    mov		%ax,%ds
    mov		%ax,%es
    call	inthandler21

    pop		%eax
    popal
    pop		%ds
    pop		%es
    iret

.global inthandler27
.global asm_inthandler27
asm_inthandler27:
    push	%es
    push	%ds
    pushal
    mov		%esp,%eax

    push	%eax
    mov		%ss,%ax
    mov		%ax,%ds
    mov		%ax,%es
    call	inthandler27

    pop		%eax
    popal
    pop		%ds
    pop		%es
    iret

.global inthandler2c
.global asm_inthandler2c
asm_inthandler2c:
    push	%es
    push	%ds
    pushal
    mov		%esp,%eax
    push	%eax

    mov		%ss,%ax
    mov		%ax,%ds
    mov		%ax,%es
    call	inthandler2c

    pop		%eax
    popal
    pop		%ds
    pop		%es
    iret
