set(devices_source_files
  simdevice.h
  simdevice.c
  usbdevice.h
  usbdevice.c
  sounddevice.h
  sounddevice.c
  serialdevice.h
  serialdevice.c
  tachdevice.h
  tachdevice.c
  wheeldevice.h
  wheeldevice.c
  usbhapticdevice.h
  usbhapticdevice.c
  hapticeffect.h
  hapticeffect.c
  sound.h
  sound.c
  usb/revburner.h
  usb/revburner.c
  usb/cslelitev3.h
  usb/cslelitev3.c
  usb/wheels/cammusc5.h
  usb/wheels/cammusc5.c
  usb/wheels/cammusc12.h
  usb/wheels/cammusc12.c
  sound/usb_generic_shaker.h
  sound/usb_generic_shaker.c
  sound/usb_generic_shaker_pulse.c
  serial/arduino.h
  serial/arduino.c
)

include_directories("." "usb" "sound" "serial")

add_library(devices STATIC ${devices_source_files})
