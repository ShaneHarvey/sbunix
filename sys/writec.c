#include <stdlib.h>
#include <string.h>
#include <sys/writec.h>

#define VIDEO_BASE ((void *)0xb8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_XY(X, Y) (void *)(VIDEO_BASE + 2 * ((X) + (Y) * SCREEN_WIDTH))

static uint8_t _x = 0;
static uint8_t _y = 0;
static uint8_t color = 0x07;

void clear_line(uint8_t lineno) {
    char *v = VIDEO_XY(0, lineno);
    int i = 0;
    for(; i < SCREEN_WIDTH * 2; i+=2) {
        v[i] = 0;
        v[i+1] = color;
    }
}

void writec_xy(const char *buf, size_t count, uint8_t x, uint8_t y, int save_xy) {
    char *v = VIDEO_XY(x, y);
   // if(!(x < SCREEN_WIDTH && y < SCREEN_HEIGHT))
    //    return;

    if(buf == NULL)
        return;
    while(count-- > 0) {
        char c = *buf++;

        if(c != '\n') {
            *v++ = c;
            *v++ = color;
        }

        if(x >= SCREEN_WIDTH - 1 || c == '\n') {
            x = 0;
            y++;
        } else {
            x++;
        }
        if(y >= SCREEN_HEIGHT) {
            memmove(VIDEO_BASE, VIDEO_XY(0, 1), 2 * (SCREEN_WIDTH * (SCREEN_HEIGHT - 1)));
            clear_line(SCREEN_HEIGHT - 1);
            y--;
        }
    }
    if(save_xy) {
        _x = x;
        _y = y;
    }
}


void writec(const char *buf, size_t count) {
    writec_xy(buf, count, _x, _y, 1);
}

void writec_time(uint64_t seconds) {
    char strsec[21] = {0};
    uint8_t len;
    /* Convert to string */
    uitoa(seconds, 10, strsec, sizeof(strsec));
    len = (uint8_t)strlen(strsec);
    writec_xy(strsec, strlen(strsec), SCREEN_WIDTH - 1 - len, 0, 0);
}

void fill_screen(void) {
    char *v = VIDEO_XY(0, 0);
    char *end = VIDEO_XY(80, 25);
    int i = 0;

    while(v < end) {
        *v++ = '0' + (i++ % 10);
        *v++ = color;
    }
}
