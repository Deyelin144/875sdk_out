/*********************************************************************************
  *Copyright(C),2014-2024,GuRobot
  *Author:  weigang
  *Date:  2024.06.08
  *Description:  小型hash表实现
  *History:  
**********************************************************************************/
#ifndef _REALIZE_UNIT_MEM_STATISTICS_HASH_H_
#define _REALIZE_UNIT_MEM_STATISTICS_HASH_H_
//hash table实现, 和引擎的 realize_unit_hash实现不同，不用每次结点都malloc，而是一次性分配好
//key和value都存在着main_table中，当key冲突时，将冲突的key和value放到collison_table中
//main_table中的每个元素都是一个node，node中包含key和value
//collison_table中的每个元素都是一个collison_node，collison_node中包含key和value
//main_table和collison_table的大小是初始化时固定的
//main_table中的每个node的大小是固定的，collison_table中的每个collison_node的大小是固定的
//一个结点的大小是node_size + body_size

typedef int (*get_hash_code_fn)(void *key);
typedef struct mem_statistics_mini_hash_table_t {
    void *hash_table;
    void *collison_table;
    int  main_size;
    int collison_size;
    short node_size;
    short body_size;
    short key_size;
    char *empty_key;
    short collison_index;
    get_hash_code_fn get_hash_code;
} mem_statistics_mini_hash_table_t;

#define COLLISON_FIELD_LEN 2
mem_statistics_mini_hash_table_t * realize_unit_mem_statistics_hash_init(int hash_table_size, int collison_size, int body_size, int key_size, get_hash_code_fn get_hash_code);
int realize_unit_mem_statistics_hash_ins_node(void *hash, void *node, void *key);
int realize_unit_mem_statistics_hash_del_node(void *hash, void *key);
void *realize_unit_mem_statistics_hash_get_node_value(void *hash_p, void *key);
void realize_unit_mem_statistics_hash_destory(void *hash);
typedef void (*mini_hash_walker)(void *key, void *value, char * collison_index_str, char *table_name, int hash_table_index, void* user);
void realize_unit_mem_statistics_hash_print(void *hash_p, mini_hash_walker walker, void *user_data); 

#endif