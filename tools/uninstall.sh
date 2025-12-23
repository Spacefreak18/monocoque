#!/bin/bash
# Monocoque Uninstaller
# Removes monocoque and all related components

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

INSTALL_DIR="${MONOCOQUE_INSTALL_DIR:-$HOME/.local/share/monocoque}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}"
BIN_DIR="$HOME/.local/bin"
SYSTEMD_DIR="$HOME/.config/systemd/user"

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║                  Monocoque Uninstaller                           ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
}

print_header

log_warn "This will remove:"
echo "  • Monocoque installation ($INSTALL_DIR)"
echo "  • Configuration files ($CONFIG_DIR/monocoque, $CONFIG_DIR/simd)"
echo "  • Launcher scripts ($BIN_DIR/start-*, test-monocoque)"
echo "  • systemd service files ($SYSTEMD_DIR/simd.service)"
echo "  • Log files (~/.cache/monocoque)"
echo ""
echo "This will NOT remove:"
echo "  • System dependencies (yder, libuv, etc.)"
echo "  • Compiled simapi library (/usr/local/lib/libsimapi.so)"
echo ""

read -p "Continue with uninstallation? [y/N]: " confirm

if [[ ! $confirm =~ ^[Yy]$ ]]; then
    log_info "Uninstallation cancelled"
    exit 0
fi

echo ""
log_info "Starting uninstallation..."

# Stop running services
log_info "Stopping running services..."
pkill -x simd 2>/dev/null || true
pkill -x monocoque 2>/dev/null || true
systemctl --user stop simd.service 2>/dev/null || true
systemctl --user disable simd.service 2>/dev/null || true

# Remove installation directory
if [ -d "$INSTALL_DIR" ]; then
    log_info "Removing installation directory..."
    rm -rf "$INSTALL_DIR"
    log_success "Installation directory removed"
else
    log_info "Installation directory not found, skipping"
fi

# Remove configuration (ask first)
if [ -d "$CONFIG_DIR/monocoque" ] || [ -d "$CONFIG_DIR/simd" ]; then
    read -p "Remove configuration files? [y/N]: " remove_config
    if [[ $remove_config =~ ^[Yy]$ ]]; then
        rm -rf "$CONFIG_DIR/monocoque" 2>/dev/null || true
        rm -rf "$CONFIG_DIR/simd" 2>/dev/null || true
        log_success "Configuration files removed"
    else
        log_info "Keeping configuration files"
    fi
fi

# Remove launcher scripts
log_info "Removing launcher scripts..."
rm -f "$BIN_DIR/start-simd" 2>/dev/null || true
rm -f "$BIN_DIR/start-monocoque" 2>/dev/null || true
rm -f "$BIN_DIR/test-monocoque" 2>/dev/null || true
rm -f "$BIN_DIR/monocoque-manager" 2>/dev/null || true

# Remove systemd services
if [ -f "$SYSTEMD_DIR/simd.service" ]; then
    log_info "Removing systemd service files..."
    rm -f "$SYSTEMD_DIR/simd.service"
    systemctl --user daemon-reload 2>/dev/null || true
fi

# Remove logs
if [ -d "$HOME/.cache/monocoque" ]; then
    read -p "Remove log files? [y/N]: " remove_logs
    if [[ $remove_logs =~ ^[Yy]$ ]]; then
        rm -rf "$HOME/.cache/monocoque"
        log_success "Log files removed"
    else
        log_info "Keeping log files"
    fi
fi

echo ""
log_success "Uninstallation complete!"
echo ""
log_info "To remove system dependencies and simapi library:"
echo "  • Remove packages: yder, libuv, argtable, libserialport, etc."
echo "  • Remove library: sudo rm /usr/local/lib/libsimapi.so*"
echo "  • Run: sudo ldconfig"
echo ""

