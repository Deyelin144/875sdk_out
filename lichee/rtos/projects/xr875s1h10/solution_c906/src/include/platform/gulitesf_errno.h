#ifndef _GULIESF_ERRNO_H_
#define _GULIESF_ERRNO_H_

#include <errno.h>

/********************************************************************************
 * ps: 第一列：宏定义， 第二列：中文描述，方便理解， 第三列： 打印关键字，用于log输出
 * 例如：
 * 第一列：#define ENOMEM 12	
 * 第二列：内存不足
 * 第三列：Out of memory
*********************************************************************************/

#ifdef EOK
#undef EOK
#endif

#define EOK 0

/*
 *base errno
#define	EPERM		 1	操作不允许	Operation not permitted
#define	ENOENT		 2	没有这样的文件或目录	No such file or directory
#define	ESRCH		 3	没有这样的进程	No such process
#define	EINTR		 4	系统调用中断	Interrupted system call
#define	EIO		     5	I/O 错误	I/O error
#define	ENXIO		 6	没有这样的设备或地址	No such device or address
#define	E2BIG		 7	参数列表太长	Argument list too long
#define	ENOEXEC		 8	执行格式错误	Exec format error
#define	EBADF		 9	错误文件号	Bad file number
#define	ECHILD		10	没有子进程	No child processes
#define	EAGAIN		11	重试	Try again
#define	ENOMEM		12	内存不足	Out of memory
#define	EACCES		13	权限被拒绝	Permission denied
#define	EFAULT		14	错误地址	Bad address
#define	ENOTBLK		15	需要块设备	Block device required
#define	EBUSY		16	设备或资源繁忙	Device or resource busy
#define	EEXIST		17	文件存在	File exists
#define	EXDEV		18	跨设备链接	Cross-device link
#define	ENODEV		19	没有这样的设备	No such device
#define	ENOTDIR		20	不是目录	Not a directory
#define	EISDIR		21	是目录	Is a directory
#define	EINVAL		22	参数无效	Invalid argument
#define	ENFILE		23	文件表溢出	File table overflow
#define	EMFILE		24	打开的文件太多	Too many open files
#define	ENOTTY		25	不是打字机	Not a typewriter
#define	ETXTBSY		26	文本文件繁忙	Text file busy
#define	EFBIG		27	文件太大	File too large
#define	ENOSPC		28	设备上没有剩余空间	No space left on device
#define	ESPIPE		29	非法寻道	Illegal seek
#define	EROFS		30	只读文件系统	Read-only file system
#define	EMLINK		31	链接过多	Too many links
#define	EPIPE		32	管道损坏	Broken pipe
#define	EDOM		33	数学参数超出函数范围	Math argument out of domain of func
#define	ERANGE		34	数学结果无法表示	Math result not representable
*/

/*
 *标准扩展
#define EDEADLK 35	将发生资源死锁	Resource deadlock would occur
#define ENAMETOOLONG 36	文件名太长	File name too long
#define ENOLCK 37	没有可用的记录锁	No record locks available
#define ENOSYS 38	无效的系统调用号	Invalid system call number
#define ENOTEMPTY 39	目录不为空	Directory not empty
#define ELOOP 40	遇到过多的符号链接	Too many symbolic links encountered
#define EWOULDBLOCK EAGAIN	操作将阻塞	Operation would block
#define ENOMSG 42	没有所需类型的消息	No message of desired type
#define EIDRM 43	标识符已删除	Identifier removed
#define ECHRNG 44	通道号超出范围	Channel number out of range
#define EL2NSYNC 45	级别 2 未同步	Level 2 not synchronized
#define EL3HLT 46	级别 3 已停止	Level 3 halted
#define EL3RST 47	级别 3 已重置	Level 3 reset
#define ELNRNG 48	链接号超出范围	Link number out of range
#define EUNATCH 49	未连接协议驱动程序	Protocol driver not attached
#define ENOCSI 50	没有可用的 CSI 结构	No CSI structure available
#define EL2HLT 51	2 级停止	Level 2 halted
#define EBADE 52	无效交换	Invalid exchange
#define EBADR 53	无效请求描述符	Invalid request descriptor
#define EXFULL 54	交换已满	Exchange full
#define ENOANO 55	没有阳极	No anode
#define EBADRQC 56	无效请求代码	Invalid request code
#define EBADSLT 57	无效插槽	Invalid slot
#define EDEADLOCK EDEADLK		
#define EBFONT 59	字体文件格式错误	Bad font file format
#define ENOSTR 60	设备不是流	Device not a stream
#define ENODATA 61	没有可用数据	No data available
#define ETIME 62	计时器已过期	Timer expired
#define ENOSR 63	流资源不足	Out of streams resources
#define ENONET 64	机器不在网络上	Machine is not on the network
#define ENOPKG 65	软件包未安装	Package not installed
#define EREMOTE 66	对象是远程的	Object is remote
#define ENOLINK 67	链接已断开	Link has been severed
#define EADV 68	通告错误	Advertise error
#define ESRMNT 69	Srmount 错误	Srmount error
#define ECOMM 70	发送时发生通信错误	Communication error on send
#define EPROTO 71	协议错误	Protocol error
#define EMULTIHOP 72	尝试多跳	Multihop attempted
#define EDOTDOT 73	RFS 特定错误	RFS specific error
#define EBADMSG 74	不是数据消息	Not a data message
#define EOVERFLOW 75	值对于定义的数据类型来说太大	Value too large for defined data type
#define ENOTUNIQ 76	名称在网络上不唯一	Name not unique on network
#define EBADFD 77	文件描述符处于错误状态	File descriptor in bad state
#define EREMCHG 78	远程地址已更改	Remote address changed
#define ELIBACC 79	无法访问所需的共享库	Can not access a needed shared library
#define ELIBBAD 80	访问已损坏的共享库	Accessing a corrupted shared library
#define ELIBSCN 81	a.out 中的 .lib 部分已损坏	.lib section in a.out corrupted
#define ELIBMAX 82	尝试链接过多共享库	Attempting to link in too many shared libraries
#define ELIBEXEC 83	无法直接执行共享库	Cannot exec a shared library directly
#define EILSEQ 84	非法字节序列	Illegal byte sequence
#define ERESTART 85	中断的系统调用应重新启动	Interrupted system call should be restarted
#define ESTRPIPE 86	流管道错误	Streams pipe error
#define EUSERS 87	用户过多	Too many users
#define ENOTSOCK 88	非套接字上的套接字操作	Socket operation on non-socket
#define EDESTADDRREQ 89	需要目标地址	Destination address required
#define EMSGSIZE 90	消息太长	Message too long
#define EPROTOTYPE 91	套接字的协议类型错误	Protocol wrong type for socket
#define ENOPROTOOPT 92	协议不可用	Protocol not available
#define EPROTONOSUPPORT 93	协议不支持	Protocol not supported
#define ESOCKTNOSUPPORT 94	套接字类型不支持	Socket type not supported
#define EOPNOTSUPP 95	传输端点不支持操作	Operation not supported on transport endpoint
#define EPFNOSUPPORT 96	协议系列不支持	Protocol family not supported
#define EAFNOSUPPORT 97	协议不支持地址系列	Address family not supported by protocol
#define EADDRINUSE 98	地址已在使用中	Address already in use
#define EADDRNOTAVAIL 99	无法分配请求的地址	Cannot assign requested address
#define ENETDOWN 100	网络已关闭	Network is down
#define ENETUNREACH 101	网络无法访问	Network is unreachable
#define ENETRESET 102	网络因重置而断开连接	Network dropped connection because of reset
#define ECONNABORTED 103	软件导致连接中止	Software caused connection abort
#define ECONNRESET 104	对等方重置连接	Connection reset by peer
#define ENOBUFS 105	没有可用的缓冲区空间	No buffer space available
#define EISCONN 106	传输端点已连接	Transport endpoint is already connected
#define ENOTCONN 107	传输端点未连接	Transport endpoint is not connected
#define ESHUTDOWN 108	传输端点关闭后无法发送	Cannot send after transport endpoint shutdown
#define ETOOMANYREFS 109	引用过多：无法拼接	Too many references: cannot splice
#define ETIMEDOUT 110	连接超时	Connection timed out
#define ECONNREFUSED 111	连接被拒绝	Connection refused
#define EHOSTDOWN 112	主机已关闭	Host is down
#define EHOSTUNREACH 113	没有到主机的路由	No route to host
#define EALREADY 114	操作已在进行中	Operation already in progress
#define EINPROGRESS 115	操作正在进行中	Operation now in progress
#define ESTALE 116	过时的文件句柄	Stale file handle
#define EUCLEAN 117	结构需要清理	Structure needs cleaning
#define ENOTNAM 118	不是 XENIX 命名类型文件	Not a XENIX named type file
#define ENAVAIL 119	没有可用的 XENIX 信号量	No XENIX semaphores available
#define EISNAM 120	是命名类型文件	Is a named type file
#define EREMOTEIO 121	远程 I/O 错误	Remote I/O error
#define EDQUOT 122	超出配额	Quota exceeded
#define ENOMEDIUM 123	未找到介质	No medium found
#define EMEDIUMTYPE 124	介质类型错误	Wrong medium type
#define ECANCELED 125	“操作已取消”，异步操作在完成之前被取消	Operation Canceled
*/

/********************************************
 * 自定义扩展(公共部分)
 * 宏定义从1001开始，写法：EGLXXX
 * 范围 1001-2000
 ********************************************/
#define EGLGENERAL      1001    // 常规错误，下面没有的都归类到这里
#define EGLCREATEOBJ    1002    // 创建对象失败，例如：realize_unit_record_create返回null 可返回这个宏 obj create failed
#define EGLNOSUPPORT    1003    // 不支持的功能 unsupport
#define EGLFSOPT        1004	// 文件操作失败	Fs option falied
#define EGLTHREADCREATE 1005	// 线程创建失败	Thread create falied
#define EGLOPTCOPMP     1006	// 操作已完成	The operation was completed
#define EGLMUTEXCREATE  1007    // 互斥锁创建失败 Mutex create failed
#define EGLMUTEXLOCK    1008    // 互斥锁加锁失败 Mutex lock failed
#define EGLSEMCREATE    1009    // 信号量创建失败 Sem create failed
#define EGLSEMWAIT      1010    // 信号量获取失败 Sem wait failed
#define EGLSEMPOST      1011    // 信号量释放失败 Sem post failed
#define EGLNULLPOINTER  1012    // 空指针 Null pointer
#define EGLQUEUECREATE  1013    // 队列创建失败 Queue create failed
#define EGLQUEUESEND    1014    // 队列发送失败 Queue send failed
#define EGLQUEUERECV    1015    // 队列接收失败 Queue recv failed
#define EGLNODATA       1016    // 没有数据 No data
#define EGLDATAFORMAT   1017    // 数据格式错误，例如json解析失败，可以认为数据不是json格式 Data format error
#define EGLGETADDRINFO  1018    // 获取地址信息失败 Get addrinfo failed
#define EGLSOCKET       1019    // Socket失败 Socket failed
#define EGLCONNECT      1020    // 连接失败 Connect failed
#define EGLSEND         1021    // 发送失败 Send failed
#define EGLRECV         1022    // 接收失败 Receive failed
#define EGLSELECT       1023    // Select失败 Select failed
#define EGLGETHOST      1024    // 获取主机失败 Get host failed


/********************************************
 * 自定义扩展(模块特有部分)
 * 宏定义从2001开始，写法：EGLXXX
 * 范围 2001-3000
 ********************************************/

/********************************************
 * recode
 * 范围 2001-2020
 ********************************************/
#define EGLRECORDOPEN 2001	// 打开录音模块失败	record open falied
#define EGLRECORDRPCM 2002	// 录音时读取pcm失败	read pcm falied

/********************************************
 * player
 * 范围 2021-2040
 ********************************************/
#define EGLPLAYERNOST 2021 // 播放器未启动 Player not start
#define EGLPLAYERPUT 2022 // 播放器推送数据失败 Player put data failed
#define EGLPLAYERCREATE 2023 // 播放器创建失败 Player create failed
#define EGLPLAYERSEEK 2024 // 播放器seek失败 Player seek failed

/********************************************
 * 授权模块
 * 范围 2041-2060
 ********************************************/
#define EGLAUTHPATH         2041 // 授权路径为空 Path is null
#define EGLAUTHLOCALPARAM   2042 // 本地参数错误 Local param error
#define EGLAUTHCLOUDPARAM   2043 // 云端参数错误 Cloud param error
#define EGLAUTHEXPIRE       2044 // 鉴权过期 Auth is expired
#define EGLAUTHDEVICEINFO   2045 // 本地设备信息和云端设备信息不一致 Device info error
#define EGLAUTHCLOUDRESP    2046 // 云端响应信息有误 Cloud response error
#define EGLAUTHCLOUDFAIL    2047 // 云端鉴权失败 Cloud auth fail

/********************************************
 * 电池模块
 * 范围 2061-2080
 ********************************************/
#define EGLBATDIFFLARGE     2061 // 两次电量差距过大 Battery diff is too large
#define EGLBATCALIDIFFLARGE 2062 // 电量校准值差异较大 Battery cali diff is too large

/********************************************
 * 数据库模块
 * 范围 2081-2100
 ********************************************/
#define EGLDBOPEN 2081 // 数据库打开失败 DB open failed
#define EGLDBCLOSE 2082 // 数据库关闭失败 DB close failed
#define EGLDBOPT 2083 // 数据库操作失败 DB opt failed
#define EGLDBNUM 2084 // 数据库数量错误 DB num error


/********************************************
 * 设备device模块
 * 范围 2101-2120
 ********************************************/
#define EGLDEVSETVOLU 2101 // 设置音量失败 Set volume failed
#define EGLDEVSETHAND 2102 // 设置左右手失败 Set handedness failed
#define EGLDEVSNTP 2103 // SNTP请求失败 SNTP request failed
#define EGLDEVSETRTC 2104 // 设置RTC失败 Set rtc failed
#define EGLDEVSETBRT 2105 // 设置亮度失败 Set brightness failed

/********************************************
 * diskinfo模块
 * 范围 2121-2140
 ********************************************/
#define EGLDISKINFOGET 2121// 获取磁盘信息失败 Disk info get failed


/********************************************
 * 加解密模块
 * 范围 2141-2160
 ********************************************/
#define EGLENCSSLEST 2141 // ssl建立失败 Enc ssl establish failed
#define EGLENCSSLDES 2142 // ssl销毁失败 Enc ssl destroy failed
#define EGLENCSSLSEND 2143 // ssl发送失败 Enc ssl send failed
#define EGLENCSSLRECV 2144 // ssl接收失败 Enc ssl recv failed
#define EGLENCMD5INIT 2145 // md5初始化失败 Enc md5 init failed
#define EGLENCMD5APPEND 2146 // md5追加失败 Enc md5 append failed
#define EGLENCMD5FINISH 2147 // md5结束失败 Enc md5 finish failed
#define EGLENCAESINIT 2148 // aes初始化失败 Enc aes init failed
#define EGLENCAESCBC 2149 // aescbc加解密失败 Enc aes cbc failed
#define EGLENCAESFINISH 2150 // aes结束失败 Enc aes finish failed
#define EGLENCBASE64 2151 // base64编解码失败 ENc base64 encdec failed
#define EGLENCHMACSHA 2152 // hmac sha 失败 Enc hmacsha failed



/********************************************
 * fs模块
 * 范围 2161-2200
 ********************************************/
#define EGLFSMOUNT 2161 // 文件系统挂载失败 Fs mount failed
#define EGLFSOPEN 2162 // 文件打开失败 Fs open failed
#define EGLFSREAD 2163 // 文件读取失败 Fs read failed
#define EGLFSWRITE 2164 // 文件写入失败 Fs write failed
#define EGLFSSEEK 2165 // 文件seek失败 Fs seek failed
#define EGLFSTELL 2166 // 文件tell失败 Fs tell failed
#define EGLFSSYNC 2167 // 文件sync失败 Fs sync failed
#define EGLFSOPENDIR 2168 // 目录打开失败 Dir open failed
#define EGLFSREADDIR 2169 // 目录读取失败 Dir read failed
#define EGLFSMKDIR 2170 // 目录创建失败 Mkdir failed
#define EGLFSSTAT 2171 // 文件stat失败 Fs stat failed
#define EGLFSRENAME 2172 // 文件重命名失败 Fs rename failed
#define EGLFSREMOVE 2173 // 文件删除失败 Fs rm failed
#define EGLFSSPACE 2174 // 获取剩余空间失败 Get space failed
#define EGLFSFMT 2175 // 文件系统格式化失败 Fs format failed
#define EGLFSRMDIR 2176 // 目录删除失败 Dir rm failed
#define EGLFSCLOSE 2177 // 文件关闭失败 Fs close failed
#define EGLFSCLOSEDIR 2178 // 目录关闭失败 Dir close failed
#define EGLFSUNMOUNT 2179 // 文件系统卸载失败 Fs unmount failed
#define EGLFSCOPY     2180 // 文件复制失败  Fs file copy fail


/********************************************
 * http模块
 * 范围 2201-2230
 ********************************************/
#define EGLHTTPESTABLISH 2201 // 建立连接失败 Http establish failed
#define EGLHTTPCODE 2202 // 错误的响应码 Http wrong code
#define EGLHTTPNOBUFFER 2203 // buffer不够 Http buffer too small
#define EGLHTTPNOSOCK 2204 // Socket资源不够(用于agent) Socket is not enough

/********************************************
 * input模块
 * 范围 2231-2250
 ********************************************/
#define EGLINPUTNOBTN 2231 // 没有合适的按键 Input keyboard no button
#define EGLINPUTNOKB 2232 // 没有合适的键盘 Input no keyboard


/********************************************
 * kv模块
 * 范围 2251-2270
 ********************************************/
#define EGLKVINIT 2251 // kv初始化失败 Kv init failed
#define EGLKVSET 2252 // 设置kv失败 Kv set failed
#define EGLKVDEINIT 2253 // kv去初始化失败 Kv deinit failed


/********************************************
 * log模块
 * 范围 2271-2290
 ********************************************/
#define EGLLOGFLUSHBUFFER 2271 // 刷新缓冲区失败 Flush buffer fail
#define EGLLOGFILEROTATE 2272 // 文件滚动失败 File rotate fail
#define EGLLOGPAUSEUL 2273 // 暂停上传 Log upload pause

/********************************************
 * mqtt模块
 * 范围 2291-2310
 ********************************************/
#define EGLMQTTNETINIT 2291 // mqtt网络初始化失败 Mqtt network init failed
#define EGLMQTTCLIINIT 2292 // mqtt客户端初始化失败 Mqtt client init failed
#define EGLMQTTNETCONN 2293 // mqtt网络连接失败 Mqtt network connect failed
#define EGLMQTTCLICONN 2294 // mqtt客户端连接失败 Mqtt client connect failed
#define EGLMQTTCLISUB 2295 // mqtt客户端订阅失败 Mqtt client subscribe failed
#define EGLMQTTCLIUNSUB 2295 // mqtt客户端取消订阅失败 Mqtt client unsubscribe failed
#define EGLMQTTCLIPUB 2296 // mqtt客户端发布失败 Mqtt client publish failed
#define EGLMQTTCLIYIELD 2297 // mqtt客户端yield失败 Mqtt client yield failed

/********************************************
 * pm模块
 * 范围 2311-2330
 ********************************************/
#define EGLPMINIT 2311 // 初始化失败 Pm init failed
#define EGLPMGET 2312 // 获取唤醒源失败 Pm get wakeup src failed
#define EGLPMSET 2313 // 设置唤醒源失败 Pm set wakeup src failed

/********************************************
 * sleep模块
 * 范围 2331-2350
 ********************************************/

/********************************************
 * systimer模块
 * 范围 2351-2370
 ********************************************/
#define EGLTIMERCREATE 2351 // 定时器创建失败 Timer create failed
#define EGLTIMERSTART 2352 // 定时器启动失败 Timer start failed
#define EGLTIMERSTOP 2353 // 定时器停止失败 Timer stop failed

/********************************************
 * 透传模块
 * 范围 2371-2390
 ********************************************/
#define EGLTRSPSET 2371 // 透传设置失败 Trsp set failed

/********************************************
 * websocket模块
 * 范围 2391-2410
 ********************************************/
#define EGLWSCONN 2391 // ws连接失败 Ws connect failed
#define EGLWSSEND 2392 // ws发送失败 Ws send failed

/********************************************
 * wifi模块
 * 范围 2411-2470
 ********************************************/
#define EGLWIFINOTINIT       2411 // wifi未初始化 Wifi is not init
#define EGLWIFIINIT          2412 // wifi初始化失败 Wifi init failed
#define EGLWIFIGETSAVEWIFI   2413 // 获取保存wifi失败 Wifi get save wifi failed
#define EGLWIFIDISCONNECT    2414 // 断开连接失败 Wifi disconnect failed
#define EGLWIFICONNECT       2415 // 连接失败 Wifi connect failed
#define EGLWIFISCAN          5416 // 扫描失败 Wifi scan failed
#define EGLWIFIGETAPINFO     2417 // 获取ap信息失败 Wifi get ap info failed

/********************************************
 * zilb模块
 * 范围 2471-2490
 ********************************************/
#define EGLZLIBINIT 2471 // zlib初始化失败 Zlib init failed
#define EGLZLIBDECOM 2472 // zlib解压失败 Zlib decompress failed

/********************************************
 * lv_expand模块
 * 范围 2491-2530
 ********************************************/
#define EGLCAMOPEN 2491 // 摄像头打开失败 Camera open failed
#define EGLCAMFRM 2492 // 摄像头更新下一帧失败 Camera update next frame failed
#define EGLCAMCTL 2493 // 摄像头控制失败 Camera ctrl failed
#define EGLCAMCLOSE 2494 // 摄像头关闭失败 Camera close failed
#define EGLQRRESIZE 2495 // 二维码调整大小失败 QR resize failed

/********************************************
 * ui模块
 * 范围 2531-2630
 ********************************************/

/********************************************
 * amr编解码器
 * 范围 2631-2640
 ********************************************/
#define EGLAMRECODEINIT 2631	//AMR编码器初始化失败	amr encode init failed

/********************************************
 * SPX编解码器
 * 范围 2641-2650
 ********************************************/
#define EGLSPXECODEFREAM 2641 // SPX编码帧失败	spx encode frame failed
#define EGLSPXDECODEFREAM 2642	// SPX解码帧失败	spx decode frame failed
#define EGLNOSPXFORMAT 2643	// 不是SPX格式	no spx format

/********************************************
 * 线程池
 * 范围 2651-2660
 ********************************************/
#define EGLTHREADPOOLFULL 2651 // 线程池满 Thread pool full

/********************************************
 * List
 * 范围 2661-2670
 ********************************************/
#define EGLLISTINSERT 2661 // 链表插入失败 List insert failed
#define EGLLISTSEARCH 2662 // 链表搜索失败 List search failed

/********************************************
 * bh
 * 范围 2671-2680
 ********************************************/
#define EGLBHINSERT 2671 // 二叉堆插入失败 Bh insert failed
#define EGLBHSEARCH 2672 // 二叉堆搜索失败 Bh search failed
#define EGLBHFULL 2673 // 二叉堆满 Bh full
#define EGLBHEMPTY 2674 // 二叉堆空 Bh empty

/********************************************
 * flash
 * 范围 2681-2690
 ********************************************/
#define EGLFLASHDEV 2681 // flash设备表获取失败 Flash dev table get failed
#define EGLFLASHPAR 2682 // flash分区表获取失败 Flash partition table get failed

/********************************************
 * hash
 * 范围 2691-2700
 ********************************************/
#define EGLHASHADD 2691 // hash增加节点失败 Hash add node failed

/********************************************
 * scan
 * 范围 2701-2720
 ********************************************/
#define EGLSCANINIT 2701 // 扫描未初始化或初始化失败 Scan init failed
#define EGLSCANNLP 2702 // nlp失败 Scan nlp failed
#define EGLSCANSPD 2703 // 设置语速失败 Scan set speed failed
#define EGLSCANCTL 2704 // 控制失败 Scan control failed

/********************************************
 * serial_trans
 * 范围 2721-2730
 ********************************************/
#define EGLSRLINIT 2721 // 初始化失败 Serial trans init failed
#define EGLSRLRECV 2722 // 接收失败 Serial trans recv failed
#define EGLSRLSEND 2723 // 发送失败 Serial trans send failed

#endif  