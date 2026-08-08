#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "config.h"

int is_file(const char *path)
{
    struct stat buf;
    return (stat(path, &buf) == 0 && (buf.st_mode & S_IFREG));
}

int item_exists(const char *path)
{
    if (access(path, F_OK) == 0)
        return 0;
    else
        return 1;
}

int mkdir_p(const char *path)
{
    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    size_t len = strlen(tmp);
    if (len == 0)
        return -1;

    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    char *p;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0775) != 0) {
                if (errno != EEXIST)
                    return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0775) != 0) {
        if (errno != EEXIST)
            return -1;
    }
    return 0;
}

void xpt_print_build_info(FILE *out)
{
    fprintf(out, "prefix:        %s\n", XPT_PREFIX);
    fprintf(out, "sysconfdir:    %s\n", XPT_SYSCONFDIR);
    fprintf(out, "localstatedir: %s\n", XPT_LOCALSTATEDIR);
    fprintf(out, "compiler:      %s\n", XPT_COMPILER);
#if XPT_DISABLE_NETWORKING
    fprintf(out, "libraries:     sqlite3 %s, libarchive %s\n",
            XPT_SQLITE3_VERSION, XPT_LIBARCHIVE_VERSION);
#else
    fprintf(out, "libraries:     sqlite3 %s, libarchive %s, openssl %s\n",
            XPT_SQLITE3_VERSION, XPT_LIBARCHIVE_VERSION, XPT_OPENSSL_VERSION);
#endif
    fprintf(out, "build type:    %s\n", XPT_BUILDTYPE);
    char *networking;
#if XPT_DISABLE_NETWORKING
    networking = "no";
#else
    networking = "yes";
#endif
    fprintf(out, "networking:    %s\n", networking);
}
