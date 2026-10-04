#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <resolver.h>

static void test_seen_and_resolved(void)
{
    NodeContainer c = {0};

    assert(resolver_seen(&c, "foo") == 0);

    assert(resolver_add_seen(&c, "foo") == 0);
    assert(resolver_seen(&c, "foo") == 1);
    assert(resolver_seen(&c, "bar") == 0);

    assert(resolver_add_resolved(&c, "foo") == 0);
    assert(resolver_add_resolved(&c, "bar") == 0);

    assert(strcmp(c.resolved->name, "foo") == 0);
    assert(strcmp(c.resolved->next->name, "bar") == 0);
    assert(c.resolved->next->next == NULL);

    resolver_free(&c);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    test_seen_and_resolved();
    return 0;
}
