#ifndef _GIF_BIN_H_
#define _GIF_BIN_H_

#include <stdint.h>
#include "../../../lvgl.h"
#include "../../../misc/lv_fs.h"

typedef struct {
	void *gif_ctx;
    char *data;
    void *handle;
} gifbin_t;

gifbin_t *gifbin_open(const char *src);
void gifbin_close(void *handle);
void gifbin_rewind(void *handle);
int gifbin_render_frame(void *handle, unsigned char *buffer);
/* Return 1 if got a frame; 0 if got GIF trailer; -1 if error. */
int gifbin_get_frame(void *handle);
unsigned int gifbin_get_frame_delay(void *handle);

#endif
