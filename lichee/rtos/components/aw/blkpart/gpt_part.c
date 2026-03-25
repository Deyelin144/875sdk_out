/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <string.h>
#include <awlog.h>
#include <stdlib.h>
#include <blkpart.h>

#include <gpt_part.h>

static bool is_gpt_valid(void *gpt_buf)
{
    struct gpt_header *gpt_head = (struct gpt_header *)(gpt_buf + GPT_HEAD_OFFSET);

    if (gpt_head->signature != GPT_HEADER_SIGNATURE) {
        printf("invalid GPT partition table\n");
        hexdump(gpt_buf, 128);
        return false;
    }
    return true;
}

int show_gpt_part(void *gpt_buf)
{
    struct gpt_header *header = (struct gpt_header *)(gpt_buf + GPT_HEAD_OFFSET);
    struct gpt_entry *entry = (struct gpt_entry *)(gpt_buf + GPT_ENTRY_OFFSET);
    int i;

    if (!is_gpt_valid(gpt_buf))
        return -1;

    printf("(GPT) Partition Table:\n");
    printf("\tname\toff_sect\tsects\n");
    for(i = 0; i < header->entries_count; i++)
        printf("\t%s\t%lx\t%lx\n", (char *)entry[i].part_name,
                (long)entry[i].starting_lba, (long)entry[i].ending_lba);

    return 0;
}

int get_part_info_by_name(void *gpt_buf, char* name, unsigned int* start_sector, unsigned int* sectors)
{
    int i, j;

    char char8_name[GPT_NAMELEN] = {0};
    struct gpt_header *gpt_head = (struct gpt_header*)(gpt_buf + GPT_HEAD_OFFSET);
    struct gpt_entry *entry = (struct gpt_entry*)(gpt_buf + GPT_ENTRY_OFFSET);

    if (!is_gpt_valid(gpt_buf))
        return -1;

    for(i = 0; i < gpt_head->entries_count; i++ ){
        for(j = 0; j < GPT_NAMELEN; j++ ) {
            char8_name[j] = (char)(entry[i].part_name[j]);
        }
        if(!strcmp(name,char8_name)){
            *start_sector = (uint)entry[i].starting_lba;
            *sectors = ((uint)entry[i].ending_lba + 1) - (uint)entry[i].starting_lba;
            return 0;
        }
    }

    return -1;
}

int gpt_part_cnt(void *gpt_buf)
{
    struct gpt_header *header = (struct gpt_header *)(gpt_buf + GPT_HEAD_OFFSET);

    if (!is_gpt_valid(gpt_buf))
        return -1;

    return header->entries_count;
}

static void gpt_part_name_copy(char *dest, uint16_t *src, int len)
{
    int i;

    for (i = 0; i < len; i++)
        dest[i] = (char)(src[i]);
}

struct gpt_part *first_gpt_part(void *gpt_buf)
{
    struct gpt_part *part;
    struct gpt_header *header;
    struct gpt_entry *entry;

    if (!is_gpt_valid(gpt_buf))
        return NULL;

    header = (struct gpt_header *)(gpt_buf + GPT_HEAD_OFFSET);
    if (header->entries_count == 0)
        return NULL;

    part = malloc(sizeof(*part));
    if (!part)
        return NULL;

    entry = (struct gpt_entry *)(gpt_buf + GPT_ENTRY_OFFSET);
    gpt_part_name_copy(part->name, entry[0].part_name, GPT_NAMELEN);
    part->header = header;
    part->entry = entry;
    part->index = 0;
    part->off_sects = (u32)entry[0].starting_lba + GPT_START_MAPPING;
    part->sects = ((u32)entry[0].ending_lba + 1) - (u32)entry[0].starting_lba;
    return part;
}

struct gpt_part *next_gpt_part(struct gpt_part *part)
{
    int i;
    struct gpt_header *header = part->header;
    struct gpt_entry *entry = part->entry;

    part->index += 1;
    if (part->index >= header->entries_count)
        goto end;

    i = part->index;
    gpt_part_name_copy(part->name, entry[i].part_name, GPT_NAMELEN);
    part->off_sects = entry[i].starting_lba + GPT_START_MAPPING;
    part->sects = entry[i].ending_lba + 1 - entry[i].starting_lba;
    return part;

end:
    /**
     * the part is allocate by first_gpt_part(), should be free when there
     * is no any other partition
     */
    free(part);
    return NULL;
}
