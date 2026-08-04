#ifndef XPT_H
#define XPT_H

int xpt_lock_acquire(const char *destdir);
void xpt_lock_release(int fd);

#define LOG_OK 1
#define LOG_NO 0

int package_install(const char *file, const char *destdir, int log,
                    int netinstall_deps);
int package_uninstall(const char *name, const char *destdir, int log);
int package_listfiles(const char *name, const char *destdir);
int package_install_from_repo(const char *name, const char *destdir, int log);

int database_list(const char *destdir);
int database_printinfo(const char *package, const char *destdir);
char *database_getver(const char *package, const char *destdir);

int repos_sync(const char *destdir);

#endif /* XPT_H */
