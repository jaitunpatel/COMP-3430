
// Header file already provided in lab 4
#ifndef FAT32_H
#define FAT32_H

#include <inttypes.h>

#define NAME_LENGTH 8
#define VOLUME_LENGTH 11
#define TYPE_LENGTH 8 

#pragma pack(push)
#pragma pack(1)
struct FAT32_INFO {
	char boot_jump[3];
	char oem_name[NAME_LENGTH];
	uint16_t bytes_per_sector;
	uint8_t sector_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t num_of_fats;
	uint16_t root_entry_count;
	uint16_t total_sector16;
	uint8_t media;
	uint16_t fat_size16;
	uint16_t sectors_per_track;
	uint16_t num_of_heads;
	uint32_t hidden_sectors;
	uint32_t total_sector32;
	uint32_t fat_size32;
	uint16_t extra_flags;
	uint8_t version_number;
	uint8_t version_high;
	uint32_t root_cluster;
	uint16_t fs_info;
	uint16_t backup_boot_sector;
	char reserved_area[12];
	uint8_t drive_number;
	uint8_t reserved_sector;
	uint8_t boot_signature;
	uint32_t volume_id;
	char volume_label[VOLUME_LENGTH];
	int free_clusters;
	int bad_clusters;
	int used_clusters;
};
#pragma pack(pop)
typedef struct FAT32_INFO fat32BS;

#define EOC 0xFFFFFFF8  // page 18
#define BAD_CLUSTER 0x0FFFFFF7
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID

#endif
