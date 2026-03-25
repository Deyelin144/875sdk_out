一、AB 系统升级

1.ota-step-ab.fex, ota-subimgs-ab.cfg, sys_partition_xip_ab.fex 这 3 个文件是 AB 系统用的。

目录下存在默认可用 sys_partition_xip_ab.fex，如因环境更新无法使用或不存在，请按照以下方法配置。

2.目前 AB 系统支持 XIP 功能后，分区表改动需要将原本的分区复制一份*-b的形式。

即（arm-hpsram、dsp-hpsram、rv-hpsram、rtos-xip、config）增加为（arm-b、dsp-b、rv-b、xip-b、config-b）的格式，设置相同 downloadfile，设置相同size。

注：目前支持 XIP 后，AB 系统打包可能会超过 flash 大小，使用时可将不用的功能裁剪，或将 udisk 分区注释掉。

3.编译后，先 pack 打包，再执行 ota_pack -ab 命令，打包 ota 文件。将在 cout 命令对应的目录生成 ota/update 文件夹。

4.将 ota/update/ 文件夹下的所有文件放到服务器或者推到设备端用来升级。


二、主系统 + recovery 系统升级

1.ota-step-recovery.fex, ota-subimgs-recovery.cfg, sys_partition_xip_recovery.fex 这 3 个文件是 recovery 系统用的。

2.将sys_partition_xip_recovery.fex文件复制到 cconfigs 命令对应的目录，覆盖 sys_partition_xip.fex。

3.image_header 无需修改，在 c906 配置中选中 CONFIG_COMPONENTS_AW_OTA_V2_RECOVERY 选项，打包默认开启 recovery。

4.编译后，先 pack 打包，再执行 ota_pack -recovery 命令，打包 ota 文件。将在 cout 命令对应的目录生成 ota/update 文件夹。

5.将目录下 ota/update/ 文件夹下的所有文件放到服务器或者推到设备端用来升级。


三、安全固件升级

只需将 ota-subimgs-ab.cfg 或 ota-subimgs-recovery.cfg 文件的 out/r128s3/pro/image/boot0_spinor.fex:boot0。

改为 out/r128s3/pro/image/toc0.fex:boot0。先执行 pack -s 进行安全打包，然后执行 ota_pack -ab 或 ota_pack -recovery 命令打包。
