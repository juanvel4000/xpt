#ifndef XPT_H
#define XPT_H

#define XPT_LOG_OK 1
#define XPT_LOG_NO 0

#define XPT_EX_OK 0        /* ok */
#define XPT_EX_USAGE 1     /* bad flags / user error */
#define XPT_EX_NOINPUT 2   /* not found */
#define XPT_EX_DATAERR 3   /* invalid entry */
#define XPT_EX_IOERR 4     /* io failure */
#define XPT_EX_CANTCREAT 5 /* couldn't create item */
#define XPT_EX_TEMPFAIL 6  /* network / download failure */
#define XPT_EX_PROTOCOL 7  /* checksum mismatch */
#define XPT_EX_NOPERM 8    /* lock held */
#define XPT_EX_SOFTWARE 9  /* internal err */
#define XPT_EX_EXISTS 10   /* exists */

#include <stdio.h>

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

/**
 * install a package
 *
 * @param file the package file
 * @param destdir the root directory to install into
 * @param log the log level
 * @param netinstall_deps whether to use the network subsystem to install
 * dependencies
 *
 * @return XPT_EX_*
 */
int xpt_package_install(const char *file, const char *destdir, int log,
                        int netinstall_deps);

/**
 * uninstall a package
 *
 * @param name the package name in database
 * @param destdir the root directory to uninstall from
 * @param log the log level
 *
 * @return XPT_EX_*
 */
int xpt_package_remove(const char *name, const char *destdir, int log);

/**
 * list files installed by a package
 *
 * @param name the package name in database
 * @param destdir the root directory to read from
 *
 * @return XPT_EX_*
 */
int xpt_package_listfiles(const char *name, const char *destdir);

/**
 * install a package from the network repositories
 *
 * @param name the package name
 * @param destdir the root directory to install into
 * @param log the log level
 *
 * @return XPT_EX_*
 */
int xpt_package_install_from_repo(const char *name, const char *destdir,
                                  int log);

/**
 * list all packages
 *
 * @param destdir the root directory to read from
 *
 * @return XPT_EX_*
 */
int xpt_package_list(const char *destdir);

/**
 * print information about a package
 *
 * @param package the package to read from
 * @param destdir the root directory to read from
 *
 * @return XPT_EX_*
 */
int xpt_package_printinfo(const char *package, const char *destdir);

/**
 * get a package version
 *
 * @param package the package to read from
 * @param destdir the root directory to read from
 *
 * @return the package version
 */
char *xpt_package_getversion(const char *package, const char *destdir);

/**
 * synchronize the network repositories
 *
 * @param destdir the root directory containing the repository data
 *
 * @return XPT_EX_*
 */
int xpt_repos_sync(const char *destdir);

/**
 * get information about a package from the network
 *
 * @param name the package to read from
 * @param destdir the root directory to read from
 *
 * @return XPT_EX_*
 */
int xpt_repos_getpkginfo(const char *name, const char *destdir);

/**
 * print build information about xpt
 *
 * @param out the file to print into
 */
void xpt_print_build_info(FILE *out);

#endif /* XPT_H */
