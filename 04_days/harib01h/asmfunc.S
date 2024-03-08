.code32

# 定义全局符号，这个类似于C语言的export
.global io_hlt

# 指定.text段
.section .text

# 函数实现
io_hlt:
    hlt
    ret
