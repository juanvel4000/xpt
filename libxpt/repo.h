#ifndef REPO_H
#define REPO_H
#include <stdio.h>

typedef struct {
    char *name;
    char *version;
    char *arch;
    char *url;
    char *sha256;

    char **depends;
    size_t depends_count;
} RepoPackage;

#define MAX_REPO_LINE 1024

int repo_download(const char *url, const char *dest);
int repo_fetch_index(const char *base_url, const char *cachefile);

RepoPackage *repo_index_lookup(const char *indexfile, const char *name);
void repo_package_free(RepoPackage *rp);
int verify_sha256(const char *file, const char *expected_hex);

int xpt_repos_sync(const char *destdir);
int xpt_repos_getpkginfo(const char *name, const char *destdir);
#endif /* REPO_H */
