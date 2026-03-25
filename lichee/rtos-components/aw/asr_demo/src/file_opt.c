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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int load_data_from_file(const char *file_path, unsigned char *data, int limit_size)
{
	int ret;
	unsigned int read_size = 0;
	unsigned int need_size = limit_size;
	int fd = open(file_path, O_RDONLY);

	if (fd < 0){
		printf("open %s failed - (%d)\n", file_path, fd);
		goto err;
	}

	while(ret = read(fd, &data[read_size], need_size)){
		if(ret<0){
			printf("read %s failed - (%d)\n", file_path, ret);
			goto err;
		}
		read_size += ret;
		need_size -= ret;
	}

	close(fd);
    printf("%s: file: %s, limit_size: %d, read_size: %d\n", __func__, file_path, limit_size, read_size);
	return read_size;
err:
	if(fd>=0)
		close(fd);
    printf("%s: failed! file: %s, limit_size: %d, read_size: %d, fd: %d\n", __func__, file_path, limit_size, read_size, fd);
	return -1;
}

int save_data_to_file(const char *file_path, const unsigned char *data, int size)
{
	//save
	int ret;
	int write_size = 0;
	int need_size = size;
	int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if (fd < 0){
		printf("open %s failed - (%d)\n", file_path, fd);
		goto err;
	}

	lseek(fd, 0, SEEK_SET);

	while( (need_size > 0) && (ret = write(fd, &data[write_size], need_size))) {
		if(ret < 0){
			printf("write %s failed - (%d)\n", file_path, ret);
			goto err;
		}
        write_size += ret;
        need_size -= ret;
	}

	close(fd);
    printf("%s: file: %s, write_size: %d\n", __func__, file_path, write_size);
	return write_size;
err:
	if(fd>=0)
		close(fd);
    printf("%s: failed! file: %s, size: %d, write_size: %d, fd: %d\n", __func__, file_path, size, write_size, fd);
	return -1;
}