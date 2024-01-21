
# make new build
apio build

# set WSL directory for the host build
# Use ln /mnt/c/... ~/... to make this more elegant
$WSLdir = '~/nixieIce/software/UP5K/pico-ice-blinky'
wsl bin2uf2 -o $WSLdir/NEW.UF2 $WSLdir/hardware.bin

# copy output to pico iCE
copy NEW.UF2 D:\NEW.UF2
