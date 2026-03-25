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

#include <stdarg.h>
#include <stdbool.h>
#include <getopt.h>
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "dsp_head.h"
#include "dsp_elf.h"

#define a_argument required_argument
static const char usage_short_opts[] = "S:D:@";
static struct option const usage_long_opts[] = {
	{"src-bin",	a_argument, NULL, 'S'},
	{"dest-bin",	a_argument, NULL, 'D'},
	{"symbols",	no_argument, NULL, '@'},
	{NULL,		no_argument, NULL, 0x0},
};
#define util_getopt_long() getopt_long(argc, argv, usage_short_opts, \
				       usage_long_opts, NULL)

int main(int argc, char *argv[])
{
	FILE* fd;
	unsigned int img_len = 0;
	const char *src_path = NULL;
	const char *dest_path = NULL;
	int opt;
	int i = 0;
	unsigned long change_addr = 0;
	unsigned char *strtab = 0;

	while ((opt = util_getopt_long()) != EOF) {
		switch (opt) {
		case 'S':
			src_path = optarg;
			printf("src dsp bin path = %s\n",src_path);
			break;
		case 'D':
			dest_path = optarg;
			printf("dest dsp bin path = %s\n",dest_path);
			break;
		default:
			printf("argv err \n");
			goto err0;
		}
	}

	if(src_path == NULL){
		printf("get src dsp bin path failed! \n");
		goto err0;
	}

	if(dest_path == NULL){
		printf("get dest dsp bin path failed! \n");
		goto err0;
	}

	/* open dsp bin */
	fd = fopen(src_path, "rb+");
	if(fd == NULL){
		printf("file %s open failed! \n", src_path);
		goto err1;
	}
	/* get dsp bin length*/
	fseek(fd,0L,SEEK_END);
	img_len = ftell(fd);
	printf("dsp.bin length = %d \n", img_len);

	/* read dsp bin */
	char *pImg = NULL;
	pImg = (char *)malloc(img_len);
	if(pImg == NULL){
		printf("img malloc failed! \n");
		goto err2;
	}
	fseek(fd,0L,SEEK_SET);
	fread(pImg,img_len,1,fd);

	Elf32_Ehdr *ehdr = NULL; /* Elf header structure pointer */
	Elf32_Phdr *phdr = NULL; /* Program header structure pointer */
	Elf32_Shdr *shdr = NULL;
	ehdr = (Elf32_Ehdr *)pImg;
	phdr = (Elf32_Phdr *)(pImg + ehdr->e_phoff);
	shdr =  (Elf32_Shdr *)(pImg + ehdr->e_shoff + (ehdr->e_shstrndx * sizeof(Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *)(pImg + shdr->sh_offset);

	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *)(pImg + ehdr->e_shoff +
				     (i * sizeof(Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC) ||
		    shdr->sh_addr == 0 || shdr->sh_size == 0) {
			continue;
		}

		if(strtab){

			if(strcmp((char*)&strtab[shdr->sh_name],".oemhead.text") == 0)
			{
				printf("%sing %s @ 0x%08lx (%ld bytes)\n",
					  (shdr->sh_type == SHT_NOBITS) ? "Clear" : "Load",
					   &strtab[shdr->sh_name],
					   (unsigned long)shdr->sh_addr,
					   (long)shdr->sh_size);
				change_addr = shdr->sh_addr;
				break;
			}
		}
	}

	/* no find .oemhead.text */
	if (change_addr == 0) {
		printf("change_addr 0x%lx , value failed !!! \n",change_addr);
		goto err2;
	}

	/* create new dsp bin */
	FILE* fd_img;
	fd_img = fopen(dest_path, "wb+");
	if(fd_img == NULL){
		printf("file %s open failed! \n", dest_path);
		goto err3;
	}

	/* wirte image length to bin */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		if(change_addr == (unsigned long)phdr->p_paddr)
		{
			struct spare_rtos_head_t *prtos = (struct spare_rtos_head_t *)(pImg + phdr->p_offset);
			prtos->rtos_img_hdr.image_size = img_len;
			printf("write image_size = %d\n", prtos->rtos_img_hdr.image_size);
			break;
		}
		++phdr;
	}


	/* write data to new dsp bin */
	fseek(fd_img,0L,SEEK_SET);
	fwrite((void*)pImg,img_len,1,fd_img);
	printf("create dsp bin ok ..\n");

err3:
	fclose(fd_img);
err2:
	free(pImg);
err1:
	fclose(fd);
err0:
	return 0;
}
