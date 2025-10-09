#include <stdio.h>
#include <manager.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <common.h>
#include <database.h>
int main(int argc, char *argv[]) {
    int opt;
    char *filename = NULL;
    char *package = NULL;
    int action = 0; /* 0 = none; 1 = install; 2 = get ver */
    char destdir[PATH_MAX] = "/";
    if (argc == 1) {
        fprintf(stderr, "usage: %s <cmd> [opts]\n", argv[0]);
        return 1;
    }
    while ((opt = getopt(argc, argv, "ighrf:p:d:")) != -1) {
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
            case 'g':
                action = 2;
                break;
            case 'r':
                action = 3;
                break;
            case 'd':
                snprintf(destdir, sizeof(destdir), "%s", optarg);
                break;
            case 'h':
                printf("xpt - the px package tool\n");
                printf("usage: %s <cmd> [opts]\n", argv[0]);
                printf("commands\n");
                printf(" -i         install <file> (requires specifying file with -f)\n");
                printf(" -g         get the version of an installed package\n");
                printf(" -h         show this message\n");
                printf(" -r         remove a package\n");
                printf("options\n");
                printf(" -f <file>  specify a file\n");
                printf(" -p <pkg>   specify an installed package name\n");
                printf(" -d <dir>   specify a destination directory for the operations\n");

                printf("xpt is licensed with the 3-clause BSD license\n");
                return 0;
        }
    }
    if (action == 1) {
        if (filename == NULL) {
            fprintf(stderr, "please specify a file with -f <file>\n");
            return 1;
        }
        return package_install(filename, destdir, LOG_OK);
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
        return package_uninstall(package, destdir, LOG_OK);
    } else {
        fprintf(stderr, "no action specified\n");
        return 1;
    }
    return 0;
}
