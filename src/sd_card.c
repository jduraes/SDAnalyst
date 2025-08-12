#include "sd_card.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

static spi_inst_t *sd_spi;
static uint sd_cs_pin;
static sd_card_info_t sd_info;

static void sd_cs_select() {
    gpio_put(sd_cs_pin, 0);
}

static void sd_cs_deselect() {
    gpio_put(sd_cs_pin, 1);
}

static uint8_t sd_spi_write(uint8_t data) {
    uint8_t rx_data;
    spi_write_read_blocking(sd_spi, &data, &rx_data, 1);
    return rx_data;
}

static void sd_wait_not_busy() {
    int timeout = 1000;
    while (sd_spi_write(0xFF) != 0xFF && --timeout > 0);
}

static uint8_t sd_send_command(uint8_t cmd, uint32_t arg) {
    uint8_t response;
    
    sd_wait_not_busy();
    
    // Send command packet
    sd_spi_write(cmd);
    sd_spi_write((arg >> 24) & 0xFF);
    sd_spi_write((arg >> 16) & 0xFF);
    sd_spi_write((arg >> 8) & 0xFF);
    sd_spi_write(arg & 0xFF);
    
    // CRC (only matters for CMD0 and CMD8)
    if (cmd == CMD0) {
        sd_spi_write(0x95);
    } else if (cmd == CMD8) {
        sd_spi_write(0x87);
    } else {
        sd_spi_write(0x01);
    }
    
    // Wait for response (increased attempts)
    for (int i = 0; i < 20; i++) {
        response = sd_spi_write(0xFF);
        if ((response & 0x80) == 0) break;
    }
    
    return response;
}

// Enhanced reset sequence for problematic cards
static int sd_enhanced_reset() {
    printf("Enhanced reset sequence for problematic cards...\n");
    
    // Power cycle simulation - CS high for longer
    sd_cs_deselect();
    sleep_ms(100);  // Much longer delay
    
    // Send many more clock cycles (200+) 
    printf("Sending 200 clock cycles...\n");
    for (int i = 0; i < 25; i++) {
        sd_spi_write(0xFF);
    }
    
    sleep_ms(50);
    
    // Try multiple CMD0 attempts with different approaches
    for (int attempt = 0; attempt < 10; attempt++) {
        printf("CMD0 attempt %d...\n", attempt + 1);
        
        sd_cs_select();
        sleep_ms(1);
        
        uint8_t response = sd_send_command(CMD0, 0);
        printf("CMD0 response: 0x%02X\n", response);
        
        if (response == 0x01) {
            printf("CMD0 successful after %d attempts\n", attempt + 1);
            return 0;  // Success
        }
        
        sd_cs_deselect();
        
        // Progressive delays and more clock cycles
        for (int i = 0; i < (attempt + 1) * 5; i++) {
            sd_spi_write(0xFF);
        }
        sleep_ms((attempt + 1) * 10);
    }
    
    return -1;  // Failed
}

int sd_init(spi_inst_t *spi, uint sck, uint mosi, uint miso, uint cs) {
    sd_spi = spi;
    sd_cs_pin = cs;
    
    // Initialize SPI at much slower speed
    spi_init(spi, 25 * 1000); // Start at 25kHz for maximum compatibility
    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(miso, GPIO_FUNC_SPI);
    
    // Initialize CS pin
    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    sd_cs_deselect();
    
    sleep_ms(100); // Much longer power-up delay
    
    // Enhanced reset sequence
    if (sd_enhanced_reset() != 0) {
        printf("Enhanced reset failed, trying standard reset...\n");
        
        // Fallback to original sequence
        sd_cs_select();
        uint8_t response = sd_send_command(CMD0, 0);
        printf("Fallback CMD0 response: 0x%02X\n", response);
        
        if (response != 0x01) {
            printf("CMD0 failed - card not responding\n");
            sd_cs_deselect();
            return -1;
        }
    }
    
    // Continue with CMD8 and rest of initialization...
    printf("Sending CMD8 (voltage check)...\n");
    uint8_t response = sd_send_command(CMD8, 0x1AA);
    printf("CMD8 response: 0x%02X\n", response);
    
    if (response == 0x01) {
        printf("SD v2.0 card detected\n");
        uint32_t ocr = 0;
        for (int i = 0; i < 4; i++) {
            ocr = (ocr << 8) | sd_spi_write(0xFF);
        }
        printf("CMD8 OCR response: 0x%08X\n", ocr);
        
        if ((ocr & 0xFFF) != 0x1AA) {
            printf("CMD8 OCR check failed\n");
            sd_cs_deselect();
            return -2;
        }
        
        // Rest of initialization with longer timeouts...
        printf("Starting enhanced ACMD41 sequence...\n");
        int timeout = 200; // Double the timeout
        int attempt = 0;
        
        do {
            uint8_t cmd55_resp = sd_send_command(CMD55, 0);
            sleep_ms(5); // Longer delay
            response = sd_send_command(ACMD41, 0x40000000); // Start with HCS
            
            attempt++;
            if (attempt % 5 == 0) { // More frequent updates
                printf("Attempt %d: CMD55=0x%02X, ACMD41=0x%02X\n", attempt, cmd55_resp, response);
            }
            
            if (response == 0x00) {
                printf("ACMD41 successful after %d attempts\n", attempt);
                break;
            }
            
            sleep_ms(20); // Much longer delay between attempts
        } while (--timeout > 0);
        
        if (timeout == 0) {
            printf("ACMD41 timeout - card not ready\n");
            sd_cs_deselect();
            return -3;
        }
        
        // Continue with rest of initialization...
        response = sd_send_command(CMD58, 0);
        if (response != 0x00) {
            sd_cs_deselect();
            return -1;
        }
        
        uint32_t ocr_resp = 0;
        for (int i = 0; i < 4; i++) {
            ocr_resp = (ocr_resp << 8) | sd_spi_write(0xFF);
        }
        
        if (ocr_resp & 0x40000000) {
            sd_info.type = SD_CARD_TYPE_SDHC;
            printf("SDHC card detected\n");
        } else {
            sd_info.type = SD_CARD_TYPE_SD;
            printf("Standard SD card detected\n");
        }
        
        // Get card size
        response = sd_send_command(CMD9, 0);
        if (response != 0x00) {
            sd_cs_deselect();
            return -1;
        }
        
        // Wait for data start token
        while (sd_spi_write(0xFF) != 0xFE);
        
        uint8_t csd[16];
        for (int i = 0; i < 16; i++) {
            csd[i] = sd_spi_write(0xFF);
        }
        sd_spi_write(0xFF); // CRC
        sd_spi_write(0xFF); // CRC
        
        // Parse CSD for SDHC
        if (sd_info.type == SD_CARD_TYPE_SDHC) {
            uint32_t c_size = ((csd[7] & 0x3F) << 16) | (csd[8] << 8) | csd[9];
            sd_info.blocks = (c_size + 1) * 1024;
        }
        
        sd_cs_deselect();
        
        // Increase SPI speed after successful initialization
        spi_set_baudrate(spi, 1000 * 1000); // 1MHz for data transfer
        
        printf("SD card initialization complete!\n");
        printf("Card type: %s\n", sd_info.type == SD_CARD_TYPE_SDHC ? "SDHC" : "SD");
        printf("Blocks: %u\n", sd_info.blocks);
        
        return 0;
    }
    
    sd_cs_deselect();
    return -1;
}

// Rest of the functions remain the same...

const sd_card_info_t* sd_get_info_ptr() {
    return &sd_info;
}

int sd_read_block(uint32_t lba, uint8_t* buffer) {
    uint8_t response;
    uint32_t address;
    
    // Convert LBA to byte address for standard capacity cards
    if (sd_info.type == SD_CARD_TYPE_SDHC) {
        address = lba;  // SDHC uses block addressing
    } else {
        address = lba * 512;  // Standard SD uses byte addressing
    }
    
    sd_cs_select();
    
    // Send CMD17 (read single block)
    response = sd_send_command(CMD17, address);
    if (response != 0x00) {
        sd_cs_deselect();
        return -1;
    }
    
    // Wait for data start token
    int timeout = 1000;
    do {
        response = sd_spi_write(0xFF);
    } while (response != 0xFE && --timeout > 0);
    
    if (timeout == 0) {
        sd_cs_deselect();
        return -1;
    }
    
    // Read 512 bytes of data
    for (int i = 0; i < 512; i++) {
        buffer[i] = sd_spi_write(0xFF);
    }
    
    // Read CRC (but don't check it)
    sd_spi_write(0xFF);
    sd_spi_write(0xFF);
    
    sd_cs_deselect();
    return 0;
}

void sd_print_debug_info() {
    printf("=== SD Card Debug Info ===\n");
    printf("Type: %s\n", sd_info.type == SD_CARD_TYPE_SDHC ? "SDHC" : "SD");
    printf("Blocks: %u\n", sd_info.blocks);
    printf("Capacity: %.2f MB\n", (sd_info.blocks * 512.0) / (1024 * 1024));
    printf("========================\n");
}

int sd_get_info(sd_card_info_t *info) {
    if (info == NULL) return -1;
    *info = sd_info;
    return 0;
}
