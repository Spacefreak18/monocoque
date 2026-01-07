# Monocoque
```
___   |/  /____________________________________ ____  ______ 
__  /|_/ /_  __ \_  __ \  __ \  ___/  __ \  __ `/  / / /  _ \
_  /  / / / /_/ /  / / / /_/ / /__ / /_/ / /_/ // /_/ //  __/
/_/  /_/  \____//_/ /_/\____/\___/ \____/\__, / \__,_/ \___/ 
                                           /_/
```
Cross Platform device manager for driving and flight simulators, for use with common simulator software titles.

📚 **Usage Documentation:** [spacefreak18.github.io/simapi/](https://spacefreak18.github.io/simapi/)

## Features
- Updates at 60 frames per seconds.
- Modular design for support with various titles and devices.
- Supports bass shakers, tachometers, several wheels and pedals, simlights, simwind etc, through usb and arduino serial.
- Tachometer support is currently  limited to the Revburner model. Supports existing revburner xml configuration files.
- Includes utility to configure revburner tachometer
- Can send data to any serial device. So far only tested with arduino. and ESP32. Includes sample arduino sketch for sim lights, simwind, and simhaptic effects for motors.
- Support for custom arduino (or any serial) device, with a [custom lua format](https://spacefreak18.github.io/simapi/serial_custom) for sending data
- Convincing shaker effects for noise tranducers for wheel slip, wheel lock, and abs, as well as engine rpm and gear shifts.
- Support for many [wheels and pedals](https://spacefreak18.github.io/simapi/thirdpartydevices) including Clubsport Elite V3, [Logitech G29](https://spacefreak18.github.io/simapi/logitechg29), and Moza R5.

## Quick Install

**One-Line Installation:**
```bash
curl -fsSL https://raw.githubusercontent.com/Spacefreak18/monocoque/master/install.sh | bash
```
***AUR Package following git master:***
```https://aur.archlinux.org/packages/monocoque-git```

**Or download and review first:**
```bash
wget https://raw.githubusercontent.com/Spacefreak18/monocoque/master/install.sh
chmod +x install.sh
./install.sh
```

**TUI Manager:**
After installation, use the interactive manager:
```bash
monocoque-manager
```

**Supported Games**
[Supported Sims](https://spacefreak18.github.io/simapi/supportedsims)
***please note on Linux some titles will require a compatibility exe from simshmbridge to be setup. Please follow the linked Documentation
for installation instructions***


## Building

### Dependencies
- libserialport - arduino serial devices
- hidapi - usb hid devices
- portaudio - sound devices (haptic bass shakers)
- libpulse - sound devices (haptic bass shakers)
- libuv base event loop
- libxml2
- argtable2
- libconfig
- xdg-basedir
- lua
- [slog](https://github.com/kala13x/slog) (static)
- [simshmbridge](https://github.com/spacefreak18/simshmbridge) - for sims that need shared memory mapping like AC and Project Cars related.
- [simapi](https://github.com/spacefreak18/simapi)
```
pacman -Syu git cmake pulse-native-provider libxdg-basedir libserialport libconfig libuv argtable hidapi lua
```

This code depends on the shared memory data headers in the simapi [repo](https://github.com/spacefreak18/simapi). When pulling lastest if the submodule does not download run:
```
git submodule sync --recursive
git submodule update --init --recursive
```

Then to compile simply:
```
mkdir build; cd build
cmake ..
make
```

## User Setup Guide
See the dedicated [How To](HOW-TO-USE.md) for detailed instructions to set up and run 'monocoque`

## Testing
```
./monocoque test -vv # Make sure that ~/.config/monocque/monocoque.config only contains the devices you have connected.
```

### Logs file location
`~/.cache/monocoque/*.log`

### Static Analysis
```
    mkdir build; cd build
    make clean
    cmake -Danalyze=on ..
    make
```

### Valgrind
```
    cd build
    valgrind -v --leak-check=full --show-leak-kinds=all --suppressions=../.valgrindrc ./monocoque play
```

## Join the Discussion
[Sim Racing Matrix Space](https://matrix.to/#/#simracing:matrix.org)

## ToDo
 - add frequency cap (low pass filter) to sound haptic effects
 - add road and kerb sound haptic effects
 - windows port
 - more memory testing
 - cleanup tests which are basically just copies of the example from their respective projects
 - much, much more
