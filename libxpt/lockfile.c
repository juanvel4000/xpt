#include "config.h"
#include <fcntl.h>
#include <internal/common.h>
#include <limits.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>

int xpt_lock_acquire(const char *destdir)
{
    char dir[PATH_MAX];
    char path[PATH_MAX];
    int fd;

    if (snprintf(dir, sizeof(dir), "%s/%s/lib/xpt", destdir,
                 XPT_LOCALSTATEDIR) >= (int)sizeof(dir))
        return -1;

    if (mkdir_p(dir) == -1)
        return -1;

    if (snprintf(path, sizeof(path), "%s/xpt.lock", dir) >= (int)sizeof(path))
        return -1;

    fd = open(path, O_CREAT | O_RDWR, 0644);
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
