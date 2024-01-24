/*
 * MIT License
 *
 * Copyright (c) 2023 tinyVision.ai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#define DOC_FORWARD_SPI \
"https://pico-ice.tinyvision.ai/group__ice__usb.html#autotoc_md2"

#define DOC_DEFAULT_FIRMWARE \
"https://github.com/tinyvision-ai-inc/pico-ice/tree/main/Firmware/pico-ice-default"

// for repl_ungetchar() to take back the last character from repl_getchar():
int repl_last_char;
bool repl_last_held;

static int repl_getchar(void)
{
    int c;

    if (repl_last_held) {
        repl_last_held = false;
        return repl_last_char;
    }

    // busy-wait with a slow delay: this is for interactive I/O, no need to be fast
    while ((c = getchar_timeout_us(10000)) == PICO_ERROR_TIMEOUT) {
        // call tud_task() since we are blocking
        tud_task();
    }

    if (c == '\r' || c == '\n') {
        printf("\r\n");
    } else {
        putchar(c);
    }
    return c;
}

static void repl_ungetchar(int c)
{
    assert(!repl_last_held);
    repl_last_char = c;
    repl_last_held = true;
}

static inline bool repl_parse_error(char *msg, char c)
{
    // reset whatever was being input
    repl_last_held = false;

    printf("\nerror: expected %s got '%c'\n", msg, c);
    return false;
}

static bool repl_parse_newline(void)
{
    int c;

    switch (c = repl_getchar()) {
    case '\r':
    case '\n':
    case EOF:
        return true;
    default:
        repl_ungetchar(c);
        return repl_parse_error("newline", c);
    }
}

static void repl_command_version(void)
{
    if (!repl_parse_newline()) {
        return;
    }
    printf("pico-ice-sdk %s\r\n", VERSION);
}

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
        tud_task();

        printf("\x1b[1mpico-ice>\x1b[m ");

        switch (repl_getchar()) {
        case 'v':
            repl_command_version();
            break;
        default:
            printf("\r\n");
            printf("pico-ice default firmware\r\n", VERSION);
            printf("   %s\r\n", DOC_DEFAULT_FIRMWARE);
            printf("\r\n");
            printf("Serial port #0 - this shell, with commands:\r\n");
            printf("  v - print pico-ice-sdk version\r\n");
            printf("\r\n");
            printf("Serial port #1 - forwarding to UART\r\n");
            printf("  UART TX on RP0 = ICE27\r\n");
            printf("  UART RX on RP1 = ICE25\r\n");
            printf("\r\n");
            printf("Serial port #2 - forwarding to SPI:\r\n");
            printf("  %s\r\n", DOC_FORWARD_SPI);
            printf("\r\n");
            break;
        }

        printf("hello world\r\n");
    }
    return 0;
}