#ifndef XPT_INFO_H
#define XPT_INFO_H

#include <stdio.h>

/**
 * @file info.h
 * @brief interfaces to display build information about libxpt
 */

/**
 * print build information about libxpt
 *
 * @param out the file to print into
 */
void xpt_print_build_info(FILE *out);

#endif /* XPT_INFO_H */
