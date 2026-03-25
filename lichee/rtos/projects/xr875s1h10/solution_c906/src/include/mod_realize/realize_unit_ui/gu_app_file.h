/*********************************************************************************
  *Copyright(C),2014-2023,GuRobot
  *Author:  weigang
  *Date:  2023.11.26
  *Description:  APP 文件
  *History:
**********************************************************************************/
#ifndef GU_APP_FILE_H
#define GU_APP_FILE_H
#include "../realize_unit_hash/realize_unit_hash.h"
int realize_unit_ui_read_app_file(const char *path, unsigned char ** data, unsigned int *size);
int realize_unit_ui_close_app_file(const char *app_path);
#endif /*GU_APP_FILE_H*/