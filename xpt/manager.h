#ifndef MANAGER_H
    #define MANAGER_H
    int package_install(const char *file, const char *destdir, int log);
    int package_uninstall(const char* name, const char* destdir, int log);
    void prune_empty_dirs(const char* file_path, const char* root_dir);
    int package_listfiles(const char* name, const char* destdir);

#endif /* MANAGER_H */
