#ifndef COMMON_H
#define COMMON_H
int is_file(const char *path);
int item_exists(const char *path);

int mkdir_p(const char *path);

#define XPT_LOG_OK 1
#define XPT_LOG_NO 0

#define XPT_EX_OK          0  /* ok */
#define XPT_EX_USAGE       1  /* bad flags / user error */
#define XPT_EX_NOINPUT     2  /* not found */
#define XPT_EX_DATAERR     3  /* invalid entry */
#define XPT_EX_IOERR       4  /* io failure */
#define XPT_EX_CANTCREAT   5  /* couldn't create item */
#define XPT_EX_TEMPFAIL    6  /* network / download failure */
#define XPT_EX_PROTOCOL    7  /* checksum mismatch */
#define XPT_EX_NOPERM      8  /* lock held */
#define XPT_EX_SOFTWARE    9  /* internal err */
#define XPT_EX_EXISTS      10 /* exists */
#endif /* COMMON_H */
