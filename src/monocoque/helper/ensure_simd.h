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

int simd_process_running(void);

/* Caller frees. Returns NULL if none are usable. */
char* simd_find_binary_from_candidates(const char* const* candidates);

/* Locate via $SIMD, PATH, then install locations. Caller frees. */
char* simd_find_binary(void);

/* No-op if already running; otherwise start via user unit or binary. */
SimdEnsureStatus ensure_simd(void);

#ifdef __cplusplus
}
#endif

#endif
