/*
 * Copyright (C) 2023 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include "CdxStream.h"

#define PSRAM_STREAM_SCHEME "psram://"
#define DEFAULT_PROBE_DATA_LEN (4 * 1024)

static int __PsramRead(uint32_t addr, uint8_t *data, uint32_t size)
{
    memcpy(data, (uint8_t *)(uintptr_t)addr, size);
    return 0;
}

enum PsramStreamStateE
{
    PSRAM_STREAM_IDLE = 0x00L,
    PSRAM_STREAM_READING = 0x01L,
    PSRAM_STREAM_SEEKING = 0x02L,
    PSRAM_STREAM_CLOSING = 0x03L,
    PSRAM_STREAM_WRITING = 0x04L,
};

/*fmt: "psram://xxx" */
struct CdxPsramStreamImplS
{
    CdxStreamT base;
    cdx_atomic_t state;
    CdxStreamProbeDataT probeData;
    cdx_int32 ioErr;

    cdx_int32 offset;
    cdx_int32 size;

    char *path;

    /*realization*/
    cdx_int32 start;
    cdx_int32 end;
    cdx_int32 p;
};

static inline cdx_int32 WaitIdleAndSetState(cdx_atomic_t *state, cdx_ssize val)
{
    cdx_int32 timeout = 0xFFFFFFF;

    while (!CdxAtomicCAS(state, PSRAM_STREAM_IDLE, val) || !(timeout--))
    {
        if (CdxAtomicRead(state) == PSRAM_STREAM_CLOSING)
        {
            CDX_LOGW("file is closing.");
            return CDX_FAILURE;
        }
        logd("dead loop wait");
    }
    return timeout > 0 ? CDX_SUCCESS : CDX_FAILURE;
}

static CdxStreamProbeDataT *__PsramStreamGetProbeData(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    return &impl->probeData;
}

static cdx_int32 __PsramStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    struct CdxPsramStreamImplS *impl;
    cdx_int32 ret;
    cdx_int64 nHadReadLen;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    if (len == 0)
    {
        return 0;
    }

    //* we must limit the HadReadLen within impl->size,
    //* or in some case will be wrong, such as cts
    nHadReadLen = impl->p - impl->start - impl->offset;
    if (nHadReadLen >= impl->size)
    {
        CDX_LOGD("eos, pos(%d)", (uint32_t)impl->size);
        return 0;
    }

    if (WaitIdleAndSetState(&impl->state, PSRAM_STREAM_READING) != CDX_SUCCESS)
    {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        return -1;
    }

    if ((nHadReadLen + len) > impl->size)
    {
        len = impl->size - nHadReadLen;
    }

    logd("p:0x%x, buf:%p, len:%d", (uint32_t)impl->p, buf, len);
    ret = __PsramRead(impl->p, buf, len);
    if (ret != 0)
    {
        CDX_LOGE("ret(%d),  cur pos:(%d), impl->size(%d)",
                 ret,  impl->p - impl->start - impl->offset, impl->size);
    }
    else
    {
        impl->p += len;
        ret = len;
    }

    if ((cdx_uint32)ret < len)
    {
        CDX_LOGE("should not be here");
        if ((impl->p - impl->start - impl->offset) == impl->size) /*end of file*/
        {
            CDX_LOGD("eos, ret(%d), pos(%d)...", ret, impl->size);
            impl->ioErr = CDX_IO_STATE_EOS;
        }
        else
        {
            CDX_LOGE("ret(%d), fresult(%d), cur pos:(%d), impl->size(%d)",
                    ret, impl->ioErr, impl->p - impl->start - impl->offset, impl->size);
        }
    }


    CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
    return ret;
}

static cdx_int32 __PsramStreamClose(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;
    cdx_int32 ret;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    ret = WaitIdleAndSetState(&impl->state, PSRAM_STREAM_CLOSING);

    CDX_FORCE_CHECK(CDX_SUCCESS == ret);

    if (impl->probeData.buf)
    {
        free(impl->probeData.buf);
    }
    if (impl->path)
    {
        free(impl->path);
    }

    free(impl);
    return CDX_SUCCESS;
}

static cdx_int32 __PsramStreamGetIoState(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    return impl->ioErr;
}

static cdx_uint32 __PsramStreamAttribute(CdxStreamT *stream)
{
    CDX_UNUSE(stream);
    return CDX_STREAM_FLAG_SEEK;
}

static cdx_int32 __PsramStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    struct CdxPsramStreamImplS *impl;
    cdx_int64 *value = NULL;
    CDX_UNUSE(param);
    CDX_UNUSE(impl);

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    switch (cmd)
    {
        case STREAM_CMD_SET_RELATIVE_START_POS:
            value = (cdx_int64 *)param;
            impl->offset = (*value);
            break;
        case STREAM_CMD_SET_RELATIVE_FILE_SIZE:
            value = (cdx_int64 *)param;
            impl->size = (cdx_int64)(*value);
            break;
        case STREAM_CMD_NEXT_PROBE_DATA:
            return CDX_FAILURE;
        case STREAM_CMD_FREE_PROBE_DATA:
            if (impl->probeData.buf)
            {
                free(impl->probeData.buf);
                impl->probeData.buf = NULL;
                impl->probeData.len = 0;
            }
            return CDX_SUCCESS;
        default:
            break;
    }

    return CDX_SUCCESS;
}

static cdx_int32 __PsramStreamSeek(CdxStreamT *stream, cdx_int64 offset, cdx_int32 whence)
{
    struct CdxPsramStreamImplS *impl;
    cdx_int64 ret = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    if (WaitIdleAndSetState(&impl->state, PSRAM_STREAM_SEEKING) != CDX_SUCCESS)
    {
        CDX_LOGE("set state(%d) fail.", CdxAtomicRead(&impl->state));
        impl->ioErr = CDX_IO_STATE_INVALID;
        return -1;
    }

    switch (whence)
    {
    case STREAM_SEEK_SET:
    {
        if (offset < 0 || offset > impl->size)
        {
            CDX_LOGE("invalid arguments, offset(%d), size(%d)", (int)offset, impl->size);
            CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
            return -1;
        }
        impl->p = impl->start + impl->offset + offset;
        break;
    }
    case STREAM_SEEK_CUR:
    {
        cdx_int64 curPos = impl->p - impl->start - impl->offset;
        if (curPos + offset < 0 || curPos + offset > impl->size)
        {
            CDX_LOGE("invalid arguments, offset(%d), size(%d), curPos(%d)",
                     (int)offset, impl->size, (int)curPos);
            CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
            return -1;
        }
        impl->p += offset;
        break;
    }
    case STREAM_SEEK_END:
    {
        cdx_int64 absOffset = impl->offset + impl->size + offset;
        if (absOffset < impl->offset || absOffset > impl->offset + impl->size)
        {
            CDX_LOGE("invalid arguments, offset(%d), size(%d)",
                     (int)absOffset, impl->offset + impl->size);
            CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
            return -1;
        }
        impl->p = impl->start + absOffset;
        break;
    }
    default:
        CDX_CHECK(0);
        break;
    }

    logd("psram stream seek: offset:%d, whence:%d", (cdx_uint32)offset, whence);
    logd("****** size=%d, start=0x%x, end=0x%x, offset=%d, p=0x%x",
            (int)impl->size, (int)impl->start, (int)impl->end, (int)impl->offset, (int)impl->p);
    CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
    return (ret >= 0 ? 0 : -1);
}

static cdx_int64 __PsramStreamTell(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;
    cdx_int64 pos;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);
    pos = impl->p - impl->start - impl->offset;
    if (pos < 0)
    {
        impl->ioErr = CDX_IO_STATE_INVALID;
        CDX_LOGE("psram pointer error");
    }
    logd("psram stream tell: pos:%d", (int)pos);

    return pos;
}

static cdx_bool __PsramStreamEos(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    return (impl->p == impl->end);
}

static cdx_int64 __PsramStreamSize(CdxStreamT *stream)
{
    struct CdxPsramStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    return impl->size;
}

static cdx_int32 __PsramStreamGetMetaData(CdxStreamT *stream, const cdx_char *key, void **pVal)
{
    struct CdxPsramStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    if (strcmp(key, "uri") == 0)
    {
        *pVal = impl->path;
        return 0;
    }

    CDX_LOGW("key(%s) not found...", key);
    return -1;
}

cdx_int32 __PsramStreamConnect(CdxStreamT *stream)
{
    cdx_int32 ret = 0;
    struct CdxPsramStreamImplS *impl;
    cdx_uint32 start;
    cdx_uint32 size;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxPsramStreamImplS, base);

    if (strncmp(impl->path, PSRAM_STREAM_SCHEME, 8) == 0) /*psram://... */
    {
        int param_cnt = sscanf(impl->path, "psram://addr=%d&length=%d", &start, &size);
        if (param_cnt != 2)
        {
            loge("url error, can't get 2 parameters.");
            goto failure;
        }
        impl->start = start;
        impl->size = size;
        impl->offset = 0;
        impl->p = impl->start;
        impl->end = impl->start + impl->size;
        impl->size -= impl->offset;
        logd("****** size=%d, start=0x%x, end=0x%x, offset=%d, p=0x%x",
            (int)impl->size, (int)impl->start, (int)impl->end, (int)impl->offset, (int)impl->p);

        logd("psram stream init success");

    } else {
        CDX_LOG_CHECK(0, "uri(%s) not psram stream...", impl->path);
    }

    CdxAtomicSet(&impl->state, PSRAM_STREAM_IDLE);
    impl->probeData.buf = malloc(DEFAULT_PROBE_DATA_LEN);    // BUFFER too large, but if set too little i think
    impl->probeData.len = DEFAULT_PROBE_DATA_LEN;            // the parser will go wrong. notice the parser probe.
    impl->ioErr = CDX_SUCCESS;

    /* if data not enough, only probe 'size' data */
    if (impl->size > 0 && impl->size < DEFAULT_PROBE_DATA_LEN)
    {
        CDX_LOGW("File too small, size(%d), will read all for probe...", impl->size);
        impl->probeData.len = impl->size;
    }

    logd("seek to %d\n", (int)impl->offset);
    impl->p = impl->start + impl->offset;

    ret = __PsramRead(impl->p, (uint8_t *)impl->probeData.buf, impl->probeData.len);
    if (ret != 0)
    {
        CDX_LOGW("io fail");
        ret = -1;
        goto failure;
    }

    impl->ioErr = 0;

    return ret;

failure:
loge("failed! exit return %d", ret);
    return ret;

}

static const struct CdxStreamOpsS psramStreamOps =
{
    .connect      = __PsramStreamConnect,
    .getProbeData = __PsramStreamGetProbeData,
    .read         = __PsramStreamRead,
    .write        = NULL,
    .close        = __PsramStreamClose,
    .getIOState   = __PsramStreamGetIoState,
    .attribute    = __PsramStreamAttribute,
    .control      = __PsramStreamControl,
    .getMetaData  = __PsramStreamGetMetaData,
    .seek         = __PsramStreamSeek,
    .seekToTime   = NULL,
    .eos          = __PsramStreamEos,
    .tell         = __PsramStreamTell,
    .size         = __PsramStreamSize,
};

static CdxStreamT *__PsramStreamCreate(CdxDataSourceT *source)
{
    struct CdxPsramStreamImplS *impl;

    impl = malloc(sizeof(*impl));

    CDX_FORCE_CHECK(impl);
    memset(impl, 0x00, sizeof(*impl));

    impl->base.ops = &psramStreamOps;
    impl->path = strdup(source->uri);
    impl->ioErr = -1;

    return &impl->base;
}

const CdxStreamCreatorT PsramStreamCtor =
{
    .create = __PsramStreamCreate
};

extern int CedarxStreamRegister(const void *creator, char *type);
int CedarxStreamRegisterPsram(void)
{
    return CedarxStreamRegister(&PsramStreamCtor, "psram");
}
