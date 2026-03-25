/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the people's Republic of China and other countries.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "adb_sync.h"
#include "adb_log.h"
#include "misc.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

extern unsigned int gLocalID;

static int adb_sync_write(adb_server *aserver, unsigned char *buf, int len)
{
    int ret = hal_ringbuffer_wait_put(aserver->read_from_pc, buf, len, GET_PACKAGE_TIME_OUT);
    return ret;
}

static int adb_sync_read(adb_server *aserver, void *buf, int len, int out_time)
{
    unsigned char *get_data = buf;
    int get_size = 0;
    int ret = 0;

    while(get_size < len) {
        ret = hal_ringbuffer_get(aserver->read_from_pc, (get_data + get_size), (len - get_size), out_time);
        if (ret < 0) {
            adbd_err("get sync data time out\n");
            return -1;
        }
        get_size += ret;
    }

    return get_size;
}

static int adb_sync_close(adb_server *aserver)
{
    if (aserver->priv != NULL)
        adb_free(aserver->priv);

    if (aserver->read_from_pc)
        hal_ringbuffer_release(aserver->read_from_pc);

    adb_free(aserver);
    return 0;
}

static int do_stat(const char *path, adb_server *aserver)
{
    syncmsg msg;
    struct stat st;

    msg.stat.id = ID_STAT;
    adbd_debug("stat %s", path);
    if (stat(path, &st)) {
        adbd_debug("");
        msg.stat.mode = 0;
        msg.stat.size = 0;
        msg.stat.time = 0;
    } else {
        if (S_ISBLK(st.st_mode))
            st.st_mode = S_IFREG;
        /* linux S_IFDIR mask is 0x4000 */
        if (S_ISDIR(st.st_mode))
            st.st_mode = 0x4000;

        msg.stat.mode = st.st_mode;
        msg.stat.size = st.st_size;
        msg.stat.time = st.st_mtime;
    }

    usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)&msg.stat,
                   sizeof(msg.stat));
    return 0;
}

static int do_recv(const char *path, adb_server *aserver)
{
    syncmsg msg;
    int fd, read_size;
    char *buffer = adb_malloc(SYNC_DATA_MAX);

    if (!buffer)
        adbd_err("no memory");

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        adbd_err("%s open fail\n", path);
        goto do_recv_exit;
    }

    msg.data.id = ID_DATA;
    for (;;) {
        char *data = buffer;

        adbd_debug("read data...");
        read_size = read(fd, buffer, SYNC_DATA_MAX);
        if (read_size <= 0) {
            adbd_debug("read return %d", read_size);
            if (read_size == 0)
                break;
            if (errno == EINTR)
                continue;
            goto do_recv_exit;
        }
        msg.data.size = read_size;

        usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)&msg.data,
                       sizeof(msg.data));

        for (int i = 0; i < read_size / MAX_PAYLOAD; i++) {
            usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)data, MAX_PAYLOAD);
            data += MAX_PAYLOAD;
        }
        read_size = read_size % MAX_PAYLOAD;
        if (read_size != 0) {
            usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)data, read_size);
        } else {
            usbd_abd_write(aserver->localid, aserver->remoteid, NULL, 0);
        }
        adbd_debug("do recv loop");
    }
    close(fd);
    fd = -1;

    msg.data.id = ID_DONE;
    msg.data.size = 0;
    usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)&msg.data,
                   sizeof(msg.data));
    if (buffer)
        adb_free(buffer);
    return 0;
do_recv_exit:
    if (fd > 0)
        close(fd);
    if (buffer)
        adb_free(buffer);
    return -1;
}

static int is_dir_exists(const char *path)
{
    struct stat info;

    if (stat(path, &info) != 0) {
        return 0;
    }

    return S_ISDIR(info.st_mode);
}

static int create_directories(const char *path)
{
    char *tmp;
    char *p = NULL;
    size_t len;

    tmp = adb_malloc(PATH_MAX_LENGHT);
    if (tmp == NULL) {
        adbd_err("malloc fail");
        return -1;
    }
    memset(tmp, 0, PATH_MAX_LENGHT);

    snprintf(tmp, PATH_MAX_LENGHT, "%s", path);
    len = strlen(tmp);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                adbd_err("create path fail");
                adb_free(tmp);
                return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        adbd_err("create path fail");
        adb_free(tmp);
        return -1;
    }
    adb_free(tmp);
    return 0;
}

static char *extract_dir(const char *path)
{
    char *path_buff;
    char *last_slash;

    path_buff = adb_malloc(PATH_MAX_LENGHT);
    if (path_buff == NULL) {
        return NULL;
    }
    memset(path_buff, 0, PATH_MAX_LENGHT);

    strncpy(path_buff, path, PATH_MAX_LENGHT - 1);
    path_buff[PATH_MAX_LENGHT - 1] = '\0';

    last_slash = strrchr(path_buff, '/');

    if (!last_slash) {
        path_buff[0] = '.';
        path_buff[1] = '\0';
    } else {
        *last_slash = '\0';

        if (path_buff[0] == '\0') {
            strcpy(path_buff, "/");
        }
    }
    return path_buff;
}

static int create_file(const char *path, mode_t mode)
{
    char *path_copy = adb_strdup(path);
    if (!path_copy) {
        adbd_err("create_file fail");
        return -1;
    }

    char *dir = extract_dir(path_copy);
    if (is_dir_exists(path_copy) < 0) {
        adbd_debug("dir is not exit create it -- %s\n", dir);
        if (create_directories(dir) == -1) {
            adb_free(path_copy);
            adb_free(dir);
            return -1;
        }
    }
    adb_free(path_copy);
    adb_free(dir);

    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
    if (fd < 0) {
        adbd_err("create_file fail");
        return -1;
    }

    return fd;
}

static int handle_send_file(const char *path, mode_t mode, adb_server *aserver)
{
    int fd, ret;
    int total_size, write_size, been_size;
    char *buf = aserver->priv;
    char *rec_data;
    syncmsg msg;
    int last_pack = 0;

    adbd_debug("");
    fd = create_file(path, mode);
    if (fd < 0)
        return -1;

    while (1) {
        adbd_debug("");
        ret = adb_sync_read(aserver, &msg.data, sizeof(msg.data),
                                 GET_PACKAGE_TIME_OUT);
        if (ret <= 0) {
            adbd_err("");
            goto handle_send_file_err;
        }

        if (msg.data.id != ID_DATA) {
            adbd_err("");
            if (msg.data.id == ID_DONE) {
                adbd_err("");
                ret = 0;
                goto handle_send_file_err;
            }
            adbd_err("invalid data message, %d\n", msg.data.id);
            goto handle_send_file_err;
        }

        total_size = msg.data.size;
        if (total_size < SYNC_DATA_MAX) {
            adbd_debug("get size is %d, last pack\n", total_size);
            last_pack = 1;
        }

        adbd_debug("total size is %d\n", total_size);
        while (total_size > 0) {
            if (MAX_PAYLOAD > total_size) {
                write_size = total_size;
            } else {
                write_size = MAX_PAYLOAD;
            }

            ret = adb_sync_read(aserver, buf, write_size, GET_PACKAGE_TIME_OUT);
            if (ret < 0) {
                adbd_err("");
                break;
            }

            been_size = write(fd, buf, ret);
            adbd_debug("write size %d\n", been_size);
            if (been_size < 0) {
                adbd_err("write error");
            }
            total_size -= been_size;
            adbd_debug("resize %d\n", total_size);
        }

        if (last_pack == 1) {
            ret = adb_sync_read(aserver, &msg.data, sizeof(msg.data),
                                 GET_PACKAGE_TIME_OUT);
            if (ret <= 0) {
                adbd_err("");
                goto handle_send_file_err;
            }

            if (msg.data.id != ID_DONE) {
                adbd_err("");
            }
            break;
        }
    }
    ret = 0;
handle_send_file_err:
    if (fd > 0) {
        syncmsg smsg;
        close(fd);
        adbd_debug("write finish..send ok");
        smsg.status.id = ID_OKAY;
        smsg.status.msglen = 0;
        usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)&smsg.status,
                       sizeof(smsg.status));
    }

    adbd_debug("");
    return ret;
}

static int do_send(const char *path, adb_server *aserver)
{
    int ret;
    char *tmp;
    mode_t mode = 644;

    tmp = strrchr(path, ',');
    if (tmp) {
        *tmp = 0;
        errno = 0;
        mode = strtoul(tmp + 1, NULL, 0);
        mode &= 0777;
    }

    unlink(path);

    mode |= ((mode >> 3) & 0070);
    mode |= ((mode >> 3) & 0007);
    adbd_debug("send %s", path);
    ret = handle_send_file(path, mode, aserver);
    adbd_debug("");
    return ret;
}

static void *adb_sync_task(void *arg)
{
    int rb_size = 0;
    struct adb_server *aserver = (struct adb_server *)arg;
    syncmsg msg;
    unsigned char *name = (unsigned char *)aserver->priv;
    struct adb_msg amsg;

    while (1) {
        rb_size = adb_sync_read(aserver, &msg.req, sizeof(msg.req), -1);
        adbd_debug("");
        if (rb_size <= 0)
            continue;
        adbd_debug("%s --\n", (char *)&msg.req);

        if (msg.req.id == ID_QUIT)
            goto adb_sync_task_exit;

        // msg.req.namelen will reback
        rb_size = adb_sync_read(aserver, name, msg.req.namelen,
                                     GET_PACKAGE_TIME_OUT);
        if (rb_size <= 0)
            continue;
        name[msg.req.namelen] = '\0';
        switch (msg.req.id) {
        case ID_STAT:
            if (do_stat(name, aserver))
                goto adb_sync_task_exit;
            break;
        case ID_LIST:
            // if(do_list(p->payload, ser))
            //     goto adb_sync_task_exit;
            break;
        case ID_SEND:
            if (do_send(name, aserver))
                goto adb_sync_task_exit;
            break;
        case ID_RECV:
            if (do_recv(name, aserver))
                goto adb_sync_task_exit;
            break;
        default:
            adbd_debug("unknown command");
            goto adb_sync_task_exit;
        }
    }
adb_sync_task_exit:
    amsg.command = A_CLSE;
    amsg.arg0 = aserver->localid;
    amsg.arg1 = aserver->remoteid;
    amsg.data_length = 0;
    adb_send_msg((struct adb_packet *)&amsg);
    pthread_exit(NULL);
    return NULL;
}

struct adb_server *adb_sync_task_create(unsigned int remoteid)
{
    struct adb_server *aserver = adb_malloc(sizeof(struct adb_server));
    if (aserver == NULL)
        return NULL;
    memset(aserver, 0, sizeof(struct adb_server));

    aserver->read_from_pc = hal_ringbuffer_init(ADB_SYNC_RINGBUFF_SIZE);

    aserver->localid = ++gLocalID;
    aserver->remoteid = remoteid;
    aserver->service_type = ADB_SERVICE_TYPE_SYNC;
    aserver->adb_write = adb_sync_write;
    aserver->adb_close = adb_sync_close;

    aserver->priv = adb_malloc(MAX_PAYLOAD);
    if (aserver->priv == NULL) {
        adb_free(aserver);
    }
    memset(aserver->priv, 0, MAX_PAYLOAD);

    adb_thread_create(&aserver->tid, adb_sync_task, "adb-sync", (void *)aserver,
                      ADB_THREAD_NORMAL_PRIORITY, 0);

    return aserver;
}
