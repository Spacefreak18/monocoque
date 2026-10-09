#!/bin/bash
# After a source install, prove play mode starts simd without a human launch order.
set -euo pipefail

INSTALL_DIR="${MONOCOQUE_INSTALL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/monocoque}"
BIN_DIR="${HOME}/.local/bin"
PLAY_TIMEOUT_SEC=12
SIMD_REQUIRED_MSG="simd is required but is not installed"

export PATH="$BIN_DIR:$PATH"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}:/usr/local/lib:/usr/local/lib64"

MONOCOQUE_BIN="${MONOCOQUE_BIN:-}"
if [ -z "$MONOCOQUE_BIN" ] && [ -x "$INSTALL_DIR/monocoque/build/monocoque" ]; then
    MONOCOQUE_BIN="$INSTALL_DIR/monocoque/build/monocoque"
fi
if [ -z "$MONOCOQUE_BIN" ] && command -v monocoque >/dev/null 2>&1; then
    MONOCOQUE_BIN="$(command -v monocoque)"
fi
if [ -z "$MONOCOQUE_BIN" ] || [ ! -x "$MONOCOQUE_BIN" ]; then
    echo "monocoque binary not found" >&2
    exit 1
fi

if ! command -v timeout >/dev/null 2>&1; then
    echo "timeout(1) is required" >&2
    exit 1
fi

pkill -x simd 2>/dev/null || true
sleep 1

isolated="$(mktemp -d /tmp/monocoque-startup-missing-XXXXXX)"
mkdir -p "$isolated/.config/monocoque" "$isolated/.cache/monocoque"
cat > "$isolated/.config/monocoque/monocoque.config" << 'EOF'
configs = (
    {
        sim = "default";
        car = "default";
        devices = ();
    }
);
EOF

set +e
missing_out="$(
    env -i \
        HOME="$isolated" \
        XDG_CONFIG_HOME="$isolated/.config" \
        XDG_CACHE_HOME="$isolated/.cache" \
        XDG_DATA_HOME="$isolated/.local/share" \
        PATH="/bin:/usr/bin" \
        TERM=dumb \
        timeout --signal=TERM --kill-after=2 6 \
        "$MONOCOQUE_BIN" play --disable_audio 2>&1
)"
set -e
rm -rf "$isolated"

if ! printf '%s\n' "$missing_out" | grep -Fq "$SIMD_REQUIRED_MSG"; then
    echo "FAIL: play mode did not demand simd when it was not installed" >&2
    printf '%s\n' "$missing_out" >&2
    exit 1
fi
echo "PASS play mode reports missing simd"

set +e
timeout --signal=TERM --kill-after=2 "$PLAY_TIMEOUT_SEC" \
    "$MONOCOQUE_BIN" play --disable_audio >/tmp/monocoque-startup-play.log 2>&1
set -e

if ! pgrep -x simd >/dev/null 2>&1; then
    echo "FAIL: play mode did not start simd" >&2
    tail -n 80 /tmp/monocoque-startup-play.log >&2 || true
    exit 1
fi
echo "PASS play mode started simd"

pkill -x simd 2>/dev/null || true
exit 0
