#include <common.h>
#include <repo.h>

#include <ctype.h>
#include <fetch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "config.h"

static char *rtrim(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    if (!*s)
        return s;

    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e))
        e--;
    *(e + 1) = 0;

    return s;
}

static int parse_depends(RepoPackage *rp, const char *value)
{
    if (value[0] == '\0')
        return XPT_EX_OK;

    char *copy = strdup(value);
    if (!copy)
        return 1;

    char *tok = strtok(copy, ",");

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
    if (!rp)
        return;

    free(rp->name);
    free(rp->version);
    free(rp->arch);
    free(rp->url);
    free(rp->sha256);

    size_t i;
    for (i = 0; i < rp->depends_count; i++)
        free(rp->depends[i]);
    free(rp->depends);
    free(rp);
}

RepoPackage *repo_index_lookup(const char *indexfile, const char *name)
{
    FILE *fp = fopen(indexfile, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    char line[MAX_REPO_LINE];

    while (fgets(line, sizeof(line), fp)) {
        char *trmd = rtrim(line);
        if (trmd[0] == '\0')
            continue;

        char *fields[6] = {0};
        char *cursor = trmd;
        int i;

        for (i = 0; i < 6; i++) {
            fields[i] = cursor;
            char *pipe = strchr(cursor, '|');
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

        RepoPackage *rp = malloc(sizeof(RepoPackage));
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

    char buf[4096];
    ssize_t n;

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

    snprintf(indexfile, sizeof(indexfile), "%s%s/xpt.d/index", destdir,
             XPT_SYSCONFDIR);
    snprintf(cachefile, sizeof(cachefile), "%s%s/cache/xpt.repositories",
             destdir, XPT_LOCALSTATEDIR);

    FILE *fp = fopen(indexfile, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    FILE *out = fopen(cachefile, "w");
    if (!out) {
        perror("fopen");
        fclose(fp);
        return 1;
    }

    fclose(out);
    char line[MAX_REPO_LINE];
    int had_error = 0;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0')
            continue;

        snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", cachefile);

        if (repo_fetch_index(line, tmpfile) != 0) {
            fprintf(stderr, "warning: could not fetch index from %s\n", line);
            had_error = 1;
            continue;
        }

        FILE *src = fopen(tmpfile, "r");
        FILE *dst = fopen(cachefile, "a");
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
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    unsigned char buf[4096];

    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        EVP_DigestUpdate(ctx, buf, n);
    fclose(fp);

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    char hex[EVP_MAX_MD_SIZE * 2 + 1];
    unsigned int i;
    for (i = 0; i < hash_len; i++)
        sprintf(hex + i * 2, "%02x", hash[i]);
    hex[hash_len * 2] = '\0';

    return strcasecmp(hex, expected_hex) == 0 ? 0 : 1;
}

int xpt_repos_getpkginfo(const char *name, const char *destdir)
{
    char cachefile[PATH_MAX];
    snprintf(cachefile, sizeof(cachefile), "%s%s/cache/xpt.repositories",
             destdir, XPT_LOCALSTATEDIR);

    RepoPackage *rp;

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
