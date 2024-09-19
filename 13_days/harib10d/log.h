#ifndef __LOG_H__
#define __LOG_H__

void serial_printf(const char *fmt, ...);

void _print_current_time(void);

#define LOG_INFO(fmt, ...) \
    _print_current_time(); \
    serial_printf("[%s:%d %s] " fmt "\r\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif  // __LOG_H__
