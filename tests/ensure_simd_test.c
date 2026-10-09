// Exercises simd binary discovery without spawning a daemon.
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/monocoque/helper/ensure_simd.h"

static int failures = 0;

static void fail(const char* msg)
{
    fprintf(stderr, "FAIL: %s\n", msg);
    failures++;
}

static char* make_temp_dir(void)
{
    char tmpl[] = "/tmp/monocoque-simd-test-XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (dir == NULL)
    {
        perror("mkdtemp");
        return NULL;
    }
    return strdup(dir);
}

static char* write_file(const char* dir, const char* name, int executable)
{
    char* path = NULL;
    if (asprintf(&path, "%s/%s", dir, name) < 0)
    {
        return NULL;
    }
    FILE* f = fopen(path, "w");
    if (f == NULL)
    {
        free(path);
        return NULL;
    }
    fputs("#!/bin/sh\nexit 0\n", f);
    fclose(f);
    if (executable)
    {
        chmod(path, 0755);
    }
    else
    {
        chmod(path, 0644);
    }
    return path;
}

int main(void)
{
    const char* empty[] = { NULL };
    char* found = simd_find_binary_from_candidates(empty);
    if (found != NULL)
    {
        fail("empty candidate list should return NULL");
        free(found);
    }

    const char* missing[] = { "/no/such/simd-binary", NULL };
    found = simd_find_binary_from_candidates(missing);
    if (found != NULL)
    {
        fail("missing path should return NULL");
        free(found);
    }

    char* dir = make_temp_dir();
    if (dir == NULL)
    {
        return 1;
    }

    char* not_exec = write_file(dir, "not-exec", 0);
    char* exec_path = write_file(dir, "simd", 1);
    if (not_exec == NULL || exec_path == NULL)
    {
        fail("could not create temp files");
    }
    else
    {
        const char* skip_not_exec[] = { not_exec, NULL };
        found = simd_find_binary_from_candidates(skip_not_exec);
        if (found != NULL)
        {
            fail("non-executable candidate should be skipped");
            free(found);
        }

        const char* prefer_exec[] = { not_exec, exec_path, NULL };
        found = simd_find_binary_from_candidates(prefer_exec);
        if (found == NULL || strcmp(found, exec_path) != 0)
        {
            fail("should return the first executable candidate");
        }
        free(found);
    }

    if (not_exec != NULL)
    {
        unlink(not_exec);
        free(not_exec);
    }
    if (exec_path != NULL)
    {
        unlink(exec_path);
        free(exec_path);
    }
    rmdir(dir);
    free(dir);

    if (failures > 0)
    {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    printf("ensure_simd_test: all checks passed\n");
    return 0;
}
