#include <manifests.h>
#include <packages.h>
#include <common.h>
#include <database.h>
#include <resolver.h>
#include <manager.h>
#include "config.h"

#if !XPT_DISABLE_NETWORKING
#include <repo.h>
#endif

#include <libgen.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int xpt_package_install(const char *file, const char *destdir, int log,
                        int netinstall_deps)
{
#if XPT_DISABLE_NETWORKING
    netinstall_deps = 0;
#endif

    if (!is_file(file)) {
        fprintf(stderr, "%s not found\n", file);
        return XPT_EX_NOINPUT;
    }

    if (preflight_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        return XPT_EX_DATAERR;
    }

    if (extract_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        return XPT_EX_IOERR;
    }

    char xptdir[PATH_MAX];
    snprintf(xptdir, sizeof(xptdir), "%s%s/lib/xpt", destdir, XPT_SYSCONFDIR);

    if (mkdir_p(xptdir) != 0)
        return XPT_EX_CANTCREAT;

    char manifest[PATH_MAX];
    char tree[PATH_MAX];

    snprintf(manifest, sizeof(manifest), "%s/xpt.manifest", destdir);
    snprintf(tree, sizeof(tree), "%s/xpt.tree", destdir);
    if (!is_file(tree) || !is_file(manifest)) {
        fprintf(stderr, "either the manifest OR the treefile doesn't exist\n");
        return XPT_EX_NOINPUT;
    }

    PackageInfo *pi = parse_manifest(manifest);
    if (!pi) {
        fprintf(stderr, "invalid manifest: %s\n", manifest);
        return XPT_EX_DATAERR;
    }

    if (log == XPT_LOG_OK)
        printf("installing %s...\n", pi->name);

    if (database_exists(pi->name, destdir) == 0) {
        fprintf(stderr, "this package is already installed\n");
        delete_package_info(pi);
        return XPT_EX_EXISTS;
    }

    package_info_add_provide(pi, pi->name);
    for (size_t i = 0; i < pi->provides_count; i++) {
        char *owner = database_who_provides(pi->provides[i], destdir);

        if (owner == NULL)
            continue;

        printf("%s provides %s\n", owner, pi->provides[i]);
        printf("do you want to replace %s with %s? [y/N]: ", owner, pi->name);

        char answer;
        if (scanf(" %c", &answer) != 1 || (answer != 'y' && answer != 'Y')) {
            fprintf(stderr, "could not install %s\n", pi->name);
            free(owner);
            delete_package_info(pi);
            return XPT_EX_USAGE;
        }

        if (xpt_package_remove(owner, destdir, log) != XPT_EX_OK) {
            fprintf(stderr, "could not remove %s\n", owner);
            free(owner);
            delete_package_info(pi);
            return XPT_EX_IOERR;
        }

        free(owner);
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
        return XPT_EX_NOINPUT;
    }
    resolver_free(&container);

    if (database_add(pi, destdir) != 0) {
        fprintf(stderr, "error adding %s to database\n", pi->name);
        return XPT_EX_IOERR;
    }

    if (database_files_add(pi->name, tree, destdir) != 0) {
        fprintf(stderr, "error saving file list for %s\n", pi->name);
        database_delete(pi->name, destdir);
        delete_package_info(pi);
        return XPT_EX_IOERR;
    }

    if (database_provides_add(pi->name, pi->provides, pi->provides_count,
                              destdir) != 0) {
        fprintf(stderr, "error saving capabilities to database\n");
        database_delete(pi->name, destdir);
        delete_package_info(pi);
        return XPT_EX_IOERR;
    }

    if (remove(tree) != 0) {
        fprintf(stderr, "warning: failed to delete %s\n", tree);
    }

    if (log == XPT_LOG_OK)
        printf("installed %s.\n", pi->name);

    delete_package_info(pi);
    return XPT_EX_OK;
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
    const char *name;
    const char *destdir;
    int log;
};

static int uninstall_file(const char *path, void *userdata)
{

    struct uninstall_ctx *ctx = userdata;

    char full[PATH_MAX];

    snprintf(full, sizeof(full), "%s/%s", ctx->destdir, path);

    int shared;
    if (database_file_has_other_owners(ctx->name, path, ctx->destdir,
                                       &shared) != 0)

        return XPT_EX_IOERR;

    if (shared) {
        if (ctx->log == XPT_LOG_OK)
            printf("keeping shared file: %s\n", path);
        return 0;
    }

    if (is_file(full) != 0) {
        remove(full);
        prune_empty_dirs(full, ctx->destdir);
    }

    return 0;
}

int xpt_package_remove(const char *name, const char *destdir, int log)
{
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return XPT_EX_NOINPUT;
    }
    if (log == XPT_LOG_OK)
        printf("uninstalling %s...\n", name);

    struct uninstall_ctx ctx = {.name = name, .destdir = destdir, .log = log};

    if (database_foreach_file(name, destdir, uninstall_file, &ctx) != 0) {
        fprintf(stderr, "failed to enumerate files for %s\n", name);
        return XPT_EX_IOERR;
    }

    if (database_delete(name, destdir) == 1)
        fprintf(stderr, "an error ocurred removing %s from the database.\n",
                name);

    if (log == XPT_LOG_OK)
        printf("uninstalled %s.\n", name);

    return XPT_EX_OK;
}

static int print_file(const char *path, void *userdata)
{
    (void)userdata;

    puts(path);

    return 0;
}

int xpt_package_listfiles(const char *name, const char *destdir)
{
    if (database_exists(name, destdir) != 0) {
        fprintf(stderr, "package %s is not installed\n", name);
        return XPT_EX_NOINPUT;
    }

    return database_foreach_file(name, destdir, print_file, NULL);
}

#if !XPT_DISABLE_NETWORKING
int xpt_package_install_from_repo(const char *name, const char *destdir,
                                  int log)
{
    char cachefile[PATH_MAX];
    char destfile[PATH_MAX];
    char destdir_copy[PATH_MAX];

    snprintf(cachefile, sizeof(cachefile), "%s%s/cache/xpt.repositories",
             destdir, XPT_LOCALSTATEDIR);

    RepoPackage *rp = repo_index_lookup(cachefile, name);
    if (!rp) {
        fprintf(stderr, "package %s not found in any repository\n", name);
        return XPT_EX_NOINPUT;
    }

    snprintf(destfile, sizeof(destfile), "%s%s/cache/xpt.packages/%s-%s.xpt",
             destdir, XPT_LOCALSTATEDIR, rp->name, rp->version);

    snprintf(destdir_copy, sizeof(destdir_copy), "%s", destfile);

    if (mkdir_p(dirname(destdir_copy)) != 0) {
        fprintf(stderr, "could not create cache directory for %s\n", destfile);
        repo_package_free(rp);
        return XPT_EX_CANTCREAT;
    }

    if (repo_download(rp->url, destfile) != 0) {
        fprintf(stderr, "could not download %s\n", rp->url);
        repo_package_free(rp);
        return XPT_EX_TEMPFAIL;
    }
    if (verify_sha256(destfile, rp->sha256) != 0) {
        fprintf(stderr, "checksum mismatch for %s\n", destfile);
        remove(destfile);
        repo_package_free(rp);
        return XPT_EX_PROTOCOL;
    }

    repo_package_free(rp);

    return xpt_package_install(destfile, destdir, log, 1);
}
#else
int xpt_package_install_from_repo(const char *name, const char *destdir,
                                  int log)
{
    (void)name;
    (void)destdir;
    (void)log;

    fprintf(stderr, "libxpt has been built without networking support.\n");
    return XPT_EX_USAGE;
}
#endif
