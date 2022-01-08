// 声明一个外部函数
extern void io_hlt(void);
// extern void write_mem8(char *addr, char data);

void write_mem8(char *addr, char data) { *addr = data; }

void HariMain(void) {
    int i;
    char *p;

    p = (char *)0xa0000;
    for (i = 0x00000; i < 0x0ffff; ++i) {
        p[i] = i & 0x0f;
    }

    for (;;) {
        io_hlt();
    }
}
