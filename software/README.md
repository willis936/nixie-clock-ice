# nixie_pico_ice

## Build Instructions

Build in Windows by running the powershell script `build.ps1`.

WSL will need to be installed.  WSL will need CMake and APIO installed (see repository base README).

Output will be a UF2 file in the `build` directory.  The build script attempts to copy this to the `D:` drive after compilation.

## Features

Host a UF2 interface over USB (broken right now).

Synthesize gateware then have RP2040 image write the bitstream to FPGA configuration NVM.

Initialize pinout to validate hardware pinout configuration.
