
// pico-sdk
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/multicore.h"
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

    // keep track of loop iterations
    uint32_t iLoopCounter = 0;
    // ADC temporary input
    uint16_t ADC_val;
    // ADC inputs
    uint16_t HV_ADC_counts;
    float HV_measure;
    uint16_t brightness_ADC_counts;
    float brightness_control;
    
    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float ADC_Vref = 3.3f;
    const float ADC_conversion_factor = ADC_Vref / ((1 << 12) - 1);
    // 1/101 voltage divider
    const float HV_conversion_factor = 101;
    // 0 - 100 %
    const float brightness_conversion_factor = 1 / ADC_Vref;
    
    while (true) {
        tud_task();
        
        // Select ADC input 2, HV_MEASURE
        adc_select_input(2);
        HV_ADC_counts = adc_read();
        HV_measure = HV_ADC_counts * ADC_conversion_factor * HV_conversion_factor;
        
        // Select ADC input 3, brightness_control
        adc_select_input(3);
        brightness_ADC_counts = adc_read();
        brightness_control = brightness_ADC_counts * ADC_conversion_factor * brightness_conversion_factor;
        
        printf("Loop Count:     %d\r\n", iLoopCounter);
        printf("HV raw:         0x%03x\r\n", HV_ADC_counts);
        printf("HV:             %f V\r\n", HV_measure);
        printf("brightness raw: 0x%03x\r\n", brightness_ADC_counts);
        printf("brightness:     %f %%\r\n", brightness_control * 100);
        
        iLoopCounter++;
        
        sleep_ms(100);
    }
    return 0;
}
