#include <stdlib.h>
#include <string.h>
#include "video_debug/vd_log.h"

typedef enum __AAC_BITSTREAM_SUB {
    AAC_SUB_TYPE_MAIN = 1,
    AAC_SUB_TYPE_LC = 2,
    AAC_SUB_TYPE_SSR = 3,
    AAC_SUB_TYPE_LTP = 4,
    AAC_SUB_TYPE_SBR = 5,
    AAC_SUB_TYPE_SCALABLE = 6,
    AAC_SUB_TYPE_TWINVQ = 7,
    AAC_SUB_TYPE_CELP = 8,
    AAC_SUB_TYPE_HVXC = 9,
    AAC_SUB_TYPE_TTSI = 12,
    AAC_SUB_TYPE_GENMIDI = 15,
    AAC_SUB_TYPE_AUDIOFX = 16,

    AAC_SUB_TYPE_

} __aac_bitstream_sub_t;

typedef enum __AAC_PROFILE_TYPE {
    AAC_PROFILE_MP = 0,
    AAC_PROFILE_LC = 1,
    AAC_PROFILE_SSR = 2,

    AAC_PROFILE_

} __aac_profile_type_t;

#define AAC_SAMPLE_RATE_INDEX (13)
#define AAC_HEADER_LENGTH        7
const int AACSampRateTbl[AAC_SAMPLE_RATE_INDEX] = { 96000, 88200, 64000, 48000, 44100,
                                                       32000, 24000, 22050, 16000, 12000,
                                                       11025, 8000,  7350 };

unsigned char *BuildAACPacketHdr(unsigned char *extraData, int extraDataLen, int uPacketLen,
                                        int *pHdrLen, int channels, int sampleRate)
{
    int tmpChnCfg, tmpSampRateIdx;
    int tmpProfile = AAC_PROFILE_LC;

    //clear aac packet header length
    *pHdrLen = 0;

    if (extraData && !(extraDataLen < 2)) {
        //parse audio obiect type
        switch (*extraData >> 3) {
        case AAC_SUB_TYPE_LC:
            tmpProfile = AAC_PROFILE_LC;
            break;
        case AAC_SUB_TYPE_MAIN:
            tmpProfile = AAC_PROFILE_MP;
            break;
        case AAC_SUB_TYPE_SSR:
            tmpProfile = AAC_PROFILE_SSR;
            break;
        case AAC_SUB_TYPE_AUDIOFX:
            tmpProfile = AAC_PROFILE_LC;
            break;
        default:
            tmpProfile = AAC_PROFILE_LC;
            break;
        }

        tmpSampRateIdx = (*extraData & 7) << 1 | *(extraData + 1) >> 7;
        if (tmpSampRateIdx == 0xf) {
            int samplingFrequency = ((*(extraData + 1) & 0x7f) << 17) | (*(extraData + 2) << 9) |
                                    (*(extraData + 3) << 1) | (*(extraData + 4) >> 7);

            for (tmpSampRateIdx = 0; tmpSampRateIdx < AAC_SAMPLE_RATE_INDEX; tmpSampRateIdx++) {
                if (samplingFrequency == AACSampRateTbl[tmpSampRateIdx]) {
                    break;
                }
            }
            tmpChnCfg = (*(extraData + 4) >> 3) & 0x7;
        } else {
            tmpChnCfg = (*(extraData + 1) >> 3) & 0x7;
        }
    } else {
        tmpProfile = 1;
        tmpChnCfg = channels & 7;
        for (tmpSampRateIdx = 0; tmpSampRateIdx < AAC_SAMPLE_RATE_INDEX; tmpSampRateIdx++) {
            if (sampleRate == AACSampRateTbl[tmpSampRateIdx]) {
                break;
            }
        }
    }
    unsigned char *pBuf = malloc(AAC_HEADER_LENGTH);
    if (pBuf == NULL)
        return NULL;
    memset(pBuf, 0, AAC_HEADER_LENGTH);
    *pHdrLen = AAC_HEADER_LENGTH;

    //set sync word, 12bits
    *(pBuf + 0) |= 0xff;
    *(pBuf + 1) |= 0xf << 4;
    //set ID, 1bit
    *(pBuf + 1) |= 0x0 << 3;
    //set layer, 2bits
    *(pBuf + 1) |= 0x0 << 1;
    //set protect bit, 1bit
    *(pBuf + 1) |= 0x1 << 0;
    //set profile, 2bits
    *(pBuf + 2) |= tmpProfile << 6;
    //set sample rate index, 4bits
    *(pBuf + 2) |= tmpSampRateIdx << 2;
    //set private bit
    *(pBuf + 2) |= 0 << 1;
    //set channel config, 3bits
    *(pBuf + 2) |= (tmpChnCfg >> 2) & 0x01;
    *(pBuf + 3) |= (tmpChnCfg << 6) & 0xff;
    //set orignal copy, 1bit
    *(pBuf + 3) |= 0 << 5;
    //set home, 1bit
    *(pBuf + 3) |= 0 << 4;
    //set copy bit, 1bit
    *(pBuf + 3) |= 0 << 3;
    //set copy start, 1bit
    *(pBuf + 3) |= 0 << 2;
    //set frame length, 13bits
    *(pBuf + 3) |= (uPacketLen >> 11) & 0x03;
    *(pBuf + 4) |= (uPacketLen >> 3) & 0xff;
    *(pBuf + 5) |= ((uPacketLen >> 0) & 0x07) << 5;
    //set buffer full, 11bits
    *(pBuf + 5) |= 0x1f << 0;
    *(pBuf + 6) |= (0xff << 2) & 0xff;
    //set raw block, 2bits
    *(pBuf + 6) |= 0 << 0;
    return pBuf;
}

int UpdateAACPacketHdr(unsigned char *pBuf, int uHdrLen, int uPacketLen)
{
    if (uHdrLen == AAC_HEADER_LENGTH) {
        //clear frame length, 13bits
        *(pBuf + 3) &= ~(0x03 << 0);
        *(pBuf + 4) = 0; //&= ~(0xff<<0);
        *(pBuf + 5) &= ~(0x07 << 5);
        //set frame length, 13bits
        *(pBuf + 3) |= (((uPacketLen + 7) >> 11) & 0x03) << 0;
        *(pBuf + 4) |= (((uPacketLen + 7) >> 3) & 0xff) << 0;
        *(pBuf + 5) |= (((uPacketLen + 7) >> 0) & 0x07) << 5;
        return 0;
    } else {
        return -1;
    }
}

void free_AACPacketHdr(unsigned char *pbuf)
{
    free(pbuf);
}

void AvccToAnnexB(unsigned char *dst, unsigned size)
{
    unsigned nalu_size = 0;
    unsigned char *data_p = dst;
    unsigned Noffset = 0;
    unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};
    while (Noffset < (size - 4)) {
        nalu_size = ((data_p[0] << 24) | (data_p[1] << 16) | (data_p[2] << 8) | data_p[3]);
        memcpy(data_p, start_code, 4);
        Noffset += 4;
        Noffset += nalu_size;
        data_p += Noffset;
    }
}