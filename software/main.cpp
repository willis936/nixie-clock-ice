
// pico-sdk
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "boards/pico_ice.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

// pico-ice-sdk
#include "ice_usb.h"
#include "ice_cram.h"
#include "ice_fpga.h"
#include "ice_led.h"
#include "ice_spi.h"

uint8_t bitstream[] = {
#include "bitstream.h"
};

// pin definitions

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define I2C i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define UART_GPS_RSTN_PIN 19
#define UART_GPS_TX_PIN 20
#define UART_GPS_RX_PIN 21
#define HV_CONTROL_PIN 17
#define HV_MEASURE_PIN 28
#define BRIGHTNESS_CONTROL_PIN 29
#define INT_IO_EXPAND_PIN 16
#define RESET_IO_EXPAND_PIN 22

int main(void) {
    // Enable USB-CDC #0 (serial console)
    stdio_init_all();

    // configure pins

    // RP2040 I2C
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // GPS UART
    uart_init(uart1, 115200);
    gpio_set_function(UART_GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_GPS_RX_PIN, GPIO_FUNC_UART);

    // ADCs
    adc_init();
    adc_gpio_init(HV_MEASURE_PIN);
    adc_gpio_init(BRIGHTNESS_CONTROL_PIN);

    // PWM output
    gpio_set_function(HV_CONTROL_PIN, GPIO_FUNC_PWM);

    // General Purpose Input/Output
    gpio_init(UART_GPS_RSTN_PIN);
    gpio_set_dir(UART_GPS_RSTN_PIN, GPIO_OUT);
    gpio_init(INT_IO_EXPAND_PIN);
    gpio_set_dir(INT_IO_EXPAND_PIN, GPIO_IN);
    gpio_init(RESET_IO_EXPAND_PIN);
    gpio_set_dir(RESET_IO_EXPAND_PIN, GPIO_OUT);

    // Enable the physical UART
    uart_init(uart0, 115200);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Let the FPGA start
    ice_fpga_init(48);
	ice_led_init();
    ice_fpga_start();

    // Write the whole bitstream to the FPGA CRAM
    ice_cram_open();
    ice_cram_write(bitstream, sizeof(bitstream));
    ice_cram_close();

    // Configure USB as defined in tusb_config.h
    ice_usb_init();


    while (true) {
        
    }
    return 0;
}
