/**
 * @file libevent_test.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-23
 * 
 * @copyright
 * Tencent is pleased to support the open source community by making IoT Hub available. 
 * Copyright(C) 2021 - 2026 THL A29 Limited, a Tencent company.All rights reserved.
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "libevent_test.h"

static uint32_t get_current_time_ms(void)
{
    return (uint32_t)HAL_GetTimeMs();
}

static void my_event_log(int severity, const char *msg)
{
    const char *severity_str;
    switch (severity) {
        case EVENT_LOG_DEBUG:
            severity_str = "DBG";
            break;
        case EVENT_LOG_MSG:
            severity_str = "MSG";
            break;
        case EVENT_LOG_WARN:
            severity_str = "WRN";
            break;
        case EVENT_LOG_ERR:
            severity_str = "ERR";
            break;
        default:
            severity_str = "UNK";
            break;
    }
    HAL_Printf("[EV][%s][%u]%s\r\n", severity_str, get_current_time_ms(), msg);
}

///////////////////////////////////////////////
// simple STUN protocol test
///////////////////////////////////////////////
static const uint8_t kCookie[4]   = {0x12, 0x21, 0x34, 0x43};
static uint8_t transactionId_[12] = {0};

#define kBindRequestType  0x0001
#define kBindResponseType 0x0101

#define kMappedAddress   0x0001
#define kResponseAddress 0x0002
#define kChangeRequest   0x0003
#define kSourceAddress   0x0004
#define kChangedAddress  0x0005

#define kDefaultTTL 64

typedef struct _StunPacket {
    uint16_t type;
    uint16_t length;
    uint8_t cookie[4];
    uint8_t transactionId[12];
} StunPacket;

typedef struct MsgHeader {
    uint16_t type;
    uint16_t length;
} AttrHeader;

static int send_evdata_to_stun(int fd)
{
    struct evbuffer *data = evbuffer_new();
    // 加上msg头部
    uint16_t type = htons(kBindRequestType);
    uint16_t len  = htons(EVBUFFER_LENGTH(data));
    evutil_secure_rng_get_bytes(transactionId_, sizeof(transactionId_));
    evbuffer_prepend(data, transactionId_, sizeof(transactionId_));
    evbuffer_prepend(data, kCookie, sizeof(kCookie));
    evbuffer_prepend(data, &len, sizeof(len));
    evbuffer_prepend(data, &type, sizeof(type));
    Log_d(">>>>>>>> socket %d send \r\n", fd);
    int ret = send(fd, EVBUFFER_DATA(data), EVBUFFER_LENGTH(data), 0);
    if (ret <= 0) {
        Log_e("udp send error %d\r\n", ret);
    }
    evbuffer_drain(data, EVBUFFER_LENGTH(data));
    evbuffer_free(data);
    return ret;
}

static int sendmsg_evdata_to_stun(int fd)
{
    struct evbuffer *data = evbuffer_new();
    // 加上msg头部
    uint16_t type = htons(kBindRequestType);
    uint16_t len  = htons(EVBUFFER_LENGTH(data));
    evutil_secure_rng_get_bytes(transactionId_, sizeof(transactionId_));
    evbuffer_prepend(data, transactionId_, sizeof(transactionId_));
    evbuffer_prepend(data, kCookie, sizeof(kCookie));
    evbuffer_prepend(data, &len, sizeof(len));
    evbuffer_prepend(data, &type, sizeof(type));

    static int cnt = 0;
    const char *host;
    if (cnt++ % 2)
        host = "180.109.157.208";
    else
        host = "113.96.17.249";

    struct sockaddr_in dest;
    dest.sin_family      = AF_INET;
    dest.sin_port        = htons(20002);
    dest.sin_addr.s_addr = inet_addr(host);
    socklen_t addrlen    = sizeof(dest);
    struct iovec io      = {(char *)EVBUFFER_DATA(data), EVBUFFER_LENGTH(data)};
    struct msghdr msg    = {&dest, addrlen, &io, 1, NULL, 0, 0};
    Log_d(">>>>>>>> socket %d sendto %s \r\n", fd, host);
    int ret = sendmsg(fd, &msg, 0);
    if (ret <= 0) {
        Log_e("udp sendmsg error %d\r\n", ret);
    }

    Log_d(">>>>>>>> socket %d sendmsg to %s ret %d\r\n", fd, host, ret);
    Log_dump((char *)EVBUFFER_DATA(data), EVBUFFER_LENGTH(data));
    evbuffer_drain(data, EVBUFFER_LENGTH(data));
    evbuffer_free(data);
    return ret;
}

static uint16_t reader_rb16(uint8_t **ptr, size_t *len)
{
    uint16_t val = (**ptr) << 8;
    *ptr += 1;
    *len -= 1;
    val |= **ptr;
    *ptr += 1;
    *len -= 1;
    return val;
}

static uint32_t reader_rb32(uint8_t **ptr, size_t *len)
{
    uint32_t val = reader_rb16(ptr, len) << 16;
    val |= reader_rb16(ptr, len);
    return val;
}

static struct sockaddr_in reader_ripv4(uint8_t **ptr, size_t *len)
{
    struct sockaddr_in addr = {0};
    addr.sin_family         = AF_INET;
    addr.sin_port           = htons(reader_rb16(ptr, len));
    addr.sin_addr.s_addr    = htonl(reader_rb32(ptr, len));
    return addr;
}

static void reader_skip(uint8_t **ptr, size_t *len, size_t offset)
{
    *ptr += offset;
}

static int parse_stun_packet(uint8_t *packet, size_t pktlen)
{
    Log_i("parse packet len %u\r\n", pktlen);
    Log_dump(packet, pktlen);
    // 解析包内容
    while (pktlen) {
        if (pktlen < 4) {
            Log_w("pktlen invalid %u\r\n", pktlen);
            break;
        }

        AttrHeader attr;
        // 从网络序转变为主机序后，可用于计算下一个attrHeader，用于判定是哪一个AttrType
        attr.type   = reader_rb16(&packet, &pktlen);
        attr.length = reader_rb16(&packet, &pktlen);
        if (pktlen < attr.length) {
            Log_w("pktlen invalid %u %d\r\n", pktlen, attr.length);
            break;
        }

        struct sockaddr_in mapAddr_, chgAddr_;
        switch (attr.type) {
            case kMappedAddress: {
                uint16_t addrType = reader_rb16(&packet, &pktlen);
                if (addrType == 2) {
                    Log_d("ipv6 not ready\r\n");
                } else {
                    mapAddr_ = reader_ripv4(&packet, &pktlen);
                }

                char ip_str[32] = {0};
                inet_ntop(AF_INET, &mapAddr_.sin_addr, ip_str, sizeof(ip_str));
                uint16_t port = ntohs(mapAddr_.sin_port);
                Log_d("stun mapped address: %s %u\r\n", ip_str, port);
                break;
            }
            case kChangedAddress: {
                uint16_t addrType = reader_rb16(&packet, &pktlen);
                if (addrType == 2) {
                    Log_w("ipv6 not ready\r\n");
                } else {
                    chgAddr_ = reader_ripv4(&packet, &pktlen);
                }

                char ip_str[32] = {0};
                inet_ntop(AF_INET, &chgAddr_.sin_addr, ip_str, sizeof(ip_str));
                uint16_t port = ntohs(chgAddr_.sin_port);
                Log_d("changed address: %s %u\r\n", ip_str, port);
                break;
            }
            default:
                Log_w("invalid attr %d len %d\r\n", attr.type, attr.length);
                reader_skip(&packet, &pktlen, attr.length);
                break;
        }
    }

    return 0;
}

static int recv_from_stun(uint8_t *data, size_t datlen)
{
    // 只校验transaction id
    if (memcmp(data + 8, transactionId_, sizeof(transactionId_))) {
        Log_w("invalid transaction id\r\n");
        return 0;
    }

    return parse_stun_packet(data + 20, datlen - 20);
}
///////////////////////////////////////////////
// simple STUN protocol test
///////////////////////////////////////////////

#define MAX_UDP_EVENT 3

typedef struct _udp_event {
    uint16_t index;
    uint16_t port;
    const char *host;
    evutil_socket_t fd;
    struct event *ev;
    void (*cb)(evutil_socket_t, short, void *);
    int use_send;
} udp_event;

typedef struct _event_context {
    struct event_base *base;
    udp_event *evs[MAX_UDP_EVENT];
    int timeout_ms;
    // TaskHandle_t thread;
} event_context;

int create_udp_event(struct event_base *base, udp_event *udp_ev)
{
    int ret = 0;
    if (base == NULL || udp_ev == NULL) {
        Log_e("invalid params\r\n");
        return -1;
    }

    udp_ev->fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_ev->fd == -1) {
        Log_e("open socket error\r\n");
        return -1;
    }

    ret = evutil_make_socket_nonblocking(udp_ev->fd);
    if (ret) {
        Log_e("make socket nonblocking error\r\n");
        goto err_exit;
    }

    if (udp_ev->use_send) {
        struct sockaddr_in server_addr;
        server_addr.sin_family      = AF_INET;
        server_addr.sin_port        = htons(udp_ev->port);
        server_addr.sin_addr.s_addr = inet_addr(udp_ev->host);
        ret                         = connect(udp_ev->fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret < 0) {
            Log_e("connect socket error\r\n");
            goto err_exit;
        }
    }

    udp_ev->ev = event_new(base, udp_ev->fd, EV_READ | EV_PERSIST, udp_ev->cb, (void *)udp_ev);
    if (udp_ev->ev == NULL) {
        Log_e("create event error\r\n");
        goto err_exit;
    }
    ret = event_add(udp_ev->ev, NULL);
    if (ret < 0) {
        Log_e("event add error %d\r\n", ret);
        goto err_exit;
    }

    Log_d("add udp %d event %p\r\n", udp_ev->fd, udp_ev->ev);
    return 0;

err_exit:
    if (udp_ev->fd > 0) {
        close(udp_ev->fd);
        udp_ev->fd = 0;
    }

    if (udp_ev->ev) {
        event_free(udp_ev->ev);
        udp_ev->ev = NULL;
    }

    return -1;
}

static void _event_thread(void *ptr)
{
    struct event_base *base = (struct event_base *)ptr;
    Log_d("event thread start dispatch.\r\n");
    event_base_dispatch(base);
    Log_d("event thread exit dispatch.\r\n");
}

static void udp_read(int fd)
{
    uint8_t buf[1500];
    int len;
    int size = sizeof(struct sockaddr);
    struct sockaddr_in peer_addr;

    memset(buf, 0, sizeof(buf));
    len = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&peer_addr, (socklen_t *)&size);

    if (len < 0) {
        Log_e("recvfrom error %d\r\n", len);
    } else if (len == 0) {
        Log_d("connection closed\r\n");
    } else {
        char peer_ip[32] = {0};
        inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
        uint16_t peer_port = ntohs(peer_addr.sin_port);

        Log_d(">>>>>>>>> udp %d recv peer addr: %s %u\r\n", fd, peer_ip, peer_port);
        Log_dump(buf, len);
        recv_from_stun(buf, len);
    }
}

void udp_cb(int fd, short event, void *arg)
{
    udp_event *udp_ev = (udp_event *)arg;
    Log_d("udp recv event callback\r\n");
    if (udp_ev) {
        udp_read(udp_ev->fd);
    }
}

void send_timer_cb(evutil_socket_t fd, short what, void *arg)
{
    udp_event *udp_evs = (udp_event *)arg;
    Log_d("send timer's up %u, udp_evs->use_send : %d, send to stun(%d)\r\n", get_current_time_ms(),udp_evs->use_send, udp_evs->fd);

    if (udp_evs->use_send)
        send_evdata_to_stun(udp_evs->fd);
    else
        sendmsg_evdata_to_stun(udp_evs->fd);
}

static void wake_timer_cb(evutil_socket_t fd, short what, void *arg)
{
    char *id = (char *)arg;

    Log_d("%s current time_ms %u\r\n", id, get_current_time_ms());
};

static int check_ip_addr(char *ip)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    if (1 == evutil_inet_pton(AF_INET, ip, &sin.sin_addr))
        return 1;
    else
        return 0;
}

/*
 * to create socketpair in freeRTOS, it needs the valid IP of this device
 * otherwise, notify is not working and it needs a timer to wake up the event thread
 */
static struct event_base *create_event_base(char *ip, long timeout_ms)
{
    struct event_base *base = NULL;

    if (ip) {
        // with valid ip, base_notifiable can be created via socketpair
        Log_d("create event base with ip %s\r\n", ip);
        // evutil_set_ifaddr_ip4(ip);
        evutil_set_ifaddr(ip);
        base = event_base_new();
        if (!base) {
            Log_d("event_base_new failed\r\n");
            return NULL;
        }
    } else {
        // have to disable base_notifiable and add a timer to wake thread time to time
        Log_d("create event base with notify disabled\r\n");

        struct event_config *cfg = event_config_new();
        // event_config_set_flag(cfg, EVENT_BASE_FLAG_PRECISE_TIMER);
        // this NOLOCK flag will disable base_notifiable
        event_config_set_flag(cfg, EVENT_BASE_FLAG_NOLOCK);
        base = event_base_new_with_config(cfg);
        event_config_free(cfg);
        if (!base) {
            Log_e("event_base_new_with_config failed\r\n");
            return NULL;
        }

        long timeout_s = timeout_ms / 1000;
        timeout_ms -= timeout_s * 1000;
        struct timeval gap = {timeout_s, timeout_ms * 1000};
        struct event *ev   = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, wake_timer_cb, (void *)"wakeup-timer");

		event_add(ev, &gap);
    }

    return base;
}

static int create_test_events(event_context *ctx)
{
    int ret;
    for (int i = 0; i < 2; i++) {
        udp_event *udp_ev = HAL_Malloc(sizeof(udp_event));
        assert(udp_ev);
        memset(udp_ev, 0, sizeof(udp_event));
        udp_ev->index = i;
        udp_ev->cb    = udp_cb;

        if (i) {
            udp_ev->host     = "192.168.17.186";
            udp_ev->port     = 5001;
            udp_ev->use_send = 1;
        } else {
            // use sendmsg and dest addr is not constant
            udp_ev->use_send = 0;
        }

        ret = create_udp_event(ctx->base, udp_ev);
        if (ret) {
            Log_e("create_udp_event failed: %d\r\n", ret);
            HAL_Free(udp_ev);
            continue;
        }
        Log_e("ip:%s\n", udp_ev->host);
        struct timeval gap = {i + 1, 0};
        struct event *ev   = event_new(ctx->base, -1, EV_TIMEOUT | EV_PERSIST, send_timer_cb, (void *)udp_ev);
        assert(ev);
        ret = event_add(ev, &gap);
        if (ret) {
            Log_e("event_add failed: %d\r\n", ret);
            event_free(ev);
            HAL_Free(udp_ev);
            continue;
        }

        ctx->evs[i] = udp_ev;
        Log_d("add udp-timer-event %d done\r\n", i + 1);
    }

    return 0;
}

int init_libevent_global(int debug)
{
    static int ev_create_cnt = 0;
    if (!ev_create_cnt) {
        // only do this init once
        Log_d("libevent global init\r\n");
        event_set_mem_functions(HAL_Malloc, HAL_Realloc, HAL_Free);
        event_set_log_callback(my_event_log);
        if (debug) {
            event_enable_debug_mode();
            event_enable_debug_logging(EVENT_DBG_ALL);
        }
        evthread_use_hal();
    } else {
        Log_w("libevent global init more than once\r\n");
    }
    ++ev_create_cnt;
    return ev_create_cnt;
}

static event_context ev_test = {0};

int start_event_test(char *ip, int debug)
{
    if (ev_test.base) {
        Log_w("event test is going on\r\n");
        return 0;
    }

    init_libevent_global(debug);

    int ret                 = 0;
    long timeout_ms         = 0;
    struct event_base *base = NULL;
    if (check_ip_addr(ip)) {
        base = create_event_base(ip, timeout_ms);
    } else {
        timeout_ms = 1200;
        base       = create_event_base(NULL, timeout_ms);
    }

    if (!base) {
        Log_e("event_base_new failed\r\n");
        return -1;
    }

    // 打印当前使用的事件后端
    Log_d("!!!! Using event method: %s timeout_ms:%ld\n", event_base_get_method(base), timeout_ms);

    memset(&ev_test, 0, sizeof(ev_test));
    ev_test.base       = base;
    ev_test.timeout_ms = timeout_ms;
    Log_d("create event base done\r\n");

    create_test_events(&ev_test);

    static ThreadParams thread_params;
    thread_params.thread_func = _event_thread;
    thread_params.user_arg    = base;
    thread_params.thread_name = "libevent_test";
    thread_params.priority    = THREAD_PRIORITY_NORMAL;
    thread_params.stack_size  = 20 * 1024;
    ret                       = HAL_ThreadCreate(&thread_params);
    if (ret < 0) {
        Log_e("failed to create the iot_video_ipc thread.");
        return -1;
    }

    Log_w("create event thread done\r\n");
    return 0;
}

int stop_event_test(void)
{
    if (!ev_test.base) {
        Log_w("event is not running\r\n");
        return -1;
    }

    Log_d("stop libevent demo\r\n");
    event_base_loopbreak(ev_test.base);
    Log_d("break event loop\r\n");

    if (ev_test.timeout_ms) {
        Log_d("wait for thread exit\r\n");
        HAL_SleepMs(1000);
    }

    Log_d("free event base\r\n");
    event_base_free(ev_test.base);

    for (int i = 0; i < MAX_UDP_EVENT; i++) {
        if (ev_test.evs[i]) {
            HAL_Free(ev_test.evs[i]);
            ev_test.evs[i] = NULL;
        }
    }

    ev_test.base = NULL;
    return 0;
}

void libevent_test(char *cmd)
{
    static char sg_ip[32] = {0};
    Log_i("libevent test cmd %s\r\n", cmd);
    memcpy(sg_ip, get_netif_ip(), sizeof(sg_ip) - 1);
    Log_d("sg_ip: %s\r\n", sg_ip);
    if (NULL != strstr(cmd, "start")) {
        start_event_test(sg_ip, 1);
    } else if (NULL != strstr(cmd, "basic")) {
        basic_libevent_test();
    } else {
        stop_event_test();
    }
}
