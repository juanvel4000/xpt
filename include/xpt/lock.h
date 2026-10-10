#ifndef XPT_LOCK_H
#define XPT_LOCK_H

/**
 * acquire the manager lock
 *
 * @param destdir the root directory containing the lock
 *
 * @return the lock file descriptor
 */
int xpt_lock_acquire(const char *destdir);

/**
 * release the manager lock
 *
 * @param fd the file descriptor of the lock
 */
void xpt_lock_release(int fd);

#endif /* XPT_LOCK_H */
