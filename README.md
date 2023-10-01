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

## Configuration

`~/.config/monocoque/monocoque.config`

```
devices = (
{   device      = "Sound";
    type        = "Engine"
    devid       = "1022:15E2" },
{   device      = "Sound";
    type        = "Gear"
    devid       = "1022:15E2" }
);
```

make sure to substitute your audio device ID in the `devid` field. You can find your audio device ID with `lspci -nnk | grep -i audio`

see `conf/` directory for more example configuration and functionality options

### Rfactor2

credit to schelgp for porting the rF2SharedMemoryMapPlugin to Wine

https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine

you can use this download link:
https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine/raw/master/build/rFactor2SharedMemoryMapPlugin64_Wine.dll

save the file to `~.steam/steam/steamapps/common/rFactor 2/Bin64/Plugins/`

It's likely you'll have to `chmod +x` the .dll file also

Launch Rfactor2 and navigate to ‘Content & Settings’ -> ‘Settings’ at the bottom right of the screen there should be a ‘Plugins’ section. Enable the shared memorymap plugin. This will allow monocoque to access rfactor's in-game variables such as rpm, traction, etc.

## Testing

```
cd build
./monocoque test
```

cd build`

### Setting up Your Arduino Device
(TODO)

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
