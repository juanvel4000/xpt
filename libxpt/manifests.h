#ifndef MANIFESTS_H
#define MANIFESTS_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
    char *name;
    char *version;
    char *desc;
    char *maintainer;
    char *arch;

    char **depends;
    size_t depends_count;

    char **provides;
    size_t provides_count;

    char *license;
    char *homepage;

    uint64_t build_epoch;

    char **triggers;
    size_t triggers_count;

} PackageInfo;

#define MAX_LINE 1024

void delete_package_info(PackageInfo *pi);
PackageInfo *parse_manifest(const char *file);
int package_info_add_provide(PackageInfo *pi, const char *capability);
#endif /* MANIFESTS_H */
