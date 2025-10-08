#include <stdio.h>
#include <manager.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char *argv[]) {
    int opt;
    char *filename = NULL;
    int action = 0; /* 0 = none; 1 = install */
    char destdir[PATH_MAX] = "/";
    if (argc == 1) {
        fprintf(stderr, "usage: %s <cmd> [opts]\n", argv[0]);
        return 1;
    }
    while ((opt = getopt(argc, argv, "f:id:h")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'i':
                action = 1;
                break;
            case 'd':
                snprintf(destdir, sizeof(destdir), "%s", optarg);
                break;
            case 'h':
                printf("xpt - px package tool");
                printf("usage: %s <cmd> [opts]\n", argv[0]);
                printf("commands\n");
                printf(" -i install <file> (requires specifying file with -f)\n");
                printf(" -f <file> specify a file\n");
                printf(" -h show this message\n");
                printf(" -d specify a destination directory; defaults to '/' if not specified\n");
                printf("xpt is licensed with the 3-clause BSD license\n");
                return 0;
        }
    }
    printf("action=%d, filename=%s, destdir=%s\n", action, filename, destdir);
    if (action == 1) {
        if (filename == NULL) {
            fprintf(stderr, "please specify a file with -f <file>\n");
            return 1;
        }
        package_install(filename, destdir);
    }
    return 0;
}
