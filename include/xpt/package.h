#ifndef XPT_PACKAGE_H
#define XPT_PACKAGE_H

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

#endif /* XPT_PACKAGE_H */
