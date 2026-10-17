#include <internal/colorize.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void print_level(FILE *stream, const char *color, const char *prefix,
                        const char *fmt, va_list ap)
{
    int use_color;

    use_color = isatty(fileno(stream)) && getenv("NO_COLOR") == NULL;

    if (use_color) {
        fputs(color, stream);
        fputs("\033[1m", stream);
        fputs(prefix, stream);
        fputs("\033[0m", stream);
    }

    vfprintf(stream, fmt, ap);

    if (use_color)
        fputs(XPT_COLOR_RESET, stream);
}

void xpt_print_err(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_level(stderr, XPT_COLOR_RED, "*> ", fmt, ap);
    va_end(ap);
}

void xpt_print_info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_level(stdout, XPT_COLOR_BLUE, ">> ", fmt, ap);
    va_end(ap);
}

void xpt_print_warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_level(stdout, XPT_COLOR_YELLOW, "!> ", fmt, ap);
    va_end(ap);
}

void xpt_print_ok(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_level(stdout, XPT_COLOR_GREEN, "=> ", fmt, ap);
    va_end(ap);
}
