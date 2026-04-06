#!/bin/bash
# Monocoque Distrobox Installer
# For immutable Linux distributions (Bazzite, Silverblue, etc.)
# where native package managers are unavailable.
#
# Installs monocoque and dependencies inside an Arch Linux distrobox container,
# with wrapper scripts on the host for transparent access.

set -euo pipefail

SCRIPT_VERSION="1.0.0"
CONTAINER="simracing"
INSTALL_DIR="$HOME/.local/share/simracing"
BIN_DIR="$HOME/.local/bin"
CONFIG_DIR="$HOME/.config/monocoque"

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

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║          Monocoque Distrobox Installer v${SCRIPT_VERSION}                ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
}

# --- Main ---

print_header

# Check ~/.local/bin is in PATH
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    log_error "$BIN_DIR is not in your PATH."
    log_info "Add this to your shell config:"
    echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    exit 1
fi

# Check distrobox is available
if ! command -v distrobox &>/dev/null; then
    log_error "distrobox is not installed."
    log_info "Install distrobox first: https://distrobox.it/#installation"
    exit 1
fi

# Ensure dialout group exists and user is a member
log_info "Checking serial device access (dialout group)..."
if ! grep -q '^dialout:' /etc/group 2>/dev/null; then
    if grep -q '^dialout:' /usr/lib/group 2>/dev/null; then
        log_info "Adding dialout group to /etc/group (Fedora Atomic fix)..."
        grep -E '^dialout:' /usr/lib/group | sudo tee -a /etc/group >/dev/null
    else
        log_info "Creating dialout group..."
        sudo groupadd dialout
    fi
fi
if ! groups | grep -qw dialout; then
    log_info "Adding $USER to the dialout group for serial device access..."
    sudo usermod -aG dialout "$USER"
    log_warn "You will need to REBOOT after setup completes for serial access to work."
    echo ""
fi

# Prevent ModemManager from grabbing Moza serial devices
UDEV_RULE="/etc/udev/rules.d/99-moza-serial.rules"
if [ ! -f "$UDEV_RULE" ]; then
    if systemctl is-active --quiet ModemManager 2>/dev/null; then
        echo ""
        log_warn "ModemManager is running on your system."
        echo "  Moza wheel bases use USB serial devices that ModemManager may claim,"
        echo "  preventing monocoque from communicating with the wheel."
        echo ""
        echo "  A udev rule can tell ModemManager to ignore Moza devices (vendor 346e)."
        echo "  This only affects Moza USB devices — other modems will work normally."
        echo ""
        read -rp "Install udev rule to prevent ModemManager from grabbing Moza devices? [Y/n]: " mm_confirm
        if [[ "$mm_confirm" =~ ^[Nn]$ ]]; then
            echo ""
            log_info "Skipped udev rule. If monocoque can't talk to your wheel, run:"
            echo "  sudo tee $UDEV_RULE <<< 'ACTION==\"add|change\", SUBSYSTEM==\"usb\", ATTR{idVendor}==\"346e\", ENV{ID_MM_DEVICE_IGNORE}=\"1\"'"
            echo "  sudo udevadm control --reload-rules && sudo udevadm trigger"
            echo ""
        else
            log_info "Installing udev rule..."
            echo 'ACTION=="add|change", SUBSYSTEM=="usb", ATTR{idVendor}=="346e", ENV{ID_MM_DEVICE_IGNORE}="1"' | sudo tee "$UDEV_RULE" >/dev/null
            sudo udevadm control --reload-rules
            sudo udevadm trigger
            log_success "ModemManager udev rule installed"
        fi
    else
        # ModemManager not running — install the rule silently
        echo 'ACTION=="add|change", SUBSYSTEM=="usb", ATTR{idVendor}=="346e", ENV{ID_MM_DEVICE_IGNORE}="1"' | sudo tee "$UDEV_RULE" >/dev/null
        sudo udevadm control --reload-rules
        sudo udevadm trigger
    fi
fi

# Create the Arch Linux distrobox container
if ! distrobox list 2>/dev/null | grep -q "$CONTAINER"; then
    log_info "Creating Arch Linux container '$CONTAINER'..."
    distrobox create \
        --image docker.io/library/archlinux:latest \
        --name "$CONTAINER" \
        --pre-init-hooks "sed -i 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf && pacman -Sy --noconfirm archlinux-keyring && pacman-key --init && pacman-key --populate archlinux && sed -i 's/^SigLevel.*/SigLevel = Required DatabaseOptional/' /etc/pacman.conf"
    log_success "Container '$CONTAINER' created"
else
    log_info "Container '$CONTAINER' already exists, skipping creation."
fi

# Install packages inside the container
log_info "Installing packages inside container (this may take a few minutes)..."
distrobox enter "$CONTAINER" -- bash -c '
    set -euo pipefail

    # Install build dependencies
    echo "Installing build dependencies..."
    sudo pacman -Syu --needed --noconfirm base-devel git mingw-w64-gcc

    # Install yay (AUR helper) if not present
    if ! command -v yay &>/dev/null; then
        echo "Installing yay AUR helper..."
        TMPDIR=$(mktemp -d)
        git clone https://aur.archlinux.org/yay-bin.git "$TMPDIR/yay-bin"
        cd "$TMPDIR/yay-bin"
        makepkg -si --noconfirm
        cd /
        rm -rf "$TMPDIR"
    fi

    # Install AUR packages sequentially (simd depends on simapi, monocoque depends on both)
    echo "Installing AUR packages..."
    yay -S --needed --noconfirm simapi-git
    yay -S --needed --noconfirm simd-git
    yay -S --needed --noconfirm monocoque-git

    # Build simshmbridge (not on AUR)
    INSTALL_DIR="$HOME/.local/share/simracing"
    mkdir -p "$INSTALL_DIR"
    cd "$INSTALL_DIR"
    if [ ! -d "simshmbridge" ]; then
        echo "Cloning simshmbridge..."
        git clone --recurse-submodules "https://github.com/Spacefreak18/simshmbridge.git"
    else
        echo "simshmbridge already cloned, pulling latest..."
        cd simshmbridge && git pull && git submodule update --init --recursive && cd ..
    fi
    echo "Building simshmbridge..."
    cd "$INSTALL_DIR/simshmbridge"
    git submodule update --init --recursive
    make clean 2>/dev/null || true
    make -j$(nproc)

    echo ""
    echo "All components built successfully inside container."
'

# Install wrapper scripts to ~/.local/bin
log_info "Installing wrapper scripts..."
mkdir -p "$BIN_DIR"

cat > "$BIN_DIR/start-simd" << 'EOF'
#!/usr/bin/env bash
exec distrobox enter simracing -- simd "$@"
EOF
chmod +x "$BIN_DIR/start-simd"

cat > "$BIN_DIR/start-monocoque" << 'EOF'
#!/usr/bin/env bash
exec distrobox enter simracing -- monocoque play "$@"
EOF
chmod +x "$BIN_DIR/start-monocoque"

cat > "$BIN_DIR/test-monocoque" << 'EOF'
#!/usr/bin/env bash
exec distrobox enter simracing -- monocoque test -vv "$@"
EOF
chmod +x "$BIN_DIR/test-monocoque"

log_success "Wrapper scripts installed"

# Setup monocoque config
mkdir -p "$CONFIG_DIR"
if [ ! -f "$CONFIG_DIR/monocoque.config" ]; then
    log_info "Creating monocoque config template..."
    cat > "$CONFIG_DIR/monocoque.config" << 'EOF'
configs = (
    {
        sim = "default";
        car = "default";
        devices = (
        {
            device       = "Serial";
            type         = "Wheel";
            subtype      = "MozaNew";
            devpath      = "/dev/serial/by-id/CHANGE_ME_TO_YOUR_MOZA_BASE";
            baud         = 115200;
        });
    }
);
EOF
    log_success "Config created at $CONFIG_DIR/monocoque.config"
else
    log_info "Config already exists, skipping"
fi

# Done
echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║                  Installation Complete!                         ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
log_success "Monocoque (distrobox) installed!"
echo ""
echo "  Installed:"
echo "    AUR packages:  simapi-git, simd-git, monocoque-git (in '$CONTAINER' container)"
echo "    Bridge:        $INSTALL_DIR/simshmbridge/"
echo "    Config:        $CONFIG_DIR/monocoque.config"
echo "    Commands:      start-simd, start-monocoque, test-monocoque"
echo ""
echo "  Next steps:"
echo "    1. Set your device path in $CONFIG_DIR/monocoque.config"
echo "       (replace CHANGE_ME_TO_YOUR_MOZA_BASE with your device from /dev/serial/by-id/)"
echo "    2. Test with: test-monocoque"
echo ""
echo "  For additional sim racing convenience tools (game launcher, Steam"
echo "  configuration, Moza auto-detection), visit:"
echo "    https://github.com/cescofry/simracing-utilities"
echo ""
