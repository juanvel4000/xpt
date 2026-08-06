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
#endif /* XPT_H */
