#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <manifests.h>

static void test_manifest_parsing(void)
{
    PackageInfo *pi = parse_manifest("fixtures/sample.xpt.manifest");

    assert(strcmp(pi->name, "foo") == 0);
    assert(strcmp(pi->version, "1.2.3") == 0);
    assert(strcmp(pi->desc, "test package") == 0);
    assert(strcmp(pi->maintainer, "you <you@example.com>") == 0);
    assert(strcmp(pi->arch, "x86_64") == 0);
    assert(strcmp(pi->license, "BSD-3-Clause") == 0);
    assert(strcmp(pi->homepage, "https://example.com/software/foo.html") == 0);

    assert(strcmp(pi->depends[0], "libfoo-dev") == 0);
    assert(strcmp(pi->depends[1], "libc") == 0);
    assert(pi->depends_count == 2);
    delete_package_info(pi);
    return;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    test_manifest_parsing();
    return 0;
}
