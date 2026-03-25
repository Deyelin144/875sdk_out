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

#define LOG_TAG "rtplayer"
#include <stdlib.h>
#include "rtplayer.h"
#include "rtosSoundControl.h"
#include "cedarx.h"

int CallbackFromXPlayer(void* pUserData, int msg, int ext1, void* param){
    RTPlayer* rtplayer = (RTPlayer*)pUserData;
    int appMsg = -1;
    logd("msg = %d,ext1 = %d",msg,ext1);
    switch(msg)
    {
        case AWPLAYER_MEDIA_PREPARED:
        {
            appMsg = RTPLAYER_NOTIFY_PREPARED;
            logd("prepared completed");
            break;
        }

        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
        {
            appMsg = RTPLAYER_NOTIFY_PLAYBACK_COMPLETE;
            logd("playback completed");
            break;
        }

        case AWPLAYER_MEDIA_SEEK_COMPLETE:
        {
            appMsg = RTPLAYER_NOTIFY_SEEK_COMPLETE;
            logd("seek completed");
            break;
        }

        case AWPLAYER_MEDIA_ERROR:
        {
            appMsg = RTPLAYER_NOTIFY_MEDIA_ERROR;
            loge("media error");
            switch(ext1)
            {
                case AW_MEDIA_ERROR_UNKNOWN:
                {
                    ext1 = RTPLAYER_MEDIA_ERROR_UNKNOWN;
                    break;
                }
                case AW_MEDIA_ERROR_UNSUPPORTED:
                {
                    ext1 = RTPLAYER_MEDIA_ERROR_UNSUPPORTED;
                    break;
                }
                case AW_MEDIA_ERROR_IO:
                {
                    ext1 = RTPLAYER_MEDIA_ERROR_IO;
                    break;
                }
            }
            break;
        }
        case AWPLAYER_MEDIA_INFO:
        {
            switch(ext1)
            {
                case AW_MEDIA_INFO_NOT_SEEKABLE:
                {
                    appMsg = RTPLAYER_NOTIFY_NOT_SEEKABLE;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_START:
                {
                    appMsg = RTPLAYER_NOTIFY_BUFFER_START;
                    break;
                }
                case AW_MEDIA_INFO_BUFFERING_END:
                {
                    appMsg = RTPLAYER_NOTIFY_BUFFER_END;
                    break;
                }
                case AW_MEDIA_INFO_DOWNLOAD_ERROR:
                {
                    appMsg = RTPLAYER_NOTIFY_DOWNLOAD_ERROR;
                    break;
                }
                case AW_MEDIA_INFO_DETAIL:
                {
                    appMsg = RTPLAYER_NOTIFY_DETAIL_INFO;
                    break;
                }
            }
            break;
        }

        default:
        {
            //logw("warning: unknown callback from xplayer,msg = %d\n",msg);
            break;
        }
    }
    if(appMsg != -1){
        rtplayer->mNotifier(rtplayer->mUserData,appMsg,rtplayer->mId,ext1,(int)(intptr_t)param);
    }
    return 0;
}


/** init player on smartbox that create framework audio player
*
* @return framework audio player handle if success; otherwise return NULL
*/
extern int XPlayer_List_Init_Flag;
void *player_init(void)
{
    if(XPlayer_List_Init_Flag==0){
        XPlayer_List_Init_Flag = 1;
        CedarxStreamListInit();
        CedarxStreamRegisterHttps();
        CedarxStreamRegisternewSsl();
        // CedarxStreamRegisterSsl();
        //CedarxStreamRegisterFlash();
        CedarxStreamRegisterFile();
        // CedarxStreamRegisterFifo();
        CedarxStreamRegisternewFifo();
        CedarxStreamRegisterHttp();
        CedarxStreamRegisterTcp();
        CedarxStreamRegisterCustomer();

        CedarxParserListInit();
        CedarxParserRegisterM3U();
        CedarxParserRegisterM4A();
        CedarxParserRegisterOGG();
        CedarxParserRegisterAAC();
        CedarxParserRegisterAMR();
        CedarxParserRegisterMP3();
        CedarxParserRegisterFLAC();
        CedarxParserRegisterTS();
        CedarxParserRegisterWAV();

        CedarxDecoderListInit();
        CedarxDecoderRegisterAAC();
        CedarxDecoderRegisterAMR();
        CedarxDecoderRegisterMP3();
        CedarxDecoderRegisterFLAC();
        CedarxDecoderRegisterOPUS();
        CedarxDecoderRegisterWAV();
        CedarxDecoderRegisterOGG();

        CedarxRenderListInit();
        CedarxRenderRegisterAudioSystem();
#ifdef CONFIG_SUN20IW2_BT_CONTROLLER
        CedarxRenderRegisterBt();
#endif
    }

    RTPlayer* mPrivateData = (RTPlayer*)malloc(sizeof(RTPlayer));
    if(mPrivateData == NULL){
        loge("malloc TPlayer fail\n");
        return NULL;
    }
    logd("player_init\n");
    memset(mPrivateData,0x00,sizeof(RTPlayer));
    mPrivateData->mUserData = NULL;
    mPrivateData->mNotifier = NULL;
    mPrivateData->mXPlayer = XPlayerCreate();
    if(mPrivateData->mXPlayer == NULL){
        loge("XPlayerCreate fail\n");
        free(mPrivateData);
        mPrivateData = NULL;
        return NULL;
    }
    int checkRet = XPlayerInitCheck(mPrivateData->mXPlayer);
    if(checkRet == -1){
        loge("the player init check fail\n");
        XPlayerDestroy(mPrivateData->mXPlayer);
        mPrivateData->mXPlayer = NULL;
        free(mPrivateData);
        mPrivateData = NULL;
        return NULL;
    }
    return mPrivateData;
}


/** set data source to audio player used by URL and local prompt
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param url the link that playback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t setDataSource_url(void* handle,void* userData, const char *url, int id){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    rtplayer->mUserData = userData;
    rtplayer->mId = id;
    int ret = 0;
    CdxKeyedVectorT *CdxHeader = NULL;

    CdxHeader = malloc(sizeof(CdxKeyedVectorT) + 1 * sizeof(KeyValuePairT));
    if (CdxHeader) {
        CdxHeader->size = 1;
        CdxHeader->item[0].key = "User-Agent";
        CdxHeader->item[0].val = "Mozilla/5.0 (FreeRTOS; OS 8.2.3) Xradio Xradio/1.0";
    }

    ret = XPlayerSetDataSourceUrl(rtplayer->mXPlayer, url, NULL, CdxHeader);

    if (CdxHeader)
        free(CdxHeader);

    return ret;
}

/** set customer stream to audio player used by URL and local prompt
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param url the link that playback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t setCustomerStream_url(void* handle,void* userData, const char *url, int id){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    rtplayer->mUserData = userData;
    rtplayer->mId = id;
    return XPlayerSetDataSourceStream(rtplayer->mXPlayer,url);
}

/** set data source to audio player used by TTS
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t setDataSource(void* handle,void* userData, int id){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    char tcpUrl[50] = "tcp:127.0.0.1:6666";
    rtplayer->mUserData = userData;
    rtplayer->mId = id;
    return XPlayerSetDataSourceUrl(rtplayer->mXPlayer,tcpUrl,NULL,NULL);
}


/** prepare the framework audio player sync mode
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t prepare(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerPrepare(rtplayer->mXPlayer);
}


/** prepare the framework audio player async mode and should callback notify smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t prepareAsync(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerPrepareAsync(rtplayer->mXPlayer);
}


/** start playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t start(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerStart(rtplayer->mXPlayer);
}


/** stop playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t stop(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerStop(rtplayer->mXPlayer);
}


/** pause playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t pause_l(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerPause(rtplayer->mXPlayer);
}


/** playback seek function called by smartbox
*
* @param handle create at player_init function
* @param sec that seek to time second
* @return 0 if success; otherwise return the error code
*/
status_t seekTo(void* handle, int sec){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerSeekTo(rtplayer->mXPlayer, sec*1000);
}


/** reset the framwork function when playback end
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t reset(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    return XPlayerReset(rtplayer->mXPlayer);
}


/** set the framework audio player to looping mode
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
MediaInfo*  getMediaInfo(void* handle)
{
    RTPlayer* rtplayer = (RTPlayer*)handle;
    MediaInfo* mi = XPlayerGetMediaInfo(rtplayer->mXPlayer);
    rtplayer->mMediaInfo = mi;
    return mi;
}

status_t setLooping(void* handle, int loop)
{
    RTPlayer* rtplayer = (RTPlayer*)handle;
    if(loop)
        return XPlayerSetLooping(rtplayer->mXPlayer,1);
    else
        return XPlayerSetLooping(rtplayer->mXPlayer,0);
}

/** set the framework audio player sound card
*
* @param handle create at player_init function
* @param card target sound card num
* @return 0 if success; otherwise return the error code
*/
status_t setSoundCard(void* handle, int card)
{
    RTPlayer* rtplayer = (RTPlayer*)handle;

    return XPlayerSetSoundCard(rtplayer->mXPlayer,card);
}

/** get the framework audio player sound card
*
* @param handle create at player_init function
* @return card num if success; otherwise return -1
*/
int getSoundCard(void* handle)
{
    RTPlayer* rtplayer = (RTPlayer*)handle;

    return XPlayerGetSoundCard(rtplayer->mXPlayer);
}

/** change the framework audio player sound card
*
* @param handle create at player_init function
* @param card target sound card num
* @return 0 if success; otherwise return the error code
*/
status_t changeSoundCard(void* handle, int card)
{
    RTPlayer* rtplayer = (RTPlayer*)handle;

    return XPlayerSwitchCard(rtplayer->mXPlayer,card);
}

/** get music total duration
*
* @param handle create at player_init function
* @param sec indicate the music total duration
* @return 0 if success; otherwise return the error code
*/
status_t getDuration(void* handle, int *sec){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    int mSec;
    int ret = XPlayerGetDuration(rtplayer->mXPlayer,&mSec);
    *sec = mSec/1000;
    return ret;
}


/** get current playback duration
*
* @param handle create at player_init function
* @param sec indicate the music current playback duration
* @return 0 if success; otherwise return the error code
*/
status_t getCurrentPosition(void* handle, int *sec){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    int mSec;
    int ret = XPlayerGetCurrentPosition(rtplayer->mXPlayer,&mSec);
    *sec = mSec/1000 + ((mSec%1000)>950 ? 1 : 0);
    return ret;
}


/** deinit player on smartbox
*
* @param handle create at player_init function
*/
void player_deinit(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    if(rtplayer->mXPlayer != NULL){
	    XPlayerDestroy(rtplayer->mXPlayer);
        rtplayer->mXPlayer = NULL;
    }
    free(rtplayer);
}


/** write data to audio player used by TTS
*
* @param handle create at player_init function
* @param buffer the point that TTS data store
* @param size the size of TTS data normally the size is 2048
* @return 0 if success; otherwise return the error code
*/
status_t WriteData(void* handle, unsigned char* buffer, int size){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    logd("writeData is not implement,to do...\n");
    return 0;
}


/** write data to audio player used by TTS and the end
*
* @param handle create at player_init function
* @param buffer the point that TTS data store
* @param size the size of TTS data normally the size is less than 2048
* @return 0 if success; otherwise return the error code
*/
status_t writeDatawithEOS(void* handle, unsigned char* buffer, int size){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    logd("writeDatawithEOS is not implement,to do...\n");
    return 0;
}


/** get the ring buffer available size
*
* @param handle create at player_init function
* @return the available size
*/
long long int getAvailiableSize(void* handle){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    logd("getAvailiableSize is not implement,to do...\n");
    return 0;
}


/** register callback that framework callback to smartbox
*
* @param handle create at player_init function
* @param fn callback function to framework audio player
*/
void registerCallback(void* handle, void* userData, player_callback_t fn){
    RTPlayer* rtplayer = (RTPlayer*)handle;
    rtplayer->mUserData = userData;
    rtplayer->mNotifier = fn;
    XPlayerSetNotifyCallback(rtplayer->mXPlayer, CallbackFromXPlayer, handle);
}

void player_show_buffer(void){

     XPlayerShowBuffer();
}

