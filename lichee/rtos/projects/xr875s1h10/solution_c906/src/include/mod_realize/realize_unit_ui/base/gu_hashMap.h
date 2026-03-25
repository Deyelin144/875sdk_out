/*********************************************************************************
  *Copyright(C),2014-2022,GuRobot
  *Author:  weigang
  *Date:  2022.02.13
  *Description:  HashMap 函数 
  *			现在没有实现，是用遍历实现的，后期要改成真正的hashMap
  *History:  
**********************************************************************************/

#ifndef GU_HASHMAP_H
#define GU_HASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif

//使用链表作为hashTable，性能低
#define USE_LINK_TABLE_AS_HASH_MAP 0
#define STATIC_HASH_TIME 1

#if USE_LINK_TABLE_AS_HASH_MAP
typedef struct hash {
    int count;
    struct hash_node *list; //第一个节点
}guHash_t;

struct hash_node
{
  char *key;
  void *value;
  struct hash_node *next;
};
#else
#include "../../realize_unit_hash/realize_unit_hash.h"
#define guHash_t hash_t
#endif
guHash_t *hash_create(int bucket);

int hash_set(guHash_t *h, const char *key, void *val);
void hash_destroy(guHash_t *h, void*destoryValueFunc);
void hash_del(guHash_t *h, const char *key, void*destoryValueFunc);
void *hash_get(guHash_t *h, const char *key);
void hash_debug_print(guHash_t *h);
char *hash_search(struct hash *h, const char *key, const char *from);


#if STATIC_HASH_TIME
void static_hash_time_start(void);
void static_hash_time_finished(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*GU_CSS_H*/