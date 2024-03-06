#include <linux/compiler_types.h>

void HariMain();

__visible void __attribute__((section(".text.first"))) start_kernel(void) {
    HariMain();
}
