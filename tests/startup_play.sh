#!/bin/bash
# Play-mode startup checks against a built monocoque binary (no packaged simd).
# Usage: startup_play.sh missing|starts <monocoque-bin>
set -euo pipefail

SIMD_REQUIRED_MSG="simd is required but is not installed"
PLAY_TIMEOUT_SEC=8
MINIMAL_CONFIG='configs = (
    {
        sim = "default";
        car = "default";
        devices = ();
    }
);
'
FAKE_SIMD_SCRIPT='#!/bin/sh
if [ "${SIMD_TEST_CHILD:-}" != "1" ]; then
    SIMD_TEST_CHILD=1 setsid "$0" </dev/null >/dev/null 2>&1 &
    exit 0
fi
trap "" HUP
while true; do sleep 1; done
'

mode="${1:-}"
bin="${2:-}"

if [ -z "$mode" ] || [ -z "$bin" ] || [ ! -x "$bin" ]; then
    echo "usage: $0 missing|starts <monocoque-bin>" >&2
    exit 2
fi

if ! command -v timeout >/dev/null 2>&1; then
    echo "timeout(1) is required" >&2
    exit 2
fi

workdir="$(mktemp -d /tmp/monocoque-play-XXXXXX)"
cleanup() {
    if [ -n "${workdir:-}" ]; then
        pkill -f "$workdir/bin/simd" 2>/dev/null || true
        rm -rf "$workdir"
    fi
}
trap cleanup EXIT

mkdir -p "$workdir/bin" "$workdir/.config/monocoque" "$workdir/.cache/monocoque"
printf '%s\n' "$MINIMAL_CONFIG" > "$workdir/.config/monocoque/monocoque.config"

run_play() {
    env -i \
        HOME="$workdir" \
        XDG_CONFIG_HOME="$workdir/.config" \
        XDG_CACHE_HOME="$workdir/.cache" \
        XDG_DATA_HOME="$workdir/.local/share" \
        PATH="$1" \
        SIMD="${2:-}" \
        TERM=dumb \
        timeout --signal=TERM --kill-after=2 "$PLAY_TIMEOUT_SEC" \
        "$bin" play --disable_audio
}

case "$mode" in
    missing)
        if [ -x /usr/bin/simd ] || [ -x /usr/local/bin/simd ]; then
            echo "SKIP missing: a packaged simd is already on this system"
            exit 0
        fi
        if pgrep -x simd >/dev/null 2>&1; then
            echo "SKIP missing: simd is already running"
            exit 0
        fi
        set +e
        out="$(run_play "$workdir/bin:/bin:/usr/bin" "" 2>&1)"
        ec=$?
        set -e
        if ! printf '%s\n' "$out" | grep -Fq "$SIMD_REQUIRED_MSG"; then
            echo "FAIL missing: did not report that simd is required" >&2
            printf '%s\n' "$out" >&2
            exit 1
        fi
        echo "PASS missing simd is reported to the user"
        ;;
    starts)
        if pgrep -x simd >/dev/null 2>&1; then
            echo "SKIP starts: simd is already running"
            exit 0
        fi
        printf '%s\n' "$FAKE_SIMD_SCRIPT" > "$workdir/bin/simd"
        chmod +x "$workdir/bin/simd"
        set +e
        out="$(run_play "$workdir/bin:/bin:/usr/bin" "$workdir/bin/simd" 2>&1)"
        set -e
        if ! pgrep -f "$workdir/bin/simd" >/dev/null 2>&1; then
            echo "FAIL starts: fake simd was not left running" >&2
            printf '%s\n' "$out" >&2
            exit 1
        fi
        echo "PASS play mode started simd"
        ;;
    *)
        echo "unknown mode: $mode" >&2
        exit 2
        ;;
esac
