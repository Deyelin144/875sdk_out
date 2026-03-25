#include "usbd_core.h"
#include "usbd_audio.h"
#include "usb_osal.h"
#include "ringbuffer.h"
#include <AudioSystem.h>

#define USBD_VID           0xffff
#define USBD_PID           0xffff
#define USBD_MAX_POWER     500
#define USBD_LANGID_STRING 1033

#ifdef CONFIG_USB_HS
#define EP_INTERVAL 0x04
#else
#define EP_INTERVAL 0x01
#endif

#define AUDIO_IN_EP  0x81
#define AUDIO_OUT_EP 0x02

#define AUDIO_IN_FU_ID  0x02
#define AUDIO_OUT_FU_ID 0x05

/* AUDIO Class Config */
#define AUDIO_SPEAKER_FREQ            48000U
#define AUDIO_SPEAKER_CH              2u
#if (AUDIO_SPEAKER_CH == 1)
#define AUDIO_SPEAKER_CH_CONFIG 0X0001
#define AUDIO_SPEAKER_FE_CONFIG 0x03, 0x00
#elif (AUDIO_SPEAKER_CH == 2)
#define AUDIO_SPEAKER_CH_CONFIG 0X0003
#define AUDIO_SPEAKER_FE_CONFIG 0x03, 0x00, 0x00
#endif
#define AUDIO_SPEAKER_FRAME_SIZE_BYTE 2u
#define AUDIO_SPEAKER_RESOLUTION_BIT  16u
#define AUDIO_MIC_FREQ                16000U
#define AUDIO_MIC_CH                  1u
#if (AUDIO_MIC_CH == 1)
#define AUDIO_MIC_CH_CONFIG 0X0001
#define AUDIO_MIC_FE_CONFIG 0x03, 0x00
#elif (AUDIO_MIC_CH == 2)
#define AUDIO_MIC_CH_CONFIG 0X0003
#define AUDIO_MIC_FE_CONFIG 0x03, 0x00, 0x00
#endif
#define AUDIO_MIC_FRAME_SIZE_BYTE     2u
#define AUDIO_MIC_RESOLUTION_BIT      16u

#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET                                                                           \
    ((uint32_t)((AUDIO_SPEAKER_FREQ * AUDIO_SPEAKER_FRAME_SIZE_BYTE * AUDIO_SPEAKER_CH) / 1000))
/* 16bit(2 Bytes) 双声道(Mono:2) */
#define AUDIO_IN_PACKET ((uint32_t)((AUDIO_MIC_FREQ * AUDIO_MIC_FRAME_SIZE_BYTE * AUDIO_MIC_CH) / 1000))

#define USB_AUDIO_CONFIG_DESC_SIZ                                                                  \
    (unsigned long)(9 + AUDIO_AC_DESCRIPTOR_INIT_LEN(2) + AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +    \
                    AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(AUDIO_MIC_CH, 1) +                           \
                    AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC + AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +   \
                    AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(AUDIO_SPEAKER_CH, 1) +                       \
                    AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC + AUDIO_AS_DESCRIPTOR_INIT_LEN(1) +       \
                    AUDIO_AS_DESCRIPTOR_INIT_LEN(1))

#define AUDIO_AC_SIZ                                                                               \
    (AUDIO_SIZEOF_AC_HEADER_DESC(2) + AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +                        \
     AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(AUDIO_MIC_CH, 1) + AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +   \
     AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC + AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(AUDIO_SPEAKER_CH, 1) +\
     AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC)

const uint8_t audio_v1_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0001, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x03, 0x01, USB_CONFIG_BUS_POWERED,
                               USBD_MAX_POWER),
    AUDIO_AC_DESCRIPTOR_INIT(0x00, 0x03, AUDIO_AC_SIZ, 0x04, 0x01, 0x02),
    // MIC UAC CONTROL DESCRIPTOR
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x01, AUDIO_BIDITERM_ECHOCANCEL, AUDIO_MIC_CH, AUDIO_MIC_CH_CONFIG, 0x05),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x02, 0x01, 0x01, AUDIO_MIC_FE_CONFIG),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x03, AUDIO_TERMINAL_STREAMING, 0x02, 0x05),
    // PLAYBACK UAC CONTROL DESCRIPTOR
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x04, AUDIO_TERMINAL_STREAMING, AUDIO_SPEAKER_CH, AUDIO_SPEAKER_CH_CONFIG, 0x06),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x05, 0x04, 0x01, AUDIO_SPEAKER_FE_CONFIG),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x06, AUDIO_BIDITERM_ECHOCANCEL, 0x05, 0x06),
    // PLAYBACK UAC STREAM DESCRIPTOR
    AUDIO_AS_DESCRIPTOR_INIT(0x01, 0x04, AUDIO_SPEAKER_CH, AUDIO_SPEAKER_FRAME_SIZE_BYTE,
                             AUDIO_SPEAKER_RESOLUTION_BIT, AUDIO_OUT_EP, 0x09, AUDIO_OUT_PACKET,
                             EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_SPEAKER_FREQ)),
    // MIC UAC STREAM DESCRIPTOR
    AUDIO_AS_DESCRIPTOR_INIT(0x02, 0x03, AUDIO_MIC_CH, AUDIO_MIC_FRAME_SIZE_BYTE, AUDIO_MIC_RESOLUTION_BIT,
                             AUDIO_IN_EP, 0x05, AUDIO_IN_PACKET, EP_INTERVAL,
                             AUDIO_SAMPLE_FREQ_3B(AUDIO_MIC_FREQ)),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'U', 0x00,                  /* wcChar10 */
    'A', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
    '1', 0x00,                  /* wcChar9 */
    ///////////////////////////////////////
    /// string4 descriptor
    ///////////////////////////////////////
    0x1A,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'A', 0x00,                  /* wcChar0 */
    'C', 0x00,                  /* wcChar1 */
    ' ', 0x00,                  /* wcChar2 */
    'I', 0x00,                  /* wcChar3 */
    'n', 0x00,                  /* wcChar4 */
    't', 0x00,                  /* wcChar5 */
    'e', 0x00,                  /* wcChar6 */
    'r', 0x00,                  /* wcChar7 */
    'f', 0x00,                  /* wcChar8 */
    'a', 0x00,                  /* wcChar9 */
    'c', 0x00,                  /* wcChar10 */
    'e', 0x00,                  /* wcChar11 */
    ///////////////////////////////////////
    /// string5 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'M', 0x00,                  /* wcChar0 */
    'I', 0x00,                  /* wcChar1 */
    'C', 0x00,                  /* wcChar2 */
    ' ', 0x00,                  /* wcChar3 */
    'I', 0x00,                  /* wcChar4 */
    'n', 0x00,                  /* wcChar5 */
    'p', 0x00,                  /* wcChar6 */
    'u', 0x00,                  /* wcChar7 */
    't', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string6 descriptor
    ///////////////////////////////////////
    0x12,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'P', 0x00,                  /* wcChar0 */
    'l', 0x00,                  /* wcChar1 */
    'a', 0x00,                  /* wcChar2 */
    'y', 0x00,                  /* wcChar3 */
    'B', 0x00,                  /* wcChar4 */
    'a', 0x00,                  /* wcChar5 */
    'c', 0x00,                  /* wcChar6 */
    'k', 0x00,                  /* wcChar7 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a, USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,
#endif
    0x00
};

#define UAC_SUPPORT_DEBUG 0
#define xruac_err(fmt, args...)                                                                    \
    printf("[XRUAC_ERR][%s] line:%d " fmt "\n", __func__, __LINE__, ##args)
#if UAC_SUPPORT_DEBUG
#define xruac_dbg(fmt, args...)                                                                    \
    printf("[XRUAC_DBG][%s] line:%d " fmt "\n", __func__, __LINE__, ##args)
#else
#define xruac_dbg(fmt, args...)
#endif

#define XRUAC_THREAD_SIZE (16 * 1024)
#define XRUAC_THREAD_PRIO 17

#define XRUAC_RB_BLOCK_NUM    100
#define XRUAC_START_PLAY_THRE 5

typedef enum { BLOCK_FREE, BLOCK_WRITING, BLOCK_READY, BLOCK_READING } BlockState;

typedef struct {
    uint8_t buffer[XRUAC_RB_BLOCK_NUM][AUDIO_OUT_PACKET];
    BlockState states[XRUAC_RB_BLOCK_NUM];
    uint32_t write_idx;
    uint32_t read_idx;
    uint32_t valid_block;
    // usb_osal_mutex_t mutex;
} xruacRB;

typedef struct _xruac_app {
#define XRUAC_LOST_ONEF 0X4C4F5354
    // play
    usb_osal_thread_t play_threadid;
    usb_osal_sem_t play_start;
    xruacRB play_rb;
    uint8_t playtemp[AUDIO_OUT_PACKET];
    uint32_t play_lost;
    uint32_t play_flag;

    // record
    usb_osal_thread_t record_get;
    usb_osal_thread_t record_put;
    usb_osal_sem_t record_start;
    usb_osal_sem_t record_stop;
    usb_osal_sem_t record_start_cd;
    usb_osal_sem_t record_usb_finish;
    xruacRB record_rb;
    uint32_t record_flag;
} xruac_app;

xruac_app *xruac = NULL;

static void xruac_rb_init(xruacRB *rb)
{
    rb->write_idx = 0;
    rb->read_idx = 0;
    rb->valid_block = 0;

    for (int i = 0; i < XRUAC_RB_BLOCK_NUM; i++) {
        rb->states[i] = BLOCK_FREE;
    }
}

static uint8_t *xruac_wb_acquire(xruacRB *rb)
{
    size_t isr_flag;
    uint8_t *block_ptr = NULL;

    isr_flag = usb_osal_enter_critical_section();

    if (rb->states[rb->write_idx] == BLOCK_FREE) {
        block_ptr = rb->buffer[rb->write_idx];
        rb->states[rb->write_idx] = BLOCK_WRITING;
    }

    usb_osal_leave_critical_section(isr_flag);
    return block_ptr;
}

static void xruac_wb_finish(xruacRB *rb)
{
    size_t isr_flag;
    isr_flag = usb_osal_enter_critical_section();

    rb->states[rb->write_idx] = BLOCK_READY;
    rb->write_idx = (rb->write_idx + 1) % XRUAC_RB_BLOCK_NUM;
    rb->valid_block++;

    usb_osal_leave_critical_section(isr_flag);
}

static uint8_t *xruac_rb_acquire(xruacRB *rb)
{
    size_t isr_flag;
    uint8_t *block_ptr = NULL;

    isr_flag = usb_osal_enter_critical_section();

    if ((rb->states[rb->read_idx] == BLOCK_READY) || (rb->states[rb->read_idx] == BLOCK_READING)) {
        block_ptr = rb->buffer[rb->read_idx];
        rb->states[rb->read_idx] = BLOCK_READING;
    }

    usb_osal_leave_critical_section(isr_flag);
    return block_ptr;
}

static void xruac_rb_finish(xruacRB *rb)
{
    size_t isr_flag;
    isr_flag = usb_osal_enter_critical_section();

    rb->states[rb->read_idx] = BLOCK_FREE;
    rb->read_idx = (rb->read_idx + 1) % XRUAC_RB_BLOCK_NUM;
    rb->valid_block--;

    usb_osal_leave_critical_section(isr_flag);
}

static uint32_t xruac_rb_valid(xruacRB *rb)
{
    return rb->valid_block;
}

void usbd_event_handler(uint8_t event)
{
    switch (event) {
    case USBD_EVENT_RESET:
        break;
    case USBD_EVENT_CONNECTED:
        break;
    case USBD_EVENT_DISCONNECTED:
        break;
    case USBD_EVENT_RESUME:
        break;
    case USBD_EVENT_SUSPEND:
        break;
    case USBD_EVENT_CONFIGURED:
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
}
// work in irq;
void usbd_audio_open(uint8_t intf)
{
    uint8_t *read_buffer = NULL;
    if (intf == 1) {
        /* setup first out ep read transfer */
        read_buffer = xruac_wb_acquire(&(xruac->play_rb));
        if (read_buffer == NULL) {
            xruac->play_lost = XRUAC_LOST_ONEF;
            read_buffer = xruac->playtemp;
        }
        usbd_ep_start_read(AUDIO_OUT_EP, read_buffer, AUDIO_OUT_PACKET);
        usb_osal_sem_give(xruac->play_start);
        xruac->play_flag = 1;
    } else {
        usb_osal_sem_give(xruac->record_start);
        xruac->record_flag = 1;
    }
}
// work in irq;
void usbd_audio_close(uint8_t intf)
{
    if (intf == 1) {
        xruac->play_flag = 0;
    } else {
        xruac->record_flag = 0;
    }
}
// work in irq;
void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes)
{
    uint8_t *write_buffer = NULL;

    if (xruac->play_lost == XRUAC_LOST_ONEF) {
        xruac->play_lost = 0;
    } else {
        xruac_wb_finish(&(xruac->play_rb));
    }

    write_buffer = xruac_wb_acquire(&(xruac->play_rb));
    if (write_buffer == NULL) {
        xruac->play_lost = XRUAC_LOST_ONEF;
        write_buffer = xruac->playtemp;
    }
    usbd_ep_start_read(AUDIO_OUT_EP, write_buffer, AUDIO_OUT_PACKET);
}
// work in irq;
void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes)
{
    usb_osal_sem_give(xruac->record_usb_finish);
}

static struct usbd_endpoint audio_in_ep = { .ep_cb = usbd_audio_in_callback,
                                            .ep_addr = AUDIO_IN_EP };

static struct usbd_endpoint audio_out_ep = { .ep_cb = usbd_audio_out_callback,
                                             .ep_addr = AUDIO_OUT_EP };

struct usbd_interface intf0;
struct usbd_interface intf1;
struct usbd_interface intf2;

struct audio_entity_info audio_entity_table[] = {
    { .bEntityId = AUDIO_IN_FU_ID,
      .bDescriptorSubtype = AUDIO_CONTROL_FEATURE_UNIT,
      .ep = AUDIO_IN_EP },
    { .bEntityId = AUDIO_OUT_FU_ID,
      .bDescriptorSubtype = AUDIO_CONTROL_FEATURE_UNIT,
      .ep = AUDIO_OUT_EP },
};

#define UAC_SINK_AS_NAME "default"
#define UAC_SINK_AS_RATE AUDIO_SPEAKER_FREQ
#define UAC_SINK_AS_CH   AUDIO_SPEAKER_CH
#define UAC_SINK_AS_BITS (AUDIO_SPEAKER_FRAME_SIZE_BYTE * 8)

void xruac_play_task(void *argument)
{
    int xruac_play_state = 0;
    xruac_app *uac = (xruac_app *)argument;
    uint8_t *play_data;
    uint8_t mute_data[AUDIO_OUT_PACKET];
    tAudioTrack *at = NULL;

    memset(mute_data, 0, AUDIO_OUT_PACKET);
    // audio system don't create twice,so we don't care memory leak even thread exit
    at = AudioTrackCreateWithStreamNoMix(UAC_SINK_AS_NAME, AUDIO_STREAM_MUSIC);
    AudioTrackSetup(at, UAC_SINK_AS_RATE, UAC_SINK_AS_CH, UAC_SINK_AS_BITS);

    while (1) {
        // wait host open
        if (usb_osal_sem_take(uac->play_start, -1) < 0) {
            xruac_err("unknown err happen\n");
            break;
        }
        xruac_dbg("UAC PLAY EP OPEN\n");
        if (xruac_rb_valid(&(uac->play_rb)) < XRUAC_START_PLAY_THRE) {
            usb_osal_msleep(10);
            if (uac->play_flag == 0) {
                xruac_rb_init(&(uac->play_rb));
                continue;
            }
        }
        AudioTrackStart(at);
        xruac_dbg("uac play threshold %dms\n", XRUAC_START_PLAY_THRE);
        while (1) {
            play_data = xruac_rb_acquire(&(uac->play_rb));
            if (play_data == NULL) {
                play_data = mute_data;
            }
            AudioTrackWrite(at, play_data, AUDIO_OUT_PACKET);
            if (play_data != mute_data) {
                xruac_rb_finish(&(uac->play_rb));
            }

            if (uac->play_flag == 0) {
                AudioTrackStop(at);
                xruac_rb_init(&(uac->play_rb));
                xruac_dbg("UAC PLAY EP CLOSE\n");
                break;
            }
        }
    }
}

static int xruac_play_init(xruac_app *uac)
{
    xruac_rb_init(&(xruac->play_rb));

    uac->play_start = usb_osal_sem_create(0);
    if (uac->play_start == NULL) {
        xruac_err("xruac play sem create fail\n");
        return -1;
    }

    uac->play_threadid = usb_osal_thread_create("uac_play", XRUAC_THREAD_SIZE, XRUAC_THREAD_PRIO,
                                                xruac_play_task, uac);
    if (uac->play_threadid == NULL) {
        xruac_err("xruac play thread create fail\n");
        usb_osal_sem_delete(uac->play_start);
        return -1;
    }

    return 0;
}

static void xruac_play_deinit(xruac_app *uac)
{
    usb_osal_thread_delete(uac->play_threadid);
    usb_osal_sem_delete(uac->play_start);
}

#define UAC_RECORD_AS_NAME "default"
#define UAC_RECORD_AS_RATE AUDIO_MIC_FREQ
#define UAC_RECORD_AS_CH   AUDIO_MIC_CH
#define UAC_RECORD_AS_BITS (AUDIO_MIC_FRAME_SIZE_BYTE * 8)
void xruac_record_get(void *argument)
{
    tAudioRecord *ar;
    uint8_t *record_data = NULL;
    xruac_app *uac = (xruac_app *)argument;
    uint8_t mute_data[AUDIO_IN_PACKET];
    int ret_size = 0, per_total = 0;
    int first_start = 0;

    memset(mute_data, 0, AUDIO_IN_PACKET);
    ar = AudioRecordCreate(UAC_RECORD_AS_NAME);
    AudioRecordSetup(ar, UAC_RECORD_AS_RATE, UAC_RECORD_AS_CH, UAC_RECORD_AS_BITS);

    while (1) {
        if (usb_osal_sem_take(uac->record_start, -1) < 0) {
            xruac_err("unknown err happen\n");
            break;
        }

        xruac_rb_init(&(uac->record_rb));
        first_start = 1;

        xruac_dbg("UAC RECORD EP OPEN\n");
        AudioRecordStart(ar);
        while (1) {
            if (xruac->record_flag == 0) {
                AudioRecordStop(ar);
                // may be never start
                if (first_start == 0) {
                    usb_osal_sem_take(uac->record_stop, -1);
                }
                xruac_dbg("UAC RECORD EP CLOSE\n");
                break;
            }

            record_data = xruac_wb_acquire(&(uac->record_rb));
            if (record_data == NULL) {
                usb_osal_msleep(1);
                continue;
            }

            ret_size = 0;
            while (AUDIO_IN_PACKET > ret_size) {
                ret_size +=
                        AudioRecordRead(ar, (record_data + ret_size), (AUDIO_IN_PACKET - ret_size));
            }

            xruac_wb_finish(&(uac->record_rb));
            if ((first_start == 1) && (xruac_rb_valid(&(uac->record_rb)) > 20)) {
                first_start = 0;
                // start to send data
                usb_osal_sem_give(uac->record_start_cd);
            }
        }
    }
}

void xruac_record_send(void *argument)
{
    uint8_t *record_data = NULL;
    xruac_app *uac = (xruac_app *)argument;

    while (1) {
        if (usb_osal_sem_take(uac->record_start_cd, -1) < 0) {
            xruac_err("unknown err happen\n");
            break;
        }
        usb_osal_sem_reset(uac->record_usb_finish);
        xruac_dbg("UAC RECORD PUT OPEN\n");
        while (1) {
            if (xruac->record_flag == 0) {
                xruac_dbg("UAC RECORD PUT CLOSE\n");
                usb_osal_sem_give(uac->record_stop);
                break;
            }

            record_data = xruac_rb_acquire(&(uac->record_rb));
            if (record_data == NULL) {
                xruac_dbg("acquire data fail\n");
                usb_osal_msleep(5);
                continue;
            }

            if (usbd_ep_start_write_force(AUDIO_IN_EP, record_data, AUDIO_IN_PACKET) < 0) {
                xruac_dbg("send data fail!\n");
                usb_osal_msleep(5);
                continue;
            }

            if (usb_osal_sem_take(uac->record_usb_finish, 1000) < 0) {
                xruac_err("record send data lost\n");
            } else {
                xruac_rb_finish(&(uac->record_rb));
            }
        }
    }
}

static int xruac_record_init(xruac_app *uac)
{
    uac->record_start = usb_osal_sem_create(0);
    if (uac->record_start == NULL) {
        xruac_err("xruac record sem create fail\n");
        return -1;
    }

    uac->record_stop = usb_osal_sem_create(0);
    if (uac->record_stop == NULL) {
        xruac_err("xruac record_stop sem create fail\n");
        goto err1;
    }

    uac->record_start_cd = usb_osal_sem_create(0);
    if (uac->record_start_cd == NULL) {
        xruac_err("xruac record_start_cd sem create fail\n");
        goto err2;
    }

    uac->record_usb_finish = usb_osal_sem_create(0);
    if (uac->record_usb_finish == NULL) {
        xruac_err("xruac record_usb_finish sem create fail\n");
        goto err3;
    }

    uac->record_get = usb_osal_thread_create("uac_record_get", XRUAC_THREAD_SIZE, XRUAC_THREAD_PRIO,
                                             xruac_record_get, uac);
    if (uac->record_get == NULL) {
        xruac_err("xruac record thread create fail\n");
        goto err4;
    }

    uac->record_put = usb_osal_thread_create("uac_record_put", XRUAC_THREAD_SIZE, XRUAC_THREAD_PRIO,
                                             xruac_record_send, uac);
    if (uac->record_put == NULL) {
        xruac_err("xruac record put create fail\n");
        goto err5;
    }

    return 0;
err5:
    usb_osal_thread_delete(uac->record_get);
err4:
    usb_osal_sem_delete(uac->record_usb_finish);
err3:
    usb_osal_sem_delete(uac->record_start_cd);
err2:
    usb_osal_sem_delete(uac->record_stop);
err1:
    usb_osal_sem_delete(uac->record_start);
    return -1;
}

static int xruac_record_deinit(xruac_app *uac)
{
    usb_osal_thread_delete(uac->record_put);
    usb_osal_thread_delete(uac->record_get);
    usb_osal_sem_delete(uac->record_start);
    usb_osal_sem_delete(uac->record_stop);
    usb_osal_sem_delete(uac->record_start_cd);
    usb_osal_sem_delete(uac->record_usb_finish);
}

static int xruac_init(void)
{
    xruac = malloc(sizeof(xruac_app));
    if (xruac == NULL) {
        xruac_err("xruac malloc fail\n");
        return -1;
    }
    memset(xruac, 0, sizeof(xruac_app));

    if (xruac_play_init(xruac) < 0) {
        free(xruac);
        xruac = NULL;
        return -1;
    }

    if (xruac_record_init(xruac) < 0) {
        xruac_play_deinit(xruac);
        free(xruac);
        xruac = NULL;
        return -1;
    }

    return 0;
}

void audio_v1_init(void)
{
    xruac_init();
    usbd_desc_register(audio_v1_descriptor);
    usbd_add_interface(usbd_audio_init_intf(&intf0, 0x0100, audio_entity_table, 2));
    usbd_add_interface(usbd_audio_init_intf(&intf1, 0x0100, audio_entity_table, 2));
    usbd_add_interface(usbd_audio_init_intf(&intf2, 0x0100, audio_entity_table, 2));
    usbd_add_endpoint(&audio_in_ep);
    usbd_add_endpoint(&audio_out_ep);

    usbd_initialize();
}
