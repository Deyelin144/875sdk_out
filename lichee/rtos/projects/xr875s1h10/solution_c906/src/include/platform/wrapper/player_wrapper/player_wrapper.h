#ifndef _PLAYER_WRAPPER_H_
#define _PLAYER_WRAPPER_H_

#include "../../../mod_realize/realize_unit_player/realize_unit_player.h"

typedef enum {
    PLAYER_WRAPPER_STATUS_PAUSE = 0, //APLAYER_STATES_PAUSE,
    PLAYER_WRAPPER_STATUS_PLAYING = 1, //APLAYER_STATES_PLAYING,
    PLAYER_WRAPPER_STATUS_PREPARING = 2, //APLAYER_STATES_PREPARING,
    PLAYER_WRAPPER_STATUS_PREPARED = 3, //APLAYER_STATES_PREPARED,
    PLAYER_WRAPPER_STATUS_STOPPED = 4,  //APLAYER_STATES_STOPPED,
    PLAYER_WRAPPER_STATUS_RESUME = 5, //APLAYER_STATES_RESUME,
    PLAYER_WRAPPER_STATUS_INIT = 6, //APLAYER_STATES_INIT,
    PLAYER_WRAPPER_STATUS_NONE = 7, //APLAYER_STATES_NONE,
    PLAYER_WRAPPER_STATUS_COMPLETED = 8, //APLAYER_STATES_COMPLETED,
    PLAYER_WRAPPER_STATUS_ERROR = 9, //APLAYER_STATES_ERROR,
} wrapper_player_state_t;

typedef int (*create_t)(player_record_t* info);
typedef int (*stop_t)(player_record_t* info);
typedef int (*destroy_t)(player_record_t* info);
typedef int (*get_status_t)(player_record_t* info);
typedef int (*get_curr_offset_t)(player_record_t* info);
typedef int (*get_size_t)(player_record_t* info);
typedef int (*seek_t)(player_record_t* info);
typedef int (*set_rate_t)(player_record_t* info);
typedef int (*set_volume_t)(int volume);

typedef int (*stream_put_data_t)(char *data, unsigned int len, unsigned long audio_ts);


typedef struct {
	create_t create;
	stop_t stop;
	destroy_t destroy;
	get_status_t get_status;
	get_curr_offset_t get_curr_offset;
	get_size_t get_size;
	seek_t seek;
	set_rate_t set_rate;
	set_volume_t set_volume;
	stream_put_data_t stream_put_data;
} player_wrapper_t;

#endif
