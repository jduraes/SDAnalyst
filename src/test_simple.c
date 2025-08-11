#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// SD card SPI pins
#define SD_SPI_PORT spi0
#define SD_PIN_MISO 4
#define SD_PIN_CS   5
#define SD_PIN_SCK  2
#define SD_PIN_MOSI 3

void test_spi_loopback() {
    printf("Testing SPI loopback (connect MOSI to MISO temporarily)...\n");
    
    gpio_put(SD_PIN_CS, 1); // CS high
    
    uint8_t test_data[] = {0xAA, 0x55, 0x00, 0xFF};
    uint8_t rx_data[4];
    
    spi_write_read_blocking(SD_SPI_PORT, test_data, rx_data, 4);
    
    printf("Sent: ");
    for(int i = 0; i < 4; i++) {
        printf("0x%02X ", test_data[i]);
    }
    printf("\nReceived: ");
    for(int i = 0; i < 4; i++) {
        printf("0x%02X ", rx_data[i]);
    }
    printf("\n");
}

void test_sd_presence() {
    printf("Testing SD card presence...\n");
    
    // Select card
    gpio_put(SD_PIN_CS, 0);
    sleep_ms(1);
    
    // Send some dummy bytes and check response
    uint8_t response;
    for(int i = 0; i < 10; i++) {
        response = 0xFF;
        spi_write_read_blocking(SD_SPI_PORT, &response, &response, 1);
        printf("Dummy byte %d response: 0x%02X\n", i, response);
    }
    
    // Deselect card
    gpio_put(SD_PIN_CS, 1);
}

int main() {
    stdio_init_all();
    
    printf("Raspberry Pi Pico SD Card Connection Test\n");
    printf("=========================================\n");
    
    // Wait for USB serial
    sleep_ms(2000);
    
    // Initialize SPI at low speed
    printf("Initializing SPI at 400kHz...\n");
    spi_init(SD_SPI_PORT, 400 * 1000);
    gpio_set_function(SD_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MISO, GPIO_FUNC_SPI);
    
    // Initialize CS pin
    gpio_init(SD_PIN_CS);
    gpio_set_dir(SD_PIN_CS, GPIO_OUT);
    gpio_put(SD_PIN_CS, 1); // Deselect initially
    
    printf("Pin assignments:\n");
    printf("SCK (Clock):  GPIO%d\n", SD_PIN_SCK);
    printf("MOSI (Out):   GPIO%d\n", SD_PIN_MOSI);  
    printf("MISO (In):    GPIO%d\n", SD_PIN_MISO);
    printf("CS (Select):  GPIO%d\n", SD_PIN_CS);
    printf("\n");
    
    // Test SPI communication
    test_sd_presence();
    
    printf("\nIf all responses are 0xFF, check:\n");
    printf("1. SD card is inserted properly\n");
    printf("2. All wire connections are secure\n");
    printf("3. SD card has proper 3.3V power\n");
    printf("4. Pin numbers match your wiring\n");
    
    while(1) {
        sleep_ms(1000);
    }
    
    return 0;
}
