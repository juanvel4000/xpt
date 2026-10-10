#ifndef XPT_STATUS_H
#define XPT_STATUS_H

/** semi-verbose */
#define XPT_LOG_OK 1

/** quiet */
#define XPT_LOG_NO 0

/** ok */
#define XPT_EX_OK 0

/** bad flags / user error */
#define XPT_EX_USAGE 1

/* not found */
#define XPT_EX_NOINPUT 2

/* invalid entry */
#define XPT_EX_DATAERR 3

/** io failure */
#define XPT_EX_IOERR 4

/** couldn't create item */
#define XPT_EX_CANTCREAT 5

/** network / download failure */
#define XPT_EX_TEMPFAIL 6

/** checksum mismatch */
#define XPT_EX_PROTOCOL 7

/** lock held */
#define XPT_EX_NOPERM 8

/** internal err */
#define XPT_EX_SOFTWARE 9

/** exists */
#define XPT_EX_EXISTS 10

#endif /* XPT_STATUS_H */
