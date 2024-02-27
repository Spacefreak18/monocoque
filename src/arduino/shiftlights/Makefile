# Makefile for Arduino based scketches
#
# Copyright 2020 Valerio Di Giampietro http://va.ler.io v@ler.io
# MIT License - see License.txt file
#
# This Makefile uses the arduino-cli, the Arduino command line interface
# and has been designed and tested to run on Linux, not on Windows.
# Probably it will run on a Mac, but it has not been tested.
#
# Please note that:
#
#   1. each sketch must reside in his own folder with this Makefile
#
#   2. the main make targets are:
#      - all     compiles and upload
#      - compile compiles only
#      - upload  upload via serial port, compile if the binary file is
#                not available
#      - ota     upload Over The Air, automatically find the device
#                IP address using the IOT_NAME (device hostname)
#      - clean   clean the build directory
#      - find    find OTA updatable devices on the local subnet
#      - requirements it the file "requirements.txt" exists it will
#                     install the libraries listed in this file
#
#      default is "all"
#
#   3. it gets the name of the sketch using the wildcard make command;
#      the name is *.ino; this means that you must have ONLY a file
#      with .ino extension, otherwise this makefile will break.  This
#      also means that you can use this Makefile, almost unmodified,
#      for any sketch as long as you keep a single .ino file in each
#      folder
#
#   4. you can split your project in multiple files, if you wish,
#      using a single .ino file and multiple .h files, that you can
#      include in the .ino file with an '#include "myfile.h"'
#      directive
#
# Optionally some environment variables can be set:
#
#   FQBN        Fully Qualified Board Name; if not set in the environment
#               it will be assigned a value in this makefile
#
#   SERIAL_DEV  Serial device to upload the sketch; if not set in the
#               environment it will be assigned:
#               /dev/ttyUSB0   if it exists, or
#               /dev/ttyACM0   if it exists, or
#               unknown
#
#   IOT_NAME    Name of the IOT device; if not set in the environment
#               it will be assigned a value in this makefile. This is
#               very useful for OTA update, the device will be searched
#               on the local subnet using this name
#
#   OTA_PORT    Port used by OTA update; if not set in the environment
#               it will be assigned the default value of 8266 in this
#               makefile
#
#   OTA_PASS    Password used for OTA update; if not set in the environment
#               it will be assigned the default value of an empty string
#
#   V           verbose flag; can be 0 (quiet) or 1 (verbose); if not set
#               in the environment it will be assigned a default value
#               in this makefile


MAKE_DIR   := $(PWD)
#
# ----- setup wor Wemos D1 mini -----
#FQBN       ?= esp8266:esp8266:d1_mini
#IOT_NAME   ?= esp8266-meteo
#OTA_PORT   ?= 8266
#OTA_PASS   ?=
# ----- setup for Arduino Uno
FQBN        ?= arduino:avr:uno
# ----- ---------------------
V          ?= 0
VFLAG      =

ifeq "$(V)" "1"
VFLAG      =-v
endif

ifndef SERIAL_DEV
  ifneq (,$(wildcard /dev/ttyUSB0))
    SERIAL_DEV = /dev/ttyUSB0
  else ifneq (,$(wildcard /dev/ttyACM0))
    SERIAL_DEV = /dev/ttyACM0
  else
    SERIAL_DEV = unknown
  endif
endif

BUILD_DIR  := $(subst :,.,build/$(FQBN))

SRC        := $(wildcard *.ino)
HDRS       := $(wildcard *.h)
BIN        := $(BUILD_DIR)/$(SRC).bin
ELF        := $(BUILD_DIR)/$(SRC).elf

$(info FQBN       is [${FQBN}])
$(info IOT_NAME   is [${IOT_NAME}])
$(info OTA_PORT   is [${OTA_PORT}])
$(info OTA_PASS   is [${OTA_PASS}])
$(info V          is [${V}])
$(info VFLAG      is [${VFLAG}])
$(info MAKE_DIR   is [${MAKE_DIR}])
$(info BUILD_DIR  is [${BUILD_DIR}])
$(info SRC        is [${SRC}])
$(info HDRS       is [${HDRS}])
$(info BIN        is [${BIN}])
$(info SERIAL_DEV is [${SERIAL_DEV}])

all: $(ELF) upload
.PHONY: all

compile: $(ELF)
.PHONY: compile

$(ELF): $(SRC) $(HDRS)
	arduino-cli compile -b $(FQBN) $(VFLAG)
	@if which arduino-manifest.pl; \
	then echo "---> Generating manifest.txt"; \
	arduino-manifest.pl -b $(FQBN) $(SRC) $(HDRS) > manifest.txt; \
	else echo "---> If you want to generate manifest.txt, listing used libraries and their versions,"; \
	echo "---> please install arduino-manifest, see https://github.com/digiampietro/arduino-manifest"; \
	fi

upload:
	@if [ ! -c $(SERIAL_DEV) ] ; \
	then echo "---> ERROR: Serial Device not available, please set the SERIAL_DEV environment variable" ; \
	else echo "---> Uploading sketch\n"; \
	arduino-cli upload -b $(FQBN) -p $(SERIAL_DEV) $(VFLAG); \
	fi

ota:
	@PLAT_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.platform.path' | awk -F= '{print $$2}'` ; \
	   PY_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.tools.python3.path' | awk -F= '{print $$2}'` ; \
	IOT_IP=`avahi-browse _arduino._tcp --resolve --parsable --terminate|grep -i ';$(IOT_NAME);'|grep ';$(OTA_PORT);'| awk -F\; '{print $$8}'|head -1`; \
	BINFILE=$(wildcard $(BUILD_DIR)/$(SRC)*bin); \
	echo "PLAT_PATH  is [$$PLAT_PATH]" ; \
	echo "PY_PATH:   is [$$PY_PATH]"  ; \
	echo "IOT_IP:    is [$$IOT_IP]"   ; \
	echo "BINFILE:   is [$$BINFILE]"  ; \
	if [ "$$IOT_IP" = "" ] ; \
	then echo "Unable to find device IP. Check that the IOT_NAME environment variable is correctly set. Use 'make find' to search devices"; \
	else echo "---> Uploading Over The Air"; \
	$$PY_PATH/python3 $$PLAT_PATH/tools/espota.py -i $$IOT_IP -p $(OTA_PORT) --auth=$(OTA_PASS) -f $$BINFILE ;\
	fi

clean:
	@echo "---> Cleaning the build directory"
	rm -rf build

find:
	avahi-browse _arduino._tcp --resolve --parsable --terminate

requirements:
	@if [ -e requirements.txt ]; \
	then while read -r i ; do echo ; \
	  echo "---> Installing " '"'$$i'"' ; \
	  arduino-cli lib install "$$i" ; \
	done < requirements.txt ; \
	else echo "---> MISSING requirements.txt file"; \
	fi
