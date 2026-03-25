/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * UserptHttp : CdxUserptHttpStream.c
 * History :
 *
 */
#include <CdxStream.h>
#include <CdxAtomic.h>
#include <cdx_log.h>
#include "rtplayer.h"
#include <stdlib.h>

// #define VERSION1
#define VERSION2

extern const CdxStreamCreatorT httpStreamCtor;
extern const CdxStreamCreatorT fileStreamCtor;

struct CdxCustomerStreamImplS {
    CdxStreamT base;
    CdxStreamT *inter_stream;
    cdx_char *uri; /* format : "scheme://..." */
    CdxStreamCreatorT ctor;
    cdx_int64 size;
    // You can add members according to the actual situation
};

static CdxStreamProbeDataT *CustomerStreamGetProbeData(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamGetProbeData(impl->inter_stream);
}

static cdx_int32 CustomerStreamRead(CdxStreamT *stream, void *buf, cdx_uint32 len)
{
    struct CdxCustomerStreamImplS *impl;
    CDX_ENTRY();

    cdx_int32 ret = 0;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    ret = CdxStreamRead(impl->inter_stream, buf, len);

    // add customer logic

    return ret;
}

static cdx_int32 CustomerStreamClose(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();
    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    CdxStreamClose(impl->inter_stream);
    free(impl);
    return 0;
}

static cdx_int32 CustomerStreamGetIoState(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamGetIoState(impl->inter_stream);
}

static cdx_uint32 CustomerStreamAttribute(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);
    return CdxStreamAttribute(impl->inter_stream);
}

static cdx_int32 CustomerStreamControl(CdxStreamT *stream, cdx_int32 cmd, void *param)
{
    struct CdxCustomerStreamImplS *impl;
    // cdx_int64* value = NULL;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);
    if (impl->inter_stream == NULL) {
        CdxDataSourceT source;
        memset(&source, 0x00, sizeof(CdxDataSourceT));
        source.uri = impl->uri;
        impl->inter_stream = impl->ctor.create(&source);
        if (impl->inter_stream == NULL) {
            return -1;
        }
    }
    return CdxStreamControl(impl->inter_stream, cmd, param);
}

static cdx_int32 CustomerStreamSeek(CdxStreamT *stream, cdx_int64 offset, cdx_int32 whence)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamSeek(impl->inter_stream, offset, whence);
}

static cdx_int64 CustomerStreamTell(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamTell(impl->inter_stream);
}

static cdx_bool CustomerStreamEos(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamEos(impl->inter_stream);
}

static cdx_int64 CustomerStreamSize(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return impl->size;
}

static cdx_int32 CustomerStreamGetMetaData(CdxStreamT *stream, const cdx_char *key, void **pVal)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamGetMetaData(impl->inter_stream, key, pVal);
}

cdx_int32 CustomerStreamConnect(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;
    //	CdxStreamProbeDataT *probeData;
    cdx_int32 ret;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    if (impl->inter_stream == NULL) {
        CdxDataSourceT source;
        memset(&source, 0x00, sizeof(CdxDataSourceT));
        source.uri = impl->uri;
        impl->inter_stream = impl->ctor.create(&source);
        if (impl->inter_stream == NULL) {
            return -1;
        }
    }
    ret = CdxStreamConnect(impl->inter_stream);
    return ret;
}

static struct CdxStreamOpsS CustomerStreamOps = {
    .connect = CustomerStreamConnect,
    .getProbeData = CustomerStreamGetProbeData,
    .read = CustomerStreamRead,
    .write = NULL,
    .close = CustomerStreamClose,
    .getIOState = CustomerStreamGetIoState,
    .attribute = CustomerStreamAttribute,
    .control = CustomerStreamControl,
    .getMetaData = CustomerStreamGetMetaData,
    .seek = CustomerStreamSeek,
    .seekToTime = NULL,
    .eos = CustomerStreamEos,
    .tell = CustomerStreamTell,
    .size = CustomerStreamSize,
};

cdx_int32 CustomerStreamSetUrl(CdxStreamT *stream, char *url)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    cdx_char *colon;
    cdx_char scheme[16] = { 0 };

    colon = strchr(url, ':');
    memcpy(scheme, url, colon - url);

    if ((strcasecmp("http", scheme) == 0) || (strcasecmp("https", scheme) == 0)) {
        impl->ctor = httpStreamCtor;
        // } else if (strcasecmp("file", scheme) == 0) { //
        //     impl->ctor = fileStreamCtor;
    } else {
        // //other you need? call me please
        // printf("unknown scheme\n");
        impl->ctor = fileStreamCtor;
    }
    impl->uri = url;

    return 0;
}

CdxStreamT *CustomerStreamCreate(void)
{
    struct CdxCustomerStreamImplS *impl;

    impl = malloc(sizeof(*impl));

    CDX_FORCE_CHECK(impl);
    memset(impl, 0x00, sizeof(*impl));

    impl->base.ops = &CustomerStreamOps;
    return &impl->base;
}

void CustomerStreamDestory(CdxStreamT *stream)
{
    struct CdxCustomerStreamImplS *impl;

    CDX_ENTRY();

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    free(impl);
}
