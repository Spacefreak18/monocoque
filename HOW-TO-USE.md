# Monocoque User Setup Guide

Game/bridge details live in the [simapi docs](https://spacefreak18.github.io/simapi/simd_usage). This page is the short path after the binaries exist.

## Install the stack

Prefer the method for your distro in the [README](README.md#quick-install). A packaged or `./install.sh` install should give you:

* `start-simd`, `start-monocoque`, `test-monocoque` in `~/.local/bin`
* configs in `~/.config/simd/` and `~/.config/monocoque/`
* optional user unit `~/.config/systemd/user/simd.service`

To compile by hand instead:

* build [monocoque](https://github.com/Spacefreak18/monocoque) — `git submodule update --init --recursive`, then `cmake` / `make`
* build [simd](https://github.com/Spacefreak18/simapi/tree/master/simd) (needs simapi installed first, including `simdata.h`)
* get [simshmbridge](https://github.com/spacefreak18/simshmbridge) compatibility EXEs ([releases](https://github.com/spacefreak18/simshmbridge/releases)) unless you only use UDP titles

## Configure SIMD & Monocoque

* `~/.config/simd/simd.config` — [example](https://github.com/Spacefreak18/simapi/blob/master/simd/conf/simd.config) (usually fine as-is)
* `~/.config/monocoque/monocoque.config` — start from the installer stub or [conf/monocoque.config](https://github.com/Spacefreak18/monocoque/blob/master/conf/monocoque.config)
    * Keep only devices you have plugged in
    * [Bass shaker config](https://spacefreak18.github.io/simapi/shakers)
    * Test with `test-monocoque` or `./monocoque test -vv`

Serial/HID devices often need your user in `input`, `dialout`, and/or `uucp`, plus the udev rules from `udev/69-monocoque.rules`.

## Steam & Game Config

### Steam

Shared-memory titles (Assetto Corsa, ACC, AMS2, PCars2, …) need a simshmbridge EXE in the **same Proton prefix** as the game. Set a launch option such as:

```bash
SIMD_BRIDGE_EXE=/home/YOU/.local/share/monocoque/simshmbridge/assets/acbridge.exe %command%
```

Exact EXE names and more examples: [simd usage](https://spacefreak18.github.io/simapi/simd_usage) and [simshmbridge](https://github.com/spacefreak18/simshmbridge?tab=readme-ov-file#basic-mapping-examples).

### Game specific settings

#### Automobilista 2 (AMS2)

Activate Shared Memory and set the protocol to Project CARS 2, then restart the game.

![System Settings in AMS2](https://static.wixstatic.com/media/910f3b_adabfa94a57944cca33e488972534fdd~mv2.png/v1/fill/w_964,h_374,al_c,q_90,usm_0.66_1.00_0.01,enc_avif,quality_auto/game_setup_ams2_1.png)
<img src="https://docs.simucube.com/Tuner/games/assets/automobilista2_telemetry_2.png" alt="Shared Memory Settings in AMS2" width="65%">

#### Assetto Corsa & Assetto Corsa Competizione (ACC)

No extra in-game telemetry toggle. You still need the AC/ACC bridge EXE in the Steam launch command.

## Run

Launchers after a packaged or `./install.sh` install: `start-simd`, `start-monocoque`, `test-monocoque`, or `monocoque-manager`. Start the game from Steam as usual.

Shared-memory titles still need the bridge EXE in the Steam launch command (`SIMD_BRIDGE_EXE=... %command%`). The installer can also enable `simd.service` so mapping is ready at login. If the simd binary was installed under `/usr/local`, the launcher already sets `LD_LIBRARY_PATH`.

## Troubleshooting

* simd should log that it found the sim after you are in session
* `hexdump /dev/shm/SIMAPI.DAT | head` should not be all zeros once mapping works
* AC/ACC: `hexdump /dev/shm/acpmf_physics | head` — non-zero while on track
* AMS2: look for `/dev/shm/$pcars2$`
* confirm the bridge EXE is actually running (`ps aux | grep -i bridge`)
* if simd sees the game but monocoque shows no RPM/gear, re-check [game settings](#steam--game-config)
* `monocoque-manager` can start/stop the two processes if `~/.local/bin` is on `PATH`
