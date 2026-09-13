#!/bin/bash
# Assert a source install produced launchers, configs, and binaries.
# Honours the same env vars as install.sh.
set -euo pipefail

INSTALL_DIR="${MONOCOQUE_INSTALL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/monocoque}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}"
BIN_DIR="${HOME}/.local/bin"

PASS=0
FAIL=0

log() { printf '%s\n' "$*"; }

need_file() {
    local path="$1"
    if [ -f "$path" ]; then
        log "PASS file $path"
        PASS=$((PASS + 1))
    else
        log "FAIL missing file $path"
        FAIL=$((FAIL + 1))
    fi
}

need_exec() {
    local path="$1"
    if [ -x "$path" ]; then
        log "PASS exec $path"
        PASS=$((PASS + 1))
    else
        log "FAIL missing executable $path"
        FAIL=$((FAIL + 1))
    fi
}

need_grep() {
    local path="$1"
    local pattern="$2"
    if grep -Eq -- "$pattern" "$path"; then
        log "PASS $path matches $pattern"
        PASS=$((PASS + 1))
    else
        log "FAIL $path does not match $pattern"
        FAIL=$((FAIL + 1))
    fi
}

need_exec "$BIN_DIR/start-monocoque"
need_exec "$BIN_DIR/start-simd"
need_exec "$BIN_DIR/test-monocoque"
need_exec "$BIN_DIR/monocoque-manager"

need_grep "$BIN_DIR/start-monocoque" 'play'

need_file "$CONFIG_DIR/monocoque/monocoque.config"
need_file "$CONFIG_DIR/simd/simd.config"
need_file "$CONFIG_DIR/systemd/user/simd.service"
need_grep "$CONFIG_DIR/systemd/user/simd.service" '^ExecStart='

if [ -x "$INSTALL_DIR/simapi/simd/build/simd" ]; then
    need_exec "$INSTALL_DIR/simapi/simd/build/simd"
elif command -v simd >/dev/null 2>&1; then
    need_exec "$(command -v simd)"
else
    log "FAIL simd binary not in $INSTALL_DIR or PATH"
    FAIL=$((FAIL + 1))
fi

if [ -x "$INSTALL_DIR/monocoque/build/monocoque" ]; then
    need_exec "$INSTALL_DIR/monocoque/build/monocoque"
elif command -v monocoque >/dev/null 2>&1; then
    need_exec "$(command -v monocoque)"
else
    log "FAIL monocoque binary not in $INSTALL_DIR or PATH"
    FAIL=$((FAIL + 1))
fi

log ""
log "Passed: $PASS  Failed: $FAIL"
if [ "$FAIL" -ne 0 ]; then
    exit 1
fi
