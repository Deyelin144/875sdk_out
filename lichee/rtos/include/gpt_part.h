#ifndef _GPT_PART_H
#define _GPT_PART_H

#include <aw_types.h>

#define GPT_ENTRIES 128
#define GPT_NAMELEN 36
#define GPT_ENTRY_OFFSET 1024
#define GPT_HEAD_OFFSET 512
#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL
#define GPT_START_MAPPING (CONFIG_COMPONENTS_AW_BLKPART_LOGICAL_OFFSET * 1024)
#define GPT_TABLE_SIZE (CONFIG_COMPONENTS_AW_BLKPART_PART_TABLE_SIZE * 1024)
#define GPT_ADDRESS (GPT_START_MAPPING - GPT_TABLE_SIZE)

struct gpt_header {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t disk_guid[16];
    uint64_t entries_lba;
    uint32_t entries_count;
    uint32_t entry_size;
    uint32_t partition_array_crc32;
} __attribute__((packed));

struct gpt_entry {
    uint8_t type_guid[16];
    uint8_t partition_guid[16];
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    uint16_t part_name[GPT_NAMELEN];
};

struct gpt_part {
    char name[GPT_NAMELEN];
    uint32_t off_sects;
    uint32_t sects;
    int index;
    struct gpt_header *header;
    struct gpt_entry *entry;
};

struct gpt_part *first_gpt_part(void *gpt_buf);
struct gpt_part *next_gpt_part(struct gpt_part *part);
#define foreach_gpt_part(gpt_buf, gpt_part)                 \
    for (gpt_part = first_gpt_part(gpt_buf);                \
            gpt_part; gpt_part = next_gpt_part(gpt_part))
int gpt_part_cnt(void *gpt_buf);
int show_gpt_part(void *gpt_buf);
int get_part_info_by_name(void *gpt_buf, char *name, unsigned int *start_sector, unsigned int* sectors);

#endif    /* _GPT_PART_H */