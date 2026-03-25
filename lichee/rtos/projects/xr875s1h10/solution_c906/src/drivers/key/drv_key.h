#ifndef _DRV_KEY_H_
#define _DRV_KEY_H_

typedef void (*irq_callback)(void *arg);
typedef struct {
    char *name;
    void (*init)(uint8_t key_type, irq_callback cb);
    void (*scan)(void);
    void (*deinit)(void);
} drv_key_ops_t;

void drv_key_matrix_ops_register(drv_key_ops_t* ops);
void drv_key_adc_ops_register(drv_key_ops_t* ops);

#endif /* _DRV_KEY_H_ */
