#include <xpt/manager.h>
#include <xpt/common.h>
#include <xpt/database.h>
#include <xpt/repo.h>
#include <xpt/lockfile.h>

#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#ifndef XPT_VERSION
#define XPT_VERSION "v0.4.2"
#endif

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
    /* actions
     * 0: none
     * 1: install
     * 2: get ver
     * 3: remove
     * 4: list
     * 5: package tree
     * 6: sync repos
     * 7: show pkg info
     * */
    int action = 0;
    char destdir[PATH_MAX] = "/";
    int loglevel = LOG_OK;

    if (argc == 1) {
        fprintf(stderr, "usage: xpt <action> [-q] [-d dir] [-f file | -p "
                        "package | -n net-package]\n");
        fprintf(stderr, "try 'xpt -h' for more information.\n");
        return 1;
    }

    while ((opt = getopt(argc, argv, "iqsVghltrIf:p:n:d:")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        case 'i':
            action = 1;
            break;
        case 'p':
            package = optarg;
            break;
        case 'n':
            net_package = optarg;
            break;
        case 'q':
            loglevel = LOG_NO;
            break;
        case 'g':
            action = 2;
            break;
        case 'r':
            action = 3;
            break;
        case 'l':
            action = 4;
            break;
        case 't':
            action = 5;
            break;
        case 's':
            action = 6;
            break;
        case 'V':
            printf("xpt (xpt package tool) %s\n", xptver);
            printf("copyright (c) 2025-2026 juanvel400.\n");
            printf("license BSD-3-Clause "
                   "<http://spdx.org/licenses/BSD-3-Clause.html>\n");
            return 0;
        case 'd':
            snprintf(destdir, sizeof(destdir), "%s", optarg);
            break;
        case 'I':
            action = 7;
            break;
        case 'h':
            printf("usage: xpt -V | -i | -g | -h | -r | -l | -t | -s [-q] [-d "
                   "dir] [-f file | -p package | -n net-package]\n");
            printf("the xpt package tool\n");
            printf("commands:\n");
            printf("  %-10s %s\n", "-V", "show the current xpt version");
            printf("  %-10s %s\n", "-i",
                   "install a package (requires specifying a package with -f "
                   "or -n)");
            printf("  %-10s %s\n", "-g",
                   "get the version of an installed package");
            printf("  %-10s %s\n", "-h", "show this message");
            printf("  %-10s %s\n", "-r", "remove a package");
            printf("  %-10s %s\n", "-l",
                   "list the packages installed in the system");
            printf("  %-10s %s\n", "-t", "show the files used by a package");
            printf("  %-10s %s\n", "-s", "download the repository indexes");
            printf("  %-10s %s\n\n", "-I", "show metadata about a package");

            printf("options:\n");
            printf("  %-10s %s\n", "-f <file>", "specify a file");
            printf("  %-10s %s\n", "-p <pkg>",
                   "specify an installed package name");
            printf("  %-10s %s\n", "-d <dir>",
                   "specify a destination directory for the operations");
            printf("  %-10s %s\n", "-n <pkg>",
                   "specify a package from one of the repositories");
            printf("  %-10s %s\n", "-q",
                   "make the output of most operations quiet (doesn't hide "
                   "errors)");
            return 0;
        }
    }
    int lockfd = xpt_lock_acquire(destdir);
    if (lockfd < 0) {
        fprintf(stderr, "error: could not acquire the lock.");
        return 1;
    }

    if (action == 1) {
        if (filename && net_package) {
            fprintf(stderr, "specify either -f or -n, not both\n");
            xpt_lock_release(lockfd);
            return 1;
        }
        if (filename) {
            int result = package_install(filename, destdir, loglevel, 0);
            xpt_lock_release(lockfd);
            return result;
        } else if (net_package) {
            int result =
                package_install_from_repo(net_package, destdir, loglevel);
            xpt_lock_release(lockfd);
            return result;
        } else {
            fprintf(stderr, "please specify a file with -f <file> or a package "
                            "with -n <pkg>\n");
            xpt_lock_release(lockfd);
            return 1;
        }
    } else if (action == 2) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            xpt_lock_release(lockfd);
            return 1;
        }
        char *version = database_getver(package, destdir);
        if (!version) {
            fprintf(stderr, "package not found\n");
            xpt_lock_release(lockfd);
            return 1;
        }
        printf("%s\n", version);

        xpt_lock_release(lockfd);
        return 0;
    } else if (action == 3) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            xpt_lock_release(lockfd);
            return 1;
        }

        int result = package_uninstall(package, destdir, loglevel);
        xpt_lock_release(lockfd);
        return result;
    } else if (action == 4) {
        int result = database_list(destdir);
        xpt_lock_release(lockfd);
        return result;
    } else if (action == 5) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            xpt_lock_release(lockfd);
            return 1;
        }
        int result = package_listfiles(package, destdir);
        xpt_lock_release(lockfd);
        return result;
    } else if (action == 6) {
        int result = repos_sync(destdir);
        xpt_lock_release(lockfd);
        return result;
    } else if (action == 7) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            xpt_lock_release(lockfd);
            return 1;
        }
        int result = database_printinfo(package, destdir);
        if (result == 1) {
            fprintf(stderr, "package not found\n");
            xpt_lock_release(lockfd);
            return 1;
        }

        xpt_lock_release(lockfd);
        return 0;
    }

    xpt_lock_release(lockfd);
    return 0;
}
