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

#include <CdxStream.h>
#include <CdxAtomic.h>
#include <cdx_log.h>
#include <stdlib.h>
#include "rtplayer.h"

extern const CdxStreamCreatorT httpStreamCtor;
extern const CdxStreamCreatorT fileStreamCtor;
extern const CdxStreamCreatorT customerStreamCtor;
extern const CdxStreamCreatorT mp4StreamCtor;

struct CdxCustomerStreamImplS
{
    CdxStreamT          base;
    CdxStreamT          *inter_stream;
    cdx_char            *uri;  /* format : "scheme://..." */
    CdxStreamCreatorT   ctor;
    cdx_int64           size;
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

    CDX_CHECK(stream);
    impl = CdxContainerOf(stream, struct CdxCustomerStreamImplS, base);

    return CdxStreamRead(impl->inter_stream, buf, len);
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
    
    cdx_int64 size = CdxStreamSize(impl->inter_stream);
    return (size == -1) ? impl->size : size;
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

static struct CdxStreamOpsS CustomerStreamOps =
{
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
    cdx_char scheme[16] = {0};

    colon = strchr(url, ':');
    if (NULL != colon) {
        memcpy(scheme, url, colon - url);
    }

    if ((strcasecmp("http", scheme) == 0) || (strcasecmp("https", scheme) == 0)) {
        impl->ctor = httpStreamCtor;
    } else {
        // other you need? call me please
        impl->ctor = fileStreamCtor;
    }
    impl->uri = url;

    return 0;
}

CdxStreamT *CustomerStreamCreate(void)
{
    struct CdxCustomerStreamImplS *impl;

    impl = malloc(sizeof(*impl));
    if (impl == NULL)
        return NULL;

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

    CdxStreamClose(impl->inter_stream);
    free(impl);
}

// int customer_player_creat(void)
// {
//     RTPlayer* mRTplayer;
//     char music_url[64];

//     mRTplayer = (RTPlayer*)(uintptr_t)player_init();
    
//     CdxStreamT *CtrStream = CustomerStreamCreate();
// 	if (CtrStream == NULL) {
// 		printf("customerStreamCreate fail!\n");
// 		return -1;
// 	}
//     CustomerStreamSetUrl(CtrStream, "sdmmc/1.mp3");
//     sprintf(music_url, "customer://%p", CtrStream);

//     status_t ret = setDataSource_stream(mRTplayer, NULL, music_url, 0);
//     if(ret){
//         LOGE("set DataSource url fail");
//         return -1;
//     }

//     ret = prepare(mRTplayer);
//     if(ret){
//         LOGE("prepare fail");
//         return -1;
//     }

//     start(mRTplayer);
//     return 0;
// }