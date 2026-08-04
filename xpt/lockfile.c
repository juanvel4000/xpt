#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

int xpt_lock_acquire(const char *destdir)
{
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/.xpt.lock", destdir);

    int fd = open(path, O_CREAT | O_RDWR, 0644);
    if (fd < 0)
        return -1;

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

void xpt_lock_release(int fd)
{
    flock(fd, LOCK_UN);
    close(fd);
}
