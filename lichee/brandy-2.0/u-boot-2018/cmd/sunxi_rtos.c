/*
 * (C) Copyright 2019-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/global_data.h>
#include <command.h>
#include <malloc.h>
#include <rtos_image.h>
#include <private_uboot.h>
#include <sunxi_image_verifier.h>
#include <mapmem.h>
#include <sys_partition.h>
#include <linux/compiler.h>
// #include <linux/elf.h>
#include <asm/io.h>
#include <elf.h>

// Add declaration for sunxi_flash_read function
extern int sunxi_flash_read(unsigned int sector, unsigned int nsectors, void *buffer);

#define ELF_DBG(fmt, arg...)

#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a) __ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)

#define LOAD_BIN    (0x1)
#define LOAD_ELF    (0x2)

/* start_sector:partition start sector
 * offset:offset(bytes) in part, not sector, not align 512 bytes is ok
 * size:read bytes, not sectors
 */
static int elf_load_data_from_flash(unsigned int start_sector, unsigned int offset, unsigned int size, void *buf)
{
	unsigned int load_sector = 0, sectors = 0;
	unsigned int poff, len;

	char *tmp_buf = malloc(512);
	if (tmp_buf == NULL) {
		printf("malloc tmp_buf failed\n");
		return -1;
	}

	ELF_DBG("load elf data from sector:%08x offset:%08x bytes, size:%08x, to mem:%08x\n", start_sector, offset, size, buf);
	// Step 1: Read initial data that is not sector-aligned (512 bytes)
	if (offset % 512) {
		load_sector = start_sector + (ALIGN_DOWN(offset, 512) / 512);
		poff = offset - ALIGN_DOWN(offset, 512);
		len = MIN(512 - poff, size);
		if (sunxi_flash_read(load_sector, 1, tmp_buf) == 0) {
			printf("flash read failed\n");
			free(tmp_buf);
			return -1;
		}

		memcpy(buf, tmp_buf + poff, len);
		offset += len;    //offset is align to 512 from now on
		buf += len;
		size -= len;
	}

	// Step 2: Read sector-aligned data
	if (size >= 512) {
		load_sector = start_sector + (offset / 512);
		len = ALIGN_DOWN(size, 512);
		sectors = len / 512;
		if (sunxi_flash_read(load_sector, sectors, buf) == 0) {
			printf("flash read failed\n");
			free(tmp_buf);
			return -1;
		}

		offset += len;   //offset is align to 512
		buf += len;
		size -= len;
	}

	// Step 3: Read final data that is not sector-aligned
	if (size) {
		memset(tmp_buf, '0', 512);
		load_sector = start_sector + (offset / 512);
		if (sunxi_flash_read(load_sector, 1, tmp_buf) == 0) {
			printf("flash read failed\n");
			free(tmp_buf);
			return -1;
		}

		memcpy(buf, tmp_buf, size);
	}

	free(tmp_buf);
	return 0;
}

int load_elf_image(unsigned int start_sector, unsigned int ih_hsize)
{
	int i;

#define TEMP_ELF_SIZE (4096)
#define TEMP_ELF_SECTORS (TEMP_ELF_SIZE / 512)
	void *img_addr = NULL;
	void *tmp_addr = malloc(TEMP_ELF_SIZE);
	if (tmp_addr == NULL) {
		printf("malloc elf head failed\n");
		return -1;
	}
	ELF_DBG("malloc elf head addr:%08x\n", tmp_addr);

	memset(tmp_addr, 0, TEMP_ELF_SIZE);
	if (sunxi_flash_read(start_sector, TEMP_ELF_SECTORS, tmp_addr) == 0) {
		printf("flash read elf head error\n");
		free(tmp_addr);
		return -1;
	}

	img_addr = tmp_addr + ih_hsize;
	ELF_DBG("elf real addr:%08x\n", img_addr);

	// First check ELF magic number
	unsigned char *e_ident = (unsigned char *)img_addr;
	if (memcmp(e_ident, ELFMAG, SELFMAG) != 0) {
		printf("Not a valid ELF image\n");
		free(tmp_addr);
		return -1;
	}

	// Handle ELF32 and ELF64 formats based on ELF class
	if (e_ident[EI_CLASS] == ELFCLASS32) {
		// Handle ELF32 format
		// printf("ELF32 format detected\n");
		Elf32_Ehdr *ehdr32 = (Elf32_Ehdr *)img_addr;
		Elf32_Phdr *phdr32 = (Elf32_Phdr *)(img_addr + ehdr32->e_phoff);

		// Load each program header
		for (i = 0; i < ehdr32->e_phnum; i++) {
			if (phdr32[i].p_type != PT_LOAD) {
				continue;
			}

			if (phdr32[i].p_memsz == 0 || phdr32[i].p_filesz == 0) {
				continue;
			}

			void *dst = (void *)(unsigned long)phdr32[i].p_paddr;
			// printf("Loading ELF32 segment to 0x%08lx (size: 0x%08lx bytes)\n", 
			// 	(unsigned long)dst, (unsigned long)phdr32[i].p_filesz);

			if (elf_load_data_from_flash(start_sector, 
					ih_hsize + phdr32[i].p_offset, 
					phdr32[i].p_filesz, 
					dst) != 0) {
				printf("Failed to load ELF32 segment\n");
				free(tmp_addr);
				return -1;
			}

			// Zero out additional memory
			if (phdr32[i].p_filesz < phdr32[i].p_memsz) {
				memset((unsigned char *)dst + phdr32[i].p_filesz, 
					0, 
					phdr32[i].p_memsz - phdr32[i].p_filesz);
			}
		}
	} else if (e_ident[EI_CLASS] == ELFCLASS64) {
		// Handle ELF64 format
		// printf("ELF64 format detected\n");
		Elf64_Ehdr *ehdr64 = (Elf64_Ehdr *)img_addr;
		Elf64_Phdr *phdr64 = (Elf64_Phdr *)(img_addr + ehdr64->e_phoff);

		// Load each program header
		for (i = 0; i < ehdr64->e_phnum; i++) {
			if (phdr64[i].p_type != PT_LOAD) {
				continue;
			}

			if (phdr64[i].p_memsz == 0 || phdr64[i].p_filesz == 0) {
				continue;
			}

			void *dst = (void *)(unsigned long)phdr64[i].p_paddr;
			// printf("Loading ELF64 segment to 0x%08lx (size: 0x%08lx bytes)\n", 
			// 	(unsigned long)dst, (unsigned long)phdr64[i].p_filesz);

			if (elf_load_data_from_flash(start_sector, 
					ih_hsize + phdr64[i].p_offset, 
					(size_t)phdr64[i].p_filesz, 
					dst) != 0) {
				printf("Failed to load ELF64 segment\n");
				free(tmp_addr);
				return -1;
			}

			// Zero out additional memory
			if (phdr64[i].p_filesz < phdr64[i].p_memsz) {
				memset((unsigned char *)dst + phdr64[i].p_filesz, 
					0, 
					(size_t)(phdr64[i].p_memsz - phdr64[i].p_filesz));
			}
		}
	} else {
		printf("Unsupported ELF class: %d\n", e_ident[EI_CLASS]);
		free(tmp_addr);
		return -1;
	}

	free(tmp_addr);
	return 0;
}

/* this is Temporary Use API */
#if defined(CONFIG_MACH_SUN20IW2) && defined(CONFIG_SUNXI_NAND)

void uboot_jmp(phys_addr_t addr)
{
	asm volatile("bx r0");
}

static u32 simple_atoi(char *s)
{
	int num = 0, flag = 0;
	int i;
	int hex = 0;

	if ((s[0] == '0') && (s[1] == 'x'))
		hex = 1;

	if ((s[0] == '0') && (s[1] == 'X'))
		hex = 1;

	if (hex == 1) {
		for (i = 2; i <= strlen((char *)s); i++) {
			if ((s[i] >= '0' && s[i] <= '9'))
				num = num * 16 + s[i] - '0';
			else if ((s[i] >= 'a' && s[i] <= 'f'))
				num = num * 16 + s[i] - 'a' + 10;
			else if ((s[i] >= 'A' && s[i] <= 'F'))
				num = num * 16 + s[i] - 'A' + 10;
			else
				break;
		}
	} else {
		for (i = 0; i <= strlen((char *)s); i++) {
			if (s[i] >= '0' && s[i] <= '9')
				num = num * 10 + s[i] -'0';
			else if (s[0] == '-' && i == 0)
				flag = 1;
			else
				break;
		}
	}

	if (flag == 1)
		num = num * -1;

	return num;
}

extern int sunxi_image_header_magic_check(sunxi_image_header_t *ih);
#if defined(CONFIG_SUNXI_RTOS_OTA_RB)
extern void ota_watchdog_start(void);
int env_save(void);

char *loadparts_a = "arm-ococci@:dsp-ococci@:rv-ococci@:config@0xc000000:";
char *loadparts_b = "arm-b@:config@0xc000000:";

//GPRCM REG data will be retained unless cold boot
#define GPRCM_SYS_PRIV_REG7 (0x40050210)  // 这是一个GPRCM(通用电源和复位控制模块)寄存器地址，用于保存系统重启次数等信息，即使在冷启动时也能保持数据。
#define UBOOT_REG_WRITE_32(reg, value) (*(volatile uint32_t *)(reg) = (value))
#define UBOOT_REG_REG_32(reg) (*(volatile uint32_t *)(reg))

#define uboot_ota_try_times() UBOOT_REG_REG_32(GPRCM_SYS_PRIV_REG7)

// 增加尝试次数
#define uboot_ota_try_times_update()                                   \
	do {                                                               \
		uint t_try_times = UBOOT_REG_REG_32(GPRCM_SYS_PRIV_REG7);      \
		t_try_times++;                                                 \
		UBOOT_REG_WRITE_32(GPRCM_SYS_PRIV_REG7, t_try_times);          \
	} while (0)
// 清除尝试次数
#define uboot_ota_try_times_clear() UBOOT_REG_WRITE_32(GPRCM_SYS_PRIV_REG7, 0)
#define uboot_update_env_to(load)                                      \
	do {                                                               \
		env_set("loadparts", load);                                    \
		env_save();                                                    \
		uboot_ota_try_times_clear();                                   \
	} while (0)

#define OTA_TRY_LIMIT 3

char *ota_adjust(char *current_load)
{
	printf("ota try times %d\n", uboot_ota_try_times());
	// 但是如果尝试次数为10，测试发现是进入深休PM_MODE_HIBERNATION导致 255是standby模式，不进行切换
	if (uboot_ota_try_times() == 10 || uboot_ota_try_times() == 255) {
		printf("wakeup, ignore!\n");
		ota_watchdog_start();
		return current_load;
	}
	// 从GPRCM寄存器读取当前启动尝试次数,如果超过限制(OTA_TRY_LIMIT = 3)，则认为新固件启动失败
	if (uboot_ota_try_times() > OTA_TRY_LIMIT) {
		printf("try to start new img fail!\n");

		if (strcmp(current_load, loadparts_a) == 0) {
			uboot_update_env_to(loadparts_b);
			return loadparts_b;
		} else {
			uboot_update_env_to(loadparts_a);
			return loadparts_a;
		}

	} else {
		uboot_ota_try_times_update();
	}
	ota_watchdog_start();
	return current_load;
}
#else
char *ota_adjust(char *current_load)
{
	return current_load;
}
#endif


int load_parts(phys_addr_t *rtos_base, phys_addr_t *rv_base, phys_addr_t *dsp_base)
{
	void *loadaddr;
	uint load_start_sector = 0;
	uint load_sectors = 0;
	uint load_start_byte = 0;
	uint load_bytes = 0;
	char *sector_buf = malloc(512);
	int ret = 0;

	char *loadparts;
	char loadname[16];
	char loadaddr_str[16];
	int i = 0;

	// Check loading method environment variable
	unsigned int load_flag = 0, elf_flag = LOAD_BIN;
	char *load_way = env_get("load_way");
	if ((load_way != NULL) && (strncmp(load_way, "elf", strlen("elf")) == 0)) {
		elf_flag = LOAD_ELF;
		printf("load_way: elf, using ELF loading mode\n");
	} else {
		printf("load_way: bin, using BIN loading mode\n");
	}

	loadparts = env_get("loadparts");
	if (loadparts == NULL) {
		printf("no loadparts\n");
		ret = 0;
		goto load_part_out;
	}
	loadparts = ota_adjust(loadparts);
	printf("loadparts:%s\n", loadparts);
	while ((loadparts != NULL) && (*loadparts != 0)) {
		i = 0;
		memset(loadname, 0, 16);
		memset(loadaddr_str, 0, 16);
		while ((loadparts != NULL) && (*loadparts != '@') && (*loadparts != 0))
			loadname[i++] = *loadparts++;

		if ((loadparts != NULL) && (*loadparts != '@')) {
			printf("%d: not @\n", i);
			break;
		}

		if (loadparts != NULL)
			loadparts++;

		i = 0;
		if (*loadparts == ':') {
			loadaddr = NULL;
		} else {
			while ((loadparts != NULL) && (*loadparts != ':') && (*loadparts != 0))
				loadaddr_str[i++] = *loadparts++;
			loadaddr = (void *)simple_atoi(loadaddr_str);
		}
		load_start_sector = 0;
		load_sectors = 0;
		sunxi_partition_get_info_byname(loadname, &load_start_sector, &load_sectors);
		load_start_byte = load_start_sector * 512;
		load_bytes = load_sectors * 512;
		if (load_start_sector) {
			printf("load_start_sector=%d\n", load_start_sector);
			memset(sector_buf, 0, 512);
			if (!sunxi_flash_read(load_start_sector, 1, sector_buf)) {
				printf("flash read ih error\n");
				ret = -1;
				goto load_part_out;
			}
			sunxi_image_header_t *ih = (sunxi_image_header_t *)(sector_buf);
			if (sunxi_image_header_magic_check(ih)) {
				printf("ih->ih_magic:%x", ih->ih_magic);
				printf("%s not image header magic\n", loadname);
				ih = NULL;
			} else {
#if 0
				printf("get image header magic\n");
				printf("-----------------------------\n");
				sunxi_image_header_dump(ih);
				printf("-----------------------------\n");
#endif
				load_start_byte += ih->ih_hsize;
				load_bytes = ih->ih_psize;
				loadaddr = (void *)ih->ih_load;
			}

			if (loadaddr == NULL) {
				printf("no loadaddr for %s\n", loadname);
			} else {
				printf("load %s to 0x%lx from %d, size %d\n",
						loadname, (unsigned long)loadaddr, load_start_byte, load_bytes);

				int first_part_size = 0;

				if (ih != NULL) {
					first_part_size = 512 - ih->ih_hsize;
					memcpy(loadaddr, sector_buf + ih->ih_hsize, first_part_size);
				}
				load_bytes -= first_part_size;
				load_sectors = (load_bytes + 511)/512;

				load_flag = LOAD_BIN;
				if ((strncmp(loadname, "arm-", 4) == 0) || (strncmp(loadname, "rv-", 3) == 0) || (strncmp(loadname, "dsp-", 4) == 0))
					load_flag = elf_flag;

				if (load_flag == LOAD_BIN) {
				printf("load_start_sector=%d, load_sectors=%d, loadaddr=0x%08x,first_part_size=0x%08x\n", load_start_sector, load_sectors, (unsigned int)loadaddr, first_part_size);
				if (!sunxi_flash_read(load_start_sector + (first_part_size > 0 ? 1 : 0), load_sectors, loadaddr + first_part_size)) {
					printf("flash read data error\n");
					ret = -1;
					goto load_part_out;
				}
				} else if (load_flag == LOAD_ELF) {
					printf("load %s elf from flash\n", loadname);
					if (load_elf_image(load_start_sector, ih->ih_hsize)) {
						printf("load elf from flash error\n");
						ret = -1;
						goto load_part_out;
					}
				}

			if (sunxi_get_secureboard()) {
				if (ih != NULL) {
					if (sunxi_image_header_check(ih, loadaddr)) {
						printf("image_header check error\n");
						ret = -1;
						goto load_part_out;
					}
					uint32_t second_part_size = 0;
					uint8_t *tlv = malloc(2048);

					if (!tlv) {
						printf("error: malloc tlv buffer failed\n");
						ret = -1;
						goto load_part_out;
					}

					second_part_size = first_part_size + load_sectors * 512 - ih->ih_psize;
					memcpy(tlv, loadaddr + ih->ih_psize, second_part_size);
					load_bytes = ih->ih_tsize - second_part_size;
					if (!sunxi_flash_read(load_start_sector + 1 + load_sectors, (load_bytes + 511) / 512, tlv + second_part_size)) {
						printf("flash read data error\n");
						free(tlv);
						ret = -1;
						goto load_part_out;
					}

					if (sunxi_image_part_verify(ih, (uint8_t *)loadaddr, (sunxi_tlv_header_t *)tlv)) {
						printf("error: %s new header verify failed\n", loadname);
						free(tlv);
						ret = -1;
						goto load_part_out;
					}

					printf("%s verify success!\n", loadname);
					free(tlv);
				}
			}
					if (strncmp(loadname, "arm-", strlen("arm-")) == 0) {
						*rtos_base = ih->ih_ep;
					} else if (strncmp(loadname, "rv-", strlen("rv-")) == 0) {
						*rv_base = ih->ih_ep;
					} else if (strncmp(loadname, "dsp-", strlen("dsp-")) == 0) {
						*dsp_base = ih->ih_ep;
					}
			}
		} else {
			printf("no %s part\n", loadname);
		}

		if ((loadparts != NULL) && (*loadparts != ':')) {
			printf("%d: not :\n", i);
			break;
		}

		if (loadparts != NULL)
			loadparts++;
	}
	printf("%s end\n", __func__);

	ret = 0;
load_part_out:
	free(sector_buf);
	return ret;
}

static int do_sunxi_boot_rtos(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	phys_addr_t rtos_base = 0, rv_base = 0, dsp_base = 0;
	void (*rtos_entry)(void);

	load_parts(&rtos_base, &rv_base, &dsp_base);
	rtos_entry = (void (*)(void))(rtos_base | 1);
	printf("rtos_base:%x \n", (unsigned int)rtos_entry);
	printf("jump to rtos!\n");
	rtos_entry();

	return 0;
}
#else

DECLARE_GLOBAL_DATA_PTR;

struct spare_rtos_head_t
{
	struct spare_boot_ctrl_head    boot_head;
	uint8_t   rotpk_hash[32];
	unsigned int rtos_dram_size;     /* rtos dram size, passed by uboot*/
};

static int do_sunxi_boot_rtos(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int src_addr = 0, dst_addr= 0;
	unsigned int src_len = 0, dst_len= 0;
	struct rtos_img_hdr *rtos_hdr;
	int ret = 0;
	void (*rtos_entry)(void);

	if(argc < 3) {
		printf("parameters error\n");
		return -1;
	}
	else {
		/* use argument only*/
		src_addr = simple_strtoul(argv[1], NULL, 16);
		dst_addr = simple_strtoul(argv[2], NULL, 16);
	}

	rtos_hdr=(struct rtos_img_hdr *)src_addr;

	if (memcmp(rtos_hdr->rtos_magic, RTOS_BOOT_MAGIC, 8)) {
		gd->debug_mode = 8;
		printf("error rtos header, reboot\n");
		reset_cpu(0);
	}

#ifdef CONFIG_SUNXI_SECURE_BOOT
	/* verify image before booting in secure boot*/
	if (gd->securemode) {
		printf("begin to verify rtos ...\n");
		ret = sunxi_verify_rtos(rtos_hdr);
		if (ret) {
			gd->debug_mode = 8;
			printf("verify rtos failed: %d, reboot\n", ret);
			reset_cpu(0);
		} else {
			printf("verify rtos success.\n");
		}
	}
#endif

	src_len = rtos_hdr->rtos_size;
	// in case of unaligned address
	dst_len |= *((unsigned char *)rtos_hdr + rtos_hdr->rtos_size + rtos_hdr->rtos_offset - 4);
	dst_len |=  (*((unsigned char *)rtos_hdr + rtos_hdr->rtos_size + rtos_hdr->rtos_offset - 3) << 8);
	dst_len |=  (*((unsigned char *)rtos_hdr + rtos_hdr->rtos_size + rtos_hdr->rtos_offset - 2) << 16);
	dst_len |=  (*((unsigned char *)rtos_hdr + rtos_hdr->rtos_size + rtos_hdr->rtos_offset - 1) << 24);

	ret = gunzip((void *)dst_addr, (int)dst_len,
			(unsigned char*)(src_addr + rtos_hdr->rtos_offset), (unsigned long*)&src_len);
	if (ret) {
		printf("Error uncompressing freertos-gz\n");
		return ret;
	}

	// prepare for rtos
	board_quiesce_devices();
	cleanup_before_linux();

	// save dram_size and rotpk_hash to rtos header
	if (!memcmp(((struct spare_rtos_head_t *)dst_addr)->boot_head.magic, "rtos", 4)) {
		((struct spare_rtos_head_t *)dst_addr)->rtos_dram_size = (unsigned int)uboot_spare_head.boot_data.dram_scan_size;
		memcpy(((struct spare_rtos_head_t *)dst_addr)->rotpk_hash, uboot_spare_head.rotpk, 32);
	}

	printf("jump to rtos!\n\n\n");
	rtos_entry = (void (*)(void))dst_addr;
	rtos_entry();

	return 0;
}

#endif

U_BOOT_CMD(
	boot_rtos,  3,  1,  do_sunxi_boot_rtos,
	"boot rtos",
	"rtos_gz_addr rtos_addr"
);
