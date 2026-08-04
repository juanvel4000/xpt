#include <xpt/database.h>
#include <xpt/common.h>
#include <xpt/manifests.h>

#include <sqlite3.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

static int make_paths(char *db, size_t db_size, char *xptdir,
                      size_t xptdir_size, const char *destdir)
{
    int ret;

    ret = snprintf(xptdir, xptdir_size, "%s/var/lib/xpt", destdir);

    if (ret < 0 || (size_t)ret >= xptdir_size)
        return -1;

    ret = snprintf(db, db_size, "%s/xpt.db", xptdir);

    if (ret < 0 || (size_t)ret >= db_size)
        return -1;

    return 0;
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
    }

    const char *schema = "CREATE TABLE IF NOT EXISTS packages ("
                         "  name TEXT PRIMARY KEY,"
                         "  version TEXT NOT NULL,"
                         "  desc    TEXT NOT NULL,"
                         "  maintainer TEXT NOT NULL,"
                         "  arch       TEXT NOT NULL"
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

char *database_getver(const char *package, const char *destdir)
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

int database_list(const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name, version FROM packages ORDER BY name;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "could not list packages: %s\n", sqlite3_errmsg(conn));
        sqlite3_close(conn);
        return 1;
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
    return 0;
}

int database_printinfo(const char *package, const char *destdir)
{
    sqlite3 *conn = open_db(destdir);
    if (!conn)
        return 1;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT name, version, desc, maintainer, arch FROM "
                      "packages WHERE name = ?;";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(conn);
        return 1;
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
    return (rc == SQLITE_ROW) ? 0 : 1;
}
