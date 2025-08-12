#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "sd_analyzer.h"
#include "partition_display.h"

#define VERSION "1.6.0"

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
    
    printf("\nSD card initialized successfully!\n");
    printf("\nAnalyzing SD card content...\n");
    
    // Get SD card analysis
    sd_analysis_t analysis;
    if (sd_analyzer_get_info(&analysis) != 0) {
        printf("Failed to analyze SD card\n");
        while (1) sleep_ms(1000);
    }
    
    printf("\n=== SD CARD ANALYSIS ===\n");
    printf("Card: %s, %.2f MB (%u blocks)\n", 
           analysis.card_info.type == SD_CARD_TYPE_SDHC ? "SDHC" : "SD",
           (analysis.card_info.blocks * 512.0) / (1024 * 1024),
           analysis.card_info.blocks);
    
    // Analyze partitions using enhanced display
    partition_info_t basic_partitions[8];
    enhanced_partition_info_t enhanced_partitions[8];
    int partition_count = 0;
    
    if (analysis.has_gpt) {
        printf("Partition table: GPT\n");
        partition_count = sd_analyzer_parse_gpt(basic_partitions, 8);
        
        // Convert to enhanced partition info
        for (int i = 0; i < partition_count; i++) {
            partition_display_enhance_partition_info(&enhanced_partitions[i], &basic_partitions[i]);
        }
        
        // Print unified table
        partition_display_print_unified_table(enhanced_partitions, partition_count, "GPT");
        
    } else if (analysis.has_mbr) {
        printf("Partition table: MBR\n");
        partition_count = sd_analyzer_parse_mbr(basic_partitions, 8);
        
        // Convert to enhanced partition info
        for (int i = 0; i < partition_count; i++) {
            partition_display_enhance_partition_info(&enhanced_partitions[i], &basic_partitions[i]);
        }
        
        // Print unified table
        partition_display_print_unified_table(enhanced_partitions, partition_count, "MBR");
        
    } else {
        printf("Partition table: None\n");
    }
    
    if (partition_count > 0) {
        // Show contents of ALL partitions
        printf("\n=== ALL PARTITION CONTENTS ===\n");
        for (int i = 0; i < partition_count; i++) {
            char size_str[32];
            uint64_t size_bytes = (uint64_t)enhanced_partitions[i].size_sectors * 512;
            partition_display_format_size(size_bytes, size_str, sizeof(size_str));
            
            const char* display_name = partition_display_get_display_name(&enhanced_partitions[i]);
            
            printf("\n--- PARTITION %d: %s (%s) ---\n", 
                   i + 1, 
                   enhanced_partitions[i].filesystem,
                   size_str);
            
            if (display_name && strlen(display_name) > 0 && strcmp(display_name, "(no label)") != 0) {
                printf("Volume Label: %s\n", display_name);
            }
            
            if (strcmp(enhanced_partitions[i].filesystem, "FAT32") == 0 ||
                strcmp(enhanced_partitions[i].filesystem, "FAT16") == 0 ||
                strcmp(enhanced_partitions[i].filesystem, "FAT12") == 0) {
                
                // Calculate actual root directory location for FAT
                uint32_t fat_start_lba = enhanced_partitions[i].start_lba;
                
                // Read the boot sector to get proper FAT parameters
                uint8_t boot_sector[512];
                if (sd_read_block(fat_start_lba, boot_sector) == 0) {
                    // Parse FAT boot sector to find root directory
                    uint16_t reserved_sectors = *(uint16_t*)(boot_sector + 14);
                    uint8_t num_fats = boot_sector[16];
                    uint32_t fat_size = *(uint32_t*)(boot_sector + 36); // FAT32
                    
                    uint32_t root_dir_lba = fat_start_lba + reserved_sectors + (num_fats * fat_size);
                    
                    printf("Root directory at LBA %u:\n", root_dir_lba);
                    sd_analyzer_list_fat_directory(root_dir_lba, "/");
                } else {
                    printf("Could not read boot sector for partition %d\n", i + 1);
                }
                
            } else if (strcmp(enhanced_partitions[i].filesystem, "exFAT") == 0) {
                printf("exFAT partition - detailed analysis not implemented yet\n");
            } else if (strncmp(enhanced_partitions[i].filesystem, "ext", 3) == 0) {
                printf("Linux ext filesystem - detailed analysis not implemented yet\n");
            } else {
                printf("Unknown filesystem type - cannot analyze contents\n");
            }
        }
        
    } else {
        printf("No partitions found.\n");
    }
    
    printf("\n=== SD CARD ANALYSIS COMPLETE ===\n");
    printf("All partitions and contents have been analyzed.\n");
    printf("System will now idle.\n");
    
    // Keep the program running
    while (1) {
        sleep_ms(1000);
    }
    
    return 0;
}
