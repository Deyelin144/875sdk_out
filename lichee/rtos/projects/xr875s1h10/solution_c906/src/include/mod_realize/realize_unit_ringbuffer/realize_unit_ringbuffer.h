
#ifndef _REALIZE_UNIT_RINGBUFFER_H_
#define _REALIZE_UNIT_RINGBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int num;
    int block_size;
} ringbuf_para_t;

typedef struct {
    const char *data;
    int size;
} ringbuf_data_t;

// 内部未使用锁，外部可根据情况决定是否使用
typedef struct ringbuf_obj {
    void *ctx;
    int (*read)(struct ringbuf_obj *obj, ringbuf_data_t *buf);
    int (*write)(struct ringbuf_obj *obj, const char *data, int len);
    int (*get_block_size)(struct ringbuf_obj *obj);
    unsigned char (*is_full)(struct ringbuf_obj *obj);
    unsigned char (*is_empty)(struct ringbuf_obj *obj);
    // 内部未作拷贝操作，read得到的数据用完后需要调用此接口通知
    int (*use_end)(struct ringbuf_obj *obj);
    // 设置忽略写入，write直接退出，不阻塞，数据不写入，用于异常情况打断write阻塞
    void (*ignore_write)(struct ringbuf_obj *obj, unsigned char is_ignore);
} ringbuf_obj_t;

ringbuf_obj_t *realize_unit_ringbuffer_new(ringbuf_para_t *para);
int realize_unit_ringbuffer_delete(ringbuf_obj_t **obj);

#ifdef __cplusplus
}
#endif

#endif
