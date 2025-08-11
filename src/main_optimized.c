#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "sd_analyzer.h"

#define VERSION "1.4.0"

int main() {
    stdio_init_all();
    
    // Wait for USB serial to be ready
    sleep_ms(2000);
    
    // Display startup banner
    sd_analyzer_print_banner("SD Card Analyzer", VERSION);
    
    // Initialize SD card
    if (sd_analyzer_init() != 0) {
        while (1) sleep_ms(1000);
    }
    
    // Get SD card analysis
    sd_analysis_t analysis;
    if (sd_analyzer_get_info(&analysis) != 0) {
        printf("Failed to analyze SD card\\n");
        while (1) sleep_ms(1000);
    }
    
    // Print card information
    sd_analyzer_print_card_info(&analysis.card_info);
    
    // Analyze partitions
    partition_info_t partitions[8];
    int partition_count = 0;
    
    if (analysis.has_gpt) {
        printf("\\nDetected GPT partition table\\n");
        partition_count = sd_analyzer_parse_gpt(partitions, 8);
    } else if (analysis.has_mbr) {
        printf("\\nDetected MBR partition table\\n");
        partition_count = sd_analyzer_parse_mbr(partitions, 8);
    } else {
        printf("\\nNo valid partition table found\\n");
    }
    
    // Analyze filesystems on each partition
    for (int i = 0; i < partition_count; i++) {
        printf("\\n--- Analyzing %s ---\\n", partitions[i].name);
        
        if (strncmp(partitions[i].filesystem, "FAT", 3) == 0) {
            sd_analyzer_analyze_fat(partitions[i].start_lba);
        } else {
            printf("Filesystem %s analysis not implemented yet\\n", partitions[i].filesystem);
        }
    }
    
    // Display raw sector data for debugging
    printf("\\n=== Raw MBR Data ===\\n");
    sd_analyzer_read_and_display_sector(0);
    
    printf("\\nSD card analysis complete!\\n");
    
    // Keep the program running
    while (1) {
        sleep_ms(1000);
    }
    
    return 0;
}
