#include "editor.h"
#include "ui.h"

typedef enum 
{
    MODE_NORMAL,
    MODE_HEX_INSERT,
    MODE_ASCII_INSERT,
    MODE_COMMAND
} Mode;

typedef struct
{
    filebuffer* fb;
    uint32_t cursor;
    uint32_t bytes_per_row;
    uint32_t row_offset;
    int cursor_nibble;
    Mode mode;
    char status[256];
    char cmd[256];
    int rows, cols;
} editor;

void init_ui()
{
    initscr();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
}

void end_ui()
{
    endwin();
}

void clear_screen()
{
    clear();
    refresh();
}

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
    uint32_t row = e.cursor / e.bytes_per_row;
    uint32_t top = e.row_offset / e.bytes_per_row;
    uint32_t visible_rows = (uint32_t)e.rows - 2;
    if (row < top)
    {
        e.row_offset = row * e.bytes_per_row;
    } 
    else if (row >= top + visible_rows)
    {
        e.row_offset = (row - visible_rows + 1) * e.bytes_per_row;    
    }
}

static void draw_screen()
{
    werase(stdscr);
    uint32_t bytes = e.bytes_per_row;
    uint32_t top_row = e.row_offset / bytes;
    uint32_t visible_rows = (uint32_t) e.rows - 2;

    for (uint32_t r = 0; r < visible_rows; r++)
    {
        uint32_t offset = (top_row + r) * bytes;
        if (offset >= e.fb->size)
        {
            mvprintw(r, 0, "~");
            continue;
        }

        mvprintw(r, 0, "%08zx:\t", offset);
        
        int col = 12;
        for (uint32_t i = 0; i < bytes; i++)
        {
            uint32_t idx = offset + i;
            
            if (idx >= e.fb->size)
            {
                mvprintw(r, col, " ");
            } else {
                if (idx == e.cursor && e.mode == MODE_HEX_INSERT) attron(A_REVERSE);
                mvprintw(r, col, "%02X", e.fb->data[idx]);
                if (idx == e.cursor && e.mode == MODE_HEX_INSERT) attroff(A_REVERSE);
            }
            col += 3;
        }

        col += 4;
        for (uint32_t i = 0; i < bytes && offset + i < e.fb->size; i++)
        {
            uint8_t c = e.fb->data[offset + i];
            char display = isprint(c) ? c : '.';
            if (offset + i == e.cursor && e.mode == MODE_ASCII_INSERT) attron(A_REVERSE);
            mvaddch(r, col + i, display);
            if (offset + i == e.cursor && e.mode == MODE_ASCII_INSERT) attroff(A_REVERSE);
        }

        mvhline(e.rows - 2, 0, 0, e.cols);
        mvprintw(e.rows - 2, 0, "%s", e.status);

        if (e.mode == MODE_COMMAND)
        {
            mvprintw(e.rows - 1, 0, "%s", e.cmd);
        }
        else if (e.mode == MODE_HEX_INSERT)
        {
            mvprintw(e.rows - 1, 0, "--- HEX INSERT ---");
        }
        else if (e.mode == MODE_ASCII_INSERT)
        {
            mvprintw(e.rows - 1, 0, "--- ASCII INSERT ---");
        }
        else
        {
            mvprintw(e.rows - 1, 0, "--- NORMAL ---");
        }

        refresh();
    }
}

static void handle_command()
{
    if (strcmp(e.cmd, "w") == 0) 
    {
        save_file(e.fb);
        set_status("Wrote %s", e.fb->filename);
    } else if (strcmp(e.cmd, "q") == 0) {
        end_ui();
        close_file(e.fb);
        exit(0);
    } else if (strcmp(e.cmd, "wq") == 0 || strcmp(e.cmd, "x") == 0) {
        save_file(e.fb);
        end_ui();
        close_file(e.fb);
        exit(0);
    } else {
        set_status("Unknown command: %s", e.cmd);
    }
}

static void process_key(int ch)
{
    switch (e.mode)
    {
        case MODE_NORMAL:
            switch(ch)
            {
                case 'h' : move_cursor_left(); break;
                case 'l' : move_cursor_right(); break;
                case 'j' : move_cursor_down(); break;
                case 'k' : move_cursor_up(); break;
                case KEY_LEFT : move_cursor_left(); break;
                case KEY_RIGHT : move_cursor_right(); break;
                case KEY_DOWN : move_cursor_down(); break;
                case KEY_UP : move_cursor_up(); break;
                case 'a' : e.mode = MODE_ASCII_INSERT; break;
                case 'i' : e.mode = MODE_HEX_INSERT; break;
                case ':' : e.mode = MODE_COMMAND; break;
            }
            break;

        case MODE_HEX_INSERT:
            if (ch == 27) { e.mode = MODE_NORMAL; break; }
            if (isxdigit(ch))
            {
                int value = (ch <= '9') ? ch - '0' : (tolower(ch) - 'a' + 10);
                if (e.cursor < e.fb->size)
                {
                    if (e.cursor_nibble == 0)
                    {
                        e.fb->data[e.cursor] = (e.fb->data[e.cursor] & 0x0F) | (value << 4);
                        e.cursor_nibble = 1;
                    } 
                    else
                    {
                        e.fb->data[e.cursor] = (e.fb->data[e.cursor] & 0xF0) | value;
                        e.cursor_nibble = 0;
                        move_cursor_right();
                    }
                }
            }
            break;

        case MODE_ASCII_INSERT:
            if (ch == 27) { e.mode = MODE_NORMAL; break; }
            if (isprint(ch) && e.cursor < e.fb->size)
            {
                e.fb->data[e.cursor] = (uint8_t)ch;
                move_cursor_right();
            }
            break;

        case MODE_COMMAND:
            if (ch == 27) { e.mode = MODE_NORMAL; break; }
            if (ch == '\n') { handle_command(); e.mode = MODE_NORMAL; break; }
            if ((ch == KEY_BACKSPACE || ch == 127) && strlen(e.cmd))
            {
                e.cmd[strlen(e.cmd)-1] = '\0';
            }
            else if (isprint(ch) && strlen(e.cmd) + 1 < sizeof(e.cmd))
            {
                size_t len = strlen(e.cmd);
                e.cmd[len] = (char) ch;
                e.cmd[len+1] = '\0';
            }
            break;
    }

    ensure_visible();
}

static void handle_resize(int sig)
{
    (void) sig;
    getmaxyx(stdscr, e.rows, e.cols);
}

void run_editor(filebuffer* fb)
{
    e.fb = fb;
    e.cursor = 0;
    e.bytes_per_row = BYTES_PER_ROW;
    e.row_offset = 0;
    e.cursor_nibble = 0;
    e.mode = MODE_NORMAL;
    e.status[0] = '\0';
    getmaxyx(stdscr, e.rows, e.cols);

    signal(SIGWINCH, handle_resize);

    set_status("Opened %s (%zu bytes)", fb->filename, fb->size);

    while(1)
    {
        draw_screen();
        int ch = getch();
        process_key(ch);
    }
}
