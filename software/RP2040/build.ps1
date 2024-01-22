
# set WSL directory for the host build
# Use ln /mnt/c/... ~/... to make this more elegant
$WSLdir = '~/nixieIce/software/RP2040/build'

# build
wsl cd $WSLdir `&`& cmake .. `&`& make

# copy output to pico iCE
#copy build\pico_usb_uart.uf2 D:\NEW.UF2
