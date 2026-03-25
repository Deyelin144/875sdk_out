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
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <turbojpeg.h>
#include "console.h"

#ifdef CONFIG_COMPONENTS_RPDATA
#include <rpdata.h>
#include <hal_thread.h>
#include <string.h>
#if defined(CONFIG_ARCH_DSP)
#define DSP_RP_DIR_CORE  2
#define DSP_RP_DATA_TYPE "jpeg"
#define DSP_RP_DATA_SEND "recv"
#define DSP_RP_DATA_RECV "send"
#define JPEG_WIDTH  640
#define JPEG_HEIGHT 480
#else
#define RV_RP_DIR_CORE  3
#define RV_RP_DATA_TYPE "jpeg"
#define RV_RP_DATA_SEND "send"
#define RV_RP_DATA_RECV "recv"
#endif
#endif
typedef unsigned char uchar;

typedef struct tjp_info {
  int outwidth;
  int outheight;
  unsigned long jpg_size;
  void *jpg_buf;
}tjp_info_t;

// RGB与BGR格式文件转换
void reverseRGB(unsigned char* data, int size)
{
    unsigned char *p = data;
    int i;

    for (i = 0; i < size / 3 ; i++) {
        unsigned char tmp = p[0];
        p[0] = p[2];
        p[2] = tmp;
        p += 3;
    }
}

/*获取当前ms数*/
#if defined(CONFIG_ARCH_DSP)
static int get_timer_now ()
{
  return (int)xTaskGetTickCount();
}
#else
static int get_timer_now ()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return(now.tv_sec * 1000 + now.tv_usec / 1000);
}
#endif

/*读取文件到内存*/
uchar *read_file2buffer(char *filepath, tjp_info_t *tinfo)
{
  FILE *fd;
  struct stat fileinfo;
  stat(filepath,&fileinfo);
  tinfo->jpg_size = fileinfo.st_size;

  fd = fopen(filepath,"rb");
  if (NULL == fd) {
    printf("file not open\n");
    return NULL;
  }
  uchar *data = (uchar *)malloc(sizeof(uchar) * fileinfo.st_size);
  fread(data,1,fileinfo.st_size,fd);
  fclose(fd);
  return data;
}

/*写内存到文件*/
void write_buffer2file(char *filename, uchar *buffer, int size)
{
  FILE *fd = fopen(filename,"wb");
  if (NULL == fd) {
    return;
  }
  fwrite(buffer,1,size,fd);
  fclose(fd);
}

/*图片解压缩*/
uchar *tjpeg_decompress(uchar *jpg_buffer, tjp_info_t *tinfo)
{
  tjhandle handle = NULL;
  int img_width,img_height,img_subsamp,img_colorspace;
  int flags = 0, pixelfmt = TJPF_RGB;
  /*创建一个turbojpeg句柄*/
  handle = tjInitDecompress();
  if (NULL == handle)  {
    return NULL;
  }
  /*获取jpg图片相关信息但并不解压缩*/
  int ret = tjDecompressHeader3(handle,jpg_buffer,tinfo->jpg_size,&img_width,&img_height,&img_subsamp,&img_colorspace);
  if (0 != ret) {
    tjDestroy(handle);
    return NULL;
  }
  /*输出图片信息*/
  printf("jpeg width:%d\n",img_width);
  printf("jpeg height:%d\n",img_height);
  printf("jpeg subsamp:%d\n",img_subsamp);
  printf("jpeg colorspace:%d\n",img_colorspace);
  /*计算1/4缩放后的图像大小,若不缩放，那么直接将上面的尺寸赋值给输出尺寸*/
  tjscalingfactor sf;
  sf.num = 1;
  sf.denom = 1;
  tinfo->outwidth = TJSCALED(img_width, sf);
  tinfo->outheight = TJSCALED(img_height, sf);
  printf("w:%d,h:%d\n",tinfo->outwidth,tinfo->outheight);
  /*解压缩时，tjDecompress2（）会自动根据设置的大小进行缩放，但是设置的大小要在它的支持范围，如1/2 1/4等*/
  flags |= 0;
  int size = tinfo->outwidth * tinfo->outheight * 3;
  uchar *rgb_buffer = (uchar *)malloc(sizeof(uchar) * size);
  ret = tjDecompress2(handle, jpg_buffer, tinfo->jpg_size, rgb_buffer, tinfo->outwidth, 0,tinfo->outheight, pixelfmt, flags);
  reverseRGB(rgb_buffer, size);//大端转化为小端
  char *outfile1 = "./data/tjtest.rgb";
  write_buffer2file(outfile1, rgb_buffer, size);
  reverseRGB(rgb_buffer, size);//小端转化回大端进行压缩。
  if (0 != ret) {
    tjDestroy(handle);
    return NULL;
  }
  tjDestroy(handle);
  return rgb_buffer;
}

/*压缩图片*/
int tjpeg_compress(uchar *rgb_buffer, tjp_info_t *tinfo, int quality, uchar **outjpg_buffer, unsigned long *outjpg_size)
{
  tjhandle handle = NULL;
  int flags = 0;
  int subsamp = TJSAMP_422;
  int pixelfmt = TJPF_RGB;
  /*创建一个turbojpeg句柄*/
  handle=tjInitCompress();
  if (NULL == handle) {
    return -1;
  }
  /*将rgb图或灰度图等压缩成jpeg格式图片*/
  int ret = tjCompress2(handle, rgb_buffer,tinfo->outwidth,0,tinfo->outheight,pixelfmt,outjpg_buffer,outjpg_size,subsamp,quality, flags);
  if (0 != ret) {
    tjDestroy(handle);
    return -1;
  }
  tjDestroy(handle);
  return 0;
}

/*测试程序*/
int tj_test()
{
  tjp_info_t tinfo;
    char *filename = "./data/test.jpg";
    int start = get_timer_now();
  /*读图像*/
    uchar *jpeg_buffer = read_file2buffer(filename,&tinfo);
    int rend = get_timer_now();
    printf("loadfile make time:%d\n",rend-start);
    if (NULL == jpeg_buffer) {
        printf("read file failed\n");
        return 1;
    }

    int dstart = get_timer_now();
  /*解压缩*/
    uchar *rgb = tjpeg_decompress(jpeg_buffer,&tinfo);
	if (NULL == rgb) {
        printf("error\n");
    free(jpeg_buffer);
    return -1;
    }
    int dend = get_timer_now();
    printf("decompress make time:%d\n",dend-dstart);
    uchar *outjpeg=NULL;
    unsigned long outjpegsize;
    int cstart = get_timer_now();
  /*压缩*/
    tjpeg_compress(rgb,&tinfo,80,&outjpeg,&outjpegsize);
    printf("out jpeg size = %lu\n",outjpegsize);
    int cend = get_timer_now();
    printf("compress make time:%d\n",cend-cstart);
    char *outfile = "./data/tjtest.jpg";
    int wstart = get_timer_now();
    write_buffer2file(outfile,outjpeg,outjpegsize);
    int wend = get_timer_now();
    printf("write file make time:%d\n",wend-wstart);
    int end = get_timer_now();
    printf("tj all make time:%d\n",end-start);
  free(jpeg_buffer);
  free(rgb);
  return 0;
}

int tj_test_main(int argc, char **argv)
{
    return tj_test();
}

FINSH_FUNCTION_EXPORT_CMD(tj_test_main, tjtest, tjtest);

#ifdef CONFIG_COMPONENTS_RPDATA
#ifdef CONFIG_ARCH_DSP
void jpeg_decode_thread(void *param)
{
    rpdata_t *rpd_recv = NULL;
    rpdata_t *rpd_send = NULL;
    unsigned char *rx_buf = NULL;
    unsigned char *rgb_buf = NULL;

    int size = JPEG_WIDTH*JPEG_HEIGHT*3;
    int times = strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0);

    while(times)
    {
        times--;
        unsigned char *tmp_rgb_buffer = (unsigned char *)malloc(sizeof(unsigned char) * size + 1);

        int recv_len = 0;
        tjhandle handle = NULL;
        handle = tjInitDecompress();
        if (handle == NULL) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }

        // 连接到远端核
        rpd_recv = rpdata_connect(DSP_RP_DIR_CORE, DSP_RP_DATA_TYPE, DSP_RP_DATA_RECV);
        if (!rpd_recv) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }
        // 获取到远端核的数据长度
        recv_len = rpdata_buffer_len(rpd_recv);
        if (recv_len <= 0) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }
        // 创建buff，用于缓存远端核的数据
        rx_buf = malloc(recv_len + 1);
        if (rx_buf == NULL) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }
        // 创建一个远程连接用于返回
        rpd_send = rpdata_create(DSP_RP_DIR_CORE, DSP_RP_DATA_TYPE, DSP_RP_DATA_SEND, JPEG_WIDTH*JPEG_HEIGHT*3);
        if (!rpd_send) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }
        // 获取返回数据的buff
        rgb_buf = rpdata_buffer_addr(rpd_send);
        if (rgb_buf == NULL) {
            printf("error[%s] line:%d \n", __func__, __LINE__);
        }
        // 接收到远端数据
        int ret = rpdata_recv(rpd_recv, rx_buf, recv_len, -1);
        if (ret <= 0) {
            printf("rpdata recv timeout \n");
            while(1){}
        }
        // 处理远端数据
        tjDecompress2(handle, rx_buf, recv_len, tmp_rgb_buffer, JPEG_WIDTH, 0, JPEG_HEIGHT, TJPF_RGB, TJFLAG_FASTUPSAMPLE);

        memcpy(rgb_buf, tmp_rgb_buffer, JPEG_WIDTH*JPEG_HEIGHT*3);

        // 返回数据
        if (rpdata_send(rpd_send, 0, JPEG_WIDTH*JPEG_HEIGHT*3) < 0) {
            printf("error[%s : %d]\n", __FUNCTION__, __LINE__);
            while(1){}
        }

        if (handle != NULL) {
            tjDestroy(handle);
        }

        if (rpd_recv != NULL) {
            rpdata_destroy(rpd_recv);
        }

        if (rx_buf != NULL) {
            free(rx_buf);
        }

        if (tmp_rgb_buffer != NULL) {
            free(tmp_rgb_buffer);
        }

        if (rpd_send != NULL) {
            rpdata_destroy(rpd_send);
        }
    }
    hal_thread_stop(NULL);
}

int run_dsp_jpeg_decode_task(void)
{
        hal_thread_t handle;

        handle = hal_thread_create(jpeg_decode_thread, NULL, "jpeg decode thread", 4096, HAL_THREAD_PRIORITY_APP);

        if(handle != NULL)
                hal_thread_start(handle);
        else
                printf("fail to create jpeg decode thread\r\n");

        return 0;
}
#else
void process_jpg_data_thread(void *param)
{
    tjp_info_t tinfo;
    int jpg_size;
    int times = strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0);
    int timediff = 0;
    int time_count = 0;

    memcpy(&tinfo, param, sizeof(tjp_info_t));
    jpg_size = tinfo.outwidth * tinfo.outheight * 3;

    while(times)
    {
        times--;
        uchar *rgb_buffer = (uchar *)malloc(sizeof(uchar) * jpg_size);

        // 交由dsp处理
        // 创建远程连接
        rpdata_t *rpd_send = rpdata_create(RV_RP_DIR_CORE, RV_RP_DATA_TYPE, RV_RP_DATA_SEND, tinfo.jpg_size);
        // 获取数据传输buff
        uchar *f_jpeg_buff = rpdata_buffer_addr(rpd_send);
        if (f_jpeg_buff == NULL) {
            printf("error[%s : %d]\n", __FUNCTION__, __LINE__);
            while (1) {
            }
        }
        // 填充需要传输的数据

        memcpy(f_jpeg_buff, tinfo.jpg_buf, tinfo.jpg_size);

        // 等待远端核连接成功
        rpdata_wait_connect(rpd_send);
        // 连接成功，发送数据到远端核
        if (rpdata_send(rpd_send, 0, tinfo.jpg_size) < 0) {
            printf("error[%s : %d]\n", __FUNCTION__, __LINE__);
            while (1) {
            }
        }
        //printf("jpeg data send finish, f_jpeg_buff = %p, jpg_buffer = %p\n", f_jpeg_buff, tinfo.jpg_buf);

        // 主动连接到远端核
        rpdata_t *rpd_recv = rpdata_connect(RV_RP_DIR_CORE, RV_RP_DATA_TYPE, RV_RP_DATA_RECV);
        rpdata_wait_connect(rpd_recv);
        //printf("*************get decode start************\n");
        int t_dec_s = get_timer_now();
        // 接收远端数据
        rpdata_recv(rpd_recv, rgb_buffer, jpg_size, -1);
        int t_dec_e = get_timer_now();
        timediff = t_dec_e - t_dec_s;
        time_count +=timediff;
        printf("*************decode jpg[%d] cov timediff : %dms************\n", (strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0)-times), timediff);
        reverseRGB(rgb_buffer, jpg_size);//大端转化为小端
        char outfile1[32];
        snprintf(outfile1, sizeof(outfile1), "./data/tjtest_%d.rgb", (strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0)-times));

        //printf("*************rgb path: %s, rgb_buffer = %p , size = %d************\n", outfile1, rgb_buffer, jpg_size);

        write_buffer2file(outfile1, rgb_buffer, jpg_size);

        if (rpd_send != NULL) {
            rpdata_destroy(rpd_send);
        }

        if (rpd_recv != NULL) {
            rpdata_destroy(rpd_recv);
        }

        if(rgb_buffer != NULL)
        {
            free(rgb_buffer);
        }
    }
    printf("*************decode %d times, average timediff : %.2fms************\n", strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0), ((float)time_count/strtol(CONFIG_RPDATA_JPEG_LOOP_TIMES, NULL, 0)));
    hal_thread_stop(NULL);
}

int run_rv_process_jpg_data_task(tjp_info_t tinfo)
{
        hal_thread_t handle;

        handle = hal_thread_create(process_jpg_data_thread, &tinfo, "process jpg data thread", 4096, HAL_THREAD_PRIORITY_APP);

        if(handle != NULL)
                hal_thread_start(handle);
        else
                printf("fail to create process jpg data thread\r\n");

        return 0;
}

static int get_jpg_info(uchar *jpg_buffer, tjp_info_t *tinfo)
{
    tjhandle handle = NULL;
    int img_width,img_height,img_subsamp,img_colorspace;
    handle = tjInitDecompress();
    if (NULL == handle)  {
      return -1;
    }
    int ret = tjDecompressHeader3(handle,jpg_buffer,tinfo->jpg_size,&img_width,&img_height,&img_subsamp,&img_colorspace);
    if (0 != ret) {
      tjDestroy(handle);
      return -1;
    }
    printf("jpeg tinfo.jpg_size:%lu\n",tinfo->jpg_size);
    printf("jpeg width:%d\n",img_width);
    printf("jpeg height:%d\n",img_height);
    printf("jpeg subsamp:%d\n",img_subsamp);
    printf("jpeg colorspace:%d\n",img_colorspace);
    tjscalingfactor sf;
    sf.num = 1;
    sf.denom = 1;
    tinfo->outwidth = TJSCALED(img_width, sf);
    tinfo->outheight = TJSCALED(img_height, sf);
    printf("w:%d,h:%d\n",tinfo->outwidth,tinfo->outheight);
    tjDestroy(handle);
    return 0;
}
#endif

int cmd_rpdata_jpg(int argc, char **argv)
{
#ifndef CONFIG_ARCH_DSP
    tjp_info_t tinfo;
    char *filename = "./data/480p.jpg";
    uchar *jpeg_buffer = read_file2buffer(filename,&tinfo);
    if (NULL == jpeg_buffer) {
        printf("read file failed\n");
        return 1;
    }
    get_jpg_info(jpeg_buffer, &tinfo);

    tinfo.jpg_buf = jpeg_buffer;

    run_rv_process_jpg_data_task(tinfo);

    if(jpeg_buffer != NULL)
    {
        free(jpeg_buffer);
    }
#else
    run_dsp_jpeg_decode_task();
#endif
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_rpdata_jpg, rpdata_jpg, rpdata_jpg);
#endif


