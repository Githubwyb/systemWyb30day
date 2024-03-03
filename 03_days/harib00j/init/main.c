
#define io_cli() asm("cli")  //!!!本次添加部分
#define io_sti() asm("sti")  //!!!本次添加部分
#define io_halt() asm("hlt")

void start_kernel(void) {
    for (;;) {
        io_halt();
    }
}
