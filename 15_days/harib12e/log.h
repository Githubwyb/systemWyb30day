#ifndef __LOG_H__
#define __LOG_H__

#include <linux/kernel.h>

/**
 * 以网络结构显示数据
 * @param data 数据首地址
 * @param length 数据长度
 */
static void log_hex_fn(const void *data, int length, void (*log_print)(const char *, ...)) {
    int i = 0, j = 0;
    const char *pData = (const char *)data;

    log_print("    ");
    for (i = 0; i < 16; i++) {
        log_print("%X  ", i);
    }
    log_print("    ");
    for (i = 0; i < 16; i++) {
        log_print("%X", i);
    }

    log_print("\r\n");

    for (i = 0; i < length; i += 16) {
        log_print("%02x  ", i / 16);
        for (j = i; j < i + 16 && j < length; j++) {
            log_print("%02x ", pData[j] & 0xff);
        }
        if (j == length && length % 16) {
            for (j = 0; j < (16 - length % 16); j++) {
                log_print("   ");
            }
        }
        log_print("    ");
        for (j = i; j < i + 16 && j < length; j++) {
            if (pData[j] < 32 || pData[j] >= 127) {
                log_print(".");
            } else {
                log_print("%c", pData[j] & 0xff);
            }
        }

        log_print("\r\n");
    }
}

void serial_printf(const char *fmt, ...);

void _print_current_time(void);

#define LOG_INFO(fmt, ...) \
    _print_current_time(); \
    serial_printf("[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOG_HEX(data, len) log_hex_fn(data, len, serial_printf)

#endif  // __LOG_H__
