#ifndef XPT_REPOS_H
#define XPT_REPOS_H

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

#endif /* XPT_REPOS_H */
