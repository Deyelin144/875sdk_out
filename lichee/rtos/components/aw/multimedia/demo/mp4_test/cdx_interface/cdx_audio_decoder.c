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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdx_audio_decoder.h"

extern struct __AudioDEC_AC320 *AudioMP3DecInit(void);
extern int AudioMP3DecExit(struct __AudioDEC_AC320 *p);

static const struct CodecInfo Mp3Dec = {
    .decInit = AudioMP3DecInit,
    .decExit = AudioMP3DecExit,
};

extern struct __AudioDEC_AC320 *AudioAACDecInit(void);
extern int AudioAACDecExit(struct __AudioDEC_AC320 *p);

static const struct CodecInfo AacDec = {
    .decInit = AudioAACDecInit,
    .decExit = AudioAACDecExit,
};

const struct CodecInfo *CustomerDec[] = {
    &Mp3Dec,
    &AacDec,
};

static struct DecoderLib *DecLibInit(const struct CodecInfo *codec_info)
{
    struct DecoderLib *p_dec_lib = NULL;

    p_dec_lib = malloc(sizeof(struct DecoderLib));
    if (p_dec_lib == NULL)
    {
        DM_LOG("malloc dec_lib fail.\n");
        goto err;
    }

    memset(p_dec_lib, 0, sizeof(struct DecoderLib));

    pthread_mutex_init(&p_dec_lib->mutex_audiodec_thread, NULL);

    p_dec_lib->codec = (CedarAudioCodec*)malloc(sizeof(CedarAudioCodec));
    if (p_dec_lib->codec == NULL)
    {
        DM_LOG("malloc codec fail.\n");
        goto err;
    }

    p_dec_lib->codec->init = codec_info->decInit;
    p_dec_lib->codec->exit = codec_info->decExit;

    p_dec_lib->dec_file_info.bufStart = (unsigned char *)malloc(DEC_BUF_SIZE);
    if (p_dec_lib->dec_file_info.bufStart == NULL)
    {
        DM_LOG("malloc dec buf fail.\n");
        goto err;
    }

    p_dec_lib->dec_file_info.frmFifo.inFrames =
                    (aframe_info_t *)malloc(PTS_CHUNK_NUM * sizeof(aframe_info_t));
    if (p_dec_lib->dec_file_info.frmFifo.inFrames == NULL)
    {
        DM_LOG("malloc dec buf frmFifo inFrames fail.\n");
        goto err;
    }

    memset(p_dec_lib->dec_file_info.frmFifo.inFrames, 0, PTS_CHUNK_NUM * sizeof(aframe_info_t));
    p_dec_lib->dec_file_info.frmFifo.maxchunkNum = PTS_CHUNK_NUM;
    p_dec_lib->dec_file_info.frmFifo.dLastValidPTS = -1;

    p_dec_lib->dec_file_info.BufToTalLen = DEC_BUF_SIZE;
    p_dec_lib->dec_file_info.bufWritingPtr = p_dec_lib->dec_file_info.bufStart;
    p_dec_lib->dec_file_info.BufValideLen = p_dec_lib->dec_file_info.BufToTalLen;
    p_dec_lib->dec_file_info.bufReadingPtr = p_dec_lib->dec_file_info.bufStart;
    p_dec_lib->dec_file_info.BufLen = 0;
    p_dec_lib->dec_file_info.BigENDFlag = 1;
    p_dec_lib->dec_file_info.FileLength = 0x7fffffff;
	p_dec_lib->dec_file_info.tmpGlobalAudioDecData = (void*)p_dec_lib;

    p_dec_lib->audio_dec = p_dec_lib->codec->init();
    if (p_dec_lib->audio_dec == NULL)
    {
        DM_LOG("codec init.\n");
        goto err;
    }

    p_dec_lib->audio_dec->FileReadInfo = &p_dec_lib->dec_file_info;
    p_dec_lib->audio_dec->DecoderCommand = &p_dec_lib->internal;
    p_dec_lib->audio_dec->DecoderCommand->ulExitMean = 1;

    p_dec_lib->audio_dec->BsInformation = malloc(sizeof(BsInFor));
    if (p_dec_lib->audio_dec->BsInformation == NULL)
    {
        DM_LOG("BsInformation malloc fail\n");
        goto err;
    }
    memset(p_dec_lib->audio_dec->BsInformation, 0 , sizeof(BsInFor));

    p_dec_lib->audio_dec->DecInit(p_dec_lib->audio_dec);

    return p_dec_lib;

err:
    if (p_dec_lib)
    {
        if (p_dec_lib->audio_dec)
        {
            p_dec_lib->audio_dec->DecExit(p_dec_lib->audio_dec);
            p_dec_lib->codec->exit(p_dec_lib->audio_dec);
            free(p_dec_lib->audio_dec->BsInformation);
        }
        free(p_dec_lib->codec);
        free(p_dec_lib->dec_file_info.bufStart);
        free(p_dec_lib->dec_file_info.frmFifo.inFrames);
    }
    free(p_dec_lib);
    return NULL;
}

static int DecLibExit(struct DecoderLib *p_dec_lib)
{
    if (p_dec_lib)
    {
        if (p_dec_lib->audio_dec)
        {
            p_dec_lib->audio_dec->DecExit(p_dec_lib->audio_dec);
            p_dec_lib->codec->exit(p_dec_lib->audio_dec);
            free(p_dec_lib->audio_dec->BsInformation);
        }
        pthread_mutex_destroy(&p_dec_lib->mutex_audiodec_thread);
        free(p_dec_lib->codec);
        free(p_dec_lib->dec_file_info.bufStart);
        free(p_dec_lib->dec_file_info.frmFifo.inFrames);
    }
    free(p_dec_lib);

    return 0;
}

static struct Decoder *DecInit(const struct CodecInfo *codec_info)
{
    struct Decoder *p_dec = NULL;

    p_dec = (struct Decoder *)malloc(sizeof(struct Decoder));

    if (p_dec == NULL)
    {
        DM_LOG("struct Decoder malloc fail.\n");
        goto err;
    }

    memset(p_dec, 0, sizeof(struct Decoder));

    p_dec->dec_lib = DecLibInit(codec_info);
    if (p_dec->dec_lib == NULL)
    {
        DM_LOG("dec lib init fail.\n");
        goto err;
    }
    return p_dec;

err:
    if (p_dec)
    {
        DecLibExit(p_dec->dec_lib);
    }
    free(p_dec);
    return NULL;
}

static int DecExit(struct Decoder *p_dec)
{
    if (p_dec)
    {
        DecLibExit(p_dec->dec_lib);
    }
    free(p_dec);
    return 0;
}

static int RequestDecBuf(struct DecoderLib *p_dec_lib, int req_size,
                        unsigned char **pp_buf, int *p_buf_size,
                        unsigned char **pp_ring_buf, int *p_ring_buf_size)
{
    int len_to_end = p_dec_lib->dec_file_info.bufStart
                    + p_dec_lib->dec_file_info.BufToTalLen
                    - p_dec_lib->dec_file_info.bufWritingPtr;

    *pp_buf = p_dec_lib->dec_file_info.bufWritingPtr;
    if (req_size >= len_to_end)
    {
        *p_buf_size = len_to_end;
        *pp_ring_buf = p_dec_lib->dec_file_info.bufStart;
        *p_ring_buf_size = req_size - len_to_end;
    }
    else
    {
        *p_buf_size = req_size;
        *pp_ring_buf = NULL;
        *p_ring_buf_size = 0;
    }

    return 0;
}

static int UpdateDecBuf(struct DecoderLib *p_dec_lib, int filled_len)
{
    astream_fifo_t *stream = &p_dec_lib->dec_file_info.frmFifo;

    if (filled_len <= 0)
    {
        DM_LOG("filled_len invalid: %d.\n", filled_len);
        return -1;
    }

    if(p_dec_lib->dec_file_info.bufWritingPtr + filled_len
        >= p_dec_lib->dec_file_info.bufStart + p_dec_lib->dec_file_info.BufToTalLen)
    {
        p_dec_lib->dec_file_info.bufWritingPtr = p_dec_lib->dec_file_info.bufWritingPtr
                            + filled_len - p_dec_lib->dec_file_info.BufToTalLen;
    }
    else
    {
        p_dec_lib->dec_file_info.bufWritingPtr += filled_len;
    }
    p_dec_lib->dec_file_info.BufLen += filled_len;
    p_dec_lib->dec_file_info.BufValideLen -= filled_len;

    memset(&stream->inFrames[stream->wtIdx],0,sizeof(aframe_info_t));
    stream->inFrames[stream->wtIdx].pts += 1;
    stream->inFrames[stream->wtIdx].len = filled_len;
    stream->inFrames[stream->wtIdx].ptsValid = 1;
    if(stream->inFrames[stream->wtIdx].ptsValid)
    {
       stream->dLastValidPTS = stream->inFrames[stream->wtIdx].pts;
    }
    stream->wtIdx++;
    if(stream->wtIdx >= PTS_CHUNK_NUM)
    {
        stream->wtIdx =0;
    }
    stream->ValidchunkCnt++;

    return 0;
}

static struct Decoder *g_dec = NULL;

enum LOAD_RAW_RESULT CustomerDecoderBuffFill(unsigned char *input, unsigned in_size)
{
    unsigned char *p_buf0 = NULL;
    unsigned char *p_buf1 = NULL;
    int buf0_size = 0;
    int buf1_size = 0;

    enum LOAD_RAW_RESULT load_raw_res = LOAD_RAW_OK;

    if (g_dec->dec_lib->dec_file_info.BufValideLen < in_size) {
        return LOAD_RAW_SKIP;
    }

    /* fill dec buf */
    if (RequestDecBuf(g_dec->dec_lib, in_size, &p_buf0, &buf0_size, &p_buf1, &buf1_size)) {
        DM_LOG("request dec buf fail: %p(%d) %p(%d).\n", p_buf0, buf0_size, p_buf1, buf1_size);
        return LOAD_RAW_ERR;
    }

    if ((buf0_size + buf1_size) < in_size) {
        DM_LOG("request dec buf size fail: %p(%d) %p(%d).\n",
            p_buf0, buf0_size, p_buf1, buf1_size);
        return LOAD_RAW_ERR;
    }

    if (buf0_size == in_size) {
        memcpy(p_buf0, input, in_size);
    } else if (buf0_size < in_size) {
        memcpy(p_buf0, input, buf0_size);
        memcpy(p_buf1, (input + buf0_size), buf1_size);
    } else {
        return LOAD_RAW_ERR;
    }

    if (UpdateDecBuf(g_dec->dec_lib, in_size)) {
        DM_LOG("update dec buf fail.\n");
        return LOAD_RAW_ERR;
    }
    return LOAD_RAW_OK;
}

int Mp4AudioDecFrame(unsigned char *output, unsigned *out_size)
{
    if (g_dec == NULL)
        return -1;

    unsigned pcm_size = 0;
    int dec_res = ERR_AUDIO_DEC_NONE;
    *out_size = 0;

    do {
        pcm_size = 0;
        dec_res = g_dec->dec_lib->audio_dec->DecFrame(g_dec->dec_lib->audio_dec, (output + *out_size), &pcm_size);

        if ((dec_res != ERR_AUDIO_DEC_NONE)
            && (dec_res != ERR_AUDIO_DEC_ABSEND)
            && (dec_res != ERR_AUDIO_DEC_NO_BITSTREAM))
        {
            printf("decode frame fail:%d\n", dec_res);
        }

        *out_size += pcm_size;
    } while ((dec_res != ERR_AUDIO_DEC_ABSEND) && (dec_res != ERR_AUDIO_DEC_NO_BITSTREAM));

    return 0;
}

int VideoAudioDecInit(unsigned type)
{
    g_dec = DecInit(CustomerDec[type]);
    if (g_dec == NULL) {
        return -1;
    }
    return 0;
}

void VideoAudioDecDeinit(void)
{
    if (g_dec == NULL)
        return ;

    DecExit(g_dec);
    g_dec = NULL;
}
