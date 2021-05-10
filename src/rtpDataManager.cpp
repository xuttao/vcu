#include "rtpDataManager.h"
#include "common.h"
#include "dataSend.h"
#include "log.h"
#include "rtpDataModel.h"
#include "rtspDataManager.h"
#include "socketbase.h"
#include <atomic>
#include <mutex>
#include <semaphore.h>
#include <thread>
namespace
{
        std::atomic_bool is_stop = {true};
        std::mutex buffer_mutex;
        std::mutex que_send_mutex;
        sem_t sem_buffer;
        sem_t sem_rtp;
        std::mutex mutex_rtp;
        bool start_send = false;
} // namespace

#define BUFFER_SIZE 4096
// #define SHOW_FRAME_TYPE

RtpDataManager::RtpDataManager()
{
}

RtpDataManager::~RtpDataManager()
{
        delete pSocket;
        while (!que_rtp.empty())
        {
                auto head = que_rtp.front();
                que_rtp.pop();
                delete[] head.pdata;
        }

        sem_destroy(&sem_buffer);
}

void RtpDataManager::init(const char *_ip, unsigned short _port, pf_rtp_callback _pf, CODEC_TYPE _type)
{
        pf_callback = _pf;
        TYPE = _type;

        pSocket = new SocketUdp;
        bool res = pSocket->createUdpServer(_ip, _port);
        FSERVO_CHECK(res);

        pSocket_send = new SocketUdp;
        std::string ip = Common::getNetPara().rtpSendIP;
        res = pSocket_send->createUdpClient(ip.c_str(), 8880);

        FSERVO_CHECK(res);
}

void RtpDataManager::start()
{
        // que_rtp.resize(QUE_SIZE);
        vec_rtp.resize(BUFFER_SIZE);

        is_stop = false;

        sem_init(&sem_buffer, 0, 0);

        pThread_receive = new std::thread(receive_thread, (void *)this);
        pThread_parser = new std::thread(parser_thread, (void *)this);
}

void RtpDataManager::stop()
{
        is_stop = true;
        sem_post(&sem_buffer);

        pThread_parser->join();
        delete pThread_parser;

        pThread_receive->join();
        delete pThread_receive;

        while (!que_rtp.empty())
        {
                auto head = que_rtp.front();
                que_rtp.pop();
                delete[] head.pdata;
        }

        char *ptemp = new char[1500 * 10];
        pSocket->read(ptemp, 1500 * 10);
        delete[] ptemp;
}
#include <iostream>
bool RtpDataManager::parserRtpH264(RtpModel &rtp_data, ByteArray &full_packet_data, bool &mark, uint32_t &timestamp) //收齐了一帧返回true，否则返回false
{
        char *pdata = rtp_data.pdata;
        int len = rtp_data.len;

        RTP_HEADER rtp_header = getRtpHeader(pdata);
        mark = rtp_header.mark;
        timestamp = rtp_header.timestamp;
// #ifdef _DEBUG
#if 1
        unsigned short seq = rtp_header.seq;
        static unsigned short last_seq = 0;
        if (seq - last_seq != 1 && last_seq != 0)
        {
                LOG_WARN("seq err:current:%d,last:%d", seq, last_seq);
        }
        last_seq = seq;
#endif

        RtspDataManager::getInstance()->setSSRC(rtp_header.ssrc);
        RtspDataManager::getInstance()->setSeq(rtp_header.seq);
        RtspDataManager::getInstance()->setRtpVersion(rtp_header.version);

        static bool is_start = false;

        bool need_box = DataSend::getInstance()->canSendBox();
        if (!need_box)
        {
                is_start = false;
                return false;
        }

        unsigned char key = pdata[12];

        if (key == 0x67 && !is_start)
        {
                is_start = true;
        }

        if (is_start)
        {
                unsigned char packet_type = key & 0x1f;
                static unsigned int iden = htonl(0x00000001);
                if (packet_type < 24) //单个NAL包
                {
                        // FSERVO_CHECK(rtp_header.mark == 1);
#ifdef SHOW_FRAME_TYPE
                        switch (key)
                        {
                                case 0x67:
                                        LOG_INFO("-----receive SPS frame");
                                        break;
                                case 0x68:
                                        LOG_INFO("-----receive PPS frame");
                                        break;
                                case 0x06:
                                        LOG_INFO("-----receive SEI frame");
                                        break;
                                default:
                                        LOG_INFO("-----receive other single frame");
                        }
#endif

                        full_packet_data.append((char *)&iden, sizeof(int));
                        full_packet_data.append(pdata + 12, len - 12);
                        return true;
                }
                else if (packet_type == 28) //分片模式
                {
                        unsigned char fu_header = pdata[13];
                        unsigned char S = (fu_header & 0x80) >> 7;
                        unsigned char E = (fu_header & 0x40) >> 6;
                        unsigned char R = (fu_header & 0x20) >> 5;

                        unsigned char nalu_header = 0;
                        nalu_header = nalu_header | (key & 0xe0);
                        nalu_header = nalu_header | (fu_header & 0x1f);

#ifdef SHOW_FRAME_TYPE
                        switch (key)
                        {
                                case 0x65:
                                        LOG_INFO("-----receive IDR frame");
                                        break;
                                case 0x61:
                                        LOG_INFO("-----receive non-IDR frame");
                                        break;
                                case 0x01:
                                        LOG_INFO("-----receive B frame");
                                        break;
                                default:
                                        LOG_INFO("-----receive other single frame");
                        }
#endif

                        bool res = false;
                        if (S == 1) //分片开始
                        {
                                FSERVO_CHECK(rtp_header.mark == 0);
                                full_packet_data.append((char *)&iden, sizeof(int));
                                full_packet_data.append(nalu_header);
                                full_packet_data.append(pdata + 14, len - 14);
                        }
                        else if (E == 1) //分片结束
                        {
                                FSERVO_CHECK(rtp_header.mark == 1);
                                full_packet_data.append(pdata + 14, len - 14);
                                res = true;

                                // mark = true;
                                // timestamp = rtp_header.timestamp;
                        }
                        else
                        {
                                full_packet_data.append(pdata + 14, len - 14);
                        }
                        // return res;
                        return true;
                }
                else
                {
                        FSERVO_CHECK(false);
                }
        }
        return false;
}

bool RtpDataManager::parserRtpH265(RtpModel &rtp_data, ByteArray &full_packet_data, bool &mark, uint32_t &timestamp)
{
        char *pdata = rtp_data.pdata;
        int len = rtp_data.len;

        RTP_HEADER rtp_header = getRtpHeader(pdata);
        if (rtp_header.mark)
        {
                mark = true;
                timestamp = rtp_header.timestamp;
        }
// #ifdef _DEBUG
#if 1
        auto seq = rtp_header.seq;
        static unsigned short last_seq = 0;
        if (seq - last_seq != 1 && last_seq != 0)
        {
                LOG_WARN("seq err:current:%d,last:%d", seq, last_seq);
        }
        last_seq = seq;
#endif

        RtspDataManager::getInstance()->setSSRC(rtp_header.ssrc);
        RtspDataManager::getInstance()->setSeq(rtp_header.seq);
        RtspDataManager::getInstance()->setRtpVersion(rtp_header.version);
        /***********************************************
                h265 三种模式，单一NAL、分片、组合
                nal unit type 48 为组合，49为分片,
                1、19 、32、33为单一
                32 视频参数集vps
                33 序列参数集sps
                34 序列参数集pps
                39 增强信息sei
        ************************************************/
        static bool is_start = false;

        bool need_box = DataSend::getInstance()->canSendBox();
        if (!need_box)
        {
                is_start = false;
                return false;
        }

        unsigned char key = pdata[12];

        unsigned char unit_type = (key >> 1) & 0x3f;

        if (key == 0x40 && !is_start)
        {
                is_start = true;
        }

        if (is_start)
        {
                static unsigned int iden = htonl(0x00000001);

                if (49 == unit_type) //分片
                {
                        unsigned char Fu_Header = pdata[14];
                        unsigned char S = (Fu_Header >> 7) & 0x01;
                        unsigned char E = (Fu_Header >> 6) & 0x01;
                        unsigned char FuType = Fu_Header & 0x1f;

                        unsigned char nalu_header_1 = 0;
                        unsigned char nalu_header_2 = pdata[13];

                        nalu_header_1 = (nalu_header_1 | (Fu_Header & 0x3f)) << 1;
                        nalu_header_1 = nalu_header_1 | (key & 0x80);

                        bool res = false;
                        if (1 == S) //分片开始
                        {
                                full_packet_data.append((char *)&iden, sizeof(int));
                                full_packet_data.append(nalu_header_1);
                                full_packet_data.append(nalu_header_2);
                                full_packet_data.append(pdata + 15, len - 15);

                                FSERVO_CHECK(rtp_header.mark == 0);
                        }
                        else if (1 == E) //分片结束
                        {
                                full_packet_data.append(pdata + 15, len - 15);
                                res = true;
                                FSERVO_CHECK(rtp_header.mark == 1);

                                // mark = true;
                                // timestamp = rtp_header.timestamp;
                        }
                        else
                        {
                                full_packet_data.append(pdata + 15, len - 15);
                        }
                        // return res;
                        return true;
                }
                else if (48 == unit_type) //组合
                {
                        LOG_ERR("not support this unit_type:%d", 48);
                        FSERVO_CHECK(false);
                }
                else //单一
                {
                        full_packet_data.append((char *)&iden, sizeof(int));
                        full_packet_data.append(pdata + 12, len - 12);

                        return true;
                }
        }

        return false;
}

void RtpDataManager::receive_thread(void *arg)
{
        LOG_INFO("rtp receive thread start! id:%u", std::this_thread::get_id());

        RtpDataManager *pThis = static_cast<RtpDataManager *>(arg);
        char *receive_buffer = new char[1500];

        uint64_t index = 0;
        int times = 0;

        for (;;)
        {
                if (is_stop) return;

                int ret = pThis->pSocket->read(receive_buffer, 1500);
                if (ret <= 0)
                {
                        usleep(1 * 1000);
                        LOG_WARN("packet lost!!!");
                        continue;
                }

                RtpModel rtpData(receive_buffer, ret);

                unsigned short seq = ntohs(*(unsigned short *)&(receive_buffer[2]));

                static unsigned short first_seq = seq;

                // int pos = index % BUFFER_SIZE;

                int pos = ((uint64_t)(seq + 65536 * times) - first_seq) % BUFFER_SIZE;

                if (UINT16_MAX == seq)
                {
                        times++;
                }

                buffer_mutex.lock();

                if (pThis->vec_rtp[pos].isValid == true)
                {
                        LOG_WARN("packet is not use!!! seq:%d", pThis->vec_rtp[pos].seq);
                        delete[] pThis->vec_rtp[pos].pdata;
                }

                pThis->vec_rtp[pos] = rtpData;

                buffer_mutex.unlock();

                index++;
                if (index > 30)
                {
                        sem_post(&sem_buffer);
                }

                // sem_post(&sem_buffer);
        }

        LOG_INFO("rtp receive thread exit");
}

void RtpDataManager::parser_thread(void *arg)
{
        LOG_INFO("rtp parser thread start! id:%u", std::this_thread::get_id());
        RtpDataManager *pThis = static_cast<RtpDataManager *>(arg);
        uint64_t index = 0; //注意此处，对应起始seq

        for (;;)
        {
                if (is_stop) return;

                int ret = sem_wait(&sem_buffer);

                int pos = index % BUFFER_SIZE;

                buffer_mutex.lock();

                auto data = pThis->vec_rtp[pos];

                if (false == data.isValid)
                {
                        LOG_WARN("packet is not valid!!! seq:%d,pos:%d", data.seq, pos);
                        buffer_mutex.unlock();
                        index++;
                        continue;
                }
                pThis->vec_rtp[pos].isValid = false;

                buffer_mutex.unlock();

                // char *pdata = data.pdata;
                // unsigned short seq = data.seq;

                static ByteArray packet_data;
                bool res = false;

                bool mark = false;
                uint32_t timestamp;
                if (CODEC_TYPE::H264 == pThis->TYPE)
                        res = pThis->parserRtpH264(data, packet_data, mark, timestamp);
                else
                        res = pThis->parserRtpH265(data, packet_data, mark, timestamp);

                if (res)
                {
                        //送入vcu
                        (pThis->pf_callback)(packet_data.data(), packet_data.size(), mark, timestamp);

                        // LOG_INFO("received full packet:%d", packet_data.size());

                        packet_data.clear();
                }
                if (DataSend::getInstance()->canSendCon())
                {
                        res = pThis->pSocket_send->send(data.pdata, data.len);
                        FSERVO_CHECK(res);
                }

                delete[] data.pdata;

                index++;
        }

        LOG_INFO("rtp parser thread exit");
}
