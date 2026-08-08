#ifndef MANAGER_H
#define MANAGER_H
int xpt_package_install(const char *file, const char *destdir, int log,
                        int netinstall_deps);
int xpt_package_remove(const char *name, const char *destdir, int log);
void prune_empty_dirs(const char *file_path, const char *root_dir);
int xpt_package_listfiles(const char *name, const char *destdir);
int xpt_package_install_from_repo(const char *name, const char *destdir,
                                  int log);
#endif /* MANAGER_H */
