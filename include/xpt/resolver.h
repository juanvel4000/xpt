#ifndef RESOLVER_H
#define RESOLVER_H
#include <xpt/manifests.h>

typedef struct PackageNode PackageNode;

struct PackageNode {
    char *name;
    struct PackageNode *next;
};

typedef struct {
    struct PackageNode *seen;
    struct PackageNode *resolved;
} NodeContainer;

int resolver_seen(NodeContainer *container, const char *name);
int resolver_add_seen(NodeContainer *container, const char *name);
int resolver_add_resolved(NodeContainer *container, const char *name);
void resolver_free(NodeContainer *container);
int resolve_package(NodeContainer *container, const char *name,
                    PackageInfo *pkg, const char *destdir, int netinstall_deps,
                    int log);

#endif /* RESOLVER_H */
