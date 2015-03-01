#include <stdlib.h>
#include <string.h>
#include "writec.h"

#define VIDEO_BASE ((void *)0xb8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_XY(X, Y) (void *)(VIDEO_BASE + 2 * ((X) + (Y) * SCREEN_WIDTH))

static size_t x = 0;
static size_t y = 0;
static unsigned char color = 0x07;

void clear_line(size_t lineno) {
    char *v = VIDEO_XY(0, lineno);
    size_t i = 0;
    for(; i < SCREEN_WIDTH * 2; i+=2) {
        v[i] = 0;
        v[i+1] = color;
    }
}

void writec(const char *buf, size_t count) {
    char *v = VIDEO_XY(x, y);

    if(buf == NULL)
        return;
    while(count-- > 0) {
        char c = *buf++;

        if(c != '\n') {
            *v++ = c;
            *v++ = color;
        }

        if(x == SCREEN_WIDTH - 1 || c == '\n') {
            x = 0;
            y++;
        } else {
            x++;
        }
        if(y == SCREEN_HEIGHT - 1) {
            memmove(VIDEO_BASE, VIDEO_XY(0, 1), 2 * (SCREEN_WIDTH * (SCREEN_HEIGHT - 1)));
            clear_line(y);
            y--;
        }
    }
}
