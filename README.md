# wolf_gba

This project tries to implement raycasting algorithms used by games such as Wolfenstein 3D on the Game Boy Advance.
A lot of tricks are involved since the GBA architecture is not intended to run 3D games.

## Build
You will need to have [devkit-arm toolchain](https://devkitpro.org/wiki/Getting_Started) installed.
You should then be able to run `make` at the root of the project, and then run the resulting `.gba` with any GBA emulator.
