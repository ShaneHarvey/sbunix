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
    char strsec[22] = {0};
    uint8_t len;
    /* Convert to string */
    uitoa(seconds, 10, strsec + 1, sizeof(strsec));
    strsec[0] = ' ';
    len = (uint8_t)strlen(strsec);
    /* Write the the top right of the screen */
    writec_xy(strsec, len, SCREEN_WIDTH - 3 - len, 0, 0);
}

void writec_glyph(char c) {
    char glyph[4] = {0};
    uint8_t len = 3;
    glyph[0] = ' ';
    if(c >= ' ' && c <= '~') {
        glyph[1] = ' ';
        glyph[2] = c;
    } else if (c >= '\0' && c < ' '){
        glyph[1] = '^';
        glyph[2] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"[(uint8_t)c];
    } else if (c == 0x7F) {
        glyph[1] = '^';
        glyph[2] = '?';
    } else {
        glyph[1] = '?';
        glyph[2] = '?';
    }

    /* Write the the top right of the screen */
    writec_xy(glyph, len, SCREEN_WIDTH - len, 0, 0);
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

/* Input functions */

#define SC_BUFSIZE 1024
struct sc_buf {
    int start;                /* Read head of the buffer  (first occupied cell) */
    int end;                  /* Write head of the buffer (next empty cell) */
    int full;                 /* If the buffer is full */
    int shift;                /* If shift is currently pressed */
    int control;              /* If control is currently pressed */
    uint8_t buf[SC_BUFSIZE];  /* Buffer holding scan codes */
};

struct sc_buf sb = {0, 0, 0, 0, 0, {0}};

char sc_to_ascii(uint8_t scan_code, int shift, int control) {
    char c;
    if (scan_code == SC_ESCAPE) {
        c = 0x05;
    } else if (scan_code <= SC_ENTER) {
        if (!shift)
            c = "1234567890-=\b\tqwertyuiop[]\n"[scan_code - SC_1];
        else
            c = "!@#$%^&*()_+\b\tQWERTYUIOP{}\n"[scan_code - SC_1];
    } else if (scan_code >= SC_A && scan_code <= SC_BACK_TICK) {
        if (!shift)
            c = "asdfghjkl;'`"[scan_code - SC_A];
        else
            c = "ASDFGHJKL:\"~"[scan_code - SC_A];
    } else if (scan_code >= SC_BACKSLASH && scan_code <= SC_FORWARDSLASH) {
        if (!shift)
            c = "\\zxcvbnm,./"[scan_code - SC_BACKSLASH];
        else
            c = "|ZXCVBNM<>?"[scan_code - SC_BACKSLASH];
    } else if (scan_code == SC_KEYPAD_STAR) {
        c = '*';
    } else if (scan_code == SC_SPACE) {
        c = ' ';
    } else if (scan_code >= SC_KEYPAD_7 && scan_code <= SC_DELETE) {
        c = "789-456+1230\x7F"[scan_code - SC_KEYPAD_7];
    } else {
        c = '\0';
    }
    if(control) {
        //@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
        switch (scan_code) {
            case SC_2: c = 0; break;
            case SC_A: c = 1; break;
            case SC_B: c = 2; break;
            case SC_C: c = 3; break;
            case SC_D: c = 4; break;
            case SC_E: c = 5; break;
            case SC_F: c = 6; break;
            case SC_G: c = 7; break;
            case SC_H: c = 8; break;
            case SC_I: c = 9; break;
            case SC_J: c = 10; break;
            case SC_K: c = 11; break;
            case SC_L: c = 12; break;
            case SC_M: c = 13; break;
            case SC_N: c = 14; break;
            case SC_O: c = 15; break;
            case SC_P: c = 16; break;
            case SC_Q: c = 17; break;
            case SC_R: c = 18; break;
            case SC_S: c = 19; break;
            case SC_T: c = 20; break;
            case SC_U: c = 21; break;
            case SC_V: c = 22; break;
            case SC_W: c = 23; break;
            case SC_X: c = 24; break;
            case SC_Y: c = 25; break;
            case SC_Z: c = 26; break;
            case SC_SQ_BRACKET_LEFT: c = 27; break;
            case SC_BACKSLASH: c = 28; break;
            case SC_SQ_BRACKET_RIGHT: c = 29; break;
            case SC_6: c = 30; break;
            case SC_HYPHEN: c = 31; break;
            default:
                break;
        }
    }
    return c;
}

void sc_buf_add(uint8_t scan_code) {
    char c;
    if(sb.full) {
        return;
    }
    sb.buf[sb.end] = scan_code;
    sb.end = (sb.end + 1) % SC_BUFSIZE;
    if(sb.end == sb.start) {
        sb.full = 1;
    }
    /* Print last glyph pressed */
    scan_code = sb.buf[sb.start];
    if (scan_code == SC_LEFT_SHIFT || scan_code == SC_RIGHT_SHIFT)
        sb.shift = 1;
    else if (scan_code == (0x80|SC_LEFT_SHIFT) || scan_code == (0x80|SC_RIGHT_SHIFT))
        sb.shift = 0;
    if (scan_code == SC_LEFT_CONTROL)
        sb.control = 1;
    else if (scan_code == (0x80|SC_LEFT_CONTROL))
        sb.control = 0;
    c = sc_to_ascii(sb.buf[sb.start], sb.shift, sb.control);
    sb.start = (sb.start + 1) % SC_BUFSIZE;
    sb.full = 0;
    if(c)
        writec_glyph(c);
}
