#ifndef DATABASE_H
#define DATABASE_H
#include <xpt/manifests.h>

char *database_getver(const char *package, const char *destdir);

int database_update(const char *package, const char *version,
                    const char *destdir);
int database_add(PackageInfo *pi, const char *destdir);
int database_exists(const char *package, const char *destdir);
int database_delete(const char *package, const char *destdir);
int database_list(const char *destdir);
int database_printinfo(const char *package, const char *destdir);
#endif /* DATABASE_H */
