#!/bin/bash
# Monocoque Distrobox Uninstaller
# Removes the distrobox-based monocoque installation.

set -euo pipefail

CONTAINER="simracing"
BIN_DIR="$HOME/.local/bin"
INSTALL_DIR="$HOME/.local/share/simracing"
CONFIG_DIR="$HOME/.config/monocoque"
UDEV_RULE="/etc/udev/rules.d/99-moza-serial.rules"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_header() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║                Monocoque Distrobox Uninstaller                  ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
}

print_header

log_warn "This will remove:"
echo "  - Distrobox container '$CONTAINER'"
echo "  - Wrapper scripts ($BIN_DIR/start-simd, start-monocoque, test-monocoque)"
echo "  - simshmbridge source ($INSTALL_DIR)"
echo ""
echo "This will NOT remove:"
echo "  - Configuration files ($CONFIG_DIR)"
echo "  - Convenience scripts (simracing-launch, configure-moza, etc.)"
echo ""

read -rp "Continue with uninstallation? [y/N]: " confirm
if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
    log_info "Uninstallation cancelled"
    exit 0
fi

echo ""

# Remove wrapper scripts
log_info "Removing wrapper scripts..."
rm -f "$BIN_DIR/start-simd"
rm -f "$BIN_DIR/start-monocoque"
rm -f "$BIN_DIR/test-monocoque"
log_success "Wrapper scripts removed"

# Remove simshmbridge source
if [ -d "$INSTALL_DIR" ]; then
    log_info "Removing simshmbridge source..."
    rm -rf "$INSTALL_DIR"
    log_success "simshmbridge source removed"
fi

# Remove distrobox container
if distrobox list 2>/dev/null | grep -q "$CONTAINER"; then
    log_info "Removing distrobox container '$CONTAINER'..."
    distrobox rm --force "$CONTAINER"
    log_success "Container '$CONTAINER' removed"
else
    log_info "Container '$CONTAINER' not found, skipping"
fi

# Optionally remove udev rule
if [ -f "$UDEV_RULE" ]; then
    echo ""
    read -rp "Remove ModemManager udev rule ($UDEV_RULE)? [y/N]: " remove_udev
    if [[ "$remove_udev" =~ ^[Yy]$ ]]; then
        sudo rm -f "$UDEV_RULE"
        sudo udevadm control --reload-rules
        sudo udevadm trigger
        log_success "udev rule removed"
    else
        log_info "Keeping udev rule"
    fi
fi

echo ""
log_success "Uninstallation complete!"
echo ""
log_info "Config files preserved at $CONFIG_DIR"
echo "  To remove those too: rm -rf $CONFIG_DIR"
echo ""
