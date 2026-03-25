#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sunxi_amp.h"
#include "hal_cache.h"
#include "xvid.h"

#define MP4_DEC_COLOR_DEPTH 12                     //16
#define MP4_DEC_OUT_FORMAT  XVID_CSP_I420          //XVID_CSP_BGRA//XVID_CSP_RGB565

static void *g_xvid_handle = NULL;
static unsigned g_width_line = 0;
static unsigned g_draw_size = 0;
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

static int __Mp4VideoDecFrame(unsigned char *input, unsigned in_size, unsigned char *output)
{
    unsigned char *mp4_ptr = input;
    int useful_bytes = in_size;
    int used_bytes = 0;
    xvid_dec_stats_t xvid_dec_stats;
    int zero_times = 10;

    hal_dcache_invalidate((unsigned long)input, in_size);

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

    hal_dcache_clean((unsigned long)output, g_draw_size);
    return 0;
}

static int __VideoDecInit(unsigned width, unsigned higth)
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
    g_draw_size = (width * higth * 3 / 2);

    return 0;
}

static void __VideoDecDeinit(void)
{
    xvid_decore(g_xvid_handle, XVID_DEC_DESTROY, NULL, NULL);
    g_xvid_handle = NULL;
}

sunxi_amp_func_table xvid_table[] =
{
    {.func = (void *)&__VideoDecInit,      .args_num = 2, .return_type = RET_POINTER},
    {.func = (void *)&__VideoDecDeinit,    .args_num = 0, .return_type = RET_POINTER},
    {.func = (void *)&__Mp4VideoDecFrame,     .args_num = 3, .return_type = RET_POINTER},
};

