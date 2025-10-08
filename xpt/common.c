#include <sys/stat.h>
#include <stdio.h.>

int is_file(const char *path) {
    struct stat buf;
    return (stat(path, &buf) == 0 && (buf.st_mode & S_IFREG));
}
