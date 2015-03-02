#ifndef _WRITEC_H
#define _WRITEC_H 1

#include <sys/types.h>

void writec_xy(const char *buf, size_t count, uint8_t x, uint8_t y, int save_xy);
void writec(const char *buf, size_t count);
void writec_time(uint64_t seconds);
void fill_screen(void);

#endif
