#ifndef __REALZIE_UNIT_HASH_H__
#define __REALZIE_UNIT_HASH_H__

#define DEFAULT_HASH_SIZE	50	

typedef enum {
    HASH_TYPE_UINT_32 = 0,
    HASH_TYPE_UINT_64,
    HASH_TYPE_STRING,
} hash_type_t;


typedef struct hash_node {
	unsigned long hash_val;       //计算出来的hash值
	void *value;            //
    void *key;              //key
    unsigned int key_len;
    unsigned char is_collision;         //是否碰撞，头节点使用
	struct hash_node* next;
} hash_node_t;


typedef unsigned int (*get_hash_t)(unsigned int, void*);

typedef void (*free_node_cb)(void *);

typedef struct hash {
    hash_type_t type;
	unsigned int arraysize; //对应的桶的大小
    hash_node_t **head_node;
} hash_t;


/**
 * 创建哈希表
 * @param type：key的类型， arraysize：数组大小
 * @return 哈希表句柄
 */
hash_t *realize_unit_hash_create_table(hash_type_t type, unsigned int arraysize);

/**
 * 删除特定key的结点
 * @param type：key 值， key_size:key的size, 
 *      destoryValueFun:销毁value的函数指针,如果为NULL，表示不销毁内容
 *  
 * @return 哈希表句柄
 */
int realize_unit_hash_delete_node(hash_t *hash, void *key, free_node_cb cb);

//添加结点
int realize_unit_hash_add_node(hash_t *hash, void *key, void *value, unsigned int value_size);

//获取特定key节点的值
void *realize_unit_hash_get_value(hash_t* hash, void* key);

//销毁哈希表, destoryValueFun:销毁value的函数指针
void realize_unit_hash_destory_table(hash_t *hash, free_node_cb cb);

//通过idx以及当前获取到的hash-node获取下一个hash-node
hash_node_t* realize_unit_hash_traversal_node(hash_t* hash, unsigned int idx, hash_node_t* curr_node);

void printhash(hash_t* hash1);
#endif
