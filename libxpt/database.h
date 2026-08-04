#ifndef DATABASE_H
#define DATABASE_H
#include <manifests.h>

char *xpt_package_getversion(const char *package, const char *destdir);

int database_update(const char *package, const char *version,
                    const char *destdir);
int database_add(PackageInfo *pi, const char *destdir);
int database_exists(const char *package, const char *destdir);
int database_delete(const char *package, const char *destdir);
int xpt_package_list(const char *destdir);
int xpt_package_printinfo(const char *package, const char *destdir);
int database_files_add(const char *pkgname, const char *treefile,
                       const char *destdir);

typedef int (*database_file_callback)(const char *path, void *userdata);
int database_foreach_file(const char *pkgname, const char *destdir,
                          database_file_callback cb, void *userdata);
#endif /* DATABASE_H */
