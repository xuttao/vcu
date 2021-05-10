/*
 * @Author: xtt
 * @Date: 2021-05-08 13:46:15
 * @Description: ...
 * @LastEditTime: 2021-05-09 16:41:15
 */
#pragma once
#include "byteArray.h"
#include "serialPort.h"
#pragma pack(push, 1)
struct GPSData
{
        uint32_t time_boot_ms;
        int32_t latitude;       //维度
        int32_t longitude;      //经度
        int32_t altitude;       //wgs高程
        int32_t relative_alt;   //相对高程
        int16_t ground_x_speed; //
        int16_t ground_y_speed;
        int16_t ground_z_speed;
        uint16_t hdg;
};

struct AttitudeData
{
        uint32_t time_boot_ms;
        float roll;  //翻滚角
        float pitch; //俯仰角
        float yaw;   //航向角
        float roll_speed;
        float pitch_speed;
        float yaw_speed;
};
#pragma pack(pop)

class AircraftDataParser : public SerialPortDataParser
{
public:
#pragma pack(push, 1)
        struct MavLinkHead
        {
                uint8_t stx;         //start of text MAVLink, v2 is 0xFD
                uint8_t payload_len; //0-255
                uint8_t incompact_flags;
                uint8_t compat_flags;
                uint8_t seq;      //packet seq, 0-255, increment value each
                uint8_t sysid;    //id of system(vehicle) sending the message, 1-255
                uint8_t comid;    //id of component sending the message,1-255
                uint8_t msgid[3]; //message id, low-middle-high, 3 Bytes 0-16777215

                const uint32_t getMsgid()
                {
                        uint32_t ret = 0;
                        return ((((ret | msgid[2]) << 16) | msgid[1]) << 8) | msgid[0];
                }
        };
        struct MavLinkEnd
        {
                uint16_t checksum; //crc16 low-high
        };
#pragma pack(pop)
        enum PayloadType
        {
                GPS = 0x21,
                ATTITUDE = 0x1E,
        };

        // GPSData gps_data;
        // AttitudeData attitude_data;

private:
        ByteArray full_packet;

public:
        AircraftDataParser() = default;
        ~AircraftDataParser() = default;

public:
        virtual void *parser(const char *_data, int _len);
};
