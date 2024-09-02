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
- Choice of Portaudio or Pulseaudio backend.

## Suported Games
  - Using [SimSHMBridge](https://github.com/spacefreak18/simshmbridge)
    - Asseto Corsa
    - Assetto Corsa Competizione
    - Project Cars 2
    - Automobilista 2
  
  - Using [scs-sdk-plugin](https://github.com/jackz314/scs-sdk-plugin/releases)
    - Euro Truck Simuator 2
    - American Truck Simulator

## Dependencies
- libserialport - arduino serial devices
- hidapi - usb hid devices
- libusb - used by hidapi
- portaudio - sound devices (haptic bass shakers)
- pulseaudio - sound devices (haptic bass shakers)
- libenet - UDP support (not yet implemented)
- libxml2
- argtable2
- libconfig
- [slog](https://github.com/kala13x/slog) (static)
- [simshmbridge](https://github.com/spacefreak18/simshmbridge) - for sims that need shared memory mapping like AC and Project Cars related.
- [simapi](https://github.com/spacefreak18/simapi)

## Building
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

to use the pulseaudio backend use this cmake command
```
cmake -DUSE_PULSEAUDIO=YES ..
```
## Bass Shaker Sound Devices

When using pulseaudio it is necesarry to provide a devid in the configuration. You can find this with:

```
pacmd list-sinks
pacmd list-sinks | grep name:
```
analyze the output to determine the appropriate hardware to which you would like to output the effects.

## Using Arduino Devices

Currently Monocoque supports simwind and shiftlights through the included arduino sketches which have been tested on Uno boards. The simwind controller requires a Motor shield.

There are included Makefiles for each controller. For now, the makefiles expect the device to be attached at /dev/ttyACM0. So unplug both controllers, and then plug in just the
controller you're updating to ensure the correct controller is at /dev/ttyACM0.

To compile and upload these sketches, the Makefiles use arduino-cli. Unfortunately it seems some distributions such as debian do not include this in the repositories. If this is
the case follow the install instructions here:
```
https://arduino.github.io/arduino-cli/0.35/installation/
```
You may have to download the core libraries, it will prompt you to do so if you do not have them and you go further
```
arduino-cli core install arduino:avr
```
Then for shiftlights navigate to included shiftlight directory ( be sure only the shiftlight controller is plugged into the machine and is available at /dev/ttyACM0 ) and

```
arduino-cli lib install FastLED
make
```
Then for simwind navigate to the included simwind directory ( be sure only the simwind controller is plugged into the machine and is available at /dev/ttyACM0 ) and
```
ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library.git
ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BusIO.git
make
```

### Uploading sketch to Arduino Uno
```
cd ../src/arduino/{simwind/shiftlights/simhaptic} # Depending on the controller you have.
make  # Make sure serial connection is the same as on the host pc and it have the right group permission to access the device without root
```

### SimHaptic Ardunio Motor Connection
- Acc Pedal Motor to be connected to M1 
- Brake Pedal Motor to be connected to M3
- you can actually connect the motors how you wish, but you'd have to make the appropriate changes to the config and to the arduino sketch.

## Getting car tyre diameter for rumbles to work
- User needs to start monocoque and drive the new car straight at 70 kmph and then stop steering, acc or brake input 

## Testing
```
./monocoque test -vv # Make sure that ~/.config/monocque/monocoque.config only contains the devices you have connected.
```

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

### Logs file location
`~/.cache/monocoque/*.log`

## Join the Discussion
[Sim Racing Matrix Space](https://matrix.to/#/#simracing:matrix.org)

## ToDo
 - windows port
 - more memory testing
 - move config code around
 - cleanup tests which are basically just copies of the example from their respective projects
 - much, much more
