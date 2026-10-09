#include <common.h>
#include <manifests.h>
#include <packages.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *s)
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

static int parse_list(const char *value, char ***list, size_t *count)
{
    char *copy;
    char *tok;
    char **tmp;

    copy = strdup(value);
    if (!copy)
        return 1;

    tok = strtok(copy, ",");

    while (tok) {
        tmp = realloc(*list, (*count + 1) * sizeof(char *));
        if (!tmp) {
            free(copy);
            return 1;
        }

        *list = tmp;

        (*list)[*count] = strdup(trim(tok));
        if (!(*list)[*count]) {
            free(copy);
            return 1;
        }

        (*count)++;

        tok = strtok(NULL, ",");
    }

    free(copy);
    return 0;
}

int verify_package_info(PackageInfo *pi)
{
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
        return XPT_EX_OK;
    }
}

void delete_package_info(PackageInfo *pi)
{
    free(pi->name);
    free(pi->desc);
    free(pi->version);
    free(pi->arch);
    free(pi->maintainer);

    for (size_t i = 0; i < pi->depends_count; i++)
        free(pi->depends[i]);

    free(pi->depends);

    for (size_t i = 0; i < pi->provides_count; i++)
        free(pi->provides[i]);

    free(pi->provides);

    for (size_t i = 0; i < pi->triggers_count; i++)
        free(pi->triggers[i]);

    free(pi->triggers);

    free(pi);
}

static int safe_strdup(char **dst, const char *src)
{
    *dst = strdup(src);
    if (!*dst) {
        perror("strdup");
        return 1;
    }
    return XPT_EX_OK;
}

static int validate_triggers(char **list, size_t count)
{
    size_t i;
    const char *name;
    const char *p;

    for (i = 0; i < count; i++) {
        name = list[i];
        if (name[0] == '\0' || name[0] == '/' || strstr(name, "..") != NULL) {
            fprintf(stderr, "invalid trigger '%s'\n", name);
            return XPT_EX_DATAERR;
        }

        for (p = name; *p != '\0'; p++) {
            if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' &&
                *p != '-' && *p != '/') {
                fprintf(stderr, "invalid character in trigger '%s'\n", name);
                return XPT_EX_DATAERR;
            }
        }
    }

    return XPT_EX_OK;
}

PackageInfo *parse_manifest(const char *file)
{
    if (!is_file(file)) {
        fprintf(stderr, "%s does not exist\n", file);
        return NULL;
    }

    PackageInfo *pi = malloc(sizeof(PackageInfo));
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
        } else if (strcmp(key, "depends") == 0) {
            if (parse_list(val, &pi->depends, &pi->depends_count) != 0) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "license") == 0) {
            if (safe_strdup(&pi->license, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "homepage") == 0) {
            if (safe_strdup(&pi->homepage, val) == 1) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "build_epoch") == 0) {
            char *end;
            unsigned long long epoch;

            errno = 0;
            epoch = strtoull(val, &end, 10);

            if (errno == ERANGE || *end != '\0') {
                fprintf(stderr, "invalid build_epoch: %s\n", val);
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }

            pi->build_epoch = epoch;
        } else if (strcmp(key, "provides") == 0) {
            if (parse_list(val, &pi->provides, &pi->provides_count) != 0) {
                fclose(fp);
                delete_package_info(pi);
                return NULL;
            }
        } else if (strcmp(key, "triggers") == 0) {
            if (parse_list(val, &pi->triggers, &pi->triggers_count) != 0 ||
                validate_triggers(pi->triggers, pi->triggers_count) != 0) {
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

int package_info_add_provide(PackageInfo *pi, const char *capability)
{
    char **tmp;

    if (pi == NULL || capability == NULL || capability[0] == '\0')
        return 1;

    for (size_t i = 0; i < pi->provides_count; i++) {
        if (strcmp(pi->provides[i], capability) == 0)
            return 0;
    }

    tmp =
        realloc(pi->provides, (pi->provides_count + 1) * sizeof(*pi->provides));
    if (tmp == NULL)
        return 1;

    pi->provides = tmp;
    pi->provides[pi->provides_count] = strdup(capability);

    if (pi->provides[pi->provides_count] == NULL)
        return 1;

    pi->provides_count++;
    return 0;
}
