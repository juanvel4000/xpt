#ifndef COLORIZE_H
#define COLORIZE_H

#define XPT_COLOR_RESET "\033[0m"
#define XPT_COLOR_GREEN "\033[32m"
#define XPT_COLOR_RED "\033[31m"
#define XPT_COLOR_YELLOW "\033[33m"
#define XPT_COLOR_BLUE "\033[34m"

void xpt_print_err(const char *fmt, ...);
void xpt_print_info(const char *fmt, ...);
void xpt_print_ok(const char *fmt, ...);
void xpt_print_warn(const char *fmt, ...);

#endif /* COLORIZE_H */
