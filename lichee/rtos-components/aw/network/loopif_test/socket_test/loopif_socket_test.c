/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "console.h"
#include "arch/sys_arch.h"

#define BUFF_SIZE 128

void hex_dump(char *pref, int width, unsigned char *buf, int len)
{
       int i,n;
    for (i = 0, n = 1; i < len; i++, n++){
        if (n == 1)
            printf("%s ", pref);
        printf("%2.2X ", buf[i]);
        if (n == width) {
            printf("\n");
            n = 0;
        }
    }
    if (i && n!=1)
        printf("\n");
}

void loop_if_server(void *unuse)
{
	char buff[BUFF_SIZE];
	int ret;
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

/*  Name the socket.  */

    server_address.sin_family = AF_INET;
//    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");;
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    ret = listen(server_sockfd, 5);
/*  Create a connection queue and wait for clients.  */

    printf("server waiting\n");

/*  Accept a connection.  */

	client_len = sizeof(client_address);
	client_sockfd = accept(server_sockfd,
	(struct sockaddr *)&client_address, &client_len);

	printf("[%s,%d],enter\n",__func__,__LINE__);

	printf("Creating loop_if_server_main task\n");

    while(1) {
        char ch;
        read(client_sockfd, buff, BUFF_SIZE);
		hex_dump(__func__,30,buff,BUFF_SIZE);
		printf("======%s->%d server: recv successful!======\n", __func__, __LINE__);
		printf("============================================================\n");
    }
    close(client_sockfd);
	while(1);
}

void loop_if_client(void *unuse)
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
	int i;
    char ch = '1';
	char buff[BUFF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

/*  Name the socket, as agreed with the server.  */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    len = sizeof(address);

	printf("[%s,%d],enter\n",__func__,__LINE__);
/*  Now connect our socket to the server's socket.  */

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if(result == -1) {
		printf("[%s,%d],enter\n",__func__,__LINE__);
        perror("oops: client");
		goto end;
    }
	while(1) {
		for(i=0; i<BUFF_SIZE; i++) {
			buff[i]=i;
		}
		write(sockfd, buff, BUFF_SIZE);
		sleep(2);
	}
    close(sockfd);
end:
	while(1);
}
void cmd_loop_server(void)
{
	int ret;
	printf("Creating loop_if_server_main task\n");
	ret = xTaskCreate(loop_if_server, (signed portCHAR *) "loop_server", 1024*4+BUFF_SIZE, NULL, 0, NULL);
	if (ret != pdPASS) {
		printf("Error creating task, status was %d\n", ret);
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_loop_server,los, Console socket server test Command);

void cmd_loop_client(void)
{
	int ret;
	printf("Creating loop_if_client_main task\n");
	ret = xTaskCreate(loop_if_client, (signed portCHAR *) "loop_client", 1024*4+BUFF_SIZE, NULL, 0, NULL);
	if (ret != pdPASS) {
		printf("Error creating task, status was %d\n", ret);
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_loop_client,loc, Console socket client test Command);
