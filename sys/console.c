#include <string.h>
#include <sbunix/sbunix.h>
#include <sbunix/console.h>
#include <sbunix/fs/terminal.h>

#define SCRN_BASE ((uint16_t *)kphys_to_virt(0xb8000))
#define SCRN_WIDTH 80U
#define SCRN_HEIGHT 25U
#define SCRN_XY(X, Y) (SCRN_BASE + ((X) + (Y) * SCRN_WIDTH))
#define SCRN_CHAR(ch) (((uint8_t)ch)|((cursor_color)<<8))

static int cursor_x = 0;
static int cursor_y = 0;
static uint16_t cursor_color = 0x07;

void clear_line(uint8_t lineno) {
    uint16_t *v = SCRN_XY(0, lineno);
    int i = 0;

    for(; i < SCRN_WIDTH; i++) {
        v[i] = SCRN_CHAR(' ');
    }
}

/* void update_cursor(int row, int col)
 * by Dark Fiber
 */
void move_csr(void) {
    uint16_t temp;

    temp = (uint16_t)((cursor_y * SCRN_WIDTH) + cursor_x);

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(temp&0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((temp>>8)&0xFF));
}

void clear_console(void) {
    uint16_t *v = SCRN_BASE;
    int i;

    for(i = 0; i < SCRN_WIDTH * SCRN_HEIGHT; i++) {
        v[i] = SCRN_CHAR(' ');
    }
    cursor_x = 0;
    cursor_y = 0;
    move_csr();
}

int curr_tab_to_spaces(void) {
    return ((cursor_x + 4) & ~3) - cursor_x;
}

/*
 * Put a character at the current cursor
 * does not update the VGA cursor
 */
void putch(char c) {
    /* Move cursor back one space */
    if(c == '\b') {
        if(!cursor_x && !cursor_y)
            return;
        cursor_x--;
        if(cursor_x < 0) {
            cursor_x = 79;
            cursor_y--;
        }
    } else if(c == '\t') {
        int spaces = curr_tab_to_spaces();
        while(spaces--)
            putch(' ');
    } else if(c == '\r') {
        cursor_x = 0;
    } else if(c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        /* Write character to the console */
        *SCRN_XY(cursor_x, cursor_y) = SCRN_CHAR(c);
        cursor_x++;
    }

    if(cursor_x >= 80) {
        /* wrap to new line */
        cursor_x = 0;
        cursor_y++;
    }

    /* Need to scroll up */
    if(cursor_y >= SCRN_HEIGHT) {
        /* Don't overwrite time in top right corner */
        memmove(SCRN_BASE, SCRN_XY(0, 1), 2 * (SCRN_WIDTH - 7));
        memmove(SCRN_XY(0, 1), SCRN_XY(0, 2), 2 * (SCRN_WIDTH * (SCRN_HEIGHT - 2)));
        clear_line(SCRN_HEIGHT - 1);
        cursor_y = SCRN_HEIGHT - 1;
    }
}

/*
 * Write a string to the console
 */
void puts(const char *text, size_t count) {
    int i;
    for(i = 0; text[i] && count--; i++){
        putch(text[i]);
    }
    move_csr();
}

void puts_xy(const char *text, size_t count, int x, int y) {
    int oldx, oldy;
    oldx = cursor_x;
    oldy = cursor_y;
    cursor_x = x;
    cursor_y = y;
    while (*text && count--){
        putch(*text++);
    }
    cursor_x = oldx;
    cursor_y = oldy;
}

void writec_time(uint64_t seconds) {
    char strsec[22] = {0};
    uint8_t len;
    /* Convert to string */
    uitoa(seconds, 10, strsec + 1, sizeof(strsec));
    strsec[0] = ' ';
    len = (uint8_t)strlen(strsec);
    /* Write the the top right of the screen */
    puts_xy(strsec, len, SCRN_WIDTH - 3 - len, 0);
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
    puts_xy(glyph, len, SCRN_WIDTH - len, 0);
}

/* Input functions */
static int shift_enabled;       /* If shift is currently pressed */
static int control_enabled;     /* If control is currently pressed */

unsigned char sc_to_ascii(uint8_t scan_code, int shift, int control) {
    unsigned char c;
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

/**
 * Add a scan code
 *
 * NOTE: interrupts are disabled here
 */
void sc_add(uint8_t scan_code) {
    int c;
    /* Print last glyph pressed and add it to the terminal */
    c = sc_getch(scan_code);
    if(c > 0) {
        writec_glyph((char)c);
        term_putch((unsigned char)c);
    }
}


/**
 * Convert a scan_code to an ascii character
 */
int sc_getch(uint8_t scan_code) {
    char c;

    /* Print last glyph pressed */
    if (scan_code == SC_LEFT_SHIFT || scan_code == SC_RIGHT_SHIFT)
        shift_enabled = 1;
    else if (scan_code == (0x80|SC_LEFT_SHIFT) || scan_code == (0x80|SC_RIGHT_SHIFT))
        shift_enabled = 0;
    if (scan_code == SC_LEFT_CONTROL)
        control_enabled = 1;
    else if (scan_code == (0x80|SC_LEFT_CONTROL))
        control_enabled = 0;
    c = sc_to_ascii(scan_code, shift_enabled, control_enabled);
    return c;
}
