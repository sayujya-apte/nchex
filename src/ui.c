#include "ui.c"

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
