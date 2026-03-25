
#ifndef __EXCEPTION_WRAPPER_H__
#define __EXCEPTION_WRAPPER_H__ 

typedef int (*exception_catch_t)(int ex_type);

typedef struct {
	exception_catch_t ex_catch;
} exception_wrapper_t;

#endif