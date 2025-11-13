#include "core.h"
#include "fileio.h"
#include "ui.h"
#include "editor.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    filebuffer* fb = load_file(argv[1], 1);

    if (!fb)
    {
        fprintf(stderr, "Error: Could not open file '%s'\n", argv[1]);
        return 1;
    }

    init_ui();
    run_editor(fb);
    end_ui();

    close_file(fb);
    
    return 0;
}
