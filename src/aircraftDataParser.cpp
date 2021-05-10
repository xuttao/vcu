/*
 * @Author: xtt
 * @Date: 2021-05-08 13:46:21
 * @Description: ...
 * @LastEditTime: 2021-05-09 16:41:32
 */
#include "aircraftDataParser.h"
#include "log.h"
#include <iostream>

#define MAVLINK_PACKET_HEAD_LEN (sizeof(AircraftDataParser::MavLinkHead))
#define MAVLINK_PACKET_END_LEN (sizeof(AircraftDataParser::MavLinkEnd))

extern GPSData gps_data;
extern AttitudeData attitude_data;

void *AircraftDataParser::parser(const char *_data, int _len)
{
        full_packet.append(_data, _len);
        assert(full_packet.size() < 1024);

        if (full_packet.size() < MAVLINK_PACKET_HEAD_LEN)
        {
                return NULL;
        }

        unsigned char *full_data = (unsigned char *)full_packet.data();
        int start_pos = -1;
        for (int i = 0; i < full_packet.size(); i++)
        {
                if (full_data[i] == 0xFD)
                {
                        start_pos = i;
                        break;
                }
        }
        if (-1 == start_pos)
        {
                full_packet.clear();
                return NULL;
        }

        MavLinkHead *pHead = (MavLinkHead *)(full_data + start_pos);
        if (full_packet.size() - start_pos < MAVLINK_PACKET_HEAD_LEN + pHead->payload_len + MAVLINK_PACKET_END_LEN)
        {
                return NULL;
        }

        // MavLinkEnd *pEnd = (MavLinkEnd *)(full_data + MAVLINK_PACKET_HEAD_LEN + pHead->payload_len);
        // auto crc = crc16_code((unsigned char *)full_data + 1, MAVLINK_PACKET_HEAD_LEN + pHead->payload_len - 1);
        // assert(crc == pEnd->checksum);
        assert((full_data + start_pos)[0] == 0xFD);

        PayloadType type = PayloadType(pHead->getMsgid());
        if (type == GPS)
        {
                gps_data = *(GPSData *)(full_data + start_pos + MAVLINK_PACKET_HEAD_LEN);
#ifdef _DEBUG
                std::cout << "Gps :time_boot_ms: " << gps_data.time_boot_ms << " latitude: " << gps_data.latitude << " longitude: " << gps_data.longitude << " altitude: " << gps_data.altitude
                          << " relative_alt: " << gps_data.relative_alt << " ground_x_speed: " << gps_data.ground_x_speed
                          << " ground_y_speed: " << gps_data.ground_y_speed << " ground_z_speed: " << gps_data.ground_z_speed
                          << " hdg: " << gps_data.hdg << std::endl;
                fflush(stdout);
#endif
        }
        else if (type == ATTITUDE)
        {
                attitude_data = *(AttitudeData *)(full_data + start_pos + MAVLINK_PACKET_HEAD_LEN);
#ifdef _DEBUG
                std::cout << "Attitude :time_boot_ms: " << attitude_data.time_boot_ms << " roll: " << attitude_data.roll
                          << " pitch: " << attitude_data.pitch << " yaw: " << attitude_data.yaw << " roll_speed:" << attitude_data.roll_speed
                          << " pitch_speed: " << attitude_data.pitch_speed << " yaw_speed:" << attitude_data.yaw_speed << std::endl;
                fflush(stdout);
#endif
        }
        else
        {
                // LOG_ERR();
        }
        full_packet.remove(start_pos, MAVLINK_PACKET_HEAD_LEN + pHead->payload_len + MAVLINK_PACKET_END_LEN);
}