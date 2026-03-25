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

#include <console.h>
#include "audio_codec.h"
#include "opus_wraper.h"
#include "log.h"

#define AUDIO_TEST_SAMPLE  16000
#define AUDIO_TEST_CHANNEL 1
#define OPUS_MAX_DATA_DURA 20

struct audio_codec *codec_test = NULL;

int audio_cb_test(uint8_t *data, int size, int is_vad)
{
    if (codec_test != NULL)
        audio_codec_output_data(codec_test, data, size);

    return 0;
}

static int cmd_ct_audio(int argc, char *argv[])
{
    codec_test = audio_codec_init(AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL,
        AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL, audio_cb_test);
    if (codec_test == NULL)
        CHATBOX_ERROR("test init fail\n");
    else
        CHATBOX_INFO("test start\n");

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ct_audio, cta_test, chat_audio_api_test);

static struct opus_wraper_decoder *t_dec = NULL;
static struct opus_wraper_encoder *t_enc = NULL;

void opus_cb_decode(uint8_t *opus, size_t opus_size)
{
    uint8_t buff[1024];
    size_t dec_size;

    opus_wraper_decoder_decode(t_dec, opus, opus_size, buff, 1024, &dec_size);

    audio_codec_output_data(codec_test, buff, dec_size);
}

int opus_cb_test(uint8_t *data, int size, int is_vad)
{
    if (t_enc != NULL) {
        opus_wraper_encoder_encode(t_enc, data, size, opus_cb_decode);
    }

    return 0;
}

static int cmd_ct_opus(int argc, char *argv[])
{
    t_dec = opus_wraper_decoder_create(AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL, OPUS_MAX_DATA_DURA);
    if (t_dec == NULL)
        goto cmd_exit;

    t_enc = opus_wraper_encoder_create(AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL, OPUS_MAX_DATA_DURA);
    if (t_enc == NULL)
        goto cmd_exit;

    codec_test = audio_codec_init(AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL,
        AUDIO_TEST_SAMPLE, AUDIO_TEST_CHANNEL, opus_cb_test);
    if (codec_test == NULL)
        CHATBOX_ERROR("test init fail\n");
    else
        CHATBOX_INFO("test start\n");

    return 0;

cmd_exit:
    if (t_dec != NULL)
        opus_wraper_decoder_destroy(t_dec);

    if (t_enc == NULL)
        opus_wraper_encoder_destroy(t_enc);
    return -1;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_ct_opus, cta_opus, chat_audio_opus_api_test);