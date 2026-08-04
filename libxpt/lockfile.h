#ifndef LOCKFILE_H
#define LOCKFILE_H

int xpt_lock_acquire(const char *destdir);
void xpt_lock_release(int fd);

#endif /* LOCKFILE_H */
