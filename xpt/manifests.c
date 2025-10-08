#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>

#include <common.h>
#include <packages.h>
#include <manifests.h>

char* trim(char* s) {
    while (isspace((unsigned char)*s)) s++;
    if (!*s)
        return s;

    char* e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) e--;
    *(e + 1) = 0;

    return s;
}
int verify_package_info(PackageInfo* pi) {
    if (pi->name == NULL) {
        return 1;
    } else if (pi->desc == NULL) {
        return 1;
    } else if (pi->version == NULL) {
        return 1;
    } else if (pi->arch == NULL) {
        return 1;
    } else if (pi->maintainer == NULL) {
        return 1;
    } else {
        return 0;
    }
}
void delete_package_info(PackageInfo* pi) {
    free(pi->name);
    free(pi->desc);
    free(pi->version);
    free(pi->arch);
    free(pi->maintainer);
    free(pi);
}
int safe_strdup(char **dst, const char *src) {
    *dst = strdup(src);
    if (!*dst) {
        perror("strdup");
        return 1;
    }
    return 0;
}
PackageInfo* parse_manifest(const char* file) {
    if (!is_file(file)) {
        fprintf(stderr, "%s does not exist\n", file);
        return NULL;
    }

    PackageInfo* pi = malloc(sizeof(PackageInfo));
    if (!pi) {
        perror("malloc");
        return NULL;
    }
    memset(pi, 0, sizeof(PackageInfo));

    FILE *fp = fopen(file, "r");
    if (!fp) {
        perror("fopen");
        delete_package_info(pi);
        return NULL;
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp)) {
        char *trmd = trim(line);
        if (trmd[0] == '\0' || (trmd[0] == '/' && trmd[1] == '/')) {
            continue;
        }

        char *eq = strchr(trmd, '=');
        if (!eq) {
            fprintf(stderr, "invalid line: '%s'\n", line);
            continue;
        }

        *eq = '\0';

        char *key = trim(trmd);
        char *val = trim(eq + 1);
        if (strcmp(key, "name") == 0) {
            if (safe_strdup(&pi->name, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "desc") == 0) {
            if (safe_strdup(&pi->desc, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "version") == 0) {
            if (safe_strdup(&pi->version, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "arch") == 0) {
            if (safe_strdup(&pi->arch, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "maintainer") == 0) {
            if (safe_strdup(&pi->maintainer, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else {
            fprintf(stderr, "unknown key: %s\n", key);
            continue;
        }
    }
    fclose(fp);
    if (verify_package_info(pi) == 0) {
        return pi;
    } else {
        delete_package_info(pi);
        return NULL;
    }
}
