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

#ifndef CDX_PARSER_HJY_H
#define CDX_PARSER_HJY_H

#ifdef CONFIG_ARCH_SUN20IW2

struct sample_map_info {
    // uint64_t offset;
    unsigned int size;
/**
 * priv
 * for video，used for I frame
 * for aidio, used for audio duration
*/
#define VIDEO_IS_IFRAME (1 << 4 | 1 << 0)
    unsigned short priv;
#define SAMPLE_IS_VIDEO 1
#define SAMPLE_IS_AUDIO 2
    unsigned char type;
};

typedef struct _Mp4_Info {
    unsigned long start_offset;

#define VIDEO_TYPR_MPEG4 0
#define VIDEO_TYPR_H264 1
#define VIDEO_NOT_SUPPORT 255
    unsigned VideoType;
    unsigned Timescale;
    unsigned Width;
    unsigned Height;

    unsigned DinfoSize;
    unsigned char *Dinfo;

#define AUDIO_TYPE_MP3 0
#define AUDIO_TYPE_AAC 1
#define AUDIO_NOT_SUPPORT 255
    unsigned AudioType;
    unsigned SampleRate;
    unsigned Channels;
    unsigned Bits;

    unsigned AinfoSize;
    unsigned char *Ainfo;

    // struct stsd_info vstsd;
    // struct stsd_info astsd;

    struct sample_map_info *sample_map;
    unsigned sample_count;
    unsigned max_video_size;
    unsigned max_audio_size;

    unsigned Vduration;
    unsigned Aduration;

    unsigned first_flag; //use for h264 first play
} VideoInfo;

#endif /* CONFIG_ARCH_SUN20IW2 */
#endif /* CDX_PARSER_HJY_H */