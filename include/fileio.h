#ifndef FILE_IO_H
#define FILE_IO_H

#include "core.h"

typedef struct
{
    int      fd;
    uint8_t* data;
    size_t   size;
    char     filename[MAX_FILENAME_LEN];
} filebuffer;

filebuffer* load_file(const char* path, int writeable);
void        save_file(filebuffer* fb);
void        close_file(filebuffer* fb);

#endif
