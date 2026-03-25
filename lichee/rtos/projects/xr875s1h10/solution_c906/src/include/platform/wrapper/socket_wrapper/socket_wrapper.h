#ifndef _SOCKET_WRAPPER_H_
#define _SOCKET_WRAPPER_H_

#include "../../gulitesf_config.h"

#ifdef CONFIG_PLATFORM_RTOS
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/def.h"
// #include "lwip/tcp.h"
#endif
#ifdef CONFIG_PLATFORM_LINUX
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <netinet/tcp.h>
#endif
#ifdef CONFIG_PLATFORM_MAC
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#endif
#ifdef CONFIG_PLATFORM_WINDOWS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

#endif


