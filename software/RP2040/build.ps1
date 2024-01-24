
# set WSL directory for the host build
# Use ln /mnt/c/... ~/... to make this more elegant
$WSLdir  = '~/nixieIce/software/RP2040/build'

# clean FPGA build directory
Remove-Item -Path UP5K\*.dblite
Remove-Item -Path UP5K\*.asc
Remove-Item -Path UP5K\*.bin
Remove-Item -Path UP5K\*.uf2
Remove-Item -Path UP5K\*.json

# clean MCU build directory
Remove-Item -Recurse -Path build\*

# build MCU image
wsl -e bash -lic "cd $WSLdir && cmake .. && make"

# copy output to pico iCE
copy build\pico_usb_uart.uf2 D:\NEW.UF2
