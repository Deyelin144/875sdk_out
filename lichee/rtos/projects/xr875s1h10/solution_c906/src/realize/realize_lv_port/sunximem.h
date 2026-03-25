/**
 * @file sunximem.h
 *
 */

 #ifndef SUNXIMEM_H
 #define SUNXIMEM_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /*********************
  *      INCLUDES
  *********************/
 #if 1
 
 #include "lvgl.h"
 
 /*********************
  *      DEFINES
  *********************/
 #define UINT32_UP_ALIGN8(x) (((x & 0x07) == 0) ? x : (x + (8 - (x & 0x07))))
 #define UINT32_UP_ALIGN64(x) (((x & 63) == 0) ? x : (x + (64 - (x & 63))))
 /**********************
  *      TYPEDEFS
  **********************/
 
 /**********************
  * GLOBAL PROTOTYPES
  **********************/
 int sunxifb_mem_init(void);
 void sunxifb_mem_deinit(void);
 void* sunxifb_mem_alloc(size_t size, char *label);
 void sunxifb_mem_free(void **data, char *label);
 void* sunxifb_mem_get_phyaddr(void *data);
 void sunxifb_mem_flush_cache(void *data, size_t size);
 
 /**********************
  *      MACROS
  **********************/
 
 #endif  /*USE_SUNXIFB_G2D*/
 
 #ifdef __cplusplus
 } /* extern "C" */
 #endif
 
 #endif /*SUNXIMEM_H*/
 
