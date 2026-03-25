#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#include <stdbool.h>
#include "player_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 播放列表句柄
 */
typedef struct playlist playlist_t;

/**
 * @brief 创建播放列表
 *
 * @return 成功: 播放列表句柄，失败: NULL
 */
playlist_t *playlist_create(void);

/**
 * @brief 销毁播放列表
 *
 * @param playlist 播放列表句柄
 *
 * @return 成功: 0, 失败: -1
 */
int playlist_destory(playlist_t *playlist);

/**
 * @brief 更新播放列表
 *
 * @note 播放列表更新时，原先列表会被清空，当前播放的音乐信息会被置为NULL
 *
 * @param playlist 播放列表句柄
 * @param list 音乐信息列表
 * @param count 音乐信息列表长度
 *
 * @return 成功: 0, 失败: -1
 */
int playlist_update(playlist_t *playlist, music_info_t *list, int count);

/**
 * @brief 添加音乐节点信息到播放列表指定位置(前面)
 *
 * @param playlist 播放列表句柄
 * @param info 音乐信息
 * @param pos 添加节点的位置
 *
 * @return 成功: 0, 失败: -1
 */
int playlist_add_node(playlist_t *playlist, music_info_t *info, int pos);

/**
 * @brief 删除播放列表指定位置的节点信息
 *
 * @param playlist 播放列表句柄
 * @param pos 删除节点的位置
 *
 * @return 成功: 0, 失败: -1
 */
int playlist_del_node(playlist_t *playlist, int pos);

/**
 * @brief 获取播放列表当前播放节点的音乐信息
 *
 * @param playlist 播放列表句柄
 * @param update_current 是否更新当前播放节点信息，默认将列表第一个节点设置为当前播放节点
 *
 * @return 成功: 音乐信息, 失败: NULL
 */
music_info_t *playlist_get_current(playlist_t *playlist, bool update_current);

/**
 * @brief 获取播放列表上一个节点的音乐信息
 *
 * @param playlist 播放列表句柄
 * @param update_current 是否更新当前播放节点信息
 *
 * @return 成功: 音乐信息, 失败: NULL
 */
music_info_t *playlist_get_prev(playlist_t *playlist, bool update_current);

/**
 * @brief 获取播放列表下一个节点的音乐信息
 *
 * @param playlist 播放列表句柄
 * @param update_current 是否更新当前播放节点信息
 *
 * @return 成功: 音乐信息, 失败: NULL
 */
music_info_t *playlist_get_next(playlist_t *playlist, bool update_current);

/**
 * @brief 获取播放列表指定位置的音乐信息
 *
 * @param playlist 播放列表句柄
 * @param pos 节点位置
 * @param update_current 是否更新当前播放节点信息
 *
 * @return 成功: 音乐信息, 失败: NULL
 */
music_info_t *playlist_get_node(playlist_t *playlist, int pos, bool update_current);

/**
 * @brief 获取播放列表随机节点的音乐信息
 *
 * @param playlist 播放列表句柄
 *
 * @return 成功: 音乐信息, 失败: NULL
 */
music_info_t *playlist_get_node_random(playlist_t *playlist);

/**
 * @brief 获取播放列表长度
 *
 * @param playlist 播放列表句柄
 *
 * @return 成功: 播放列表长度, 失败: -1
 */
int playlist_get_length(playlist_t *playlist);

/**
 * @brief 清空播放列表
 *
 * @param playlist 播放列表句柄
 *
 * @return 成功: 0, 失败: -1
 */
int playlist_clear(playlist_t *playlist);

#ifdef __cplusplus
}
#endif

#endif // __PLAYLIST_H__