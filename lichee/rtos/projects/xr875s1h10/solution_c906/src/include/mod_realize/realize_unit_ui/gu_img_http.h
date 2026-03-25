/*********************************************************************************
  *Copyright(C),2014-2023,GuRobot
  *Author:  weigang
  *Date:  2023.09.03
  *Description:  图片的http请求处理
  *History:  
**********************************************************************************/
#include "gu_attr_type.h"
#include "gu_page.h"
#ifndef GU_IMG_HTTP_H
#define GU_IMG_HTTP_H

bool threadSetHttpImgFile(guAttrValueTypeImgSrc_t *img_src_value);
bool setImageCachePath(char *path);
void setImageCacheSize(int size);
void saveImgDiskCacheToKV(void);
void cleanImgDiskCache(void);
void checkIfcleanImageCache(void);
void destoryImgRequestTask(guPage_t *page);
void setImgMaxRequestNum(int num);
guImgRequestTask_t * remove_img_request_task(guPage_t *curPage, guAttrValueTypeImgSrc_t *img_src);

#endif 