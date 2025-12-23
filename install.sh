#!/bin/bash
set -e

# Monocoque Universal Installer
# Works on: Arch, Debian/Ubuntu, Fedora, and other major distros
# Version: 1.0.0

SCRIPT_VERSION="1.0.0"
INSTALL_DIR="${MONOCOQUE_INSTALL_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/monocoque}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}"
BIN_DIR="${HOME}/.local/bin"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
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
    echo "║          Monocoque Universal Installer v${SCRIPT_VERSION}             ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
}

# Detect distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
        DISTRO_LIKE=$ID_LIKE
    elif [ -f /etc/arch-release ]; then
        DISTRO="arch"
    else
        DISTRO="unknown"
    fi
    
    # Normalize Arch-based distributions
    if [[ "$DISTRO_LIKE" == *"arch"* ]] || [[ "$DISTRO" == "manjaro" ]] || [[ "$DISTRO" == "garuda" ]] || [[ "$DISTRO" == "endeavouros" ]]; then
        log_info "Detected distribution: $DISTRO (Arch-based)"
        DISTRO="arch"
    else
        log_info "Detected distribution: $DISTRO"
    fi
}

# Check if running as root
check_root() {
    if [ "$EUID" -eq 0 ]; then
        log_error "Please do not run this script as root"
        exit 1
    fi
}

# Check for required commands
check_requirements() {
    local missing_commands=()
    
    for cmd in git cmake make gcc; do
        if ! command -v $cmd &> /dev/null; then
            missing_commands+=($cmd)
        fi
    done
    
    if [ ${#missing_commands[@]} -ne 0 ]; then
        log_error "Missing required commands: ${missing_commands[*]}"
        log_info "Please install them first"
        exit 1
    fi
}

# Install dependencies based on distro
install_dependencies() {
    log_info "Installing dependencies for $DISTRO..."
    
    case $DISTRO in
        arch|manjaro)
            local deps="yder libuv argtable libserialport libconfig hidapi lua libxdg-basedir mingw-w64-gcc"
            
            # Check if yay is available for AUR
            if command -v yay &> /dev/null; then
                log_info "Using yay to check for AUR packages..."
                echo ""
                echo "You can install monocoque from AUR instead:"
                echo "  yay -S simapi-git simd-git monocoque-git"
                echo ""
                read -p "Install from AUR? (recommended) [Y/n]: " use_aur
                
                if [[ ! $use_aur =~ ^[Nn]$ ]]; then
                    log_info "Installing from AUR..."
                    yay -S --needed simapi-git simd-git monocoque-git
                    
                    # Still need simshmbridge
                    log_info "Building simshmbridge (not in AUR)..."
                    build_simshmbridge_only
                    
                    # Setup configs and services
                    setup_configs
                    setup_systemd_services
                    # create_launcher_scripts uses SCRIPT_DIR which is set in main()
                    create_launcher_scripts
                    
                    log_success "Installation complete (AUR method)!"
                    print_next_steps
                    exit 0
                fi
            fi
            
            log_info "Installing build dependencies with pacman..."
            sudo pacman -S --needed --noconfirm $deps
            ;;
            
        ubuntu|debian|linuxmint|pop)
            local deps="build-essential git cmake libyder-dev libuv1-dev libargtable2-dev libserialport-dev libconfig-dev libhidapi-dev lua5.3 liblua5.3-dev libxdg-basedir-dev mingw-w64"
            
            log_info "Updating package lists..."
            sudo apt-get update
            
            log_info "Installing build dependencies..."
            sudo apt-get install -y $deps
            ;;
            
        fedora|rhel|centos)
            local deps="git cmake gcc gcc-c++ yder-devel libuv-devel argtable-devel libserialport-devel libconfig-devel hidapi-devel lua-devel libxdg-basedir-devel mingw64-gcc"
            
            log_info "Installing build dependencies..."
            sudo dnf install -y $deps
            ;;
            
        opensuse*)
            local deps="git cmake gcc gcc-c++ libyder-devel libuv-devel argtable-devel libserialport-devel libconfig-devel hidapi-devel lua-devel libxdg-basedir-devel mingw64-gcc"
            
            log_info "Installing build dependencies..."
            sudo zypper install -y $deps
            ;;
            
        *)
            log_error "Unsupported distribution: $DISTRO"
            log_info "Please install dependencies manually and try again"
            log_info "Required: git, cmake, gcc, yder, libuv, argtable, libserialport, libconfig, hidapi, lua, libxdg-basedir, mingw-gcc"
            exit 1
            ;;
    esac
    
    log_success "Dependencies installed"
}

# Clone repositories
clone_repositories() {
    log_info "Creating installation directory: $INSTALL_DIR"
    mkdir -p "$INSTALL_DIR"
    cd "$INSTALL_DIR"
    
    # Clone monocoque
    if [ ! -d "monocoque" ]; then
        log_info "Cloning monocoque..."
        git clone https://github.com/Spacefreak18/monocoque.git
        cd monocoque
        git submodule sync --recursive
        git submodule update --init --recursive
    else
        log_info "monocoque already cloned, updating..."
        cd monocoque
        git pull
        git submodule sync --recursive
        git submodule update --init --recursive
    fi
    cd "$INSTALL_DIR"
    
    # Clone simapi
    if [ ! -d "simapi" ]; then
        log_info "Cloning simapi..."
        git clone https://github.com/Spacefreak18/simapi.git
    else
        log_info "simapi already cloned, updating..."
        cd simapi
        git pull
    fi
    cd "$INSTALL_DIR"
    
    # Clone simshmbridge
    if [ ! -d "simshmbridge" ]; then
        log_info "Cloning simshmbridge..."
        git clone https://github.com/spacefreak18/simshmbridge.git
        cd simshmbridge
        git submodule sync --recursive
        git submodule update --init --recursive
    else
        log_info "simshmbridge already cloned, updating..."
        cd simshmbridge
        git pull
        git submodule sync --recursive
        git submodule update --init --recursive
    fi
    
    log_success "Repositories cloned/updated"
}

# Build simapi
build_simapi() {
    log_info "Building simapi..."
    cd "$INSTALL_DIR/simapi"
    
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    
    log_info "Installing simapi library..."
    sudo make install
    
    # Update library cache
    sudo ldconfig 2>/dev/null || true
    
    log_success "simapi built and installed"
}

# Build simd
build_simd() {
    log_info "Building simd..."
    cd "$INSTALL_DIR/simapi/simd"
    
    rm -rf build
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    
    log_success "simd built"
}

# Build simshmbridge
build_simshmbridge() {
    log_info "Building simshmbridge..."
    cd "$INSTALL_DIR/simshmbridge"
    
    make clean || true
    make -j$(nproc)
    
    log_success "simshmbridge built"
}

build_simshmbridge_only() {
    mkdir -p "$INSTALL_DIR"
    cd "$INSTALL_DIR"
    
    if [ ! -d "simshmbridge" ]; then
        log_info "Cloning simshmbridge..."
        git clone https://github.com/spacefreak18/simshmbridge.git
        cd simshmbridge
        git submodule sync --recursive
        git submodule update --init --recursive
    fi
    
    build_simshmbridge
}

# Build monocoque
build_monocoque() {
    log_info "Building monocoque..."
    cd "$INSTALL_DIR/monocoque"
    
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    
    log_success "monocoque built"
}

# Setup configuration files
setup_configs() {
    log_info "Setting up configuration files..."
    
    # simd config
    mkdir -p "$CONFIG_DIR/simd"
    if [ ! -f "$CONFIG_DIR/simd/simd.config" ]; then
        if [ -f "$INSTALL_DIR/simapi/simd/conf/simd.config" ]; then
            cp "$INSTALL_DIR/simapi/simd/conf/simd.config" "$CONFIG_DIR/simd/simd.config"
            log_success "Created simd config"
        fi
    else
        log_info "simd config already exists, skipping"
    fi
    
    # monocoque config
    mkdir -p "$CONFIG_DIR/monocoque"
    if [ ! -f "$CONFIG_DIR/monocoque/monocoque.config" ]; then
        cat > "$CONFIG_DIR/monocoque/monocoque.config" << 'EOF'
configs = (
    {
        sim = "default";
        car = "default";
        devices = (
        // Add your devices here
        // Example: Serial device (ESP32/Arduino)
        /*
        {
            device       = "Serial";
            type         = "Custom";
            config       = "None";
            baud         = 115200;
            devpath      = "/dev/ttyUSB0";
        },
        */
        
        // Example: Bass shaker
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
        log_success "Created monocoque config"
    else
        log_info "monocoque config already exists, skipping"
    fi
}

# Create launcher scripts
create_launcher_scripts() {
    log_info "Creating launcher scripts in $BIN_DIR..."
    mkdir -p "$BIN_DIR"
    
    # Find simd binary location
    local SIMD_BIN=""
    if [ -f "$INSTALL_DIR/simapi/simd/build/simd" ]; then
        SIMD_BIN="$INSTALL_DIR/simapi/simd/build/simd"
    elif command -v simd &> /dev/null; then
        SIMD_BIN=$(which simd)
    fi
    
    # Find monocoque binary location
    local MONOCOQUE_BIN=""
    if [ -f "$INSTALL_DIR/monocoque/build/monocoque" ]; then
        MONOCOQUE_BIN="$INSTALL_DIR/monocoque/build/monocoque"
    elif command -v monocoque &> /dev/null; then
        MONOCOQUE_BIN=$(which monocoque)
    fi
    
    # start-simd script
    cat > "$BIN_DIR/start-simd" << EOF
#!/bin/bash
export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64
exec $SIMD_BIN -vv "\$@"
EOF
    chmod +x "$BIN_DIR/start-simd"
    
    # start-monocoque script
    cat > "$BIN_DIR/start-monocoque" << EOF
#!/bin/bash
exec $MONOCOQUE_BIN play "\$@"
EOF
    chmod +x "$BIN_DIR/start-monocoque"
    
    # test-monocoque script
    cat > "$BIN_DIR/test-monocoque" << EOF
#!/bin/bash
exec $MONOCOQUE_BIN test -vv "\$@"
EOF
    chmod +x "$BIN_DIR/test-monocoque"
    
    # monocoque-manager TUI
    log_info "Looking for monocoque-manager to install..."
    local MANAGER_INSTALLED=false
    
    # Use the SCRIPT_DIR exported from main()
    # Try multiple locations
    local SEARCH_PATHS=(
        "$SCRIPT_DIR/monocoque-manager"
        "$SCRIPT_DIR/tools/monocoque-manager"
    )
    
    for MANAGER_PATH in "${SEARCH_PATHS[@]}"; do
        log_info "Checking: $MANAGER_PATH"
        if [ -f "$MANAGER_PATH" ]; then
            log_info "Found monocoque-manager at: $MANAGER_PATH"
            cp "$MANAGER_PATH" "$BIN_DIR/monocoque-manager"
            chmod +x "$BIN_DIR/monocoque-manager"
            log_success "✓ Installed monocoque-manager TUI to $BIN_DIR/monocoque-manager"
            MANAGER_INSTALLED=true
            break
        fi
    done
    
    if [ "$MANAGER_INSTALLED" = false ]; then
        log_error "monocoque-manager not found!"
        log_warn "Searched in: ${SEARCH_PATHS[*]}"
        log_warn "SCRIPT_DIR was: $SCRIPT_DIR"
        log_info "Please copy it manually: cp $SCRIPT_DIR/monocoque-manager ~/.local/bin/"
    fi
    
    # Check if BIN_DIR is in PATH
    if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
        log_warn "$BIN_DIR is not in your PATH"
        log_info "Add this to your shell config:"
        echo "    # For bash/zsh:"
        echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
        echo ""
        echo "    # For fish:"
        echo "    fish_add_path ~/.local/bin"
    fi
    
    log_success "Launcher scripts created"
}

# Setup systemd user services
setup_systemd_services() {
    local SYSTEMD_DIR="$HOME/.config/systemd/user"
    mkdir -p "$SYSTEMD_DIR"
    
    log_info "Creating systemd service files..."
    
    # Find binary locations
    local SIMD_BIN=""
    if [ -f "$INSTALL_DIR/simapi/simd/build/simd" ]; then
        SIMD_BIN="$INSTALL_DIR/simapi/simd/build/simd"
    elif command -v simd &> /dev/null; then
        SIMD_BIN=$(which simd)
    fi
    
    # simd service
    cat > "$SYSTEMD_DIR/simd.service" << EOF
[Unit]
Description=Sim Telemetry Daemon
Documentation=https://spacefreak18.github.io/simapi/
After=network.target

[Service]
Type=simple
Environment="LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64"
ExecStart=$SIMD_BIN
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
EOF
    
    log_success "systemd services created"
    log_info "To enable auto-start on boot:"
    echo "    systemctl --user enable simd.service"
    echo "    systemctl --user start simd.service"
}

# Print next steps
print_next_steps() {
    echo ""
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║                    Installation Complete!                        ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
    log_success "Monocoque and all components are installed!"
    echo ""
    echo "📁 Installation location: $INSTALL_DIR"
    echo "⚙️  Configuration: $CONFIG_DIR/simd and $CONFIG_DIR/monocoque"
    echo "🚀 Launcher scripts: $BIN_DIR"
    echo ""
    echo "Next steps:"
    echo ""
    echo "1️⃣  Configure your games (REQUIRED!)"
    echo "   See: https://github.com/Spacefreak18/monocoque/blob/master/HOW-TO-USE.md"
    echo ""
    echo "2️⃣  Start the services:"
    echo "   Terminal 1: start-simd"
    echo "   Terminal 2: start-monocoque"
    echo ""
    echo "   OR enable auto-start:"
    echo "   systemctl --user enable --now simd.service"
    echo ""
    echo "3️⃣  Configure your devices:"
    echo "   Edit: $CONFIG_DIR/monocoque/monocoque.config"
    echo ""
    echo "4️⃣  Test your setup:"
    echo "   test-monocoque"
    echo ""
    echo "🎮 Supported games:"
    echo "   • Assetto Corsa / ACC"
    echo "   • Automobilista 2"
    echo "   • Project Cars 2"
    echo "   • RFactor 2"
    echo "   • Euro/American Truck Simulator"
    echo ""
    echo "📚 Documentation: https://spacefreak18.github.io/simapi/"
    echo ""
}

# Main installation flow
main() {
    # Save the directory where this script is located
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    export SCRIPT_DIR
    
    print_header
    
    check_root
    detect_distro
    check_requirements
    
    log_info "Starting installation..."
    echo ""
    
    install_dependencies
    clone_repositories
    
    log_info "Building components (this may take a few minutes)..."
    build_simapi
    build_simd
    build_monocoque
    build_simshmbridge
    
    setup_configs
    create_launcher_scripts
    setup_systemd_services
    
    print_next_steps
}

# Run main function
main "$@"

