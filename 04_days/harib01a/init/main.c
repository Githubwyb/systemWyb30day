
#define io_cli() asm("cli")  //!!!本次添加部分
#define io_sti() asm("sti")  //!!!本次添加部分
#define io_hlt() asm("hlt")

#define write_mem8(addr, data8) (*(volatile char *)(addr)) = (char)data8

void start_kernel(void) {
    unsigned int i;
    for (i = 0x000a0000; i < 0x000affff; ++i) {
        write_mem8(i, 15);
    }

    for (;;) {
        io_hlt();
    }
}
