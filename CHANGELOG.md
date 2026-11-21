# Changelog

## [0.6.0](https://github.com/Spacefreak18/monocoque/compare/0.5.0...0.6.0) (2026-11-21)


### Features

* Add add config button ([6c06a70](https://github.com/Spacefreak18/monocoque/commit/6c06a70902fa9a9159d8e2485ddc0d4b1fb536f9))


### Bug Fixes

* pass lua serial tests ([c4e07af](https://github.com/Spacefreak18/monocoque/commit/c4e07aff17604da1e745efcc2fe6c8e93b1a37c4))

## [0.5.0](https://github.com/Spacefreak18/monocoque/compare/0.4.0...0.5.0) (2026-11-20)


### Features

* Add auto_start setting ([a2484ba](https://github.com/Spacefreak18/monocoque/commit/a2484baa2d5aec878a26e6eed4ebeaf96054362a))
* Add force config setting. Cleanup UI slightly. ([806b38e](https://github.com/Spacefreak18/monocoque/commit/806b38e3c2f90504dabde8978c89dc781744cbad))
* add umbrella header ([a51db67](https://github.com/Spacefreak18/monocoque/commit/a51db67b2537ca43803abb5ce5a0986b5f3f8844))
* gamestatus in status bar ([f9a282c](https://github.com/Spacefreak18/monocoque/commit/f9a282cd90f213fc8ddca9ef26b6be969cc20710))
* harden the installer for real distros ([#57](https://github.com/Spacefreak18/monocoque/issues/57)) ([a44f73b](https://github.com/Spacefreak18/monocoque/commit/a44f73b2a3e2556be82f4a751aebcdc2d230328d))
* improve useage of config_force and configuration resolution. refactor: config resolution ([ef43146](https://github.com/Spacefreak18/monocoque/commit/ef431462200943ede022bfd0eb3ecd6037e5fac6))


### Bug Fixes

* Allow cli to open without a config file and create a config if none exists. But still error if config is invalid ([2512135](https://github.com/Spacefreak18/monocoque/commit/25121358e8b82e5f2c1d11649a903e6037f6e1eb))
* Allow gui to open without a config file and create a config if none exists. But still error if config is invalid ([47d0c50](https://github.com/Spacefreak18/monocoque/commit/47d0c50d6402b740905fab08f4ba1d3beb8ccd5e))
* do not prevent devices configured without subtype from loading ([2c832fd](https://github.com/Spacefreak18/monocoque/commit/2c832fddb8a1512be85618ad259d0381f19739c3))
* effects playing at faster than specified frequency and gear pulsing too short. Issue 62. ([3f673af](https://github.com/Spacefreak18/monocoque/commit/3f673af986cfce7e0ca1be00966d77909e221b32))
* fail closed on serial and PulseAudio init ([#56](https://github.com/Spacefreak18/monocoque/issues/56)) ([0c7cd6f](https://github.com/Spacefreak18/monocoque/commit/0c7cd6f751635e35e0cad2b3eff5e089b10ebb51))
* fix malfunctioning test mode ([8010b0d](https://github.com/Spacefreak18/monocoque/commit/8010b0d7f7f5861ad9ee85be6f2b07d05ebaf18e))
* game stats not showing in cli mode ([af05de0](https://github.com/Spacefreak18/monocoque/commit/af05de0e63b81131a0c09d3985943f42cfa00f04))
* remove compilation of file not necessary for project ([2e3ea9e](https://github.com/Spacefreak18/monocoque/commit/2e3ea9ee837e8a8e1f45d908e9721febd2742507))
* stray debug logging ([dc749a8](https://github.com/Spacefreak18/monocoque/commit/dc749a8f80ac29341d9dcd416290d47750314bc8))
* stray printf in middle of device logic ([fbcb2f4](https://github.com/Spacefreak18/monocoque/commit/fbcb2f4246d03eeb49a9851e9d8fee76536a2eb6))
* update to latest simapi for compilation ([c77ca7c](https://github.com/Spacefreak18/monocoque/commit/c77ca7c1b897c837ca7030116fd1b217e90a78f6))
* various issues with gui ([1750a4b](https://github.com/Spacefreak18/monocoque/commit/1750a4b413acf2d1403e579751de65a2f22456f7))

## [0.4.0](https://github.com/Spacefreak18/monocoque/compare/0.3.6...0.4.0) (2026-09-24)


### Features

* Add ability to specific a lua script for testing ([5c32478](https://github.com/Spacefreak18/monocoque/commit/5c32478e081261f2c2515342c4ef15fcc311a877))


### Bug Fixes

* three crashes found while packaging ([#54](https://github.com/Spacefreak18/monocoque/issues/54)) ([8ef05ef](https://github.com/Spacefreak18/monocoque/commit/8ef05ef329c1c78bbdf2d9d42a2560b56a795acb))
