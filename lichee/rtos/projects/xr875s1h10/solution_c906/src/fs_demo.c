#include "fs_demo.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "kernel/os/os.h"

#define FS_DEMO_SRC_PATH "/data/lfs/app_config.json"
#define FS_DEMO_DST_DIR "/sdmmc/test"
#define FS_DEMO_DST_PATH "/sdmmc/test/app_config.json"
#define FS_DEMO_COPY_BUF_SIZE 512

static int fs_demo_ensure_dir(const char *path)
{
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        printf("[fs_demo] %s exists but is not a directory\n", path);
        return -1;
    }

    if (mkdir(path, 0777) == 0 || errno == EEXIST) {
        return 0;
    }

    printf("[fs_demo] mkdir %s failed: %d (%s)\n", path, errno, strerror(errno));
    return -1;
}

int fs_copy_demo(void)
{
    FILE *src = NULL;
    FILE *dst = NULL;
    unsigned char buffer[FS_DEMO_COPY_BUF_SIZE];
    size_t bytes_read;
    size_t total_bytes = 0;
    int ret = -1;

    printf("[fs_demo] copy %s -> %s\n", FS_DEMO_SRC_PATH, FS_DEMO_DST_PATH);

    if (fs_demo_ensure_dir(FS_DEMO_DST_DIR) != 0) {
        return -1;
    }

    src = fopen(FS_DEMO_SRC_PATH, "rb");
    if (src == NULL) {
        printf("[fs_demo] open source failed: %d (%s)\n", errno, strerror(errno));
        goto out;
    }

    dst = fopen(FS_DEMO_DST_PATH, "wb");
    if (dst == NULL) {
        printf("[fs_demo] open destination failed: %d (%s)\n", errno, strerror(errno));
        goto out;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dst);
        if (bytes_written != bytes_read) {
            printf("[fs_demo] write failed: wrote %u of %u bytes\n",
                   (unsigned int)bytes_written, (unsigned int)bytes_read);
            goto out;
        }
        total_bytes += bytes_written;
    }

    if (ferror(src)) {
        printf("[fs_demo] read failed while copying %s\n", FS_DEMO_SRC_PATH);
        goto out;
    }

    printf("[fs_demo] copy success, total %u bytes\n", (unsigned int)total_bytes);
    ret = 0;

out:
    if (dst != NULL) {
        fclose(dst);
    }
    if (src != NULL) {
        fclose(src);
    }
    return ret;
}
