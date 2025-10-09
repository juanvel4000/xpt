#ifndef MANAGER_H
    #define MANAGER_H
    int package_install(const char *file, const char *destdir, int log);
    int package_uninstall(const char* name, const char* destdir, int log);

#endif /* MANAGER_H */
