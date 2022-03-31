#include "msp.h"
#include <string.h>

MSP::MSP() {
    offset = 0;
    length = 0;
    crc1 = 0;
    crc2 = 0;
    dataSize = 0;
    state = MSP_IDLE;
}

uint16_t MSP::getLength() {
    return length;
}


void MSP::readByte(uint8_t byte) {
    if (length + 1 >= BUFFER_SIZE) {
        length = 0;
        state = MSP_IDLE;
    }   
    switch (state) {
        default:
        case MSP_IDLE:
            length = 0;
            if (byte == '$') {    
                state = MSP_HEADER_START;
            } else if (byte == '#') {
                state = CLI_OK;
            }
            frame[length++] = byte;
            break;
        case MSP_HEADER_START:
            if (byte == 'M') {
                state = MSP_HEADER_M;
            } else if (byte == 'X') {
                state = MSP_HEADER_X;
            } else {
                state = MSP_IDLE;
                break;
            }
            frame[length++] = byte;
            offset = 0;
            break;
        case MSP_HEADER_M:
            if (byte == '<' || byte == '>') {
                state = MSP_HEADER_V1;
                frame[length++] = byte;
                offset = 0;
                crc1 = 0;
                crc2 = 0;
            } else {
                state = MSP_IDLE;
            }
            break;
        case MSP_HEADER_X:
            if (byte == '<' || byte == '>') {
                state = MSP_HEADER_V2_NATIVE;
                frame[length++] = byte;
                offset = 0;
                crc2 = 0;
            } else {
                state = MSP_IDLE;
            }
            break;
        case MSP_HEADER_V1:
            crc1 ^= byte;
            frame[length++] = byte;
            offset++;
            if (offset == sizeof(mspHeaderV1_t)) {
                mspHeaderV1_t *hdr = (mspHeaderV1_t *)&frame[length - offset];
                if (hdr->size + length + 1 > BUFFER_SIZE) {
                    state = MSP_IDLE;
                } else if (hdr->cmd == MSP_V2_FRAME_ID) {
                    if (hdr->size >= sizeof(mspHeaderV2_t) + 1) {
                        state = MSP_HEADER_V2_OVER_V1;
                    } else {
                        state = MSP_IDLE;
                    }
                } else {
                    dataSize = hdr->size;
                    offset = 0;
                    state = dataSize > 0 ? MSP_PAYLOAD_V1 : MSP_CHECKSUM_V1;
                }
            }
            break;
        case MSP_PAYLOAD_V1:
            frame[length++] = byte;
            offset++;
            crc1 ^= byte;
            if (offset == dataSize) {
                state = MSP_CHECKSUM_V1;
            }
            break;
        case MSP_CHECKSUM_V1:
            frame[length++] = byte;
            if (crc1 == byte) {
                state = MSP_OK;
            }
            break;
        case MSP_HEADER_V2_OVER_V1:
            frame[length++] = byte;
            offset++;
            crc1 ^= byte;
            crc2 = crc8_dvb_s2(crc2, byte);
            if (offset == (sizeof(mspHeaderV2_t) + sizeof(mspHeaderV1_t))) {
                mspHeaderV2_t *hdrv2 = (mspHeaderV2_t*)&frame[length - offset + sizeof(mspHeaderV1_t)];
                dataSize = hdrv2->size;
                if (dataSize + length + 2 > BUFFER_SIZE) {
                    state = MSP_IDLE;
                } else {
                    offset = 0;
                    state = dataSize > 0 ? MSP_PAYLOAD_V2_OVER_V1
                                         : MSP_CHECKSUM_V2_OVER_V1;
                }
            }
            break;
        case MSP_PAYLOAD_V2_OVER_V1:
            frame[length++] = byte;
            offset++;
            crc1 ^= byte;
            crc2 = crc8_dvb_s2(crc2, byte);
            if (offset == dataSize) {
                state = MSP_CHECKSUM_V2_OVER_V1;
            }
            break;
        case MSP_CHECKSUM_V2_OVER_V1:
            frame[length++] = byte;
            crc1 ^= byte;
            if (crc2 == byte) {
                state = MSP_CHECKSUM_V1;
            } else {
                state = MSP_IDLE;
            }
            break;
        case MSP_HEADER_V2_NATIVE:
            frame[length++] = byte;
            offset++;
            crc2 = crc8_dvb_s2(crc2, byte);
            if (offset == sizeof(mspHeaderV2_t)) {
                mspHeaderV2_t *hdrv2 = (mspHeaderV2_t *)&frame[length - offset];
                dataSize = hdrv2->size;
                if (dataSize + length + 1 > BUFFER_SIZE) {
                    state = MSP_IDLE;
                } else {
                    offset = 0;
                    state = dataSize > 0 ? MSP_PAYLOAD_V2_NATIVE 
                                         : MSP_CHECKSUM_V2_NATIVE;
                }
            }
            break;
        case MSP_PAYLOAD_V2_NATIVE:
            frame[length++] = byte;
            offset++;
            crc2 = crc8_dvb_s2(crc2, byte);
            if (offset == dataSize) {
                state = MSP_CHECKSUM_V2_NATIVE;
            }
            break;
        case MSP_CHECKSUM_V2_NATIVE:
            frame[length++] = byte;
            if (crc2 == byte) {
                state = MSP_OK;
            } else {
                state = MSP_IDLE;
            }
            break;
        case CLI_LINE:
            frame[length++] = byte;
            if (byte == '\n') {
                state = CLI_OK;
            } else if (length == 9 && strncmp((char*)frame, "Rebooting", 9) == 0) {
                state = MSP_OK;
            }
            break;
    }
}

bool MSP::isCliMode() {
    return (state == CLI_OK || state == CLI_LINE);
}

void MSP::setCliMode(bool onOff){
    if (onOff) {
        if (!isCliMode()) {
            length = 0;
            state = CLI_LINE;
        }
    } else {
        state = MSP_IDLE;
    }
}

bool MSP::isframeValid() {
    return (state == MSP_OK || state == CLI_OK); 
}

void MSP::reset() { 
    if (state == CLI_LINE) {
        return;
    }
    if (state == CLI_OK) {
        length = 0;
        state = CLI_LINE;
    } else {
        state = MSP_IDLE; 
    }
}

uint8_t MSP::crc8_dvb_s2(uint8_t crc, unsigned char a) {
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}