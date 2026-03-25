
#ifndef __OTA_MSG_H__
#define __OTA_MSG_H__

// recovery通信相关，确定后不允许修改，否则可能造成升级失败
#define APP_DIR "/data/lfs"
#define LOCAL_UPDATE_DIR "/sdmmc"

#define UPGRADE_MSG_FILE_PREFIX "upgrade_msg_" // 文件名为upgrade_msg_ + 该文件的MD5值
#define UPGRADE_MSG_FILE_SUFFIX ".json"
#define UPGRADE_MSG_FILE_COMPLETE "upgrade_result.json"                         //升级结果文件
#define UPGRADE_MSG_FILE_COMPLETE_RECORD "upgrade_result_record.json"           //升级结果记录文件，开机复制结果文件到记录，并删除结果文件  
#define UPGRADE_STATUS_FILE APP_DIR"/upgrade_status_rcvy.json"

#define UPGRADE_STATUS_MSG_APP_FILE APP_DIR"/upgrade_status_msg_app.json"      //用于通知app升级状态

#endif