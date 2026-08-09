#include <database.h>
#include <common.h>
#include <manifests.h>

#include <sqlite3.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

static int make_paths(char *db, size_t db_size, char *xptdir,
                      size_t xptdir_size, const char *destdir)
{
    int ret;

    ret = snprintf(xptdir, xptdir_size, "%s%s/lib/xpt", destdir,
                   XPT_LOCALSTATEDIR);

    if (ret < 0 || (size_t)ret >= xptdir_size)
        return 1;

    ret = snprintf(db, db_size, "%s/xpt.db", xptdir);

    if (ret < 0 || (size_t)ret >= db_size)
        return 1;

    return 0;
}

static int tbegin(sqlite3 *conn)
{
    return sqlite3_exec(conn, "BEGIN;", NULL, NULL, NULL);
}

static int trollback(sqlite3 *conn)
{
    return sqlite3_exec(conn, "ROLLBACK;", NULL, NULL, NULL);
}

static int tcommit(sqlite3 *conn)
{
    return sqlite3_exec(conn, "COMMIT;", NULL, NULL, NULL);
}

static sqlite3 *open_db(const char *destdir)
{
    char db[PATH_MAX];
    char xptdir[PATH_MAX];

    if (make_paths(db, sizeof(db), xptdir, sizeof(xptdir), destdir) != 0)
        return NULL;

    if (mkdir_p(xptdir) != 0)
        return NULL;

    sqlite3 *conn;
    if (sqlite3_open(db, &conn) != SQLITE_OK) {
        fprintf(stderr, "could not open database: %s\n", sqlite3_errmsg(conn));
        return NULL;
    }

    sqlite3_exec(conn, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    const char *schema =
        "CREATE TABLE IF NOT EXISTS packages ("
        "  name TEXT PRIMARY KEY,"
        "  version TEXT NOT NULL,"
        "  desc TEXT NOT NULL,"
        "  maintainer TEXT NOT NULL,"
        "  arch TEXT NOT NULL,"
        "  license TEXT DEFAULT NULL,"
        "  homepage TEXT DEFAULT NULL,"
        "  build_epoch INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS files ("
        "  path TEXT NOT NULL,"
        "  pkgname TEXT NOT NULL,"
        "  FOREIGN KEY (pkgname) REFERENCES packages(name) ON DELETE CASCADE,"
        "  UNIQUE (pkgname, path)"
        ");"
        "CREATE TABLE IF NOT EXISTS provides ("
        "  capability TEXT NOT NULL,"
        "  pkgname  TEXT NOT NULL,"
        "  FOREIGN KEY (pkgname) REFERENCES packages(name) ON DELETE CASCADE,"
        "  UNIQUE (capability)"
        ");";

    char *errmsg = NULL;
    if (sqlite3_exec(conn, schema, NULL, NULL, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "could not create schema: %s\n", sqlite3_errmsg(conn));
        sqlite3_free(errmsg);
        sqlite3_close(conn);
        return NULL;
    }

    return conn;
}

int database_exists(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_STATIC);
    int found = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return found ? 0 : 1;
}

int database_add(PackageInfo *pi, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    tbegin(conn);

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO packages (name, version, desc, maintainer, "
                      "arch, license, homepage, build_epoch) VALUES (?, ?, ?, "
                      "?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not add package to database: %s\n",
                sqlite3_errmsg(conn));

        trollback(conn);
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, pi->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pi->version, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, pi->desc, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, pi->maintainer, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, pi->arch, -1, SQLITE_STATIC);

    if (pi->license)
        sqlite3_bind_text(stmt, 6, pi->license, -1, SQLITE_STATIC);
    else
        sqlite3_bind_null(stmt, 6);

    if (pi->homepage)
        sqlite3_bind_text(stmt, 7, pi->homepage, -1, SQLITE_STATIC);
    else
        sqlite3_bind_null(stmt, 7);

    sqlite3_bind_int64(stmt, 8, pi->build_epoch);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not add package to database: %s\n",
                sqlite3_errmsg(conn));

        trollback(conn);
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return 1;
    }

    tcommit(conn);
    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return 0;
}

char *xpt_package_getversion(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return NULL;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT version FROM packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_STATIC);

    char *version = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        version = strdup((const char *)text);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return version;
}

int database_update(const char *package, const char *version,
                    const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    tbegin(conn);
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE packages SET version = ? WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not update the package: %s\n",
                sqlite3_errmsg(conn));
        trollback(conn);
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, version, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, package, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not update the package: %s\n",
                sqlite3_errmsg(conn));

        trollback(conn);
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return 1;
    }

    tcommit(conn);
    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return 0;
}

int database_delete(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    tbegin(conn);
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not delete package: %s\n", sqlite3_errmsg(conn));
        trollback(conn);
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not delete package: %s\n", sqlite3_errmsg(conn));

        trollback(conn);
        sqlite3_finalize(stmt);
        sqlite3_close(conn);
        return 1;
    }

    tcommit(conn);
    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return 0;
}

int xpt_package_list(const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return XPT_EX_IOERR;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name, version FROM packages ORDER BY name;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not list packages: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return XPT_EX_IOERR;
    }

    printf("%-20s %s\n", "package", "version");
    printf("------------------------------\n");
    int pkgcount = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *name = sqlite3_column_text(stmt, 0);
        const unsigned char *version = sqlite3_column_text(stmt, 1);
        printf("%-20s %s\n", name, version);
        pkgcount++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    printf("\n%d packages installed\n", pkgcount);
    return XPT_EX_OK;
}

int xpt_package_printinfo(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return XPT_EX_IOERR;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name, version, desc, maintainer, arch, license, "
                      "homepage, build_epoch FROM "
                      "packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return XPT_EX_IOERR;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        const char *version = (const char *)sqlite3_column_text(stmt, 1);
        const char *desc = (const char *)sqlite3_column_text(stmt, 2);
        const char *maintainer = (const char *)sqlite3_column_text(stmt, 3);
        const char *arch = (const char *)sqlite3_column_text(stmt, 4);
        const char *license = (const char *)sqlite3_column_text(stmt, 5);
        const char *homepage = (const char *)sqlite3_column_text(stmt, 6);
        const char *build_epoch = (const char *)sqlite3_column_text(stmt, 7);

        printf("package: %s\n", name);
        printf("version: %s \n", version);
        printf("build epoch: %s\n", build_epoch);
        printf("description: %s\n", desc);
        printf("maintainer: %s\n", maintainer);
        printf("arch: %s\n", arch);
        if (license != NULL)
            printf("license: %s\n", license);

        if (homepage != NULL)
            printf("homepage: %s\n", homepage);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return (rc == SQLITE_ROW) ? XPT_EX_OK : XPT_EX_NOINPUT;
}

int database_files_add(const char *pkgname, const char *treefile,
                       const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    FILE *fp = fopen(treefile, "r");
    if (!fp) {
        perror("fopen");
        sqlite3_close(conn);
        return 1;
    }

    tbegin(conn);
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(conn,
                           "INSERT INTO files (path, pkgname) VALUES (?, ?);",
                           -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0')
            continue;

        sqlite3_bind_text(stmt, 1, line, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pkgname, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "failed to save %s (%s): %s\n", line,
                    pkgname, sqlite3_errmsg(conn));
            trollback(conn);
            sqlite3_finalize(stmt);
            fclose(fp);
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_reset(stmt);
    }

    tcommit(conn);
    sqlite3_finalize(stmt);
    fclose(fp);
    sqlite3_close(conn);
    return 0;
}

int database_foreach_file(const char *pkgname, const char *destdir,
                          database_file_callback cb, void *userdata)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;

    const char *sql = "SELECT path FROM files WHERE pkgname = ? ORDER BY path;";

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, pkgname, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *path = (const char *)sqlite3_column_text(stmt, 0);

        if (cb(path, userdata) != 0)
            break;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);

    return XPT_EX_OK;
}

int database_file_has_other_owners(const char *name, const char *path,
                                   const char *destdir, int *shared)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;

    const char *sql = "SELECT EXISTS("
                      "SELECT 1 FROM files WHERE path = ? AND pkgname != ?"
                      ");";

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);

    int result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *shared = sqlite3_column_int(stmt, 0);
        result = 0;
    } else
        result = 1;

    sqlite3_finalize(stmt);
    sqlite3_close(conn);

    return result;
}

int database_file_has_owner(const char *path, const char *destdir, int *owned)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;

    const char *sql = "SELECT EXISTS("
                      "SELECT 1 FROM files WHERE path = ?"
                      ");";

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    int result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *owned = sqlite3_column_int(stmt, 0);
        result = 0;
    } else {
        result = 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);

    return result;
}

int database_provides_add(const char *pkgname, char *const *capabilities,
                          size_t capability_count, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    tbegin(conn);
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(
            conn, "INSERT INTO provides (capability, pkgname) VALUES (?, ?);",
            -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
    }

    for (size_t i = 0; i < capability_count; i++) {
        const char *capability = capabilities[i];
        sqlite3_bind_text(stmt, 1, capability, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pkgname, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, " failed to save capability %s (%s): %s\n",
                    capability, pkgname, sqlite3_errmsg(conn));
            trollback(conn);
            sqlite3_finalize(stmt);
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_reset(stmt);
    }

    tcommit(conn);
    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return 0;
}

char *database_who_provides(const char *capability, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return NULL;

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(conn,
                           "SELECT pkgname FROM provides WHERE capability = ?;",
                           -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, capability, -1, SQLITE_STATIC);
    char *pkgname = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        pkgname = strdup((const char *)text);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return pkgname;
}
