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
        return -1;

    ret = snprintf(db, db_size, "%s/xpt.db", xptdir);

    if (ret < 0 || (size_t)ret >= db_size)
        return -1;

    return XPT_EX_OK;
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
        "  arch TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS files ("
        "  path TEXT NOT NULL,"
        "  pkgname TEXT NOT NULL,"
        "  FOREIGN KEY (pkgname) REFERENCES packages(name) ON DELETE CASCADE,"
        "  UNIQUE (pkgname, path)"
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

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO packages (name, version, desc, maintainer, "
                      "arch) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not add package to database: %s\n",
                sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, pi->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pi->version, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, pi->desc, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, pi->maintainer, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, pi->arch, -1, SQLITE_STATIC);

    int ret = 0;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not add package to database: %s\n",
                sqlite3_errmsg(conn));
        ret = 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return ret;
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

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE packages SET version = ? WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not update the package: %s\n",
                sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, version, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, package, -1, SQLITE_STATIC);

    int ret = 0;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not update the package: %s\n",
                sqlite3_errmsg(conn));
        ret = 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return ret;
}

int database_delete(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not delete package: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_STATIC);

    int ret = 0;
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "could not delete package: %s\n", sqlite3_errmsg(conn));
        ret = 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return ret;
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
    const char *sql = "SELECT name, version, desc, maintainer, arch FROM "
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

        printf("package: %s\n", name);
        printf("version: %s\n", version);
        printf("description: %s\n", desc);
        printf("maintainer: %s\n", maintainer);
        printf("arch: %s\n", arch);
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
            fprintf(stderr, "warning: failed to save %s (%s): %s\n", line,
                    pkgname, sqlite3_errmsg(conn));
            continue;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(conn);
    return XPT_EX_OK;
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
