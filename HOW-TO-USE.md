# Monocoque User Setup Guide

## Required Packages / Software
* build [monocoque](https://github.com/Spacefreak18/monocoque) e.g. in `~/monocoque` - follow instructions from repo
* build [simd](https://github.com/Spacefreak18/simapi/tree/master/simd) e.g. in `~/simapi/simd` - follow instructions from repo
* build [simshmbridge](https://github.com/spacefreak18/simshmbridge) e.g. in `~/simshmbridge` - follow instructions from repo

## Configure SIMD & Monocoque
* Create a `simd` Config in `~/.config/simd/simd.config` - [use example](https://github.com/Spacefreak18/simapi/blob/master/simd/conf/simd.config)
    * no changes required to the example file
* create Monocoque config in `~/.config/monocque/monocoque.config` - [example](https://github.com/Spacefreak18/monocoque/blob/master/conf/monocoque.config)
    * Adapt the config to your needs, remove unused entries
    * [Documentation for Bass Shaker Config](https://spacefreak18.github.io/simapi/shakers)

## Steam & Game Config
### Steam
* Adapt Steam Launch Commands from [simshbridge](https://github.com/spacefreak18/simshmbridge?tab=readme-ov-file#basic-mapping-examples)
    * to use with `simd` you need to `SIMD_BRIDGE_EXE` to the launch command like `SIMD_BRIDGE_EXE=~/git/simshmbridge/assets/pcars2bridge.exe %command% & sleep 5 && ~/.steam/steam/steamapps/common/Proton\ 6.3/proton run ~/git/simshmbridge/assets/pcars2bridge.exe`

### Game specific settings
#### Automobilista 2 (AMS2)
* In Games like `Automobilista 2` activate the `Shared Memory` Setting and make sure to set the Protocal to `Project CARS 2`. ![System Settings in AMS2](https://static.wixstatic.com/media/910f3b_adabfa94a57944cca33e488972534fdd~mv2.png/v1/fill/w_964,h_374,al_c,q_90,usm_0.66_1.00_0.01,enc_avif,quality_auto/game_setup_ams2_1.png)  <img src="https://docs.simucube.com/Tuner/games/assets/automobilista2_telemetry_2.png" alt="Shared Memory Settings in AMS2" width="65%">
    * Restart the Game after changing these Settings!
#### Assetto Corsa & Assetto Corsa Competizione (ACC)
* Assetto Corsa & ACC do not need any additional settings and should work out of the box

## Run everything, but in the right order
* first start `simd` like `~/simapi/simd/build/simd --no-daemon -vv`
    * this will likely fail, so you'll need to add `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib` or `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64` in front of the command like `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64 ~/simapi/simd/build/simd --no-daemon -vv`
* now start `monocoque` like `~/monocoque/build/monocoque play`
* finally start the Game you want to play from Steam

## Troubleshooting

* make sure `simd` is running and after you start a game, the CLI output confirms the game is detected
* make sure the bridge application starts e.g. with `ps aux|grep pcars2bridge.exe` before the game starts
* see if you have the path `/dev/shm/acpmf_physics` (for ACC) and/or `/dev/shm/$pcars2$` for AMS2
* when `simd` recognizes the game, see if `monocoque` shows `RPM, Gear..` and other Car Metrics once you start a game
    * if `simd` recognozies the game, but `monocoque` does not show any data, doublecheck the [Game Settings](#steam--game-config) to see if `Shared Memory` is activated