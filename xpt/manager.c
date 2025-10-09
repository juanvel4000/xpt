#include <manifests.h>
#include <packages.h>
#include <common.h>
#include <database.h>

#include <stdio.h>
#include <limits.h>
#include <string.h>

int package_install(const char *file, const char *destdir, int log) {
    if (!is_file(file)) {
        fprintf(stderr, "%s not found\n", file);
        return 1;
    }
    if (log == LOG_OK)
        printf("installing %s...\n", file);
        
    if (extract_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        return 1;
    }

    char xptdir[PATH_MAX];
    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);

    if (mkdir_p(xptdir) != 0)
        return 1;
    char manifest[PATH_MAX];
    char tree[PATH_MAX];

    snprintf(manifest, sizeof(manifest), "%s/xpt.manifest", destdir);
    snprintf(tree, sizeof(tree), "%s/xpt.tree", destdir);
    if (!is_file(tree) || !is_file(manifest)) {
        fprintf(stderr, "either the manifest OR the treefile doesn't exist\n");
        return 1;
    }
    PackageInfo *pi = parse_manifest(manifest);

    if (!pi) {
        fprintf(stderr, "invalid manifest: %s\n", manifest);
        return 1;
    }
    
    if (database_exists(pi->name, destdir) == 0) {
        fprintf(stderr, "this package is already installed\n");
        delete_package_info(pi);
        return 1;
    }

    char treedest[PATH_MAX];
    snprintf(treedest, sizeof(treedest), "%s/var/lib/xpt/%s.tree", destdir, pi->name);

    if (rename(tree, treedest) != 0) {
        fprintf(stderr, "couldn't save %s\n", treedest);
        delete_package_info(pi);
        return 1;
    }

    if (remove(manifest) != 0) {
        fprintf(stderr, "failed to delete %s\n", manifest);
    }

    if (database_add(pi->name, pi->version, destdir) != 0) {
        fprintf(stderr, "error adding %s to database\n", pi->name);
        return 1;
    }
    if (log == LOG_OK)
        printf("installed %s.\n", pi->name);
    delete_package_info(pi);
    return 0;
}

int package_uninstall(const char* name, const char* destdir, int log) {
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return 1;
    }
    if (log == LOG_OK)
        printf("uninstalling %s...\n", name);
    char tree[PATH_MAX];
    snprintf(tree, sizeof(tree), "%s/var/lib/xpt/%s.tree", destdir, name);

    if (!is_file(tree)) {
        fprintf(stderr, "treefile %s does not exist\n", tree);
        return 1;
    }
    
    FILE* fp = fopen(tree, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    char item[PATH_MAX];
    while (fgets(item, sizeof(item), fp)) {
        item[strcspn(item, "\n")] = '\0';
        if (item_exists(item) == 1)
            continue;
        if (is_file(item)) {
            if (remove(item) != 0) {
                fprintf(stderr, "warning: could not remove %s\n", item);
                continue;
            }
        }
    }
    fclose(fp);

    if (remove(tree) != 0)
        perror("remove");

    if (database_delete(name, destdir) == 1)
        fprintf(stderr, "an error ocurred removing %s from the database.\n", name);
 
    if (log == LOG_OK)
        printf("uninstalled %s.\n", name);
    return 0;
}
