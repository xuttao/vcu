/*
 * @Author: xtt
 * @Date: 2020-12-02 12:00:24
 * @Description: ...
 * @LastEditTime: 2021-05-09 17:01:16
 */
#pragma once
#include "common.h"

#pragma pack(push, 1)

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long uint64;

#define PACKET_HEAD_FLAG 0x55
#define PACKET_END_FLAG 0xAA

#define PACKET_LEN 1024

#define PACKET_HEAD_LEN (sizeof(PacketHead))
#define PACKET_END_LEN (sizeof(PacketEnd))

enum PacketType
{
        CON = 0x01,
        RES = 0x02,
        ERR = 0x03,
        BOX = 0x04,
        GPS = 0x05,
        STREAMPARAM
};

struct StreamType
{
        uchar type;
        int h;
        int w;
};

struct PacketHead
{
        uchar headIden;  //报文标识
        uchar frameType; //报文类型
        uint curFrame;   //当前帧
        uint totFrame;   //总帧数
        uint frameSize;  //内容长度
        uint picID;      //图片id
        PacketHead() : headIden(PACKET_HEAD_FLAG), frameType(CON), curFrame(0), totFrame(0), frameSize(0), picID(0) {}
        PacketHead(uchar _headIden, uchar _frameType, uint _curFrame, uint _totFrame, uint _frameSize)
        {
                headIden = _headIden;
                frameType = _frameType;
                curFrame = _curFrame;
                totFrame = _totFrame;
                frameSize = _frameSize;
                picID = 0;
        }
};

struct PacketBody
{
        char *pData;
};

struct PacketEnd
{
        uint64 frameTime;
        uchar endIden;

        PacketEnd()
        {
                frameTime = get_current_Mtime();
                endIden = PACKET_END_FLAG;
        }
};

#pragma pack(pop)