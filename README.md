# Monocoque
```
___   |/  /____________________________________ ____  ______ 
__  /|_/ /_  __ \_  __ \  __ \  ___/  __ \  __ `/  / / /  _ \
_  /  / / / /_/ /  / / / /_/ / /__ / /_/ / /_/ // /_/ //  __/
/_/  /_/  \____//_/ /_/\____/\___/ \____/\__, / \__,_/ \___/ 
                                           /_/
```
Cross Platform device manager for driving and flight simulators, for use with common simulator software titles.

## Features
- Updates at 120 frames per seconds.
- Modular design for support with various titles and devices.
- Supports bass shakers, tachometers, simlights, simwind etc, through usb and arduino serial.
- Tachometer support is currently  limited to the Revburner model. Supports existing revburner xml configuration files.
- Includes utility to configure revburner tachometer
- Can send data to any serial device. So far only tested with arduino. Includes sample arduino sketch for sim lights.
- The support for haptic bass shakers is limited and needs the most work. So far the engine rev is a simple sine wave, which I find convincing. The gear shift event works but not convincing enough for me.

## Dependencies
- libserialport - arduino serial devices
- hidapi - usb hid devices
- libusb - used by hidapi
- portaudio - sound devices (haptic bass shakers)
- libenet - UDP support (not yet implemented)
- libxml2
- argtable2
- libconfig
- [slog](https://github.com/kala13x/slog) (static)
- [wine-linux-shm-adapter](https://github.com/spacefreak18/simshmbridge) - for sims that need shared memory mapping like AC.
- [simapi](https://github.com/spacefreak18/simapi)

## Building
This code depends on the shared memory data headers in the simapi [repo](https://github.com/spacefreak18/simapi). When pulling lastest if the submodule does not download run:
```
git submodule sync --recursive
git submodule update --init --recursive
```

Then to compile simply:
```
cd build
cmake ..
make
```

## Testing

### Setting up Your Arduino Device

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

## ToDo
 - windows port
 - more memory testing
 - move config code around
 - cleanup tests which are basically just copies of the example from their respective projects
 - much, much more
