# LWIP 宏

宏说明参考：

https://www.nongnu.org/lwip/2_1_x/index.html

宏的默认值在opt.h 中，lwipopts.h文件中的宏定义会覆盖opt.h文件中的同名宏定义。

## 部分宏说明

### Memory options

MEM_SIZE : 堆内存分配可分配的总大小，mem_malloc\mem_free 等管理这些内存。

MEM_LIBC_MALLOC : 开启这个宏会使用libc 的malloc\free 进行动态内存分配，而不使用mem_malloc\mem_free等堆内存管理模块的代码。

MEMP_USE_CUSTOM_POOLS : 是否包含用户文件 lwippools.h,该文件定义了 lwIp所需的“标准”池以外的其他池。如果将其设置为 1，则必须在某个包含路径中包含 lwippools.h。

MEMP_MEM_MALLOC : 使用mem_malloc 等堆内存分配方法代替memp_malloc等池内存分配方法。

MEM_USE_POOLS : 使用内存池分配代替mem_malloc，依赖MEMP_USE_CUSTOM_POOLS == 1。

MEM_ALIGNMENT : 字节对齐，根据架构进行设置。例如，如果是32位架构，这个宏的值设置为4。

### Internal Memory Pool Sizes

MEMP_NUM_TCP_PCB : 设置最大同时存在的tcp连接数量。

MEMP_NUM_UDP_PCB : 设置最大同时存在的udp连接数量。

MEMP_NUM_RAW_PCB : 设置最大同时存在的raw连接数量。

MEMP_NUM_NETCONN : 设置tcp、udp和raw连接总数。是MEMP_NUM_TCP_PCB、MEMP_NUM_UDP_PCB和MEMP_NUM_RAW_PCB 之和。

MEMP_NUM_TCP_PCB_LISTEN : 设置最大同时处于监听状态的tcp 连接数量。

MEMP_NUM_REASSDATA : 设置同时排队等待重组的IP报文数。

MEMP_NUM_FRAG_PBUF : 设置同时发出的分片ip报文数量。

MEMP_NUM_TCP_SEG : 设置池内存分配时最大可分配的tcp报文段数量。

MEMP_NUM_TCPIP_MSG_INPKT : 设置tcpip_msg结构体的数量，用于报文输入。

MEMP_NUM_TCPIP_MSG_API : 设置tcpip_msg结构体的数量，用于callback/timeout api消息。

### ARP options

ARP_TABLE_SIZE ：设置arp缓存表的大小。

ARP_QUEUEING : 启用这个宏可以在arp解决mac地址问题过程中缓存上层发送的报文；禁用这个宏则只会保留最近上层发送的报文。

MEMP_NUM_ARP_QUEUE : 设置arp解决mac地址问题过程中，缓存的pbuf数量。开启ARP_QUEUEING 后生效。

ETHARP_TRUST_IP_MAC : 启用-IP报文可以更新arp表；禁止-只有arp respone报文可以更新arp表。

ETH_PAD_SIZE : 设置在以太网报文头部之前添加的字节数，确保字节对齐。

ETHARP_SUPPORT_STATIC_ENTRIES : 启用后，支持应用通过etharp_add_static_entry和 etharp_remove_static_entry接口更新arp缓存表。

### IP options

IP_REASSEMBLY : 启用后支持重组分片ip报文。

IP_FRAG : 启用后支持ip分片。

IP_REASS_MAXAGE : 收到一个分片报文后等待其他所有分片报文到达的最大等待时间，超时后会丢弃该报文。

IP_REASS_MAX_PBUFS ：设置输入的分片报文最大分片数量。

IP_FRAG_USES_STATIC_BUF : 启用-使用静态内存进行ip分片；禁止-使用动态内存进行ip分片。

IP_FRAG_MAX_MTU : 设置ip报文分片阈值。

IP_DEFAULT_TTL : 设置ip报文默认的存活时间。

LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS ：启用后，使用随机数初始化端口号。依赖随机数生成器（1.4.1版本和2.0.3版本）。2.1.2版本是LWIP_RAND。

### ICMP options

LWIP_ICMP : 使能icmp模块。

ICMP_TTL : 设置icmp消息的ttl值。

### RAW options

LWIP_RAW : 使能原始套接字功能。

RAW_TTL : 设置原始套接字消息的ttl值。

### DHCP options

LWIP_DHCP : 启用dhcp模块。

### AUTOIP options

LWIP_AUTOIP : 启用或者禁止AUTOIP模块。

### Multicast options

LWIP_IGMP : 启用igmp模块。

LWIP_SNTP : 启用sntp模块。

### DNS options

LWIP_DNS : 启用dns客户端模块。

DNS_TABLE_SIZE : 设置DNS缓存表的大小。

DNS_MAX_NAME_LENGTH : 设置域名的最大长度。域名最大长度不能超过255。

DNS_MAX_SERVERS : 设置dns服务器数量。

### UDP options

LWIP_UDP : 使能udp模块。

UDP_TTL : 设置udp消息的ttl值。

### TCP options

LWIP_TCP : 使能tcp模块。

TCP_TTL : 设置tcp消息的ttl值。

TCP_WND : 设置TCP窗口的大小。

TCP_MAXRTX : 设置tcp报文的最大重传次数。

TCP_SYNMAXRTX : 设置tcp syn报文最大重传次数。

TCP_QUEUE_OOSEQ : 支持将收到的乱序报文进行缓存。

TCP_MSS : 设置tcp报文的最大分段大小。

TCP_SND_BUF : 设置tcp发送数据缓冲区的大小。

TCP_SND_QUEUELEN : 设置tcp报文发送队列长度。

TCP_OOSEQ_MAX_BYTES : 设置每个pcb的ooseq上面缓存的最大字节数，设置为0则没有限制；TCP_QUEUE_OOSEQ  == 1时生效。

TCP_OOSEQ_MAX_PBUFS : 设置每个pcb的ooseq上面缓存的最大pbuf数量，设置为0则没有限制；TCP_QUEUE_OOSEQ  == 1时生效。

TCP_LISTEN_BACKLOG : 已收到连接请求而未建立连接的最大设备数量。

LWIP_TCP_SACK_OUT : tcp支持sack功能。

LWIP_TCP_MAX_SACK_NUM : tcp报文上携带sack值的最大个数。依赖LWIP_TCP_SACK_OUT 使能。

### Pbuf options

PBUF_LINK_HLEN : 给链路层分配的头部长度。

### Network Interfaces options

LWIP_NETIF_API : 支持netif api。

### Thread options

TCPIP_THREAD_NAME ; tcpip线程名字。

DEFAULT_RAW_RECVMBOX_SIZE : 每一个raw连接维持一个接收报文队列，这个设置接收报文队列的长度。

DEFAULT_UDP_RECVMBOX_SIZE ：每一个udp连接维持一个接收报文队列，这个设置接收报文队列的长度。

DEFAULT_TCP_RECVMBOX_SIZE : 每一个tcp连接维持一个接收报文队列，这个设置接收报文队列的长度。

DEFAULT_ACCEPTMBOX_SIZE : 成功建立的tcp连接会加入该队列中，等待应用层accept。

TCPIP_MBOX_SIZE : 设置tcpip线程的消息队列长度。应用层api消息和驱动的报文输入消息都会进入这个消息队列中。

### Sequential layer options

LWIP_TCPIP_TIMEOUT : 启用运行在tcpip线程上任意自定义的定时器处理函数。

### Socket options

LWIP_COMPAT_SOCKETS : 使能bsd风格的套接字接口函数名。

### Statistics options

LWIP_STATS : 统计所有在线连接。



## 优化内存

调整PBUF_POOL_SIZE的值（即所需的pbuf数量）

根据MTU设置PBUF_POOL_BUFSIZE的值

根据所需要的TCP、UDP和RAW的连接数设置MEMP_NUM_TCP_PCB、MEMP_NUM_UDP_PCB和MEMP_NUM_RAW_PCB。并且保证MEMP_NUM_NETCONN 等于三者之和。

调整TCPIP_MBOX_SIZE的值。

调整TCPIP_NUM_TCPIP_MSG_INPKT的值。

关掉所有LWIP_DEBUG选项，并且不定义LWIP_DEBUG。

根据实际情况调整MEM_SIEZ。

**如何修改：**

待补充。

















