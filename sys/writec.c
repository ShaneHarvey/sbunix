#include <stdlib.h>
#include "writec.h"

#define VIDEO_BASE 0xb8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define CURRENT_START (VIDEO_BASE + 2*(x + y * SCREEN_WIDTH))

static size_t x = 0;
static size_t y = 0;
// static char color = 0;

void writec(const char *buf, size_t count) {
    char *v = (char *)CURRENT_START;
    while(count > 0) {
        --count;
        if(*buf == '\n') {
            x = 0;
            y++;
        }
        *v = *buf++;
        v += 2;
        if(x == SCREEN_WIDTH - 1) {
            x = 0;
            y++;
        } else {
            x++;
        }
    }
}
