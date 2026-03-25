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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cdx_video_dec.h"
#include "welsDecoderExt.h"

extern "C" {

// int lcnt = 0;

// void write_buffer2file(char *filename, unsigned char *buffer, int size) {
//     FILE *fd = fopen(filename,"wb");
//     if (NULL == fd) {
//         return;
//     }
//     fwrite(buffer,1,size,fd);
//     fclose(fd);
// }

// unsigned char liout[320 * 240 * 2] = {0};
// FILE *lfd = NULL;

static unsigned uiTimeStamp = 0;
ISVCDecoder *pDecoder = NULL;

void Write2Memory(unsigned char *output, unsigned char *pData[3], int iStride[2], int iWidth,
                  int iHeight)
{
    int i;
    int offset = 0;
    unsigned char *pPtr = NULL;

    pPtr = pData[0];
    for (i = 0; i < iHeight; i++) {
        // fwrite (pPtr, 1, iWidth, pFp);
        memcpy((output + offset), pPtr, iWidth);
        offset += iWidth;
        pPtr += iStride[0];
    }

    iHeight = iHeight / 2;
    iWidth = iWidth / 2;
    pPtr = pData[1];
    for (i = 0; i < iHeight; i++) {
        // fwrite (pPtr, 1, iWidth, pFp);
        memcpy((output + offset), pPtr, iWidth);
        offset += iWidth;
        pPtr += iStride[1];
    }

    pPtr = pData[2];
    for (i = 0; i < iHeight; i++) {
        memcpy((output + offset), pPtr, iWidth);
        offset += iWidth;
        pPtr += iStride[1];
    }
}

int Mp4VideoDecFrame(unsigned char *input, unsigned in_size, unsigned char *output)
{
    uint8_t *pData[3] = { NULL };
    SBufferInfo sDstBufInfo = { 0 };

    uiTimeStamp++;
    memset(&sDstBufInfo, 0, sizeof(SBufferInfo));
    sDstBufInfo.uiInBsTimeStamp = uiTimeStamp;

    pDecoder->DecodeFrameNoDelay(input, in_size, pData, &sDstBufInfo);

    if (sDstBufInfo.iBufferStatus == 1) {
        int iStride[2];
        int iWidth = sDstBufInfo.UsrData.sSystemBuffer.iWidth;
        int iHeight = sDstBufInfo.UsrData.sSystemBuffer.iHeight;
        iStride[0] = sDstBufInfo.UsrData.sSystemBuffer.iStride[0];
        iStride[1] = sDstBufInfo.UsrData.sSystemBuffer.iStride[1];
        pData[0] = sDstBufInfo.pDst[0];
        pData[1] = sDstBufInfo.pDst[1];
        pData[2] = sDstBufInfo.pDst[2];
        Write2Memory(output, (unsigned char **)pData, iStride, iWidth, iHeight);
        return 0;
        //   pDst[0] = sDstBufInfo.pDst[0];
        //   pDst[1] = sDstBufInfo.pDst[1];
        //   pDst[2] = sDstBufInfo.pDst[2];
    }
    return -1;
}

int VideoDecInit(unsigned width, unsigned higth)
{
    if (WelsCreateDecoder(&pDecoder) || (NULL == pDecoder)) {
        printf("Create Decoder failed.\n");
        return -1;
    }

    SDecodingParam sDecParam = { 0 };
    sDecParam.uiTargetDqLayer = (uint8_t)-1;
    sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
    if (pDecoder->Initialize(&sDecParam)) {
        printf("Decoder initialization failed.\n");
        return -1;
    }

    return 0;
}

void VideoDecDeinit(void)
{
    if (pDecoder) {
        pDecoder->Uninitialize();

        WelsDestroyDecoder(pDecoder);
        pDecoder = NULL;
    }
}
}