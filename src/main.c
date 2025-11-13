#include "core.h"
#include "fileio.h"

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
        return 1;
    }


    close_file(fb);
    
    return 0;
}
