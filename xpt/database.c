#include <gdbm.h>
#include <stdio.h>
#include <common.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <database.h>

int database_exists(const char* package, const char* destdir) {
    char db[PATH_MAX];
    char xptdir[PATH_MAX];
    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    snprintf(db, sizeof(db), "%s/xpt.db", xptdir);

    if (mkdir_p(xptdir) != 0)
        return 1;

    GDBM_FILE dbf = gdbm_open(db, 512, GDBM_READER, 0644, NULL);
    if (!dbf) {
        return 1;
    }
    datum pkg;
    pkg.dptr = (char*)package;
    pkg.dsize = strlen(package);
    datum v = gdbm_fetch(dbf, pkg);
    if (v.dptr == NULL) {
        free(v.dptr);
        gdbm_close(dbf);
        return 1;
    }
    free(v.dptr);

    gdbm_close(dbf);
    return 0;
}
int database_add(const char* package, const char* version, const char* destdir) {
    char db[PATH_MAX];
    char xptdir[PATH_MAX];

    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    snprintf(db, sizeof(db), "%s/xpt.db", xptdir);
    
    if (mkdir_p(xptdir) != 0)
        return 1; 

    GDBM_FILE dbf = gdbm_open(db, 512, GDBM_WRCREAT, 0644, NULL);
    if (!dbf) {
        perror("gdbm_open");
        return 1;
    }
    
    datum name, ver;

    name.dptr = (char*)package;
    name.dsize = strlen(package);

    ver.dptr = (char*)version;
    ver.dsize = strlen(version);

    if (gdbm_store(dbf, name, ver, GDBM_INSERT) == -1) {
        perror("gdbm_store");
        gdbm_close(dbf);
        return 1;
    }

    gdbm_close(dbf);
    return 0;
}

char* database_getver(const char* package, const char* destdir) {
    char db[PATH_MAX];
    char xptdir[PATH_MAX];
    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    snprintf(db, sizeof(db), "%s/xpt.db", xptdir);

    if (mkdir_p(xptdir) != 0) {
        return NULL;
    }

    GDBM_FILE dbf = gdbm_open(db, 512, GDBM_READER, 0644, NULL);
    if (!dbf) {
        return NULL;
    }
    datum pkg;
    pkg.dptr = (char*)package;
    pkg.dsize = strlen(package);
    datum v = gdbm_fetch(dbf, pkg);
    if (v.dptr == NULL) {
        return NULL;
    }
    gdbm_close(dbf);

    char* version = strdup(v.dptr);

    free(v.dptr);
    return version;
}

int database_update(const char* package, const char* version, const char* destdir) {
    char db[PATH_MAX];
    char xptdir[PATH_MAX];

    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    snprintf(db, sizeof(db), "%s/xpt.db", xptdir);

    if (mkdir_p(xptdir) != 0)
        return 1;

    GDBM_FILE dbf = gdbm_open(db, 512, GDBM_WRCREAT, 0644, NULL);;
    if (!dbf) {
        perror("gdbm_open");
        return 1;
    }
    
    datum name, ver;

    name.dptr = (char*)package;
    name.dsize = strlen(package);

    ver.dptr = (char*)version;
    ver.dsize = strlen(version);

    if (gdbm_store(dbf, name, ver, GDBM_REPLACE) == -1) {
        perror("gdbm_store"); 
        gdbm_close(dbf);
        return 1;
    }

    gdbm_close(dbf);
    return 0;
}

int database_delete(const char* package, const char* destdir) {
    char db[PATH_MAX];
    char xptdir[PATH_MAX];

    snprintf(xptdir, sizeof(xptdir), "%s/var/lib/xpt", destdir);
    snprintf(db, sizeof(db), "%s/xpt.db", xptdir);

    if (mkdir_p(xptdir) != 0)
        return 1;

    GDBM_FILE dbf = gdbm_open(db, 512, GDBM_WRCREAT, 0644, NULL);
    if (!dbf) {
        perror("gdbm_open");
        return 1;
    }
    
    datum name;

    name.dptr = (char*)package;
    name.dsize = strlen(package);

    if (gdbm_delete(dbf, name) == -1) {
        perror("gdbm_delete"); 
        gdbm_close(dbf);
        return 1;
    }

    gdbm_close(dbf);
    return 0;
}
