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

int package_install(const char *file, const char *destdir, int log,
                    int netinstall_deps)
{
    if (!is_file(file)) {
        fprintf(stderr, "%s not found\n", file);
        return 1;
    }

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

    if (log == LOG_OK)
        printf("installing %s...\n", pi->name);

    if (database_exists(pi->name, destdir) == 0) {
        fprintf(stderr, "this package is already installed\n");
        delete_package_info(pi);
        return 1;
    }

    if (remove(manifest) != 0) {
        fprintf(stderr, "failed to delete %s\n", manifest);
    }

    NodeContainer container = {0};
    if (resolve_package(&container, pi->name, pi, destdir, netinstall_deps,
                        log) != 0) {
        fprintf(stderr, "dependency resolution failed for %s\n", pi->name);
        resolver_free(&container);
        delete_package_info(pi);
        return 1;
    }
    resolver_free(&container);

    if (database_add(pi, destdir) != 0) {
        fprintf(stderr, "error adding %s to database\n", pi->name);
        return 1;
    }

    if (database_files_add(pi->name, tree, destdir) != 0) {
        fprintf(stderr, "error saving file list for %s\n", pi->name);
        database_delete(pi->name, destdir);
        delete_package_info(pi);
        return 1;
    }

    if (log == LOG_OK)
        printf("installed %s.\n", pi->name);
    delete_package_info(pi);
    return 0;
}

void prune_empty_dirs(const char *file_path, const char *root_dir)
{
    char path_buf[PATH_MAX];
    char *dir;

    snprintf(path_buf, sizeof(path_buf), "%s", file_path);

    dir = dirname(path_buf);

    while (dir != NULL && strcmp(dir, ".") != 0 && strcmp(dir, "/") != 0) {
        if (strcmp(dir, root_dir) == 0)
            break;

        if (rmdir(dir) != 0) {
            break;
        }
        dir = dirname(dir);
    }
}

struct uninstall_ctx {
    const char *destdir;
};

static int uninstall_file(const char *path, void *userdata)
{

    struct uninstall_ctx *ctx = userdata;

    char full[PATH_MAX];

    snprintf(full, sizeof(full), "%s/%s", ctx->destdir, path);

    if (is_file(full) != 0) {
        remove(full);
        prune_empty_dirs(full, ctx->destdir);
    }

    return 0;
}

int package_uninstall(const char *name, const char *destdir, int log)
{
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return 1;
    }
    if (log == LOG_OK)
        printf("uninstalling %s...\n", name);

    struct uninstall_ctx ctx = {.destdir = destdir};

    if (database_foreach_file(name, destdir, uninstall_file, &ctx) != 0) {
        fprintf(stderr, "failed to enumerate files for %s\n", name);
        return 1;
    }

    if (database_delete(name, destdir) == 1)
        fprintf(stderr, "an error ocurred removing %s from the database.\n",
                name);

    if (log == LOG_OK)
        printf("uninstalled %s.\n", name);

    return 0;
}

static int print_file(const char *path, void *userdata)
{
    (void)userdata;

    puts(path);

    return 0;
}

int package_listfiles(const char *name, const char *destdir)
{
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return 1;
    }

    return database_foreach_file(name, destdir, print_file, NULL);
}

int package_install_from_repo(const char *name, const char *destdir, int log)
{
    char cachefile[PATH_MAX];
    char destfile[PATH_MAX];
    char destdir_copy[PATH_MAX];

    snprintf(cachefile, sizeof(cachefile), "%s/var/cache/xpt.repositories",
             destdir);

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

    if (repo_download(rp->url, destfile) != 0) {
        fprintf(stderr, "could not download %s\n", rp->url);
        repo_package_free(rp);
        return 1;
    }

    if (verify_sha256(destfile, rp->sha256) != 0) {
        fprintf(stderr, "checksum mismatch for %s\n", destfile);
        remove(destfile);
        repo_package_free(rp);
        return 1;
    }

    repo_package_free(rp);

    int ret = package_install(destfile, destdir, log, 1);

    return ret;
}
