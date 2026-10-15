#ifndef PACKAGES_H
#define PACKAGES_H

int extract_member(const char *file, const char *member, char *tmppath);
int extract_payload(const char *file, const char *destdir);
int preflight_payload(const char *file, const char *destdir);

#endif /* PACKAGES_H */
