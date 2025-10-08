#ifndef MANIFESTS_H
    #define MANIFESTS_H
    typedef struct {
        char* name;
        char* version;
        char* desc;
        char* maintainer;
        char* arch;
    } PackageInfo;

    #define MAX_LINE 1024

    void delete_package_info(PackageInfo* pi);
    PackageInfo* parse_manifest(const char* file);
#endif /* MANIFESTS_H */
