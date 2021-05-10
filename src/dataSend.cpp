/*
 * @Author: xtt
 * @Date: 2021-01-18 12:54:44
 * @Description: ...
 * @LastEditTime: 2021-05-09 18:30:40
 */

#include "dataSend.h"
#include "aircraftDataParser.h"
#include "rtspDataManager.h"
#include "yoloDetect.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <thread>

static int get_local_ip(const char *eth_inf, std::string &ip)
{
        int sd;
        struct sockaddr_in sin;
        struct ifreq ifr;

        sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (-1 == sd)
        {
                LOG_ERR("socket error: %s", strerror(errno));
                return -1;
        }

        strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = 0;

        // if error: No such device
        if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
        {
                LOG_ERR("ioctl error: %s", strerror(errno));
                close(sd);
                return -1;
        }

        LOG_INFO("interfac: %s, ip: %s", eth_inf, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

        close(sd);
        return 0;
}

DataSend *DataSend::getInstance()
{
        static DataSend ins;
        return &ins;
}

DataSend::DataSend()
{
        std::string local_ip;
#ifdef _ARM64
        FSERVO_CHECK(get_local_ip("eth0", local_ip) == 0);
#else
        FSERVO_CHECK(get_local_ip("enp3s0", local_ip) == 0);
#endif

        pSocketTcp = new SocketTcp;
        pSocketTcp->createTcpServer(local_ip, 7780);

        is_stop = false;
        pThread = new std::thread(receive_thread, this);

        // pSocket = new SocketUdp;
        // pSocket->createUdpClient("192.168.3.110", 7780);
}

DataSend::~DataSend()
{

        is_stop = true;
        pThread->join();
        delete pThread;

        if (pSocket)
        {
                pSocket->closeSocket();
                delete pSocket;
        }
        if (pSocketTcp)
        {
                pSocketTcp->closeSocket();
                delete pSocket;
        }
}

void DataSend::receive_thread(void *arg)
{
        DataSend *pThis = static_cast<DataSend *>(arg);

        int len = PACKET_HEAD_LEN + PACKET_END_LEN;
        char receive_buffer[PACKET_HEAD_LEN + PACKET_END_LEN];

        bool not_first = false;

        for (;;)
        {
                if (pThis->is_stop) break;
                pThis->can_send_box = false;
                pThis->can_send_con = false;
                pThis->pSocketTcp->waitBeConnected();
                pThis->pSocketTcp->setNoDelay();

                if (not_first)
                {
                        pThis->pf();
                }

                not_first = true;

                for (;;)
                {
                        if (pThis->is_stop) break;

                        int ret = pThis->pSocketTcp->read(receive_buffer, len);
                        // LOG_INFO("receive request len:%d, need len:%d", ret, len);

                        if (ret == len)
                        {
                                PacketHead *pHead = (PacketHead *)receive_buffer;
                                if (pHead->frameType == PacketType::CON)
                                {
                                        pThis->can_send_con = true;

                                        LOG_INFO("receive con request from client");
                                }
                                else if (pHead->frameType == PacketType::BOX)
                                {
                                        pThis->can_send_con = true;
                                        pThis->can_send_box = true;

                                        if (!pThis->is_send)
                                        {
                                                // RtspDataManager::getInstance()->play();
                                                pThis->is_send = true;
                                        }

                                        LOG_INFO("receive box request from client");
                                }
                                else if (pHead->frameType == PacketType::STREAMPARAM)
                                {
                                        pThis->send_streamInfo();

                                        LOG_INFO("receive stream info request from client");
                                }
                        }
                        else if (ret <= 0)
                        {
                                LOG_WARN("client disconnected!");
                                break;
                        }
                        else
                        {
                                LOG_ERR("unknow packet! len:%d", ret);
                        }
                }
        }
}

void DataSend::send_streamInfo()
{
        int con_size = sizeof(StreamType);
        PacketHead head(PACKET_HEAD_FLAG, STREAMPARAM, 1, 1, con_size);
        PacketEnd end;
        StreamType info;
        info.type = type;
        info.h = -1;
        info.w = -1;
        int packet_size1 = PACKET_HEAD_LEN + con_size + PACKET_END_LEN;
        char *send_buffer = new char[packet_size1];

        memcpy(send_buffer, &head, PACKET_HEAD_LEN);
        memcpy(send_buffer + PACKET_HEAD_LEN, &info, con_size);
        memcpy(send_buffer + PACKET_HEAD_LEN + con_size, &end, PACKET_END_LEN);

        bool ret = pSocketTcp->send(send_buffer, packet_size1);
        if (!ret)
        {
                LOG_ERR("send stream info fail!");
        }

        delete[] send_buffer;
}

GPSData gps_data;
AttitudeData attitude_data;
#define PACKET_BOX_LEN (42 * sizeof(BoxInfo2))
void DataSend::sendBox(BoxInfo2 *pBoxinfo, int num, uint32_t id, uint64_t _time)
{

        if (!can_send_box)
        {
                return;
        }

        if (num == 0)
        {
                int con_size = sizeof(BoxInfo2);
                PacketHead head(PACKET_HEAD_FLAG, BOX, 1, 1, con_size);
                PacketEnd end;
                end.frameTime = _time;
                head.picID = id;
                BoxInfo2 box;

                int packet_size1 = PACKET_HEAD_LEN + con_size + PACKET_END_LEN;
                char *send_buffer = new char[packet_size1];

                memcpy(send_buffer, &head, PACKET_HEAD_LEN);
                memcpy(send_buffer + PACKET_HEAD_LEN, &box, con_size);
                memcpy(send_buffer + PACKET_HEAD_LEN + con_size, &end, PACKET_END_LEN);

                // bool ret = pSocket->send(send_buffer, packet_size1);

                bool ret = pSocketTcp->send(send_buffer, packet_size1);
                if (!ret)
                {
                        LOG_ERR("send box fail!");
                }

                delete[] send_buffer;
                return;
        }

        int size = sizeof(BoxInfo2);
        int len = num * size;

        //42个框打一个包

        int packet_num;
        if (len % PACKET_BOX_LEN == 0)
        {
                packet_num = len / PACKET_BOX_LEN;
        }
        else
        {
                packet_num = len / PACKET_BOX_LEN + 1;
        }

        int last_packet_len = len - (packet_num - 1) * PACKET_BOX_LEN;

#ifdef _GPS
        int packet_size1 = PACKET_HEAD_LEN + PACKET_BOX_LEN + PACKET_END_LEN + sizeof(GPSData) + sizeof(AttitudeData);
        int packet_size2 = PACKET_HEAD_LEN + last_packet_len + PACKET_END_LEN + sizeof(GPSData) + sizeof(AttitudeData);
#else
        int packet_size1 = PACKET_HEAD_LEN + PACKET_BOX_LEN + PACKET_END_LEN;
        int packet_size2 = PACKET_HEAD_LEN + last_packet_len + PACKET_END_LEN;
#endif
        char *pPacket1 = new char[packet_size1];
        char *pPacket2 = new char[packet_size2];

#ifdef _GPS
        PacketHead head(PACKET_HEAD_FLAG, GPS, 0, packet_num, 0);
#else
        PacketHead head(PACKET_HEAD_FLAG, BOX, 0, packet_num, 0);
#endif
        PacketEnd end;

        end.frameTime = _time;
        head.picID = id;

        char *pSrc = (char *)pBoxinfo;

        for (int i = 0; i < packet_num && can_send_box; i++)
        {
                head.curFrame = i + 1;
                if (i == packet_num - 1)
                {
#ifdef _GPS
                        head.frameSize = last_packet_len + sizeof(GPSData) + sizeof(AttitudeData);
#else
                        head.frameSize = last_packet_len;
#endif

                        memcpy(pPacket2, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket2 + PACKET_HEAD_LEN, pSrc + len - last_packet_len, last_packet_len);
#ifdef _GPS
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len, &gps_data, sizeof(GPSData));
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len + sizeof(GPSData), &attitude_data, sizeof(AttitudeData));
#endif

#ifdef _GPS
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len + sizeof(GPSData) + sizeof(AttitudeData), &end, PACKET_END_LEN);
#else
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len, &end, PACKET_END_LEN);
#endif

                        // bool ret = pSocket->send(pPacket2, packet_size2);

                        bool ret = pSocketTcp->send(pPacket2, packet_size2);
                        if (!ret)
                        {
                                LOG_ERR("send box fail!");
                                break;
                        }
                }
                else
                {
#ifdef _GPS
                        head.frameSize = PACKET_BOX_LEN + sizeof(GPSData) + sizeof(AttitudeData);
#else
                        head.frameSize = PACKET_BOX_LEN;
#endif

                        memcpy(pPacket1, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket1 + PACKET_HEAD_LEN, pSrc + (i * PACKET_BOX_LEN), PACKET_BOX_LEN);
#ifdef _GPS
                        memcpy(pPacket1 + PACKET_HEAD_LEN + PACKET_BOX_LEN, &gps_data, sizeof(GPSData));
                        memcpy(pPacket1 + PACKET_HEAD_LEN + PACKET_BOX_LEN + sizeof(GPSData), &attitude_data, sizeof(AttitudeData));
#endif

#ifdef _GPS
                        memcpy(pPacket1 + PACKET_HEAD_LEN + PACKET_BOX_LEN + sizeof(GPSData) + sizeof(AttitudeData), &end, PACKET_END_LEN);
#else
                        memcpy(pPacket1 + PACKET_HEAD_LEN + PACKET_BOX_LEN, &end, PACKET_END_LEN);
#endif

                        // bool ret = pSocket->send(pPacket1, packet_size1);

                        bool ret = pSocketTcp->send(pPacket1, packet_size1);
                        if (!ret)
                        {
                                LOG_ERR("send box fail!");
                                break;
                        }
                }

                // send_sem.acquire();
        }

        delete[] pPacket1;
        delete[] pPacket2;
}
