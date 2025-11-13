#include "editor.h"

enum Mode
{
    MODE_NORMAL,
    MODE_HEX_INSERT,
    MODE_ASCII_INSERT,
    MODE_COMMAND
};

typedef struct
{
    filebuffer* fb;
    size_t cursor;
    size_t bytes_per_row;
    size_t row_offset;
    int cursor_nibble;
    Mode mode;
    char status[256];
    char cmd[256];
    int rows, cols;
} editor;

static editor e;

static void set_status(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e.status, sizeof(e.status), fmt, ap);
    va_end(ap);
}

static void move_cursor_left()
{
    if (e.cursor > 0) e.cursor--; 
}

static void move_cursor_right()
{
    if (e.cursor+1 < e.fb->size) e.cursor++;
}

static void move_cursor_up()
{
    if (e.cursor >= e.bytes_per_row) e.cursor -= e.bytes_per_row;
}

static void move_cursor_down()
{
    if (e.cursor + e.bytes_per_row < e.fb->size) e.cursor += e.bytes_per_row;
}

static void ensure_visible()
{
}
