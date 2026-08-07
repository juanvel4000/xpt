#include <manager.h>
#include <resolver.h>
#include <database.h>
#include <manifests.h>
#include <common.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int resolver_seen(NodeContainer *container, const char *name)
{
    PackageNode *node = container->seen;

    while (node != NULL) {
        if (strcmp(node->name, name) == 0)
            return 1;

        node = node->next;
    }

    return 0;
}

int resolver_add_seen(NodeContainer *container, const char *name)
{
    PackageNode *node = malloc(sizeof(PackageNode));

    if (!node)
        return 1;

    node->name = strdup(name);

    if (!node->name) {
        free(node);
        return 1;
    }

    node->next = container->seen;
    container->seen = node;

    return 0;
}

int resolver_add_resolved(NodeContainer *container, const char *name)
{
    PackageNode *node = malloc(sizeof(PackageNode));

    if (!node)
        return 1;

    node->name = strdup(name);

    if (!node->name) {
        free(node);
        return 1;
    }

    node->next = NULL;

    if (container->resolved == NULL) {
        container->resolved = node;
        return 0;
    }

    PackageNode *current = container->resolved;

    while (current->next != NULL)
        current = current->next;

    current->next = node;

    return XPT_EX_OK;
}

void resolver_free(NodeContainer *container)
{
    PackageNode *node;
    while (container->seen) {
        node = container->seen;
        container->seen = node->next;

        free(node->name);
        free(node);
    }

    while (container->resolved) {
        node = container->resolved;
        container->resolved = node->next;

        free(node->name);
        free(node);
    }
}

int resolve_package(NodeContainer *container, const char *name,
                    PackageInfo *pkg, const char *destdir, int netinstall_deps,
                    int log)
{
    if (resolver_seen(container, name))
        return XPT_EX_OK;

    if (resolver_add_seen(container, name))
        return 1;

    size_t i;
    for (i = 0; i < pkg->depends_count; i++) {
        if (database_exists(pkg->depends[i], destdir) != 0) {
            if (netinstall_deps) {
                if (xpt_package_install_from_repo(pkg->depends[i], destdir,
                                                  log) != 0) {
                    fprintf(stderr, "failed to install dependency: %s\n",
                            pkg->depends[i]);
                    return 1;
                }
            } else {
                fprintf(stderr, "missing dependency: %s\n", pkg->depends[i]);
                return 1;
            }
        }
    }

    if (resolver_add_resolved(container, pkg->name))
        return 1;

    return 0;
}
