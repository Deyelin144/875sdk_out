
#ifndef __REALIZE_UNIT_EXCEPTION_H__
#define __REALIZE_UNIT_EXCEPTION_H__ 

typedef enum {
    EX_OK,
    EX_JS_TIMEOUT, // js执行太久或队列满
    EX_OUT_OF_MEM, // 内存不足
    EX_BLOCK_TOO_LONG, // 阻塞太久
    EX_JS_ERROR, // js执行报错（不重启）
} ex_type_t;

int realize_unit_exception_throw(ex_type_t type);

#endif