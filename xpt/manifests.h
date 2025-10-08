#ifndef MANIFESTS_H
    #define MANIFESTS_H
    typedef struct {
        const char* str;
        const char* version;
        const char* desc;
        const char* maintainer;
        const char* arch;
    } PackageInfo;

    #define MAX_LINE 1024

    void delete_package_info(PackageInfo* pi);
    PackageInfo* parse_manifest(const char* file);
#endif /* MANIFESTS_H */
