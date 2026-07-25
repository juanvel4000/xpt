#include <manifests.h>
#include <packages.h>
#include <common.h>
#include <database.h>
#include <resolver.h>
#include <repo.h>

#include <libgen.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

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

    NodeContainer container = {0};
    if (resolve_package(&container, pi->name, pi, destdir) != 0) {
        fprintf(stderr, "dependency resolution failed for %s\n", pi->name);
        resolver_free(&container);
        delete_package_info(pi);
        return 1;
    }
    resolver_free(&container);

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

void prune_empty_dirs(const char* file_path, const char* root_dir) {
    char path_buf[PATH_MAX];
    char *dir;

    snprintf(path_buf, sizeof(path_buf), "%s", file_path);

    dir = dirname(path_buf);

    while (dir != NULL && strcmp(dir, ".") != 0 && strcmp(dir, "/") != 0) {
        if (strcmp(dir, root_dir) == 0) break;

        if (rmdir(dir) != 0) {
            break;
        }
        dir = dirname(dir);
    }
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
    char full_path[PATH_MAX];

    while (fgets(item, sizeof(item), fp)) {
        item[strcspn(item, "\n")] = '\0';
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", destdir, item);

        if (ret < 0 || (size_t)ret >= sizeof(full_path)) {
            fprintf(stderr, "path too long: %s/%s\n", destdir, item);
            return 1;
        }

        if (item_exists(full_path) == 1) {
            continue;
        }

        if (is_file(full_path)) {
            if (remove(full_path) != 0) {
                fprintf(stderr, "warning: could not remove %s\n", item);
                perror("remove");
                continue;
            }
            prune_empty_dirs(full_path, destdir);
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

int package_listfiles(const char* name, const char* destdir) {
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return 1;
    }

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
        printf("%s\n", item);
    }

    return 0;
}

int package_install_from_repo(const char *name, const char *destdir, int log) {
    char cachefile[PATH_MAX];
    char destfile[PATH_MAX];
    char destdir_copy[PATH_MAX];

    snprintf(cachefile, sizeof(cachefile), "%s/var/cache/xpt.repositories", destdir);

    RepoPackage *rp = repo_index_lookup(cachefile, name);
    if (!rp) {
        fprintf(stderr, "package %s not found in any repository\n", name);
        return 1;
    }


    snprintf(destfile, sizeof(destfile), "%s/var/cache/xpt.packages/%s-%s.xpt",
             destdir, rp->name, rp->version);

    snprintf(destdir_copy, sizeof(destdir_copy), "%s", destfile);

    if (mkdir_p(dirname(destdir_copy)) != 0) {
        fprintf(stderr, "could not create cache directory for %s\n", destfile);
        repo_package_free(rp);
        return 1;
    }

    if (log == LOG_OK)
        printf("fetching %s...\n", rp->name);

    if (repo_download(rp->url, destfile) != 0) {
        fprintf(stderr, "could not download %s\n", rp->url);
        repo_package_free(rp);
        return 1;
    }

    repo_package_free(rp);

    int ret = package_install(destfile, destdir, LOG_NO);
    if (ret == 0 && log == LOG_OK)
        printf("installed %s.\n", name);

    return ret;
}
