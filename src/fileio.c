#include "fileio.h"

filebuffer* load_file(const char* path, int writeable)
{
    int flags = writeable ? O_RDWR : O_RDONLY;
    int prot  = writeable ? (PROT_READ | PROT_WRITE) : PROT_READ;

    int fd    = open(path, flags);
    if (fd < 0)
    {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        close(fd);
        return NULL;
    }

    if (st.st_size == 0)
    {
        fprintf(stderr, "File is empty\n");
        close(fd);
        return NULL;
    }

    uint8_t* data = mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return NULL;
    }

    filebuffer* fb = malloc(sizeof(filebuffer));
    fb->fd         = fd;
    fb->data       = data;
    fb->size       = st.st_size;
    strncpy(fb->filename, path, sizeof(fb->filename) - 1);
    fb->filename[sizeof(fb->filename) - 1] = '\0';

    return fb;
}

void save_file(filebuffer* fb)
{
    if (msync(fb->data, fb->size, MS_SYNC) < 0)
    {
        perror("msync");
    }
}

void close_file(filebuffer* fb)
{
    if (!fb)
        return;

    if (munmap(fb->data, fb->size) < 0)
    {
        perror("munmap");
    }

    close(fb->fd);
    free(fb);
}
