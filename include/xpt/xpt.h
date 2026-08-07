#ifndef XPT_H
#define XPT_H

#include <stdio.h>

int xpt_lock_acquire(const char *destdir);
void xpt_lock_release(int fd);

#define XPT_LOG_OK 1
#define XPT_LOG_NO 0

int xpt_package_install(const char *file, const char *destdir, int log,
                    int netinstall_deps);
int xpt_package_remove(const char *name, const char *destdir, int log);
int xpt_package_listfiles(const char *name, const char *destdir);
int xpt_package_install_from_repo(const char *name, const char *destdir, int log);

int xpt_package_list(const char *destdir);
int xpt_package_printinfo(const char *package, const char *destdir);
char *xpt_package_getversion(const char *package, const char *destdir);

int xpt_repos_sync(const char *destdir);

void xpt_print_build_info(FILE *out);

#define XPT_EX_OK          0  /* ok */
#define XPT_EX_USAGE       1  /* bad flags / user error */
#define XPT_EX_NOINPUT     2  /* not found */
#define XPT_EX_DATAERR     3  /* invalid entry */
#define XPT_EX_IOERR       4  /* io failure */
#define XPT_EX_CANTCREAT   5  /* couldn't create item */
#define XPT_EX_TEMPFAIL    6  /* network / download failure */
#define XPT_EX_PROTOCOL    7  /* checksum mismatch */
#define XPT_EX_NOPERM      8  /* lock held */
#define XPT_EX_SOFTWARE    9  /* internal err */
#define XPT_EX_EXISTS      10 /* exists */
#endif /* XPT_H */
