#include <packages.h>
#include <common.h>
#include <database.h>

#include <archive.h>
#include <archive_entry.h>

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

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

        int owned;
        if (database_file_has_owner(cfil, destdir, &owned) != 0) {
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
                 ARCHIVE_EXTRACT_XATTR);

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
