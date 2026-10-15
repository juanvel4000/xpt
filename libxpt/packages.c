#include <archive.h>
#include <archive_entry.h>
#include <internal/common.h>
#include <internal/database.h>
#include <internal/packages.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *strip_dot_slash(const char *path)
{
    while (path[0] == '.' && path[1] == '/')
        path += 2;
    return path;
}

int extract_member(const char *file, const char *member, char *tmppath)
{
    struct archive *a;
    struct archive_entry *entry;
    int ret = 1;

    a = archive_read_new();
    archive_read_support_filter_zstd(a);
    archive_read_support_format_tar(a);

    if (archive_read_open_filename(a, file, 10240) != ARCHIVE_OK) {
        fprintf(stderr, "could not open %s: %s\n", file,
                archive_error_string(a));
        archive_read_free(a);
        return 1;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *name = strip_dot_slash(archive_entry_pathname(entry));
        if (strcmp(name, member) != 0)
            continue;

        int fd = mkstemp(tmppath);
        if (fd < 0) {
            perror("mkstemp");
            break;
        }

        char buf[8192];
        la_ssize_t n;
        int ok = 1;

        while ((n = archive_read_data(a, buf, sizeof(buf))) > 0) {
            if (write(fd, buf, (size_t)n) != (ssize_t)n) {
                ok = 0;
                break;
            }
        }

        if (n < 0)
            ok = 0;

        close(fd);
        if (!ok) {
            fprintf(stderr, "could not extract %s from %s\n", member, file);
            unlink(tmppath);
            break;
        }

        ret = 0;
        break;
    }

    if (ret != 0 && tmppath[0] != '\0' && access(tmppath, F_OK) != 0)
        fprintf(stderr, "%s not found in %s\n", member, file);

    archive_read_free(a);
    return ret;
}

int preflight_payload(const char *file, const char *destdir)
{

    if (!is_file(file)) {
        fprintf(stderr, "%s does not exist\n", file);
        return 1;
    }

    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();

    archive_read_support_filter_zstd(a);
    archive_read_support_format_tar(a);

    if ((r = archive_read_open_filename(a, file, 10240))) {
        fprintf(stderr, "could not open %s: %s\n", file,
                archive_error_string(a));

        archive_read_close(a);
        archive_read_free(a);
        return 1;
    }

    int hr;
    while ((hr = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *cfil = archive_entry_pathname(entry);
        if (cfil[0] == '/' || strstr(cfil, "..") != NULL) {
            fprintf(stderr, "refusing to extract unsafe path: %s\n", cfil);
            archive_read_close(a);
            archive_read_free(a);
            return 1;
        }

        if (strlen(cfil) >= 4 && strcmp(cfil + strlen(cfil) - 4, ".xpt") == 0)
            continue;

        const char *rel = cfil;
        if (rel[0] == '.' && rel[1] == '/')
            rel += 2;

        int owned;
        if (database_file_has_owner(rel, destdir, &owned) != 0) {
            archive_read_close(a);
            archive_read_free(a);
            return 1;
        }

        if (owned) {
            fprintf(stderr,
                    "refusing to install file %s: file is already owned\n",
                    cfil);
            archive_read_close(a);
            archive_read_free(a);
            return 1;
        }
    }

    if (hr != ARCHIVE_EOF) {
        fprintf(stderr, "read error: %s\n", archive_error_string(a));

        archive_read_close(a);
        archive_read_free(a);
        return 1;
    }

    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

int extract_payload(const char *file, const char *destdir)
{

    if (!is_file(file)) {
        fprintf(stderr, "%s does not exist\n", file);
        return 1;
    }

    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();

    archive_read_support_filter_zstd(a);
    archive_read_support_format_tar(a);

    if ((r = archive_read_open_filename(a, file, 10240))) {
        fprintf(stderr, "could not open %s: %s\n", file,
                archive_error_string(a));

        archive_read_close(a);
        archive_read_free(a);
        return 1;
    }

    ext = archive_write_disk_new();
    archive_write_disk_set_standard_lookup(ext);

    archive_write_disk_set_options(
        ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL |
                 ARCHIVE_EXTRACT_FFLAGS | ARCHIVE_EXTRACT_OWNER |
                 ARCHIVE_EXTRACT_XATTR | ARCHIVE_EXTRACT_SECURE_SYMLINKS |
                 ARCHIVE_EXTRACT_SECURE_NODOTDOT);

    int hr;
    while ((hr = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *cfil = archive_entry_pathname(entry);

        if (cfil[0] == '/' || strstr(cfil, "..") != NULL) {
            fprintf(stderr, "refusing to extract unsafe path: %s\n", cfil);
            archive_read_close(a);
            archive_read_free(a);
            archive_write_close(ext);
            archive_write_free(ext);
            return 1;
        }

        const char *rel = strip_dot_slash(cfil);
        if (strcmp(rel, "xpt.manifest") == 0 || strcmp(rel, "xpt.tree") == 0) {
            archive_read_data_skip(a);
            continue;
        }

        char target[PATH_MAX];
        snprintf(target, sizeof(target), "%s/%s", destdir, cfil);

        archive_entry_set_pathname(entry, target);

        r = archive_write_header(ext, entry);

        if (r != ARCHIVE_OK) {
            fprintf(stderr, "warning: %s\n", archive_error_string(ext));
        } else {
            const void *buff;
            size_t size;
            la_int64_t offset;
            int rr;

            while ((rr = archive_read_data_block(a, &buff, &size, &offset)) ==
                   ARCHIVE_OK) {
                if (archive_write_data_block(ext, buff, size, offset) !=
                    ARCHIVE_OK) {
                    fprintf(stderr, "write error: %s\n",
                            archive_error_string(ext));

                    archive_read_close(a);
                    archive_read_free(a);
                    archive_write_close(ext);
                    archive_write_free(ext);

                    return 1;
                }
            }
            if (rr != ARCHIVE_EOF) {
                fprintf(stderr, "read error: %s\n", archive_error_string(a));

                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
                return 1;
            }
        }
        archive_write_finish_entry(ext);
    }

    if (hr != ARCHIVE_EOF) {
        fprintf(stderr, "read error: %s\n", archive_error_string(a));

        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        return 1;
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    return 0;
}
