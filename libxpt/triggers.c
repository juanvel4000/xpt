#include <config.h>
#include <limits.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xpt/xpt.h>

int xpt_run_triggers(const char *destdir, char **names, size_t count, int log)
{
    char path[PATH_MAX];
    const char *name;
    size_t i;
    pid_t proc;
    int status;

    for (i = 0; i < count; i++) {
        name = names[i];

        if (log == XPT_LOG_OK)
            printf("running trigger: %s\n", name);

        snprintf(path, sizeof(path), "%s%s/xpt/triggers/%s", destdir,
                 XPT_DATADIR, name);

        if (access(path, X_OK) != 0) {
            fprintf(stderr, "warning: trigger %s not found or not executable\n",
                    name);
            continue;
        }

        proc = fork();
        if (proc == -1) {
            perror("fork");
            continue;
        }

        if (proc == 0) {
            execl(path, path, (char *)NULL);
            _exit(127);
        }

        if (waitpid(proc, &status, 0) == -1) {
            perror("waitpid");
            continue;
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            fprintf(stderr, "warning: trigger %s failed\n", name);
    }

    return XPT_EX_OK;
}
