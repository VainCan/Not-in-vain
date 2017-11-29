#ifndef ALLCRC_H
#define ALLCRC_H

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned int    uint_len;


class CRC
{
public:
    CRC();
public:
    static uint8_t crc4_itu(const char *data, uint_len length);
    static uint8_t crc5_epc(const char *data, uint_len length);
    static uint8_t crc5_itu(const char *data, uint_len length);
    static uint8_t crc5_usb(const char *data, uint_len length);
    static uint8_t crc6_itu(const char *data, uint_len length);
    static uint8_t crc7_mmc(const char *data, uint_len length);
    static uint8_t crc8(const char *data, uint_len length);
    static uint8_t crc8_itu(const char *data, uint_len length);
    static uint8_t crc8_rohc(const char *data, uint_len length);
    static uint8_t crc8_maxim(const char *data, uint_len length);
    static uint16_t crc16_ibm(const char *data, uint_len length);
    static uint16_t crc16_maxim(const char *data, uint_len length);
    static uint16_t crc16_usb(const char *data, uint_len length);
    static uint16_t crc16_modbus(const char *data, uint_len length);
    static uint16_t crc16_ccitt(const char *data, uint_len length);
    static uint16_t crc16_ccitt_false(const char *data, uint_len length);
    static uint16_t crc16_x25(const char *data, uint_len length);
    static uint16_t crc16_xmodem(const char *data, uint_len length);
    static uint16_t crc16_dnp(const char *data, uint_len length);
    static uint32_t crc32(const char *data, uint_len length);
    static uint32_t crc32_mpeg_2(const char *data, uint_len length);
};



#endif // ALLCRC_H
