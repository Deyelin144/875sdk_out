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

#include "xvid.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cdx_video_dec.h"

static void *g_xvid_handle = NULL;
static unsigned g_width_line = 0;
/* decode one frame  */
static int dec_xvid_main(unsigned char *istream, unsigned char *ostream, int istream_size,
                         xvid_dec_stats_t *xvid_dec_stats)
{
    xvid_dec_frame_t xvid_dec_frame;

    /* Reset all structures */
    memset(&xvid_dec_frame, 0, sizeof(xvid_dec_frame_t));
    memset(xvid_dec_stats, 0, sizeof(xvid_dec_stats_t));

    /* Set version */
    xvid_dec_frame.version = XVID_VERSION;
    xvid_dec_stats->version = XVID_VERSION;

    /* No general flags to set */

    xvid_dec_frame.general = 0;
    xvid_dec_frame.general |= XVID_LOWDELAY;

    /* Input stream */
    xvid_dec_frame.bitstream = istream;
    xvid_dec_frame.length = istream_size;

    /* Output frame structure */
    xvid_dec_frame.output.plane[0] = ostream;
    /* note 16bit bpp = 2; 12bit bpp = 1 */
    xvid_dec_frame.output.stride[0] = g_width_line;
    xvid_dec_frame.output.csp = MP4_DEC_OUT_FORMAT;

    return xvid_decore(g_xvid_handle, XVID_DEC_DECODE, &xvid_dec_frame, xvid_dec_stats);
}

int Mp4VideoDecFrame(unsigned char *input, unsigned in_size, unsigned char *output)
{
    unsigned char *mp4_ptr = input;
    int useful_bytes = in_size;
    int used_bytes = 0;
    xvid_dec_stats_t xvid_dec_stats;
    int zero_times = 10;

    do {
        used_bytes = dec_xvid_main(mp4_ptr, output, useful_bytes, &xvid_dec_stats);
        if (xvid_dec_stats.type == XVID_TYPE_VOL) {
            // if (g_dpi->fp.Width * g_dpi->fp.Height <
            //     xvid_dec_stats.data.vol.width * xvid_dec_stats.data.vol.height) {
            //     printf("[error]ohhhh TODO\n");
            // }
        }
        /* Update buffer pointers */
        if (used_bytes > 0) {
            mp4_ptr += used_bytes;
            useful_bytes -= used_bytes;
        } else if (used_bytes == 0) {
            zero_times --;
            if (zero_times == 0) {
                break;
            }
        }
    } while (xvid_dec_stats.type <= 0 && useful_bytes > 1);
    return 0;
}

int VideoDecInit(unsigned width, unsigned higth)
{
    xvid_gbl_init_t xvid_gbl_init;
    xvid_dec_create_t xvid_dec_create;
    xvid_gbl_info_t xvid_gbl_info;

    /* Reset the structure with zeros */
    memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init_t));
    memset(&xvid_dec_create, 0, sizeof(xvid_dec_create_t));
    memset(&xvid_gbl_info, 0, sizeof(xvid_gbl_info));
    /*------------------------------------------------------------------------
     * Xvid core initialization
     *----------------------------------------------------------------------*/
    xvid_gbl_info.version = XVID_VERSION;
    xvid_global(NULL, XVID_GBL_INFO, &xvid_gbl_info, NULL);

    if (xvid_gbl_info.build != NULL) {
        printf("xvidcore build version: %s\n", xvid_gbl_info.build);
    }
    /* Version */
    xvid_gbl_init.version = XVID_VERSION;
    xvid_gbl_init.cpu_flags = XVID_CPU_FORCE;
    xvid_gbl_init.debug = 0;
    xvid_global(NULL, 0, &xvid_gbl_init, NULL);

    /*------------------------------------------------------------------------
     * Xvid decoder initialization
     *----------------------------------------------------------------------*/

    /* Version */
    xvid_dec_create.version = XVID_VERSION;

    /*
     * Image dimensions -- set to 0, xvidcore will resize when ever it is
     * needed
     */
    xvid_dec_create.width = width;
    xvid_dec_create.height = higth;

    xvid_dec_create.num_threads = xvid_gbl_info.num_threads;

    if (xvid_decore(NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL) < 0) {
        return -1;
    }

    g_width_line = width * (MP4_DEC_COLOR_DEPTH >> 3);
    g_xvid_handle = xvid_dec_create.handle;

    return 0;
}

void VideoDecDeinit(void)
{
    xvid_decore(g_xvid_handle, XVID_DEC_DESTROY, NULL, NULL);
    g_xvid_handle = NULL;
}