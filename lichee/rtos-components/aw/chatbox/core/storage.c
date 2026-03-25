#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <log.h>
#include "storage.h"

struct storage {
    int fd;
    uint8_t enabled;
    const char *path;
};
struct storage *audio_storage_init(const char *path)
{
    if (!path || *path == '\0') {
        CHATBOX_ERROR("Invalid file path");
        return NULL;
    }

    struct storage *s = malloc(sizeof(struct storage));
    if (!s) {
        CHATBOX_ERROR("Memory allocation failed");
        return NULL;
    }

    s->fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);
    if (s->fd < 0) {
        CHATBOX_ERROR("Failed to open %s: %s", path, strerror(errno));
        free(s);
        return NULL;
    }

    s->path = path;
    s->enabled = 0; 
    CHATBOX_INFO("Storage initialized for %s", path);
    return s;
}

void storage_deinit(struct storage *storage)
{
    if (!storage) return;

    if (storage->fd >= 0) {
        close(storage->fd);
        CHATBOX_DEBUG("File descriptor %d closed", storage->fd);
    }

    free(storage);
    CHATBOX_INFO("Storage deinitialized");
}

void storage_enable(struct storage *storage, uint8_t enable)
{
    if (!storage) return;

    storage->enabled = enable ? 1 : 0;
    CHATBOX_INFO("Storage %s", enable ? "ENABLED" : "DISABLED");

    if (!enable) {
        close(storage->fd);
        storage->fd = -1;
    }

    if (enable && storage->fd < 0) {
        storage->fd = open(storage->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (storage->fd < 0) {
            CHATBOX_ERROR("Failed to open %s: %s", storage->path, strerror(errno));
            return;
        }
    }
}

int storage_write(struct storage *storage, void *data, size_t size)
{
    if (!storage || !data || size == 0) {
        CHATBOX_ERROR("Invalid parameters");
        return -EINVAL;
    }

    if (!storage->enabled || storage->fd < 0) {
        return -EBADF;
    }

    ssize_t written = write(storage->fd, data, size);

    if (written < 0) {
        CHATBOX_ERROR("Write error: %s", strerror(errno));
        return -errno;
    }

    if ((size_t)written != size) {
        CHATBOX_WARNG("Partial write: %zd/%zu bytes", written, size);
        return -EIO;
    }

    return 0;
}
