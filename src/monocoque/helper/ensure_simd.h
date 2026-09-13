#ifndef _ENSURE_SIMD_H
#define _ENSURE_SIMD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SIMD_OK             = 0,
    SIMD_NOT_INSTALLED  = 1,
    SIMD_START_FAILED   = 2
}
SimdEnsureStatus;

/* True if a process whose name/cmdline matches simd is alive. */
int simd_process_running(void);

/*
 * Return the first executable path in a NULL-terminated candidate list.
 * Caller frees. Returns NULL if none are usable.
 */
char* simd_find_binary_from_candidates(const char* const* candidates);

/*
 * Locate simd: $SIMD, PATH, then well-known install locations.
 * Caller frees. Returns NULL if simd is not installed.
 */
char* simd_find_binary(void);

/*
 * Closed startup for the telemetry daemon. If simd is already running this
 * is a no-op. Otherwise start it (user unit if present, else the binary).
 * The only expected human action is installing simd when it is missing.
 */
SimdEnsureStatus ensure_simd(void);

#ifdef __cplusplus
}
#endif

#endif
