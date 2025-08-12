#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "sd_analyzer.h"

#define VERSION "1.5.0"

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
    
    // Analyze partitions
    partition_info_t partitions[8];
    int partition_count = 0;
    
    if (analysis.has_gpt) {
        printf("Partition table: GPT\n");
        partition_count = sd_analyzer_parse_gpt(partitions, 8);
    } else if (analysis.has_mbr) {
        printf("Partition table: MBR\n");
        partition_count = sd_analyzer_parse_mbr(partitions, 8);
    } else {
        printf("Partition table: None\n");
    }
    
    if (partition_count > 0) {
        printf("\n+-----+-------------+---------+-----------+\n");
        printf("| #   | Name        | Type    | Size      |\n");
        printf("+-----+-------------+---------+-----------+\n");
        
        for (int i = 0; i < partition_count; i++) {
            char name[13];
            if (strlen(partitions[i].name) == 0) {
                snprintf(name, sizeof(name), "Partition %d", i + 1);
            } else {
                snprintf(name, sizeof(name), "%.11s", partitions[i].name);
            }
            
            printf("| %-3d | %-11s | %-7s | %6.1f MB |\n", 
                   i + 1,
                   name,
                   partitions[i].filesystem,
                   (partitions[i].size_sectors * 512.0) / (1024 * 1024));
        }
        printf("+-----+-------------+---------+-----------+\n");
        
        // Show contents of ALL partitions
        printf("\n=== ALL PARTITION CONTENTS ===\n");
        for (int i = 0; i < partition_count; i++) {
            printf("\n--- PARTITION %d: %s (%.1f MB) ---\n", 
                   i + 1, 
                   partitions[i].filesystem,
                   (partitions[i].size_sectors * 512.0) / (1024 * 1024));
            
            if (strcmp(partitions[i].filesystem, "FAT32") == 0 ||
                strcmp(partitions[i].filesystem, "FAT16") == 0 ||
                strcmp(partitions[i].filesystem, "FAT12") == 0) {
                
                // Calculate actual root directory location for FAT
                uint32_t fat_start_lba = partitions[i].start_lba;
                
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
                
            } else if (strcmp(partitions[i].filesystem, "exFAT") == 0) {
                printf("exFAT partition - detailed analysis not implemented yet\n");
            } else if (strncmp(partitions[i].filesystem, "ext", 3) == 0) {
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
