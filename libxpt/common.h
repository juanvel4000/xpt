#ifndef COMMON_H
#define COMMON_H
int is_file(const char *path);
int item_exists(const char *path);

int mkdir_p(const char *path);

#define XPT_LOG_OK 1
#define XPT_LOG_NO 0
#endif /* COMMON_H */
