#ifndef _SBUNIX_WRITEC_H
#define _SBUNIX_WRITEC_H 1

#include <sys/types.h>

void writec_xy(const char *buf, size_t count, uint8_t x, uint8_t y, int save_xy);
void writec(const char *buf, size_t count);
void writec_time(uint64_t seconds);
void writec_glyph(char c);
void fill_screen(void);


void sc_buf_add(uint8_t scan_code);

/* Scan Code Set 1 */
enum scan_codes {
    SC_ESCAPE           = 0x01,
    SC_1                = 0x02,
    SC_2                = 0x03,
    SC_3                = 0x04,
    SC_4                = 0x05,
    SC_5                = 0x06,
    SC_6                = 0x07,
    SC_7                = 0x08,
    SC_8                = 0x09,
    SC_9                = 0x0A,
    SC_0                = 0x0B,
    SC_HYPHEN           = 0x0C,
    SC_EQUAL            = 0x0D,
    SC_BACKSPACE        = 0x0E,
    SC_TAB              = 0x0F,
    SC_Q                = 0x10,
    SC_W                = 0x11,
    SC_E                = 0x12,
    SC_R                = 0x13,
    SC_T                = 0x14,
    SC_Y                = 0x15,
    SC_U                = 0x16,
    SC_I                = 0x17,
    SC_O                = 0x18,
    SC_P                = 0x19,
    SC_SQ_BRACKET_LEFT  = 0x1A,
    SC_SQ_BRACKET_RIGHT = 0x1B,
    SC_ENTER            = 0x1C,
    SC_LEFT_CONTROL     = 0x1D,
    SC_A                = 0x1E,
    SC_S                = 0x1F,
    SC_D                = 0x20,
    SC_F                = 0x21,
    SC_G                = 0x22,
    SC_H                = 0x23,
    SC_J                = 0x24,
    SC_K                = 0x25,
    SC_L                = 0x26,
    SC_SEMICOLON        = 0x27,
    SC_SINGLE_QUOTE     = 0x28,
    SC_BACK_TICK        = 0x29,
    SC_LEFT_SHIFT       = 0x2A,
    SC_BACKSLASH        = 0x2B,
    SC_Z                = 0x2C,
    SC_X                = 0x2D,
    SC_C                = 0x2E,
    SC_V                = 0x2F,
    SC_B                = 0x30,
    SC_N                = 0x31,
    SC_M                = 0x32,
    SC_COMMA            = 0x33,
    SC_DOT              = 0x34,
    SC_FORWARDSLASH     = 0x35,
    SC_RIGHT_SHIFT      = 0x36,
    SC_KEYPAD_STAR      = 0x37,
    SC_LEFT_ALT         = 0x38,
    SC_SPACE            = 0x39,
    SC_CAPSLOCK         = 0x3A,
    SC_F1               = 0x3B,
    SC_F2               = 0x3C,
    SC_F3               = 0x3D,
    SC_F4               = 0x3E,
    SC_F5               = 0x3F,
    SC_F6               = 0x40,
    SC_F7               = 0x41,
    SC_F8               = 0x42,
    SC_F9               = 0x43,
    SC_F10              = 0x44,
    SC_NUMBERLOCK       = 0x45,
    SC_SCROLLLOCK       = 0x46,
    SC_KEYPAD_7         = 0x47,
    SC_KEYPAD_8         = 0x48,
    SC_KEYPAD_9         = 0x49,
    SC_KEYPAD_HYPHEN    = 0x4A,
    SC_KEYPAD_4         = 0x4B,
    SC_KEYPAD_5         = 0x4C,
    SC_KEYPAD_6         = 0x4D,
    SC_KEYPAD_PLUS      = 0x4E,
    SC_KEYPAD_1         = 0x4F,
    SC_KEYPAD_2         = 0x50,
    SC_KEYPAD_3         = 0x51,
    SC_KEYPAD_0         = 0x52,
    SC_DELETE           = 0x53,
    SC_F11              = 0x57,
    SC_F12              = 0x58
};

#endif
