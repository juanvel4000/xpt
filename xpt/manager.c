#include <manifests.h>
#include <packages.h>
#include <common.h>

#include <stdio.h>
#include <limits.h>

int package_install(const char *file, const char *destdir) {
    if (!is_file(file)) {
        fprintf(stderr, "%s not found\n", file);
        return 1;
    }

    char xptdir[PATH_MAX];
    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    if (mkdir_p(xptdir) != 0) {
        fprintf(stderr, "could not create %s\n", xptdir);
        return 1;
    }
    if (extract_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        return 1;
    }

    char manifest[PATH_MAX];

    snprintf(manifest, sizeof(manifest), "%s/manifest.xpt", destdir);
    if (!is_file(manifest)) {
        fprintf(stderr, "%s does not have a manifest\n", file);
        return 1;
    }

    PackageInfo *pi = parse_manifest(manifest);

    if (!pi) {
        fprintf(stderr, "invalid manifest: %s\n", manifest);
        return 1;
    }


    char db[PATH_MAX];
    snprintf(db, sizeof(db), "%s/database", xptdir);

    FILE *fp = fopen(db, "a");
    if (!fp) {
        delete_package_info(pi);
        fprintf(stderr, "could not open database");
        return 1;
    }
    if (fprintf(fp, "%s@%s\n", pi->name, pi->version) > 0) {
        delete_package_info(pi);
        fclose(fp);

        fprintf(stderr, "couldn't to write to database");
        return 1;
    }

    delete_package_info(pi);
    fclose(fp);

    if (remove(manifest) != 0) {
        fprintf(stderr, "failed to delete %s\n", manifest);
    }
    return 0;
}
