#include <xpt/xpt.h>

#include "config.h"
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char *xptver;
    if (XPT_VERSION[0] == 'v') {
        xptver = XPT_VERSION + 1;
    } else {
        xptver = XPT_VERSION;
    }
    int opt;
    char *filename = NULL;
    char *package = NULL;
    char *net_package = NULL;
    typedef enum {
        ACTION_NONE,
        ACTION_INSTALL,
        ACTION_GETVER,
        ACTION_REMOVE,
        ACTION_LIST,
        ACTION_TREE,
        ACTION_SYNC,
        ACTION_INFO,
        ACTION_NPKGINFO
    } xpt_action_t;

    xpt_action_t action = ACTION_NONE;

    char destdir[PATH_MAX] = "/";
    int loglevel = XPT_LOG_OK;

    if (argc == 1) {
        fprintf(stderr, "usage: xpt command [options]\n");
        fprintf(stderr, "try 'xpt -h' for more information.\n");
        return XPT_EX_USAGE;
    }

    while ((opt = getopt(argc, argv, "iqsVghltrIBQf:p:n:d:")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        case 'i':
            action = ACTION_INSTALL;
            break;
        case 'p':
            package = optarg;
            break;
        case 'n':
#if XPT_DISABLE_NETWORKING
            fprintf(stderr,
                    "libxpt has been built without networking support.\n");
            return XPT_EX_USAGE;
#endif
            net_package = optarg;
            break;
        case 'q':
            loglevel = XPT_LOG_NO;
            break;
        case 'g':
            action = ACTION_GETVER;
            break;
        case 'r':
            action = ACTION_REMOVE;
            break;
        case 'l':
            action = ACTION_LIST;
            break;
        case 't':
            action = ACTION_TREE;
            break;
        case 's':
#if XPT_DISABLE_NETWORKING
            fprintf(stderr,
                    "libxpt has been built without networking support.\n");
            return XPT_EX_USAGE;
#endif
            action = ACTION_SYNC;
            break;
        case 'Q':
#if XPT_DISABLE_NETWORKING
            fprintf(stderr,
                    "libxpt has been built without networking support.\n");
            return XPT_EX_USAGE;
#endif
            action = ACTION_NPKGINFO;
            break;
        case 'V':
            printf("xpt (xpt package tool) v%s\n", xptver);
            printf("copyright (c) 2025-2026 juanvel400.\n");
            printf("license BSD-3-Clause "
                   "<http://spdx.org/licenses/BSD-3-Clause.html>\n");
#if XPT_DISABLE_NETWORKING
            printf("libxpt has been built without networking support.\n");
#endif

            return XPT_EX_OK;
        case 'd':
            snprintf(destdir, sizeof(destdir), "%s", optarg);
            break;
        case 'I':
            action = ACTION_INFO;
            break;
        case 'B':
            xpt_print_build_info(stdout);
            return XPT_EX_OK;
        case 'h':
            printf("xpt v%s\n", xptver);
            printf("usage: xpt command [options]\n\n");
            printf(
                "xpt is a package management utility designed to install,\n"
                "remove and administer .xpt packages. it is designed as\n"
                "both a high-level and a medium-level interface for package\n"
                "management.\n\n");
            printf("commands:\n");
            printf("  %-10s %s\n", "-V", "show the current xpt version");
            printf("  %-10s %s\n", "-i", "install a package");
            printf("  %-10s %s\n", "-g",
                   "get the version of an installed package");
            printf("  %-10s %s\n", "-h", "show this message");
            printf("  %-10s %s\n", "-r", "remove a package");
            printf("  %-10s %s\n", "-l",
                   "list the packages installed in the system");
            printf("  %-10s %s\n", "-t", "show the files used by a package");
#if !XPT_DISABLE_NETWORKING
            printf("  %-10s %s\n", "-s", "download the repository indexes");
            printf("  %-10s %s\n", "-Q",
                   "get information of a package in a repository");
#endif
            printf("  %-10s %s\n", "-I", "show metadata about a package");
            printf("  %-10s %s\n\n", "-B", "print the libxpt build info");

            printf("options:\n");
            printf("  %-10s %s\n", "-f <file>",
                   "operate on a .xpt package file");
            printf("  %-10s %s\n", "-p <pkg>",
                   "operate on an installed package");
#if !XPT_DISABLE_NETWORKING
            printf("  %-10s %s\n", "-n <pkg>", "operate on a network package");
#endif
            printf("  %-10s %s\n", "-d <dir>",
                   "specify a destination directory for the operations");
            printf("  %-10s %s\n", "-q",
                   "make the output of most operations quiet (doesn't hide "
                   "errors)");
            printf("\nsee xpt(8) for more information about the available "
                   "commands\n");
            return XPT_EX_OK;
        default:
            fprintf(stderr, "usage: xpt <action> [-q] [-d dir] [-f file | -p "
                            "package | -n net-package]\n");
            return XPT_EX_USAGE;
        }
    }

    if (action == ACTION_INSTALL) {
        int lockfd = xpt_lock_acquire(destdir);
        if (lockfd < 0) {
            fprintf(stderr, "error: could not acquire the lock.\n");
            return XPT_EX_NOPERM;
        }

        if (filename && net_package) {
            fprintf(stderr, "specify either -f or -n, not both\n");
            xpt_lock_release(lockfd);
            return XPT_EX_USAGE;
        }
        if (filename) {
            int result = xpt_package_install(filename, destdir, loglevel, 0);
            xpt_lock_release(lockfd);
            return result;
        } else if (net_package) {
            int result =
                xpt_package_install_from_repo(net_package, destdir, loglevel);
            xpt_lock_release(lockfd);
            return result;
        } else {
            fprintf(stderr, "please specify a file with -f <file> or a package "
                            "with -n <pkg>\n");
            xpt_lock_release(lockfd);
            return XPT_EX_USAGE;
        }
    } else if (action == ACTION_GETVER) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            return XPT_EX_USAGE;
        }
        char *version = xpt_package_getversion(package, destdir);
        if (!version) {
            fprintf(stderr, "package not found\n");
            return XPT_EX_NOINPUT;
        }
        printf("%s\n", version);

        return XPT_EX_OK;
    } else if (action == ACTION_REMOVE) {
        int lockfd = xpt_lock_acquire(destdir);
        if (lockfd < 0) {
            fprintf(stderr, "error: could not acquire the lock.\n");
            return XPT_EX_NOPERM;
        }
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            xpt_lock_release(lockfd);
            return XPT_EX_USAGE;
        }

        int result = xpt_package_remove(package, destdir, loglevel);
        xpt_lock_release(lockfd);
        return result;
    } else if (action == ACTION_LIST) {
        int result = xpt_package_list(destdir);
        return result;
    } else if (action == ACTION_TREE) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            return XPT_EX_USAGE;
        }
        int result = xpt_package_listfiles(package, destdir);
        return result;
    } else if (action == ACTION_SYNC) {
#if !XPT_DISABLE_NETWORKING
        int lockfd = xpt_lock_acquire(destdir);
        if (lockfd < 0) {
            fprintf(stderr, "error: could not acquire the lock.\n");
            return XPT_EX_NOPERM;
        }
        int result = xpt_repos_sync(destdir);
        xpt_lock_release(lockfd);
        return result;
#else
        fprintf(stderr, "libxpt has been built without networking support.\n");
        return XPT_EX_USAGE;
#endif
    } else if (action == ACTION_INFO) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            return XPT_EX_USAGE;
        }
        int result = xpt_package_printinfo(package, destdir);
        if (result == 1) {
            fprintf(stderr, "package not found\n");
            return XPT_EX_NOINPUT;
        }

        return XPT_EX_OK;
    } else if (action == ACTION_NPKGINFO) {
#if !XPT_DISABLE_NETWORKING
        if (net_package == NULL) {
            fprintf(stderr, "please specify a network-package with -n <pkg>\n");
            return XPT_EX_USAGE;
        }

        xpt_repos_getpkginfo(net_package, destdir);
        return XPT_EX_OK;
#else
        fprintf(stderr, "libxpt has been built without networking support.\n");
        return XPT_EX_USAGE;
#endif
    } else if (action == ACTION_NONE) {
        fprintf(stderr, "no action specified; try 'xpt -h'\n");
        return XPT_EX_USAGE;
    }

    return XPT_EX_OK;
}
