/*********************************************************************************
  *Copyright(C),2014-2024,GuRobot
  *Author:  weigang
  *Date:  2024.03.02
  *Description:  AC自动机算法
  *History:
**********************************************************************************/
// # AC自动机实现算法
// ## 目标
// 实现一套对内存消耗低的AC自动机算法
// ## 当前网上的算法存在的问题
// AC自动机原理参考链接 https://zhuanlan.zhihu.com/p/146369212
// 参考git算法 https://github.com/morenice/ahocorasick 改算法的Trie树的放在内存中的，每个字树固定了256个节点，一个Trie树的节点就需要2K字节


// ## 方案介绍
// * 为了节约内存，本方案将Trie树存在文件里，通过文件的seek来使用Trie，每个节点的子节点使用hash表保存，有多少就保留多少，不是固定的值
// * Trie树的创建使用Python语言生成好保存在文件里
// * 字典里的元素如果是英文，用单词表示（开源的一般使用字母表示），如果是中文用单个汉字表示

// 一个Trie树的节点描述

// 每个节点按广度优先依次保存，每个节点有一个唯一的索引，就是节点的保存的顺序
// 计算节点的偏移，就简单的 节点索引*单个节点的长度

// 单个节点长度11字节

// | 属性名称 | 长度 | 描述 | 字节序号 |
// |:-----|:----|:----|:----|
// | 是否尾节点 | 1bite | |0|
// | 单词长度 | 5bite |单词长度 |0|
// | 单词内容 | 3bite | 中文使用utf，英文保留3个字节，超过3个字节保存字典表的偏移量 |1|
// | 子节点个数 | 2byte | 最多65535个子节点 |4|
// | 首个子节点索引 | 3byte |  |6|
// | hash冲突节点索引 | 2byte |在同一层的索引|9|
// | fail节点索引 | 3byte |  |11|

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../realize_unit_fs/realize_unit_fs.h"
#ifndef GU_AC_H
#define GU_AC_H

#define NODE_SIZE 14
typedef struct ac_node
{
    bool is_end;
    bool is_root;
    int word_len;
    int fail_index;
    int collison_next_index;
    int hash_code;
    char *word;
    int child_count;
    int index;
    int child_first_index;
}ac_node_t;

int init_ac(char *ac_file, char *word_dict_file, unsigned int trie_buff_size);
int deinit_ac(void);
char *match_one_base_trie_file(char *source);

#endif