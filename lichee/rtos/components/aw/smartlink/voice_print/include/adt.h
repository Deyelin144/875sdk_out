/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ADT_H_
#define _ADT_H_

/* frequency type selector */
typedef enum {
	FREQ_TYPE_LOW = 0,      /* low frequency, 2K~5K */
	FREQ_TYPE_MIDDLE,       /* middle frequency, 8K~12K */
	FREQ_TYPE_HIGH          /* high frequency, 16K~20K */
} freq_type_t;

/* macros of return valule of decoder */
#define RET_DEC_ERROR           -1      /* decoder error */
#define RET_DEC_NORMAL          0       /* decoder normal return */
#define RET_DEC_NOTREADY        1       /* cant get result for decoder has not finished */
#define RET_DEC_END             2       /* decoder end */


/* definition of decoder config paramters */
typedef struct {
	int max_strlen;         /* max supported length of string */
	int sample_rate;        /* sample rate */
	freq_type_t freq_type;  /* freq type */
	int group_symbol_num;   /* symbol number of every group */
	int error_correct;      /* use error correcting code function */
	int error_correct_num;  /* error correcting code capabilty */
} config_decoder_t;

/**
 * @brief Create decoder .
 * @param decode_config:
 *        @decode config param.
 * @retval  decoder handler, faild if NULL.
 */
void *decoder_create(config_decoder_t *decode_config);

/**
 * @brief Reset decoder.
 * @param handle:
 *        @decoder handler.
 * @retval  None.
 */
void decoder_reset(void *handle);

/**
 * @brief Get sample number of every sample frame.
 * @param handle:
 *        @decoder handler.
 * @retval  sample number of every sample frame(16bit of every sample data).
 */
int decoder_getbsize(void *handle);

/**
 * @brief Fed decoder data.
 * @param handle:
 *        @decoder handler.
 * @param pcm:
 *        @pcm data, the sample number is equal to decoder_getbsize.
 * @retval  defined by RET_DEC_XX.
 */
int decoder_fedpcm(void *handle, short *pcm);

/**
 * @brief Get decode result.
 * @param handle:
 *        @decoder handler.
 * @param str:
 *        @decode result string.
 * @retval defined by RET_DEC_XX.
 */
int decoder_getstr(void *handle, unsigned char *str);

/**
 * @brief Release decoder handler.
 * @param handle:
 *        @decoder handler.
 * @retval  None.
 */
void decoder_destroy(void *handle);

#endif
