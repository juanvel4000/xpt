#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpt/info.h>
#include <xpt/lock.h>
#include <xpt/package.h>
#include <xpt/repos.h>
#include <xpt/status.h>

#if XPT_DISABLE_NETWORKING
static int no_networking_error(void)
{
    fprintf(stderr, "libxpt has been built without networking support.\n");
    return XPT_EX_USAGE;
}
#endif

typedef enum {
    ACTION_NONE,
    ACTION_INSTALL,
    ACTION_REMOVE,
    ACTION_GETVER,
    ACTION_LIST,
    ACTION_TREE,
    ACTION_SYNC,
    ACTION_QUERY
} xpt_action_t;

static int action_set(xpt_action_t *action, xpt_action_t new_action)
{
    if (*action != ACTION_NONE) {
        fprintf(stderr, "only one action can be given\n");
        return XPT_EX_USAGE;
    }

    *action = new_action;
    return XPT_EX_OK;
}

static void print_usage(void) { printf("usage: xpt action [options]\n"); }
static void print_version(void)
{
    printf("xpt package tool %s (%s)\n", XPT_VERSION, XPT_ARCH);
}

static void print_help(void)
{

    print_version();
    print_usage();
    printf("\nxpt is a package-management utility designed to install,\n"
           "remove, and administer .xpt packages. it is designed as\n"
           "both a high-level and a medium-level interface for package\n"
           "management.\n\n");
    printf("commands:\n");
    printf("  %-10s %s\n", "-i", "install a package");
    printf("  %-10s %s\n", "-r", "remove a package");
    printf("  %-10s %s\n", "-g", "get a package's version");
    printf("  %-10s %s\n", "-l", "list installed packages");
    printf("  %-10s %s\n", "-t", "list files from a package");
    printf("  %-10s %s\n", "-Q", "query a package");
#if !XPT_DISABLE_NETWORKING
    printf("  %-10s %s\n", "-s", "synchronize network repositories");
#endif
    printf("\n  %-10s %s\n", "-h", "show this message");
    printf("  %-10s %s\n", "-V", "show the xpt version");
    printf("\noptions:\n");
    printf("  %-10s %s\n", "-f <file>", "operate on a file");
    printf("  %-10s %s\n", "-p <pkg>", "operate on an installed package");
#if !XPT_DISABLE_NETWORKING
    printf("  %-10s %s\n", "-n <pkg>", "operate on a network package");
#endif
    printf("  %-10s %s\n\n", "-q", "hide all output except errors");
    printf("view xpt(8) for more information about the available "
           "commands and options\n");
}

static int lock_init(const char *destdir)
{
    int lockfd = xpt_lock_acquire(destdir);
    if (lockfd < 0) {
        fprintf(stderr, "error: could not acquire the lock.\n");
        return -1;
    }

    return lockfd;
}

static void lock_close(int lockfd) { xpt_lock_release(lockfd); }

int main(int argc, char **argv)
{
    int opt;
    xpt_action_t action = ACTION_NONE;
    int lockfd;
    int res;
    int loglevel = XPT_LOG_OK;

    const char *filename = NULL;
    const char *destdir = "/";
    const char *pkgname = NULL;
    const char *netpkg = NULL;
    char *version = NULL;

    while ((opt = getopt(argc, argv, "VBQstlhirqgf:p:n:d:")) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            return XPT_EX_OK;
        case 'B':
            xpt_print_build_info(stdout);
            return XPT_EX_OK;
        case 'i':
            if (action_set(&action, ACTION_INSTALL) != 0)
                return XPT_EX_USAGE;
            break;
        case 'r':
            if (action_set(&action, ACTION_REMOVE) != 0)
                return XPT_EX_USAGE;

            break;
        case 'g':
            if (action_set(&action, ACTION_GETVER) != 0)
                return XPT_EX_USAGE;
            break;
        case 'l':
            if (action_set(&action, ACTION_LIST) != 0)
                return XPT_EX_USAGE;
            break;
        case 't':
            if (action_set(&action, ACTION_TREE) != 0)
                return XPT_EX_USAGE;
            break;
        case 's':
            if (action_set(&action, ACTION_SYNC) != 0)
                return XPT_EX_USAGE;
            break;
        case 'Q':
            if (action_set(&action, ACTION_QUERY) != 0)
                return XPT_EX_USAGE;
            break;
        case 'f':
            filename = optarg;
            break;
        case 'p':
            pkgname = optarg;
            break;
        case 'n':
            netpkg = optarg;
            break;
        case 'd':
            destdir = optarg;
            break;
        case 'q':
            loglevel = XPT_LOG_NO;
            break;
        case 'V':
            print_version();
            return XPT_EX_OK;
        default:
            print_usage();
            return XPT_EX_USAGE;
        }
    }

    switch (action) {
    case ACTION_INSTALL:
        if ((lockfd = lock_init(destdir)) == -1)
            return XPT_EX_NOPERM;

        if (filename && netpkg) {
            fprintf(
                stderr,
                "specify either a filename (-f) or package (-n), not both\n");
            lock_close(lockfd);
            return XPT_EX_USAGE;
        }

        if (filename) {
            res = xpt_package_install(filename, destdir, loglevel, 0);
        } else if (netpkg) {
#if XPT_DISABLE_NETWORKING
            lock_close(lockfd);
            return no_networking_error();
#else
            res = xpt_package_install_from_repo(netpkg, destdir, loglevel);
#endif
        } else {
            fprintf(stderr, "specify a package with -f <file> or -n <pkg>\n");
            res = XPT_EX_USAGE;
        }

        lock_close(lockfd);
        return res;
    case ACTION_REMOVE:
        if ((lockfd = lock_init(destdir)) == -1)
            return XPT_EX_NOPERM;

        if (pkgname == NULL) {
            fprintf(stderr, "specify a package with -p <pkg>\n");
            res = XPT_EX_USAGE;
        } else {
            res = xpt_package_remove(pkgname, destdir, loglevel);
        }
        lock_close(lockfd);
        return res;
    case ACTION_GETVER:
        if (pkgname == NULL) {
            fprintf(stderr, "specify a package with -p <pkg>\n");
            return XPT_EX_USAGE;
        }

        version = xpt_package_getversion(pkgname, destdir);
        if (!version) {
            fprintf(stderr, "package not found\n");
            return XPT_EX_NOINPUT;
        }

        printf("%s@%s\n", pkgname, version);
        free(version);
        return XPT_EX_OK;
    case ACTION_LIST:
        return xpt_package_list(destdir);
    case ACTION_TREE:
        if (pkgname == NULL) {
            fprintf(stderr, "specify a package with -p <pkg>\n");
            return XPT_EX_USAGE;
        }

        return xpt_package_listfiles(pkgname, destdir);
    case ACTION_SYNC:
#if XPT_DISABLE_NETWORKING
        return no_networking_error();
#else
        if ((lockfd = lock_init(destdir)) == -1)
            return XPT_EX_NOPERM;
        res = xpt_repos_sync(destdir);
        lock_close(lockfd);
        return res;
#endif
    case ACTION_QUERY:
        if (pkgname && netpkg) {
            fprintf(stderr,
                    "specify a package with either -n or -p, not both\n");
            return XPT_EX_USAGE;
        }

        if (pkgname) {
            res = xpt_package_printinfo(pkgname, destdir);
            if (res == XPT_EX_NOINPUT) {
                fprintf(stderr, "package not found\n");
                return XPT_EX_NOINPUT;
            }
        } else if (netpkg) {
#if XPT_DISABLE_NETWORKING
            return no_networking_error();
#else
            return xpt_repos_getpkginfo(netpkg, destdir);
#endif
        } else {
            fprintf(stderr, "specify a package with -n or -p\n");
            return XPT_EX_USAGE;
        }

        return res;

    case ACTION_NONE:
        fprintf(stderr, "no action specified\n");
        fprintf(stderr, "view 'xpt -h' for a list of actions\n");
        return XPT_EX_USAGE;
    }

    return XPT_EX_OK;
}
