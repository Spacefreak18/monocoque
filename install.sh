#!/bin/bash
# Monocoque Universal Installer
# Works on: Arch, Debian/Ubuntu, Fedora-based (incl. Nobara), openSUSE
set -euo pipefail

SCRIPT_VERSION="1.1.0"
INSTALL_DIR="${MONOCOQUE_INSTALL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/monocoque}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}"
BIN_DIR="${HOME}/.local/bin"
SIMAPI_PREFIX="${SIMAPI_PREFIX:-/usr/local}"
BRIDGE_RELEASE_URL="https://github.com/Spacefreak18/simshmbridge/releases/download/0.1.0/compatbinaries.zip"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

MODE="auto"
BUILD_BRIDGES=0
SKIP_BRIDGES=0
ALLOW_ROOT=0
DEPS_ONLY=0
DETECT_ONLY=0
FORCE_NATIVE=0
DO_DISTROBOX=0

DISTRO_ID=""
DISTRO_LIKE=""
DISTRO_VERSION=""
DISTRO_VARIANT=""
DISTRO_FAMILY="unknown"
DISTRO_IMMUTABLE=0

SCRIPT_DIR=""
LOCAL_SRC=""
MONOCOQUE_SRC=""
SIMD_BIN=""
MONOCOQUE_BIN=""

if [ -n "${BASH_SOURCE[0]:-}" ] && [ -f "${BASH_SOURCE[0]}" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if [ -f "$SCRIPT_DIR/CMakeLists.txt" ] && [ -d "$SCRIPT_DIR/src/monocoque" ]; then
        LOCAL_SRC="$SCRIPT_DIR"
    fi
fi

log_info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error()   { echo -e "${RED}[ERROR]${NC} $1"; }

print_header() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║          Monocoque Universal Installer v${SCRIPT_VERSION}             ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
}

usage() {
    cat <<EOF
Usage: $(basename "${BASH_SOURCE[0]:-install.sh}") [options]

Options:
  --from-source     Build simapi, simd, and monocoque from source (default)
  --aur             Install simapi-git, simd-git, then monocoque-git from AUR
  --distrobox       Print (and run, if distrobox exists) immutable-distro setup
  --build-bridges   Cross-compile simshmbridge with mingw instead of prebuilts
  --skip-bridges    Do not download or build simshmbridge compatibility EXEs
  --deps-only       Install build dependencies and exit
  --detect-only     Print distro detection results and exit
  --force-native    Ignore immutable-distro detection and install on the host
  --allow-root      Allow running as root (containers / CI)
  -h, --help        Show this help

Environment:
  MONOCOQUE_INSTALL_DIR   Install prefix (default: ~/.local/share/monocoque)
  SIMAPI_PREFIX           simapi install prefix (default: /usr/local)
EOF
}

run_root() {
    if [ "$(id -u)" -eq 0 ]; then
        "$@"
    else
        sudo "$@"
    fi
}

have_cmd() {
    command -v "$1" >/dev/null 2>&1
}

is_tty() {
    [ -t 0 ] && [ -t 1 ]
}

ensure_writable_dir() {
    local dir="$1"
    mkdir -p "$dir" 2>/dev/null || true
    if [ ! -d "$dir" ] || [ ! -w "$dir" ]; then
        log_error "Cannot write to $dir"
        ls -ld "$dir" 2>/dev/null || true
        log_info "If a previous sudo run created this path, fix ownership:"
        echo "    sudo chown -R \"\$USER:\$USER\" \"$dir\""
        return 1
    fi
}

read_os_release() {
    DISTRO_ID="unknown"
    DISTRO_LIKE=""
    DISTRO_VERSION=""
    DISTRO_VARIANT=""
    if [ -f /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        DISTRO_ID="${ID:-unknown}"
        DISTRO_LIKE="${ID_LIKE:-}"
        DISTRO_VERSION="${VERSION_ID:-}"
        DISTRO_VARIANT="${VARIANT_ID:-}"
    elif [ -f /etc/arch-release ]; then
        DISTRO_ID="arch"
    fi
}

detect_distro() {
    read_os_release

    local id_lc like_lc variant_lc
    id_lc="$(echo "$DISTRO_ID" | tr '[:upper:]' '[:lower:]')"
    like_lc="$(echo "$DISTRO_LIKE" | tr '[:upper:]' '[:lower:]')"
    variant_lc="$(echo "$DISTRO_VARIANT" | tr '[:upper:]' '[:lower:]')"

    DISTRO_IMMUTABLE=0
    if [ -e /run/ostree-booted ] || [ -d /ostree ]; then
        DISTRO_IMMUTABLE=1
    fi
    case "$id_lc" in
        bazzite|silverblue|kinoite|bluefin|aurora|steamos) DISTRO_IMMUTABLE=1 ;;
    esac
    case "$variant_lc" in
        silverblue|kinoite|sericea|bluefin|aurora|bazzite) DISTRO_IMMUTABLE=1 ;;
    esac

    DISTRO_FAMILY="unknown"
    case "$id_lc" in
        arch|manjaro|endeavouros|garuda|cachyos|archcraft|arcolinux)
            DISTRO_FAMILY="arch" ;;
        fedora|nobara|rhel|centos|rocky|almalinux|ol|ultramarine)
            DISTRO_FAMILY="fedora" ;;
        debian|ubuntu|linuxmint|pop|elementary|zorin|neon|kali|raspbian)
            DISTRO_FAMILY="debian" ;;
        opensuse*|suse|sles)
            DISTRO_FAMILY="opensuse" ;;
        bazzite|silverblue|kinoite|bluefin|aurora)
            DISTRO_FAMILY="fedora" ;;
        steamos)
            DISTRO_FAMILY="arch" ;;
    esac

    if [ "$DISTRO_FAMILY" = "unknown" ]; then
        case "$like_lc" in
            *arch*) DISTRO_FAMILY="arch" ;;
            *fedora*|*rhel*) DISTRO_FAMILY="fedora" ;;
            *debian*|*ubuntu*) DISTRO_FAMILY="debian" ;;
            *suse*) DISTRO_FAMILY="opensuse" ;;
        esac
    fi
}

print_detect() {
    echo "DISTRO_ID=$DISTRO_ID"
    echo "DISTRO_LIKE=$DISTRO_LIKE"
    echo "DISTRO_VERSION=$DISTRO_VERSION"
    echo "DISTRO_VARIANT=$DISTRO_VARIANT"
    echo "DISTRO_FAMILY=$DISTRO_FAMILY"
    echo "DISTRO_IMMUTABLE=$DISTRO_IMMUTABLE"
}

manual_dep_hint() {
    cat <<EOF
Required build packages (names vary by distro):
  git cmake gcc make pkg-config
  libuv argtable libserialport libconfig hidapi lua libxdg-basedir libxml2 libpulse
  yder (simd), python3
  optional: mingw-w64 (only with --build-bridges)

Arch:    pacman -S --needed git cmake base-devel libuv argtable libserialport libconfig hidapi lua54 libpulse pkgconf libxdg-basedir libxml2 python yder
Fedora:  dnf install git cmake gcc gcc-c++ make libuv-devel argtable-devel libserialport-devel libconfig-devel hidapi-devel lua-devel libxdg-basedir-devel libxml2-devel pulseaudio-libs-devel pkgconf-pkg-config python3
Debian:  apt install build-essential git cmake libuv1-dev libargtable2-dev libserialport-dev libconfig-dev libhidapi-dev liblua5.4-dev libxdg-basedir-dev libxml2-dev libpulse-dev pkg-config python3
EOF
}

print_immutable_help() {
    log_error "Detected immutable distro: $DISTRO_ID (family=$DISTRO_FAMILY)"
    echo ""
    echo "Do not install with the host package manager (rpm-ostree layering is a last resort)."
    echo "Use distrobox so the stack lives in a mutable container that shares \$HOME:"
    echo ""
    echo "    distrobox create --name monocoque --image archlinux:latest"
    echo "    distrobox enter monocoque"
    echo "    curl -fsSL https://raw.githubusercontent.com/Spacefreak18/monocoque/master/install.sh -o install.sh"
    echo "    bash install.sh --from-source"
    echo ""
    echo "Docs: https://spacefreak18.github.io/simapi/"
    echo "Override with --force-native if you really want to install on the host."
}

install_yder_from_source() {
    if pkg-config --exists yder 2>/dev/null; then
        log_info "yder already available"
        return 0
    fi

    log_warn "yder is not in the distro repos; building orcania + yder from source"
    local src="$INSTALL_DIR/src-deps"
    mkdir -p "$src"

    if [ ! -d "$src/orcania" ]; then
        git clone --depth 1 https://github.com/babelouest/orcania.git "$src/orcania"
    fi
    mkdir -p "$src/orcania/build"
    cmake -S "$src/orcania" -B "$src/orcania/build"
    cmake --build "$src/orcania/build" -j"$(nproc)"
    run_root cmake --install "$src/orcania/build"

    if [ ! -d "$src/yder" ]; then
        git clone --depth 1 https://github.com/babelouest/yder.git "$src/yder"
    fi
    mkdir -p "$src/yder/build"
    cmake -S "$src/yder" -B "$src/yder/build" -DWITH_JOURNALD=off
    cmake --build "$src/yder/build" -j"$(nproc)"
    run_root cmake --install "$src/yder/build"
    if [ -d /usr/local/lib64 ]; then
        echo "/usr/local/lib64" | run_root tee /etc/ld.so.conf.d/usr-local-lib64.conf >/dev/null
    fi
    run_root ldconfig 2>/dev/null || true
    export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"
    log_success "yder installed to /usr/local"
}

install_deps_arch() {
    local deps=(
        git cmake make gcc pkgconf python curl unzip
        libuv argtable libserialport libconfig hidapi lua54
        libpulse libxdg-basedir libxml2 yder procps-ng
    )
    if [ "$BUILD_BRIDGES" -eq 1 ]; then
        deps+=(mingw-w64-gcc)
    fi
    log_info "Installing Arch packages: ${deps[*]}"
    run_root pacman -Sy --needed --noconfirm "${deps[@]}"
}

install_deps_fedora() {
    local deps=(
        git cmake gcc gcc-c++ make pkgconf-pkg-config python3 curl unzip ca-certificates
        libuv-devel argtable-devel libserialport-devel libconfig-devel
        hidapi-devel lua-devel libxdg-basedir-devel libxml2-devel
        pulseaudio-libs-devel procps-ng-devel
    )
    if [ "$BUILD_BRIDGES" -eq 1 ]; then
        deps+=(mingw64-gcc)
    fi
    log_info "Installing Fedora packages: ${deps[*]}"
    run_root dnf install -y "${deps[@]}"

    if run_root dnf install -y yder-devel; then
        log_success "yder-devel installed from repos"
    else
        install_yder_from_source
    fi
}

install_deps_debian() {
    export DEBIAN_FRONTEND=noninteractive
    log_info "Updating apt package lists..."
    run_root apt-get update

    local deps=(
        build-essential git cmake pkg-config python3 curl unzip ca-certificates
        libuv1-dev libargtable2-dev libserialport-dev libconfig-dev
        libhidapi-dev libxdg-basedir-dev libxml2-dev libpulse-dev
    )
    if [ "$BUILD_BRIDGES" -eq 1 ]; then
        deps+=(mingw-w64)
    fi
    log_info "Installing Debian/Ubuntu packages: ${deps[*]}"
    run_root apt-get install -y "${deps[@]}"

    if ! run_root apt-get install -y libproc2-dev; then
        log_warn "libproc2-dev not available, trying libprocps-dev"
        run_root apt-get install -y libprocps-dev
    fi

    if ! run_root apt-get install -y liblua5.4-dev lua5.4; then
        log_warn "lua 5.4 not available, trying lua 5.3"
        run_root apt-get install -y liblua5.3-dev lua5.3
    fi

    if ! run_root apt-get install -y libyder-dev; then
        install_yder_from_source
    fi
}

install_deps_opensuse() {
    local deps=(
        git cmake gcc gcc-c++ make pkg-config python3 curl unzip
        libuv-devel argtable-devel libserialport-devel libconfig-devel
        hidapi-devel lua-devel libxdg-basedir-devel libxml2-devel
        libpulse-devel procps-devel
    )
    if [ "$BUILD_BRIDGES" -eq 1 ]; then
        deps+=(mingw64-gcc)
    fi
    log_info "Installing openSUSE packages: ${deps[*]}"
    run_root zypper install -y "${deps[@]}"
    if ! run_root zypper install -y libyder-devel; then
        install_yder_from_source
    fi
}

install_dependencies() {
    log_info "Installing dependencies for $DISTRO_ID (family=$DISTRO_FAMILY)..."
    case "$DISTRO_FAMILY" in
        arch) install_deps_arch ;;
        fedora) install_deps_fedora ;;
        debian) install_deps_debian ;;
        opensuse) install_deps_opensuse ;;
        *)
            log_error "Unsupported distribution: $DISTRO_ID (ID_LIKE=$DISTRO_LIKE)"
            manual_dep_hint
            exit 1
            ;;
    esac
    log_success "Dependencies installed"
}

check_requirements() {
    local missing=()
    local cmd
    for cmd in git cmake make gcc; do
        if ! have_cmd "$cmd"; then
            missing+=("$cmd")
        fi
    done
    if [ "${#missing[@]}" -ne 0 ]; then
        log_error "Missing required commands after dependency install: ${missing[*]}"
        exit 1
    fi
}

maybe_install_aur() {
    if [ "$MODE" = "aur" ]; then
        return 0
    fi
    if [ "$MODE" = "from-source" ]; then
        return 1
    fi
    if [ "$DISTRO_FAMILY" != "arch" ]; then
        return 1
    fi
    if ! have_cmd yay && ! have_cmd paru; then
        return 1
    fi
    if ! is_tty; then
        log_info "No TTY; skipping AUR prompt (use --aur to force)"
        return 1
    fi
    echo ""
    echo "You can install packaged builds from AUR instead of compiling:"
    echo "  yay -S simapi-git && yay -S simd-git && yay -S monocoque-git"
    echo ""
    local use_aur=""
    read -r -p "Install from AUR? [y/N]: " use_aur
    [[ "$use_aur" =~ ^[Yy]$ ]]
}

aur_helper() {
    if have_cmd yay; then
        echo yay
    elif have_cmd paru; then
        echo paru
    else
        log_error "AUR install requested but neither yay nor paru is installed"
        exit 1
    fi
}

install_from_aur() {
    local helper
    helper="$(aur_helper)"
    log_info "Installing AUR packages sequentially with $helper (simapi first)"
    "$helper" -S --needed simapi-git
    "$helper" -S --needed simd-git
    "$helper" -S --needed monocoque-git

    if [ "$SKIP_BRIDGES" -eq 0 ]; then
        install_bridges
    fi
    setup_configs
    create_launcher_scripts
    setup_systemd_services
    install_udev_rules
    verify_install
    print_next_steps
}

git_clone_or_update() {
    local url="$1"
    local dest="$2"
    local with_submodules="${3:-0}"

    if [ -d "$dest/.git" ]; then
        log_info "Updating $(basename "$dest")..."
        git -C "$dest" pull --ff-only || log_warn "git pull failed in $dest; using existing tree"
    else
        log_info "Cloning $url"
        git clone "$url" "$dest"
    fi
    if [ "$with_submodules" = "1" ]; then
        git -C "$dest" submodule sync --recursive
        git -C "$dest" submodule update --init --recursive
    fi
}

prepare_sources() {
    mkdir -p "$INSTALL_DIR"
    cd "$INSTALL_DIR"

    if [ -n "$LOCAL_SRC" ]; then
        log_info "Using local monocoque source: $LOCAL_SRC"
        mkdir -p "$INSTALL_DIR/monocoque"
        tar -C "$LOCAL_SRC" --exclude='./build' --exclude='./.git' -cf - . \
            | tar -C "$INSTALL_DIR/monocoque" -xf -
        MONOCOQUE_SRC="$INSTALL_DIR/monocoque"
    else
        git_clone_or_update https://github.com/Spacefreak18/monocoque.git "$INSTALL_DIR/monocoque" 1
        MONOCOQUE_SRC="$INSTALL_DIR/monocoque"
    fi

    local simapi_submodule="$MONOCOQUE_SRC/src/monocoque/simulatorapi/simapi"
    if [ ! -f "$simapi_submodule/simapi/simdata.h" ]; then
        log_error "simapi submodule is missing under $MONOCOQUE_SRC"
        log_info "Run: git submodule update --init --recursive"
        exit 1
    fi
    if [ ! -f "$simapi_submodule/simd/CMakeLists.txt" ]; then
        log_error "simapi submodule does not include simd ($simapi_submodule/simd)"
        exit 1
    fi

    # simd must come from the same simapi tree monocoque compiles against so both
    # share one SimData layout for /dev/shm/SIMAPI.DAT (do not clone simapi master).
    log_info "Using pinned simapi submodule for simd"
    rm -rf "$INSTALL_DIR/simapi"
    mkdir -p "$INSTALL_DIR/simapi"
    tar -C "$simapi_submodule" --exclude='./.git' --exclude='./build' --exclude='./simd/build' -cf - . \
        | tar -C "$INSTALL_DIR/simapi" -xf -
    if [ -e "$simapi_submodule/.git" ]; then
        log_info "simapi pin: $(git -C "$simapi_submodule" rev-parse --short HEAD 2>/dev/null || echo unknown)"
    fi
    log_success "Sources ready"
}

build_simapi() {
    log_info "Building simapi..."
    local gcc_major
    gcc_major="$(gcc -dumpfullversion -dumpversion 2>/dev/null | cut -d. -f1 || echo 0)"
    if [ "${gcc_major:-0}" -gt 0 ] && [ "$gcc_major" -lt 13 ]; then
        log_warn "GCC $gcc_major may fail to compile current simapi (C23 typed enums need GCC 13+)."
        log_info "On Debian 12 use the .deb from GitHub Releases, or Ubuntu 24.04 / Debian testing."
    fi
    mkdir -p "$INSTALL_DIR/simapi/build"
    cmake -S "$INSTALL_DIR/simapi" -B "$INSTALL_DIR/simapi/build" -DCMAKE_INSTALL_PREFIX="$SIMAPI_PREFIX"
    cmake --build "$INSTALL_DIR/simapi/build" -j"$(nproc)"
    run_root cmake --install "$INSTALL_DIR/simapi/build"
    run_root ldconfig 2>/dev/null || true

    if [ ! -f "$SIMAPI_PREFIX/include/simdata.h" ]; then
        log_error "simapi headers were not installed to $SIMAPI_PREFIX/include/simdata.h"
        log_info "simd will fail to compile without them"
        exit 1
    fi
    log_success "simapi installed ($SIMAPI_PREFIX/include/simdata.h)"
}

build_simd() {
    log_info "Building simd..."
    rm -rf "$INSTALL_DIR/simapi/simd/build"
    mkdir -p "$INSTALL_DIR/simapi/simd/build"
    cmake -S "$INSTALL_DIR/simapi/simd" -B "$INSTALL_DIR/simapi/simd/build" \
        -DCMAKE_PREFIX_PATH="$SIMAPI_PREFIX" \
        -DCMAKE_INCLUDE_PATH="$SIMAPI_PREFIX/include" \
        -DCMAKE_LIBRARY_PATH="$SIMAPI_PREFIX/lib;$SIMAPI_PREFIX/lib64" \
        -DCMAKE_C_FLAGS="-I$SIMAPI_PREFIX/include" \
        -DCMAKE_EXE_LINKER_FLAGS="-L$SIMAPI_PREFIX/lib -L$SIMAPI_PREFIX/lib64 -Wl,-rpath,$SIMAPI_PREFIX/lib -Wl,-rpath,$SIMAPI_PREFIX/lib64"
    cmake --build "$INSTALL_DIR/simapi/simd/build" -j"$(nproc)"
    SIMD_BIN="$INSTALL_DIR/simapi/simd/build/simd"
    if [ ! -x "$SIMD_BIN" ]; then
        log_error "simd binary was not produced"
        exit 1
    fi
    log_success "simd built"
}

build_monocoque() {
    log_info "Building monocoque..."
    mkdir -p "$MONOCOQUE_SRC/build"
    cmake -S "$MONOCOQUE_SRC" -B "$MONOCOQUE_SRC/build"
    cmake --build "$MONOCOQUE_SRC/build" -j"$(nproc)"
    MONOCOQUE_BIN="$MONOCOQUE_SRC/build/monocoque"
    if [ ! -x "$MONOCOQUE_BIN" ]; then
        log_error "monocoque binary was not produced"
        exit 1
    fi
    log_success "monocoque built"
}

install_prebuilt_bridges() {
    log_info "Downloading prebuilt simshmbridge compatibility binaries..."
    if ! have_cmd unzip; then
        log_warn "unzip not found; skip bridge download (install unzip or pass --build-bridges)"
        return 0
    fi
    mkdir -p "$INSTALL_DIR/simshmbridge/assets"
    local zip="$INSTALL_DIR/compatbinaries.zip"
    if curl -fsSL -o "$zip" "$BRIDGE_RELEASE_URL"; then
        unzip -o "$zip" -d "$INSTALL_DIR/simshmbridge/assets"
        log_success "Bridge EXEs extracted to $INSTALL_DIR/simshmbridge/assets"
        echo "    Set Steam launch option, for example:"
        echo "    SIMD_BRIDGE_EXE=$INSTALL_DIR/simshmbridge/assets/acbridge.exe %command%"
        echo "    See: https://spacefreak18.github.io/simapi/simd_usage"
    else
        log_warn "Could not download $BRIDGE_RELEASE_URL"
        log_info "UDP-only titles still work. Get EXEs from https://github.com/spacefreak18/simshmbridge/releases"
    fi
}

build_simshmbridge_from_source() {
    log_info "Building simshmbridge from source (mingw)..."
    git_clone_or_update https://github.com/spacefreak18/simshmbridge.git "$INSTALL_DIR/simshmbridge" 1
    make -C "$INSTALL_DIR/simshmbridge" clean || true
    make -C "$INSTALL_DIR/simshmbridge" -j"$(nproc)"
    log_success "simshmbridge built"
}

install_bridges() {
    if [ "$SKIP_BRIDGES" -eq 1 ]; then
        log_info "Skipping simshmbridge (--skip-bridges)"
        return 0
    fi
    if [ "$BUILD_BRIDGES" -eq 1 ]; then
        build_simshmbridge_from_source
    else
        install_prebuilt_bridges
    fi
}

setup_configs() {
    log_info "Setting up configuration files..."
    mkdir -p "$CONFIG_DIR/simd" "$CONFIG_DIR/monocoque"

    if [ ! -f "$CONFIG_DIR/simd/simd.config" ]; then
        if [ -f "$INSTALL_DIR/simapi/simd/conf/simd.config" ]; then
            cp "$INSTALL_DIR/simapi/simd/conf/simd.config" "$CONFIG_DIR/simd/simd.config"
            log_success "Created $CONFIG_DIR/simd/simd.config"
        else
            log_warn "simd example config not found; create $CONFIG_DIR/simd/simd.config from simapi docs"
        fi
    else
        log_info "simd config already exists, skipping"
    fi

    local example_src=""
    if [ -f "$MONOCOQUE_SRC/conf/monocoque.config" ]; then
        example_src="$MONOCOQUE_SRC/conf/monocoque.config"
    elif [ -f "$INSTALL_DIR/monocoque/conf/monocoque.config" ]; then
        example_src="$INSTALL_DIR/monocoque/conf/monocoque.config"
    fi
    if [ -n "$example_src" ]; then
        cp "$example_src" "$CONFIG_DIR/monocoque/monocoque.config.example"
    fi

    if [ ! -f "$CONFIG_DIR/monocoque/monocoque.config" ]; then
        cat > "$CONFIG_DIR/monocoque/monocoque.config" << 'EOF'
// Starter config — add only devices you actually have.
// Full examples: ~/.config/monocoque/monocoque.config.example
// Device docs: https://spacefreak18.github.io/simapi/
configs = (
    {
        sim = "default";
        car = "default";
        devices = (
        // Serial wheel / Arduino example
        /*
        {
            device       = "Serial";
            type         = "Wheel";
            subtype      = "MozaR5";
            baud         = 115200;
            devpath      = "/dev/ttyACM0";
        },
        */
        // Bass shaker (use `pactl list sinks` for devid)
        /*
        {
            device       = "Sound";
            effect       = "Engine";
            devid        = "alsa_output.your_device_here";
            pan          = 0;
            fps          = 60;
            threshold    = 0.2;
            channels     = 2;
            volume       = 70;
            modulation   = "frequency";
            frequency    = 17;
            frequencyMax = 37;
        },
        */
        );
    }
);
EOF
        log_success "Created $CONFIG_DIR/monocoque/monocoque.config"
    else
        log_info "monocoque config already exists, skipping"
    fi
}

resolve_binaries() {
    if [ -z "${SIMD_BIN}" ] && [ -x "$INSTALL_DIR/simapi/simd/build/simd" ]; then
        SIMD_BIN="$INSTALL_DIR/simapi/simd/build/simd"
    fi
    if [ -z "${SIMD_BIN}" ] && have_cmd simd; then
        SIMD_BIN="$(command -v simd)"
    fi
    if [ -z "${MONOCOQUE_BIN}" ] && [ -x "$INSTALL_DIR/monocoque/build/monocoque" ]; then
        MONOCOQUE_BIN="$INSTALL_DIR/monocoque/build/monocoque"
    fi
    if [ -z "${MONOCOQUE_BIN}" ] && have_cmd monocoque; then
        MONOCOQUE_BIN="$(command -v monocoque)"
    fi
}

create_launcher_scripts() {
    log_info "Creating launcher scripts in $BIN_DIR..."
    ensure_writable_dir "$BIN_DIR"
    resolve_binaries

    cat > "$BIN_DIR/start-simd" << EOF
#!/bin/bash
export LD_LIBRARY_PATH="\${LD_LIBRARY_PATH:-}:$SIMAPI_PREFIX/lib:$SIMAPI_PREFIX/lib64"
BIN="${SIMD_BIN:-simd}"
if [ ! -x "\$BIN" ]; then
    echo "simd not found at \$BIN" >&2
    exit 1
fi
exec "\$BIN" "\$@"
EOF
    chmod +x "$BIN_DIR/start-simd"

    cat > "$BIN_DIR/start-monocoque" << EOF
#!/bin/bash
BIN="${MONOCOQUE_BIN:-monocoque}"
if [ ! -x "\$BIN" ]; then
    echo "monocoque not found at \$BIN" >&2
    exit 1
fi
exec "\$BIN" play "\$@"
EOF
    chmod +x "$BIN_DIR/start-monocoque"

    cat > "$BIN_DIR/test-monocoque" << EOF
#!/bin/bash
BIN="${MONOCOQUE_BIN:-monocoque}"
if [ ! -x "\$BIN" ]; then
    echo "monocoque not found at \$BIN" >&2
    exit 1
fi
exec "\$BIN" test -vv "\$@"
EOF
    chmod +x "$BIN_DIR/test-monocoque"

    local search_paths=(
        "$INSTALL_DIR/monocoque/tools/monocoque-manager"
        "$INSTALL_DIR/monocoque/monocoque-manager"
        "${MONOCOQUE_SRC:-}/tools/monocoque-manager"
        "${SCRIPT_DIR:-}/tools/monocoque-manager"
        "${SCRIPT_DIR:-}/monocoque-manager"
    )
    local manager=""
    local path
    for path in "${search_paths[@]}"; do
        [ -n "$path" ] || continue
        if [ -f "$path" ]; then
            manager="$path"
            break
        fi
    done
    if [ -z "$manager" ]; then
        mkdir -p "$INSTALL_DIR"
        if curl -fsSL -o "$INSTALL_DIR/monocoque-manager" \
            "https://raw.githubusercontent.com/Spacefreak18/monocoque/master/tools/monocoque-manager"; then
            manager="$INSTALL_DIR/monocoque-manager"
            log_info "Downloaded monocoque-manager from GitHub"
        fi
    fi
    if [ -n "$manager" ]; then
        cp "$manager" "$BIN_DIR/monocoque-manager"
        chmod +x "$BIN_DIR/monocoque-manager"
        log_success "Installed monocoque-manager from $manager"
    else
        log_warn "monocoque-manager not found (looked in cloned tree, not the directory you launched install.sh from)"
    fi

    if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
        log_warn "$BIN_DIR is not in PATH. Add this to your shell config:"
        echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    fi
    log_success "Launcher scripts created"
}

setup_systemd_services() {
    local systemd_dir="$CONFIG_DIR/systemd/user"
    log_info "Creating systemd user service..."
    resolve_binaries

    if [ -z "${SIMD_BIN}" ]; then
        log_warn "simd binary not found; skipping systemd unit"
        return 0
    fi
    if ! ensure_writable_dir "$systemd_dir"; then
        log_warn "Skipping systemd unit (directory not writable)"
        return 0
    fi

    cat > "$systemd_dir/simd.service" << EOF
[Unit]
Description=Sim Telemetry Daemon
Documentation=https://spacefreak18.github.io/simapi/
After=default.target

[Service]
Type=simple
Environment=LD_LIBRARY_PATH=$SIMAPI_PREFIX/lib:$SIMAPI_PREFIX/lib64
ExecStart=$SIMD_BIN
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
EOF

    if have_cmd systemctl; then
        systemctl --user daemon-reload 2>/dev/null || true
    fi
    log_success "Wrote $systemd_dir/simd.service"
    log_info "Enable with: systemctl --user enable --now simd.service"
    log_info "If it should start at login while not logged in graphically:"
    echo "    loginctl enable-linger \$USER"
}

install_udev_rules() {
    local rules=""
    if [ -f "${MONOCOQUE_SRC:-}/udev/69-monocoque.rules" ]; then
        rules="$MONOCOQUE_SRC/udev/69-monocoque.rules"
    elif [ -f "$INSTALL_DIR/monocoque/udev/69-monocoque.rules" ]; then
        rules="$INSTALL_DIR/monocoque/udev/69-monocoque.rules"
    fi
    if [ -z "$rules" ]; then
        log_warn "No udev rules found in the source tree"
        return 0
    fi
    if run_root mkdir -p /etc/udev/rules.d && run_root cp "$rules" /etc/udev/rules.d/69-monocoque.rules; then
        run_root udevadm control --reload-rules 2>/dev/null || true
        log_success "Installed udev rules (some Arduino matches are serial-specific; edit if needed)"
        log_info "Serial/HID access usually needs group membership:"
        echo "    sudo usermod -aG input,dialout,uucp \$USER"
        echo "    (log out and back in after changing groups)"
    else
        log_warn "Could not install udev rules. Copy $rules to /etc/udev/rules.d/"
    fi
}

verify_install() {
    log_info "Verifying installation..."
    resolve_binaries
    local ok=1
    if [ -n "${MONOCOQUE_BIN}" ] && [ -x "$MONOCOQUE_BIN" ]; then
        log_success "monocoque: $MONOCOQUE_BIN"
    else
        log_error "monocoque binary missing"
        ok=0
    fi
    if [ -n "${SIMD_BIN}" ] && [ -x "$SIMD_BIN" ]; then
        log_success "simd: $SIMD_BIN"
    else
        log_error "simd binary missing"
        ok=0
    fi
    if [ -f "$BIN_DIR/monocoque-manager" ]; then
        log_success "manager: $BIN_DIR/monocoque-manager"
    else
        log_warn "monocoque-manager was not installed"
    fi
    if [ "$ok" -ne 1 ]; then
        exit 1
    fi
}

print_next_steps() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║                    Installation Complete!                        ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
    echo "Install dir:  $INSTALL_DIR"
    echo "Config:       $CONFIG_DIR/simd  $CONFIG_DIR/monocoque"
    echo "Launchers:    $BIN_DIR"
    echo ""
    echo "This installer does not configure Steam, Pulse/PipeWire sinks, or devices."
    echo ""
    echo "Start order (required):"
    echo "  1. start-simd"
    echo "  2. Launch the game (with SIMD_BRIDGE_EXE=... %command% if the sim needs a bridge)"
    echo "  3. start-monocoque"
    echo ""
    echo "Confirm telemetry after the session is live:"
    echo "  hexdump /dev/shm/SIMAPI.DAT | head"
    echo "  hexdump /dev/shm/acpmf_physics | head     # AC / ACC"
    echo ""
    echo "Enable simd at login:"
    echo "  systemctl --user enable --now simd.service"
    echo ""
    echo "Edit devices:  $CONFIG_DIR/monocoque/monocoque.config"
    echo "Examples:      $CONFIG_DIR/monocoque/monocoque.config.example"
    echo "Test devices:  test-monocoque"
    echo "TUI:           monocoque-manager"
    echo ""
    echo "Game setup:    https://spacefreak18.github.io/simapi/simd_usage"
    echo "Docs:          https://spacefreak18.github.io/simapi/"
    echo ""
}

run_distrobox() {
    if have_cmd distrobox; then
        log_info "Creating Arch distrobox 'monocoque' (if needed)..."
        distrobox create --name monocoque --image archlinux:latest --yes || true
        local script_arg="bash -c 'curl -fsSL https://raw.githubusercontent.com/Spacefreak18/monocoque/master/install.sh | bash -s -- --from-source'"
        if [ -n "$SCRIPT_DIR" ] && [ -f "$SCRIPT_DIR/install.sh" ]; then
            script_arg="bash \"$SCRIPT_DIR/install.sh\" --from-source"
        fi
        log_info "Running installer inside distrobox..."
        # shellcheck disable=SC2086
        distrobox enter monocoque -- $script_arg
        return 0
    fi
    print_immutable_help
    exit 1
}

parse_args() {
    while [ $# -gt 0 ]; do
        case "$1" in
            --from-source) MODE="from-source" ;;
            --aur) MODE="aur" ;;
            --distrobox) DO_DISTROBOX=1 ;;
            --build-bridges) BUILD_BRIDGES=1 ;;
            --skip-bridges) SKIP_BRIDGES=1 ;;
            --deps-only) DEPS_ONLY=1 ;;
            --detect-only) DETECT_ONLY=1 ;;
            --force-native) FORCE_NATIVE=1 ;;
            --allow-root) ALLOW_ROOT=1 ;;
            -h|--help) usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
        shift
    done
}

main() {
    parse_args "$@"
    detect_distro

    if [ "$DETECT_ONLY" -eq 1 ]; then
        print_detect
        exit 0
    fi

    print_header
    log_info "Detected $DISTRO_ID $DISTRO_VERSION (family=$DISTRO_FAMILY, immutable=$DISTRO_IMMUTABLE)"

    if [ "$DO_DISTROBOX" -eq 1 ] || { [ "$DISTRO_IMMUTABLE" -eq 1 ] && [ "$FORCE_NATIVE" -eq 0 ]; }; then
        if [ "$DO_DISTROBOX" -eq 1 ]; then
            run_distrobox
            exit $?
        fi
        print_immutable_help
        exit 1
    fi

    if [ "$(id -u)" -eq 0 ] && [ "$ALLOW_ROOT" -eq 0 ]; then
        log_error "Do not run as root. Use a normal user with sudo, or pass --allow-root for containers."
        exit 1
    fi

    if [ "$MODE" = "aur" ]; then
        install_dependencies
        check_requirements
        install_from_aur
        exit 0
    fi

    install_dependencies
    check_requirements

    if [ "$DEPS_ONLY" -eq 1 ]; then
        log_success "Dependencies only; done"
        exit 0
    fi

    if maybe_install_aur; then
        install_from_aur
        exit 0
    fi

    log_info "Building from source (this may take a few minutes)..."
    prepare_sources
    build_simapi
    build_simd
    build_monocoque
    install_bridges
    setup_configs
    create_launcher_scripts
    setup_systemd_services
    install_udev_rules
    verify_install
    print_next_steps
}

main "$@"
