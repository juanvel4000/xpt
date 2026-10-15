#include "config.h"
#include <internal/common.h>
#include <internal/database.h>
#include <internal/manager.h>
#include <internal/manifests.h>
#include <internal/packages.h>
#include <internal/resolver.h>
#include <internal/triggers.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xpt/package.h>
#include <xpt/status.h>

#if !XPT_DISABLE_NETWORKING
#include <internal/repo.h>
#endif

static int install_depth;
static char **pending_triggers;
static size_t pending_count;

static int queue_trigger(const char *name)
{
    size_t i;
    char **tmp;
    char *copy;

    for (i = 0; i < pending_count; i++) {
        if (strcmp(pending_triggers[i], name) == 0)
            return XPT_EX_OK;
    }

    copy = strdup(name);
    if (!copy)
        return XPT_EX_CANTCREAT;

    tmp = realloc(pending_triggers, (pending_count + 1) * sizeof(char *));
    if (!tmp) {
        free(copy);
        return XPT_EX_CANTCREAT;
    }

    pending_triggers = tmp;
    pending_triggers[pending_count++] = copy;
    return XPT_EX_OK;
}

static void run_pending_triggers(const char *destdir, int log)
{
    size_t i;

    if (pending_count == 0)
        return;

    xpt_run_triggers(destdir, pending_triggers, pending_count, log);

    for (i = 0; i < pending_count; i++)
        free(pending_triggers[i]);
    free(pending_triggers);

    pending_triggers = NULL;
    pending_count = 0;
}

static int install_one(const char *file, const char *destdir, int log,
                       int netinstall_deps)
{
#if XPT_DISABLE_NETWORKING
    netinstall_deps = 0;
#endif
    int ret;
    char tmp_manifest[] = "/tmp/xpt-manifest-XXXXXX";
    char tmp_tree[] = "/tmp/xpt-tree-XXXXXX";
    PackageInfo *pi = NULL;

    if (!is_file(file)) {
        fprintf(stderr, "%s not found\n", file);
        ret = XPT_EX_NOINPUT;
        goto out;
    }

    if (extract_member(file, "xpt.manifest", tmp_manifest) != 0) {
        ret = XPT_EX_DATAERR;
        goto out;
    }

    if (extract_member(file, "xpt.tree", tmp_tree) != 0) {
        ret = XPT_EX_DATAERR;
        goto out;
    }

    if (preflight_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        ret = XPT_EX_DATAERR;
        goto out;
    }

    pi = parse_manifest(tmp_manifest);
    if (!pi) {
        fprintf(stderr, "invalid manifest: %s\n", tmp_manifest);
        ret = XPT_EX_DATAERR;
        goto out;
    }

    if (log == XPT_LOG_OK)
        printf("installing %s...\n", pi->name);

    if (database_exists(pi->name, destdir) == 0) {
        fprintf(stderr, "this package is already installed\n");
        ret = XPT_EX_EXISTS;
        goto out;
    }

    NodeContainer container = {0};
    if (resolve_package(&container, pi->name, pi, destdir, netinstall_deps,
                        log) != 0) {
        fprintf(stderr, "dependency resolution failed for %s\n", pi->name);
        resolver_free(&container);
        ret = XPT_EX_NOINPUT;
        goto out;
    }

    resolver_free(&container);

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

            ret = XPT_EX_USAGE;
            goto out;
        }

        if (xpt_package_remove(owner, destdir, log) != XPT_EX_OK) {
            fprintf(stderr, "could not remove %s\n", owner);
            free(owner);

            ret = XPT_EX_IOERR;
            goto out;
        }

        free(owner);
    }

    if (extract_payload(file, destdir) != 0) {
        fprintf(stderr, "could not extract %s\n", file);
        ret = XPT_EX_IOERR;
        goto out;
    }

    if (database_add(pi, destdir) != 0) {
        fprintf(stderr, "error adding %s to database\n", pi->name);
        ret = XPT_EX_IOERR;
        goto out;
    }

    if (database_files_add(pi->name, tmp_tree, destdir) != 0) {
        fprintf(stderr, "error saving file list for %s\n", pi->name);
        database_delete(pi->name, destdir);
        ret = XPT_EX_IOERR;
        goto out;
    }

    if (database_provides_add(pi->name, pi->provides, pi->provides_count,
                              destdir) != 0) {
        fprintf(stderr, "error saving capabilities to database\n");
        database_delete(pi->name, destdir);
        ret = XPT_EX_IOERR;
        goto out;
    }

    size_t i;
    for (i = 0; i < pi->triggers_count; i++) {
        if (queue_trigger(pi->triggers[i]) != XPT_EX_OK) {
            fprintf(stderr, "warning: could not queue trigger %s\n",
                    pi->triggers[i]);
        }
    }

    if (log == XPT_LOG_OK)
        printf("installed %s.\n", pi->name);

    ret = XPT_EX_OK;

out:
    if (tmp_manifest[0])
        unlink(tmp_manifest);
    if (tmp_tree[0])
        unlink(tmp_tree);
    if (pi)
        delete_package_info(pi);
    return ret;
}

int xpt_package_install(const char *file, const char *destdir, int log,
                        int netinstall_deps)
{
    int ret;

    install_depth++;
    ret = install_one(file, destdir, log, netinstall_deps);
    install_depth--;

    if (install_depth == 0)
        run_pending_triggers(destdir, log);

    return ret;
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
