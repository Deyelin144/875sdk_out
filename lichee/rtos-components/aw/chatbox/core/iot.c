#include <cJSON.h>
#include "iot.h"

const char* generate_iot_descript(void)
{
    const char* descript ="[{\"name\":\"Speaker\",\"description\":\"扬声器\",\"properties\":{\"volume\":{\"description\":\"当前音量值\",\"type\":\"number\"}},\"methods\":{\"SetVolume\":{\"description\":\"设置音量\",\"parameters\":{\"volume\":{\"description\":\"0到100之间的整数\",\"type\":\"number\"}}}}},{\"name\":\"Screen\",\"description\":\"Backlight\",\"properties\":{\"theme\":{\"description\":\"主题\",\"type\":\"string\"},\"brightness\":{\"description\":\"当前亮度百分比\",\"type\":\"number\"}},\"methods\":{\"SetTheme\":{\"description\":\"设置屏幕主题\",\"parameters\":{\"theme_name\":{\"description\":\"主题模式, light 或 dark\",\"type\":\"string\"}}},\"SetBrightness\":{\"description\":\"设置亮度\",\"parameters\":{\"brightness\":{\"description\":\"0到100之间的整数\",\"type\":\"number\"}}}}}]";

    return descript;
   
}
