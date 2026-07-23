#include <stdio.h>
#include <manager.h>
#include <unistd.h>
#include <limits.h>
#include <common.h>
#include <database.h>
#include <getopt.h>

#ifndef XPT_VERSION
    #define XPT_VERSION "v0.1.0"
#endif

static struct option long_options[] = {
    {"help",    no_argument,       0, 'h'},
    {"install", no_argument,       0, 'i'},
    {"remove",  no_argument,       0, 'r'},
    {"file",    required_argument, 0, 'f'},
    {"package", required_argument, 0, 'p'},
    {"destdir", required_argument, 0, 'd'},
    {"quiet",   no_argument,       0, 'q'},
    {"version", no_argument,       0, 'V'},
    {"get-version", no_argument,   0, 'g'},
    {"list-packages", no_argument, 0, 'l'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    char *xptver;
    if (XPT_VERSION[0] == 'v') {
        xptver = XPT_VERSION + 1;
    } else {
        xptver = XPT_VERSION;
    }
    int opt;
    char *filename = NULL;
    char *package = NULL;
    /* actions
     * 0: none
     * 1: install
     * 2: get ver
     * 3: remove
     * 4: list
     * */
    int action = 0;
    char destdir[PATH_MAX] = "/";
    int loglevel = LOG_OK;
    if (argc == 1) {
        fprintf(stderr, "usage: xpt <CMD> [OPTION]...\n");
        fprintf(stderr, "try 'xpt --help' for more information.\n");
        return 1;
    }
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "iqVghlrf:p:d:", long_options, &option_index)) != -1) {
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
            case 'V':
                printf("xpt (xpt package tool) %s\n", xptver);
                printf("copyright (c) 2025-2026 juanvel400.\n");
                printf("license BSD-3-Clause <http://spdx.org/licenses/BSD-3-Clause.html>\n");
                return 0;
            case 'd':
                snprintf(destdir, sizeof(destdir), "%s", optarg);
                break;
            case 'h':
                printf("usage: xpt <CMD> [OPTION]...\n");
                printf("the xpt package tool\n");
                printf("example: xpt --install --file ./ebt-0.1.0.xpt \n\n");
                printf("commands:\n");
                printf("  %-22s %s\n", "-V, --version", "show the current xpt version");
                printf("  %-22s %s\n", "-i, --install", "install <file> (requires specifying file with -f)");
                printf("  %-22s %s\n", "-g, --get-version", "get the version of an installed package");
                printf("  %-22s %s\n", "-h, --help", "show this message");
                printf("  %-22s %s\n", "-r, --remove", "remove a package");
                printf("  %-22s %s\n\n", "-l, --list-packages", "list the packages installed in the system");

                printf("options:\n");
                printf("  %-22s %s\n", "-f, --file", "<file>  specify a file");
                printf("  %-22s %s\n", "-p, --package <pkg>", "specify an installed package name");
                printf("  %-22s %s\n", "-d, --destdir <dir>", "specify a destination directory for the operations");
                printf("  %-22s %s\n", "-q, --quiet", "make the output of most operations quiet (doesn't hide errors)");
                return 0;
        }
    }
    if (action == 1) {
        if (filename == NULL) {
            fprintf(stderr, "please specify a file with -f <file>\n");
            return 1;
        }
        return package_install(filename, destdir, loglevel);
    }else if (action == 2) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            return 1;
        }
        char* version = database_getver(package, destdir);
        if (!version) {
            fprintf(stderr, "package not found\n");
            return 1;
        }
        printf("%s\n", version);
        return 0;
    } else if (action == 3) {
        if (package == NULL) {
            fprintf(stderr, "please specify a package with -p <pkg>\n");
            return 1;
        }
        return package_uninstall(package, destdir, loglevel);
    } else if (action == 4) {
        return database_list(destdir@);
    }
    return 0;
}
