# nixie-clock-ice

 Hardware and software source for a GPS nixie tube clock with delay correction.

Uses a PICO-iCE v3 FPGA + MCU module.

 - Raspberry Pi RP2040 processor
 - Lattice UltraPlus ICE40UP5K FPGA with 5.3k LUTs, 1Mb SPRAM, 120Kb DPRAM, 8 Multipliers

# Building

## MCU

Follow the pico ice Getting Started section.

Using Ubuntu WSL the apt version of cmake is currently not high enough.

Install an appropriate version like this:

https://www.linuxcapable.com/how-to-install-cmake-on-ubuntu-linux/

```
sudo apt update && sudo apt install build-essential checkinstall zlib1g-dev libssl-dev g++ gdb make ninja-build rsync zip gcc-arm-none-eabi

cd /usr/local/src

sudo wget https://github.com/Kitware/CMake/archive/refs/tags/v3.28.1.tar.gz

sudo tar -zxvf v3.28.1.tar.gz

sudo rm v3.28.1.tar.gz

cd CMake-3.28.1

sudo ./bootstrap

sudo make

sudo make install
```

## FPGA

Install python 3.5+.

Install apio and oss CAD suite.

`pip install -U apio`

`apio install -a`

Test project using the APIO guide here:

https://pico-ice.tinyvision.ai/md_programming_the_fpga.html

Use UF2 on WSL to convert the bin to UF2 then copy over CURRENT.UF2 on mounted USB drive pico ice.

https://github.com/tinyvision-ai-inc/uf2-utils/

# Requirements

1. Display the time with 1 microsecond accuracy.
2. Display the time with 1 second precision.
3. Use nixie tubes to display the time.
4. Do not require or allow the user to manually set the time.
5. Display the time within 30 seconds of power on.
6. Use a single power entry.
7. Use a single radio receiver to establish local timebase.
8. Have a user-selectable mode for Daylight Savings Time (DST) checking.
9. Use longitude to estimate DST date range.
10. Have a user-selectable mode to force DST offset on (off when both this and DST check mode are off).
11. Use longitude to estimate timezone.
12. Have a user-selectable timezone offset.
13. Be able to use software to control nixie tube brightness.
14. Use an ambient light detector to automatically adjust nixie tube brightness.
15. Have a user-selectable manual brightness mode.
16. Use a potentiometer to set the brightness in manual brightness mode.
17. Run a nixie tube anti-poisoning routine once a day (cycle through all digits at high brightness for a minute).
18. Have a user-selectable mode to enable daily anti-poisoning routine
19. Have a user-selectable mode to force anti-poisoning routine on indefinitely.
20. Have a user-selectable 12-hour / 24-hour display mode.
21. Use a nixie tube decimal point to indicate AM/PM in 12-hour display mode.
22. Use a nixie tube decimal point to indicate that the radio receiver is locked.
 
 
