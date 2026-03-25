/**
  src/components/asr_component.c

  OpenMAX automatic speech recognition component. This component implements a speech recognition that
  recognition process audio PCM streams and produces a single input and output stream.

* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
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

#include <omx_base_audio_port.h>
#include "OMX_Base.h"

#include "asr_component.h"
#include <OMX_Audio.h>
// #include <OMX_EXTREN_Flag.h>
//

#define FILESAME 0

#if FILESAME
#include "base_pcm.h"
#endif

#if 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

static int bAsrVadEosTime = 0;
static int sta_cnt = 0;
int sdk_start_flag = 1;

/** @brief The library entry point. It must have the same name for each
 * library of the components loaded by the static component loader.
 *
 * This function fills the version, the component name and if existing also the roles
 * and the specific names for each role. This base function is only an explanation.
 * For each library it must be implemented, and it must fill data of any component
 * in the library
 *
 * @param OmxComponents pointer to an array of components descriptors.If NULL, the
 * function will return only the number of components contained in the library
 *
 * @return number of components contained in the library
 */
OMX_ERRORTYPE omx_audio_asr_component_setup(OmxLoaderComponentType *OmxComponents)
{

	omx_debug("In %s \n", __func__);

	/** component 1 - volume component */
	OmxComponents->componentVersion.s.nVersionMajor = 1;
	OmxComponents->componentVersion.s.nVersionMinor = 1;
	OmxComponents->componentVersion.s.nRevision = 1;
	OmxComponents->componentVersion.s.nStep = 1;

	strncpy(OmxComponents->name, ASR_COMP_NAME, OMX_STRINGNAME_SIZE);

	OmxComponents->name_specific_length = NAME_SPECIFIC_NUM;
	OmxComponents->constructor = omx_audio_asr_component_Constructor;

	strncpy(OmxComponents->name_specific[0], ASR_COMP_NAME, OMX_STRINGNAME_SIZE);
	strncpy(OmxComponents->role_specific[0], ASR_COMP_ROLE, OMX_STRINGNAME_SIZE);

	omx_debug("Out of %s \n", __func__);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_audio_asr_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp, OMX_STRING cComponentName)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private;
	omx_audio_asr_component_PortType *pPort; // *inPort;
	OMX_U32 i;

	if (!openmaxStandComp->pComponentPrivate)
	{
		omx_debug("In %s, allocating component\n", __func__);
		openmaxStandComp->pComponentPrivate = omx_alloc(sizeof(omx_audio_asr_component_PrivateType));
		if (openmaxStandComp->pComponentPrivate == NULL)
		{
			return OMX_ErrorInsufficientResources;
		}
		memset(openmaxStandComp->pComponentPrivate, 0x00, sizeof(omx_audio_asr_component_PrivateType));
	}
	else
	{
		omx_debug("In %s, Error Component %p Already Allocated\n", __func__, openmaxStandComp->pComponentPrivate);
	}

	omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_Private->ports = NULL;

	/** Calling base equalizer constructor */
	err = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

	/*Assuming 1 input port and 1 output port*/
	omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = ASR_MAX_PORTS;

	/** Allocate Ports and call port constructor. */
	if (omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_audio_asr_component_Private->ports)
	{
		omx_audio_asr_component_Private->ports = omx_alloc(omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts * sizeof(omx_base_PortType *));
		if (!omx_audio_asr_component_Private->ports)
		{
			return OMX_ErrorInsufficientResources;
		}

		memset(omx_audio_asr_component_Private->ports, 0x00, omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts * sizeof(omx_base_PortType *));

		for (i = 0; i < omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++)
		{
			omx_audio_asr_component_Private->ports[i] = omx_alloc(sizeof(omx_audio_asr_component_PortType));
			if (!omx_audio_asr_component_Private->ports[i])
			{
				return OMX_ErrorInsufficientResources;
			}
			memset(omx_audio_asr_component_Private->ports[i], 0x00, sizeof(omx_audio_asr_component_PortType));
		}
	}

	/* construct all input ports */
	for (i = 0; i < omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts - 1; i++)
	{
		base_audio_port_Constructor(openmaxStandComp, &omx_audio_asr_component_Private->ports[i], i, OMX_TRUE);
	}

	/* construct one output port */
	base_audio_port_Constructor(openmaxStandComp,
								&omx_audio_asr_component_Private->ports[omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts - 1],
								omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts - 1, OMX_FALSE);

	/** Domain specific section for the ports. */
	for (i = 0; i < omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++)
	{
		pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[i];

		pPort->sPortParam.nBufferSize = ASR_BUF_SIZE;
		pPort->sPortParam.nBufferCountActual = 8;

		setHeader(&pPort->sAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
		pPort->sAudioPcmMode.nPortIndex = i;
		pPort->sAudioPcmMode.nChannels = ASR_INPUT_CHANNEL;
		pPort->sAudioPcmMode.eNumData = OMX_NumericalDataSigned;
		pPort->sAudioPcmMode.eEndian = OMX_EndianLittle;
		pPort->sAudioPcmMode.bInterleaved = OMX_TRUE;
		pPort->sAudioPcmMode.nBitPerSample = 16;
		pPort->sAudioPcmMode.nSamplingRate = 16000;
		pPort->sAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;

		setHeader(&pPort->sAudioAsrType, sizeof(OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE));
		pPort->sAudioAsrType.nPortIndex = i;
		pPort->sAudioAsrType.bAutoSpeechRecg = OMX_FALSE;
		pPort->sAudioAsrType.bAsrVad = OMX_FALSE;
		pPort->sAudioAsrType.bAsrdump = OMX_FALSE;
	}

	omx_audio_asr_component_Private->destructor = omx_audio_asr_component_Destructor;
	openmaxStandComp->SetParameter = omx_audio_asr_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audio_asr_component_GetParameter;
	openmaxStandComp->GetConfig = omx_audio_asr_component_GetConfig;
	openmaxStandComp->SetConfig = omx_audio_asr_component_SetConfig;
	omx_audio_asr_component_Private->BufferMgmtCallback = omx_audio_asr_component_BufferMgmtCallback;
	omx_audio_asr_component_Private->BufferMgmtFunction = omx_audio_asr_BufferMgmtFunction;
	omx_audio_asr_component_Private->DoStateSet = omx_audio_asr_component_SetState;

	return err;
}

/** The destructor
 */
OMX_ERRORTYPE omx_audio_asr_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{

	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	omx_debug("In %s\n", __func__);
	/* frees port/s */
	if (omx_audio_asr_component_Private->ports)
	{
		for (i = 0; i < omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++)
		{
			if (omx_audio_asr_component_Private->ports[i])
				omx_audio_asr_component_Private->ports[i]->PortDestructor(omx_audio_asr_component_Private->ports[i]);
		}
		omx_free(omx_audio_asr_component_Private->ports);
		omx_audio_asr_component_Private->ports = NULL;
	}

	omx_base_filter_Destructor(openmaxStandComp);

	omx_debug("Out of %s\n", __func__);

	return OMX_ErrorNone;
}

static OMX_ERRORTYPE omx_audio_asr_component_init(OMX_COMPONENTTYPE *openmaxStandComp)
{
	OMX_S32 ret = 0;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#define DOUBLE_HANDLE 0
#if DOUBLE_HANDLE
	uv_audio_code stream_ret;
	uv_recog_code recog_ret;
#else
	uvoice_api_code sdk_ret;
#endif
#endif
	pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[OMX_AUDIO_ASR_INPUTPORT_INDEX];
	OMX_U32 rate;
	OMX_U32 channels;
	OMX_U32 bitwidth;

	channels = pPort->sAudioPcmMode.nChannels;
	bitwidth = pPort->sAudioPcmMode.nBitPerSample;
	rate = pPort->sAudioPcmMode.nSamplingRate;

	if (channels == 0 || bitwidth == 0 || rate == 0)
	{
		omx_err("input param error, rate %lu, ch %lu, width %lu\n",
				rate, channels, bitwidth);
		ret = OMX_ErrorBadParameter;
		return ret;
	}

	omx_audio_asr_component_Private->uBytesPerSample = bitwidth * channels / 8;

	omx_info("bits %lu, ch %lu, rate %lu, uBytesPerSample %lu\n",
			 bitwidth, channels,
			 rate, omx_audio_asr_component_Private->uBytesPerSample);

	omx_audio_asr_component_Private->pAsrInputBuffer = omx_alloc(NN * omx_audio_asr_component_Private->uBytesPerSample);
	if (omx_audio_asr_component_Private->pAsrInputBuffer == NULL)
	{
		omx_err("pAsrInputBuffer alloc failed\n");
		ret = OMX_ErrorInsufficientResources;
		return ret;
	}

	/* init asr alth */
	if (pPort->sAudioAsrType.bAutoSpeechRecg)
	{

#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
		printf("uvoice_sdk_streaming_init! %d\n", 1);
		stream_ret = uvoice_sdk_streaming_init(&omx_audio_asr_component_Private->pStreaming);
		if (UV_SDK_STREAMING_OK != stream_ret)
		{
			omx_err("Streaming init failed, ret = %d\n", stream_ret);
			ret = OMX_ErrorBadParameter;
			return ret;
		}

		recog_ret = uvoice_sdk_recognizer_init(&omx_audio_asr_component_Private->pRecognizer);
		if (UV_SDK_RECOGNIZER_OK != recog_ret)
		{
			omx_err("Recognizer init failed, ret = %d\n", recog_ret);
			ret = OMX_ErrorBadParameter;
			return ret;
		}
#else
		printf("sdk_init %d\n", 0);
		sdk_ret = uvoice_sdk_init(&omx_audio_asr_component_Private->pStreaming);
		if (UVOICE_API_OK != sdk_ret)
		{
			omx_err("Handle init failed, ret = %d\n", sdk_ret);
			printf("Handle init failed, ret = %d\n", sdk_ret);
			ret = OMX_ErrorBadParameter;
			return ret;
		}
#endif
#endif
		printf("sdk_init %d\n", 1);
	}
	return ret;
}

static void omx_audio_asr_component_exit(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
	uv_audio_code stream_ret;
	uv_recog_code recog_ret;
#else
	uvoice_api_code sdk_ret;
#endif
#endif
	if (omx_audio_asr_component_Private->pStreaming == NULL) // || omx_audio_asr_component_Private->pRecognizer == NULL)
		return;

	pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[OMX_AUDIO_ASR_INPUTPORT_INDEX];

	/* destroy asr alth */
	if (pPort->sAudioAsrType.bAutoSpeechRecg)
	{
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
		if (omx_audio_asr_component_Private->pRecognizer)
		{
			recog_ret = uvoice_sdk_recognizer_release(omx_audio_asr_component_Private->pRecognizer);
			if (UV_SDK_RECOGNIZER_OK != recog_ret)
			{
				omx_err("Streaming init failed, ret = %d\n", recog_ret);
				return;
			}
			omx_audio_asr_component_Private->pRecognizer = NULL;
		}

		if (omx_audio_asr_component_Private->pStreaming)
		{
			stream_ret = uvoice_sdk_streaming_release(omx_audio_asr_component_Private->pStreaming);
			if (UV_SDK_STREAMING_OK != stream_ret)
			{
				omx_err("Recognizer init failed, ret = %d\n", stream_ret);
				return;
			}
			omx_audio_asr_component_Private->pStreaming = NULL;
		}
#else
		sdk_ret = uvoice_sdk_deinit(&omx_audio_asr_component_Private->pStreaming);
		if (UVOICE_API_OK != sdk_ret)
		{
			omx_err("Handle init failed, ret = %d\n", sdk_ret);
			return;
		}
#endif
#endif
	}

	if (omx_audio_asr_component_Private->pAsrInputBuffer)
	{
		omx_free(omx_audio_asr_component_Private->pAsrInputBuffer);
		omx_audio_asr_component_Private->pAsrInputBuffer = NULL;
	}

	omx_audio_asr_component_Private->uBytesPerSample = 0;
}

static int omx_audio_asr_component_Pcms16leSplitToPlane(OMX_COMPONENTTYPE *openmaxStandComp, short *dst, short *src, unsigned int len)
{
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
	uint32_t rate = 0;
	uint32_t channels = 0;
	uint8_t bitwidth = 0;
	uint32_t BytesPerSample = 0;
	short nSampleCount = 0;
	int i = 0;
	int j = 0;

	pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[OMX_AUDIO_ASR_INPUTPORT_INDEX];

	channels = pPort->sAudioPcmMode.nChannels;
	bitwidth = pPort->sAudioPcmMode.nBitPerSample;
	rate = pPort->sAudioPcmMode.nSamplingRate;

	if (channels == 0 || bitwidth == 0 || rate == 0)
	{
		printf("input param error, rate %d, ch %d, width %d\n",
			   rate, channels, bitwidth);
		return -1;
	}

	BytesPerSample = bitwidth * channels / 8;

	nSampleCount = len / BytesPerSample;

	for (i = 0; i < nSampleCount; ++i)
	{
		for (j = 0; j < channels; j++)
		{
			dst[i + nSampleCount * j] = src[channels * i + j];
		}
	}

	return 0;
}

#define TIME_OUT_LICENCE 0
#if TIME_OUT_LICENCE
static int time_max = 30 * 60 * 100 * 2; // 30min
static int time_cnt = 0;				 // jishiqi
#endif

#define ORIGIN 0
#if ORIGIN
// static FILE* ori_file = NULL;
static int ori_file = NULL;
#endif

#if FILESAME
static int pcm_read_frame = 0;
static int pcm_cnt = 0;
#endif

extern int arecord_commponent_get_stop_flag(void);

/** This function is used to process the input buffer and provide one output buffer
 */
void omx_audio_asr_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE *pInBuffer, OMX_BUFFERHEADERTYPE *pOutBuffer)
{
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
	uv_audio_code stream_ret;
	uv_recog_code recog_ret;
	short out_buffer[160] = {0};
	int vad_flag = 0;
	int word_id = 0;
	float confidence = 0;
#else
	uvoice_api_code sdk_ret;
	char out_buffer[1500] = {0};
	static int stop_flag = 0;
#endif
	uvoice_stream audio;
#endif

	short *in_buffer = (short *)omx_audio_asr_component_Private->pAsrInputBuffer;

	OMX_U32 nSampleCount = 0;
	OMX_S16 nSampleOffset = 0;

	int *pIntmp = NULL;
	float *pFloattmp = NULL;

	if (pInBuffer == NULL || pOutBuffer == NULL)
		return;

	pOutBuffer->nFilledLen = 0;
	if (pInBuffer->pBuffer == NULL || pOutBuffer->pBuffer == NULL || in_buffer == NULL)
	{
		omx_err("inbuffer:%p, pBuffer:%p!!",
				pInBuffer->pBuffer, pOutBuffer->pBuffer);
		omx_err("err param!");
		return;
	}

	if (pInBuffer->nFilledLen == 0)
	{
		omx_info("nFilledLen = 0, return!\n");
		return;
	}

	pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[pInBuffer->nInputPortIndex];

	pOutBuffer->nFlags = pInBuffer->nFlags;
	pOutBuffer->nTimeStamp = pInBuffer->nTimeStamp;

	nSampleCount = pInBuffer->nFilledLen / omx_audio_asr_component_Private->uBytesPerSample;

	nSampleOffset = pInBuffer->nFilledLen / pPort->sAudioPcmMode.nChannels;

	if (nSampleCount != NN)
	{
		omx_err("nSampleCount %lu is invaild, return!\n", nSampleCount);
		return;
	}

	pOutBuffer->nOffset = sizeof(int) * 2 + sizeof(float); // nOffset = 12byte : vad_flag + word_id + confidence

	if (pPort->sAudioAsrType.bAsrdump)
		pOutBuffer->nFilledLen = (pInBuffer->nFilledLen / pPort->sAudioPcmMode.nChannels) * 4 + pOutBuffer->nOffset; // 320 * 4 + 12 = 1292 byte
	else
		pOutBuffer->nFilledLen = (pInBuffer->nFilledLen / pPort->sAudioPcmMode.nChannels) + pOutBuffer->nOffset; // 320  + 12 = 332 byte

	omx_audio_asr_component_Pcms16leSplitToPlane(openmaxStandComp, in_buffer,
												 (short *)(pInBuffer->pBuffer + pInBuffer->nOffset), pInBuffer->nFilledLen);

#if ORIGIN
	if (ori_file == NULL)
	{
		ori_file = open("/data/test.pcm", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	}
	short ipt[320 * 3];
	int tmpi = 0;
	for (tmpi = 0; tmpi < 160; tmpi++)
	{
		ipt[tmpi * 3 + 0] = *((short *)(in_buffer) + 0 * 160 + tmpi);
		ipt[tmpi * 3 + 1] = *((short *)(in_buffer) + 1 * 160 + tmpi);
		ipt[tmpi * 3 + 2] = *((short *)(in_buffer) + 2 * 160 + tmpi);
	}
	int rt = write(ori_file, ipt, 3 * 320);
	if (rt != 3 * 320)
	{
		printf("[UVOICE] Only %d byte!\n", rt);
	}
	// flush(ori_file);
#endif

#if TIME_OUT_LICENCE
	time_cnt++;
	if (time_cnt <= time_max)
	{
		time_cnt++;
#endif

#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
	if (pPort->sAudioAsrType.bAutoSpeechRecg) {
		memset(&audio, 0, sizeof(uvoice_stream));
		if (pPort->sAudioPcmMode.nChannels == 2) {
			/*default to 1mic+1ref*/
			audio.audioin[0] = (short *)(in_buffer);
			audio.audioref[0] = (short *)((char *)in_buffer + nSampleOffset);
		} else if (pPort->sAudioPcmMode.nChannels == 3) {
			/* 2mic+1ref input_mode==1 */
			audio.audioin[0] = (short *)(in_buffer);
			audio.audioin[1] = (short *)((char *)in_buffer + nSampleOffset);
			audio.audioref[0] = (short *)((char *)in_buffer + nSampleOffset * 2);
		} else if (pPort->sAudioPcmMode.nChannels == 4) {
			/*  2mic+2ref input_mode == 2 */
			audio.audioin[0] = (short *)(in_buffer);
			audio.audioin[1] = (short *)((char *)in_buffer + nSampleOffset);
			audio.audioref[0] = (short *)((char *)in_buffer + nSampleOffset * 2);
			audio.audioref[1] = (short *)((char *)in_buffer + nSampleOffset * 3);
		}
		else if (pPort->sAudioPcmMode.nChannels == 5) {
			/*  4mic+1ref input_mode == 3 */
			audio.audioin[0] = (short *)(in_buffer);
			audio.audioin[1] = (short *)((char *)in_buffer + nSampleOffset);
			audio.audioin[2] = (short *)((char *)in_buffer + nSampleOffset * 2);
			audio.audioin[3] = (short *)((char *)in_buffer + nSampleOffset * 3);
			audio.audioref[0] = (short *)((char *)in_buffer + nSampleOffset * 4);
		}

#if FILESAME // 一致性
		pcm_cnt++;
		if (pcm_cnt < 200)
			return;
		if (pcm_cnt == 200)
			printf("[UVOICE] start!\n");
		pcm_read_frame++;
		if (pcm_read_frame == pcm_max_frame) {
			// pcm_read_frame = 0;
			printf("read a round\n");
		} else if (pcm_read_frame < pcm_max_frame) {
			printf("[UVOICE] cnt %d\n", pcm_read_frame);
		}
		int cnt = 0;
		for (cnt = 0; cnt < 160; cnt++) {
			audio.audioin[0][cnt] = testPcm[pcm_read_frame - 1][3 * cnt];
			audio.audioin[1][cnt] = testPcm[pcm_read_frame - 1][3 * cnt + 1];
			audio.audioref[0][cnt] = testPcm[pcm_read_frame - 1][3 * cnt + 2];
			// audio.audioref[0][cnt] = 0;
		}
#endif
		// diff
		if (bAsrVadEosTime != 0) {
#if DOUBLE_HANDLE
			printf("[UVOICE] streaming p %p, time %d\n", omx_audio_asr_component_Private->pStreaming, bAsrVadEosTime);
			stream_ret = uvoice_sdk_streaming_change_vad_eos(omx_audio_asr_component_Private->pStreaming, bAsrVadEosTime);
			if (UV_SDK_STREAMING_OK != stream_ret) {
				omx_err("Streaming change vad eos time failed, ret = %d\n", stream_ret);
				return;
			}
			bAsrVadEosTime = 0;
#else
			printf("[UVOICE] streaming p %p, time %d\n", omx_audio_asr_component_Private->pStreaming, bAsrVadEosTime);
			sdk_ret = uvoice_sdk_change_vad_eos(omx_audio_asr_component_Private->pStreaming, bAsrVadEosTime);
			if (UVOICE_API_OK != sdk_ret) {
				omx_err("Streaming change vad eos time failed, ret = %d\n", sdk_ret);
				return;
			}
			bAsrVadEosTime = 0;
#endif
		}

		if (sdk_start_flag == 0) {
			if (stop_flag == 0 && arecord_commponent_get_stop_flag()) {
				pIntmp = (int *)(pOutBuffer->pBuffer);
				pIntmp[0] = 0xFF;
				pIntmp[1] = 0xFF;
				pIntmp[2] = 0;
				stop_flag = 1;
        memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset, (OMX_S8 *)in_buffer, pInBuffer->nFilledLen);
        memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset + pInBuffer->nFilledLen, (OMX_S8 *)in_buffer, NN * sizeof(OMX_S16));
        goto exit;
			} else {
        pOutBuffer->nFilledLen = 0;
        return;
      }
		} else {
			sdk_ret = uvoice_sdk_process(omx_audio_asr_component_Private->pStreaming, &audio, out_buffer, sdk_start_flag);
			if (UVOICE_API_OK != sdk_ret) {
				omx_err("sdk process error.\n");
				return;
			}
			stop_flag = 0;
			char *data = (char *)out_buffer;
			// printf("dsp: vad_flag %d, word_id %d, confidence %f\n", *(int *)((char *)data + 0),*(int *)((char *)data + 4), *(float *)((char *)data + 8));
			// printf("1: %d\n", *(int *)((char *)data + 12));
			memcpy(pOutBuffer->pBuffer, (OMX_S8 *)out_buffer, 4 * 3 + 320 * 4);
			// memcpy(pOutBuffer->pBuffer, (OMX_S8 *)out_buffer, 4 * 3 + 320 * 3);
			data = (char *)(pOutBuffer->pBuffer);
			sta_cnt++;
		}
	} else {
		pIntmp = (int *)(pOutBuffer->pBuffer);
		pIntmp[0] = -1;
		pIntmp[1] = -1;
		pFloattmp = (float *)(pOutBuffer->pBuffer + sizeof(int) * 2);
		pFloattmp[0] = -1.0;
		if (pPort->sAudioAsrType.bAsrdump) {
			memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset, (OMX_S8 *)in_buffer, pInBuffer->nFilledLen);
			memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset + pInBuffer->nFilledLen, (OMX_S8 *)in_buffer, NN * sizeof(OMX_S16));
		} else {
			memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset, (OMX_S8 *)in_buffer, NN * sizeof(OMX_S16)); // output left channel data
		}
	}
#else

	pIntmp = (int *)(pOutBuffer->pBuffer);
	pIntmp[0] = -1;
	pIntmp[1] = -1;
	pFloattmp = (float *)(pOutBuffer->pBuffer + sizeof(int) * 2);
	pFloattmp[0] = -1.0;

	if (pPort->sAudioAsrType.bAsrdump) {
		memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset, (OMX_S8 *)in_buffer, pInBuffer->nFilledLen);
		memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset + pInBuffer->nFilledLen, (OMX_S8 *)in_buffer, NN * sizeof(OMX_S16));
	} else {
		memcpy(pOutBuffer->pBuffer + pOutBuffer->nOffset, (OMX_S8 *)in_buffer, NN * sizeof(OMX_S16)); // output left channel data
	}	
#endif
exit:
	pInBuffer->nFilledLen = 0;
	if (pInBuffer->nFlags & OMX_BUFFERFLAG_EOS) {
		omx_err("equalizer_buf_handle get the end of stream\n");
		pOutBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
	}
}

/** setting configurations */
OMX_ERRORTYPE omx_audio_asr_component_SetConfig(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nIndex,
	OMX_PTR pComponentConfigStructure)
{

	OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE *pAsr;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
	OMX_ERRORTYPE err = OMX_ErrorNone;
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
	uv_audio_code stream_ret;
#else
	uvoice_api_code sdk_ret;
#endif
#endif

	switch (nIndex)
	{

	case OMX_IndexConfigAudioAutoSpeechRecognition:
	{
		pAsr = (OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE *)pComponentConfigStructure;
		;
		if (pAsr->nPortIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[pAsr->nPortIndex];
			omx_info("Port %i Enable=%d, vad_enable=%d\n", (int)pAsr->nPortIndex, (int)pAsr->bAutoSpeechRecg, (int)pAsr->bAsrVad);
			memcpy(&pPort->sAudioAsrType, pAsr, sizeof(OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE));
#if (defined CONFIG_COMPONENTS_PROCESS_ASR) && (defined CONFIG_XTENSA_HIFI5)
#if DOUBLE_HANDLE
			if (pAsr->bAsrVad == OMX_TRUE && omx_audio_asr_component_Private->pStreaming)
			{
				stream_ret = uvoice_sdk_streaming_enable_vad(omx_audio_asr_component_Private->pStreaming);
				if (UV_SDK_STREAMING_OK != stream_ret)
				{
					omx_err("Streaming enable vad failed, ret = %d\n", stream_ret);
					return OMX_ErrorBadParameter;
				}
			}
			else if (pAsr->bAsrVad == OMX_FALSE && omx_audio_asr_component_Private->pStreaming)
			{
				stream_ret = uvoice_sdk_streaming_disable_vad(omx_audio_asr_component_Private->pStreaming);
				if (UV_SDK_STREAMING_OK != stream_ret)
				{
					omx_err("Streaming disable vad failed, ret = %d\n", stream_ret);
					return OMX_ErrorBadParameter;
				}
			}
#else
			if (pAsr->bAsrVad == OMX_TRUE && omx_audio_asr_component_Private->pStreaming)
			{
				sdk_ret = uvoice_sdk_enable_vad(omx_audio_asr_component_Private->pStreaming);
				if (UVOICE_API_OK != sdk_ret)
				{
					omx_err("Sdk enable vad failed, ret = %d\n", sdk_ret);
					return OMX_ErrorBadParameter;
				}
			}
			else if (pAsr->bAsrVad == OMX_FALSE && omx_audio_asr_component_Private->pStreaming)
			{
				sdk_ret = uvoice_sdk_disable_vad(omx_audio_asr_component_Private->pStreaming);
				if (UVOICE_API_OK != sdk_ret)
				{
					omx_err("Sdk disable vad failed, ret = %d\n", sdk_ret);
					return OMX_ErrorBadParameter;
				}
			}
#endif
			if ((pAsr->bAsrVadEosTime != 0) && (omx_audio_asr_component_Private->pStreaming))
			{
				bAsrVadEosTime = (int)pAsr->bAsrVadEosTime;
				//				printf("[UVOICE] streaming p %p, time %d\n", omx_audio_asr_component_Private->pStreaming, (int)pAsr->bAsrVadEosTime);
				//				stream_ret = uvoice_sdk_streaming_change_vad_eos(omx_audio_asr_component_Private->pStreaming, pAsr->bAsrVadEosTime);
				//				if(UV_SDK_STREAMING_OK != stream_ret) {
				//                                        omx_err("Streaming change vad eos time failed, ret = %d\n", stream_ret);
				//                                        return OMX_ErrorBadParameter;
				//                                }
			}
			if ((pAsr->SDKStartFlag != 2) && (omx_audio_asr_component_Private->pStreaming))
			{
				sdk_start_flag = pAsr->SDKStartFlag;
			}
			if ((pAsr->bWakeUpValue > 0.0f) && (pAsr->bWakeUpValue < 1.0f) && (omx_audio_asr_component_Private->pStreaming))
			{
#if DOUBLE_HANDLE
				stream_ret = uvoice_sdk_streaming_change_wakeup_value(omx_audio_asr_component_Private->pStreaming, pAsr->bWakeUpValue);
				if (UV_SDK_STREAMING_OK != stream_ret)
				{
					omx_err("Streaming change wakeup failed, ret = %d\n", stream_ret);
					return OMX_ErrorBadParameter;
				}
#else
				sdk_ret = uvoice_sdk_change_wakeup_value(omx_audio_asr_component_Private->pStreaming, pAsr->bWakeUpValue);
				if (UVOICE_API_OK != sdk_ret)
				{
					omx_err("Sdk change wakeup failed, ret = %d\n", sdk_ret);
					return OMX_ErrorBadParameter;
				}
#endif
			}
#endif
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
	}
	break;
	default: // delegate to superclass
		err = omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audio_asr_component_GetConfig(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nIndex,
	OMX_PTR pComponentConfigStructure)
{

	OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE *pAsr;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audio_asr_component_PortType *pPort;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	switch (nIndex)
	{

	case OMX_IndexConfigAudioAutoSpeechRecognition:
	{
		pAsr = (OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE *)pComponentConfigStructure;
		;
		if (pAsr->nPortIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			pPort = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[pAsr->nPortIndex];
			omx_debug("Port %i Enable=%d, vad_enable=%d\n", (int)pAsr->nPortIndex, (int)pAsr->bAutoSpeechRecg, (int)pAsr->bAsrVad);
			memcpy(pAsr, &pPort->sAudioAsrType, sizeof(OMX_AUDIO_CONFIG_AUTOSPEECHRECOGNITIONTYPE));
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
	}
	break;
	default:
		err = omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audio_asr_component_SetParameter(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nParamIndex,
	OMX_PTR ComponentParameterStructure)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
	OMX_U32 portIndex;
	OMX_AUDIO_PARAM_PCMMODETYPE *pAudioParams = NULL;
	omx_audio_asr_component_PortType *port;

	/* Check which structure we are being fed and make control its header */
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}

	omx_debug("   Setting parameter %i\n", nParamIndex);
	switch (nParamIndex)
	{
	case OMX_IndexParamAudioPortFormat:
		pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
		portIndex = pAudioPortFormat->nPortIndex;
		err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
		if (err != OMX_ErrorNone)
		{
			omx_debug("In %s Parameter Check Error=%x\n", __func__, err);
			break;
		}
		if (portIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			port = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[portIndex];
			memcpy(&port->sAudioParam, pAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
		break;
	case OMX_IndexParamStandardComponentRole:
		pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;

		if (omx_audio_asr_component_Private->state != OMX_StateLoaded && omx_audio_asr_component_Private->state != OMX_StateWaitForResources)
		{
			omx_debug("In %s Incorrect State=%x lineno=%d\n", __func__, omx_audio_asr_component_Private->state, __LINE__);
			return OMX_ErrorIncorrectStateOperation;
		}

		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
		{
			break;
		}

		if (strcmp((char *)pComponentRole->cRole, ASR_COMP_ROLE))
		{
			return OMX_ErrorBadParameter;
		}
		break;

	case OMX_IndexParamAudioPcm:
		pAudioParams = (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;
		portIndex = pAudioParams->nPortIndex;
		err = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pAudioParams, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
		if (err != OMX_ErrorNone)
		{
			omx_debug("In %s Parameter Check Error=%x\n", __func__, err);
			break;
		}
		if (portIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			port = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[portIndex];
			memcpy(&port->sAudioPcmMode, pAudioParams, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
		break;

	default:
		err = omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audio_asr_component_GetParameter(
	OMX_HANDLETYPE hComponent,
	OMX_INDEXTYPE nParamIndex,
	OMX_PTR ComponentParameterStructure)
{

	OMX_AUDIO_PARAM_PORTFORMATTYPE *pAudioPortFormat;
	OMX_AUDIO_PARAM_PCMMODETYPE *pAudioPcmMode;
	OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	omx_audio_asr_component_PortType *port;
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	if (ComponentParameterStructure == NULL)
	{
		return OMX_ErrorBadParameter;
	}
	omx_debug("   Getting parameter %i\n", nParamIndex);
	/* Check which structure we are being fed and fill its header */
	switch (nParamIndex)
	{
	case OMX_IndexParamAudioInit:
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone)
		{
			break;
		}
		memcpy(ComponentParameterStructure, &omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio], sizeof(OMX_PORT_PARAM_TYPE));
		break;
	case OMX_IndexParamAudioPortFormat:
		pAudioPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone)
		{
			break;
		}
		if (pAudioPortFormat->nPortIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			port = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[pAudioPortFormat->nPortIndex];
			memcpy(pAudioPortFormat, &port->sAudioParam, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
		break;
	case OMX_IndexParamAudioPcm:
		pAudioPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE))) != OMX_ErrorNone)
		{
			break;
		}
		if (pAudioPcmMode->nPortIndex <= omx_audio_asr_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts)
		{
			port = (omx_audio_asr_component_PortType *)omx_audio_asr_component_Private->ports[pAudioPcmMode->nPortIndex];
			memcpy(pAudioPcmMode, &port->sAudioPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
		}
		else
		{
			err = OMX_ErrorBadPortIndex;
		}
		break;
	case OMX_IndexParamStandardComponentRole:
		pComponentRole = (OMX_PARAM_COMPONENTROLETYPE *)ComponentParameterStructure;
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone)
		{
			break;
		}
		strcpy((char *)pComponentRole->cRole, ASR_COMP_ROLE);
		break;
	default:
		err = omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audio_asr_component_SetState(OMX_COMPONENTTYPE *openmaxStandComp,
											   OMX_U32 dest_state)
{
	OMX_ERRORTYPE ret = OMX_ErrorNone;
	/* Check which structure we are being fed and make control its header */
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_STATETYPE pre_state;

	omx_debug(" %p, %ld\n", openmaxStandComp, dest_state);

	if (dest_state == OMX_StateExecuting && (omx_audio_asr_component_Private->state == OMX_StateIdle ||
											 omx_audio_asr_component_Private->state == OMX_StatePause))
	{
		if (omx_audio_asr_component_Private->pStreaming != NULL)
		{
			// if (omx_audio_asr_component_Private->pStreaming != NULL ||
			//	omx_audio_asr_component_Private->pRecognizer != NULL) {
			omx_debug("Device not closed while entering StateIdle");
			omx_audio_asr_component_exit(openmaxStandComp);
		}
		omx_debug("audio asr init\n");
		ret = omx_audio_asr_component_init(openmaxStandComp);
		if (OMX_ErrorNone != ret)
			return ret;
	}

	pre_state = omx_audio_asr_component_Private->state;
	ret = omx_base_component_DoStateSet(openmaxStandComp, dest_state);
	if (dest_state == OMX_StatePause && pre_state == OMX_StateExecuting)
	{
		omx_debug("audio asr exit\n");
		omx_audio_asr_component_exit(openmaxStandComp);
	}
	if (dest_state == OMX_StateIdle &&
		(pre_state == OMX_StateExecuting || pre_state == OMX_StatePause))
	{
		omx_debug("audio asr exit\n");
		omx_audio_asr_component_exit(openmaxStandComp);
	}
	return ret;
}

/** This is the central function for component processing. It
 * is executed in a separate thread, is synchronized with
 * semaphores at each port, those are released each time a new buffer
 * is available on the given port.
 */
void *omx_audio_asr_BufferMgmtFunction(void *param)
{

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)param;
	omx_audio_asr_component_PrivateType *omx_audio_asr_component_Private = (omx_audio_asr_component_PrivateType *)openmaxStandComp->pComponentPrivate;
	omx_base_PortType *pInPort = (omx_base_PortType *)omx_audio_asr_component_Private->ports[OMX_AUDIO_ASR_INPUTPORT_INDEX];
	omx_base_PortType *pOutPort = (omx_base_PortType *)omx_audio_asr_component_Private->ports[OMX_AUDIO_ASR_OUTPUTPORT_INDEX];
	omx_sem_t *pInputSem = pInPort->pBufferSem;
	omx_sem_t *pOutputSem = pOutPort->pBufferSem;
	queue_t *pInputQueue = pInPort->pBufferQueue;
	queue_t *pOutputQueue = pOutPort->pBufferQueue;
	OMX_BUFFERHEADERTYPE *pOutputBuffer = NULL;
	OMX_BUFFERHEADERTYPE *pInputBuffer = NULL;
	OMX_BOOL isInputBufferNeeded = OMX_TRUE, isOutputBufferNeeded = OMX_TRUE;
	int inBufExchanged = 0, outBufExchanged = 0;

	/* checks if the component is in a state able to receive buffers */
	while (omx_audio_asr_component_Private->state == OMX_StateIdle || omx_audio_asr_component_Private->state == OMX_StateExecuting || omx_audio_asr_component_Private->state == OMX_StatePause ||
		   omx_audio_asr_component_Private->transientState == OMX_TransStateLoadedToIdle)
	{

		/*Wait till the ports are being flushed*/
		omx_thread_mutex_lock(&omx_audio_asr_component_Private->flush_mutex);
		while (PORT_IS_BEING_FLUSHED(pInPort) ||
			   PORT_IS_BEING_FLUSHED(pOutPort))
		{
			omx_thread_mutex_unlock(&omx_audio_asr_component_Private->flush_mutex);

			omx_debug("In %s 1 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n",
					  __func__, inBufExchanged, isInputBufferNeeded, outBufExchanged, isOutputBufferNeeded, pInputSem->semval, pOutputSem->semval);

			if (isOutputBufferNeeded == OMX_FALSE && PORT_IS_BEING_FLUSHED(pOutPort))
			{
				pOutPort->ReturnBufferFunction(pOutPort, pOutputBuffer);
				outBufExchanged--;
				pOutputBuffer = NULL;
				isOutputBufferNeeded = OMX_TRUE;
				omx_debug("Ports are flushing,so returning output buffer\n");
			}

			if (isInputBufferNeeded == OMX_FALSE && PORT_IS_BEING_FLUSHED(pInPort))
			{
				pInPort->ReturnBufferFunction(pInPort, pInputBuffer);
				inBufExchanged--;
				pInputBuffer = NULL;
				isInputBufferNeeded = OMX_TRUE;
				omx_debug("Ports are flushing,so returning input buffer\n");
			}

			omx_debug("In %s 2 signaling flush all cond iE=%d,iF=%d,oE=%d,oF=%d iSemVal=%d,oSemval=%d\n",
					  __func__, inBufExchanged, isInputBufferNeeded, outBufExchanged, isOutputBufferNeeded, pInputSem->semval, pOutputSem->semval);

			omx_sem_up(omx_audio_asr_component_Private->flush_all_condition);
			omx_sem_down(omx_audio_asr_component_Private->flush_condition);
			omx_thread_mutex_lock(&omx_audio_asr_component_Private->flush_mutex);
		}
		omx_thread_mutex_unlock(&omx_audio_asr_component_Private->flush_mutex);

		/*No buffer to process. So wait here*/
		if ((isInputBufferNeeded == OMX_TRUE && pInputSem->semval == 0) &&
			(omx_audio_asr_component_Private->state != OMX_StateLoaded && omx_audio_asr_component_Private->state != OMX_StateInvalid))
		{
			// Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
			omx_debug("Waiting for next input/output buffer\n");
			omx_sem_down(omx_audio_asr_component_Private->bMgmtSem);
		}
		if (omx_audio_asr_component_Private->state == OMX_StateLoaded || omx_audio_asr_component_Private->state == OMX_StateInvalid)
		{
			omx_debug("In %s Buffer Management Thread is exiting\n", __func__);
			break;
		}
		if ((isOutputBufferNeeded == OMX_TRUE && pOutputSem->semval == 0) &&
			(omx_audio_asr_component_Private->state != OMX_StateLoaded && omx_audio_asr_component_Private->state != OMX_StateInvalid) &&
			!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
		{
			// Signaled from EmptyThisBuffer or FillThisBuffer or some thing else
			omx_debug("Waiting for next input/output buffer\n");
			omx_sem_down(omx_audio_asr_component_Private->bMgmtSem);
		}
		if (omx_audio_asr_component_Private->state == OMX_StateLoaded || omx_audio_asr_component_Private->state == OMX_StateInvalid)
		{
			omx_debug("In %s Buffer Management Thread is exiting\n", __func__);
			break;
		}

		omx_debug("Waiting for input buffer semval=%d in %s\n", pInputSem->semval, __func__);
		if (pInputSem->semval > 0 && isInputBufferNeeded == OMX_TRUE)
		{
			omx_sem_down(pInputSem);
			if (pInputQueue->nelem > 0)
			{
				inBufExchanged++;
				isInputBufferNeeded = OMX_FALSE;
				pInputBuffer = dequeue(pInputQueue);
				if (pInputBuffer == NULL)
				{
					omx_info("In %s Had NULL output buffer!!\n", __func__);
					int ret;
					ret = omx_sem_timed_down(omx_audio_asr_component_Private->bMgmtSem, 1000);
					if (ret < 0)
					{
						if (ret == ETIMEDOUT)
						{
							omx_err("wait timeout!");
						}
						else
						{
							omx_err("wait error %d!", ret);
						}
					}
					continue;
				}
			}
		}
		/*When we have input buffer to process then get one output buffer*/
		if (pOutputSem->semval > 0 && isOutputBufferNeeded == OMX_TRUE)
		{
			omx_sem_down(pOutputSem);
			if (pOutputQueue->nelem > 0)
			{
				outBufExchanged++;
				isOutputBufferNeeded = OMX_FALSE;
				pOutputBuffer = dequeue(pOutputQueue);
				if (pOutputBuffer == NULL)
				{
					omx_info("Had NULL output buffer!! op is=%d,iq=%d\n", pOutputSem->semval, pOutputQueue->nelem);
					omx_sem_down(omx_audio_asr_component_Private->bMgmtSem);
					continue;
				}
			}
		}

		if (isInputBufferNeeded == OMX_FALSE)
		{
			if (pInputBuffer->hMarkTargetComponent != NULL)
			{
				if ((OMX_COMPONENTTYPE *)pInputBuffer->hMarkTargetComponent == (OMX_COMPONENTTYPE *)openmaxStandComp)
				{
					/*Clear the mark and generate an event*/
					(*(omx_audio_asr_component_Private->callbacks->EventHandler))(openmaxStandComp,
																				  omx_audio_asr_component_Private->callbackData,
																				  OMX_EventMark, /* The command was completed */
																				  1,			 /* The commands was a OMX_CommandStateSet */
																				  0,			 /* The state has been changed in message->messageParam2 */
																				  pInputBuffer->pMarkData);
				}
				else
				{
					/*If this is not the target component then pass the mark*/
					omx_audio_asr_component_Private->pMark.hMarkTargetComponent = pInputBuffer->hMarkTargetComponent;
					omx_audio_asr_component_Private->pMark.pMarkData = pInputBuffer->pMarkData;
				}
				pInputBuffer->hMarkTargetComponent = NULL;
			}
		}

		if (isInputBufferNeeded == OMX_FALSE && isOutputBufferNeeded == OMX_FALSE)
		{

			if (omx_audio_asr_component_Private->pMark.hMarkTargetComponent != NULL)
			{
				pOutputBuffer->hMarkTargetComponent = omx_audio_asr_component_Private->pMark.hMarkTargetComponent;
				pOutputBuffer->pMarkData = omx_audio_asr_component_Private->pMark.pMarkData;
				omx_audio_asr_component_Private->pMark.hMarkTargetComponent = NULL;
				omx_audio_asr_component_Private->pMark.pMarkData = NULL;
			}

			pOutputBuffer->nTimeStamp = pInputBuffer->nTimeStamp;
			if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_STARTTIME) == OMX_BUFFERFLAG_STARTTIME)
			{
				omx_debug("Detected  START TIME flag in the input buffer filled len=%d\n", (int)pInputBuffer->nFilledLen);
				pOutputBuffer->nFlags = pInputBuffer->nFlags;
				pInputBuffer->nFlags = 0;
			}

			if (omx_audio_asr_component_Private->state == OMX_StateExecuting)
			{
				if (omx_audio_asr_component_Private->BufferMgmtCallback && pInputBuffer->nFilledLen > 0)
				{
					(*(omx_audio_asr_component_Private->BufferMgmtCallback))(openmaxStandComp, pInputBuffer, pOutputBuffer);
				}
				else
				{
					/*It no buffer management call back the explicitly consume input buffer*/
					pInputBuffer->nFilledLen = 0;
				}
			}
			else if (!(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
			{
				omx_debug("In %s Received Buffer in non-Executing State(%x)\n", __func__, (int)omx_audio_asr_component_Private->state);
			}
			else
			{
				pInputBuffer->nFilledLen = 0;
			}

			if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS && pInputBuffer->nFilledLen == 0)
			{
				omx_debug("Detected EOS flags in input buffer filled len=%d\n", (int)pInputBuffer->nFilledLen);
				pOutputBuffer->nFlags = pInputBuffer->nFlags;
				pInputBuffer->nFlags = 0;
				(*(omx_audio_asr_component_Private->callbacks->EventHandler))(openmaxStandComp,
																			  omx_audio_asr_component_Private->callbackData,
																			  OMX_EventBufferFlag,	 /* The command was completed */
																			  1,					 /* The commands was a OMX_CommandStateSet */
																			  pOutputBuffer->nFlags, /* The state has been changed in message->messageParam2 */
																			  NULL);
				omx_audio_asr_component_Private->bIsEOSReached = OMX_TRUE;
			}
			if (omx_audio_asr_component_Private->state == OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
			{
				/*Waiting at paused state*/
				omx_sem_wait(omx_audio_asr_component_Private->bStateSem);
			}

			/*If EOS and Input buffer Filled Len Zero then Return output buffer immediately*/
			if ((pOutputBuffer->nFilledLen != 0) || ((pOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS) == OMX_BUFFERFLAG_EOS) || (omx_audio_asr_component_Private->bIsEOSReached == OMX_TRUE))
			{
				pOutPort->ReturnBufferFunction(pOutPort, pOutputBuffer);
				outBufExchanged--;
				pOutputBuffer = NULL;
				isOutputBufferNeeded = OMX_TRUE;
			}
		}

		if (omx_audio_asr_component_Private->state == OMX_StatePause && !(PORT_IS_BEING_FLUSHED(pInPort) || PORT_IS_BEING_FLUSHED(pOutPort)))
		{
			/*Waiting at paused state*/
			omx_sem_wait(omx_audio_asr_component_Private->bStateSem);
		}

		/*Input Buffer has been completely consumed. So, return input buffer*/
		if ((isInputBufferNeeded == OMX_FALSE) && (pInputBuffer->nFilledLen == 0))
		{
			pInPort->ReturnBufferFunction(pInPort, pInputBuffer);
			inBufExchanged--;
			pInputBuffer = NULL;
			isInputBufferNeeded = OMX_TRUE;

			if (arecord_commponent_get_stop_flag() == 1) {
				omx_msleep(10);
			}
		}
	}
	omx_debug("Out of %s of component %p\n", __func__, openmaxStandComp);
	return NULL;
}
