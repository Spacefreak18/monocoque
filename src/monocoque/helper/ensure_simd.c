#define _GNU_SOURCE

#include "ensure_simd.h"
#include "dirhelper.h"

#include "../simulatorapi/simapi/simapi/simapi.h"
#include "../slog/slog.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SIMD_PID_FILE "/tmp/simd.pid"
#define SIMD_START_TIMEOUT_MS 3000
#define SIMD_POLL_MS 50

extern char** environ;

static int is_executable_file(const char* path)
{
    if (path == NULL || path[0] == '\0')
    {
        return 0;
    }
    if (access(path, X_OK) != 0)
    {
        return 0;
    }
    return does_file_exist(path) ? 1 : 0;
}

static void sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = ((long)(ms % 1000)) * 1000000L;
    nanosleep(&ts, NULL);
}

static char* join_path(const char* dir, const char* name)
{
    char* out = NULL;
    if (dir == NULL || name == NULL)
    {
        return NULL;
    }
    if (asprintf(&out, "%s/%s", dir, name) < 0)
    {
        return NULL;
    }
    return out;
}

int simd_process_running(void)
{
    char name[] = "simd";
    char* names[] = { name, NULL };
    struct SimProcessInfo info = get_process_match(names, 1);
    if (info.pid <= 0)
    {
        return 0;
    }
    return is_pid_running((pid_t)info.pid);
}

char* simd_find_binary_from_candidates(const char* const* candidates)
{
    if (candidates == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; candidates[i] != NULL; i++)
    {
        if (is_executable_file(candidates[i]))
        {
            return strdup(candidates[i]);
        }
    }
    return NULL;
}

static char* find_on_path(const char* name)
{
    const char* path = getenv("PATH");
    if (path == NULL || name == NULL)
    {
        return NULL;
    }

    char* copy = strdup(path);
    if (copy == NULL)
    {
        return NULL;
    }

    char* save = copy;
    while (save != NULL && save[0] != '\0')
    {
        char* sep = strchr(save, ':');
        if (sep != NULL)
        {
            *sep = '\0';
        }
        const char* dir = (save[0] == '\0') ? "." : save;
        char* candidate = join_path(dir, name);
        if (candidate != NULL)
        {
            if (is_executable_file(candidate))
            {
                free(copy);
                return candidate;
            }
            free(candidate);
        }
        save = (sep == NULL) ? NULL : sep + 1;
    }
    free(copy);
    return NULL;
}

char* simd_find_binary(void)
{
    const char* env_simd = getenv("SIMD");
    if (is_executable_file(env_simd))
    {
        return strdup(env_simd);
    }

    char* from_path = find_on_path("simd");
    if (from_path != NULL)
    {
        return from_path;
    }

    const char* home = getenv("HOME");
    const char* xdg_data = getenv("XDG_DATA_HOME");
    char* xdg_simd = NULL;
    char* home_simd = NULL;
    char* local_bin = NULL;
    char* found = NULL;

    if (xdg_data != NULL && xdg_data[0] != '\0')
    {
        asprintf(&xdg_simd, "%s/monocoque/simapi/simd/build/simd", xdg_data);
    }
    if (home != NULL)
    {
        asprintf(&home_simd, "%s/.local/share/monocoque/simapi/simd/build/simd", home);
        asprintf(&local_bin, "%s/.local/bin/simd", home);
    }

    const char* candidates[] = {
        xdg_simd,
        home_simd,
        local_bin,
        "/usr/local/bin/simd",
        "/usr/bin/simd",
        NULL
    };
    found = simd_find_binary_from_candidates(candidates);

    free(xdg_simd);
    free(home_simd);
    free(local_bin);
    return found;
}

static void remove_stale_pid_file(void)
{
    if (simd_process_running())
    {
        return;
    }
    if (access(SIMD_PID_FILE, F_OK) == 0)
    {
        if (unlink(SIMD_PID_FILE) == 0)
        {
            slogd("removed stale %s", SIMD_PID_FILE);
        }
    }
}

static int run_cmd_quiet(char* const argv[])
{
    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0)
    {
        return -1;
    }
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

    pid_t pid = 0;
    int spawn_rc = posix_spawn(&pid, argv[0], &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    if (spawn_rc != 0)
    {
        return -1;
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
        return -1;
    }
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    return -1;
}

static int try_systemd_start(void)
{
    char* systemctl = find_on_path("systemctl");
    if (systemctl == NULL)
    {
        return -1;
    }
    char* argv[] = { systemctl, "--user", "start", "simd.service", NULL };
    int rc = run_cmd_quiet(argv);
    free(systemctl);
    return rc;
}

static char** make_simd_env(void)
{
    const char* home = getenv("HOME");
    if (home == NULL)
    {
        home = "";
    }
    const char* old = getenv("LD_LIBRARY_PATH");
    char extra[PATH_MAX];
    snprintf(extra, sizeof(extra),
             "/usr/local/lib:/usr/local/lib64:%s/.local/lib:%s/.local/lib64",
             home, home);

    char ld[PATH_MAX * 2];
    if (old != NULL && old[0] != '\0')
    {
        snprintf(ld, sizeof(ld), "%s:%s", extra, old);
    }
    else
    {
        snprintf(ld, sizeof(ld), "%s", extra);
    }

    size_t count = 0;
    while (environ[count] != NULL)
    {
        count++;
    }

    char** envp = calloc(count + 2, sizeof(char*));
    if (envp == NULL)
    {
        return NULL;
    }

    size_t j = 0;
    int replaced = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (strncmp(environ[i], "LD_LIBRARY_PATH=", 16) == 0)
        {
            if (asprintf(&envp[j], "LD_LIBRARY_PATH=%s", ld) < 0)
            {
                envp[j] = NULL;
                break;
            }
            replaced = 1;
            j++;
        }
        else
        {
            envp[j++] = environ[i];
        }
    }
    if (replaced == 0)
    {
        if (asprintf(&envp[j], "LD_LIBRARY_PATH=%s", ld) < 0)
        {
            envp[j] = NULL;
        }
        else
        {
            j++;
        }
    }
    envp[j] = NULL;
    return envp;
}

static void free_simd_env(char** envp)
{
    if (envp == NULL)
    {
        return;
    }
    for (size_t i = 0; envp[i] != NULL; i++)
    {
        if (strncmp(envp[i], "LD_LIBRARY_PATH=", 16) == 0)
        {
            free(envp[i]);
        }
    }
    free(envp);
}

static int spawn_simd(const char* path)
{
    char* argv[] = { (char*)path, NULL };
    char** envp = make_simd_env();
    pid_t pid = 0;
    int rc = posix_spawn(&pid, path, NULL, NULL, argv, envp != NULL ? envp : environ);
    free_simd_env(envp);
    if (rc != 0)
    {
        sloge("failed to start simd at %s: %s", path, strerror(rc));
        return -1;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
        sloge("waitpid for simd failed: %s", strerror(errno));
        return -1;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        sloge("simd exited with status %d before daemonizing", WEXITSTATUS(status));
        return -1;
    }
    return 0;
}

static int wait_until_running(void)
{
    int waited = 0;
    while (waited < SIMD_START_TIMEOUT_MS)
    {
        if (simd_process_running())
        {
            return 1;
        }
        sleep_ms(SIMD_POLL_MS);
        waited += SIMD_POLL_MS;
    }
    return simd_process_running();
}

static void report_not_installed(void)
{
    const char* msg =
        "simd is required but is not installed.\n"
        "Install simd (package simd / simd-git, or run ./install.sh) and try again.";
    fprintf(stderr, "%s\n", msg);
    sloge("simd is required but is not installed");
}

SimdEnsureStatus ensure_simd(void)
{
    if (simd_process_running())
    {
        slogd("simd is already running");
        return SIMD_OK;
    }

    remove_stale_pid_file();

    if (try_systemd_start() == 0 && wait_until_running())
    {
        slogi("started simd via systemd user unit");
        return SIMD_OK;
    }

    char* path = simd_find_binary();
    if (path == NULL)
    {
        report_not_installed();
        return SIMD_NOT_INSTALLED;
    }

    slogi("starting simd from %s", path);
    if (spawn_simd(path) != 0)
    {
        free(path);
        fprintf(stderr, "simd failed to start. See logs or /tmp/simd.log\n");
        sloge("simd failed to start");
        return SIMD_START_FAILED;
    }
    free(path);

    if (!wait_until_running())
    {
        fprintf(stderr, "simd did not stay running after start. See /tmp/simd.log\n");
        sloge("simd did not stay running after start");
        return SIMD_START_FAILED;
    }

    slogi("simd is running");
    return SIMD_OK;
}
