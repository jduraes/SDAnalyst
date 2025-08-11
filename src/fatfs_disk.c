#include "sd_card.h"
#include <stdio.h>
#include <string.h>

static uint32_t fat_partition_start = 0;
static uint32_t fat_sectors_per_cluster = 0;
static uint32_t fat_reserved_sectors = 0;
static uint32_t fat_root_dir_first_cluster = 0;

void analyze_fat_boot_sector(uint32_t lba_start) {
    uint8_t boot_sector[512];
    
    printf("\n--- FAT Boot Sector at LBA %u ---\n", lba_start);
    
    if (sd_read_block(lba_start, boot_sector) != 0) {
        printf("Error reading boot sector\n");
        return;
    }
    
    fat_partition_start = lba_start;
    
    // Check for FAT signature
    if (boot_sector[510] != 0x55 || boot_sector[511] != 0xAA) {
        printf("Invalid boot sector signature\n");
        return;
    }
    
    // Parse BPB (BIOS Parameter Block)
    uint16_t bytes_per_sector = *(uint16_t*)&boot_sector[11];
    uint8_t sectors_per_cluster = boot_sector[13];
    uint16_t reserved_sectors = *(uint16_t*)&boot_sector[14];
    uint8_t num_fats = boot_sector[16];
    uint16_t root_entries = *(uint16_t*)&boot_sector[17];
    uint16_t total_sectors_16 = *(uint16_t*)&boot_sector[19];
    uint16_t sectors_per_fat_16 = *(uint16_t*)&boot_sector[22];
    uint32_t total_sectors_32 = *(uint32_t*)&boot_sector[32];
    uint32_t sectors_per_fat_32 = *(uint32_t*)&boot_sector[36];
    uint32_t root_cluster = *(uint32_t*)&boot_sector[44];
    
    fat_sectors_per_cluster = sectors_per_cluster;
    fat_reserved_sectors = reserved_sectors;
    fat_root_dir_first_cluster = root_cluster;
    
    printf("Bytes per sector: %u\n", bytes_per_sector);
    printf("Sectors per cluster: %u\n", sectors_per_cluster);
    printf("Reserved sectors: %u\n", reserved_sectors);
    printf("Number of FATs: %u\n", num_fats);
    printf("Root entries (FAT12/16): %u\n", root_entries);
    printf("Total sectors: %u\n", total_sectors_32 ? total_sectors_32 : total_sectors_16);
    printf("Sectors per FAT: %u\n", sectors_per_fat_32 ? sectors_per_fat_32 : sectors_per_fat_16);
    
    if (sectors_per_fat_32) {
        printf("Root directory first cluster (FAT32): %u\n", root_cluster);
        printf("Filesystem: FAT32\n");
    } else {
        printf("Filesystem: FAT12/16\n");
    }
    
    // Extract volume label
    char volume_label[12];
    if (sectors_per_fat_32) {
        // FAT32
        memcpy(volume_label, &boot_sector[71], 11);
    } else {
        // FAT12/16
        memcpy(volume_label, &boot_sector[43], 11);
    }
    volume_label[11] = 0;
    
    // Trim spaces
    for (int i = 10; i >= 0 && volume_label[i] == ' '; i--) {
        volume_label[i] = 0;
    }
    
    if (strlen(volume_label) > 0) {
        printf("Volume label: '%s'\n", volume_label);
    }
}

uint32_t get_fat_partition_start(void) {
    return fat_partition_start;
}

uint32_t get_root_cluster(void) {
    return fat_root_dir_first_cluster;
}

uint32_t cluster_to_sector(uint32_t cluster) {
    if (fat_partition_start == 0) return 0;
    
    // For FAT32, cluster 2 is the first data cluster
    uint32_t first_data_sector = fat_partition_start + fat_reserved_sectors + 
                                (2 * (fat_sectors_per_cluster ? fat_sectors_per_cluster : 1));
    
    return first_data_sector + ((cluster - 2) * fat_sectors_per_cluster);
}
