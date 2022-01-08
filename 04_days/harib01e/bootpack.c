// 声明一个外部函数
extern void io_hlt(void);
// extern void write_mem8(char *addr, char data);

void write_mem8(char *addr, char data) { *addr = data; }

void HariMain(void) {
    int i;
    for (i = 0xa0000; i < 0xaffff; ++i) {
        write_mem8((char *)i, i & 0x0f);
    }

    for (;;) {
        io_hlt();
    }
}
