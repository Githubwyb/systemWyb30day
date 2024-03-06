.code32

# 定义全局符号，这个类似于C语言的export
.global io_hlt

# 指定.text段
.section .text

# 函数实现
io_hlt:
    hlt
    ret

# .global io_load_eflags
# io_load_eflags:
#     pushfd
#     pop     %eax
#     ret

# .global io_store_eflags
# io_store_eflags:
#     mov     0x4(%esp),%eax
#     push    %eax
#     popfd
#     ret
