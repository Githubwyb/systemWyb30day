#include <linux/compiler_types.h>
#include <linux/init.h>

void HariMain();

__visible void __section(".text.first") start_kernel(void) { HariMain(); }
