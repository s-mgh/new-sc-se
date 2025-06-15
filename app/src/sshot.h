#ifndef SSHOT_H
#define SSHOT_H
#include <stddef.h>  // اضافه کردن این خط برای تعریف size_t
void generate_screenshot_filename(char *filename, size_t size);
void take_screenshot(const char *filename);
void capture_screenshot(void);

#endif