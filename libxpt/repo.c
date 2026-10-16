#include "config.h"
#include <ctype.h>
#include <internal/common.h>
#include <internal/repo.h>
#include <libfetch/fetch.h>
#include <limits.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpt/status.h>

static char *rtrim(char *s)
{
    char *e;
    while (isspace((unsigned char)*s))
        s++;
    if (!*s)
        return s;

    e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e))
        e--;
    *(e + 1) = 0;

    return s;
}

static int parse_depends(RepoPackage *rp, const char *value)
{
    char *copy;
    char *tok;
    if (value[0] == '\0')
        return XPT_EX_OK;

    copy = strdup(value);
    if (!copy)
        return 1;

    tok = strtok(copy, ",");

    while (tok) {
        char **tmp =
            realloc(rp->depends, (rp->depends_count + 1) * sizeof(char *));

        if (!tmp) {
            free(copy);
            return 1;
        }

        rp->depends = tmp;
        rp->depends[rp->depends_count] = strdup(rtrim(tok));

        if (!rp->depends[rp->depends_count]) {
            free(copy);
            return 1;
        }

        rp->depends_count++;
        tok = strtok(NULL, ",");
    }

    free(copy);
    return XPT_EX_OK;
}

void repo_package_free(RepoPackage *rp)
{
    size_t i;
    if (!rp)
        return;

    free(rp->name);
    free(rp->version);
    free(rp->arch);
    free(rp->url);
    free(rp->sha256);

    for (i = 0; i < rp->depends_count; i++)
        free(rp->depends[i]);
    free(rp->depends);
    free(rp);
}

RepoPackage *repo_index_lookup(const char *indexfile, const char *name)
{
    FILE *fp;
    char line[MAX_REPO_LINE];

    fp = fopen(indexfile, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *trmd = rtrim(line);
        char *fields[6] = {0};
        char *cursor = trmd;
        int i;
        RepoPackage *rp;

        if (trmd[0] == '\0')
            continue;

        for (i = 0; i < 6; i++) {
            char *pipe = strchr(cursor, '|');
            fields[i] = cursor;
            if (pipe == NULL) {
                if (i < 5) {
                    fprintf(stderr, "malformed index line: '%s'\n", line);
                    fields[0] = NULL;
                }
                break;
            }
            *pipe = '\0';
            cursor = pipe + 1;
        }

        if (fields[0] == NULL || i < 5)
            continue;

        if (strcmp(fields[0], name) != 0)
            continue;

        rp = malloc(sizeof(RepoPackage));
        if (!rp) {
            perror("malloc");
            fclose(fp);
            return NULL;
        }

        memset(rp, 0, sizeof(RepoPackage));

        rp->name = strdup(fields[0]);
        rp->version = strdup(fields[1]);
        rp->arch = strdup(fields[2]);
        rp->url = strdup(fields[4]);
        rp->sha256 = strdup(fields[5]);

        if (!rp->name || !rp->version || !rp->arch || !rp->url || !rp->sha256) {
            perror("strdup");
            fclose(fp);
            repo_package_free(rp);
            return NULL;
        }

        if (parse_depends(rp, fields[3]) != 0) {
            fclose(fp);
            repo_package_free(rp);
            return NULL;
        }

        fclose(fp);
        return rp;
    }

    fclose(fp);
    fprintf(stderr, "package %s not found in %s\n", name, indexfile);
    return NULL;
}

int repo_download(const char *url, const char *dest)
{
    fetchIO *src;
    FILE *dst;
    char buf[4096];
    ssize_t n;

    src = fetchXGetURL(url, NULL, "");
    if (!src) {
        fprintf(stderr, "download failed: %d\n", fetchLastErrCode.code);
        return 6;
    }

    dst = fopen(dest, "wb");
    if (!dst) {
        perror("fopen");
        fetchIO_close(src);
        return XPT_EX_IOERR;
    }

    while ((n = fetchIO_read(src, buf, sizeof(buf))) > 0)
        fwrite(buf, 1, n, dst);

    fetchIO_close(src);
    fclose(dst);

    return XPT_EX_OK;
}

int repo_fetch_index(const char *base_url, const char *cachefile)
{
    char url[MAX_REPO_LINE];
    int ret = snprintf(url, sizeof(url), "%s/xpt.index", base_url);

    if (ret < 0 || (size_t)ret >= sizeof(url)) {
        fprintf(stderr, "repo url too long: %s\n", base_url);
        return 1;
    }

    return repo_download(url, cachefile);
}

int xpt_repos_sync(const char *destdir)
{
    char indexfile[PATH_MAX];
    char cachefile[PATH_MAX];
    char tmpfile[PATH_MAX + 5];
    FILE *fp;
    FILE *out;
    char line[MAX_REPO_LINE];
    int had_error = 0;

    snprintf(indexfile, sizeof(indexfile), "%s%s/xpt.d/index", destdir,
             XPT_SYSCONFDIR);
    snprintf(cachefile, sizeof(cachefile), "%s%s/cache/xpt.repositories",
             destdir, XPT_LOCALSTATEDIR);

    fp = fopen(indexfile, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    out = fopen(cachefile, "w");
    if (!out) {
        perror("fopen");
        fclose(fp);
        return 1;
    }

    fclose(out);

    while (fgets(line, sizeof(line), fp)) {
        FILE *src;
        FILE *dst;
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0')
            continue;

        if (line[0] == '#')
            continue;

        snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", cachefile);

        if (repo_fetch_index(line, tmpfile) != 0) {
            fprintf(stderr, "warning: could not fetch index from %s\n", line);
            had_error = 1;
            continue;
        }

        src = fopen(tmpfile, "r");
        dst = fopen(cachefile, "a");
        if (src && dst) {
            char buf[4096];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
                fwrite(buf, 1, n, dst);
        }
        if (src)
            fclose(src);
        if (dst)
            fclose(dst);
        remove(tmpfile);
    }

    fclose(fp);
    return had_error;
}

int verify_sha256(const char *file, const char *expected_hex)
{
    FILE *fp;
    EVP_MD_CTX *ctx;
    unsigned char buf[4096];
    size_t n;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    char hex[EVP_MAX_MD_SIZE * 2 + 1];
    unsigned int i;

    fp = fopen(file, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        EVP_DigestUpdate(ctx, buf, n);
    fclose(fp);

    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    for (i = 0; i < hash_len; i++)
        sprintf(hex + i * 2, "%02x", hash[i]);
    hex[hash_len * 2] = '\0';

    return strcasecmp(hex, expected_hex) == 0 ? 0 : 1;
}

int xpt_repos_getpkginfo(const char *name, const char *destdir)
{
    char cachefile[PATH_MAX];
    RepoPackage *rp;

    snprintf(cachefile, sizeof(cachefile), "%s%s/cache/xpt.repositories",
             destdir, XPT_LOCALSTATEDIR);

    rp = repo_index_lookup(cachefile, name);
    if (rp == NULL) {
        fprintf(stderr, "package not found: %s\n", name);
        repo_package_free(rp);
        return XPT_EX_NOINPUT;
    }

    printf("%s@%s (%s)\n", rp->name, rp->version, rp->arch);
    repo_package_free(rp);
    return XPT_EX_OK;
}
