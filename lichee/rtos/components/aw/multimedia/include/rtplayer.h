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

#ifndef RTPLAYER_H
#define RTPLAYER_H

#include <xplayer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  *The response of state change notices APP what current state of player.
  */
typedef enum RTplayerNotifyAppType
{
    RTPLAYER_NOTIFY_PREPARED			          = 0,
    RTPLAYER_NOTIFY_PLAYBACK_COMPLETE       = 1,
    RTPLAYER_NOTIFY_SEEK_COMPLETE		 = 2,
    RTPLAYER_NOTIFY_MEDIA_ERROR			 = 3,
    RTPLAYER_NOTIFY_NOT_SEEKABLE			 = 4,
    RTPLAYER_NOTIFY_BUFFER_START			 = 5, /*this means no enough data to play*/
    RTPLAYER_NOTIFY_BUFFER_END			 = 6, /*this means got enough data to play*/
    RTPLAYER_NOTIFY_DOWNLOAD_START		 = 7,//not support now
    RTPLAYER_NOTIFY_DOWNLOAD_END	         = 8,//not support now
    RTPLAYER_NOTIFY_DOWNLOAD_ERROR           = 9,//not support now
    RTPLAYER_NOTIFY_AUDIO_FRAME                    = 10,//notify the decoded audio frame
    RTPLAYER_NOTIFY_DETAIL_INFO = 11,
}RTplayerNotifyAppType;

typedef enum RTplayerMediaErrorType
{
    RTPLAYER_MEDIA_ERROR_UNKNOWN			= 1,
    RTPLAYER_MEDIA_ERROR_OUT_OF_MEMORY	= 2,//not support now
    RTPLAYER_MEDIA_ERROR_IO				= 3,
    RTPLAYER_MEDIA_ERROR_UNSUPPORTED	= 4,
    RTPLAYER_MEDIA_ERROR_TIMED_OUT		= 5,//not support now
}RTplayerMediaErrorType;

typedef enum XplayerSoundCardType {
    CARD_DEFAULT = 0,
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_MULTI_PCM
    CARD_MULTI  = 6,
#endif
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_AMP
    CARD_AMP_PB = 7,
#endif
#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM_HW_BT
    CARD_BT_SRC = 8,
#endif
} XplayerSoundCardType;

/*
typedef struct AudioPcmData
{
    unsigned char* pData;
    unsigned int   nSize;
    unsigned int samplerate;
    unsigned int channels;
    int accuracy;
} AudioPcmData;
*/

/** define of player_callback_t function
*
* @param userData playback set by setDataSource
* @param msg audio player callback to smartbox
* @param id playback set by setDataSource
* @param ext1 not use now
* @param ext2 not use now
*/
typedef void (*player_callback_t)(void* userData,int msg, int id, int ext1, int ext2);


typedef struct RTPlayerContext
{
    XPlayer*					mXPlayer;
    void*						mUserData;
    int                            mId;
    player_callback_t		mNotifier;
    SoundCtrl*			    mSoundCtrl;
    MediaInfo*                  mMediaInfo;
}RTPlayer;

typedef  int status_t;

/** init player on smartbox that create framework audio player
*
* @return framework audio player handle if success; otherwise return NULL
*/
void *player_init(void);


/** set data source to audio player used by URL and local prompt
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param url the link that playback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t setDataSource_url(void* handle,void* userData, const char *url, int id);


/** set data source to audio player used by TTS
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t setDataSource(void* handle,void* userData, int id);


/** prepare the framework audio player sync mode
*
* @param handle create at player_init function
* @param userData indicate the smarbox class that when callback used
* @param id indicate the smarbox playback index use when callback
* @return 0 if success; otherwise return the error code
*/
status_t prepare(void* handle);


/** prepare the framework audio player async mode and should callback notify smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t prepareAsync(void* handle);


/** start playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t start(void* handle);


/** stop playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t stop(void* handle);


/** pause playback called by smartbox
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t pause_l(void* handle);


/** playback seek function called by smartbox
*
* @param handle create at player_init function
* @param sec that seek to time second
* @return 0 if success; otherwise return the error code
*/
status_t seekTo(void* handle, int sec);


/** reset the framwork function when playback end
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t reset(void* handle);


MediaInfo*  getMediaInfo(void* handle);

/** set the framework audio player to looping mode
*
* @param handle create at player_init function
* @return 0 if success; otherwise return the error code
*/
status_t setLooping(void* handle, int loop);

/** set play sound card,should used before setDataSource_url
*
* @param handle create at player_init function
* @param card target soundcard
* @return 0 if success; otherwise return the error code
*/
status_t setSoundCard(void* handle, int card);

/** get play sound card,should used before setDataSource_url
*
* @param handle create at player_init function
* @return -1 if fail; otherwise return the card num
*/
int getSoundCard(void* handle);

/** change play sound card,should used after play
*
* @param handle create at player_init function
* @param card target soundcard
* @return 0 if success; otherwise return the error code
*/
status_t changeSoundCard(void* handle, int card);

/** control sound card
*
* @param handle create at player_init function
* @param cmd sound card command
* @param para maybe used
* @return 0 if success; otherwise return the error code
*/
status_t setSoundControl(void* handle, int cmd, void *para);

/** get music total duration
*
* @param handle create at player_init function
* @param sec indicate the music total duration
* @return 0 if success; otherwise return the error code
*/
status_t getDuration(void* handle, int *sec);


/** get current playback duration
*
* @param handle create at player_init function
* @param sec indicate the music current playback duration
* @return 0 if success; otherwise return the error code
*/
status_t getCurrentPosition(void* handle, int *sec);


/** deinit player on smartbox
*
* @param handle create at player_init function
*/
void player_deinit(void* handle);


/** write data to audio player used by TTS
*
* @param handle create at player_init function
* @param buffer the point that TTS data store
* @param size the size of TTS data normally the size is 2048
* @return 0 if success; otherwise return the error code
*/
status_t WriteData(void* handle, unsigned char* buffer, int size);


/** write data to audio player used by TTS and the end
*
* @param handle create at player_init function
* @param buffer the point that TTS data store
* @param size the size of TTS data normally the size is less than 2048
* @return 0 if success; otherwise return the error code
*/
status_t writeDatawithEOS(void* handle, unsigned char* buffer, int size);


/** get the ring buffer available size
*
* @param handle create at player_init function
* @return the available size
*/
long long int getAvailiableSize(void* handle);


/** register callback that framework callback to smartbox
*
* @param handle create at player_init function
* @param fn callback function to framework audio player
*/
void registerCallback(void* handle, void* userData, player_callback_t fn);

void player_show_buffer(void);

#ifdef __cplusplus
}
#endif

#endif
