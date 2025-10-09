#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

int is_file(const char *path) {
    struct stat buf;
    return (stat(path, &buf) == 0 && (buf.st_mode & S_IFREG));
}


int item_exists(const char *path) {
    if (access(path, F_OK) == 0)
        return 0;
    else
        return 1;
}


int mkdir_p(const char *path) {
    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    size_t len = strlen(tmp);
    if (len == 0) return -1;

    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0775) != 0) {
                if (errno != EEXIST) return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0775) != 0) {
        if (errno != EEXIST) return -1;
    }
    return 0;
}
