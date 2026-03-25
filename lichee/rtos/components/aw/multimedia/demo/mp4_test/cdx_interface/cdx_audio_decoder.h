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

#ifndef CDX_CUSTOMER_DECODER
#define CDX_CUSTOMER_DECODER

#include "cedarx_include/ad_cedarlib_com.h"
#include "cedarx_include/AudioDec_Decode.h"

#define DEC_BUF_SIZE (6*1024)
#define PCM_BUF_SIZE (8*1024)
#define PTS_CHUNK_NUM 7

#define DBG_AAC_MARK 0

#define DM_LOG(fmt, arg...)  \
do { \
        printf("%20s(%4d):" fmt "\n", \
                __FUNCTION__, __LINE__, ##arg); \
} while (0)

enum LOAD_RAW_RESULT
{
    LOAD_RAW_OK,
    LOAD_RAW_SKIP,
    LOAD_RAW_END,
    LOAD_RAW_ERR,
};

struct CodecInfo
{
    struct __AudioDEC_AC320 *(*decInit)(void);
    int (*decExit)(struct __AudioDEC_AC320 *p);
};

struct DecoderLib
{
    pthread_mutex_t   mutex_audiodec_thread;
    int               nAudioInfoFlags;
    InputStreamBufT   Streambuffer;
    Ac320FileRead     dec_file_info;
    com_internal      internal;

    CedarAudioCodec   *codec;
    void              *libhandle;
    AudioDEC_AC320    *audio_dec;
    DecoderLibCfg     DecLibCfg;
    const char        *handlename;
};

struct DecStat
{
    uint32_t load_file_size;
    uint32_t store_pcm_size;
    uint32_t dec_start;
    uint32_t dec_end;
    uint32_t dec_time;
    uint32_t frame_cnt;
};

struct Decoder
{
    struct DecoderLib *dec_lib;
    struct DecStat dec_stat;
};

extern enum LOAD_RAW_RESULT CustomerDecoderBuffFill(unsigned char *input, unsigned in_size);
extern int Mp4AudioDecFrame(unsigned char *output, unsigned *out_size);
extern int VideoAudioDecInit(unsigned type);
extern void VideoAudioDecDeinit(void);
#endif
