#!/bin/bash
# Installer checks inside distro containers (podman or docker).
# Usage:
#   test-install-containers.sh                  # detect + mocks + immutable + full installs
#   test-install-containers.sh syntax
#   test-install-containers.sh detect [name]
#   test-install-containers.sh mocks
#   test-install-containers.sh immutable
#   test-install-containers.sh deps <name>
#   test-install-containers.sh full <name>
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
INSTALLER="$ROOT/install.sh"
ENGINE="${CONTAINER_ENGINE:-}"

if [ -z "$ENGINE" ]; then
    if command -v docker >/dev/null 2>&1; then
        ENGINE=docker
    elif command -v podman >/dev/null 2>&1; then
        ENGINE=podman
    else
        echo "Need docker or podman" >&2
        exit 1
    fi
fi

if [ ! -f "$INSTALLER" ]; then
    echo "Missing $INSTALLER" >&2
    exit 1
fi

PASS=0
FAIL=0
LOGDIR="${TMPDIR:-/tmp}/monocoque-install-tests"
mkdir -p "$LOGDIR"

log() { printf '%s\n' "$*"; }

# name|image|family|immutable
DETECT_MATRIX=(
    "archlinux|archlinux:latest|arch|0"
    "fedora|fedora:43|fedora|0"
    "ubuntu|ubuntu:24.04|debian|0"
    "debian|debian:bookworm|debian|0"
)

# name|image  (full source install; debian bookworm is GCC 12 and cannot build current simapi)
FULL_MATRIX=(
    "archlinux|archlinux:latest"
    "fedora|fedora:43"
    "ubuntu|ubuntu:24.04"
)

lookup_row() {
    local name="$1"
    local row
    for row in "${DETECT_MATRIX[@]}"; do
        if [ "${row%%|*}" = "$name" ]; then
            printf '%s\n' "$row"
            return 0
        fi
    done
    return 1
}

lookup_full() {
    local name="$1"
    local row
    for row in "${FULL_MATRIX[@]}"; do
        if [ "${row%%|*}" = "$name" ]; then
            printf '%s\n' "$row"
            return 0
        fi
    done
    return 1
}

run_ctr() {
    local image="$1"
    shift
    "$ENGINE" run --rm \
        -v "$INSTALLER:/install.sh:ro" \
        -v "$ROOT:/src:ro" \
        "$image" "$@"
}

expect_detect() {
    local label="$1"
    local image="$2"
    local os_release="$3"
    local expect_family="$4"
    local expect_immutable="${5:-0}"
    local logf="$LOGDIR/${label}.detect.log"

    log "== detect: $label ($image) expect family=$expect_family immutable=$expect_immutable"
    local script='bash /install.sh --detect-only'
    if [ -n "$os_release" ]; then
        script="$(cat <<EOF
set -euo pipefail
cat > /etc/os-release << 'OSR'
$os_release
OSR
bash /install.sh --detect-only
EOF
)"
    fi

    local out=""
    if ! out="$(run_ctr "$image" bash -lc "$script" 2>"$logf")"; then
        log "FAIL $label detect (container error)"
        cat "$logf" >&2 || true
        FAIL=$((FAIL + 1))
        return 1
    fi
    printf '%s\n' "$out" | tee -a "$logf"
    if ! printf '%s\n' "$out" | grep -qx "DISTRO_FAMILY=$expect_family"; then
        log "FAIL $label family (want DISTRO_FAMILY=$expect_family)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    if ! printf '%s\n' "$out" | grep -qx "DISTRO_IMMUTABLE=$expect_immutable"; then
        log "FAIL $label immutable (want DISTRO_IMMUTABLE=$expect_immutable)"
        FAIL=$((FAIL + 1))
        return 1
    fi
    log "PASS $label detect"
    PASS=$((PASS + 1))
}

run_detect_named() {
    local name="${1:-}"
    local row image family immutable
    if [ -n "$name" ]; then
        row="$(lookup_row "$name")" || {
            log "Unknown detect target: $name"
            exit 1
        }
        IFS='|' read -r name image family immutable <<<"$row"
        expect_detect "$name" "$image" "" "$family" "$immutable"
        return
    fi
    for row in "${DETECT_MATRIX[@]}"; do
        IFS='|' read -r name image family immutable <<<"$row"
        expect_detect "$name" "$image" "" "$family" "$immutable" || true
    done
}

run_mocks() {
    expect_detect nobara fedora:43 "$(cat <<'EOF'
NAME="Nobara Linux"
ID=nobara
ID_LIKE=fedora
VERSION_ID=43
EOF
)" fedora 0 || true

    expect_detect linuxmint ubuntu:24.04 "$(cat <<'EOF'
NAME="Linux Mint"
ID=linuxmint
ID_LIKE="ubuntu debian"
VERSION_ID=22
EOF
)" debian 0 || true

    expect_detect bazzite fedora:43 "$(cat <<'EOF'
NAME="Bazzite"
ID=bazzite
ID_LIKE=fedora
VERSION_ID=43
VARIANT_ID=bazzite
EOF
)" fedora 1 || true
}

run_immutable() {
    local logf="$LOGDIR/bazzite.refuse.log"
    log "== immutable: bazzite must refuse a native install"
    local ec=0
    set +e
    run_ctr fedora:43 bash -lc 'cat > /etc/os-release <<EOF
ID=bazzite
ID_LIKE=fedora
VERSION_ID=43
VARIANT_ID=bazzite
EOF
bash /install.sh --from-source --allow-root --skip-bridges' >"$logf" 2>&1
    ec=$?
    set -e
    if [ "$ec" -eq 1 ] && grep -q "immutable distro" "$logf"; then
        log "PASS bazzite refuse"
        PASS=$((PASS + 1))
    else
        log "FAIL bazzite refuse (exit $ec)"
        tail -n 40 "$logf" >&2 || true
        FAIL=$((FAIL + 1))
        return 1
    fi
}

deps_or_full() {
    local label="$1"
    local image="$2"
    local mode="$3"
    local logf="$LOGDIR/${label}.${mode}.log"
    local extra=(--from-source --allow-root --skip-bridges)
    if [ "$mode" = "deps" ]; then
        extra+=(--deps-only)
        log "== deps-only: $label ($image)"
    else
        log "== full install: $label ($image)"
    fi

    if run_ctr "$image" bash /src/install.sh "${extra[@]}" >"$logf" 2>&1; then
        log "PASS $label $mode"
        PASS=$((PASS + 1))
    else
        log "FAIL $label $mode — see $logf"
        tail -n 80 "$logf" >&2 || true
        FAIL=$((FAIL + 1))
        return 1
    fi
}

run_full_named() {
    local mode="$1"
    local name="$2"
    local row image
    row="$(lookup_full "$name")" || {
        log "Unknown full-install target: $name (supported: archlinux fedora ubuntu)"
        exit 1
    }
    IFS='|' read -r name image <<<"$row"
    deps_or_full "$name" "$image" "$mode"
}

run_syntax() {
    log "== bash -n install.sh"
    bash -n "$INSTALLER"
    bash -n "$ROOT/tools/uninstall.sh"
    bash -n "$ROOT/tools/distro/test-install-containers.sh"
    log "PASS syntax"
    PASS=$((PASS + 1))
}

finish() {
    log ""
    log "Passed: $PASS  Failed: $FAIL"
    if [ "$FAIL" -ne 0 ]; then
        exit 1
    fi
}

cmd="${1:-all}"
arg="${2:-}"

case "$cmd" in
    syntax)
        run_syntax
        finish
        ;;
    detect)
        run_detect_named "$arg"
        finish
        ;;
    mocks)
        run_mocks
        finish
        ;;
    immutable)
        run_immutable
        finish
        ;;
    deps)
        [ -n "$arg" ] || { echo "usage: $0 deps <archlinux|fedora|ubuntu>" >&2; exit 1; }
        run_full_named deps "$arg"
        finish
        ;;
    full)
        [ -n "$arg" ] || { echo "usage: $0 full <archlinux|fedora|ubuntu>" >&2; exit 1; }
        run_full_named full "$arg"
        finish
        ;;
    all)
        log "Using $ENGINE ; repo $ROOT"
        run_syntax || true
        run_detect_named "" || true
        run_mocks || true
        run_immutable || true
        local_row=""
        for local_row in "${FULL_MATRIX[@]}"; do
            IFS='|' read -r name image <<<"$local_row"
            deps_or_full "$name" "$image" full || true
        done
        finish
        ;;
    -h|--help|help)
        sed -n '2,12p' "$0"
        ;;
    *)
        echo "Unknown command: $cmd" >&2
        sed -n '2,12p' "$0" >&2
        exit 1
        ;;
esac
