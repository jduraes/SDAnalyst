#include "partition_display.h"
#include "table_formatter.h"
#include "sd_card.h"
#include <stdio.h>
#include <string.h>

void partition_display_print_unified_table(const enhanced_partition_info_t* partitions,
                                          int partition_count,
                                          const char* table_type) {
    if (partition_count <= 0) {
        printf("No partitions found.\n");
        return;
    }
    
    // Create table
    table_t* table = table_create(5, partition_count);
    if (!table) return;
    
    // Set headers
    table_set_header(table, 0, "#");
    table_set_header(table, 1, "Name/Label");
    table_set_header(table, 2, "Type");
    table_set_header(table, 3, "Size");
    table_set_header(table, 4, "Status/Type");
    
    // Right-align the size column
    table_set_right_align(table, 3, true);
    
    // Fill data
    for (int i = 0; i < partition_count; i++) {
        char partition_num[8];
        char size_str[32];
        
        snprintf(partition_num, sizeof(partition_num), "%d", i + 1);
        
        uint64_t size_bytes = (uint64_t)partitions[i].size_sectors * 512;
        partition_display_format_size(size_bytes, size_str, sizeof(size_str));
        
        const char* display_name = partition_display_get_display_name(&partitions[i]);
        
        table_set_cell(table, i, 0, partition_num);
        table_set_cell(table, i, 1, display_name);
        table_set_cell(table, i, 2, partitions[i].filesystem);
        table_set_cell(table, i, 3, size_str);
        table_set_cell(table, i, 4, partitions[i].status_description);
    }
    
    // Print the table
    printf("\n");
    table_print(table);
    
    // Clean up
    table_free(table);
}

// Keep all other functions unchanged from the original
// [Previous functions remain the same]
