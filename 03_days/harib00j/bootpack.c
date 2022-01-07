// 声明一个外部函数
extern void io_hlt(void);

void HariMain(void) {
fin:
	// 调用外部函数
    io_hlt();
    goto fin;
}
