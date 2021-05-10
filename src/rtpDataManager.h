/*
 * @Author: xtt
 * @Date: 2020-12-24 13:45:29
 * @Description: ...
 * @LastEditTime: 2021-04-08 19:54:54
 */

#include "byteArray.h"
#include "dataModel.h"
#include <cstring>
#include <deque>
#include <netinet/in.h>
#include <queue>
#include <vector>
#define RTP_PACKET_LEN 1500

struct RtpModel
{
        char *pdata = nullptr;
        int len = -1;
        unsigned short seq = -1;
        bool isValid = false;
        RtpModel(char *_data, int _len) : len(_len)
        {
                pdata = new char[_len];
                memcpy(pdata, _data, len);

                seq = ntohs(*(unsigned short *)&(pdata[2]));
                isValid = true;
        }
        RtpModel &operator=(const RtpModel &_rtpdata)
        {
                pdata = _rtpdata.pdata;
                len = _rtpdata.len;
                isValid = _rtpdata.isValid;
                seq = _rtpdata.seq;
                return *this;
        }
        RtpModel() = default;
};

class SocketUdp;

namespace std
{
        class thread;
}

typedef void (*pf_rtp_callback)(const char *, int, bool, uint32_t);

class RtpDataManager
{

public:
        RtpDataManager();
        ~RtpDataManager();

public:
        void init(const char *_ip, unsigned short _port, pf_rtp_callback, CODEC_TYPE _type);
        void start();
        void stop();
        void sendRtp() = delete;

private:
        static void receive_thread(void *arg);
        static void parser_thread(void *arg);
        bool parserRtpH264(RtpModel &, ByteArray &full_packet_data, bool &mark, uint32_t &timestamp);
        bool parserRtpH265(RtpModel &, ByteArray &full_packet_data, bool &mark, uint32_t &timestamp);

private:
        SocketUdp *pSocket = nullptr;
        SocketUdp *pSocket_send = nullptr;
        std::thread *pThread_receive = nullptr;
        std::thread *pThread_parser = nullptr;

        std::queue<RtpModel> que_rtp;
        std::queue<RtpModel> que_wait_send;
        std::vector<RtpModel> vec_rtp;

        pf_rtp_callback pf_callback = nullptr;
        CODEC_TYPE TYPE;
};