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
#endif /* MANIFESTS_H */
