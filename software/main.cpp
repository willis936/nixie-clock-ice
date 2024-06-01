
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
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

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

#define FLAG_VALUE 123

#define HV_MIN 145
#define HV_MAX 190
#define HV_CUTOFF 195


// keep track of loop iterations
uint32_t iLoopCounter = 0;
uint32_t iLoopCounterLast = 0;
float tNow = 0.0f;
float tLast = 0.0f;
// ADC temporary input
uint16_t ADC_val;
// ADC inputs
uint16_t HV_ADC_counts;
float HV_measure;
uint16_t brightness_ADC_counts;
float brightness_control;
// HV control values
float HV_target;
float HV_error;

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
const float ADC_Vref = 3.3f;
const float ADC_conversion_factor = ADC_Vref / (float)((1 << 12) - 1);
// 1/101 voltage divider
const float HV_conversion_factor = 101.0f;
// 0 - 100 %
const float brightness_conversion_factor = 1.0f / ADC_Vref;



bool hv_timer_callback(struct repeating_timer *t) {
    // Read in ADC measuring HV, run control algorithm, and update HV boost PWM output
    
    iLoopCounter++;
    
    // Select ADC input 2, HV_MEASURE
    adc_select_input(2);
    HV_ADC_counts = adc_read();
    HV_measure = HV_ADC_counts * ADC_conversion_factor * HV_conversion_factor;
    
    // Select ADC input 3, brightness_control
    adc_select_input(3);
    brightness_ADC_counts = adc_read();
    brightness_control = brightness_ADC_counts * ADC_conversion_factor * brightness_conversion_factor;
    
    HV_target = HV_MIN + ((HV_MAX - HV_MIN) * brightness_control);
    HV_error = HV_measure - HV_target;
    
    return true;
}


void core1_entry() {
    multicore_fifo_push_blocking(FLAG_VALUE);
    
    uint32_t g = multicore_fifo_pop_blocking();
    
    if (g != FLAG_VALUE) {
        printf("Hmm, that's not right on core 1!\n");
    } else {
        printf("Its all gone well on core 1!");
    }
    
    // repeating 1 kHz timer for HV control
    struct repeating_timer hv_timer;
    add_repeating_timer_us(1000, hv_timer_callback, NULL, &hv_timer);
    
    while (true) {
        sleep_ms(100);
    }
}


bool print_timer_callback(struct repeating_timer *t) {
    // print out status info
    
    tNow = (float)time_us_64() / 1000000.0f;
    
    printf("Repeat at:      %f\r\n", tNow - tLast);
    printf("Loop Count:     %d\r\n", iLoopCounter - iLoopCounterLast);
    printf("HV:             %f V\r\n", HV_measure);
    printf("HV target:      %f V\r\n", HV_target);
    printf("HV error:       %f V\r\n", HV_error);
    printf("brightness:     %f %%\r\n", brightness_control * 100.0f);
    printf("\r\n");
    
    tLast = tNow;
    iLoopCounterLast = iLoopCounter;
    
    return true;
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
    
    // repeating 1 Hz timer for serial printout
    struct repeating_timer print_timer;
    add_repeating_timer_ms(1000, print_timer_callback, NULL, &print_timer);
    
    // launch HV control thread
    multicore_launch_core1(core1_entry);
    // Wait for it to start up
    uint32_t g = multicore_fifo_pop_blocking();
 
    if (g != FLAG_VALUE) {
        printf("Hmm, that's not right on core 0!\n");
    } else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!");
    }
    
    while (true) {
        tud_task();
        
        sleep_ms(100);
    }
    return 0;
}

