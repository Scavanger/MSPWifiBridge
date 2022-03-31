#pragma once

#include <inttypes.h>

#define BUFFER_SIZE 1024
#define MSP_V2_FRAME_ID 0xFF

typedef enum {
    MSP_IDLE,
    MSP_HEADER_START,
    MSP_HEADER_M,
    MSP_HEADER_X,
    MSP_HEADER_V1,
    MSP_PAYLOAD_V1,
    MSP_CHECKSUM_V1,
    MSP_HEADER_V2_OVER_V1,
    MSP_PAYLOAD_V2_OVER_V1,
    MSP_CHECKSUM_V2_OVER_V1,
    MSP_HEADER_V2_NATIVE,
    MSP_PAYLOAD_V2_NATIVE,
    MSP_CHECKSUM_V2_NATIVE,
    MSP_OK,
    CLI_LINE,
    CLI_OK
} MspState_e;

typedef struct __attribute__((packed)) {
    uint8_t size;
    uint8_t cmd;
} mspHeaderV1_t;

typedef struct __attribute__((packed)) {
    uint8_t flags;
    uint16_t cmd;
    uint16_t size;
} mspHeaderV2_t;

class MSP {
   private:
    MspState_e state;
    uint16_t offset;
    uint16_t dataSize;
    uint16_t length;
    uint8_t crc1;
    uint8_t crc2;
    static uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a);
   public:
    uint8_t frame[BUFFER_SIZE] = {0};
    MSP();
    uint16_t getLength();
    void readByte(uint8_t byte);
    bool isframeValid();
    void reset();
    bool isCliMode();
    void setCliMode(bool onOff);
};
