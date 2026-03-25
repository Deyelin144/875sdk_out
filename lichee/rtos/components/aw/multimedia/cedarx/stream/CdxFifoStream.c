/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * Fifo : CdxFifoStream.c
 * Description : Fifo Stream Definition
 * History :
 *
 */
#include <CdxStream.h>
#include <CdxFifoStream.h>
#include <CdxAtomic.h>
#include <cdx_log.h>
#include <errno.h>
#include <stdlib.h>

extern int g_probe_data_len;

#define FIFO_STREAM_SCHEME     "fifo://"
#define DEFAULT_PROBE_DATA_LEN g_probe_data_len

/*
 * A specific processing for mp3 play...
 * MP3 may has several id3 header.
 * And we should support to seek for these header no matter how large the offset is.
 * For other offset, we should only support offset < CHANGE_BUFFER_OFFSET_THRESHOLD
 */
#define IOT_ID3_SEEK_CHECK 1

/*
 * because fifo stream can not seek randomly, so we use a cache to keep
 * necessary data for seek request.
 * for MP3 fifo, usually 10K is enough
 * for AAC fifo, suggest more than 30K
 */
#define CACHE_DATA_LEN (1024 * 30)

/*
 * for mp3 with hander id3v2, these mp3 will seek a large offset when parser init.
 * mainly for baseoffser and buffer_offser
 */
#define CHANGE_BUFFER_OFFSET_THRESHOLD (1024 * 9)

#define MAX_WAITING_TIME_MS 5000

enum FifoStreamStateE {
    FIFO_STREAM_IDLE = 0x00L,
    FIFO_STREAM_READING = 0x01L,
    FIFO_STREAM_SEEKING = 0x02L,
    FIFO_STREAM_CLOSING = 0x03L,
};

struct CdxFifoStreamImplS {
    CdxStreamT base;
    cdx_atomic_t state;
    CdxStreamProbeDataT probeData;
    struct CdxFifoStreamS *fifobase;
    cdx_int32 ioErr;
    cdx_int64 size;
    cdx_int64 baseOffset;
    unsigned char has_set_offset;
#if IOT_ID3_SEEK_CHECK
    unsigned char seek_for_id3;
#endif
    cdx_int64 readPos;
    cdx_int64 outPos;
    unsigned char *path;
    unsigned char *buffer;
    unsigned int buffer_len;
    cdx_int64 buffer_offser;
};

static inline cdx_int32 WaitIdleAndSetState(cdx_atomic_t *state, cdx_ssize val)
{
    cdx_int32 timeout = 0xFFFFFFF;

    while (!CdxAtomicCAS(state, FIFO_STREAM_IDLE, val) || !(timeout--)) {
        if (CdxAtomicRead(state) == FIFO_STREAM_CLOSING) {
            CDX_LOGW("file is closing.");
            return CDX_FAILURE;
        }
        logd("dead loop wait");
    }
    return timeout > 0 ? CDX_SUCCESS : CDX_FAILURE;
}

static CdxStreamProbeDataT *__FifoStreamGetProbeData(CdxStreamT *stream)
{
    struct CdxFifoStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    return &impl->probeData;
}

static cdx_int32 Min(cdx_int32 a, cdx_int32 b, cdx_int32 c)
{
    cdx_int32 ret;
    ret = (a > b) ? b : a;
    ret = (ret > c) ? c : ret;
    return ret;
}

static cdx_int32 __FifoStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    struct CdxFifoStreamImplS *impl;
    cdx_int32 valid;
    cdx_uint32 nHadReadLen = 0;
    cdx_uint32 readlen;
    cdx_uint32 waiting_time = 0;
    cdx_bool eos;
    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    if (WaitIdleAndSetState(&impl->state, FIFO_STREAM_READING) != CDX_SUCCESS) {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        return -1;
    }

    if (impl->readPos > impl->outPos) {
        cdx_uint32 total;
        cdx_uint32 garbage_len = 1024;
        unsigned char *garbage_can = (unsigned char *)malloc(garbage_len);
        /* drop some data, and consider whether we should put them to impl->buffer */
        while ((impl->readPos != impl->outPos) && (waiting_time < MAX_WAITING_TIME_MS)) {
            total = impl->readPos - impl->outPos;
            CdxFifoStreamLock(impl->fifobase);
            eos = CdxFifoStreamIseos(impl->fifobase);
            valid = CdxFifoStreamValid(impl->fifobase);
            readlen = Min(total, valid, garbage_len);
            CdxFifoStreamOut(impl->fifobase, garbage_can, readlen);
            valid = CdxFifoStreamValid(impl->fifobase);
            CdxFifoStreamUnlock(impl->fifobase);

            if ((impl->outPos == impl->buffer_offser + impl->buffer_len) &&
                (impl->buffer_len < CACHE_DATA_LEN)) {
                cdx_uint32 copy_len;
                copy_len = (readlen > (CACHE_DATA_LEN - impl->buffer_len)) ?
                                   (CACHE_DATA_LEN - impl->buffer_len) :
                                   readlen;
                memcpy(impl->buffer + impl->buffer_len, garbage_can, copy_len);
                impl->buffer_len += copy_len;
            }
            impl->outPos += readlen;
            if (valid == 0) {
                usleep(5000);
                waiting_time += 5;
            }
            if (valid == 0 && eos) {
                free(garbage_can);
                impl->size = impl->outPos;
                impl->ioErr = CDX_IO_STATE_EOS;
                goto exit;
            }
        }
        free(garbage_can);
    } else if (impl->readPos < impl->outPos) {
        /* find in impl->buffer, if fail, return error */
        if (((impl->readPos >= impl->buffer_offser) &&
             (impl->readPos < impl->buffer_offser + impl->buffer_len))) {
            cdx_uint32 valid_len = impl->buffer_offser + impl->buffer_len - impl->readPos;
            readlen = (len > valid_len) ? valid_len : len;
            if (eos) {
                readlen = len;
            }
            memcpy(buf, impl->buffer + impl->readPos - impl->buffer_offser, readlen);
            impl->readPos += readlen;
            nHadReadLen += readlen;
            CDX_LOGW(
                    "read<out:readPos:%d, outPos:%d, impl->offser:%d, nHadReadLen:%d, buffer_len:%d\n",
                    impl->readPos, impl->outPos, impl->buffer_offser, nHadReadLen,
                    impl->buffer_len);
        }
        if ((nHadReadLen != len) && (impl->readPos != impl->outPos)) {
            CDX_LOGE("read fail, should read:%u, hadread:%u", len, nHadReadLen);
            goto exit;
        }
    }
    /* make sure readPos equal to outPos when going to run this section code */
    while ((nHadReadLen != len) && (waiting_time < MAX_WAITING_TIME_MS)) {
        CdxFifoStreamLock(impl->fifobase);
        eos = CdxFifoStreamIseos(impl->fifobase);
        valid = CdxFifoStreamValid(impl->fifobase);
        readlen = valid > (len - nHadReadLen) ? (len - nHadReadLen) : valid;
        CdxFifoStreamOut(impl->fifobase, (char *)buf + nHadReadLen, readlen);
        valid = CdxFifoStreamValid(impl->fifobase);
        CdxFifoStreamUnlock(impl->fifobase);
        if ((impl->outPos == impl->buffer_offser + impl->buffer_len) &&
            (impl->buffer_len < CACHE_DATA_LEN)) {
            CDX_LOGW("equal:read->outPos == impl->buffer_offser+impl->buffer_len");
            cdx_uint32 copy_len;
            copy_len = (readlen > (CACHE_DATA_LEN - impl->buffer_len)) ?
                               (CACHE_DATA_LEN - impl->buffer_len) :
                               readlen;
            memcpy(impl->buffer + impl->buffer_len, (char *)buf + nHadReadLen, copy_len);
            impl->buffer_len += copy_len;
        }

        CDX_LOGW("equal:readPos:%d, outPos:%d, impl->offser:%d, nHadReadLen:%d, buffer_len:%d\n",
                 impl->readPos, impl->outPos, impl->buffer_offser, nHadReadLen, impl->buffer_len);
        CDX_LOGW("equal:readlen:%d, valid:%d, len:%d, len - n HadReadLen:%d, ioErr:%d\n", readlen,
                 valid, len, len - nHadReadLen, impl->ioErr);
        impl->readPos += readlen;
        impl->outPos += readlen;
        nHadReadLen += readlen;
        if (nHadReadLen != len) {
            usleep(5000);
            waiting_time += 5;
        }

        if (valid == 0 && eos) {
            impl->size = impl->outPos;
            impl->ioErr = CDX_IO_STATE_EOS;
            goto exit;
        }
    }

exit:
    CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
    return nHadReadLen;
}

static cdx_int32 __FifoStreamClose(CdxStreamT *stream)
{
    struct CdxFifoStreamImplS *impl;
    cdx_int32 ret;

    CDX_ENTRY();
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    ret = WaitIdleAndSetState(&impl->state, FIFO_STREAM_CLOSING);

    CDX_FORCE_CHECK(CDX_SUCCESS == ret);

    if (impl->probeData.buf) {
        free(impl->probeData.buf);
        impl->probeData.buf = NULL;
    }
    if (impl->path) {
        free(impl->path);
        impl->path = NULL;
    }
    if (impl->buffer) {
        free(impl->buffer);
        impl->buffer = NULL;
        impl->buffer_len = 0;
    }
    free(impl);
    CDX_EXIT(CDX_SUCCESS);
    return CDX_SUCCESS;
}

static cdx_int32 __FifoStreamGetIoState(CdxStreamT *stream)
{
    struct CdxFifoStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    return impl->ioErr;
}

static cdx_uint32 __FifoStreamAttribute(CdxStreamT *stream)
{
    CDX_UNUSE(stream);
    return CDX_STREAM_FLAG_SEEK | CDX_STREAM_FLAG_NET;
}

static cdx_int32 __FifoStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    struct CdxFifoStreamImplS *impl;
    cdx_int64 *value = NULL;
    cdx_int64 addoffset;
    CDX_UNUSE(param);
    CDX_UNUSE(impl);

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    switch (cmd) {
    case STREAM_CMD_SET_RELATIVE_START_POS:
        value = (cdx_int64 *)param;
        if (impl->buffer_offser > (*value)) {
            CDX_LOGE("should not be here, old offset:%d, new offset:%d\n", (int)impl->buffer_offser,
                     (int)(*value));
            return CDX_FAILURE;
        }
        addoffset = (*value) - impl->buffer_offser;
        if (impl->buffer_len > addoffset) {
            impl->buffer_len = impl->buffer_len - addoffset;
            memmove(impl->buffer, impl->buffer + addoffset, impl->buffer_len);
        } else {
            impl->buffer_len = 0;
        }
        impl->baseOffset = (*value);
        impl->buffer_offser = (*value);
        impl->has_set_offset = 1;
        break;
    case STREAM_CMD_SET_RELATIVE_FILE_SIZE:
        value = (cdx_int64 *)param;
        impl->size = (cdx_int64)(*value);
        break;
    case STREAM_CMD_NEXT_PROBE_DATA:
        CDX_LOGW("STREAM_CMD_NEXT_PROBE_DATA not support now");
        {
            int out_len = 0, valid = 0, eos = 0;
            CdxFifoStreamLock(impl->fifobase);
            valid = CdxFifoStreamValid(impl->fifobase);
            eos = CdxFifoStreamIseos(impl->fifobase);
            out_len = valid > (DEFAULT_PROBE_DATA_LEN - impl->probeData.len) ?
                                (DEFAULT_PROBE_DATA_LEN - impl->probeData.len) :
                                valid;
            CdxFifoStreamOut(impl->fifobase, impl->probeData.buf + impl->probeData.len, out_len);
            impl->probeData.len += out_len;
            printf("%s, %d, out_len = %d\n", __func__, impl->probeData.len, out_len);
            CdxFifoStreamUnlock(impl->fifobase);
            if (eos) {
                break;
            }
        
            memcpy(impl->buffer, impl->probeData.buf, impl->probeData.len);
            impl->buffer_len = impl->probeData.len;
            impl->outPos = impl->probeData.len;
            impl->ioErr = 0;
        }
        
        break;
    case STREAM_CMD_FREE_PROBE_DATA:
        if (impl->probeData.buf) {
            free(impl->probeData.buf);
            impl->probeData.buf = NULL;
            impl->probeData.len = 0;
        }
        break;
#if IOT_ID3_SEEK_CHECK
    case STREAM_CMD_PREPARE_SEEK_FOR_ID3:
        impl->seek_for_id3 = 1; /* mean next seek is for id3 header */
        break;
#endif
    default:
        break;
    }

    return CDX_SUCCESS;
}

static void BufferManager(struct CdxFifoStreamImplS *impl)
{
    if (impl->has_set_offset)
        return;

    if (impl->readPos > CHANGE_BUFFER_OFFSET_THRESHOLD) {
        cdx_int64 addoffset;
        addoffset = impl->readPos - impl->buffer_offser;
        if (impl->buffer_len > addoffset) {
            impl->buffer_len = impl->buffer_len - addoffset;
            memmove(impl->buffer, impl->buffer + addoffset, impl->buffer_len);
        } else {
            impl->buffer_len = 0;
        }
        impl->buffer_offser = impl->readPos;
    }
}

static cdx_int32 __FifoStreamSeek(CdxStreamT *stream, cdx_int64 offset, cdx_int32 whence)
{
    struct CdxFifoStreamImplS *impl;
    cdx_bool eos;
    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    if (WaitIdleAndSetState(&impl->state, FIFO_STREAM_SEEKING) != CDX_SUCCESS) {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        impl->ioErr = CDX_IO_STATE_INVALID;
        return -1;
    }

    if ((whence == STREAM_SEEK_SET) && (impl->readPos == (offset + impl->baseOffset))) {
        goto exit;
    }

#if IOT_ID3_SEEK_CHECK
    if (impl->seek_for_id3) {
        impl->seek_for_id3 = 0;
    } else if (offset > CHANGE_BUFFER_OFFSET_THRESHOLD) {
        CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
        return -1;
    }
#endif

    switch (whence) {
    case STREAM_SEEK_SET: {
        if (offset < 0) {
            CDX_LOGE("invalid arguments, offset(%d), size(%d)", (int)offset, (int)impl->size);
            // CdxDumpThreadStack((pthread_t)gettid());
            CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
            return -1;
        }
        impl->readPos = offset + impl->baseOffset;
        BufferManager(impl);

        break;
    }
    case STREAM_SEEK_CUR: {
        if (impl->readPos + offset < 0 || impl->readPos + offset > impl->size) {
            CDX_LOGE("invalid arguments, offset(%lld), size(%lld), curPos(%lld)", offset,
                     impl->size, impl->readPos);
            // CdxDumpThreadStack((pthread_t)gettid());
            CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
            return -1;
        }
        impl->readPos += offset;
        BufferManager(impl);
        break;
    }
    case STREAM_SEEK_END: {
        CDX_LOGE("do not support seek to end\n");
        // CdxDumpThreadStack((pthread_t)gettid());
        CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
        return -1;
    }
    default:
        CDX_CHECK(0);
        break;
    }

exit:
    CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
    CDX_EXIT(CDX_SUCCESS);
    return CDX_SUCCESS;
}

static cdx_int64 __FifoStreamTell(CdxStreamT *stream)
{
    struct CdxFifoStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);
    return impl->readPos - impl->baseOffset;
}

static cdx_int64 __FifoStreamSize(CdxStreamT *stream)
{
    struct CdxFifoStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);
    return impl->size;
}

static cdx_int32 __FifoStreamGetMetaData(CdxStreamT *stream, const cdx_char *key, void **pVal)
{
    struct CdxFifoStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    if (strcmp(key, "uri") == 0) {
        *pVal = impl->path;
        return 0;
    }

    CDX_LOGW("key(%s) not found...", key);
    return -1;
}

#define MAX_PROBE_DATA_WAIT_TIME_MS 5000
static cdx_int32 __FifoStreamConnect(CdxStreamT *stream)
{
    cdx_int32 valid;
    cdx_int32 out_len;
    cdx_bool eos;
    cdx_uint32 wait_time = 0;
    struct CdxFifoStreamImplS *impl;

    CDX_ENTRY();
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxFifoStreamImplS, base);

    CdxAtomicSet(&impl->state, FIFO_STREAM_IDLE);
    impl->probeData.buf =
            malloc(DEFAULT_PROBE_DATA_LEN); // BUFFER too large, but if set too little i think
    impl->probeData.len = 0;                   // the parser will go wrong. notice the parser probe.
    impl->ioErr = CDX_SUCCESS;
    CDX_LOG_CHECK(impl->probeData.buf != 0, "malloc failed, buf = 0x%x",
                  impl->probeData.buf);

    logd("seek to %d\n", (int)impl->baseOffset);
    impl->readPos = impl->baseOffset;
    CDX_LOG_CHECK(impl->baseOffset == 0, "baseoffset is not init to zero");

    while (1) {
        CdxFifoStreamLock(impl->fifobase);
        valid = CdxFifoStreamValid(impl->fifobase);
        eos = CdxFifoStreamIseos(impl->fifobase);
        out_len = valid > (DEFAULT_PROBE_DATA_LEN - impl->probeData.len) ?
                          (DEFAULT_PROBE_DATA_LEN - impl->probeData.len) :
                          valid;
        CdxFifoStreamOut(impl->fifobase, impl->probeData.buf + impl->probeData.len, out_len);
        impl->probeData.len += out_len;
        CdxFifoStreamUnlock(impl->fifobase);

        if ((eos) || (impl->probeData.len > 0))
            break;
        if (wait_time >= MAX_PROBE_DATA_WAIT_TIME_MS)
            break;

        usleep(50 * 1000);
        wait_time += 50;
    }

    memcpy(impl->buffer, impl->probeData.buf, impl->probeData.len);
    impl->buffer_len = impl->probeData.len;
    impl->outPos += impl->probeData.len;
    impl->ioErr = 0;

    CDX_EXIT(CDX_SUCCESS);

    return (eos && (impl->probeData.len == 0)) ? CDX_FAILURE : CDX_SUCCESS;
}
/*
static const struct CdxStreamOpsS FifoStreamOps =
{
    .cdxConnect      = __FifoStreamConnect,
    .cdxGetProbeData = __FifoStreamGetProbeData,
    .cdxRead         = __FifoStreamRead,
    .cdxWrite        = NULL,
    .cdxClose        = __FifoStreamClose,
    .cdxGetIOState   = __FifoStreamGetIoState,
    .cdxAttribute    = __FifoStreamAttribute,
    .cdxControl      = __FifoStreamControl,
    .cdxGetMetaData  = __FifoStreamGetMetaData,
    .cdxSeek         = __FifoStreamSeek,
    .cdxSeekToTime   = NULL,
    .cdxEos          = NULL,
    .cdxTell         = __FifoStreamTell,
    .cdxSize         = __FifoStreamSize,
};
*/
static const struct CdxStreamOpsS FifoStreamOps = {
    .connect = __FifoStreamConnect,
    .getProbeData = __FifoStreamGetProbeData,
    .read = __FifoStreamRead,
    .write = NULL,
    .close = __FifoStreamClose,
    .getIOState = __FifoStreamGetIoState,
    .attribute = __FifoStreamAttribute,
    .control = __FifoStreamControl,
    .getMetaData = __FifoStreamGetMetaData,
    .seek = __FifoStreamSeek,
    .seekToTime = NULL,
    .eos = NULL,
    .tell = __FifoStreamTell,
    .size = __FifoStreamSize,
};
static CdxStreamT *__FifoStreamCreate(CdxDataSourceT *source)
{
    struct CdxFifoStreamImplS *impl;
    int ret;

    impl = malloc(sizeof(*impl));

    CDX_FORCE_CHECK(impl);

    memset(impl, 0x00, sizeof(*impl));

    impl->base.ops = &FifoStreamOps;
    ret = sscanf(source->uri, "fifo://%p", &impl->fifobase);
    if (ret != 1) {
        CDX_LOGE("sscanf failure...(%s)", source->uri);
        return NULL;
    }

    impl->path = (unsigned char *)strdup(source->uri);
    impl->ioErr = -1;
    impl->size = -1; /* assume this stream has infinite data first */
    impl->buffer = (unsigned char *)malloc(CACHE_DATA_LEN);
    CDX_FORCE_CHECK(impl->buffer);
    impl->baseOffset = 0;
    impl->buffer_len = 0;
#if ID3_IOT_IMPLEMENT
    /* for id3 implement */
    impl->baseOffset = source->offset > 0 ? source->offset : 0;
    logd("source->offset: %d", (int)source->offset);
#endif
    return &impl->base;
}

const CdxStreamCreatorT newfifoStreamCtor = { .create = __FifoStreamCreate };

extern int CedarxStreamRegister(const void *creator, char *type);

int CedarxStreamRegisternewFifo(void)
{
    return CedarxStreamRegister(&newfifoStreamCtor, "fifo");
}